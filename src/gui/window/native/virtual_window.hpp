// virtual_window.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2021 Leigh Johnston.  All Rights Reserved.
  
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
#include "native_window.hpp"

namespace neogfx
{
    class i_surface_window;

    class virtual_window : public native_window
    {
    public:
        virtual_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_window& aParent, const basic_size<int>& aDimensions, std::string const& aWindowTitle, window_style aStyle = window_style::Default);
        virtual_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_window& aParent, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, std::string const& aWindowTitle, window_style aStyle = window_style::Default);
        ~virtual_window();
    public:
        render_target_type target_type() const override;
        const i_texture& target_texture() const override;
        point target_origin() const override;
        size target_extents() const override;
    public:
        neogfx::logical_coordinate_system logical_coordinate_system() const override;
        void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) override;
        neogfx::logical_coordinates logical_coordinates() const override;
        void set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates) override;
    public:
        rect_i32 viewport() const override;
        rect_i32 set_viewport(const rect_i32& aViewport) const override;
    public:
        bool target_active() const override;
        void activate_target() const override;
        void deactivate_target() const override;
    public:
        neogfx::color_space color_space() const override;
        color read_pixel(const point& aPosition) const override;
    public:
        uint64_t frame_counter() const override;
        double fps() const override;
        double potential_fps() const override;
    public:
        void invalidate(const rect& aInvalidatedRect) override;
        bool has_invalidated_area() const override;
        const rect& invalidated_area() const override;
        rect validate() override;
        void render(bool aOOBRequest = false) override;
        bool is_rendering() const override;
    public:
        void debug(bool aEnableDebug) override;
    public:
        bool metrics_available() const override;
        size extents() const override;
    protected:
        i_surface_window& surface_window() const override;
        void set_destroying() override;
        void set_destroyed() override;
    public:
        void* target_handle() const override;
        void* target_device_handle() const override;
        pixel_format_t pixel_format() const override;
    public:
        bool has_parent() const override;
        const i_native_window& parent() const override;
        i_native_window& parent() override;
        bool is_nested() const override;
    public:
        bool initialising() const override;
        void initialisation_complete() override;
        void* handle() const override;
        void* native_handle() const override;
        point surface_position() const override;
        void move_surface(const point& aPosition) override;
        size surface_extents() const override;
        void resize_surface(const size& aExtents) override;
    public:
        bool can_render() const override;
    public:
        std::unique_ptr<i_rendering_context> create_graphics_context(blending_mode aBlendingMode = blending_mode::Default) const override;
        std::unique_ptr<i_rendering_context> create_graphics_context(const i_widget& aWidget, blending_mode aBlendingMode = blending_mode::Default) const override;
    public:
        void close(bool aForce = false) override;
        bool visible() const override;
        void show(bool aActivate = false) override;
        void hide() override;
        double opacity() const override;
        void set_opacity(double aOpacity) override;
        double transparency() const override;
        void set_transparency(double aTransparency) override;
        bool is_active() const override;
        void activate() override;
        bool is_iconic() const override;
        void iconize() override;
        bool is_maximized() const override;
        void maximize() override;
        bool is_restored() const override;
        void restore() override;
        bool is_fullscreen() const override;
        void enter_fullscreen(const video_mode& aVideoMode) override;
        bool enabled() const override;
        void enable(bool aEnable) override;
        bool is_capturing() const override;
        void set_capture() override;
        void release_capture() override;
        void non_client_set_capture() override;
        void non_client_release_capture() override;
        void set_title_text(i_string const& aTitleText) override;
        padding border_thickness() const override;
    private:
        void debug_message(std::string const& aMessage);
    private:
        i_surface_window& iSurfaceWindow;
        i_native_window& iParent;
        neogfx::logical_coordinate_system iLogicalCoordinateSystem;
        mutable std::optional<neogfx::logical_coordinates> iLogicalCoordinates;
        point iPosition;
        size iExtents;
        bool iEnabled;
        bool iVisible;
        double iOpacity;
        bool iActive;
        window_state iState;
        bool iMouseEntered;
        bool iCapturingMouse;
        bool iNonClientCapturing;
        bool iReady;
        mutable padding iBorderThickness;
        widget_part_e iClickedWidgetPart;
        bool iSystemMenuOpen;
        bool iDebug;
    };
}