// color_dialog.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present Leigh Johnston
  
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
#include <neogfx/core/color.hpp>
#include <neogfx/gui/dialog/colour_dialog.hpp>

namespace neogfx
{
	/// The pesky US spelling variant
	class color_dialog : public colour_dialog
	{
	public:
		typedef custom_colour_list custom_color_list;
	public:
		color_dialog(const color& aCurrentColor = color::Black) : colour_dialog(aCurrentColor) {}
		color_dialog(i_widget& aParent, const color& aCurrentColor = color::Black) : colour_dialog(aParent, aCurrentColor) {}
	public:
		color current_color() const { return current_colour(); }
		color selected_color() const { return selected_colour(); }
		hsv_color selected_color_as_hsv() const { return selected_colour_as_hsv(); }
		void select_color(const color& aColor) { select_colour(aColor); }
		const custom_color_list& custom_colors() const { return custom_colours(); }
		custom_color_list& custom_colors() { return custom_colours(); }
	};
}