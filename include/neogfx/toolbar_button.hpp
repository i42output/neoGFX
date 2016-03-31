// toolbar_button.hpp
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
#include "push_button.hpp"
#include "i_action.hpp"

namespace neogfx
{
	class toolbar_button : public push_button
	{
	public:
		toolbar_button(i_action& aAction);
		toolbar_button(i_widget& aParent, i_action& aAction);
		toolbar_button(i_layout& aLayout, i_action& aAction);
		~toolbar_button();
	protected:
		virtual void layout_items_completed();
	public:
		virtual neogfx::size_policy size_policy() const;
		virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
		virtual size maximum_size(const optional_size& aAvailableSpace = optional_size()) const;
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
	public:
		virtual colour foreground_colour() const;
	public:
		virtual neogfx::focus_policy focus_policy() const;
	public:
		virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition);
		virtual void mouse_button_released(mouse_button aButton, const point& aPosition);
	protected:
		virtual void handle_pressed();
	private:
		void init();
	private:
		i_action& iAction;
	};
}