// opengl_window.hpp
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
#include <unordered_set>
#include <boost/lexical_cast.hpp>
#include <neolib/string_utils.hpp>
#include <neolib/timer.hpp>
#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/texture.hpp>
#include "../../../gfx/native/opengl.hpp"
#include "../../../gfx/native/opengl.hpp"
#include "native_window.hpp"

namespace neogfx
{
    class i_surface_window;

    class opengl_window : public native_window
    {
    public:
        struct busy_rendering : std::logic_error { busy_rendering() : std::logic_error("neogfx::opengl_window::busy_rendering") {} };
        struct bad_pause_count : std::logic_error { bad_pause_count() : std::logic_error("neogfx::opengl_window::bad_pause_count") {} };
    public:
        opengl_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow);
        ~opengl_window();
    public:
        render_target_type target_type() const override;
        const i_texture& target_texture() const override;
        size target_extents() const override;
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
        colour read_pixel(const point& aPosition) const override;
    public:
        uint64_t frame_counter() const override;
        double fps() const override;
    public:
        void invalidate(const rect& aInvalidatedRect) override;
        bool has_invalidated_area() const override;
        const rect& invalidated_area() const override;
        rect validate() override;
        bool can_render() const override;
        void render(bool aOOBRequest = false) override;
        void pause() override;
        void resume() override;
        bool is_rendering() const override;
    public:
        bool metrics_available() const override;
        size extents() const override;
    protected:
        i_surface_window& surface_window() const override;
        virtual void set_destroying();
        void set_destroyed() override;
    private:
        virtual void display() = 0;
    private:
        i_surface_window& iSurfaceWindow;
        neogfx::logical_coordinate_system iLogicalCoordinateSystem;
        mutable std::optional<neogfx::logical_coordinates> iLogicalCoordinates;
        GLuint iFrameBuffer;
        mutable optional_texture iFrameBufferTexture;
        GLuint iDepthStencilBuffer;
        size iFrameBufferExtents;
        std::optional<rect> iInvalidatedArea;
        uint64_t iFrameCounter;
        uint64_t iLastFrameTime;
        std::deque<std::chrono::time_point<std::chrono::steady_clock>> iFpsData;
        bool iRendering;
        uint32_t iPaused;
    };
}