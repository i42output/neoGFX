// push_button.cpp
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

#include <neogfx/app/i_app.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/pen.hpp>
#include <neogfx/gui/widget/push_button.hpp>

namespace neogfx
{
    inline alignment default_push_button_alignment(push_button_style aStyle)
    {
        switch (aStyle)
        {
        case push_button_style::Normal:
        case push_button_style::ButtonBox:
        case push_button_style::SpinBox:
        case push_button_style::Tab:
            return alignment::Center | alignment::VCenter;
        default:
            return alignment::Left | alignment::VCenter;
        }
    }

    push_button::push_button(push_button_style aStyle) :
        button{ default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(std::string const& aText, push_button_style aStyle) :
        button{ aText, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(const i_texture& aTexture, push_button_style aStyle) :
        button{ aTexture, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(const i_image& aImage, push_button_style aStyle) :
        button{ aImage, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }
    
    push_button::push_button(std::string const& aText, const i_texture& aTexture, push_button_style aStyle) :
        button{ aText, aTexture, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(std::string const& aText, const i_image& aImage, push_button_style aStyle) :
        button{ aText, aImage, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(i_widget& aParent, push_button_style aStyle) :
        button{ aParent, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(i_widget& aParent, std::string const& aText, push_button_style aStyle) :
        button{ aParent, aText, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(i_widget& aParent, const i_texture& aTexture, push_button_style aStyle) :
        button{ aParent, aTexture, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(i_widget& aParent, const i_image& aImage, push_button_style aStyle) :
        button{ aParent, aImage, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(i_widget& aParent, std::string const& aText, const i_texture& aTexture, push_button_style aStyle) :
        button{ aParent, aText, aTexture, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(i_widget& aParent, std::string const& aText, const i_image& aImage, push_button_style aStyle) :
        button{ aParent, aText, aImage, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(i_layout& aLayout, push_button_style aStyle) :
        button{ aLayout, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(i_layout& aLayout, std::string const& aText, push_button_style aStyle) :
        button{ aLayout, aText, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(i_layout& aLayout, const i_texture& aTexture, push_button_style aStyle) :
        button{ aLayout, aTexture, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(i_layout& aLayout, const i_image& aImage, push_button_style aStyle) :
        button{ aLayout, aImage, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(i_layout& aLayout, std::string const& aText, const i_texture& aTexture, push_button_style aStyle) :
        button{ aLayout, aText, aTexture, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    push_button::push_button(i_layout& aLayout, std::string const& aText, const i_image& aImage, push_button_style aStyle) :
        button{ aLayout, aText, aImage, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    size push_button::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (has_minimum_size())
            return button::minimum_size(aAvailableSpace);
        size result = button::minimum_size(aAvailableSpace);
        if (iStyle == push_button_style::ButtonBox)
        {
            if (!iStandardButtonWidth.has_value() || iStandardButtonWidth->first != label().text_widget().font())
            {
                graphics_context gc{ *this, graphics_context::type::Unattached };
                iStandardButtonWidth.emplace(label().text_widget().font(), gc.text_extent("#StdButton", label().text_widget().font()));
                iStandardButtonWidth->second.cx += (result.cx - label().text_widget().minimum_size(aAvailableSpace).cx);
            }
            result.cx = std::max(result.cx, iStandardButtonWidth->second.cx);
        }
        return result;
    }

    size push_button::maximum_size(optional_size const& aAvailableSpace) const
    {
        if (has_maximum_size())
            return button::maximum_size(aAvailableSpace);
        if (iStyle == push_button_style::ButtonBox)
            return minimum_size(aAvailableSpace);
        return button::maximum_size(aAvailableSpace);
    }

    padding push_button::padding() const
    {
        if (has_padding())
            return button::padding();
        switch (iStyle)
        {
        case push_button_style::ItemViewHeader:
            return neogfx::padding{ 1.0_dip, 2.0_dip };
        case push_button_style::Toolbar:
            return neogfx::padding{ 2.0_dip, 2.0_dip };
        case push_button_style::TitleBar:
            return neogfx::padding{};
        case push_button_style::SpinBox:
            return neogfx::padding{ 2.0_dip, 2.0_dip };
        case push_button_style::Tab:
            return button::padding();
        case push_button_style::Normal:
            return neogfx::padding{ 8.0_dip, 4.0_dip };
        default:
            return neogfx::padding{ 4.0_dip, 4.0_dip };
        }
    }

    void push_button::paint_non_client(i_graphics_context& aGc) const
    {
        button::paint_non_client(aGc);
        if ((iStyle == push_button_style::Toolbar || iStyle == push_button_style::TitleBar) && enabled() && (entered() || capturing()) && !ignore_mouse_events())
        {
            color background = (capturing() && entered() ? 
                service<i_app>().current_style().palette().color(color_role::Selection) : 
                background_color().shaded(0x40));
            background.set_alpha(0x80);
            aGc.fill_rect(client_rect(), background);
        }
    }

    void push_button::paint(i_graphics_context& aGc) const
    {
        // todo: move to default skin

        color faceColor = effective_face_color();
        color colorStart = faceColor.lighter(0x0A);
        color colorEnd = faceColor;
        color outerBorderColor = background_color().darker(0x10);
        color innerBorderColor = border_color();
        std::optional<line_style> lineStyle;

        scoped_units su{ *this, units::Pixels };

        neogfx::path outlinePath = path();

        auto const& borderRadii = style_sheet_value("." + class_name(), "border-radius", std::optional<std::array<std::array<length, 2u>, 4u>>{});
        auto const& border = style_sheet_value("." + class_name(), "border", std::tuple<std::optional<color>, std::optional<length>, std::optional<border_style>>{});
        auto const& borderStyle = style_sheet_value("." + class_name(), "border-style", std::optional<border_style>{});

        if (std::get<0>(border).has_value())
        {
            outerBorderColor = std::get<0>(border).value();
            innerBorderColor = outerBorderColor;
        }
        
        if (borderStyle.has_value())
            lineStyle = to_line_style(borderStyle.value());

        if (std::get<2>(border).has_value())
            lineStyle = to_line_style(std::get<2>(border).value());

        if (borderRadii.has_value())
        {
            auto to_vec2 = [&](std::array<length, 2u> const& l)
            {
                basic_length<vec2> lx{ vec2{ l[0].unconverted_value(), 0.0 }, l[0].units() };
                basic_length<vec2> ly{ vec2{ 0.0, l[1].unconverted_value(), }, l[1].units() };
                return vec2{ lx.value().x, ly.value().y };
            };
            pen outline{ outerBorderColor, pen_width(), lineStyle.value_or(line_style::Solid) };
            if (outerBorderColor != innerBorderColor)
                outline.set_secondary_color(innerBorderColor);
            aGc.draw_ellipse_rect(
                path_bounding_rect().inflate(pen_width() / 2.0),
                vec4{ to_vec2(borderRadii.value()[0]).x, to_vec2(borderRadii.value()[1]).x, to_vec2(borderRadii.value()[2]).x, to_vec2(borderRadii.value()[3]).x },
                vec4{ to_vec2(borderRadii.value()[0]).y, to_vec2(borderRadii.value()[1]).y, to_vec2(borderRadii.value()[2]).y, to_vec2(borderRadii.value()[3]).y },
                outline,
                !spot_color() ? 
                    brush{ gradient{ colorStart, colorEnd } } :
                    brush{ faceColor });
        }
        else
        {
            switch (iStyle)
            {
            case push_button_style::Normal:
            case push_button_style::ButtonBox:
            case push_button_style::Tab:
            case push_button_style::DropList:
            case push_button_style::SpinBox:
                if (outerBorderColor != innerBorderColor)
                    aGc.draw_path(
                        outlinePath, 
                        pen{ outerBorderColor, pen_width(), lineStyle.value_or(line_style::Solid), false }.set_secondary_color(innerBorderColor),
                        !spot_color() ? brush{ gradient{ colorStart, colorEnd } } : brush{ faceColor });
                else
                    aGc.draw_path(
                        outlinePath, 
                        pen{ outerBorderColor, pen_width(), lineStyle.value_or(line_style::Solid), false },
                        !spot_color() ? brush{ gradient{ colorStart, colorEnd } } : brush{ faceColor });
                break;
            }
            switch (iStyle)
            {
            case push_button_style::Toolbar:
            case push_button_style::TitleBar:
                if (!spot_color())
                    aGc.fill_path(outlinePath, gradient{ colorStart.with_lightness(colorStart.to_hsl().lightness() + 0.1), colorEnd });
                else
                    aGc.fill_path(outlinePath, faceColor);
                break;
            case push_button_style::ItemViewHeader:
                if (!spot_color())
                    aGc.fill_path(outlinePath, gradient{ colorStart, colorEnd });
                else
                    aGc.fill_path(outlinePath, faceColor);
                break;
            }
        }
        if (has_focus())
        {
            rect focusRect = outlinePath.bounding_rect();
            focusRect.deflate(2.0, 2.0);
            aGc.draw_focus_rect(focusRect);
        }
    }

    color push_button::palette_color(color_role aColorRole) const
    {
        if (has_palette_color(aColorRole))
            return button::palette_color(aColorRole);
        if (aColorRole == color_role::Base)
            return background_color().shaded(0x0B);
        return button::palette_color(aColorRole);
    }

    void push_button::mouse_entered(const point& aPosition)
    {
        button::mouse_entered(aPosition);
        if (perform_hover_animation() || !finished_animation())
            iAnimator.again_if();
        update();
    }

    void push_button::mouse_left()
    {
        button::mouse_left();
        if (perform_hover_animation() || !finished_animation())
            iAnimator.again_if();
        update();
    }

    push_button_style push_button::style() const
    {
        return iStyle;
    }

    rect push_button::path_bounding_rect() const
    {
        auto result = client_rect();
        result.deflate(pen_width() / 2.0, pen_width() / 2.0);
        return result;
    }

    path push_button::path() const
    {
        neogfx::path ret;
        size const pixel = units_converter{ *this }.from_device_units(size(1.0, 1.0));
        size const pathSize = path_bounding_rect().extents();
        box_areas const pathBox{ 0.0, 0.0, pathSize.cx, pathSize.cy };
        switch (iStyle)
        {
        case push_button_style::Normal:
        case push_button_style::ButtonBox:
        case push_button_style::Tab:
        case push_button_style::DropList:
        case push_button_style::SpinBox:
            ret.move_to(pathBox.left + pixel.cx, pathBox.top, 12);
            ret.line_to(pathBox.right - pixel.cx, pathBox.top);
            ret.line_to(pathBox.right - pixel.cx, pathBox.top + pixel.cy);
            ret.line_to(pathBox.right, pathBox.top + pixel.cy);
            ret.line_to(pathBox.right, pathBox.bottom - pixel.cy);
            ret.line_to(pathBox.right - pixel.cx, pathBox.bottom - pixel.cy);
            ret.line_to(pathBox.right - pixel.cx, pathBox.bottom);
            ret.line_to(pathBox.left + pixel.cx, pathBox.bottom);
            ret.line_to(pathBox.left + pixel.cx, pathBox.bottom - pixel.cy);
            ret.line_to(pathBox.left, pathBox.bottom - pixel.cy);
            ret.line_to(pathBox.left, pathBox.top + pixel.cy);
            ret.line_to(pathBox.left + pixel.cx, pathBox.top + pixel.cy);
            ret.line_to(pathBox.left + pixel.cx, pathBox.top);
            break;
        case push_button_style::ItemViewHeader:
            ret.move_to(pathBox.left, pathBox.top, 4);
            ret.line_to(pathBox.right, pathBox.top);
            ret.line_to(pathBox.right, pathBox.bottom);
            ret.line_to(pathBox.left, pathBox.bottom);
            ret.line_to(pathBox.left, pathBox.top);
            break;
        }
        ret.set_position(path_bounding_rect().top_left());
        return ret;
    }

    bool push_button::spot_color() const
    {
        return false;
    }

    color push_button::border_color() const
    {
        return effective_face_color().shaded(0x20);
    }

    bool push_button::perform_hover_animation() const
    {
        return true;
    }

    bool push_button::has_face_color() const
    {
        return iFaceColor.has_value();
    }

    color push_button::face_color() const
    {
        if (has_face_color())
            return iFaceColor.value();
        else if (iStyleSheetFaceColor.has_value())
            return iStyleSheetFaceColor.value();
        return (iStyleSheetFaceColor = style_sheet_value("." + class_name(), "background-color", base_color())).value();
    }

    void push_button::set_face_color(const optional_color& aFaceColor)
    {
        if (iFaceColor != aFaceColor)
        {
            iFaceColor = aFaceColor;
            update();
        }
    }

    bool push_button::has_hover_color() const
    {
        return iHoverColor.has_value();
    }

    color push_button::hover_color() const
    {
        return (has_hover_color() ? *iHoverColor : service<i_app>().current_style().palette().color(color_role::Hover));
    }

    void push_button::set_hover_color(const optional_color& aHoverColor)
    {
        if (iHoverColor != aHoverColor)
        {
            iHoverColor = aHoverColor;
            update();
        }
    }

    void push_button::animate()
    {
        if (!root().has_native_surface())
            return;

        if (entered() && enabled())
        {
            if (iAnimationFrame < kMaxAnimationFrame)
            {
                ++iAnimationFrame;
                iAnimator.again();
            }
        }
        else
        {
            if (iAnimationFrame > 0)
            {
                --iAnimationFrame;
                iAnimator.again();
            }
        }
        update();
    }

    bool push_button::finished_animation() const
    {
        return iAnimationFrame == 0;
    }

    color push_button::effective_face_color() const
    {
        return animation_color(iAnimationFrame);
    }

    color push_button::effective_hover_color() const
    {
        return capturing() ? hover_color().shaded(0x40) : hover_color();
    }

    color push_button::animation_color(std::uint32_t aAnimationFrame) const
    {
        color faceColor = face_color();
        if (capturing())
            faceColor.shade(0x40);
        auto animationColor = (enabled() && entered() && perform_hover_animation()) || !finished_animation() ? 
            gradient(faceColor, effective_hover_color()).at(static_cast<coordinate>(aAnimationFrame), 0, static_cast<coordinate>(kMaxAnimationFrame)) : faceColor;
        if (is_checked())
            animationColor.unshade(0x0A);
        return animationColor;
    }

    void push_button::init()
    {
        iSink += StyleSheetChanged([&](auto const&) { iPenWidth = std::nullopt; iStyleSheetFaceColor = std::nullopt; });
        iSink += Palette.Changed([&](auto const&) { iPenWidth = std::nullopt; iStyleSheetFaceColor = std::nullopt; });
        iSink += service<i_app>().current_style_changed([&](auto const&)
            { iSink2 = service<i_app>().current_style().changed([&](auto const&)
                { iPenWidth = std::nullopt; iStyleSheetFaceColor = std::nullopt; }); });

        layout().set_padding(neogfx::padding{});
        label().set_padding(neogfx::padding{});
        switch(iStyle)
        {
        case push_button_style::ItemViewHeader:
            label().text_widget().set_alignment(neogfx::alignment::Left | neogfx::alignment::VCenter);
            break;
        case push_button_style::Toolbar:
            label().text_widget().set_font_role(neogfx::font_role::Toolbar);
            break;
        case push_button_style::TitleBar:
            label().text_widget().set_font_role(neogfx::font_role::Caption);
            break;
        case push_button_style::SpinBox:
            break;
        case push_button_style::Tab:
            break;
        case push_button_style::Normal:
            break;
        default:
            break;
        }
    }

    scalar push_button::pen_width() const
    {
        if (!iPenWidth.has_value())
        {
            switch (iStyle)
            {
            case push_button_style::Normal:
            case push_button_style::ButtonBox:
            case push_button_style::Tab:
            case push_button_style::DropList:
            case push_button_style::SpinBox:
                {
                    color outerBorderColor = background_color().darker(0x10);
                    color innerBorderColor = border_color();
                    auto const& borderRadii = style_sheet_value("." + class_name(), "border-radius", std::optional<std::array<std::array<length, 2u>, 4u>>{});
                    auto const& border = style_sheet_value("." + class_name(), "border", std::tuple<std::optional<color>, std::optional<length>, std::optional<border_style>>{});
                    if (std::get<0>(border).has_value())
                    {
                        outerBorderColor = std::get<0>(border).value();
                        innerBorderColor = outerBorderColor;
                    }
                    if (std::get<1>(border).has_value())
                        iPenWidth = std::get<1>(border).value();
                    else if (borderRadii.has_value())
                        iPenWidth = 2.0;
                    if (!iPenWidth.has_value())
                        iPenWidth = outerBorderColor != innerBorderColor ? 2.0 : 1.0;
                }
                break;
            default:
                iPenWidth = 0.0;
                break;
            }
        }
        return iPenWidth.value();
    }
}

