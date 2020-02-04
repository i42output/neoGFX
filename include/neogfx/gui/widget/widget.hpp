// widget.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neolib/timer.hpp>
#include <neogfx/core/object.hpp>
#include <neogfx/core/property.hpp>
#include <neogfx/gui/layout/layout_item.hpp>
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
    class widget : public layout_item<i_widget>
    {
    public:
        define_declared_event(VisibilityChanged, visibility_changed)
        define_declared_event(PositionChanged, position_changed)
        define_declared_event(SizeChanged, size_changed)
        define_declared_event(LayoutCompleted, layout_completed)
        define_declared_event(Painting, painting, i_graphics_context&)
        define_declared_event(Painted, painted, i_graphics_context&)
        define_declared_event(ChildrenPainted, children_painted, i_graphics_context&)
        define_declared_event(Mouse, mouse_event, const neogfx::mouse_event&)
        define_declared_event(NonClientMouse, non_client_mouse_event, const neogfx::non_client_mouse_event&)
        define_declared_event(Keyboard, keyboard_event, const neogfx::keyboard_event&)
        define_declared_event(Focus, focus_event, neogfx::focus_event)
    public:
        typedef i_widget abstract_type;
    private:
        typedef abstract_type property_context_type;
    public:
        widget();
        widget(const widget&) = delete;
        widget(i_widget& aParent);
        widget(i_layout& aLayout);
        ~widget();
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
        bool has_parent() const override;
        const i_widget& parent() const override;
        i_widget& parent() override;
        void set_parent(i_widget& aParent) override;
        void parent_changed() override;
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
        bool has_layout() const override;
        void set_layout(i_layout& aLayout, bool aMoveExistingItems = true) override;
        void set_layout(std::shared_ptr<i_layout> aLayout, bool aMoveExistingItems = true) override;
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
        bool high_dpi() const override;
        dimension dpi_scale_factor() const override;
    public:
        bool device_metrics_available() const override;
        const i_device_metrics& device_metrics() const override;
        // i_geometry
    public:
        point position() const override;
        void set_position(const point& aPosition) override;
        size extents() const override;
        void set_extents(const size& aSize) override;
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
        // i_skinnable_item
    public:
        bool is_widget() const override;
        const i_widget& as_widget() const override;
        i_widget& as_widget() override;
    public:
        rect element_rect(skin_element aElement) const override;
        // i_widget
    public:
        bool has_logical_coordinate_system() const override;
        neogfx::logical_coordinate_system logical_coordinate_system() const override;
        void set_logical_coordinate_system(const optional_logical_coordinate_system& aLogicalCoordinateSystem) override;
        point origin() const override;
        rect non_client_rect() const override;
        rect client_rect(bool aIncludeMargins = true) const override;
        void move(const point& aPosition) override;
        void moved() override;
        void parent_moved() override;
        void resize(const size& aSize) override;
        void resized() override;
        const i_widget& get_widget_at(const point& aPosition) const override;
        i_widget& get_widget_at(const point& aPosition) override;
        widget_part hit_test(const point& aPosition) const override;
        // i_layout_item
    public:
        bool is_layout() const override;
        const i_layout& as_layout() const override;
        i_layout& as_layout() override;
    public:
        bool has_parent_layout() const override;
        const i_layout& parent_layout() const override;
        i_layout& parent_layout() override;
        void set_parent_layout(i_layout* aParentLayout) override;
        bool has_layout_owner() const override;
        const i_widget& layout_owner() const override;
        i_widget& layout_owner() override;
        void set_layout_owner(i_widget* aOwner) override;
        bool is_proxy() const override;
        const i_layout_item_proxy& proxy_for_layout() const override;
        i_layout_item_proxy& proxy_for_layout() override;
    public:
        void layout_as(const point& aPosition, const size& aSize) override;
        // i_widget
    public:
        using i_widget::update;
        bool update(const rect& aUpdateRect) override;
        bool requires_update() const override;
        rect update_rect() const override;
        rect default_clip_rect(bool aIncludeNonClient = false) const override;
        bool ready_to_render() const override;
        void render(i_graphics_context& aGraphicsContext) const override;
        bool transparent_background() const override;
        void paint_non_client(i_graphics_context& aGraphicsContext) const override;
        void paint(i_graphics_context& aGraphicsContext) const override;
        void paint_non_client_after(i_graphics_context& aGraphicsContext) const override;
    public:
        double opacity() const override;
        void set_opacity(double aOpacity) override;
        double transparency() const override;
        void set_transparency(double aTransparency) override;
        bool has_foreground_color() const override;
        color foreground_color() const override;
        void set_foreground_color(const optional_color& aForegroundColor = optional_color{}) override;
        bool has_background_color() const override;
        color background_color() const override;
        void set_background_color(const optional_color& aBackgroundColor = optional_color{}) override;
        color container_background_color() const override;
        bool has_font() const override;
        const neogfx::font& font() const override;
        void set_font(const optional_font& aFont) override;
    public:
        bool visible() const override;
        bool effectively_visible() const override;
        bool hidden() const override;
        bool effectively_hidden() const override;
        bool show(bool aVisible) override;
        bool enabled() const override;
        bool effectively_enabled() const override;
        bool disabled() const override;
        bool effectively_disabled() const override;
        bool enable(bool aEnable) override;
        bool entered() const override;
        bool can_capture() const override;
        bool capturing() const override;
        const optional_point& capture_position() const override;
        void set_capture(capture_reason aReason = capture_reason::Other, const optional_point& aPosition = {}) override;
        void release_capture(capture_reason aReason = capture_reason::Other) override;
        void non_client_set_capture() override;
        void non_client_release_capture() override;
        void captured() override;
        void capture_released() override;
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
        const i_widget& widget_for_mouse_event(const point& aPosition, bool aForHitTest = false) const override;
        i_widget& widget_for_mouse_event(const point& aPosition, bool aForHitTest = false) override;
        // helpers
    public:
        using i_widget::set_size_policy;
        using i_widget::show;
        using i_widget::hide;
        using i_widget::enable;
        using i_widget::disable;
        // state
    private:
        bool iSingular;
        i_widget* iParent;
        mutable std::optional<const i_window*> iRoot;
        mutable std::optional<bool> iDeviceMetricsAvailable;
        widget_list iChildren;
        bool iAddingChild;
        i_widget* iLinkBefore;
        i_widget* iLinkAfter;
        i_layout* iParentLayout;
        uint32_t iLayoutInProgress;
        std::shared_ptr<i_layout> iLayout;
        class layout_timer;
        std::unique_ptr<layout_timer> iLayoutTimer;
        mutable std::pair<optional_rect, optional_rect> iDefaultClipRect;
        mutable optional_point iOrigin;
        optional_point iCapturePosition;
        // properties / anchors
    public:
        define_property(property_category::hard_geometry, optional_logical_coordinate_system, LogicalCoordinateSystem, logical_coordinate_system)
        define_property(property_category::hard_geometry, bool, Visible, visible, true)
        define_property(property_category::other_appearance, bool, Enabled, enabled, true)
        define_property(property_category::other, neogfx::focus_policy, FocusPolicy, focus_policy, neogfx::focus_policy::NoFocus)
        define_property(property_category::other_appearance, double, Opacity, opacity, 1.0)
        define_property(property_category::color, optional_color, ForegroundColor, foreground_color)
        define_property(property_category::color, optional_color, BackgroundColor, background_color)
        define_property(property_category::font, optional_font, Font, font)
        define_property(property_category::other, bool, IgnoreMouseEvents, ignore_mouse_events, false)
        define_property(property_category::other, bool, IgnoreNonClientMouseEvents, ignore_non_client_mouse_events, true)
   };
}