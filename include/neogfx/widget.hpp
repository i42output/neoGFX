// widget.hpp
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
#include <neolib/timer.hpp>
#include "i_widget.hpp"

namespace neogfx
{
	class widget : public i_widget
	{
	private:
		class device_metrics_forwarder : public i_device_metrics
		{
		public:
			device_metrics_forwarder(widget& aOwner);
		public:
			virtual size extents() const;
			virtual dimension horizontal_dpi() const;
			virtual dimension vertical_dpi() const;
			virtual dimension em_size() const;
		private:
			widget& iOwner;
		};
	public:
		widget();
		widget(const widget&) = delete;
		widget(i_widget& aParent);
		widget(i_layout& aLayout);
		~widget();
	public:
		virtual const i_device_metrics& device_metrics() const;
		virtual units_e units() const;
		virtual units_e set_units(units_e aUnits) const;
	public:
		virtual bool is_root() const;
		virtual bool has_parent() const;
		virtual void set_parent(i_widget& aParent);
		virtual const i_widget& parent() const;
		virtual i_widget& parent();
		virtual const i_widget& ultimate_ancestor() const;
		virtual i_widget& ultimate_ancestor();
		virtual bool is_ancestor(const i_widget& aWidget) const;
		virtual void add_widget(i_widget& aWidget);
		virtual void add_widget(std::shared_ptr<i_widget> aWidget);
		virtual void remove_widget(i_widget& aWidget);
		virtual const widget_list& children() const;
		virtual bool has_surface() const;
		virtual const i_surface& surface() const;
		virtual i_surface& surface();
		virtual bool has_layout() const;
		virtual void set_layout(i_layout& aLayout);
		virtual void set_layout(std::shared_ptr<i_layout> aLayout);
		virtual const i_layout& layout() const;
		virtual i_layout& layout();
		virtual bool can_defer_layout() const;
		virtual bool has_managing_layout() const;
		virtual const i_widget& managing_layout() const;
		virtual i_widget& managing_layout();
		virtual bool is_managing_layout() const;
		virtual void layout_items(bool aDefer = false);
		virtual void layout_items_completed();
	public:
		virtual point position() const;
		virtual point origin(bool aNonClient = false) const;
		virtual void move(const point& aPosition);
		virtual void moved();
		virtual size extents() const;
		virtual void resize(const size& aSize);
		virtual void resized();
		virtual rect window_rect() const;
		virtual rect client_rect(bool aIncludeMargins = true) const;
		virtual i_widget& widget_at(const point& aPosition);
		virtual bool has_minimum_size() const;
		virtual size minimum_size() const;
		virtual void set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout = true);
		virtual bool has_maximum_size() const;
		virtual size maximum_size() const;
		virtual void set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout = true);
		virtual bool is_fixed_size() const;
		virtual void set_fixed_size(const optional_size& aFixedSize, bool aUpdateLayout = true);
		virtual bool has_margins() const;
		virtual neogfx::margins margins() const;
		virtual void set_margins(const optional_margins& aMargins);
		virtual size size_hint() const;
	public:
		virtual void update(bool aIncludeNonClient = false);
		virtual void update(const rect& aUpdateRect);
		virtual bool requires_update() const;
		virtual rect update_rect() const;
		virtual rect default_clip_rect(bool aIncludeNonClient = false) const;
		virtual void render(graphics_context& aGraphicsContext) const;
		virtual bool transparent_background() const;
		virtual void paint_non_client(graphics_context& aGraphicsContext) const;
		virtual void paint(graphics_context& aGraphicsContext) const;
	public:
		virtual bool has_foreground_colour() const;
		virtual colour foreground_colour() const;
		virtual void set_foreground_colour(const optional_colour& aForegroundColour = optional_colour());
		virtual bool has_background_colour() const;
		virtual colour background_colour() const;
		virtual void set_background_colour(const optional_colour& aBackgroundColour = optional_colour());
		virtual bool has_font() const;
		virtual const neogfx::font& font() const;
		virtual void set_font(const optional_font& aFont);
	public:
		virtual bool visible() const;
		virtual bool hidden() const;
		virtual void show(bool aVisible);
		virtual void show();
		virtual void hide();
		virtual bool enabled() const;
		virtual bool disabled() const;
		virtual void enable(bool aEnable);
		virtual void enable();
		virtual void disable();
		virtual bool entered() const;
		virtual bool capturing() const;
		virtual void set_capture();
		virtual void release_capture();
		virtual void captured();
		virtual void released();
		virtual neogfx::focus_policy focus_policy() const;
		virtual void set_focus_policy(neogfx::focus_policy aFocusPolicy);
		virtual bool has_focus() const;
		virtual void set_focus();
		virtual void release_focus();
		virtual void focus_gained();
		virtual void focus_lost();
		virtual bool has_tab_before() const;
		virtual i_widget& tab_before();
		virtual void set_tab_before(i_widget& aWidget);
		virtual void unset_tab_before();
		virtual bool has_tab_after() const;
		virtual i_widget& tab_after();
		virtual void set_tab_after(i_widget& aWidget);
		virtual void unset_tab_after();
	public:
		virtual bool ignore_mouse_events() const;
		virtual void set_ignore_mouse_events(bool aIgnoreMouseEvents);
		virtual void mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta);
		virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition);
		virtual void mouse_button_double_clicked(mouse_button aButton, const point& aPosition);
		virtual void mouse_button_released(mouse_button aButton, const point& aPosition);
		virtual void mouse_moved(const point& aPosition);
		virtual void mouse_entered();
		virtual void mouse_left();
	public:
		virtual void key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual void key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual void text_input(const std::string& aText);
	public:
		virtual graphics_context create_graphics_context() const;
	protected:
		virtual i_widget& widget_for_mouse_event(const point& aPosition);
	private:
		i_widget* iParent;
		widget_list iChildren;
		std::shared_ptr<i_layout> iLayout;
		std::unique_ptr<neolib::callback_timer> iLayoutTimer;
		device_metrics_forwarder iDeviceMetricsForwarder;
		units_context iUnitsContext;
		point iPosition;
		size iSize;
		optional_margins iMargins;
		optional_size iMinimumSize;
		optional_size iMaximumSize;
		mutable std::vector<rect> iUpdateRects;
		bool iVisible;
		bool iEnabled;
		neogfx::focus_policy iFocusPolicy;
		i_widget* iTabBefore;
		i_widget* iTabAfter;
		optional_colour iForegroundColour;
		optional_colour iBackgroundColour;
		optional_font iFont;
		bool iIgnoreMouseEvents;
	};
}