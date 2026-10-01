// push_button.ipp
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

#pragma once

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

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(push_button_style aStyle) :
        base_type{ default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(string const& aText, push_button_style aStyle) :
        base_type{ aText, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(const i_texture& aTexture, push_button_style aStyle) :
        base_type{ aTexture, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(const i_image& aImage, push_button_style aStyle) :
        base_type{ aImage, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }
    
    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(string const& aText, const i_texture& aTexture, push_button_style aStyle) :
        base_type{ aText, aTexture, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(string const& aText, const i_image& aImage, push_button_style aStyle) :
        base_type{ aText, aImage, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(i_widget& aParent, push_button_style aStyle) :
        base_type{ aParent, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(i_widget& aParent, string const& aText, push_button_style aStyle) :
        base_type{ aParent, aText, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(i_widget& aParent, const i_texture& aTexture, push_button_style aStyle) :
        base_type{ aParent, aTexture, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(i_widget& aParent, const i_image& aImage, push_button_style aStyle) :
        base_type{ aParent, aImage, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(i_widget& aParent, string const& aText, const i_texture& aTexture, push_button_style aStyle) :
        base_type{ aParent, aText, aTexture, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(i_widget& aParent, string const& aText, const i_image& aImage, push_button_style aStyle) :
        base_type{ aParent, aText, aImage, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(i_layout& aLayout, push_button_style aStyle) :
        base_type{ aLayout, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(i_layout& aLayout, string const& aText, push_button_style aStyle) :
        base_type{ aLayout, aText, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(i_layout& aLayout, const i_texture& aTexture, push_button_style aStyle) :
        base_type{ aLayout, aTexture, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(i_layout& aLayout, const i_image& aImage, push_button_style aStyle) :
        base_type{ aLayout, aImage, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(i_layout& aLayout, string const& aText, const i_texture& aTexture, push_button_style aStyle) :
        base_type{ aLayout, aText, aTexture, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline basic_push_button<PushButtonInterface>::basic_push_button(i_layout& aLayout, string const& aText, const i_image& aImage, push_button_style aStyle) :
        base_type{ aLayout, aText, aImage, default_push_button_alignment(aStyle) },
        iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false },
        iAnimationFrame{ 0 },
        iStyle{ aStyle }
    {
        init();
    }

    template <typename PushButtonInterface>
    inline size basic_push_button<PushButtonInterface>::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (this->has_minimum_size())
            return base_type::minimum_size(aAvailableSpace);
        size result = base_type::minimum_size(aAvailableSpace);
        if (iStyle == push_button_style::ButtonBox)
        {
            if (!iStandardButtonWidth.has_value() || iStandardButtonWidth->first != this->label().text_widget().font())
            {
                graphics_context gc{ *this, graphics_context::type::Unattached };
                iStandardButtonWidth.emplace(this->label().text_widget().font(), gc.text_extent("#StdButton", this->label().text_widget().font()));
                iStandardButtonWidth->second.cx += (result.cx - this->label().text_widget().minimum_size(aAvailableSpace).cx);
            }
            result.cx = std::max(result.cx, iStandardButtonWidth->second.cx);
        }
        return result;
    }

    template <typename PushButtonInterface>
    inline size basic_push_button<PushButtonInterface>::maximum_size(optional_size const& aAvailableSpace) const
    {
        if (this->has_maximum_size())
            return base_type::maximum_size(aAvailableSpace);
        if (iStyle == push_button_style::ButtonBox)
            return minimum_size(aAvailableSpace);
        return base_type::maximum_size(aAvailableSpace);
    }

    template <typename PushButtonInterface>
    inline padding basic_push_button<PushButtonInterface>::padding() const
    {
        if (this->has_padding())
            return base_type::padding();
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
            return base_type::padding();
        case push_button_style::Normal:
            return neogfx::padding{ 8.0_dip, 4.0_dip };
        default:
            return neogfx::padding{ 4.0_dip, 4.0_dip };
        }
    }

    template <typename PushButtonInterface>
    inline void basic_push_button<PushButtonInterface>::paint_non_client(i_graphics_context& aGc) const
    {
        base_type::paint_non_client(aGc);
        if ((iStyle == push_button_style::Toolbar || iStyle == push_button_style::TitleBar) && this->enabled() && (this->entered() || this->capturing()) && !this->ignore_mouse_events())
        {
            color background = (this->capturing() && this->entered() ? 
                service<i_app>().current_style().palette().color(color_role::Selection) : 
                this->background_color().shaded(0x40));
            background.set_alpha(0x80);
            aGc.fill_rect(this->client_rect(), background);
        }
    }

    template <typename PushButtonInterface>
    inline void basic_push_button<PushButtonInterface>::paint(i_graphics_context& aGc) const
    {
        // todo: move to default skin

        color faceColor = effective_face_color();
        color colorStart = faceColor.lighter(0x0A);
        color colorEnd = faceColor;
        color outerBorderColor = outer_border_color();
        color innerBorderColor = border_color();    
        std::optional<line_style> lineStyle;

        scoped_units su{ *this, units::Pixels };

        auto const& borderRadii = this->style_sheet_value("." + this->class_name(), "border-radius", border_radius());
        auto const& border = this->style_sheet_value("." + this->class_name(), "border", std::tuple<std::optional<color>, std::optional<length>, std::optional<border_style>>{});
        auto const& borderStyle = this->style_sheet_value("." + this->class_name(), "border-style", std::optional<border_style>{});

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
            neogfx::path outlinePath = path();

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
        if (this->has_focus())
        {
            rect focusRect = path_bounding_rect();
            focusRect.deflate(2.0, 2.0);
            aGc.draw_focus_rect(focusRect);
        }
    }

    template <typename PushButtonInterface>
    inline color basic_push_button<PushButtonInterface>::palette_color(color_role aColorRole) const
    {
        if (this->has_palette_color(aColorRole))
            return base_type::palette_color(aColorRole);
        if (aColorRole == color_role::Base)
            return this->background_color().shaded(0x0B);
        return base_type::palette_color(aColorRole);
    }

    template <typename PushButtonInterface>
    inline void basic_push_button<PushButtonInterface>::mouse_entered(const point& aPosition)
    {
        base_type::mouse_entered(aPosition);
        if (perform_hover_animation() || !finished_animation())
            iAnimator.again_if();
        this->update();
    }

    template <typename PushButtonInterface>
    inline void basic_push_button<PushButtonInterface>::mouse_left()
    {
        base_type::mouse_left();
        if (perform_hover_animation() || !finished_animation())
            iAnimator.again_if();
        this->update();
    }

    template <typename PushButtonInterface>
    inline push_button_style basic_push_button<PushButtonInterface>::style() const
    {
        return iStyle;
    }

    template <typename PushButtonInterface>
    inline rect basic_push_button<PushButtonInterface>::path_bounding_rect() const
    {
        auto result = this->client_rect();
        result.deflate(pen_width() / 2.0, pen_width() / 2.0);
        return result;
    }

    template <typename PushButtonInterface>
    inline path basic_push_button<PushButtonInterface>::path() const
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

    template <typename PushButtonInterface>
    inline bool basic_push_button<PushButtonInterface>::spot_color() const
    {
        return false;
    }

    template <typename PushButtonInterface>
    inline bool basic_push_button<PushButtonInterface>::perform_hover_animation() const
    {
        return true;
    }

    template <typename PushButtonInterface>
    inline bool basic_push_button<PushButtonInterface>::has_face_color() const
    {
        return this->has_base_color();
    }

    template <typename PushButtonInterface>
    inline color basic_push_button<PushButtonInterface>::face_color() const
    {
        if (this->has_base_color())
            return this->base_color();
        return iStyleSheetFaceColor.value();
    }

    template <typename PushButtonInterface>
    inline void basic_push_button<PushButtonInterface>::set_face_color(const optional_color& aFaceColor)
    {
        this->set_base_color(aFaceColor);
    }

    template <typename PushButtonInterface>
    inline bool basic_push_button<PushButtonInterface>::has_hover_color() const
    {
        return HoverColor.value().has_value();
    }

    template <typename PushButtonInterface>
    inline color basic_push_button<PushButtonInterface>::hover_color() const
    {
        return HoverColor.value().value_or(service<i_app>().current_style().palette().color(color_role::Hover));
    }

    template <typename PushButtonInterface>
    inline void basic_push_button<PushButtonInterface>::set_hover_color(const optional_color& aHoverColor)
    {
        HoverColor = aHoverColor;
    }

    template <typename PushButtonInterface>
    inline bool basic_push_button<PushButtonInterface>::has_border_color() const
    {
        return BorderColor.value().has_value();
    }

    template <typename PushButtonInterface>
    inline color basic_push_button<PushButtonInterface>::border_color() const
    {
        if (has_border_color())
            return BorderColor.value().value();
        return iStyleSheetBorderColor.value();
    }

    template <typename PushButtonInterface>
    inline void basic_push_button<PushButtonInterface>::set_border_color(const optional_color& aBorderColor)
    {
        BorderColor = aBorderColor;
    }

    template <typename PushButtonInterface>
    inline bool basic_push_button<PushButtonInterface>::has_outer_border_color() const
    {
        return OuterBorderColor.value().has_value();
    }

    template <typename PushButtonInterface>
    inline color basic_push_button<PushButtonInterface>::outer_border_color() const
    {
        return OuterBorderColor.value().value_or(this->background_color().darker(0x10));
    }

    template <typename PushButtonInterface>
    inline void basic_push_button<PushButtonInterface>::set_outer_border_color(const optional_color& aOuterBorderColor)
    {
        OuterBorderColor = aOuterBorderColor;
    }

    template <typename PushButtonInterface>
    inline std::optional<border_radii> const& basic_push_button<PushButtonInterface>::border_radius() const
    {
        return BorderRadius.value();
    }

    template <typename PushButtonInterface>
    inline void basic_push_button<PushButtonInterface>::set_border_radius(std::optional<border_radii> const& aBorderRadii)
    {
        BorderRadius = aBorderRadii;
    }

    template <typename PushButtonInterface>
    inline void basic_push_button<PushButtonInterface>::set_border_radius(length const& aBorderRadius)
    {
        set_border_radius(border_radii{ { { aBorderRadius , aBorderRadius }, { aBorderRadius , aBorderRadius }, { aBorderRadius , aBorderRadius }, { aBorderRadius , aBorderRadius } } });
    }

    template <typename PushButtonInterface>
    inline void basic_push_button<PushButtonInterface>::animate()
    {
        if (!this->root().has_native_surface())
            return;

        if (this->entered() && this->enabled())
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
        this->update();
    }

    template <typename PushButtonInterface>
    inline bool basic_push_button<PushButtonInterface>::finished_animation() const
    {
        return iAnimationFrame == 0;
    }

    template <typename PushButtonInterface>
    inline color basic_push_button<PushButtonInterface>::effective_face_color() const
    {
        return animation_color(iAnimationFrame);
    }

    template <typename PushButtonInterface>
    inline color basic_push_button<PushButtonInterface>::effective_hover_color() const
    {
        return this->capturing() ? hover_color().shaded(0x40) : hover_color();
    }

    template <typename PushButtonInterface>
    inline color basic_push_button<PushButtonInterface>::effective_border_color() const
    {
        return effective_face_color().shaded(0x20);
    }

    template <typename PushButtonInterface>
    inline color basic_push_button<PushButtonInterface>::animation_color(std::uint32_t aAnimationFrame) const
    {
        color faceColor = face_color();
        if (this->capturing())
            faceColor.shade(0x40);
        auto animationColor = (this->enabled() && this->entered() && perform_hover_animation()) || !finished_animation() ? 
            gradient(faceColor, effective_hover_color()).at(static_cast<coordinate>(aAnimationFrame), 0, static_cast<coordinate>(kMaxAnimationFrame)) : faceColor;
        if (this->is_checked())
            animationColor.unshade(0x0A);
        return animationColor;
    }

    template <typename PushButtonInterface>
    inline void basic_push_button<PushButtonInterface>::init()
    {
        iSink += this->StyleSheetChanged([&](auto const&) { iPenWidth = std::nullopt; iStyleSheetFaceColor.reset(); iStyleSheetBorderColor.reset(); });
        iSink += this->Palette.Changed([&](auto const&) { iPenWidth = std::nullopt; });
        iSink += service<i_app>().current_style_changed([&](auto const&)
            { iSink2 = service<i_app>().current_style().changed([&](auto const&)
                { iPenWidth = std::nullopt; }); });

        this->layout().set_padding(neogfx::padding{});
        this->label().set_padding(neogfx::padding{});
        switch(iStyle)
        {
        case push_button_style::ItemViewHeader:
            this->label().text_widget().set_alignment(neogfx::alignment::Left | neogfx::alignment::VCenter);
            break;
        case push_button_style::Toolbar:
            this->label().text_widget().set_font_role(neogfx::font_role::Toolbar);
            break;
        case push_button_style::TitleBar:
            this->label().text_widget().set_font_role(neogfx::font_role::Caption);
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

    template <typename PushButtonInterface>
    inline scalar basic_push_button<PushButtonInterface>::pen_width() const
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
                    color outerBorderColor = this->background_color().darker(0x10);
                    color innerBorderColor = border_color();
                    auto const& borderRadii = this->style_sheet_value("." + this->class_name(), "border-radius", std::optional<std::array<std::array<length, 2u>, 4u>>{});
                    auto const& border = this->style_sheet_value("." + this->class_name(), "border", std::tuple<std::optional<color>, std::optional<length>, std::optional<border_style>>{});
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

