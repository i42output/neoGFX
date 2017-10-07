// title_bar.cpp
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
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/title_bar.hpp>

namespace neogfx
{
	title_bar::title_bar(i_widget& aParent, const std::string& aTitle) :
		widget{ aParent },
		iLayout{ *this },
		iIcon{ iLayout, app::instance().default_window_icon() },
		iTitle{ iLayout, aTitle },
		iSpacer{ iLayout },
		iMinimizeButton{ iLayout, "Mi", push_button_style::TitleBar },
		iMaximizeButton{ iLayout, "Ma", push_button_style::TitleBar },
		iRestoreButton{ iLayout, "Re", push_button_style::TitleBar },
		iCloseButton{ iLayout, "Cl", push_button_style::TitleBar }
	{
		init();
	}

	title_bar::title_bar(i_widget& aParent, const i_texture& aIcon, const std::string& aTitle) :
		widget{ aParent },
		iLayout{ *this },
		iIcon{ iLayout, aIcon },
		iTitle{ iLayout, aTitle },
		iSpacer{ iLayout },
		iMinimizeButton{ iLayout, "Mi", push_button_style::TitleBar },
		iMaximizeButton{ iLayout, "Ma", push_button_style::TitleBar },
		iRestoreButton{ iLayout, "Re", push_button_style::TitleBar },
		iCloseButton{ iLayout, "Cl", push_button_style::TitleBar }
	{
		init();
	}

	title_bar::title_bar(i_widget& aParent, const i_image& aIcon, const std::string& aTitle) :
		widget{ aParent },
		iLayout{ *this },
		iIcon{ iLayout, aIcon },
		iTitle{ iLayout, aTitle },
		iSpacer{ iLayout },
		iMinimizeButton{ iLayout, "Mi", push_button_style::TitleBar },
		iMaximizeButton{ iLayout, "Ma", push_button_style::TitleBar },
		iRestoreButton{ iLayout, "Re", push_button_style::TitleBar },
		iCloseButton{ iLayout, "Cl", push_button_style::TitleBar }
	{
		init();
	}

	title_bar::title_bar(i_layout& aLayout, const std::string& aTitle) :
		widget{ aLayout },
		iLayout{ *this },
		iIcon{ iLayout, app::instance().default_window_icon() },
		iTitle{ iLayout, aTitle },
		iSpacer{ iLayout },
		iMinimizeButton{ iLayout, "Mi", push_button_style::TitleBar },
		iMaximizeButton{ iLayout, "Ma", push_button_style::TitleBar },
		iRestoreButton{ iLayout, "Re", push_button_style::TitleBar },
		iCloseButton{ iLayout, "Cl", push_button_style::TitleBar }
	{
		init();
	}

	title_bar::title_bar(i_layout& aLayout, const i_texture& aIcon, const std::string& aTitle) :
		widget{ aLayout },
		iLayout{ *this },
		iIcon{ iLayout, aIcon },
		iTitle{ iLayout, aTitle },
		iSpacer{ iLayout },
		iMinimizeButton{ iLayout, "Mi", push_button_style::TitleBar },
		iMaximizeButton{ iLayout, "Ma", push_button_style::TitleBar },
		iRestoreButton{ iLayout, "Re", push_button_style::TitleBar },
		iCloseButton{ iLayout, "Cl", push_button_style::TitleBar }
	{
		init();
	}

	title_bar::title_bar(i_layout& aLayout, const i_image& aIcon, const std::string& aTitle) :
		widget{ aLayout },
		iLayout{ *this },
		iIcon{ iLayout, aIcon },
		iTitle{ iLayout, aTitle },
		iSpacer{ iLayout },
		iMinimizeButton{ iLayout, "Mi", push_button_style::TitleBar },
		iMaximizeButton{ iLayout, "Ma", push_button_style::TitleBar },
		iRestoreButton{ iLayout, "Re", push_button_style::TitleBar },
		iCloseButton{ iLayout, "Cl", push_button_style::TitleBar }
	{
		init();
	}

	const image_widget& title_bar::icon() const
	{
		return iIcon;
	}

	image_widget& title_bar::icon()
	{
		return iIcon;
	}

	const text_widget& title_bar::title() const
	{
		return iTitle;
	}

	text_widget& title_bar::title()
	{
		return iTitle;
	}

	neogfx::size_policy title_bar::size_policy() const
	{
		return neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Minimum };
	}

	widget_part title_bar::hit_test(const point&) const
	{
		return widget_part::NonClientTitleBar;
	}

	void title_bar::init()
	{
		set_margins(neogfx::margins{});
		iLayout.set_margins(neogfx::margins{ 4.0, 4.0, 4.0, 4.0 });
		iLayout.set_spacing(8.0);
		icon().set_ignore_mouse_events(false);
		if (icon().image().is_empty())
			icon().set_fixed_size(size{ 24.0, 24.0 });
		else
			icon().set_fixed_size(size{ std::min(icon().image().extents().cx, 24.0), std::min(icon().image().extents().cy, 24.0) });
		iMinimizeButton.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Minimum });
		iMaximizeButton.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Minimum });
		iRestoreButton.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Minimum });
		iCloseButton.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Minimum });
	}
}