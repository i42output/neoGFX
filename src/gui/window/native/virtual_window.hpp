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
#include "native_surface.hpp"

namespace neogfx
{
    class i_surface_window;
    class i_widget;

    class virtual_window : public native_window
    {
    public:
        virtual_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_window& aParent, const basic_size<int>& aDimensions, std::string const& aWindowTitle, window_style aStyle = window_style::Default);
        virtual_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_window& aParent, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, std::string const& aWindowTitle, window_style aStyle = window_style::Default);
        ~virtual_window();
    protected:
        void set_destroying() final;
        void set_destroyed() final;
    public:
        void* target_handle() const final;
        void* target_device_handle() const final;
    public:
        bool has_parent() const final;
        const i_native_window& parent() const final;
        i_native_window& parent() final;
        bool is_nested() const final;
    public:
        bool initialising() const final;
        void initialisation_complete() final;
        void* handle() const final;
        void* native_handle() const final;
        point surface_position() const final;
        void move_surface(const point& aPosition) final;
        size surface_extents() const final;
        void resize_surface(const size& aExtents) final;
        bool resizing_or_moving() const final;
    public:
        bool can_render() const final;
        void render(bool aOOBRequest = false) final;
        void display() final;
    public:
        void close(bool aForce = false) final;
        bool visible() const final;
        void show(bool aActivate = false) final;
        void hide() final;
        double opacity() const final;
        void set_opacity(double aOpacity) final;
        double transparency() const final;
        void set_transparency(double aTransparency) final;
        bool is_effectively_active() const final;
        bool is_active() const final;
        void activate() final;
        void deactivate() final;
        bool is_iconic() const final;
        void iconize() final;
        bool is_maximized() const final;
        void maximize() final;
        bool is_restored() const final;
        void restore() final;
        bool is_fullscreen() const final;
        void enter_fullscreen(const video_mode& aVideoMode) final;
        bool enabled() const final;
        void enable(bool aEnable) final;
        bool is_capturing() const final;
        void set_capture() final;
        void release_capture() final;
        void non_client_set_capture() final;
        void non_client_release_capture() final;
        void set_title_text(i_string const& aTitleText) final;
        border border_thickness() const final;
    private:
        i_widget const& as_widget() const;
        i_widget& as_widget();
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
        mutable border iBorderThickness;
        widget_part_e iClickedWidgetPart;
        bool iSystemMenuOpen;
        bool iDebug;
    };
}