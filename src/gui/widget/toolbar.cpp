// toolbar.cpp
/*
neogfx C++ GUI Library
Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.

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
#include <neogfx/gui/widget/toolbar.hpp>

namespace neogfx
{
	toolbar::toolbar() : 
		iLayout{ *this }
	{
	}

	toolbar::toolbar(i_widget& aParent) : 
		widget{ aParent }, iLayout{ *this }
	{
	}

	toolbar::toolbar(i_layout& aLayout) : 
		widget{ aLayout }, iLayout{ *this }
	{
	}

	neogfx::size_policy toolbar::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		return neogfx::size_policy::Minimum;
	}

	size toolbar::button_image_extents() const
	{
		if (iButtonImageExtents != std::nullopt)
			return *iButtonImageExtents;
		return dpi_scale(iButtonSize);
	}

	void toolbar::set_button_image_extents(const optional_size& aExtents)
	{
		iButtonImageExtents = aExtents;
	}

	uint32_t toolbar::button_count() const
	{
		return iButtons.size();
	}

	const toolbar_button& toolbar::button(button_index aButtonIndex) const
	{
		if (aButtonIndex >= button_count())
			throw bad_button_index();
		return *iButtons[aButtonIndex];
	}

	toolbar_button& toolbar::button(button_index aButtonIndex)
	{
		return const_cast<toolbar_button&>(const_cast<const toolbar*>(this)->button(aButtonIndex));
	}

    i_action& toolbar::add_action(i_action& aAction)
	{
		insert_action(button_count(), aAction);
        return aAction;
	}

	void toolbar::add_separator()
	{
		insert_separator(button_count());
	}

    i_action& toolbar::insert_action(button_index aButtonIndex, i_action& aAction)
	{
		if (aButtonIndex > iButtons.size())
			throw bad_button_index();
		iButtons.insert(iButtons.begin() + aButtonIndex, std::make_unique<toolbar_button>(layout(), aAction));
		iButtons[aButtonIndex]->image().set_fixed_size(button_image_extents());
        return aAction;
	}

	void toolbar::insert_separator(button_index aButtonIndex)
	{
		if (aButtonIndex > iButtons.size())
			throw bad_button_index();
		iButtons.insert(iButtons.begin() + aButtonIndex, std::make_unique<toolbar_button>(layout(), iSeparator));
	}
}