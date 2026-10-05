// push_button.hpp
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

#include <neogfx/gui/widget/button.hpp>
#include <neogfx/gui/widget/i_push_button.hpp>

namespace neogfx
{
    extern template class widget<i_push_button>;
    extern template class button<i_push_button>;

    template <typename PushButtonInterface = i_push_button>
    class basic_push_button : public button<PushButtonInterface>
    {
        meta_object(button<PushButtonInterface>)
        typedef basic_push_button property_context_type;
    public:
        static const std::uint32_t kMaxAnimationFrame = 10;
    public:
        basic_push_button(push_button_style aStyle = push_button_style::Normal);
        basic_push_button(string const& aText, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(const i_texture& aTexture, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(const i_image& aImage, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(string const& aText, const i_texture& aTexture, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(string const& aText, const i_image& aImage, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(i_widget& aParent, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(i_widget& aParent, string const& aText, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(i_widget& aParent, const i_texture& aTexture, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(i_widget& aParent, const i_image& aImage, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(i_widget& aParent, string const& aText, const i_texture& aTexture, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(i_widget& aParent, string const& aText, const i_image& aImage, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(i_layout& aLayout, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(i_layout& aLayout, string const& aText, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(i_layout& aLayout, const i_texture& aTexture, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(i_layout& aLayout, const i_image& aImage, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(i_layout& aLayout, string const& aText, const i_texture& aTexture, push_button_style aStyle = push_button_style::Normal);
        basic_push_button(i_layout& aLayout, string const& aText, const i_image& aImage, push_button_style aStyle = push_button_style::Normal);
        // button
    public:
        size minimum_size(optional_size const& aAvailableSpace = {}) const override;
        size maximum_size(optional_size const& aAvailableSpace = {}) const override;
        neogfx::padding padding() const override;
    public:
        void paint_non_client(i_graphics_context& aGc) const override;
        void paint(i_graphics_context& aGc) const override;
    public:
        color palette_color(color_role aColorRole) const override;
    public:
        void mouse_entered(const point& aPosition) override;
        void mouse_left() override;
        // i_push_button
    public:
        push_button_style style() const override;
        // push_button
    public:
        std::optional<border_radii> const& border_radius() const;
        void set_border_radius(std::optional<border_radii> const& aBorderRadii = {});
        void set_border_radius(length const& aBorderRadius);
    public:
        virtual bool has_face_color() const;
        virtual color face_color() const;
        virtual void set_face_color(const optional_color& aFaceColor = {});
        virtual bool has_hover_color() const;
        virtual color hover_color() const;
        virtual void set_hover_color(const optional_color& aHoverColor = {});
        virtual bool has_border_color() const;
        virtual color border_color() const;
        virtual void set_border_color(const optional_color& aBorderColor = {});
        virtual bool has_outer_border_color() const;
        virtual color outer_border_color() const;
        virtual void set_outer_border_color(const optional_color& aOuterBorderColor = {});
    protected:
        virtual rect path_bounding_rect() const;
        virtual neogfx::path path() const;
        virtual bool spot_color() const;
        virtual bool perform_hover_animation() const;
        virtual void animate();
        virtual bool finished_animation() const;
        virtual color effective_face_color() const;
        virtual color effective_hover_color() const;
        virtual color effective_border_color() const;
        virtual color animation_color(std::uint32_t aAnimationFrame) const;
    private:
        void init();
        scalar pen_width() const;
    private:
        widget_timer iAnimator;
        std::uint32_t iAnimationFrame;
        push_button_style iStyle;
        define_style_sheet_value(iStyleSheetFaceColor, color, "background-color", this->base_color())
        define_style_sheet_value(iStyleSheetBorderColor, color, "border-color", effective_border_color())
        mutable std::optional<std::pair<neogfx::font, size>> iStandardButtonWidth;
        mutable std::optional<dimension> iPenWidth;
        sink iSink;
        sink iSink2;
    public:
        define_property(property_category::appearance, optional_color, BorderColor, border_color)
        define_property(property_category::appearance, optional_color, OuterBorderColor, outer_border_color)
        define_property(property_category::appearance, optional_color, HoverColor, hover_color)
        define_property(property_category::other_appearance, std::optional<border_radii>, BorderRadius, border_radius)
    };

    extern template class basic_push_button<i_push_button>;

    typedef basic_push_button<i_push_button> push_button;
}
