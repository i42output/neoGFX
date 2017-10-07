// font_dialog.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/gui/dialog/dialog.hpp>

namespace neogfx
{
	class font_dialog : public dialog
	{
	public:
		event<> selection_changed;
	public:
		typedef std::array<std::pair<std::string, neogfx::font>, 16> custom_font_list;
	private:
		font_dialog(const neogfx::font& aCurrentFont = neogfx::font{});
		font_dialog(i_widget& aParent, const neogfx::font& aCurrentFont = neogfx::font{});
		~font_dialog();
	public:
		neogfx::font current_font() const;
		neogfx::font selected_font() const;
		void select_font(const neogfx::font& aFont);
		const custom_font_list& custom_fonts() const;
		custom_font_list& custom_fonts();
	private:
		void init();
		void select_font(const neogfx::font& aFont, const i_widget& aUpdatingWidget);
		custom_font_list::iterator current_custom_font() const;
		void set_current_custom_font(custom_font_list::iterator aCustomFont);
		void update_widgets(const i_widget& aUpdatingWidget);
	private:
		neogfx::font iCurrentFont;
		neogfx::font iSelectedFont;
		custom_font_list iCustomFonts;
		custom_font_list::iterator iCurrentCustomFont;
		bool iUpdatingWidgets;
		vertical_layout iLayout;
		push_button iAddToCustomFonts;
	};
}