// widget.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <boost/unordered/unordered_flat_map.hpp>

#include <neogfx/gui/widget/timer.hpp>
#include <neogfx/core/object.hpp>
#include <neogfx/core/property.hpp>
#include <neogfx/app/palette.hpp>
#include <neogfx/gfx/text/i_font_manager.hpp>
#include <neogfx/gui/layout/layout_item.hpp>
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
    template <typename T>
    concept WidgetInterface = std::is_base_of_v<i_widget, T>;

    template <typename T>
    concept Widget = std::is_base_of_v<i_widget, T> && T::IsWidget;

    template <WidgetInterface Interface = i_widget>
    class widget : public layout_item<object<Interface>>
    {
    public:
        static bool constexpr IsWidget = true;
    public:
        meta_object(layout_item<object<Interface>>)
    public:
        define_declared_event(DeviceMetricsUpdated, device_metrics_updated, const i_widget&)
        define_declared_event(ChildAdded, child_added, i_widget&)
        define_declared_event(ChildRemoved, child_removed, i_widget&)
        define_declared_event(VisibilityChanged, visibility_changed)
        define_declared_event(PositionChanged, position_changed)
        define_declared_event(ParentPositionChanged, parent_position_changed)
        define_declared_event(SizeChanged, size_changed)
        define_declared_event(LayoutCompleted, layout_completed)
        define_declared_event(PaintingNonClient, painting_non_client, i_graphics_context&)
        define_declared_event(PaintedNonClient, painted_non_client, i_graphics_context&)
        define_declared_event(Painting, painting, i_graphics_context&)
        define_declared_event(PaintingChildren, painting_children, i_graphics_context&)
        define_declared_event(Painted, painted, i_graphics_context&)
        define_declared_event(Mouse, mouse_event, const neogfx::mouse_event&)
        define_declared_event(NonClientMouse, non_client_mouse_event, const neogfx::non_client_mouse_event&)
        define_declared_event(MouseEntered, mouse_entered_event, const point&)
        define_declared_event(MouseLeft, mouse_left_event)
        define_declared_event(QueryMouseCursor, query_mouse_cursor, neogfx::mouse_cursor&)
        define_declared_event(Keyboard, keyboard_event, const neogfx::keyboard_event&)
        define_declared_event(Focus, focus_event, neogfx::focus_event, focus_reason)
    public:
        using typename base_type::no_parent;
        using typename base_type::no_root;
        using typename base_type::no_surface;
        using typename base_type::no_children;
        using typename base_type::not_child;
        using typename base_type::no_update_rect;
        using typename base_type::widget_not_entered;
        using typename base_type::widget_cannot_capture;
        using typename base_type::widget_not_focused;
        using typename base_type::widget_cannot_defer_layout;
        using typename base_type::layout_already_set;
        using typename base_type::no_layout;
    public:
        using abstract_type = i_widget;
        using widget_list = neolib::vector<ref_ptr<i_widget>>;
    private:
        using widget_child_pos = widget_list::size_type;
        using widget_map = boost::unordered_flat_map<i_widget const*, widget_child_pos>;
        using property_context_type = widget;
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
        bool is_singular() const final;
        void set_singular(bool aSingular) final;
        bool is_root() const final;
        bool has_root() const final;
        const i_window& root() const final;
        i_window& root() final;
        void set_root(i_window& aRoot) final;
        bool is_surface() const override;
        bool has_surface() const override;
        const i_surface& surface() const override;
        i_surface& surface() override;
        const i_surface& real_surface() const override;
        i_surface& real_surface() override;
        bool has_parent() const final;
        const i_widget& parent() const final;
        i_widget& parent() final;
        void set_parent(i_widget& aParent) override;
        void parent_changed() override;
        bool adding_child() const final;
        i_widget& add(i_widget& aChild) final;
        i_widget& add(const i_ref_ptr<i_widget>& aChild) final;
        void remove(i_widget& aChild, bool aSingular, i_ref_ptr<i_widget>& aChildRef) final;
        void remove_all() final;
        bool has_children() const final;
        const widget_list& children() const final;
        widget_list::const_iterator last() const final;
        widget_list::iterator last() final;
        widget_list::const_iterator find(const i_widget& aChild, bool aThrowIfNotFound = true) const final;
        widget_list::iterator find(const i_widget& aChild, bool aThrowIfNotFound = true) final;
    public:
        void bring_child_to_front(const i_widget& aChild) override;
        void send_child_to_back(const i_widget& aChild) override;
        layer_t layer() const override;
        void set_layer(layer_t aLayer) override;
    public:
        const i_widget& before() const override;
        i_widget& before() override;
        const i_widget& after() const override;
        i_widget& after() override;
        void link_before(i_widget* aPreviousWidget) override;
        void link_after(i_widget* aNextWidget) override;
        void unlink() override;
        bool has_layout() const override;
        void set_layout(i_layout& aLayout, bool aMoveExistingItems = true) override;
        void set_layout(const i_ref_ptr<i_layout>& aLayout, bool aMoveExistingItems = true) override;
        const i_layout& layout() const override;
        i_layout& layout() override;
        bool can_defer_layout() const override;
        bool is_managing_layout() const override;
        optional<neogfx::layout_reason>& layout_reason() override;
        void layout_items(bool aDefer = false) override;
        void layout_items_started() override;
        bool layout_items_in_progress() const override;
        void layout_items_completed() override;
        // i_units_context
    public:
        bool device_metrics_available() const override;
        const i_device_metrics& device_metrics() const override;
        // i_geometry
    public:
        neogfx::size_policy size_policy() const override;
        size minimum_size(optional_size const& aAvailableSpace = {}) const override;
        size maximum_size(optional_size const& aAvailableSpace = {}) const override;
        neogfx::padding padding() const override;
    public:
        rect element_rect(skin_element aElement) const override;
        // i_widget
    public:
        bool has_logical_coordinate_system() const override;
        neogfx::logical_coordinate_system logical_coordinate_system() const override;
        void set_logical_coordinate_system(const optional_logical_coordinate_system& aLogicalCoordinateSystem) override;
        rect non_client_rect() const override;
        rect client_rect(bool aExtendIntoPadding = true) const override;
        void move(const point& aPosition) override;
        void moved() override;
        void parent_moved() override;
        bool resizing() const override;
        void resize(const size& aSize) override;
        void resized() override;
        const i_widget& get_widget_at(const point& aPosition) const override;
        i_widget& get_widget_at(const point& aPosition) override;
        neogfx::widget_type widget_type() const override;
        bool part_active(widget_part aPart) const override;
        widget_part part(const point& aPosition) const override;
        widget_part hit_test(const point& aPosition) const override;
        // i_layout_item
    public:
        bool has_parent_layout() const final;
        const i_layout& parent_layout() const final;
        i_layout& parent_layout() final;
        void set_parent_layout(i_layout* aParentLayout) final;
        bool has_parent_widget() const final;
        const i_widget& parent_widget() const final;
        i_widget& parent_widget() final;
        void set_parent_widget(i_widget* aParentWidget) final;
    public:
        void layout_as(const point& aPosition, const size& aSize) final;
        // i_widget
    public:
        bool has_view() const override;
        neogfx::view view(bool aExtendIntoPadding = true) const override;
        void set_view(optional_view const& aView) override;
        // i_widget
    public:
        layer_t render_layer() const override;
        void set_render_layer(const std::optional<layer_t>& aLayer) override;
        bool can_update() const override;
        bool update(bool aIncludeNonClient = false) override;        
        bool update(const rect& aUpdateRect) override;
        bool requires_update() const override;
        rect update_rect() const override;
        rect default_clip_rect(bool aIncludeNonClient = false) const override;
        bool ready_to_render() const override;
        void render(i_graphics_context& aGc) const override;
        void paint_non_client(i_graphics_context& aGc) const override;
        void paint(i_graphics_context& aGc) const override;
        void paint_non_client_after(i_graphics_context& aGc) const override;
    public:
        double opacity() const override;
        void set_opacity(double aOpacity) override;
        bool has_background_opacity() const override;
        double background_opacity() const override;
        void set_background_opacity(double aOpacity) override;
        bool has_palette() const override;
        const i_palette& palette() const override;
        void set_palette(const i_palette& aPalette) override;
        bool has_palette_color(color_role aColorRole) const override;
        color palette_color(color_role aColorRole) const override;
        void set_palette_color(color_role aColorRole, const optional_color& aColor) override;
        color container_background_color() const override;
        bool has_font_role() const override;
        neogfx::font_role font_role() const override;
        void set_font_role(const optional_font_role& aFontRole) override;
        bool has_font() const override;
        const neogfx::font& font() const override;
        void set_font(optional_font const& aFont) override;
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
        bool entered(bool aChildEntered = false) const override;
        bool can_capture() const override;
        bool capture_locks_cursor() const override;
        bool capturing() const override;
        const optional_point& capture_position() const override;
        void set_capture(capture_reason aReason = capture_reason::Other, const optional_point& aPosition = {}) override;
        void release_capture(capture_reason aReason = capture_reason::Other) override;
        void non_client_set_capture() override;
        void non_client_release_capture() override;
        void captured() override;
        void capture_released() override;
        bool has_focus_policy() const override;
        neogfx::focus_policy focus_policy() const override;
        void set_focus_policy(const optional_focus_policy& aFocusPolicy) override;
        bool can_set_focus(focus_reason aFocusReason) const override;
        bool has_focus() const override;
        bool child_has_focus() const override;
        bool set_focus(focus_reason aFocusReason = focus_reason::Other) override;
        bool release_focus() override;
        void focus_gained(focus_reason aFocusReason) override;
        void focus_lost(focus_reason aFocusReason) override;
    public:
        bool consider_ancestors_for_mouse_events() const override;
        void set_consider_ancestors_for_mouse_events(bool aConsiderAncestors = true) override;
        bool ignore_mouse_events(bool aConsiderAncestors = true) const override;
        void set_ignore_mouse_events(bool aIgnoreMouseEvents) override;
        bool ignore_non_client_mouse_events(bool aConsiderAncestors = true) const override;
        void set_ignore_non_client_mouse_events(bool aIgnoreNonClientMouseEvents) override;
        neogfx::mouse_event_location mouse_event_location() const override;
        bool mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifiers_e aKeyModifiers) override;
        void mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_click(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_double_click(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_released(mouse_button aButton, const point& aPosition) override;
        void mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_entered(const point& aPosition) override;
        void mouse_left() override;
        point mouse_position() const override;
        neogfx::mouse_cursor mouse_cursor() const override;
    public:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
        bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
        bool text_input(i_string const& aText) override;
        bool sys_text_input(i_string const& aText) override;
    public:
        const i_widget& widget_for_mouse_event(const point& aPosition, bool aForHitTest = false) const override;
        i_widget& widget_for_mouse_event(const point& aPosition, bool aForHitTest = false) override;
        // helpers
    public:
        using base_type::set_size_policy;
    public:
        using base_type::is_surface;
        using base_type::has_surface;
        using base_type::surface;
        using base_type::same_surface;
    public:
        using base_type::is_parent_of;
        using base_type::is_ancestor_of;
        using base_type::is_descendent_of;
        using base_type::is_sibling_of;
        using base_type::remove;
    public:
        using base_type::to_window_coordinates;
        using base_type::to_client_coordinates;
    public:
        using base_type::can_update;
        using base_type::update;
    public:
        using base_type::show;
        using base_type::hide;
        using base_type::enable;
        using base_type::disable;
    public:
        using base_type::has_background_color;
        using base_type::background_color;
        using base_type::set_background_color;
        using base_type::has_foreground_color;
        using base_type::foreground_color;
        using base_type::set_foreground_color;
        using base_type::has_base_color;
        using base_type::base_color;
        using base_type::set_base_color;
        using base_type::has_alternate_base_color;
        using base_type::alternate_base_color;
        using base_type::set_alternate_base_color;
        // state
    private:
        bool iSingular;
        i_widget* iParent;
        mutable std::optional<const i_window*> iRoot;
        struct device_metrics_proxy : i_device_metrics
        {
            widget const& owner;
            i_device_metrics const& surrogate;

            device_metrics_proxy(widget const& owner) : owner{ owner }, surrogate{ owner.surface() }
            {
            }
            dimension horizontal_dpi() const final
            {
                return surrogate.horizontal_dpi();
            }
            dimension vertical_dpi() const final
            {
                return surrogate.vertical_dpi();
            }
            dimension ppi() const final
            {
                return surrogate.ppi();
            }
            bool metrics_available() const final
            {
                return true;
            }
            size extents() const final
            {
                return !owner.Anchor_Size.active() ? 
                    static_cast<size>(owner.Size) : owner.Anchor_Size.evaluate_constraints();
            }
            dimension em_size() const final
            {
                return surrogate.em_size();
            }
        };
        mutable std::optional<device_metrics_proxy> iDeviceMetrics;
        widget_list iChildren;
        widget_map iChildMap;
        bool iAddingChild;
        i_widget* iLinkBefore;
        i_widget* iLinkAfter;
        i_layout* iParentLayout;
        bool iResizing;
        bool iLayoutPending;
        std::uint32_t iLayoutInProgress;
        optional<neogfx::layout_reason> iLayoutReason;
        ref_ptr<i_layout> iLayout;
        mutable cache<rect> iDefaultNonClientClipRect;
        mutable cache<rect> iDefaultClientClipRect;
        optional_point iCapturePosition;
        std::int32_t iLayer;
        optional_view iView;
        std::optional<std::int32_t> iRenderLayer;
        // properties / anchors
    public:
        define_property(property_category::hard_geometry, optional_logical_coordinate_system, LogicalCoordinateSystem, logical_coordinate_system)
        define_property(property_category::hard_geometry, bool, Visible, visible, true)
        define_property(property_category::other_appearance, bool, Enabled, enabled, true)
        define_property(property_category::other, optional_focus_policy, FocusPolicy, focus_policy)
        define_property(property_category::other_appearance, double, Opacity, opacity, 1.0)
        define_property(property_category::other_appearance, optional<double>, BackgroundOpacity, background_opacity)
        define_property(property_category::other_appearance, optional<neogfx::palette>, Palette, palette)
        define_property(property_category::font, optional_font_role, FontRole, font_role)
        define_property(property_category::font, optional_font, Font, font)
        define_property(property_category::other, bool, ConsiderAncestorsForMouseEvents, consider_ancestors_for_mouse_events, true)
        define_property(property_category::other, bool, IgnoreMouseEvents, ignore_mouse_events, false)
        define_property(property_category::other, bool, IgnoreNonClientMouseEvents, ignore_non_client_mouse_events, true)
   };
}
