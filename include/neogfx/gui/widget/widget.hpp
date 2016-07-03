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

#include <neogfx/neogfx.hpp>
#include <unordered_set>
#include <neolib/destroyable.hpp>
#include <neolib/timer.hpp>
#include "i_widget.hpp"

namespace neogfx
{
	class widget : public i_widget, protected neolib::destroyable
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
		virtual const i_widget& parent() const;
		virtual i_widget& parent();
		virtual void set_parent(i_widget& aParent);
		virtual void parent_changed();
		virtual const i_widget& ultimate_ancestor(bool aSameSurface = true) const;
		virtual i_widget& ultimate_ancestor(bool aSameSurface = true);
		virtual bool is_ancestor_of(const i_widget& aWidget, bool aSameSurface = true) const;
		virtual bool is_descendent_of(const i_widget& aWidget, bool aSameSurface = true) const;
		virtual bool is_sibling_of(const i_widget& aWidget) const;
		virtual i_widget& link_before() const;
		virtual void set_link_before(i_widget& aWidget);
		virtual void set_link_before_ptr(i_widget& aWidget);
		virtual i_widget& link_after() const;
		virtual void set_link_after(i_widget& aWidget);
		virtual void set_link_after_ptr(i_widget& aWidget);
		virtual void unlink();
		virtual void add_widget(i_widget& aWidget);
		virtual void add_widget(std::shared_ptr<i_widget> aWidget);
		virtual void remove_widget(i_widget& aWidget);
		virtual void remove_widgets();
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
		virtual void layout_items_started();
		virtual bool layout_items_in_progress() const;
		virtual void layout_items_completed();
	public:
		virtual neogfx::logical_coordinate_system logical_coordinate_system() const;
		virtual point position() const;
		virtual void set_position(const point& aPosition);
		virtual point origin(bool aNonClient = false) const;
		virtual void move(const point& aPosition);
		virtual void moved();
		virtual size extents() const;
		virtual void set_extents(const size& aSize);
		virtual void resize(const size& aSize);
		virtual void resized();
		virtual rect window_rect() const;
		virtual rect client_rect(bool aIncludeMargins = true) const;
		virtual const i_widget& widget_at(const point& aPosition) const;
		virtual i_widget& widget_at(const point& aPosition);
	public:
		virtual bool has_size_policy() const;
		virtual neogfx::size_policy size_policy() const;
		virtual void set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout = true);
		virtual bool has_weight() const;
		virtual size weight() const;
		virtual void set_weight(const optional_size& aWeight, bool aUpdateLayout = true);
		virtual bool has_minimum_size() const;
		virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
		virtual void set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout = true);
		virtual bool has_maximum_size() const;
		virtual size maximum_size(const optional_size& aAvailableSpace = optional_size()) const;
		virtual void set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout = true);
		virtual bool has_margins() const;
		virtual neogfx::margins margins() const;
		virtual void set_margins(const optional_margins& aMargins, bool aUpdateLayout = true);
	public:
		virtual void update(bool aIncludeNonClient = false);
		virtual void update(const rect& aUpdateRect);
		virtual bool requires_update() const;
		virtual rect update_rect() const;
		virtual rect default_clip_rect(bool aIncludeNonClient = false) const;
		virtual bool ready_to_render() const;
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
		virtual colour container_background_colour() const;
		virtual bool has_font() const;
		virtual const neogfx::font& font() const;
		virtual void set_font(const optional_font& aFont);
	public:
		virtual bool visible() const;
		virtual bool effectively_visible() const;
		virtual bool hidden() const;
		virtual bool effectively_hidden() const;
		virtual void show(bool aVisible);
		virtual void show();
		virtual void hide();
		virtual bool enabled() const;
		virtual bool effectively_enabled() const;
		virtual bool disabled() const;
		virtual bool effectively_disabled() const;
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
	public:
		virtual bool ignore_mouse_events() const;
		virtual void set_ignore_mouse_events(bool aIgnoreMouseEvents);
		virtual void mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta);
		virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
		virtual void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
		virtual void mouse_button_released(mouse_button aButton, const point& aPosition);
		virtual void mouse_moved(const point& aPosition);
		virtual void mouse_entered();
		virtual void mouse_left();
		virtual neogfx::mouse_cursor mouse_cursor() const;
	public:
		virtual bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual bool text_input(const std::string& aText);
		virtual bool sys_text_input(const std::string& aText);
	public:
		virtual graphics_context create_graphics_context() const;
	protected:
		virtual const update_rect_list& update_rects() const;
	protected:
		virtual const i_widget& widget_for_mouse_event(const point& aPosition) const;
		virtual i_widget& widget_for_mouse_event(const point& aPosition);
		// helpers
	public:
		using i_widget::set_size_policy;
	private:
		i_widget* iParent;
		i_widget* iLinkBefore;
		i_widget* iLinkAfter;
		widget_list iChildren;
		std::shared_ptr<i_layout> iLayout;
		std::unique_ptr<neolib::callback_timer> iLayoutTimer;
		device_metrics_forwarder iDeviceMetricsForwarder;
		units_context iUnitsContext;
		point iPosition;
		size iSize;
		optional_margins iMargins;
		optional_size_policy iSizePolicy;
		optional_size iWeight;
		optional_size iMinimumSize;
		optional_size iMaximumSize;
		uint32_t iLayoutInProgress;
		mutable update_rect_list iUpdateRects;
		bool iVisible;
		bool iEnabled;
		neogfx::focus_policy iFocusPolicy;
		optional_colour iForegroundColour;
		optional_colour iBackgroundColour;
		optional_font iFont;
		bool iIgnoreMouseEvents;
	};
}