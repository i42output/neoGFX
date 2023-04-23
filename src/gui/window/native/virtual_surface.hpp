// virtual_surface.hpp
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
#include <boost/lexical_cast.hpp>

#include <neolib/core/string_utils.hpp>

#include <neogfx/neogfx.hpp>
#include <neogfx/core/object.hpp>
#include <neogfx/gui/widget/timer.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/hid/i_native_surface.hpp>

namespace neogfx
{
    class i_surface_window;

    class virtual_surface : public reference_counted<object<i_native_surface>>
    {
        typedef reference_counted<object<i_native_surface>> base_type;
    public:
        define_declared_event(TargetActivating, target_activating)
        define_declared_event(TargetActivated, target_activated)
        define_declared_event(TargetDeactivating, target_deactivating)
        define_declared_event(TargetDeactivated, target_deactivated)
    public:
        virtual_surface(i_rendering_engine& aRenderingEngine, i_surface_window& aWindow);
        ~virtual_surface();
    public:
        i_surface_window& surface_window() const;
        i_native_surface& parent() const;
    public:
        render_target_type target_type() const final;
        void* target_handle() const final;
        void* target_device_handle() const final;
        pixel_format_t pixel_format() const final;
        const i_texture& target_texture() const final;
        point target_origin() const final;
        size target_extents() const final;
    public:
        dimension horizontal_dpi() const final;
        dimension vertical_dpi() const final;
        dimension ppi() const final;
    public:
        bool metrics_available() const final;
        size extents() const final;
        dimension em_size() const final;
    public:
        neogfx::logical_coordinate_system logical_coordinate_system() const final;
        void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) final;
        neogfx::logical_coordinates logical_coordinates() const final;
        void set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates) final;
    public:
        rect_i32 viewport() const final;
        rect_i32 set_viewport(const rect_i32& aViewport) const final;
    public:
        bool target_active() const final;
        void activate_target() const final;
        void deactivate_target() const final;
    public:
        neogfx::color_space color_space() const final;
        color read_pixel(const point& aPosition) const final;
    public:
        std::unique_ptr<i_rendering_context> create_graphics_context(blending_mode aBlendingMode) const final;
        std::unique_ptr<i_rendering_context> create_graphics_context(const i_widget& aWidget, blending_mode aBlendingMode) const final;
    public:
        uint64_t frame_counter() const final;
        double fps() const final;
        double potential_fps() const final;
    public:
        void invalidate(const rect& aInvalidatedRect) final;
        bool has_invalidated_area() const final;
        const rect& invalidated_area() const final;
        rect validate() final;
        bool can_render() const final;
        void render(bool aOOBRequest = false) final;
        void pause() final;
        void resume() final;
        bool is_rendering() const final;
    public:
        void debug(bool aEnableDebug) final;
    private:
        i_rendering_engine& iRenderingEngine;
        i_surface_window& iWindow;
        neogfx::logical_coordinate_system iLogicalCoordinateSystem;
        mutable std::optional<neogfx::logical_coordinates> iLogicalCoordinates;
        bool iDebug;
    };
}