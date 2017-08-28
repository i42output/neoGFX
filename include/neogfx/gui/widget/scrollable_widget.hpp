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

#include <neogfx/neogfx.hpp>
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
			ScrollChildWidgetHorizontally = 0x02,
			DontConsiderChildWidgets = 0x08
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
		scrollable_widget(scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::SolidFrame);
		scrollable_widget(i_widget& aParent, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::SolidFrame);
		scrollable_widget(i_layout& aLayout, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::SolidFrame);
		~scrollable_widget();
		scrollable_widget(const scrollable_widget&) = delete;
	public:
		void scroll_to(i_widget& aChild);
	public:
		void layout_items_started() override;
		void layout_items_completed() override;
	public:
		void resized() override;
		rect client_rect(bool aIncludeMargins = true) const override;
	public:
		void paint_non_client_after(graphics_context& aGraphicsContext) const override;
	public:
		void mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta) override;
		void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
		void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
		void mouse_button_released(mouse_button aButton, const point& aPosition) override;
		void mouse_moved(const point& aPosition) override;
		void mouse_entered() override;
		void mouse_left() override;
	public:
		bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
	public:
		virtual const i_scrollbar& vertical_scrollbar() const;
		virtual i_scrollbar& vertical_scrollbar();
		virtual const i_scrollbar& horizontal_scrollbar() const;
		virtual i_scrollbar& horizontal_scrollbar();
		virtual child_widget_scrolling_disposition_e scrolling_disposition() const;
		virtual child_widget_scrolling_disposition_e scrolling_disposition(const i_widget& aChildWidget) const;
	private:
		rect scrollbar_geometry(const i_units_context& aContext, const i_scrollbar& aScrollbar) const override;
		void scrollbar_updated(const i_scrollbar& aScrollbar, i_scrollbar::update_reason_e aReason) override;
		colour scrollbar_colour(const i_scrollbar& aScrollbar) const override;
		const i_widget& as_widget() const override;
	protected:
		virtual void update_scrollbar_visibility();
		virtual void update_scrollbar_visibility(usv_stage_e aStage);
	protected:
		void init();
	private:
		scrollbar iVerticalScrollbar;
		scrollbar iHorizontalScrollbar;
		point iOldScrollPosition;
		point iOldScrollbarPositions;
		uint32_t iIgnoreScrollbarUpdates;
	};
}