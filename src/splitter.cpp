// spliter.cpp
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

#include "neogfx.hpp"
#include "app.hpp"
#include "splitter.hpp"
#include "horizontal_layout.hpp"
#include "vertical_layout.hpp"
#include "i_surface.hpp"

namespace neogfx
{
	splitter::splitter(type_e aType) :
		iType(aType)
	{
		set_margins(neogfx::margins(0.0));
		if (iType == HorizontalSplitter)
			set_layout(std::make_shared<horizontal_layout>(*this));
		else
			set_layout(std::make_shared<vertical_layout>(*this));
		layout().set_margins(neogfx::margins(0.0));
	}

	splitter::splitter(i_widget& aParent, type_e aType) : 
		widget(aParent), 
		iType(aType)
	{
		set_margins(neogfx::margins(0.0));
		if (iType == HorizontalSplitter)
			set_layout(std::make_shared<horizontal_layout>(*this));
		else
			set_layout(std::make_shared<vertical_layout>(*this));
		layout().set_margins(neogfx::margins(0.0));
	}

	splitter::splitter(i_layout& aLayout, type_e aType) :
		widget(aLayout), 
		iType(aType)
	{
		set_margins(neogfx::margins(0.0));
		if (iType == HorizontalSplitter)
			set_layout(std::make_shared<horizontal_layout>(*this));
		else
			set_layout(std::make_shared<vertical_layout>(*this));
		layout().set_margins(neogfx::margins(0.0));
	}

	splitter::~splitter()
	{
	}

	i_widget& splitter::widget_at(const point& aPosition)
	{
		auto s = separator_at(aPosition);
		if (s != boost::none)
			return *this;
		return widget::widget_at(aPosition);
	}

	void splitter::mouse_button_pressed(mouse_button aButton, const point& aPosition)
	{
		widget::mouse_button_pressed(aButton, aPosition);
		if (aButton == mouse_button::Left)
		{
			auto s = separator_at(aPosition);
			if (s != boost::none)
			{
				iTracking = s;
				iTrackFrom = aPosition;
				iSizeBeforeTracking = std::make_pair(
					layout().get_widget(iTracking->first).minimum_size().cx, 
					layout().get_widget(iTracking->second).minimum_size().cx);
				surface().set_mouse_cursor(iType == HorizontalSplitter ? mouse_system_cursor::SizeWE : mouse_system_cursor::SizeNS);
			}
		}
	}

	void splitter::mouse_button_double_clicked(mouse_button aButton, const point& aPosition)
	{
		widget::mouse_button_double_clicked(aButton, aPosition);
		if (aButton == mouse_button::Left)
		{
			auto s = separator_at(aPosition);
			if (s != boost::none)
			{
				if (app::instance().keyboard().is_key_pressed(ScanCode_LSHIFT) || app::instance().keyboard().is_key_pressed(ScanCode_RSHIFT))
					reset_pane_sizes_requested();
				else
					reset_pane_sizes_requested(s->first);
				layout_items();
			}
		}
	}

	void splitter::mouse_moved(const point& aPosition)
	{
		auto s = separator_at(aPosition);
		if (s != boost::none || iTracking != boost::none)
			surface().set_mouse_cursor(iType == HorizontalSplitter ? mouse_system_cursor::SizeWE : mouse_system_cursor::SizeNS);
		else
			surface().set_mouse_cursor(mouse_system_cursor::Arrow);
		if (iTracking != boost::none)
		{
			if (iType == HorizontalSplitter)
			{

				layout().get_widget(iTracking->first).set_fixed_size(size(
					std::max(iSizeBeforeTracking.first + (aPosition.x - iTrackFrom.x), layout().spacing().cx * 3.0),
					layout().get_widget(iTracking->first).minimum_size().cy), false);
				if (app::instance().keyboard().is_key_pressed(ScanCode_LSHIFT) || app::instance().keyboard().is_key_pressed(ScanCode_RSHIFT))
				{
					layout().get_widget(iTracking->second).set_fixed_size(size(
						std::max(iSizeBeforeTracking.second - (aPosition.x - iTrackFrom.x), layout().spacing().cx * 3.0),
						layout().get_widget(iTracking->second).minimum_size().cy), false);
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

	void splitter::mouse_entered()
	{
		surface().save_mouse_cursor();
		auto s = separator_at(surface().mouse_position() - origin());
		if (s != boost::none || iTracking != boost::none)
			surface().set_mouse_cursor(iType == HorizontalSplitter ? mouse_system_cursor::SizeWE : mouse_system_cursor::SizeNS);
		else
			surface().set_mouse_cursor(mouse_system_cursor::Arrow);
	}

	void splitter::mouse_left()
	{
		surface().restore_mouse_cursor();
	}

	void splitter::released()
	{
		iTracking = boost::none;
	}

	void splitter::panes_resized()
	{
	}

	void splitter::reset_pane_sizes_requested(const boost::optional<uint32_t>& aPane)
	{
	}

	boost::optional<std::pair<std::size_t, std::size_t>> splitter::separator_at(const point& aPosition)
	{
		for (std::size_t i = 1; i < layout().item_count(); ++i)
		{
			rect r1(layout().get_widget(i - 1).position(), layout().get_widget(i - 1).extents());
			rect r2(layout().get_widget(i).position(), layout().get_widget(i).extents());
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
		return boost::optional<separator_type>();
	}
}