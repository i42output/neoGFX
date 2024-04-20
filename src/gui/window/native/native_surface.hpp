// native_surface.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2023 Leigh Johnston.  All Rights Reserved.
  
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

#include <neolib/core/string_utils.hpp>

#include <neogfx/neogfx.hpp>
#include <neogfx/core/object.hpp>
#include <neogfx/gui/widget/timer.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/hid/i_native_surface.hpp>

namespace neogfx
{
    class i_surface_window;

    class native_surface : public reference_counted<object<i_native_surface>>
    {
        typedef reference_counted<object<i_native_surface>> base_type;
    public:
        define_declared_event(TargetActivating, target_activating)
        define_declared_event(TargetActivated, target_activated)
        define_declared_event(TargetDeactivating, target_deactivating)
        define_declared_event(TargetDeactivated, target_deactivated)
    public:
        struct bad_pause_count : std::logic_error { bad_pause_count() : std::logic_error("neogfx::native_surface::bad_pause_count") {} };
    public:
        native_surface(i_rendering_engine& aRenderingEngine, i_surface_window& aWindow);
        ~native_surface();
    public:
        i_rendering_engine& rendering_engine() const;
        i_surface_window& surface_window() const;
    public:
        render_target_type target_type() const override;
        void* target_handle() const override;
        void* target_device_handle() const override;
        pixel_format_t pixel_format() const override;
        point target_origin() const override;
        size target_extents() const override;
    public:
        dimension horizontal_dpi() const override;
        dimension vertical_dpi() const override;
        dimension ppi() const override;
    public:
        bool metrics_available() const override;
        size extents() const override;
        dimension em_size() const override;
    public:
        neogfx::logical_coordinate_system logical_coordinate_system() const override;
        void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) override;
        neogfx::logical_coordinates logical_coordinates() const override;
        void set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates) override;
    public:
        bool target_active() const override;
        void activate_target() const override;
        void deactivate_target() const override;
    public:
        neogfx::color_space color_space() const override;
    public:
        std::uint64_t frame_counter() const override;
        double fps() const override;
        double potential_fps() const override;
    public:
        void invalidate(const rect& aInvalidatedRect) override;
        bool has_invalidated_area() const override;
        const rect& invalidated_area() const override;
        rect validate() override;
        bool can_render() const override;
        void pause() override;
        void resume() override;
        void render(bool aOOBRequest = false) override;
        bool is_rendering() const override;
    public:
        void debug(bool aEnableDebug) override;
    protected:
        void set_destroying() override;
        void set_destroyed() override;
    private:
        virtual void do_activate_target() const = 0;
        virtual void do_render() = 0;
    private:
        void debug_message(std::string const& aMessage);
    private:
        i_rendering_engine& iRenderingEngine;
        i_surface_window& iSurfaceWindow;
        mutable std::optional<pixel_format_t> iPixelFormat;
        neogfx::logical_coordinate_system iLogicalCoordinateSystem;
        mutable std::optional<neogfx::logical_coordinates> iLogicalCoordinates;
        std::optional<rect> iInvalidatedArea;
        std::uint64_t iFrameCounter;
        typedef std::chrono::time_point<std::chrono::high_resolution_clock> frame_time_point;
        typedef std::pair<frame_time_point, frame_time_point> frame_times;
        std::optional<frame_time_point> iLastFrameTime;
        std::deque<frame_times> iFpsData;
        std::uint32_t iPaused;
        bool iRendering;
        bool iDebug;
    };
}