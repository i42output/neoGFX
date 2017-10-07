// font_dialog.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/dialog/font_dialog.hpp>

namespace neogfx
{
	font_dialog::font_dialog(const neogfx::font& aCurrentFont) :
		dialog{ "Select Font", window_style::Modal | window_style::TitleBar | window_style::Close },
		iCurrentFont{ aCurrentFont },
		iSelectedFont{ aCurrentFont },
		iCurrentCustomFont{ iCustomFonts.end() },
		iUpdatingWidgets{ false },
		iAddToCustomFonts{ iLayout, "&Add to Custom Fonts" }
	{
		init();
	}

	font_dialog::font_dialog(i_widget& aParent, const neogfx::font& aCurrentFont) :
		dialog{ aParent, "Select Font", window_style::Modal | window_style::TitleBar | window_style::Close },
		iCurrentFont{ aCurrentFont },
		iSelectedFont{ aCurrentFont },
		iCurrentCustomFont{ iCustomFonts.end() },
		iUpdatingWidgets{ false },
		iAddToCustomFonts{ iLayout, "&Add to Custom Fonts" }
	{
		init();
	}

	font_dialog::~font_dialog()
	{
	}

	font font_dialog::current_font() const
	{
		return iCurrentFont;
	}

	font font_dialog::selected_font() const
	{
		return iSelectedFont;
	}

	void font_dialog::select_font(const neogfx::font& aFont)
	{
		select_font(aFont, *this);
	}

	const font_dialog::custom_font_list& font_dialog::custom_fonts() const
	{
		return iCustomFonts;
	}

	font_dialog::custom_font_list& font_dialog::custom_fonts()
	{
		return iCustomFonts;
	}

	void font_dialog::init()
	{
		scoped_units su(static_cast<framed_widget&>(*this), units::Pixels);
		set_margins(neogfx::margins{16.0});
		iLayout.set_margins(neogfx::margins{});
		iLayout.set_spacing(16.0);
		update_widgets(*this);
	}

	void font_dialog::select_font(const neogfx::font& aFont, const i_widget& aUpdatingWidget)
	{
		if (iUpdatingWidgets)
			return;
		if (iSelectedFont != aFont)
		{
			iSelectedFont = aFont;
			update_widgets(aUpdatingWidget);
			selection_changed.trigger();
		}
	}

	font_dialog::custom_font_list::iterator font_dialog::current_custom_font() const
	{
		return iCurrentCustomFont;
	}

	void font_dialog::set_current_custom_font(custom_font_list::iterator aCustomFont)
	{
		iCurrentCustomFont = aCustomFont;
		update_widgets(*this);
		update();
	}

	void font_dialog::update_widgets(const i_widget& aUpdatingWidget)
	{
		if (iUpdatingWidgets)
			return;
		iUpdatingWidgets = true;
		iUpdatingWidgets = false;
	}
}