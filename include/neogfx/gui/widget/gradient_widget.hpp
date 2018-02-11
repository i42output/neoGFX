// gradient_widget.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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
#include <neolib/variant.hpp>
#include "widget.hpp"
#include <neogfx/app/i_resource.hpp>
#include <neogfx/core/colour.hpp>
#include <neogfx/gui/window/context_menu.hpp>

namespace neogfx
{
	class gradient_dialog;

	class gradient_widget : public widget
	{
	public:
		event<> gradient_changed;
	private:
		typedef neolib::variant<neogfx::gradient::colour_stop_list::const_iterator, neogfx::gradient::alpha_stop_list::const_iterator> stop_const_iterator;
		typedef neolib::variant<neogfx::gradient::colour_stop_list::iterator, neogfx::gradient::alpha_stop_list::iterator> stop_iterator;
	public:
		gradient_widget(const neogfx::gradient& aGradient = neogfx::gradient{});
		gradient_widget(i_widget& aParent, const neogfx::gradient& aGradient = neogfx::gradient{});
		gradient_widget(i_layout& aLayout, const neogfx::gradient& aGradient = neogfx::gradient{});
		gradient_widget(gradient_dialog& aParent, i_layout& aLayout, const neogfx::gradient& aGradient = neogfx::gradient{});
	public:
		const neogfx::gradient& gradient() const;
		void set_gradient(const neogfx::gradient& aGradient);
	public:
		virtual neogfx::size_policy size_policy() const;
		virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
	public:
		virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
		virtual void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
		virtual void mouse_button_released(mouse_button aButton, const point& aPosition);
		virtual void mouse_moved(const point& aPosition);
		virtual neogfx::mouse_cursor mouse_cursor() const;
	private:
		rect contents_rect() const;
		stop_const_iterator stop_at(const point& aPosition) const;
		stop_iterator stop_at(const point& aPosition);
		rect colour_stop_rect(const neogfx::gradient::colour_stop& aColourStop) const;
		rect alpha_stop_rect(const neogfx::gradient::alpha_stop& aAlphaStop) const;
		void draw_colour_stop(graphics_context& aGraphicsContext, const neogfx::gradient::colour_stop& aColourStop) const;
		void draw_alpha_stop(graphics_context& aGraphicsContext, const neogfx::gradient::alpha_stop& aAlphaStop) const;
	private:
		bool iInGradientDialog;
		neogfx::gradient iSelection;
		boost::optional<point> iClicked;
		boost::optional<gradient::colour_stop_list::iterator> iCurrentColourStop;
		boost::optional<gradient::alpha_stop_list::iterator> iCurrentAlphaStop;
		bool iTracking;
		std::unique_ptr<context_menu> iMenu;
		mutable std::map<i_resource::hash_digest_type, texture> iStopTextures;
	};
}