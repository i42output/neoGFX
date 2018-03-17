// popup_menu.hpp
/*
neogfx C++ GUI Library
Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.

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
#include "window.hpp"
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/widget/i_menu.hpp>

namespace neogfx
{
	class popup_menu : public window
	{
	public:
		static const window_style DEFAULT_STYLE = window_style::NoDecoration | window_style::NoActivate | window_style::RequiresOwnerFocus | window_style::HideOnOwnerClick | window_style::InitiallyHidden | window_style::DropShadow;
	public:
		struct no_menu : std::logic_error { no_menu() : std::logic_error("neogfx::popup_menu::no_menu") {} };
	public:
		popup_menu(const point& aPosition, i_menu& aMenu, window_style aStyle = DEFAULT_STYLE);
		popup_menu(i_widget& aParent, const point& aPosition, i_menu& aMenu, window_style aStyle = DEFAULT_STYLE);
		popup_menu(const point& aPosition, window_style aStyle = DEFAULT_STYLE);
		popup_menu(i_widget& aParent, const point& aPosition, window_style aStyle = DEFAULT_STYLE);
		~popup_menu();
	public:
		bool has_menu() const;
		i_menu& menu() const;
		void set_menu(i_menu& aMenu, const point& aPosition = point{});
		void clear_menu();
	public:
		void resized() override;
	public:
		bool can_dismiss(const i_widget* aClickedWidget) const override;
		dismissal_type_e dismissal_type() const override;
		bool dismissed() const override;
		void dismiss() override;
	public:
		bool has_rendering_priority() const override;
	public:
		neogfx::size_policy size_policy() const override;
		size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
		size maximum_size(const optional_size& aAvailableSpace = optional_size()) const override;
	public:
		colour background_colour() const override;
		colour frame_colour() const override;
	public:
		bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
		bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
		bool text_input(const std::string& aText) override;
	private:
		void init();
		void close_sub_menu();
		void update_position();
	private:
		sink iSink;
		sink iSink2;
		i_widget* iParentWidget;
		i_menu* iMenu;
		vertical_layout iLayout;
		std::unique_ptr<popup_menu> iOpenSubMenu;
		bool iOpeningSubMenu;
	};
}