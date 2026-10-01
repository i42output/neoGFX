 // color_dialog.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
  This program is free software: you can redistribute it and / or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <neogfx/neogfx.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <boost/algorithm/string/trim.hpp>

#include <neolib/task/thread.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/app/i_clipboard.hpp>
#include <neogfx/app/file_dialog.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/dialog/message_box.hpp>
#include <neogfx/gui/dialog/color_dialog.hpp>

namespace neogfx
{
    namespace
    {
        class swatch_presentation_model : public default_drop_list_presentation_model<>
        {
            typedef default_drop_list_presentation_model<> base_type;
        private:
            static constexpr std::size_t kSwatchColumns = 12;
        public:
            swatch_presentation_model(drop_list& aDropList, color_dialog const& aOwner) : base_type{ aDropList }, iOwner{ aOwner }
            {
                iSink += service<i_app>().current_style_changed([this](style_aspect)
                {
                    iTextures.clear();
                    VisualAppearanceChanged.async_trigger();
                });
            }
        public:
            static scalar cell_extent() { return 2.0_dip; } // 24dip swatch grid in drop list view
            static scalar border_width() { return 1.0_dip; }
            static scalar icon_extent() { return cell_extent() * kSwatchColumns + border_width() * 2.0; }
        public:
            optional_size cell_image_size(item_presentation_model_index const&) const final
            {
                return size{ icon_extent() };
            }
            optional_texture cell_image(item_presentation_model_index const& aIndex) const final
            {
                auto const row = to_item_model_index(aIndex).row();
                if (row >= iOwner.swatches().size())
                    return optional_texture{};
                auto const& colors = iOwner.swatches()[row].colors;
                auto existing = iTextures.find(row);
                if (existing != iTextures.end() && existing->second.first == colors)
                    return existing->second.second;
                dimension const d = icon_extent();
                scalar const border = border_width();
                scalar const cellExtent = cell_extent();
                texture newTexture{ size{ d, d }, 1.0, texture_sampling::Multisample };
                graphics_context gc{ newTexture };
                auto const edge = [&](std::size_t aCell) { return border + aCell * cellExtent; };
                for (std::size_t slot = 0; slot < colors.size(); ++slot)
                {
                    auto const gridColumn = slot % kSwatchColumns;
                    auto const gridRow = slot / kSwatchColumns;
                    // texture render target is bottom-up so flip rows
                    point const topLeft{ edge(gridColumn), d - edge(gridRow + 1) };
                    point const bottomRight{ edge(gridColumn + 1), d - edge(gridRow) };
                    gc.fill_rect(rect{ topLeft, size{ bottomRight.x - topLeft.x, bottomRight.y - topLeft.y } }, colors[slot]);
                }
                auto const textColor = service<i_app>().current_style().palette().color(color_role::Text);
                gc.draw_rect(rect{ point{ border / 2.0, border / 2.0 }, size{ d - border, d - border } }, pen{ textColor, border });
                iTextures.insert_or_assign(row, std::make_pair(colors, newTexture));
                return newTexture;
            }
        private:
            color_dialog const& iOwner;
            mutable std::map<std::uint32_t, std::pair<std::vector<color>, texture>> iTextures;
            sink iSink;
        };

        texture picker_mode_icon(bool aWheel)
        {
            auto const textColor = service<i_app>().current_style().palette().color(color_role::Text);
            dimension const d = std::ceil(16.0_dip);
            texture icon{ size{ d, d }, 1.0, texture_sampling::Multisample };
            graphics_context gc{ icon };
            dimension const lineWidth = 1.0_dip;
            pen const linePen{ textColor, lineWidth };
            if (aWheel)
            {
                point const center{ d / 2.0, d / 2.0 };
                auto const radius = (d - lineWidth) / 2.0;
                gc.draw_circle(center, radius, linePen);
                auto const triangleRadius = radius - 2.0_dip;
                auto const vertex = [&](scalar aDegrees)
                {
                    auto const a = aDegrees * math::pi<double>() / 180.0;
                    return point{ center.x + triangleRadius * std::cos(a), center.y + triangleRadius * std::sin(a) }; // texture is bottom-up so +y is up
                };
                gc.draw_triangle(vertex(90.0), vertex(210.0), vertex(330.0), linePen);
            }
            else
            {
                // strokes are centred on the rect edges so inset by half the line width
                // wide (Y-Z) rect occupies 0..11, 1dip gap, narrow (X) rect occupies 12..16
                gc.draw_rect(rect{ point{ 0.5_dip, 2.5_dip }, size{ 10.0_dip, 10.0_dip } }, linePen);
                gc.draw_rect(rect{ point{ 12.5_dip, 2.5_dip }, size{ 3.0_dip, 10.0_dip } }, linePen);
            }
            return icon;
        }
    }

    color_dialog::color_box::color_box(color_dialog& aOwner, const optional_color& aColor, const optional_custom_color_list_iterator& aCustomColor, const std::optional<std::size_t>& aSwatchSlot) :
        base_type(frame_style::SolidFrame), iOwner(aOwner), iColor(aColor), iCustomColor(aCustomColor), iSwatchSlot(aSwatchSlot)
    {
        set_padding(neogfx::padding{});
    }

    size color_dialog::color_box::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (has_minimum_size())
            return base_type::minimum_size(aAvailableSpace);
        return ceil_rasterized(base_type::minimum_size(aAvailableSpace) + size{ 4_mm, 3.5_mm });
    }

    size color_dialog::color_box::maximum_size(optional_size const& aAvailableSpace) const
    {
        if (has_maximum_size())
            return base_type::maximum_size(aAvailableSpace);
        return minimum_size();
    }

    void color_dialog::color_box::paint_non_client(i_graphics_context& aGc) const
    {
        if (unused_swatch_slot())
            return; // no frame or background for unused swatch slot
        base_type::paint_non_client(aGc);
    }

    void color_dialog::color_box::paint(i_graphics_context& aGc) const
    {
        if (unused_swatch_slot())
            return;
        const optional_color fillColor = (iSwatchSlot != std::nullopt ? iOwner.swatch_color(*iSwatchSlot) : iCustomColor == std::nullopt ? iColor : **iCustomColor);
        base_type::paint(aGc);
        draw_alpha_background(aGc, client_rect(false));
        if (fillColor != std::nullopt)
            aGc.fill_rect(client_rect(false), *fillColor);
        if ((iCustomColor != std::nullopt && iOwner.current_custom_color() == *iCustomColor) ||
            (iSwatchSlot != std::nullopt && iOwner.current_swatch_editable() && iOwner.current_swatch_color() == iSwatchSlot))
        {
            auto const radius = client_rect(false).width() * 0.28125;
            aGc.fill_circle(client_rect(false).center(), radius, color::White);
            aGc.fill_circle(client_rect(false).center(), radius - 1.0_dip, color::Black);
        }
    }

    bool color_dialog::color_box::unused_swatch_slot() const
    {
        return iSwatchSlot != std::nullopt && iOwner.swatch_color(*iSwatchSlot) == std::nullopt;
    }

    void color_dialog::color_box::mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier)
    {
        base_type::mouse_button_clicked(aButton, aPosition, aKeyModifier);
        if (aButton == mouse_button::Left)
        {
            if (iSwatchSlot != std::nullopt)
            {
                auto const swatchColor = iOwner.swatch_color(*iSwatchSlot);
                if (swatchColor != std::nullopt)
                {
                    if (iOwner.current_swatch_editable())
                        iOwner.select_color(*swatchColor);
                    else
                        iOwner.select_color(swatchColor->with_alpha(iOwner.selected_color().alpha()));
                    iOwner.set_current_swatch_color(*iSwatchSlot);
                }
            }
            else if (iCustomColor == std::nullopt)
            {
                if (iColor != std::nullopt)
                    iOwner.select_color(iColor->with_alpha(iOwner.selected_color().alpha()));
                else
                    service<i_basic_services>().system_beep();
            }
            else
            {
                if (**iCustomColor != std::nullopt)
                    iOwner.select_color(***iCustomColor);
                iOwner.set_current_custom_color(*iCustomColor);
            }
        }
        else if (aButton == mouse_button::Right)
        {
            if (iCustomColor != std::nullopt)
                iOwner.set_current_custom_color(*iCustomColor);
            else if (iSwatchSlot != std::nullopt && iOwner.swatch_color(*iSwatchSlot) != std::nullopt)
                iOwner.set_current_swatch_color(*iSwatchSlot);
        }
    }

    namespace
    {
        const char* sLeftXPickerCursorImage
        {
            "[9,9]"
            "{0,paper}"
            "{1,black}"
            "{2,white}"

            "111110000"
            "122221000"
            "122222100"
            "122222210"
            "122222221"
            "122222210"
            "122222100"
            "122221000"
            "111110000"
        };
        const char* sLeftXPickerCursorHighDpiImage
        {
            "[19,19]"
            "{0,paper}"
            "{1,black}"
            "{2,white}"

            "1111111111000000000"
            "1222222222100000000"
            "1222222222210000000"
            "1222222222221000000"
            "1222222222222100000"
            "1222222222222210000"
            "1222222222222221000"
            "1222222222222222100"
            "1222222222222222210"
            "1222222222222222221"
            "1222222222222222210"
            "1222222222222222100"
            "1222222222222221000"
            "1222222222222210000"
            "1222222222222100000"
            "1222222222221000000"
            "1222222222210000000"
            "1222222222100000000"
            "1111111111000000000"
        };
        const char* sRightXPickerCursorImage
        {
            "[9,9]"
            "{0,paper}"
            "{1,black}"
            "{2,white}"

            "000011111"
            "000122221"
            "001222221"
            "012222221"
            "122222221"
            "012222221"
            "001222221"
            "000122221"
            "000011111"
        };
        const char* sRightXPickerCursorHighDpiImage
        {
            "[19,19]"
            "{0,paper}"
            "{1,black}"
            "{2,white}"

            "0000000001111111111"
            "0000000012222222221"
            "0000000122222222221"
            "0000001222222222221"
            "0000012222222222221"
            "0000122222222222221"
            "0001222222222222221"
            "0012222222222222221"
            "0122222222222222221"
            "1222222222222222221"
            "0122222222222222221"
            "0012222222222222221"
            "0001222222222222221"
            "0000122222222222221"
            "0000012222222222221"
            "0000001222222222221"
            "0000000122222222221"
            "0000000012222222221"
            "0000000001111111111"
        };

    }

    color_dialog::x_picker::cursor_widget::cursor_widget(x_picker& aParent, type_e aType) :
        image_widget{
            aParent.iOwner.client_widget(),
            neogfx::image{ aType == LeftCursor ?
                aParent.dpi_select(sLeftXPickerCursorImage, sLeftXPickerCursorHighDpiImage) :
                aParent.dpi_select(sRightXPickerCursorImage, sRightXPickerCursorHighDpiImage),
            { { "paper", color{} }, { "black", color::Black } , { "white", color::White } }, aParent.dpi_select(1.0, 2.0) } },
        iParent(aParent)
    {
        set_ignore_mouse_events(false);
        resize(minimum_size());
    }

    void color_dialog::x_picker::cursor_widget::mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier)
    {
        image_widget::mouse_button_clicked(aButton, aPosition, aKeyModifier);
        if (aButton == mouse_button::Left)
            iDragOffset = point{ aPosition - client_rect().center() };
    }

    void color_dialog::x_picker::cursor_widget::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        image_widget::mouse_button_released(aButton, aPosition);
        if (!capturing())
            iDragOffset = std::nullopt;
    }

    void color_dialog::x_picker::cursor_widget::mouse_moved(const point& aPosition, key_modifier aKeyModifier)
    {
        if (iDragOffset != std::nullopt)
        {
            point pt{ aPosition - *iDragOffset };
            pt += position();
            pt -= iParent.position();
            pt += size{ iParent.effective_frame_width() };
            iParent.select(point{ 0.0, pt.y});
        }
    }

    color_dialog::x_picker::x_picker(color_dialog& aOwner) :
        base_type(aOwner.iRightTopLayout),
        iOwner(aOwner), 
        iTracking(false),
        iLeftCursor(*this, cursor_widget::LeftCursor),
        iRightCursor(*this, cursor_widget::RightCursor)
    {
        iSink = iOwner.SelectionChanged([this]()
        {
            update_cursors();
            update();
        });
        iSink += iOwner.ColorSpaceChanged([this]()
        {
            update_cursors();
            update();
        });
        iSink += VisibilityChanged([this]()
        {
            iLeftCursor.show(visible());
            iRightCursor.show(visible());
        });
        update_cursors();
    }

    scalar color_dialog::x_picker::cursor_width() const
    {
        return iRightCursor.extents().cx;
    }

    size color_dialog::x_picker::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (has_minimum_size())
            return base_type::minimum_size(aAvailableSpace);
        return base_type::minimum_size(aAvailableSpace) + size{ 32_dip, 256_dip };
    }

    size color_dialog::x_picker::maximum_size(optional_size const& aAvailableSpace) const
    {
        if (has_maximum_size())
            return base_type::maximum_size(aAvailableSpace);
        return minimum_size();
    }

    void color_dialog::x_picker::moved()
    {
        base_type::moved();
        update_cursors();
    }

    void color_dialog::x_picker::resized()
    {
        base_type::resized();
        update_cursors();
    }

    void color_dialog::x_picker::paint(i_graphics_context& aGc) const
    {
        base_type::paint(aGc);
        scoped_units su{ *this, units::Pixels };
        rect cr = client_rect(false);
        if (iOwner.current_channel() == ChannelAlpha)
            draw_alpha_background(aGc, cr);
        for (std::uint32_t y = 0; y < cr.height(); ++y)
        {
            rect line{ cr.top_left() + point{ 0.0, static_cast<coordinate>(y) }, size{ cr.width(), 1.0 } };
            auto r = color_at_position(point{ 0.0, static_cast<coordinate>(y) * (1.0 / dpi_scale_factor())});
            color rgb;
            if (std::holds_alternative<hsv_color>(r))
            {
                hsv_color hsv = static_variant_cast<hsv_color>(r);
                if (iOwner.current_channel() == ChannelHue)
                {
                    hsv.set_saturation(1.0);
                    hsv.set_value(1.0);
                }
                rgb = hsv.to_rgb<color>();
            }
            else
                rgb = static_variant_cast<color>(r);
            if (iOwner.current_channel() != ChannelAlpha)
                rgb.set_alpha(255);
            aGc.fill_rect(line, rgb);
        }
    }

    void color_dialog::x_picker::mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier)
    {
        base_type::mouse_button_clicked(aButton, aPosition, aKeyModifier);
        if (aButton == mouse_button::Left)
        {
            select(aPosition - client_rect(false).top_left());
            iTracking = true;
        }
    }

    void color_dialog::x_picker::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        base_type::mouse_button_released(aButton, aPosition);
        if (!capturing())
            iTracking = false;
    }

    void color_dialog::x_picker::mouse_moved(const point& aPosition, key_modifier aKeyModifier)
    {
        if (iTracking)
            select(aPosition - client_rect(false).top_left());
    }

    neogfx::mouse_cursor color_dialog::x_picker::mouse_cursor() const
    {
        point mousePos = mouse_position();
        if (client_rect(false).contains(mousePos))
            return mouse_system_cursor::Crosshair;
        return base_type::mouse_cursor();
    }

    void color_dialog::x_picker::select(const point& aPosition)
    {
        iOwner.select_color(color_at_position(aPosition * (1.0 / dpi_scale_factor())), *this);
    }

    color_dialog::representations color_dialog::x_picker::color_at_position(const point& aCursorPos) const
    {
        point pos = aCursorPos.min(point{ 255.0, 255.0 }).max(point{ 0.0, 0.0 });
        switch (iOwner.current_channel())
        {
        case ChannelHue:
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                hsv.set_hue((255.0 - pos.y) / 255.0 * 359.9);
                return hsv;
            }
            break;
        case ChannelSaturation:
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                hsv.set_saturation((255.0 - pos.y) / 255.0);
                return hsv;
            }
            break;
        case ChannelValue:
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                hsv.set_value((255.0 - pos.y) / 255.0);
                return hsv;
            }
            break;
        case ChannelRed:
            {
                auto rgb = iOwner.selected_color();
                rgb.set_red(static_cast<color::component>(to_sRGB(*iOwner.iColorSpace, static_cast<color::component>(255.0 - pos.y), 255.0)));
                return rgb;
            }
            break;
        case ChannelGreen:
            {
                auto rgb = iOwner.selected_color();
                rgb.set_green(static_cast<color::component>(to_sRGB(*iOwner.iColorSpace, static_cast<color::component>(255.0 - pos.y), 255.0)));
                return rgb;
            }
            break;
        case ChannelBlue:
            {
                auto rgb = iOwner.selected_color();
                rgb.set_blue(static_cast<color::component>(to_sRGB(*iOwner.iColorSpace, static_cast<color::component>(255.0 - pos.y), 255.0)));
                return rgb;
            }
            break;
        case ChannelAlpha:
            if (iOwner.current_mode() == ModeHSV)
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                hsv.set_alpha((255.0 - pos.y) / 255.0);
                return hsv;
            }
            else
            {
                auto rgb = iOwner.selected_color();
                rgb.set_alpha(static_cast<color::component>(255.0 - pos.y));
                return rgb;
            }
            break;
        default:
            return color::Black;
        }
    }

    void color_dialog::x_picker::update_cursors()
    {
        parent().update(parent().to_client_coordinates(iLeftCursor.to_window_coordinates(iLeftCursor.client_rect(true))));
        parent().update(parent().to_client_coordinates(iRightCursor.to_window_coordinates(iRightCursor.client_rect(true))));
        iLeftCursor.move(dpi_scale(current_cursor_position()) + position() + client_rect(false).top_left() + point{ -iLeftCursor.extents().cx - effective_frame_width(), -std::floor(iLeftCursor.client_rect().center().y) });
        iRightCursor.move(dpi_scale(current_cursor_position()) + position() + client_rect(false).top_right() + point{ effective_frame_width(), -std::floor(iLeftCursor.client_rect().center().y) });
        parent().update(parent().to_client_coordinates(iLeftCursor.to_window_coordinates(iLeftCursor.client_rect(true))));
        parent().update(parent().to_client_coordinates(iRightCursor.to_window_coordinates(iRightCursor.client_rect(true))));
    }

    point color_dialog::x_picker::current_cursor_position() const
    {
        switch (iOwner.current_channel())
        {
        case ChannelHue:
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                return point{ 0.0, 255.0 - hsv.hue() / 360.0 * 255.0};
            }
            break;
        case ChannelSaturation:
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                return point{ 0.0, 255.0 - hsv.saturation() * 255.0 };
            }
            break;
        case ChannelValue:
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                return point{ 0.0, 255.0 - hsv.value() * 255.0 };
            }
            break;
        case ChannelRed:
            {
                auto rgb = iOwner.selected_color();
                return point{ 0.0, 255.0 - from_sRGB(*iOwner.iColorSpace, static_cast<coordinate>(rgb.red()), 255.0) };
            }
            break;
        case ChannelGreen:
            {
                auto rgb = iOwner.selected_color();
                return point{ 0.0, 255.0 - from_sRGB(*iOwner.iColorSpace, static_cast<coordinate>(rgb.green()), 255.0) };
            }
            break;
        case ChannelBlue:
            {
                auto rgb = iOwner.selected_color();
                return point{ 0.0, 255.0 - from_sRGB(*iOwner.iColorSpace, static_cast<coordinate>(rgb.blue()), 255.0) };
            }
            break;
        case ChannelAlpha:
            if (iOwner.current_mode() == ModeHSV)
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                return point{ 0.0, 255.0 - static_cast<coordinate>(hsv.alpha() * 255.0) };
            }
            else
            {
                auto rgb = iOwner.selected_color();
                return point{ 0.0, 255.0 - static_cast<coordinate>(rgb.alpha()) };
            }
            break;
        default:
            return point{};
        }
    }

    color_dialog::yz_picker::yz_picker(color_dialog& aOwner) :
        framed_scrollable_widget(aOwner.iRightTopLayout), iOwner(aOwner), iLayout{ *this }, iCanvas{ iLayout }, iTexture{ image{ size{256, 256}, color::Black } }, iTracking{ false },
        iAnimationTimer{ *this, [this](widget_timer& aTimer)
        {
            aTimer.again();
            animate();
        }, std::chrono::milliseconds{ 10 }, true }
    {
        iCanvas.set_image(iTexture);
        iCanvas.set_fixed_size(size{ 256.0_dip, 256.0_dip });
        iCanvas.set_aspect_ratio(aspect_ratio::Stretch);
        set_fixed_size(size{ 256.0_dip, 256.0_dip } + size{ effective_frame_width() * 2.0 });
        iLayout.set_padding(neogfx::padding{});
        set_padding(neogfx::padding{});
        iOwner.SelectionChanged([this]
        {
            update_texture();
        });
        iOwner.ColorSpaceChanged([this]
        {
            update_texture();
        });
        update_texture();
        iCanvas.Painted([this](i_graphics_context& aGc)
        {
            if (iImage)
                return;
            point center = dip(current_cursor_position());
            auto const radius = dip(CURSOR_RADIUS);
            auto const circumference = 2.0 * math::pi<double>() * radius;
            aGc.draw_circle(center, radius, pen{ color::White, dip(CURSOR_THICKNESS) });
            aGc.draw_circle(center, radius, pen{ color::Black, dip(CURSOR_THICKNESS), 
                line_dash{ 0x5555u, circumference / 6.0, circumference * neolib::this_process::elapsed_ms() / 1000.0 } });
        });
    }

    void color_dialog::yz_picker::set_image(image&& aImage)
    {
        iImage.emplace(std::move(aImage));
        iCanvas.set_image(*iImage);
        iOwner.iXPicker.hide();
        update();
    }

    void color_dialog::yz_picker::clear_image()
    {
        auto const selection = iOwner.selected_color();
        iImage.reset();
        iCanvas.set_image(iTexture);
        iCursorPosition.reset();
        iOwner.iXPicker.show();
        iOwner.select_color(selection);
        update();
    }

    void color_dialog::yz_picker::mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier)
    {
        framed_scrollable_widget::mouse_button_clicked(aButton, aPosition, aKeyModifier);
        if (aButton == mouse_button::Left && client_rect().contains(aPosition))
        {
            select(aPosition - client_rect(false).top_left());
            iTracking = true;
        }
    }

    void color_dialog::yz_picker::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        framed_scrollable_widget::mouse_button_released(aButton, aPosition);
        if (!capturing())
            iTracking = false;
    }

    void color_dialog::yz_picker::mouse_moved(const point& aPosition, key_modifier aKeyModifier)
    {
        framed_scrollable_widget::mouse_moved(aPosition, aKeyModifier);
        if (iTracking)
            select(aPosition - client_rect(false).top_left());
    }

    neogfx::mouse_cursor color_dialog::yz_picker::mouse_cursor() const
    {
        point mousePos = mouse_position();
        if (client_rect(false).contains(mousePos))
            return mouse_system_cursor::Crosshair;
        return framed_scrollable_widget::mouse_cursor();
    }

    void color_dialog::yz_picker::select(const point& aPosition)
    {
        if (iImage)
            iCursorPosition = aPosition;
        iOwner.select_color(color_at_position(aPosition * (1.0 / dpi_scale_factor())), *this);
    }

    color_dialog::representations color_dialog::yz_picker::color_at_position(const point& aCursorPos) const
    {
        point pos{ std::max(std::min(aCursorPos.x, 255.0), 0.0), std::max(std::min(aCursorPos.y, 255.0), 0.0) };
        if (iImage)
        {
            auto const imagePos = ((pos + scroll_position()) * dpi_scale_factor() / iCanvas.extents() * (iImage->extents() - size{ 1.0, 1.0 })).floor();
            return iImage->get_pixel(imagePos);
        }
        switch (iOwner.current_channel())
        {
        case ChannelHue:
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                hsv.set_saturation(std::max(std::min(pos.x / 255.0, 1.0), 0.0));
                hsv.set_value(std::max(std::min((255.0 - pos.y) / 255.0, 1.0), 0.0));
                return hsv;
            }
            break;
        case ChannelSaturation:
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                hsv.set_hue(std::max(std::min(pos.x / 255.0 * 360.0, 360.0), 0.0));
                hsv.set_value(std::max(std::min((255.0 - pos.y) / 255.0, 1.0), 0.0));
                return hsv;
            }
            break;
        case ChannelValue:
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                hsv.set_hue(std::max(std::min(pos.x / 255.0 * 360.0, 360.0), 0.0));
                hsv.set_saturation(std::max(std::min((255.0 - pos.y) / 255.0, 1.0), 0.0));
                return hsv;
            }
            break;
        case ChannelRed:
            {
                auto rgb = iOwner.selected_color();
                rgb.set_blue(static_cast<color::component>(to_sRGB(*iOwner.iColorSpace, pos.x, 255.0)));
                rgb.set_green(static_cast<color::component>(to_sRGB(*iOwner.iColorSpace, 255.0 - pos.y, 255.0)));
                return rgb;
            }
            break;
        case ChannelGreen:
            {
                auto rgb = iOwner.selected_color();
                rgb.set_blue(static_cast<color::component>(to_sRGB(*iOwner.iColorSpace, pos.x, 255.0)));
                rgb.set_red(static_cast<color::component>(to_sRGB(*iOwner.iColorSpace, 255.0 - pos.y, 255.0)));
                return rgb;
            }
            break;
        case ChannelBlue:
            {
                auto rgb = iOwner.selected_color();
                rgb.set_red(static_cast<color::component>(to_sRGB(*iOwner.iColorSpace, pos.x, 255.0)));
                rgb.set_green(static_cast<color::component>(to_sRGB(*iOwner.iColorSpace, 255.0 - pos.y, 255.0)));
                return rgb;
            }
            break;
        case ChannelAlpha:
            if (iOwner.current_mode() == ModeHSV)
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                hsv.set_saturation(std::max(std::min(pos.x / 255.0, 1.0), 0.0));
                hsv.set_value(std::max(std::min((255.0 - pos.y) / 255.0, 1.0), 0.0));
                return hsv;
            }
            else
            {
                auto rgb = iOwner.selected_color();
                rgb.set_blue(static_cast<color::component>(to_sRGB(*iOwner.iColorSpace, pos.x, 255.0)));
                rgb.set_green(static_cast<color::component>(to_sRGB(*iOwner.iColorSpace, 255.0 - pos.y, 255.0)));
                return rgb;
            }
            break;
        }
        return color::Black;
    }

    point color_dialog::yz_picker::current_cursor_position() const
    {
        if (iCursorPosition)
            return *iCursorPosition;
        switch (iOwner.current_channel())
        {
        case ChannelHue:
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                return point{ hsv.saturation() * 255.0, (1.0 - hsv.value()) * 255.0 };
            }
            break;
        case ChannelSaturation:
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                return point{ hsv.hue() / 360.0 * 255.0, (1.0 - hsv.value()) * 255.0 };
            }
            break;
        case ChannelValue:
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                return point{ hsv.hue() / 360.0 * 255.0, (1.0 - hsv.saturation()) * 255.0 };
            }
            break;
        case ChannelRed:
            {
                auto rgb = iOwner.selected_color();
                return point{ from_sRGB(*iOwner.iColorSpace, static_cast<coordinate>(rgb.blue()), 255.0), from_sRGB(*iOwner.iColorSpace, static_cast<coordinate>(255 - rgb.green()), 255.0) };
            }
            break;
        case ChannelGreen:
            {
                auto rgb = iOwner.selected_color();
                return point{ from_sRGB(*iOwner.iColorSpace, static_cast<coordinate>(rgb.blue()), 255.0), from_sRGB(*iOwner.iColorSpace, static_cast<coordinate>(255 - rgb.red()), 255.0) };
            }
            break;
        case ChannelBlue:
            {
                auto rgb = iOwner.selected_color();
                return point{ from_sRGB(*iOwner.iColorSpace, static_cast<coordinate>(rgb.red()), 255.0), from_sRGB(*iOwner.iColorSpace, static_cast<coordinate>(255 - rgb.green()), 255.0) };
            }
            break;
        case ChannelAlpha:
            if (iOwner.current_mode() == ModeHSV)
            {
                auto hsv = iOwner.selected_color_as_hsv(true);
                return point{ hsv.saturation() * 255.0, (1.0 - hsv.value()) * 255.0 };
            }
            else
            {
                auto rgb = iOwner.selected_color();
                return point{ from_sRGB(*iOwner.iColorSpace, static_cast<coordinate>(rgb.blue()), 255.0), from_sRGB(*iOwner.iColorSpace, static_cast<coordinate>(255 - rgb.green()), 255.0) };
            }
            break;
        default:
            return point{};
        }
    }

    void color_dialog::yz_picker::update_texture()
    {
        for (std::uint32_t y = 0; y < 256; ++y)
        {
            for (std::uint32_t z = 0; z < 256; ++z)
            {
                auto r = color_at_position(point{ static_cast<coordinate>(y), static_cast<coordinate>(255 - z) });
                color rgbColor = (std::holds_alternative<hsv_color>(r) ? static_variant_cast<const hsv_color&>(r).to_rgb<color>() : static_variant_cast<const color&>(r));
                iPixels[z][y][0] = rgbColor.red();
                iPixels[z][y][1] = rgbColor.green();
                iPixels[z][y][2] = rgbColor.blue();
                iPixels[z][y][3] = 255; // alpha
            }
        }
        iTexture.set_pixels(rect{ point{}, size{256, 256} }, &iPixels[0][0][0]);
        update();
    }

    void color_dialog::yz_picker::animate()
    {
        rect cr = client_rect();
        point center = dip(current_cursor_position());
        auto const cursorLength = dip(CURSOR_RADIUS) + dip(CURSOR_THICKNESS);
        update(rect{ center - point{ cursorLength, cursorLength }, size{ cursorLength * 2.0, cursorLength * 2.0 } });
    }

    namespace
    {
        // wheel picker geometry in normalized (0..1) wheel coordinates, y down
        constexpr scalar kWheelOuterRadius = 0.5;
        constexpr scalar kWheelInnerRadius = 0.4;
        constexpr scalar kWheelTriangleRadius = 0.38;

        scalar wheel_to_radians(scalar aDegrees)
        {
            return aDegrees * math::pi<double>() / 180.0;
        }

        point wheel_triangle_vertex(scalar aHue, scalar aOffsetDegrees)
        {
            auto const a = wheel_to_radians(aHue + aOffsetDegrees);
            return point{ 0.5 + kWheelTriangleRadius * std::cos(a), 0.5 - kWheelTriangleRadius * std::sin(a) };
        }

        scalar wheel_hue_at(const point& aWheelPosition)
        {
            auto hue = std::atan2(-(aWheelPosition.y - 0.5), aWheelPosition.x - 0.5) * 180.0 / math::pi<double>();
            if (hue < 0.0)
                hue += 360.0;
            return std::min(hue, 359.9);
        }

        // barycentric weights of aP for triangle (hue vertex, white vertex, black vertex)
        std::array<scalar, 3> wheel_barycentric(const point& aP, const point& aH, const point& aW, const point& aK)
        {
            auto const d = (aW.y - aK.y) * (aH.x - aK.x) + (aK.x - aW.x) * (aH.y - aK.y);
            auto const h = ((aW.y - aK.y) * (aP.x - aK.x) + (aK.x - aW.x) * (aP.y - aK.y)) / d;
            auto const w = ((aK.y - aH.y) * (aP.x - aK.x) + (aH.x - aK.x) * (aP.y - aK.y)) / d;
            return { h, w, 1.0 - h - w };
        }

        std::array<scalar, 3> clamp_barycentric(std::array<scalar, 3> aWeights)
        {
            for (auto& weight : aWeights)
                weight = std::max(weight, 0.0);
            auto const sum = aWeights[0] + aWeights[1] + aWeights[2];
            if (sum > 0.0)
                for (auto& weight : aWeights)
                    weight /= sum;
            return aWeights;
        }

        scalar wheel_coverage(scalar aDistance)
        {
            return std::clamp(aDistance + 0.5, 0.0, 1.0);
        }
    }

    color_dialog::wheel_picker::wheel_picker(color_dialog& aOwner) :
        base_type{ aOwner.iRightTopLayout },
        iOwner{ aOwner },
        iPixels(TEXTURE_SIZE * TEXTURE_SIZE),
        iTexture{ image{ size{ static_cast<dimension>(TEXTURE_SIZE), static_cast<dimension>(TEXTURE_SIZE) }, color::Black } },
        iDragging{ drag_e::None },
        iAnimationTimer{ *this, [this](widget_timer& aTimer)
        {
            aTimer.again();
            animate();
        }, std::chrono::milliseconds{ 10 }, true }
    {
        set_padding(neogfx::padding{});
        iSink += iOwner.SelectionChanged([this]()
        {
            update_texture();
            update();
        });
        iSink += VisibilityChanged([this]()
        {
            update_texture();
        });
        iSink += service<i_app>().current_style_changed([this](style_aspect)
        {
            update_texture(true);
        });
    }

    void color_dialog::wheel_picker::paint(i_graphics_context& aGc) const
    {
        base_type::paint(aGc);
        auto const wr = wheel_rect();
        aGc.draw_texture(wr, iTexture);
        // same animated cursor as yz_picker
        auto const radius = dip(CURSOR_RADIUS);
        auto const circumference = 2.0 * math::pi<double>() * radius;
        for (auto const& center : cursor_positions())
        {
            aGc.draw_circle(center, radius, pen{ color::White, dip(CURSOR_THICKNESS) });
            aGc.draw_circle(center, radius, pen{ color::Black, dip(CURSOR_THICKNESS),
                line_dash{ 0x5555u, circumference / 6.0, circumference * neolib::this_process::elapsed_ms() / 1000.0 } });
        }
    }

    std::array<point, 2> color_dialog::wheel_picker::cursor_positions() const
    {
        auto const hsv = iOwner.selected_color_as_hsv(false);
        // hue cursor on ring
        auto const ringMiddle = (kWheelOuterRadius + kWheelInnerRadius) / 2.0;
        auto const hueAngle = wheel_to_radians(hsv.hue());
        auto const hueCursor = from_wheel(point{ 0.5 + ringMiddle * std::cos(hueAngle), 0.5 - ringMiddle * std::sin(hueAngle) });
        // saturation/value cursor in triangle
        auto const h = wheel_triangle_vertex(hsv.hue(), 0.0);
        auto const w = wheel_triangle_vertex(hsv.hue(), 120.0);
        auto const k = wheel_triangle_vertex(hsv.hue(), 240.0);
        auto const hWeight = hsv.saturation() * hsv.value();
        auto const wWeight = (1.0 - hsv.saturation()) * hsv.value();
        auto const kWeight = 1.0 - hsv.value();
        auto const svCursor = from_wheel(point{
            h.x * hWeight + w.x * wWeight + k.x * kWeight,
            h.y * hWeight + w.y * wWeight + k.y * kWeight });
        return { hueCursor, svCursor };
    }

    void color_dialog::wheel_picker::animate()
    {
        if (!visible())
            return;
        auto const cursorLength = dip(CURSOR_RADIUS) + dip(CURSOR_THICKNESS);
        for (auto const& center : cursor_positions())
            update(rect{ center - point{ cursorLength, cursorLength }, size{ cursorLength * 2.0, cursorLength * 2.0 } });
    }

    void color_dialog::wheel_picker::mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier)
    {
        base_type::mouse_button_clicked(aButton, aPosition, aKeyModifier);
        if (aButton != mouse_button::Left)
            return;
        auto const wheelPosition = to_wheel(aPosition);
        auto const distance = std::hypot(wheelPosition.x - 0.5, wheelPosition.y - 0.5);
        if (distance >= kWheelInnerRadius && distance <= kWheelOuterRadius)
            iDragging = drag_e::Ring;
        else if (distance < kWheelInnerRadius)
            iDragging = drag_e::Triangle;
        else
            return;
        select(aPosition);
    }

    void color_dialog::wheel_picker::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        base_type::mouse_button_released(aButton, aPosition);
        if (!capturing())
            iDragging = drag_e::None;
    }

    void color_dialog::wheel_picker::mouse_moved(const point& aPosition, key_modifier aKeyModifier)
    {
        base_type::mouse_moved(aPosition, aKeyModifier);
        if (iDragging != drag_e::None)
            select(aPosition);
    }

    neogfx::mouse_cursor color_dialog::wheel_picker::mouse_cursor() const
    {
        auto const wheelPosition = to_wheel(mouse_position());
        if (std::hypot(wheelPosition.x - 0.5, wheelPosition.y - 0.5) <= kWheelOuterRadius)
            return mouse_system_cursor::Crosshair;
        return base_type::mouse_cursor();
    }

    rect color_dialog::wheel_picker::wheel_rect() const
    {
        auto const cr = client_rect(false);
        auto const side = std::min(cr.cx, cr.cy);
        return rect{ point{ cr.x + (cr.cx - side) / 2.0, cr.y + (cr.cy - side) / 2.0 }, size{ side, side } };
    }

    point color_dialog::wheel_picker::to_wheel(const point& aPosition) const
    {
        auto const wr = wheel_rect();
        if (wr.cx <= 0.0)
            return point{};
        return point{ (aPosition.x - wr.x) / wr.cx, (aPosition.y - wr.y) / wr.cy };
    }

    point color_dialog::wheel_picker::from_wheel(const point& aWheelPosition) const
    {
        auto const wr = wheel_rect();
        return point{ wr.x + aWheelPosition.x * wr.cx, wr.y + aWheelPosition.y * wr.cy };
    }

    void color_dialog::wheel_picker::select(const point& aPosition)
    {
        auto const wheelPosition = to_wheel(aPosition);
        auto hsv = iOwner.selected_color_as_hsv(true);
        if (iDragging == drag_e::Ring)
            hsv.set_hue(wheel_hue_at(wheelPosition));
        else if (iDragging == drag_e::Triangle)
        {
            auto const weights = clamp_barycentric(wheel_barycentric(wheelPosition,
                wheel_triangle_vertex(hsv.hue(), 0.0), wheel_triangle_vertex(hsv.hue(), 120.0), wheel_triangle_vertex(hsv.hue(), 240.0)));
            auto const value = std::clamp(weights[0] + weights[1], 0.0, 1.0);
            if (value > 0.0)
                hsv.set_saturation(std::clamp(weights[0] / value, 0.0, 1.0));
            hsv.set_value(value);
        }
        iOwner.select_color(hsv, *this);
    }

    void color_dialog::wheel_picker::update_texture(bool aForce)
    {
        if (!visible())
            return;
        auto const hue = iOwner.selected_color_as_hsv(false).hue();
        auto background = container_background_color();
        background.set_alpha(255);
        if (!aForce && iTextureState != std::nullopt && iTextureState->first == hue && iTextureState->second == background)
            return;
        iTextureState.emplace(hue, background);
        auto const h = wheel_triangle_vertex(hue, 0.0);
        auto const w = wheel_triangle_vertex(hue, 120.0);
        auto const k = wheel_triangle_vertex(hue, 240.0);
        scalar const pixelsPerUnit = static_cast<scalar>(TEXTURE_SIZE);
        scalar const triangleEdgeScale = 1.5 * kWheelTriangleRadius * pixelsPerUnit; // barycentric weight to edge distance (pixels)
        for (std::uint32_t y = 0; y < TEXTURE_SIZE; ++y)
        {
            for (std::uint32_t x = 0; x < TEXTURE_SIZE; ++x)
            {
                point const p{ (x + 0.5) / pixelsPerUnit, (y + 0.5) / pixelsPerUnit };
                auto const distance = std::hypot(p.x - 0.5, p.y - 0.5);
                color result = background;
                auto const ringCoverage = wheel_coverage((kWheelOuterRadius - distance) * pixelsPerUnit) * wheel_coverage((distance - kWheelInnerRadius) * pixelsPerUnit);
                if (ringCoverage > 0.0)
                    result = mix(background, hsv_color{ wheel_hue_at(p), 1.0, 1.0 }.to_rgb<color>(), ringCoverage);
                else if (distance < kWheelInnerRadius)
                {
                    auto const weights = wheel_barycentric(p, h, w, k);
                    auto const triangleCoverage = wheel_coverage(std::min({ weights[0], weights[1], weights[2] }) * triangleEdgeScale);
                    if (triangleCoverage > 0.0)
                    {
                        auto const clamped = clamp_barycentric(weights);
                        auto const value = clamped[0] + clamped[1];
                        auto const saturation = (value > 0.0 ? clamped[0] / value : 0.0);
                        result = mix(background, hsv_color{ hue, saturation, value }.to_rgb<color>(), triangleCoverage);
                    }
                }
                auto& pixel = iPixels[(TEXTURE_SIZE - 1 - y) * TEXTURE_SIZE + x]; // texture rows are bottom-up
                pixel[0] = result.red();
                pixel[1] = result.green();
                pixel[2] = result.blue();
                pixel[3] = 0xFF;
            }
        }
        iTexture.set_pixels(rect{ point{}, size{ static_cast<dimension>(TEXTURE_SIZE), static_cast<dimension>(TEXTURE_SIZE) } }, &iPixels[0][0]);
        update();
    }

    color_dialog::color_selection::color_selection(color_dialog& aOwner) :
        base_type{ aOwner.iRightBottomLayout }, iOwner(aOwner)
    {
        set_padding(neogfx::padding{});
        iOwner.SelectionChanged([this]
        {
            update();
        });
    }

    size color_dialog::color_selection::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (has_minimum_size())
            return base_type::minimum_size(aAvailableSpace);
        return base_type::minimum_size(aAvailableSpace) + size{ 60_dip, 80_dip };
    }

    size color_dialog::color_selection::maximum_size(optional_size const& aAvailableSpace) const
    {
        if (has_maximum_size())
            return base_type::maximum_size(aAvailableSpace);
        return minimum_size();
    }

    void color_dialog::color_selection::paint(i_graphics_context& aGc) const
    {
        base_type::paint(aGc);
        scoped_units su{ *this, units::Pixels };
        rect cr = client_rect(false);
        draw_alpha_background(aGc, cr);
        rect top = cr;
        rect bottom = top;
        top.cy = top.cy / 2.0;
        bottom.y = top.bottom();
        bottom.cy = bottom.cy / 2.0;
        aGc.fill_rect(top, iOwner.selected_color());
        aGc.fill_rect(bottom, iOwner.current_color());
    }

    color_dialog::color_dialog(const color& aCurrentColor) :
        dialog{ "Select Color"_t, window_style::Dialog | window_style::Modal | window_style::TitleBar | window_style::Close },
        iCurrentChannel{ ChannelHue },
        iCurrentColor{ aCurrentColor },
        iSelectedColor{ aCurrentColor.to_hsv() },
        iCustomColors{ previous_custom_colors() },
        iCurrentCustomColor{ iCustomColors.end() },
        iUpdatingWidgets{ false },
        iScreenPickerActive{ false },
        iLayout{ client_layout() },
        iLayout2{ iLayout },
        iLeftLayout{ iLayout2 },
        iRightLayout{ iLayout2 },
        iRightTopLayout{ iRightLayout },
        iRightBottomLayout{ iRightLayout, alignment::Left | alignment::Top },
        iColorSelection{ *this },
        iScreenPicker{ iRightBottomLayout },
        iPickerMode{ iRightBottomLayout },
        iSpacer0{ iRightBottomLayout },
        iChannelLayout{ iRightBottomLayout, alignment::Left | alignment::VCenter },
        iSwatchesGroup{ iLeftLayout, "S&watches"_t },
        iSwatchToolbar{ iSwatchesGroup.item_layout() },
        iSwatchSelector{ iSwatchToolbar },
        iNewSwatch{ iSwatchToolbar, "&New..."_t },
        iImportSwatch{ iSwatchToolbar, "&Import..."_t },
        iDeleteSwatch{ iSwatchToolbar, "&Delete"_t },
        iSwatchGrid{ iSwatchesGroup.item_layout() },
        iSwatchEditLayout{ iSwatchesGroup.item_layout() },
        iAddToSwatch{ iSwatchEditLayout, "Add to Swa&tch"_t },
        iRemoveFromSwatch{ iSwatchEditLayout, "Re&move from Swatch"_t },
        iSpacer{ iLeftLayout },
        iCustomColorsGroup{ iLeftLayout, "&Custom colors"_t },
        iCustomColorsGrid{ iCustomColorsGroup.item_layout() },
        iSpacer2{ iRightTopLayout },
        iYZPicker{ *this },
        iXPicker{ *this },
        iWheelPicker{ *this },
        iModelLayout{ client_layout() },
        iSpacer3{ iModelLayout },
        iColorSpaceSelector{ iModelLayout },
        iH{ client_widget(), client_widget() },
        iS{ client_widget(), client_widget() },
        iV{ client_widget(), client_widget() },
        iR{ client_widget(), client_widget() },
        iG{ client_widget(), client_widget() },
        iB{ client_widget(), client_widget() },
        iA{ client_widget(), client_widget() },
        iRgb{ client_widget() },
        iAddToCustomColors{ iRightLayout, "&Add to Custom Colors"_t }
    {
        init();
    }

    color_dialog::color_dialog(i_widget& aParent, const color& aCurrentColor) :
        dialog{ aParent, "Select Color"_t, window_style::Dialog | window_style::Modal | window_style::TitleBar | window_style::Close },
        iCurrentChannel{ChannelHue },
        iCurrentColor{aCurrentColor },
        iSelectedColor{aCurrentColor.to_hsv() },
        iCustomColors{ previous_custom_colors() },
        iCurrentCustomColor{ iCustomColors.end() },
        iUpdatingWidgets{ false },
        iScreenPickerActive{ false },
        iLayout{ client_layout() },
        iLayout2{ iLayout },
        iLeftLayout{ iLayout2 },
        iRightLayout{ iLayout2 },
        iRightTopLayout{ iRightLayout },
        iRightBottomLayout{ iRightLayout, alignment::Left | alignment::Top },
        iColorSelection{ *this },
        iScreenPicker{ iRightBottomLayout },
        iPickerMode{ iRightBottomLayout },
        iSpacer0{ iRightBottomLayout },
        iChannelLayout{ iRightBottomLayout, alignment::Left | alignment::VCenter },
        iSwatchesGroup{ iLeftLayout, "S&watches"_t },
        iSwatchToolbar{ iSwatchesGroup.item_layout() },
        iSwatchSelector{ iSwatchToolbar },
        iNewSwatch{ iSwatchToolbar, "&New..."_t },
        iImportSwatch{ iSwatchToolbar, "&Import..."_t },
        iDeleteSwatch{ iSwatchToolbar, "&Delete"_t },
        iSwatchGrid{ iSwatchesGroup.item_layout() },
        iSwatchEditLayout{ iSwatchesGroup.item_layout() },
        iAddToSwatch{ iSwatchEditLayout, "Add to Swa&tch"_t },
        iRemoveFromSwatch{ iSwatchEditLayout, "Re&move from Swatch"_t },
        iSpacer{ iLeftLayout },
        iCustomColorsGroup{ iLeftLayout, "&Custom colors"_t },
        iCustomColorsGrid{ iCustomColorsGroup.item_layout() },
        iSpacer2{ iRightTopLayout },
        iYZPicker{ *this },
        iXPicker{ *this },
        iWheelPicker{ *this },
        iModelLayout{ client_layout() },
        iSpacer3{ iModelLayout },
        iColorSpaceSelector{ iModelLayout },
        iH{ client_widget(), client_widget() },
        iS{ client_widget(), client_widget() },
        iV{ client_widget(), client_widget() },
        iR{ client_widget(), client_widget() },
        iG{ client_widget(), client_widget() },
        iB{ client_widget(), client_widget() },
        iA{ client_widget(), client_widget() },
        iRgb{ client_widget() },
        iAddToCustomColors{ iRightLayout, "&Add to Custom Colors"_t }
    {
        init();
    }

    color_dialog::~color_dialog()
    {
        previous_custom_colors() = iCustomColors;
    }

    color color_dialog::current_color() const
    {
        return iCurrentColor;
    }

    color color_dialog::selected_color() const
    {
        if (std::holds_alternative<color>(iSelectedColor))
            return static_variant_cast<const color&>(iSelectedColor);
        else
            return static_variant_cast<const hsv_color&>(iSelectedColor).to_rgb<color>();
    }

    vec4 color_dialog::selected_color_in_color_space() const
    {
        auto const selectedColor = selected_color();
        switch (*iColorSpace)
        {
        case color_space::LinearRGB:
            return selectedColor.to_linear();
        case color_space::sRGB:
            return selectedColor;
        }
        return {};
    }

    hsv_color color_dialog::selected_color_as_hsv() const
    {
        return selected_color_as_hsv(true);
    }

    void color_dialog::select_color(const color& aColor)
    {
        select_color(aColor, *this);
    }

    const color_dialog::custom_color_list& color_dialog::custom_colors() const
    {
        return iCustomColors;
    }

    void color_dialog::set_custom_colors(const custom_color_list& aCustomColors)
    {
        iCustomColors = aCustomColors;
        iCurrentCustomColor = std::find_if(iCustomColors.begin(), iCustomColors.end(), [](const optional_color& aColor) { return aColor == std::nullopt; });
        if (iCurrentCustomColor == iCustomColors.end())
            iCurrentCustomColor = iCustomColors.begin();
    }

    const color_dialog::swatch_list& color_dialog::swatches() const
    {
        return iSwatches;
    }

    void color_dialog::mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier)
    {
        dialog::mouse_button_clicked(aButton, aPosition, aKeyModifier);
    }

    void color_dialog::init()
    {
        thread_local basic_item_model<color_space> model;
        if (model.empty())
        {
            model.insert_item(model.end(), color_space::LinearRGB, "Linear RGB");
            model.insert_item(model.end(), color_space::sRGB, "sRGB");
        }
        change_color_space(color_space::sRGB);
        iColorSpaceSelector.set_model(model);
        iColorSpaceSelector.set_presentation_model(make_ref<basic_item_presentation_model<decltype(model)>>(model));
        iColorSpaceSelector.selection_model().set_current_index(static_cast<std::underlying_type_t<color_space>>(color_space::sRGB));
        iColorSpaceSelector.accept_selection();
        iSink += iColorSpaceSelector.selection_model().current_index_changed([&](const optional_item_presentation_model_index& aCurrentIndex, const optional_item_presentation_model_index& aPreviousIndex)
        {
            change_color_space(static_cast<color_space>(aCurrentIndex->row()));
        });

        scoped_units su{ *this, units::Pixels };
        auto const standardSpacing = client_layout().spacing();
        iLayout.set_padding(neogfx::padding{});
        iLayout.set_spacing(standardSpacing);
        iLayout2.set_padding(neogfx::padding{});
        iLayout2.set_spacing(standardSpacing);
        iRightLayout.set_spacing(standardSpacing);
        iRightTopLayout.set_spacing(standardSpacing * 2.0);
        auto adjustedPadding = iRightTopLayout.internal_spacing();
        adjustedPadding.right = std::max(adjustedPadding.right, iXPicker.cursor_width());
        iRightTopLayout.set_padding(adjustedPadding);
        iRightBottomLayout.set_spacing(standardSpacing / 2.0);
        iChannelLayout.set_padding(neogfx::padding{});
        iChannelLayout.set_spacing(standardSpacing / 2.0);
        iScreenPicker.set_checkable();
        iScreenPicker.set_size_policy(size_constraint::Minimum);
        iScreenPicker.set_image(image{ ":/neogfx/resources/icons/eyedropper.png" });
        iScreenPicker.set_image_extents(size{ 16_dip });
        iScreenPicker.enable(service<i_clipboard>().has_image());
        iSink += service<i_clipboard>().updated([this]()
        {
            iScreenPicker.enable(service<i_clipboard>().has_image());
        });
        iSink += iScreenPicker.Checked([&, this]()
        {
            iScreenPicker.set_image(image{ ":/neogfx/resources/icons/colour.png" });
            iYZPicker.set_image(service<i_clipboard>().image());
            update_picker_mode();
        });
        iSink += iScreenPicker.Unchecked([&, this]()
        {
            iScreenPicker.set_image(image{ ":/neogfx/resources/icons/eyedropper.png" });
            iYZPicker.clear_image();
            update_picker_mode();
        });
        iPickerMode.set_checkable();
        iPickerMode.set_size_policy(size_constraint::Minimum);
        iPickerMode.set_image_extents(size{ 16_dip });
        iSink += iPickerMode.Checked([this]() { update_picker_mode(); });
        iSink += iPickerMode.Unchecked([this]() { update_picker_mode(); });
        iSink += service<i_app>().current_style_changed([this](style_aspect) { update_picker_mode(); });
        // wheel occupies the same area as the X/Y-Z pickers so the dialog doesn't resize when toggling modes
        // (X picker is taller than Y-Z picker as it has padding)
        iWheelPicker.set_fixed_size(size{ 
            iYZPicker.minimum_size().cx + iRightTopLayout.spacing().cx + iXPicker.minimum_size().cx, 
            std::max(iYZPicker.minimum_size().cy, iXPicker.minimum_size().cy) });
        update_picker_mode();
        iH.first.set_size_policy(size_constraint::Minimum); iH.first.label().set_text("&Hue:"_t); iH.second.set_size_policy(size_constraint::Minimum); iH.second.set_text_box_size_hint(size_hint{ "999.9" }); iH.second.set_minimum(0.0); iH.second.set_maximum(359.9); iH.second.set_step(1);
        iS.first.set_size_policy(size_constraint::Minimum); iS.first.label().set_text("&Sat:"_t); iS.second.set_size_policy(size_constraint::Minimum); iS.second.set_text_box_size_hint(size_hint{ "999.9" }); iS.second.set_minimum(0.0); iS.second.set_maximum(100.0); iS.second.set_step(1);
        iV.first.set_size_policy(size_constraint::Minimum); iV.first.label().set_text("&Val:"_t); iV.second.set_size_policy(size_constraint::Minimum); iV.second.set_text_box_size_hint(size_hint{ "999.9" }); iV.second.set_minimum(0.0); iV.second.set_maximum(100.0); iV.second.set_step(1);
        iR.first.set_size_policy(size_constraint::Minimum); iR.first.label().set_text("&Red:"_t); iR.second.set_size_policy(size_constraint::Minimum); iR.second.set_text_box_size_hint(size_hint{ "8.888" });
        iG.first.set_size_policy(size_constraint::Minimum); iG.first.label().set_text("&Green:"_t); iG.second.set_size_policy(size_constraint::Minimum); iG.second.set_text_box_size_hint(size_hint{ "8.888" });
        iB.first.set_size_policy(size_constraint::Minimum); iB.first.label().set_text("&Blue:"_t); iB.second.set_size_policy(size_constraint::Minimum); iB.second.set_text_box_size_hint(size_hint{ "8.888" });
        iA.first.set_size_policy(size_constraint::Minimum); iA.first.label().set_text("&Alpha:"_t); iA.second.set_size_policy(size_constraint::Minimum); iA.second.set_text_box_size_hint(size_hint{ "8.888" });
        iChannelLayout.set_dimensions(5, 4);
        iChannelLayout.add_span(grid_layout::cell_coordinates{ 0, 0 }, grid_layout::cell_coordinates{ 3, 0 });
        iChannelLayout.add(iModelLayout);
        iChannelLayout.add(iH.first); iChannelLayout.add(iH.second);
        iChannelLayout.add(iR.first); iChannelLayout.add(iR.second);
        iChannelLayout.add(iS.first); iChannelLayout.add(iS.second);
        iChannelLayout.add(iG.first); iChannelLayout.add(iG.second);
        iChannelLayout.add(iV.first); iChannelLayout.add(iV.second);
        iChannelLayout.add(iB.first); iChannelLayout.add(iB.second);
        iChannelLayout.add_span(grid_layout::cell_coordinates{ 0, 4 }, grid_layout::cell_coordinates{ 1, 4 });
        iChannelLayout.add(iRgb);
        iChannelLayout.add(iA.first); iChannelLayout.add(iA.second);
        iSwatchGrid.set_dimensions(12, 12);
        for (std::size_t swatchSlot = 0; swatchSlot < MaxSwatchColors; ++swatchSlot)
            iSwatchGrid.add(make_ref<color_box>(*this, optional_color{}, optional_custom_color_list_iterator{}, swatchSlot));
        iSwatchSelector.set_presentation_model(make_ref<swatch_presentation_model>(iSwatchSelector, *this));
        iSwatchSelector.set_input_image_size(size{ 12.0_dip + 1.0_dip }); // half of drop list view icon (24dip grid + 1dip border each side): 12dip grid + border
        init_swatches();
        iCustomColorsGrid.set_dimensions(2, 12);
        for (auto customColor = iCustomColors.begin(); customColor != iCustomColors.end(); ++customColor)
            iCustomColorsGrid.add(make_ref<color_box>(*this, *customColor, customColor));
        button_box().add_button(standard_button::Ok);
        button_box().add_button(standard_button::Cancel);
        iSink += iH.first.checked([this]() { set_current_channel(ChannelHue); });
        iSink += iS.first.checked([this]() { set_current_channel(ChannelSaturation); });
        iSink += iV.first.checked([this]() { set_current_channel(ChannelValue); });
        iSink += iR.first.checked([this]() { set_current_channel(ChannelRed); });
        iSink += iG.first.checked([this]() { set_current_channel(ChannelGreen); });
        iSink += iB.first.checked([this]() { set_current_channel(ChannelBlue); });
        iSink += iA.first.checked([this]() { set_current_channel(ChannelAlpha); });
        iSink += iH.second.ValueChanged([this]() { if (iUpdatingWidgets) return; auto hsv = selected_color_as_hsv(); hsv.set_hue(iH.second.value()); select_color(hsv, iH.second); });
        iSink += iS.second.ValueChanged([this]() { if (iUpdatingWidgets) return; auto hsv = selected_color_as_hsv(); hsv.set_saturation(iS.second.value() / 100.0); select_color(hsv, iS.second); });
        iSink += iV.second.ValueChanged([this]() { if (iUpdatingWidgets) return; auto hsv = selected_color_as_hsv(); hsv.set_value(iV.second.value() / 100.0); select_color(hsv, iV.second); });
        iSink += iR.second.ValueChanged([this]() { if (iUpdatingWidgets) return; auto rgb = selected_color_in_color_space(); rgb[0] = iR.second.value() / color_space_coefficient(); select_color_in_color_space(rgb, iR.second); });
        iSink += iG.second.ValueChanged([this]() { if (iUpdatingWidgets) return; auto rgb = selected_color_in_color_space(); rgb[1] = iG.second.value() / color_space_coefficient(); select_color_in_color_space(rgb, iG.second); });
        iSink += iB.second.ValueChanged([this]() { if (iUpdatingWidgets) return; auto rgb = selected_color_in_color_space(); rgb[2] = iB.second.value() / color_space_coefficient(); select_color_in_color_space(rgb, iB.second); });
        iSink += iA.second.ValueChanged([this]()
        { 
            if (iUpdatingWidgets) 
                return;
            if (std::holds_alternative<color>(iSelectedColor))
            {
                auto rgb = selected_color_in_color_space(); 
                rgb[3] = iA.second.value() / color_space_coefficient();
                select_color_in_color_space(rgb, iA.second);
            }            
            else
            {
                auto rgb = selected_color_in_color_space();
                rgb[3] = iA.second.value() / color_space_coefficient();
                select_color_in_color_space(rgb, iA.second);
                auto hsv = selected_color_as_hsv();
                select_color(hsv, iA.second);
            }
        });
        iSink += iRgb.TextChanged([this]() { if (iUpdatingWidgets) return; select_color(color{ iRgb.text() }, iRgb); });

        iSink += iSwatchSelector.SelectionChanged([this](const optional_item_model_index& aIndex)
        {
            if (aIndex != std::nullopt && aIndex->row() < iSwatches.size())
                set_current_swatch(aIndex->row());
        });
        iSink += iNewSwatch.clicked([this]()
        {
            auto const newPath = save_file_dialog(*this, file_dialog_spec{ "New Swatch", swatch_folder() + "/New Swatch.csw", { "*.csw" }, "Color Swatch Files" });
            if (newPath == std::nullopt)
                return;
            std::filesystem::path path{ *newPath };
            if (path.extension() != ".csw")
                path += ".csw";
            swatch newSwatch{ path.stem().string(), {}, false, path.string() };
            if (!save_swatch(newSwatch))
            {
                message_box::error(*this, "New Swatch", "Failed to save swatch");
                return;
            }
            add_swatch(std::move(newSwatch));
        });
        iSink += iImportSwatch.clicked([this]()
        {
            auto const imports = open_file_dialog(*this, file_dialog_spec{ "Import Swatches", swatch_folder() + "/", { "*.csw" }, "Color Swatch Files" }, true);
            if (imports == std::nullopt)
                return;
            bool failed = false;
            for (auto const& path : *imports)
            {
                swatch importedSwatch;
                if (load_swatch(path, importedSwatch))
                    add_swatch(std::move(importedSwatch));
                else
                    failed = true;
            }
            if (failed)
                message_box::error(*this, "Import Swatches", "Failed to import swatch(es)");
        });
        iSink += iDeleteSwatch.clicked([this]()
        {
            if (!current_swatch_editable())
                return;
            auto const& existing = iSwatches[iCurrentSwatch];
            if (message_box::question(*this, "Delete Swatch", string{ "Delete swatch '" + existing.name + "'?" }) != standard_button::Yes)
                return;
            std::error_code ec;
            std::filesystem::remove(std::filesystem::path{ existing.path }, ec);
            iSwatches.erase(std::next(iSwatches.begin(), iCurrentSwatch));
            if (iCurrentSwatch > 0)
                --iCurrentSwatch;
            iCurrentSwatchColor = std::nullopt;
            previous_swatch() = iSwatches[iCurrentSwatch].name;
            update_swatch_selector();
            update_swatch_buttons();
            update();
        });
        iSink += iAddToSwatch.clicked([this]()
        {
            if (!current_swatch_editable())
                return;
            auto& existing = iSwatches[iCurrentSwatch];
            if (existing.colors.size() >= MaxSwatchColors)
                return;
            existing.colors.push_back(selected_color());
            iCurrentSwatchColor = existing.colors.size() - 1;
            if (!save_swatch(existing))
                message_box::error(*this, "Add to Swatch", "Failed to save swatch");
            update_swatch_selector();
            update_swatch_buttons();
            update();
        });
        iSink += iRemoveFromSwatch.clicked([this]()
        {
            if (!current_swatch_editable() || iCurrentSwatchColor == std::nullopt)
                return;
            auto& existing = iSwatches[iCurrentSwatch];
            if (*iCurrentSwatchColor >= existing.colors.size())
                return;
            existing.colors.erase(std::next(existing.colors.begin(), *iCurrentSwatchColor));
            if (existing.colors.empty())
                iCurrentSwatchColor = std::nullopt;
            else if (*iCurrentSwatchColor >= existing.colors.size())
                iCurrentSwatchColor = existing.colors.size() - 1;
            if (!save_swatch(existing))
                message_box::error(*this, "Remove from Swatch", "Failed to save swatch");
            update_swatch_selector();
            update_swatch_buttons();
            update();
        });

        iSink += iAddToCustomColors.clicked([this]()
        {
            if (iCurrentCustomColor == iCustomColors.end())
                iCurrentCustomColor = iCustomColors.begin();
            *iCurrentCustomColor = selected_color();
            if (iCurrentCustomColor != iCustomColors.end())
                ++iCurrentCustomColor;
            update();
        });

        update_widgets(*this);

        update_layout();
        center_on_parent(true);
        set_ready_to_render(true);
    }

    color_dialog::custom_color_list& color_dialog::previous_custom_colors()
    {
        static custom_color_list sCustomColors;
        return sCustomColors;
    }

    scalar color_dialog::color_space_coefficient() const
    {
        switch (*iColorSpace)
        {
        case color_space::LinearRGB:
            return 1.0;
        case color_space::sRGB:
            return 255.0;
        default:
            return 1.0;
        }
    }

    void color_dialog::change_color_space(color_space aColorSpace)
    {
        if (iColorSpace != aColorSpace)
        {
            iColorSpace = aColorSpace;
            ColorSpaceChanged();
            {
                neolib::scoped_flag sf{ iUpdatingWidgets };
                switch (*iColorSpace)
                {
                case color_space::LinearRGB:
                    iR.second.set_format("{:.3f}"); iR.second.set_minimum(0.0); iR.second.set_maximum(1.0); iR.second.set_step(0.001);
                    iG.second.set_format("{:.3f}"); iG.second.set_minimum(0.0); iG.second.set_maximum(1.0); iG.second.set_step(0.001);
                    iB.second.set_format("{:.3f}"); iB.second.set_minimum(0.0); iB.second.set_maximum(1.0); iB.second.set_step(0.001);
                    iA.second.set_format("{:.3f}"); iA.second.set_minimum(0.0); iA.second.set_maximum(1.0); iA.second.set_step(0.001);
                    iRgb.hide();
                    break;
                case color_space::sRGB:
                    iR.second.set_format("{:.0f}"); iR.second.set_minimum(0); iR.second.set_maximum(255); iR.second.set_step(1);
                    iG.second.set_format("{:.0f}"); iG.second.set_minimum(0); iG.second.set_maximum(255); iG.second.set_step(1);
                    iB.second.set_format("{:.0f}"); iB.second.set_minimum(0); iB.second.set_maximum(255); iB.second.set_step(1);
                    iA.second.set_format("{:.0f}"); iA.second.set_minimum(0); iA.second.set_maximum(255); iA.second.set_step(1);
                    iRgb.show();
                    break;
                }
            }
            update_widgets(*this);
        }
    }

    color_dialog::mode_e color_dialog::current_mode() const
    {
        if (std::holds_alternative<hsv_color>(iSelectedColor))
            return ModeHSV;
        else
            return ModeRGB;
    }

    color_dialog::channel_e color_dialog::current_channel() const
    {
        return iCurrentChannel;
    }

    void color_dialog::set_current_channel(channel_e aChannel)
    {
        if (iCurrentChannel != aChannel)
        {
            iCurrentChannel = aChannel;
            SelectionChanged();
            update();
        }
    }

    hsv_color color_dialog::selected_color_as_hsv(bool aChangeRepresentation) const
    {
        if (std::holds_alternative<color>(iSelectedColor))
        {
            hsv_color result = static_variant_cast<const color&>(iSelectedColor).to_hsv();
            if (aChangeRepresentation)
                iSelectedColor = result;
            return result;
        }
        else
            return static_variant_cast<const hsv_color&>(iSelectedColor);
    }

    void color_dialog::select_color(const representations& aColor, const i_widget& aUpdatingWidget)
    {
        if (iUpdatingWidgets)
            return;
        if (iSelectedColor != aColor)
        {
            iSelectedColor = aColor;
            update_widgets(aUpdatingWidget);
            SelectionChanged();
        }
    }

    void color_dialog::select_color_in_color_space(const vec4& aColor, const i_widget& aUpdatingWidget)
    {
        if (iUpdatingWidgets)
            return;
        color newColor;
        switch (*iColorSpace)
        {
        case color_space::LinearRGB:
            newColor = color::from_linear(linear_color{ aColor });
            break;
        case color_space::sRGB:
            newColor = color{ aColor };
            break;
        }
        select_color(newColor, aUpdatingWidget);
    }

    color_dialog::custom_color_list::iterator color_dialog::current_custom_color() const
    {
        return iCurrentCustomColor;
    }

    void color_dialog::set_current_custom_color(custom_color_list::iterator aCustomColor)
    {
        if (iCurrentCustomColor == aCustomColor)
            return;
        iCurrentCustomColor = aCustomColor;
        update_widgets(*this);
        update();
    }

    std::string& color_dialog::previous_swatch()
    {
        static std::string sPreviousSwatch;
        return sPreviousSwatch;
    }

    std::string color_dialog::swatch_folder()
    {
        return service<i_app>().info().settings_folder().to_std_string();
    }

    bool color_dialog::load_swatch(std::string const& aPath, swatch& aSwatch)
    {
        std::ifstream input{ std::filesystem::path{ aPath } };
        if (!input)
            return false;
        aSwatch = swatch{ std::filesystem::path{ aPath }.stem().string(), {}, false, aPath };
        std::string line;
        while (std::getline(input, line))
        {
            boost::algorithm::trim(line);
            if (line.empty() || line[0] == ';')
                continue;
            if (line.rfind("name=", 0) == 0)
            {
                auto name = line.substr(5);
                boost::algorithm::trim(name);
                if (!name.empty())
                    aSwatch.name = name;
                continue;
            }
            if (aSwatch.colors.size() >= MaxSwatchColors)
                break;
            try
            {
                aSwatch.colors.push_back(color{ line });
            }
            catch (...)
            {
                // skip unrecognized entry
            }
        }
        return true;
    }

    bool color_dialog::save_swatch(swatch const& aSwatch)
    {
        if (aSwatch.predefined || aSwatch.path.empty())
            return false;
        std::ofstream output{ std::filesystem::path{ aSwatch.path }, std::ios::trunc };
        if (!output)
            return false;
        output << "; neoGFX color swatch" << std::endl;
        output << "name=" << aSwatch.name << std::endl;
        for (auto const& swatchColor : aSwatch.colors)
            output << swatchColor.to_hex_string() << std::endl;
        return static_cast<bool>(output);
    }

    void color_dialog::init_swatches()
    {
        static const std::set<color> sBasicColors
        {
            color::AliceBlue, color::AntiqueWhite, color::Aquamarine, color::Azure, color::Beige, color::Bisque, color::Black, color::BlanchedAlmond, 
            color::Blue, color::BlueViolet, color::Brown, color::Burlywood, color::CadetBlue, color::Chartreuse, color::Chocolate, color::Coral, 
            color::CornflowerBlue, color::Cornsilk, color::Cyan, color::DarkBlue, color::DarkCyan, color::DarkGoldenrod, color::DarkGray, color::DarkGreen, 
            color::DarkKhaki, color::DarkMagenta, color::DarkOliveGreen, color::DarkOrange, color::DarkOrchid, color::DarkRed, color::DarkSalmon, 
            color::DarkSeaGreen, color::DarkSlateBlue, color::DarkSlateGray, color::DarkTurquoise, color::DarkViolet, color::DebianRed, color::DeepPink, 
            color::DeepSkyBlue, color::DimGray, color::DodgerBlue, color::Firebrick, color::FloralWhite, color::ForestGreen, color::Gainsboro, 
            color::GhostWhite, color::Gold, color::Goldenrod, color::Gray, color::Green, color::GreenYellow, color::Honeydew, color::HotPink, 
            color::IndianRed, color::Ivory, color::Khaki, color::Lavender, color::LavenderBlush, color::LawnGreen, color::LemonChiffon, color::LightBlue, 
            color::LightCoral, color::LightCyan, color::LightGoldenrod, color::LightGoldenrodYellow, color::LightGray, color::LightGreen, color::LightPink, 
            color::LightSalmon, color::LightSeaGreen, color::LightSkyBlue, color::LightSlateBlue, color::LightSlateGray, color::LightSteelBlue, 
            color::LightYellow, color::LimeGreen, color::Linen, color::Magenta, color::Maroon, color::MediumAquamarine, color::MediumBlue, color::MediumOrchid, 
            color::MediumPurple, color::MediumSeaGreen, color::MediumSlateBlue, color::MediumSpringGreen, color::MediumTurquoise, color::MediumVioletRed,
            color::MidnightBlue, color::MintCream, color::MistyRose, color::Moccasin, color::NavajoWhite, color::Navy, color::NavyBlue, color::OldLace, 
            color::OliveDrab, color::Orange, color::OrangeRed, color::Orchid, color::PaleGoldenrod, color::PaleGreen, color::PaleTurquoise, color::PaleVioletRed, 
            color::PapayaWhip, color::PeachPuff, color::Peru, color::Pink, color::Plum, color::PowderBlue, color::Purple, color::Red, color::RosyBrown, 
            color::RoyalBlue, color::SaddleBrown, color::Salmon, color::SandyBrown, color::SeaGreen, color::Seashell, color::Sienna, color::SkyBlue, 
            color::SlateBlue, color::SlateGray, color::Snow, color::SpringGreen, color::SteelBlue, color::Tan, color::Thistle, color::Tomato, 
            color::Turquoise, color::Violet, color::VioletRed, color::Wheat, color::White, color::WhiteSmoke, color::Yellow, color::YellowGreen 
        };
        iSwatches.clear();

        iSwatches.push_back(swatch{ "X11", std::vector<color>(sBasicColors.begin(), sBasicColors.end()), true });

        // CSS named colors (duplicate values such as aqua/cyan and gray/grey omitted)
        static const std::set<color> sHtmlColors
        {
            color{ 0xF0, 0xF8, 0xFF }, color{ 0xFA, 0xEB, 0xD7 }, color{ 0x00, 0xFF, 0xFF }, color{ 0x7F, 0xFF, 0xD4 }, color{ 0xF0, 0xFF, 0xFF }, color{ 0xF5, 0xF5, 0xDC },
            color{ 0xFF, 0xE4, 0xC4 }, color{ 0x00, 0x00, 0x00 }, color{ 0xFF, 0xEB, 0xCD }, color{ 0x00, 0x00, 0xFF }, color{ 0x8A, 0x2B, 0xE2 }, color{ 0xA5, 0x2A, 0x2A },
            color{ 0xDE, 0xB8, 0x87 }, color{ 0x5F, 0x9E, 0xA0 }, color{ 0x7F, 0xFF, 0x00 }, color{ 0xD2, 0x69, 0x1E }, color{ 0xFF, 0x7F, 0x50 }, color{ 0x64, 0x95, 0xED },
            color{ 0xFF, 0xF8, 0xDC }, color{ 0xDC, 0x14, 0x3C }, color{ 0x00, 0x00, 0x8B }, color{ 0x00, 0x8B, 0x8B }, color{ 0xB8, 0x86, 0x0B }, color{ 0xA9, 0xA9, 0xA9 },
            color{ 0x00, 0x64, 0x00 }, color{ 0xBD, 0xB7, 0x6B }, color{ 0x8B, 0x00, 0x8B }, color{ 0x55, 0x6B, 0x2F }, color{ 0xFF, 0x8C, 0x00 }, color{ 0x99, 0x32, 0xCC },
            color{ 0x8B, 0x00, 0x00 }, color{ 0xE9, 0x96, 0x7A }, color{ 0x8F, 0xBC, 0x8F }, color{ 0x48, 0x3D, 0x8B }, color{ 0x2F, 0x4F, 0x4F }, color{ 0x00, 0xCE, 0xD1 },
            color{ 0x94, 0x00, 0xD3 }, color{ 0xFF, 0x14, 0x93 }, color{ 0x00, 0xBF, 0xFF }, color{ 0x69, 0x69, 0x69 }, color{ 0x1E, 0x90, 0xFF }, color{ 0xB2, 0x22, 0x22 },
            color{ 0xFF, 0xFA, 0xF0 }, color{ 0x22, 0x8B, 0x22 }, color{ 0xFF, 0x00, 0xFF }, color{ 0xDC, 0xDC, 0xDC }, color{ 0xF8, 0xF8, 0xFF }, color{ 0xFF, 0xD7, 0x00 },
            color{ 0xDA, 0xA5, 0x20 }, color{ 0x80, 0x80, 0x80 }, color{ 0x00, 0x80, 0x00 }, color{ 0xAD, 0xFF, 0x2F }, color{ 0xF0, 0xFF, 0xF0 }, color{ 0xFF, 0x69, 0xB4 },
            color{ 0xCD, 0x5C, 0x5C }, color{ 0x4B, 0x00, 0x82 }, color{ 0xFF, 0xFF, 0xF0 }, color{ 0xF0, 0xE6, 0x8C }, color{ 0xE6, 0xE6, 0xFA }, color{ 0xFF, 0xF0, 0xF5 },
            color{ 0x7C, 0xFC, 0x00 }, color{ 0xFF, 0xFA, 0xCD }, color{ 0xAD, 0xD8, 0xE6 }, color{ 0xF0, 0x80, 0x80 }, color{ 0xE0, 0xFF, 0xFF }, color{ 0xFA, 0xFA, 0xD2 },
            color{ 0xD3, 0xD3, 0xD3 }, color{ 0x90, 0xEE, 0x90 }, color{ 0xFF, 0xB6, 0xC1 }, color{ 0xFF, 0xA0, 0x7A }, color{ 0x20, 0xB2, 0xAA }, color{ 0x87, 0xCE, 0xFA },
            color{ 0x77, 0x88, 0x99 }, color{ 0xB0, 0xC4, 0xDE }, color{ 0xFF, 0xFF, 0xE0 }, color{ 0x00, 0xFF, 0x00 }, color{ 0x32, 0xCD, 0x32 }, color{ 0xFA, 0xF0, 0xE6 },
            color{ 0x80, 0x00, 0x00 }, color{ 0x66, 0xCD, 0xAA }, color{ 0x00, 0x00, 0xCD }, color{ 0xBA, 0x55, 0xD3 }, color{ 0x93, 0x70, 0xDB }, color{ 0x3C, 0xB3, 0x71 },
            color{ 0x7B, 0x68, 0xEE }, color{ 0x00, 0xFA, 0x9A }, color{ 0x48, 0xD1, 0xCC }, color{ 0xC7, 0x15, 0x85 }, color{ 0x19, 0x19, 0x70 }, color{ 0xF5, 0xFF, 0xFA },
            color{ 0xFF, 0xE4, 0xE1 }, color{ 0xFF, 0xE4, 0xB5 }, color{ 0xFF, 0xDE, 0xAD }, color{ 0x00, 0x00, 0x80 }, color{ 0xFD, 0xF5, 0xE6 }, color{ 0x80, 0x80, 0x00 },
            color{ 0x6B, 0x8E, 0x23 }, color{ 0xFF, 0xA5, 0x00 }, color{ 0xFF, 0x45, 0x00 }, color{ 0xDA, 0x70, 0xD6 }, color{ 0xEE, 0xE8, 0xAA }, color{ 0x98, 0xFB, 0x98 },
            color{ 0xAF, 0xEE, 0xEE }, color{ 0xDB, 0x70, 0x93 }, color{ 0xFF, 0xEF, 0xD5 }, color{ 0xFF, 0xDA, 0xB9 }, color{ 0xCD, 0x85, 0x3F }, color{ 0xFF, 0xC0, 0xCB },
            color{ 0xDD, 0xA0, 0xDD }, color{ 0xB0, 0xE0, 0xE6 }, color{ 0x80, 0x00, 0x80 }, color{ 0x66, 0x33, 0x99 }, color{ 0xFF, 0x00, 0x00 }, color{ 0xBC, 0x8F, 0x8F },
            color{ 0x41, 0x69, 0xE1 }, color{ 0x8B, 0x45, 0x13 }, color{ 0xFA, 0x80, 0x72 }, color{ 0xF4, 0xA4, 0x60 }, color{ 0x2E, 0x8B, 0x57 }, color{ 0xFF, 0xF5, 0xEE },
            color{ 0xA0, 0x52, 0x2D }, color{ 0xC0, 0xC0, 0xC0 }, color{ 0x87, 0xCE, 0xEB }, color{ 0x6A, 0x5A, 0xCD }, color{ 0x70, 0x80, 0x90 }, color{ 0xFF, 0xFA, 0xFA },
            color{ 0x00, 0xFF, 0x7F }, color{ 0x46, 0x82, 0xB4 }, color{ 0xD2, 0xB4, 0x8C }, color{ 0x00, 0x80, 0x80 }, color{ 0xD8, 0xBF, 0xD8 }, color{ 0xFF, 0x63, 0x47 },
            color{ 0x40, 0xE0, 0xD0 }, color{ 0xEE, 0x82, 0xEE }, color{ 0xF5, 0xDE, 0xB3 }, color{ 0xFF, 0xFF, 0xFF }, color{ 0xF5, 0xF5, 0xF5 }, color{ 0xFF, 0xFF, 0x00 },
            color{ 0x9A, 0xCD, 0x32 }
        };
        iSwatches.push_back(swatch{ "HTML", std::vector<color>(sHtmlColors.begin(), sHtmlColors.end()), true });

        iSwatches.push_back(swatch{ "Basic", {
            color{ 0x00, 0x00, 0x00 }, color{ 0x80, 0x80, 0x80 }, color{ 0xC0, 0xC0, 0xC0 }, color{ 0xFF, 0xFF, 0xFF },
            color{ 0x80, 0x00, 0x00 }, color{ 0xFF, 0x00, 0x00 }, color{ 0x80, 0x00, 0x80 }, color{ 0xFF, 0x00, 0xFF },
            color{ 0x00, 0x80, 0x00 }, color{ 0x00, 0xFF, 0x00 }, color{ 0x80, 0x80, 0x00 }, color{ 0xFF, 0xFF, 0x00 },
            color{ 0x00, 0x00, 0x80 }, color{ 0x00, 0x00, 0xFF }, color{ 0x00, 0x80, 0x80 }, color{ 0x00, 0xFF, 0xFF } }, true });

        swatch greyscale{ "Greyscale", {}, true };
        for (std::int32_t step = 0; step < 24; ++step)
        {
            auto const level = static_cast<std::int32_t>(std::lround(step * 255.0 / 23.0));
            greyscale.colors.push_back(color{ level, level, level });
        }
        iSwatches.push_back(std::move(greyscale));

        // 12 hues x (6 tints, 5 shades) followed by a row of greys
        swatch spectrum{ "Spectrum", {}, true };
        for (std::int32_t tint = 1; tint <= 6; ++tint)
            for (std::int32_t hue = 0; hue < 12; ++hue)
                spectrum.colors.push_back(hsv_color{ hue * 30.0, tint / 6.0, 1.0 }.to_rgb<color>());
        for (std::int32_t shade = 5; shade >= 1; --shade)
            for (std::int32_t hue = 0; hue < 12; ++hue)
                spectrum.colors.push_back(hsv_color{ hue * 30.0, 1.0, shade / 6.0 }.to_rgb<color>());
        for (std::int32_t grey = 11; grey >= 0; --grey)
        {
            auto const level = static_cast<std::int32_t>(std::lround(grey * 255.0 / 11.0));
            spectrum.colors.push_back(color{ level, level, level });
        }
        iSwatches.push_back(std::move(spectrum));

        swatch_list userSwatches;
        try
        {
            std::filesystem::path const folder{ swatch_folder() };
            if (std::filesystem::is_directory(folder))
                for (auto const& file : std::filesystem::directory_iterator{ folder })
                    if (file.is_regular_file() && file.path().extension() == ".csw")
                    {
                        swatch userSwatch;
                        if (load_swatch(file.path().string(), userSwatch))
                            userSwatches.push_back(std::move(userSwatch));
                    }
        }
        catch (...)
        {
            // swatch library unavailable; predefined swatches only
        }
        std::sort(userSwatches.begin(), userSwatches.end(), [](swatch const& lhs, swatch const& rhs) { return lhs.name < rhs.name; });
        for (auto& userSwatch : userSwatches)
            iSwatches.push_back(std::move(userSwatch));

        iCurrentSwatch = 0;
        for (std::size_t swatchIndex = 0; swatchIndex < iSwatches.size(); ++swatchIndex)
            if (iSwatches[swatchIndex].name == previous_swatch())
            {
                iCurrentSwatch = swatchIndex;
                break;
            }
        iCurrentSwatchColor = std::nullopt;

        update_swatch_selector();
        update_swatch_buttons();
    }

    std::size_t color_dialog::add_swatch(swatch&& aSwatch)
    {
        auto existing = std::find_if(iSwatches.begin(), iSwatches.end(), [&](swatch const& s)
        {
            return !s.predefined && std::filesystem::path{ s.path }.lexically_normal() == std::filesystem::path{ aSwatch.path }.lexically_normal();
        });
        if (existing != iSwatches.end())
            *existing = std::move(aSwatch);
        else
            existing = iSwatches.insert(iSwatches.end(), std::move(aSwatch));
        iCurrentSwatch = static_cast<std::size_t>(std::distance(iSwatches.begin(), existing));
        iCurrentSwatchColor = std::nullopt;
        previous_swatch() = iSwatches[iCurrentSwatch].name;
        update_swatch_selector();
        update_swatch_buttons();
        update();
        return iCurrentSwatch;
    }

    void color_dialog::update_swatch_selector()
    {
        iSwatchSelector.selection_model().clear_current_index(); // ensures selected swatch icon is refreshed
        iSwatchSelector.model().clear();
        for (std::uint32_t swatchIndex = 0; swatchIndex < iSwatches.size(); ++swatchIndex)
            iSwatchSelector.model().insert_item(item_model_index{ swatchIndex }, string{ iSwatches[swatchIndex].name });
        if (iCurrentSwatch < iSwatches.size())
        {
            iSwatchSelector.selection_model().set_current_index(iSwatchSelector.presentation_model().from_item_model_index(item_model_index{ static_cast<std::uint32_t>(iCurrentSwatch) }));
            iSwatchSelector.accept_selection();
        }
    }

    void color_dialog::update_swatch_buttons()
    {
        bool const editable = current_swatch_editable();
        iDeleteSwatch.enable(editable);
        iAddToSwatch.enable(editable && iSwatches[iCurrentSwatch].colors.size() < MaxSwatchColors);
        iRemoveFromSwatch.enable(editable && iCurrentSwatchColor != std::nullopt && *iCurrentSwatchColor < iSwatches[iCurrentSwatch].colors.size());
    }

    void color_dialog::set_current_swatch(std::size_t aSwatch)
    {
        if (iCurrentSwatch == aSwatch || aSwatch >= iSwatches.size())
            return;
        iCurrentSwatch = aSwatch;
        iCurrentSwatchColor = std::nullopt;
        previous_swatch() = iSwatches[iCurrentSwatch].name;
        update_swatch_buttons();
        update();
    }

    bool color_dialog::current_swatch_editable() const
    {
        return iCurrentSwatch < iSwatches.size() && !iSwatches[iCurrentSwatch].predefined;
    }

    optional_color color_dialog::swatch_color(std::size_t aSlot) const
    {
        if (iCurrentSwatch < iSwatches.size() && aSlot < iSwatches[iCurrentSwatch].colors.size())
            return iSwatches[iCurrentSwatch].colors[aSlot];
        return std::nullopt;
    }

    std::optional<std::size_t> color_dialog::current_swatch_color() const
    {
        return iCurrentSwatchColor;
    }

    void color_dialog::set_current_swatch_color(std::optional<std::size_t> const& aSlot)
    {
        if (iCurrentSwatchColor == aSlot)
            return;
        iCurrentSwatchColor = aSlot;
        update_swatch_buttons();
        update();
    }

    void color_dialog::update_picker_mode()
    {
        bool const screenPicking = iScreenPicker.is_checked();
        bool const wheel = iPickerMode.is_checked() && !screenPicking;
        iYZPicker.show(!wheel);
        if (!screenPicking)
            iXPicker.show(!wheel);
        iWheelPicker.show(wheel);
        // channel selection only applies to the X/Y-Z picker
        for (auto* channel : { &iH.first, &iS.first, &iV.first, &iR.first, &iG.first, &iB.first, &iA.first })
            channel->enable(!wheel);
        iPickerMode.enable(!screenPicking);
        iPickerMode.set_image(picker_mode_icon(!iPickerMode.is_checked())); // icon shows the mode the button switches to
    }

    void color_dialog::update_widgets(const i_widget& aUpdatingWidget)
    {
        if (iUpdatingWidgets)
            return;
        neolib::scoped_flag sf{ iUpdatingWidgets };
        if (&aUpdatingWidget != &iH.second)
            iH.second.set_value(static_cast<std::int32_t>(selected_color_as_hsv(false).hue()));
        if (&aUpdatingWidget != &iS.second)
            iS.second.set_value(static_cast<std::int32_t>(selected_color_as_hsv(false).saturation() * 100.0));
        if (&aUpdatingWidget != &iV.second)
            iV.second.set_value(static_cast<std::int32_t>(selected_color_as_hsv(false).value() * 100.0));
        if (&aUpdatingWidget != &iR.second)
            iR.second.set_value(selected_color_in_color_space()[0] * color_space_coefficient());
        if (&aUpdatingWidget != &iG.second)
            iG.second.set_value(selected_color_in_color_space()[1] * color_space_coefficient());
        if (&aUpdatingWidget != &iB.second)
            iB.second.set_value(selected_color_in_color_space()[2] * color_space_coefficient());
        if (&aUpdatingWidget != &iA.second)
            iA.second.set_value(selected_color_in_color_space()[3] * color_space_coefficient());
        if (&aUpdatingWidget != &iRgb)
            iRgb.set_text(string{ selected_color().to_hex_string() });
    }
}