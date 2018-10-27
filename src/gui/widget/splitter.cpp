// spliter.cpp
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
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/splitter.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/hid/i_surface.hpp>

namespace neogfx
{
	splitter::splitter(type_e aType) :
		iType(aType)
	{
		set_margins(neogfx::margins(0.0));
		if (iType == HorizontalSplitter)
			set_layout(std::make_shared<horizontal_layout>());
		else
			set_layout(std::make_shared<vertical_layout>());
		layout().set_margins(neogfx::margins(0.0));
	}

	splitter::splitter(i_widget& aParent, type_e aType) : 
		widget(aParent), 
		iType(aType)
	{
		set_margins(neogfx::margins(0.0));
		if (iType == HorizontalSplitter)
			set_layout(std::make_shared<horizontal_layout>());
		else
			set_layout(std::make_shared<vertical_layout>());
		layout().set_margins(neogfx::margins(0.0));
	}

	splitter::splitter(i_layout& aLayout, type_e aType) :
		widget(aLayout), 
		iType(aType)
	{
		set_margins(neogfx::margins(0.0));
		if (iType == HorizontalSplitter)
			set_layout(std::make_shared<horizontal_layout>());
		else
			set_layout(std::make_shared<vertical_layout>());
		layout().set_margins(neogfx::margins(0.0));
	}

	splitter::~splitter()
	{
	}

	i_widget& splitter::get_widget_at(const point& aPosition)
	{
		auto s = separator_at(aPosition);
		if (s != std::nullopt)
			return *this;
		return widget::get_widget_at(aPosition);
	}

	neogfx::size_policy splitter::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		if (iType == HorizontalSplitter)
			return neogfx::size_policy{neogfx::size_policy::Expanding, neogfx::size_policy::Minimum};
		else
			return neogfx::size_policy{neogfx::size_policy::Minimum, neogfx::size_policy::Expanding};
	}

	void splitter::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left)
		{
			auto s = separator_at(aPosition);
			if (s != std::nullopt)
			{
				iTracking = s;
				iTrackFrom = aPosition;
				iSizeBeforeTracking = std::make_pair(
					layout().get_widget_at(iTracking->first).minimum_size().cx, 
					layout().get_widget_at(iTracking->second).minimum_size().cx);
				if (has_root())
					root().window_manager().update_mouse_cursor(root());
			}
		}
	}

	void splitter::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		widget::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left)
		{
			auto s = separator_at(aPosition);
			if (s != std::nullopt)
			{
				if ((aKeyModifiers & KeyModifier_SHIFT) != KeyModifier_NONE)
					reset_pane_sizes_requested();
				else
					reset_pane_sizes_requested(s->first);
				layout_items();
			}
		}
	}

	void splitter::mouse_moved(const point& aPosition)
	{
		if (iTracking != std::nullopt)
		{
			if (iType == HorizontalSplitter)
			{

				layout().get_widget_at(iTracking->first).set_minimum_size(size(
					std::max(iSizeBeforeTracking.first + (aPosition.x - iTrackFrom.x), layout().spacing().cx * 3.0),
					layout().get_widget_at(iTracking->first).minimum_size().cy), false);
				if (service<i_keyboard>::instance().is_key_pressed(ScanCode_LSHIFT) || service<i_keyboard>::instance().is_key_pressed(ScanCode_RSHIFT))
				{
					layout().get_widget_at(iTracking->second).set_minimum_size(size(
						std::max(iSizeBeforeTracking.second - (aPosition.x - iTrackFrom.x), layout().spacing().cx * 3.0),
						layout().get_widget_at(iTracking->second).minimum_size().cy), false);
				}
				layout_items();
				panes_resized();
			}
			else
			{
				// todo
			}
		}
	}

	void splitter::mouse_entered(const point& aPosition)
	{
	}

	void splitter::mouse_left()
	{
	}
	
	neogfx::mouse_cursor splitter::mouse_cursor() const
	{
		auto s = separator_at(root().mouse_position() - origin());
		if (s != std::nullopt || iTracking != std::nullopt)
			return iType == HorizontalSplitter ? mouse_system_cursor::SizeWE : mouse_system_cursor::SizeNS;
		else
			return widget::mouse_cursor();
	}

	void splitter::released()
	{
		iTracking = std::nullopt;
	}

	void splitter::panes_resized()
	{
	}

	void splitter::reset_pane_sizes_requested(const std::optional<uint32_t>&)
	{
	}

	std::optional<std::pair<std::size_t, std::size_t>> splitter::separator_at(const point& aPosition) const
	{
		for (std::size_t i = 1; i < layout().count(); ++i)
		{
			rect r1(layout().get_widget_at(i - 1).position(), layout().get_widget_at(i - 1).extents());
			rect r2(layout().get_widget_at(i).position(), layout().get_widget_at(i).extents());
			if (iType == HorizontalSplitter)
			{
				rect r3(point(r1.right(), r1.top()), size(r2.left() - r1.right(), r1.height()));
				rect r4 = r3;
				r4.x -= r3.width();
				r4.cx *= 3.0;
				if (r4.contains(aPosition))
					return separator_type(i - 1, i);
			}
			else
			{
				rect r3(point(r1.left(), r1.bottom()), size(r2.width(), r2.top() - r1.bottom()));
				rect r4 = r3;
				r4.y -= r3.height();
				r4.cy *= 3.0;
				if (r4.contains(aPosition))
					return separator_type(i - 1, i);
			}
		}
		return std::optional<separator_type>();
	}
}