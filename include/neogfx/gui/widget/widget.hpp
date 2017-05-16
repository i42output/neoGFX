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
			bool metrics_available() const override;
			size extents() const override;
			dimension horizontal_dpi() const override;
			dimension vertical_dpi() const override;
			dimension em_size() const override;
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
		const i_device_metrics& device_metrics() const override;
		units_e units() const override;
		units_e set_units(units_e aUnits) const override;
	public:
		bool is_root() const override;
		bool has_parent(bool aSameSurface = true) const override;
		const i_widget& parent() const override;
		i_widget& parent() override;
		void set_parent(i_widget& aParent) override;
		void parent_changed() override;
		const i_widget& ultimate_ancestor(bool aSameSurface = true) const override;
		i_widget& ultimate_ancestor(bool aSameSurface = true) override;
		bool is_ancestor_of(const i_widget& aWidget, bool aSameSurface = true) const override;
		bool is_descendent_of(const i_widget& aWidget, bool aSameSurface = true) const override;
		bool is_sibling_of(const i_widget& aWidget) const override;
		void add_widget(i_widget& aWidget) override;
		void add_widget(std::shared_ptr<i_widget> aWidget) override;
		void remove_widget(i_widget& aWidget) override;
		void remove_widgets() override;
		bool has_children() const override;
		const widget_list& children() const override;
		widget_list::const_iterator last_child() const override;
		widget_list::iterator last_child() override;
		widget_list::const_iterator find_child(const i_widget& aChild, bool aThrowIfNotFound = true) const override;
		widget_list::iterator find_child(const i_widget& aChild, bool aThrowIfNotFound = true) override;
		const i_widget& before() const override;
		i_widget& before() override;
		const i_widget& after() const override;
		i_widget& after() override;
		void link_before(i_widget* aPreviousWidget) override;
		void link_after(i_widget* aNextWidget) override;
		void unlink() override;
		bool has_surface() const override;
		const i_surface& surface() const override;
		i_surface& surface() override;
		bool is_surface() const override;
		bool has_layout() const override;
		void set_layout(i_layout& aLayout) override;
		void set_layout(std::shared_ptr<i_layout> aLayout) override;
		const i_layout& layout() const override;
		i_layout& layout() override;
		bool can_defer_layout() const override;
		bool has_managing_layout() const override;
		const i_widget& managing_layout() const override;
		i_widget& managing_layout() override;
		bool is_managing_layout() const override;
		bool has_parent_layout() const override;
		const i_layout& parent_layout() const override;
		i_layout& parent_layout() override;
		void layout_items(bool aDefer = false) override;
		void layout_items_started() override;
		bool layout_items_in_progress() const override;
		void layout_items_completed() override;
	public:
		neogfx::logical_coordinate_system logical_coordinate_system() const override;
		point position() const override;
		void set_position(const point& aPosition) override;
		point origin(bool aNonClient = false) const override;
		void move(const point& aPosition) override;
		void moved() override;
		size extents() const override;
		void set_extents(const size& aSize) override;
		void resize(const size& aSize) override;
		void resized() override;
		rect window_rect() const override;
		rect client_rect(bool aIncludeMargins = true) const override;
		const i_widget& widget_at(const point& aPosition) const override;
		i_widget& widget_at(const point& aPosition) override;
	public:
		bool has_size_policy() const override;
		neogfx::size_policy size_policy() const override;
		void set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout = true) override;
		bool has_weight() const override;
		size weight() const override;
		void set_weight(const optional_size& aWeight, bool aUpdateLayout = true) override;
		bool has_minimum_size() const override;
		size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
		void set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout = true) override;
		bool has_maximum_size() const override;
		size maximum_size(const optional_size& aAvailableSpace = optional_size()) const override;
		void set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout = true) override;
		bool has_margins() const override;
		neogfx::margins margins() const override;
		void set_margins(const optional_margins& aMargins, bool aUpdateLayout = true) override;
	public:
		void update(bool aIncludeNonClient = false) override;
		void update(const rect& aUpdateRect) override;
		bool requires_update() const override;
		rect update_rect() const override;
		rect default_clip_rect(bool aIncludeNonClient = false) const override;
		bool ready_to_render() const override;
		void render(graphics_context& aGraphicsContext) const override;
		bool transparent_background() const override;
		void paint_non_client(graphics_context& aGraphicsContext) const override;
		void paint(graphics_context& aGraphicsContext) const override;
	public:
		bool has_foreground_colour() const override;
		colour foreground_colour() const override;
		void set_foreground_colour(const optional_colour& aForegroundColour = optional_colour()) override;
		bool has_background_colour() const override;
		colour background_colour() const override;
		void set_background_colour(const optional_colour& aBackgroundColour = optional_colour()) override;
		colour container_background_colour() const override;
		bool has_font() const override;
		const neogfx::font& font() const override;
		void set_font(const optional_font& aFont) override;
	public:
		bool visible() const override;
		bool effectively_visible() const override;
		bool hidden() const override;
		bool effectively_hidden() const override;
		void show(bool aVisible) override;
		void show() override;
		void hide() override;
		bool enabled() const override;
		bool effectively_enabled() const override;
		bool disabled() const override;
		bool effectively_disabled() const override;
		void enable(bool aEnable) override;
		void enable() override;
		void disable() override;
		bool entered() const override;
		bool capturing() const override;
		void set_capture() override;
		void release_capture() override;
		void captured() override;
		void released() override;
		neogfx::focus_policy focus_policy() const override;
		void set_focus_policy(neogfx::focus_policy aFocusPolicy) override;
		bool has_focus() const override;
		void set_focus() override;
		void release_focus() override;
		void focus_gained() override;
		void focus_lost() override;
	public:
		bool ignore_mouse_events() const override;
		void set_ignore_mouse_events(bool aIgnoreMouseEvents) override;
		void mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta) override;
		void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
		void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
		void mouse_button_released(mouse_button aButton, const point& aPosition) override;
		void mouse_moved(const point& aPosition) override;
		void mouse_entered() override;
		void mouse_left() override;
		neogfx::mouse_cursor mouse_cursor() const override;
	public:
		bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
		bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
		bool text_input(const std::string& aText) override;
		bool sys_text_input(const std::string& aText) override;
	public:
		graphics_context create_graphics_context() const override;
	protected:
		const update_rect_list& update_rects() const override;
	protected:
		const i_widget& widget_for_mouse_event(const point& aPosition) const override;
		i_widget& widget_for_mouse_event(const point& aPosition) override;
		// helpers
	public:
		using i_widget::set_size_policy;
	private:
		i_widget* iParent;
		widget_list iChildren;
		i_widget* iLinkBefore;
		i_widget* iLinkAfter;
		std::shared_ptr<i_layout> iLayout;
		class layout_timer;
		std::unique_ptr<layout_timer> iLayoutTimer;
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