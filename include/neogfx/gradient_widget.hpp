// gradient_widget.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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

#include "neogfx.hpp"
#include <neolib/variant.hpp>
#include "widget.hpp"
#include "colour.hpp"

namespace neogfx
{
	class gradient_widget : public widget
	{
	public:
		event<> gradient_changed;
	private:
		typedef neolib::variant<gradient::colour_stop_list::const_iterator, gradient::alpha_stop_list::const_iterator> stop_const_iterator;
		typedef neolib::variant<gradient::colour_stop_list::iterator, gradient::alpha_stop_list::iterator> stop_iterator;
	public:
		gradient_widget();
		gradient_widget(i_widget& aParent);
		gradient_widget(i_layout& aLayout);
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
	private:
		rect contents_rect() const;
		stop_const_iterator stop_at(const point& aPosition) const;
		stop_iterator stop_at(const point& aPosition);
		rect colour_stop_rect(const gradient::colour_stop& aColourStop) const;
		rect alpha_stop_rect(const gradient::alpha_stop& aAlphaStop) const;
		void draw_colour_stop(graphics_context& aGraphicsContext, const gradient::colour_stop& aColourStop) const;
		void draw_alpha_stop(graphics_context& aGraphicsContext, const gradient::alpha_stop& aAlphaStop) const;
	private:
		gradient iSelection;
		boost::optional<std::size_t> iCurrentColourStop;
		boost::optional<std::size_t> iCurrentAlphaStop;
		bool iTracking;
	};
}