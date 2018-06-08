// widget.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/lifetime.hpp>
#include <neolib/timer.hpp>
#include <neogfx/core/object.hpp>
#include <neogfx/core/property.hpp>
#include "i_widget.hpp"

namespace neogfx
{
	class widget : public object<i_widget>, protected neolib::lifetime
	{
	public:
		widget();
		widget(const widget&) = delete;
		widget(i_widget& aParent);
		widget(i_layout& aLayout);
		~widget();
		neolib::i_lifetime& as_lifetime() override;
		// i_object
	public:
		void property_changed(i_property& aProperty) override;
		// i_widget
	public:
		bool is_singular() const override;
		void set_singular(bool aSingular) override;
		bool is_root() const override;
		bool has_root() const override;
		const i_window& root() const override;
		i_window& root() override;
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
		bool adding_child() const override;
		i_widget& add(i_widget& aChild) override;
		i_widget& add(std::shared_ptr<i_widget> aChild) override;
		std::shared_ptr<i_widget> remove(i_widget& aChild, bool aSingular = false) override;
		void remove_all() override;
		bool has_children() const override;
		const widget_list& children() const override;
		widget_list::const_iterator last() const override;
		widget_list::iterator last() override;
		widget_list::const_iterator find(const i_widget& aChild, bool aThrowIfNotFound = true) const override;
		widget_list::iterator find(const i_widget& aChild, bool aThrowIfNotFound = true) override;
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
		void layout_items(bool aDefer = false) override;
		void layout_items_started() override;
		bool layout_items_in_progress() const override;
		void layout_items_completed() override;
		// i_units_context
	public:
		bool device_metrics_available() const override;
		const i_device_metrics& device_metrics() const override;
		neogfx::units units() const override;
		neogfx::units set_units(neogfx::units aUnits) const override;
		// i_geometry
	public:
		bool high_dpi() const override;
		dimension dpi_scale_factor() const override;
		bool has_logical_coordinate_system() const override;
		neogfx::logical_coordinate_system logical_coordinate_system() const override;
		void set_logical_coordinate_system(const optional_logical_coordinate_system& aLogicalCoordinateSystem) override;
		point position() const override;
		void set_position(const point& aPosition) override;
		point origin() const override;
		void move(const point& aPosition) override;
		void moved() override;
		size extents() const override;
		void set_extents(const size& aSize) override;
		void resize(const size& aSize) override;
		void resized() override;
		rect window_rect() const override;
		rect client_rect(bool aIncludeMargins = true) const override;
		const i_widget& get_widget_at(const point& aPosition) const override;
		i_widget& get_widget_at(const point& aPosition) override;
		widget_part hit_test(const point& aPosition) const override;
	public:
		bool has_size_policy() const override;
		neogfx::size_policy size_policy() const override;
		void set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout = true) override;
		bool has_weight() const override;
		size weight() const override;
		void set_weight(const optional_size& aWeight, bool aUpdateLayout = true) override;
		bool has_minimum_size() const override;
		size minimum_size(const optional_size& aAvailableSpace = optional_size{}) const override;
		void set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout = true) override;
		bool has_maximum_size() const override;
		size maximum_size(const optional_size& aAvailableSpace = optional_size{}) const override;
		void set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout = true) override;
		bool has_margins() const override;
		neogfx::margins margins() const override;
		void set_margins(const optional_margins& aMargins, bool aUpdateLayout = true) override;
		// i_layout_item
	public:
		bool is_layout() const override;
		const i_layout& as_layout() const override;
		i_layout& as_layout() override;
		bool is_widget() const override;
		const i_widget& as_widget() const override;
		i_widget& as_widget() override;
	public:
		bool has_parent_layout() const override;
		const i_layout& parent_layout() const override;
		i_layout& parent_layout() override;
		void set_parent_layout(i_layout* aParentLayout) override;
		bool has_layout_owner() const override;
		const i_widget& layout_owner() const override;
		i_widget& layout_owner() override;
		void set_layout_owner(i_widget* aOwner) override;
	public:
		void layout_as(const point& aPosition, const size& aSize) override;
		uint32_t layout_id() const override;
		void next_layout_id() override;
		// i_widget
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
		void paint_non_client_after(graphics_context& aGraphicsContext) const override;
	public:
		double opacity() const override;
		void set_opacity(double aOpacity) override;
		double transparency() const override;
		void set_transparency(double aTransparency) override;
		bool has_foreground_colour() const override;
		colour foreground_colour() const override;
		void set_foreground_colour(const optional_colour& aForegroundColour = optional_colour{}) override;
		bool has_background_colour() const override;
		colour background_colour() const override;
		void set_background_colour(const optional_colour& aBackgroundColour = optional_colour{}) override;
		colour container_background_colour() const override;
		bool has_font() const override;
		const neogfx::font& font() const override;
		void set_font(const optional_font& aFont) override;
	public:
		bool visible() const override;
		bool effectively_visible() const override;
		bool hidden() const override;
		bool effectively_hidden() const override;
		bool show(bool aVisible) override;
		bool show() override;
		bool hide() override;
		bool enabled() const override;
		bool effectively_enabled() const override;
		bool disabled() const override;
		bool effectively_disabled() const override;
		bool enable(bool aEnable) override;
		bool enable() override;
		bool disable() override;
		bool entered() const override;
		bool can_capture() const override;
		bool capturing() const override;
		void set_capture(capture_reason aReason = capture_reason::Other) override;
		void release_capture(capture_reason aReason = capture_reason::Other) override;
		void non_client_set_capture() override;
		void non_client_release_capture() override;
		void captured() override;
		void released() override;
		neogfx::focus_policy focus_policy() const override;
		void set_focus_policy(neogfx::focus_policy aFocusPolicy) override;
		bool has_focus() const override;
		void set_focus(focus_reason aFocusReason = focus_reason::Other) override;
		void release_focus() override;
		void focus_gained(focus_reason aFocusReason) override;
		void focus_lost(focus_reason aFocusReason) override;
	public:
		bool ignore_mouse_events() const override;
		void set_ignore_mouse_events(bool aIgnoreMouseEvents) override;
		bool ignore_non_client_mouse_events() const override;
		void set_ignore_non_client_mouse_events(bool aIgnoreNonClientMouseEvents) override;
		bool mouse_event_is_non_client() const override;
		void mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta) override;
		void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
		void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
		void mouse_button_released(mouse_button aButton, const point& aPosition) override;
		void mouse_moved(const point& aPosition) override;
		void mouse_entered(const point& aPosition) override;
		void mouse_left() override;
		neogfx::mouse_cursor mouse_cursor() const override;
	public:
		bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
		bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
		bool text_input(const std::string& aText) override;
		bool sys_text_input(const std::string& aText) override;
	public:
		graphics_context create_graphics_context() const override;
	public:
		const i_widget& widget_for_mouse_event(const point& aPosition, bool aForHitTest = false) const override;
		i_widget& widget_for_mouse_event(const point& aPosition, bool aForHitTest = false) override;
	private:
		const i_surface* find_surface() const override;
		i_surface* find_surface() override;
		const i_window* find_root() const override;
		i_window* find_root() override;
		// helpers
	public:
		using i_widget::set_size_policy;
	private:
		bool iSingular;
		i_widget* iParent;
		mutable const i_surface* iSurface;
		mutable const i_window* iRoot;
		widget_list iChildren;
		bool iAddingChild;
		i_widget* iLinkBefore;
		i_widget* iLinkAfter;
		i_layout* iParentLayout;
		uint32_t iLayoutId;
		uint32_t iLayoutInProgress;
		std::shared_ptr<i_layout> iLayout;
		class layout_timer;
		std::unique_ptr<layout_timer> iLayoutTimer;
		units_context iUnitsContext;
		mutable std::pair<optional_rect, optional_rect> iDefaultClipRect;
	public:
		struct property_category
		{
			struct geometry {};
			struct font {};
			struct colour {};
			struct other_appearance {};
			struct other {};
		};
		define_property(property_category::geometry, optional_logical_coordinate_system, LogicalCoordinateSystem)
		define_property(property_category::geometry, point, Position)
		define_property(property_category::geometry, size, Size)
		define_property(property_category::geometry, optional_margins, Margins)
		define_property(property_category::geometry, optional_size_policy, SizePolicy)
		define_property(property_category::geometry, optional_size, Weight)
		define_property(property_category::geometry, optional_size, MinimumSize)
		define_property(property_category::geometry, optional_size, MaximumSize)
		define_property(property_category::geometry, bool, Visible, true)
		define_property(property_category::other_appearance, bool, Enabled, true)
		define_property(property_category::other, neogfx::focus_policy, FocusPolicy, neogfx::focus_policy::NoFocus)
		define_property(property_category::other_appearance, double, Opacity, 1.0)
		define_property(property_category::colour, optional_colour, ForegroundColour)
		define_property(property_category::colour, optional_colour, BackgroundColour)
		define_property(property_category::font, optional_font, Font)
		define_property(property_category::other, bool, IgnoreMouseEvents, false)
		define_property(property_category::other, bool, IgnoreNonClientMouseEvents, true)
	};
}