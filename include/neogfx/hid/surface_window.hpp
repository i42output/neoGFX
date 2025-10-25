// surface_window.hpp
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

#include <neogfx/core/object.hpp>
#include <neogfx/gui/window/i_native_window.hpp>
#include <neogfx/hid/i_surface_window.hpp>

namespace neogfx
{
    class i_native_window;

    class surface_window : public reference_counted<object<i_surface_window>>
    {
    public:
        define_declared_event(DpiChanged, dpi_changed)
        define_declared_event(Rendering, rendering)
        define_declared_event(RenderingFinished, rendering_finished)
        define_declared_event(Closing, closing)
        define_declared_event(Closed, closed)
    public:
        typedef i_surface_window abstract_type;
    public:
        surface_window(i_window& aWindow, std::function<void(i_surface_window&)> aNativeWindowCreator);
        ~surface_window();
    public:
        dimension horizontal_dpi() const final;
        dimension vertical_dpi() const final;
        dimension ppi() const final;
        bool metrics_available() const final;
        size extents() const final;
        dimension em_size() const final;
    public:
        bool device_metrics_available() const final;
        const i_device_metrics& device_metrics() const final;
    public:
        i_rendering_engine& rendering_engine() const final;
    public:
        bool is_strong() const final;
        bool is_weak() const final;
        bool can_close() const final;
        bool is_closed() const final;
        void close() final;
        bool has_parent_surface() const final;
        const i_surface& parent_surface() const final;
        i_surface& parent_surface() final;
        bool is_owner_of(const i_surface& aChildSurface) const final;
    public:
        bool is_window() const final;
        bool is_nested_window() const final;
        const i_surface_window& as_surface_window() const final;
        i_surface_window& as_surface_window() final;
    public:
        neogfx::surface_type surface_type() const final;
        neogfx::logical_coordinate_system logical_coordinate_system() const final;
        void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) final;
        neogfx::logical_coordinates logical_coordinates() const final;
        void set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates) final;
        double z_order() const final;
        void layout_surface() final;
        void invalidate_surface(const rect& aInvalidatedRect, bool aInternal = true) final;
        bool has_invalidated_area() const final;
        const rect& invalidated_area() const final;
        rect validate() final;
        double rendering_priority() const final;
        void render_surface() final;
        void pause_rendering() final;
        void resume_rendering() final;
        bool has_native_window() const final;
        const i_native_window& native_window() const final;
        i_native_window& native_window() final;
        void set_native_window(i_native_window& aNativeWindow) final;
        bool has_native_surface() const final;
        const i_native_surface& native_surface() const final;
        i_native_surface& native_surface() final;
    public:
        void handle_dpi_changed() final;
    public:
        point surface_position() const final;
        void move_surface(const point& aPosition) final;
        size surface_extents() const final;
        void resize_surface(const size& aExtents) final;
        double surface_opacity() const final;
        void set_surface_opacity(double aOpacity) final;
        double surface_transparency() const final;
        void set_surface_transparency(double aTransparency) final;
    public:
        bool has_clicked_widget() const final;
        i_widget& clicked_widget() const final;
        bool has_capturing_widget() const final;
        i_widget& capturing_widget() const final;
        void set_capture(i_widget& aWidget) final;
        void release_capture(i_widget& aWidget) final;
        void non_client_set_capture(i_widget& aWidget) final;
        void non_client_release_capture(i_widget& aWidget) final;
    private:
        mouse_event_location current_mouse_event_location() const final;
        bool is_closing() const final;
        bool native_window_can_close() const final;
        void native_window_closing() final;
        void native_window_closed() final;
        void native_window_focus_gained() final;
        void native_window_focus_lost() final;
        void native_window_resizing() final;
        void native_window_resized() final;
        void native_window_moving() final;
        void native_window_moved() final;
        double native_window_rendering_priority() const final;
        bool native_window_ready_to_render() const final;
        void native_window_render(const rect& aInvalidatedRect) const final;
        void native_window_dismiss_children() final;
        void native_window_mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifier aKeyModifier) final;
        void native_window_mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) final;
        void native_window_mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) final;
        void native_window_mouse_button_click(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) final;
        void native_window_mouse_button_double_click(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) final;
        void native_window_mouse_button_released(mouse_button aButton, const point& aPosition) final;
        void native_window_mouse_moved(const point& aPosition, key_modifier aKeyModifier) final;
        void native_window_non_client_mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifier aKeyModifier) final;
        void native_window_non_client_mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) final;
        void native_window_non_client_mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) final;
        void native_window_non_client_mouse_button_click(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) final;
        void native_window_non_client_mouse_button_double_click(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) final;
        void native_window_non_client_mouse_button_released(mouse_button aButton, const point& aPosition) final;
        void native_window_non_client_mouse_moved(const point& aPosition, key_modifier aKeyModifier) final;
        void native_window_mouse_entered(const point& aPosition) final;
        void native_window_mouse_left() final;
        widget_part native_window_hit_test(const point& aPosition) const final;
        rect native_window_widget_part_rect(widget_part_e aWidgetPart) const final;
        void native_window_key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier) final;
        void native_window_key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier) final;
        void native_window_text_input(i_string const& aText) final;
        void native_window_sys_text_input(i_string const& aText) final;
        neogfx::mouse_cursor native_window_mouse_cursor() const final;
        void native_window_title_text_changed(i_string const& aTitleText) final;
    public:
        window_style style() const final;
        void set_style(window_style aStyle) final;
    public:
        const i_window& as_window() const final;
        i_window& as_window() final;
        const i_widget& as_widget() const final;
        i_widget& as_widget()  final;
    public:
        const i_window& window_at_position(const point& aPosition, bool aForMouseEvent = false) const final;
        i_window& window_at_position(const point& aPosition, bool aForMouseEvent = false) final;
        const i_widget& widget_for_mouse_event(const point& aPosition, bool aForHitTest = false) const final;
        i_widget& widget_for_mouse_event(const point& aPosition, bool aForHitTest = false) final;
    private:
        std::optional<destroyed_flag> iNativeWindowDestroyed;
        i_window& iWindow;
        i_rendering_engine& iRenderingEngine;
        ref_ptr<i_native_window> iNativeWindow;
        bool iNativeWindowClosing;
        bool iClosing;
        bool iClosed;
        std::optional<char32_t> iSurrogatePairPart;
        i_widget* iCapturingWidget;
        i_widget* iClickedWidget;
    };
}