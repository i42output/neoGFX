// scrollable_widget.hpp
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
#include "framed_widget.hpp"
#include "scrollbar.hpp"

namespace neogfx
{
	class scrollable_widget : public framed_widget, private i_scrollbar_container
	{
	public:
		enum child_widget_scrolling_disposition_e
		{
			DontScrollChildWidget = 0x00,
			ScrollChildWidgetVertically = 0x01,
			ScrollChildWidgetHorizontally = 0x02
		};
	protected:
		enum usv_stage_e
		{
			UsvStageInit,
			UsvStageCheckVertical1,
			UsvStageCheckHorizontal,
			UsvStageCheckVertical2,
			UsvStageDone
		};
	public:
		scrollable_widget(bool aHasFrame = true);
		scrollable_widget(const scrollable_widget&) = delete;
		scrollable_widget(i_widget& aParent, bool aHasFrame = true);
		scrollable_widget(i_layout& aLayout, bool aHasFrame = true);
		~scrollable_widget();
	public:
		virtual const i_surface& surface() const;
		virtual void layout_items_completed();
	public:
		virtual void resized();
		virtual rect client_rect(bool aIncludeMargins = true) const;
	public:
		virtual void paint_non_client(graphics_context& aGraphicsContext) const;
	public:
		virtual void mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta);
		virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition);
		virtual void mouse_button_double_clicked(mouse_button aButton, const point& aPosition);
		virtual void mouse_button_released(mouse_button aButton, const point& aPosition);
		virtual void mouse_moved(const point& aPosition);
		virtual void mouse_entered();
		virtual void mouse_left();
	public:
		virtual void key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
	public:
		virtual const i_scrollbar& vertical_scrollbar() const;
		virtual i_scrollbar& vertical_scrollbar();
		virtual const i_scrollbar& horizontal_scrollbar() const;
		virtual i_scrollbar& horizontal_scrollbar();
	protected:
		virtual void init();
		virtual child_widget_scrolling_disposition_e scrolling_disposition() const;
		virtual child_widget_scrolling_disposition_e scrolling_disposition(const i_widget& aChildWidget) const;
	private:
		virtual rect scrollbar_geometry(const i_units_context& aContext, const i_scrollbar& aScrollbar) const;
		virtual void scrollbar_updated(const i_scrollbar& aScrollbar, i_scrollbar::update_reason_e aReason);
		virtual colour scrollbar_colour(const i_scrollbar& aScrollbar) const;
	protected:
		virtual void update_scrollbar_visibility();
		virtual void update_scrollbar_visibility(usv_stage_e aStage);
	private:
		scrollbar iVerticalScrollbar;
		scrollbar iHorizontalScrollbar;
		point iOldScrollPosition;
		std::pair<i_scrollbar::value_type, i_scrollbar::value_type> iOldScrollbarValues;
		uint32_t iIgnoreScrollbarUpdates;
	};
}