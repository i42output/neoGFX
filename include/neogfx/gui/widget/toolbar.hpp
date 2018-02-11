// toolbar.hpp
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
#include <neogfx/app/action.hpp>
#include "widget.hpp"
#include "toolbar_button.hpp"

namespace neogfx
{
	class toolbar : public widget
	{
	private:
		typedef std::unique_ptr<toolbar_button> button_pointer;
		typedef std::vector<button_pointer> button_list;
	public:
		typedef uint32_t button_index;
	public:
		struct bad_button_index : std::logic_error { bad_button_index() : std::logic_error("neogfx::toolbar::bad_button_index") {} };
	public:
		toolbar();
		toolbar(i_widget& aParent);
		toolbar(i_layout& aLayout);
	public:
		neogfx::size_policy size_policy() const override;
	public:
		virtual size button_image_extents() const;
		virtual void set_button_image_extents(const optional_size& aExtents);
		virtual uint32_t button_count() const;
		virtual const toolbar_button& button(button_index aButtonIndex) const;
		virtual toolbar_button& button(button_index aButtonIndex);
		virtual void add_action(i_action& aAction);
		virtual void add_separator();
		virtual void insert_action(button_index aButtonIndex, i_action& aAction);
		virtual void insert_separator(button_index aButtonIndex);
	private:
		button_list iButtons;
		action iSeparator;
		optional_size iButtonImageExtents;
	};
}