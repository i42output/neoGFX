// gradient_widget.hpp
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
#include <neolib/core/variant.hpp>
#include <neogfx/app/i_resource.hpp>
#include <neogfx/gfx/color.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/window/context_menu.hpp>
#include <neogfx/gui/dialog/color_dialog.hpp>

namespace neogfx
{
    class gradient_dialog;

    class gradient_widget : public widget
    {
    public:
        define_event(GradientChanged, gradient_changed)
        define_event(ColorStopSelected, color_stop_selected)
        define_event(ColorStopDeselected, color_stop_deselected)
        define_event(AlphaStopSelected, alpha_stop_selected)
        define_event(AlphaStopDeselected, alpha_stop_deselected)
    private:
        typedef neolib::variant<neogfx::gradient::color_stop_list::const_iterator, neogfx::gradient::alpha_stop_list::const_iterator> stop_const_iterator;
        typedef neolib::variant<neogfx::gradient::color_stop_list::iterator, neogfx::gradient::alpha_stop_list::iterator> stop_iterator;
    public:
        gradient_widget(i_gradient const& aGradient = neogfx::gradient{});
        gradient_widget(i_widget& aParent, i_gradient const& aGradient = neogfx::gradient{});
        gradient_widget(i_layout& aLayout, i_gradient const& aGradient = neogfx::gradient{});
        gradient_widget(gradient_dialog& aParent, i_layout& aLayout, i_gradient const& aGradient = neogfx::gradient{});
    public:
        i_gradient const& gradient() const;
        void set_gradient(i_gradient const& aGradient);
        void set_gradient(i_ref_ptr<i_gradient> const& aGradient);
        const std::optional<color_dialog::custom_color_list>& custom_colors() const;
        std::optional<color_dialog::custom_color_list>& custom_colors();
    public:
        gradient::color_stop_list::const_iterator selected_color_stop() const;
        gradient::alpha_stop_list::const_iterator selected_alpha_stop() const;
    public:
        neogfx::size_policy size_policy() const override;
        size minimum_size(optional_size const& aAvailableSpace = optional_size()) const override;
    public:
        void paint(i_graphics_context& aGc) const override;
    public:
        void mouse_button_pressed(mouse_button aButton, point const& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_double_clicked(mouse_button aButton, point const& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_released(mouse_button aButton, point const& aPosition) override;
        void mouse_moved(point const& aPosition, key_modifiers_e aKeyModifiers) override;
        neogfx::mouse_cursor mouse_cursor() const override;
    private:
        rect contents_rect() const;
        bool is_stop_at(point const& aPosition) const;
        stop_const_iterator stop_at(point const& aPosition) const;
        stop_iterator stop_at(point const& aPosition);
        void set_current_color_stop(std::optional<gradient::color_stop_list::iterator> const& aStop);
        void set_current_alpha_stop(std::optional<gradient::alpha_stop_list::iterator> const& aStop);
        rect color_stop_rect(neogfx::gradient::color_stop const& aColorStop) const;
        rect alpha_stop_rect(neogfx::gradient::alpha_stop const& aAlphaStop) const;
        void draw_color_stop(i_graphics_context& aGc, neogfx::gradient::color_stop const& aColorStop) const;
        void draw_alpha_stop(i_graphics_context& aGc, neogfx::gradient::alpha_stop const& aAlphaStop) const;
    private:
        dimension stop_width() const;
        dimension stop_height() const;
        dimension control_height() const;
        dimension alpha_pattern_size() const;
        dimension small_alpha_pattern_size() const;
        dimension border_thickness() const;
        dimension border_spacer_thickness() const;
    private:
        bool iInGradientDialog;
        unique_gradient iSelection;
        std::optional<point> iClicked;
        std::optional<gradient::color_stop_list::iterator> iCurrentColorStop;
        std::optional<gradient::alpha_stop_list::iterator> iCurrentAlphaStop;
        bool iTracking;
        std::unique_ptr<context_menu> iMenu;
        mutable std::map<i_resource::hash_digest_type, texture> iStopTextures;
        std::optional<color_dialog::custom_color_list> iCustomColors;
    };
}