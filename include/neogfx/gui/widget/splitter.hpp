// spliter.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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
#include "widget.hpp"

namespace neogfx
{
	class splitter : public widget
	{
	public:
		enum type_e
		{
			HorizontalSplitter,
			VerticalSplitter
		};
	private:
		typedef std::pair<uint32_t, uint32_t> separator_type;
	public:
		splitter(type_e aType = HorizontalSplitter);
		splitter(i_widget& aParent, type_e aType = HorizontalSplitter);
		splitter(i_layout& aLayout, type_e aType = HorizontalSplitter);
		~splitter();
	public:
		virtual i_widget& widget_at(const point& aPosition);
	public:
		virtual neogfx::size_policy size_policy() const;
	public:
		virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
		virtual void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
		virtual void mouse_moved(const point& aPosition);
		virtual void mouse_entered();
		virtual void mouse_left();
		virtual neogfx::mouse_cursor mouse_cursor() const;
		virtual void released();
	public:
		virtual void panes_resized();
		virtual void reset_pane_sizes_requested(const boost::optional<uint32_t>& aPane = boost::optional<uint32_t>());
	private:
		boost::optional<separator_type> separator_at(const point& aPosition) const;
	private:
		type_e iType;
		boost::optional<separator_type> iTracking;
		std::pair<dimension, dimension> iSizeBeforeTracking;
		point iTrackFrom;
	};
}