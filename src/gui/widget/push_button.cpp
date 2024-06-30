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
            if (iStandardButtonWidth == std::nullopt || iStandardButtonWidth->first != label().text_widget().font())
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
        color outerBorderColor = background_color().darker(0x10);
        color innerBorderColor = border_color();
        scoped_units su{ *this, units::Pixels };
        neogfx::path outline = path();
        dimension penWidth = 1.0;
        switch (iStyle)
        {
        case push_button_style::Normal:
        case push_button_style::ButtonBox:
        case push_button_style::Tab:
        case push_button_style::DropList:
        case push_button_style::SpinBox:
            aGc.fill_path(outline, outerBorderColor);
            outline.deflate(penWidth, penWidth);
            aGc.fill_path(outline, innerBorderColor);
            outline.deflate(penWidth, penWidth);
            break;
        }
        color colorStart = faceColor.lighter(0x0A);
        color colorEnd = faceColor;
        switch(iStyle)
        {
        case push_button_style::Toolbar:
        case push_button_style::TitleBar:
            if (!spot_color())
                aGc.fill_path(outline, gradient{ colorStart.with_lightness(colorStart.to_hsl().lightness() + 0.1), colorEnd });
            else
                aGc.fill_path(outline, faceColor);
            break;
        case push_button_style::Normal:
        case push_button_style::ButtonBox:
        case push_button_style::ItemViewHeader:
        case push_button_style::Tab:
        case push_button_style::DropList:
        case push_button_style::SpinBox:
            if (!spot_color())
                aGc.fill_path(outline, gradient{ colorStart, colorEnd });
            else
                aGc.fill_path(outline, faceColor);
            break;
        }
        if (has_focus())
        {
            rect focusRect = outline.bounding_rect();
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
        return client_rect();
    }

    path push_button::path() const
    {
        neogfx::path ret;
        size pixel = units_converter{ *this }.from_device_units(size(1.0, 1.0));
        size currentSize = path_bounding_rect().extents();
        switch (iStyle)
        {
        case push_button_style::Normal:
        case push_button_style::ButtonBox:
        case push_button_style::Tab:
        case push_button_style::DropList:
        case push_button_style::SpinBox:
            ret.move_to(pixel.cx, 0, 12);
            ret.line_to(currentSize.cx - pixel.cx, 0);
            ret.line_to(currentSize.cx - pixel.cx, pixel.cy);
            ret.line_to(currentSize.cx - 0, pixel.cy);
            ret.line_to(currentSize.cx - 0, currentSize.cy - pixel.cy);
            ret.line_to(currentSize.cx - pixel.cx, currentSize.cy - pixel.cy);
            ret.line_to(currentSize.cx - pixel.cx, currentSize.cy - 0);
            ret.line_to(pixel.cx, currentSize.cy - 0);
            ret.line_to(pixel.cx, currentSize.cy - pixel.cy);
            ret.line_to(0, currentSize.cy - pixel.cy);
            ret.line_to(0, pixel.cy);
            ret.line_to(pixel.cx, pixel.cy);
            ret.line_to(pixel.cx, 0);
            break;
        case push_button_style::ItemViewHeader:
            ret.move_to(0, 0, 4);
            ret.line_to(currentSize.cx, 0);
            ret.line_to(currentSize.cx, currentSize.cy);
            ret.line_to(0, currentSize.cy);
            ret.line_to(0, 0);
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
        return iFaceColor != std::nullopt;
    }

    color push_button::face_color() const
    {
        if (has_face_color())
            return iFaceColor.value();
        color result = base_color();
        style_sheet_value(".button", "background-color", result);
        return result;
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
        return iHoverColor != std::nullopt;
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
        layout().set_padding(neogfx::padding{});
        label().set_padding(neogfx::padding{});
        switch(iStyle)
        {
        case push_button_style::ItemViewHeader:
            set_padding(neogfx::padding{ 1.0_dip, 2.0_dip });
            label().text_widget().set_alignment(neogfx::alignment::Left | neogfx::alignment::VCenter);
            break;
        case push_button_style::Toolbar:
            set_padding(neogfx::padding{ 2.0_dip, 2.0_dip });
            label().text_widget().set_font_role(neogfx::font_role::Toolbar);
            break;
        case push_button_style::TitleBar:
            set_padding(neogfx::padding{});
            label().text_widget().set_font_role(neogfx::font_role::Caption);
            break;
        case push_button_style::SpinBox:
            set_padding(neogfx::padding{ 2.0_dip, 2.0_dip });
            break;
        case push_button_style::Tab:
            break;
        case push_button_style::Normal:
            set_padding(neogfx::padding{ 8.0_dip, 4.0_dip });
            break;
        default:
            set_padding(neogfx::padding{ 4.0_dip, 4.0_dip });
            break;
        }
    }
}

