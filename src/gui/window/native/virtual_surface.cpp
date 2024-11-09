// virtual_surface.cpp
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

#include <neogfx/neogfx.hpp>

#include <numeric>

#include <neolib/task/thread.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/gfx/i_rendering_context.hpp>
#include "virtual_surface.hpp"

namespace neogfx
{
    virtual_surface::virtual_surface(i_rendering_engine& aRenderingEngine, i_surface_window& aWindow) :
        iRenderingEngine{ aRenderingEngine },
        iWindow{ aWindow },
        iLogicalCoordinateSystem{ neogfx::logical_coordinate_system::AutomaticGui },
        iDebug{ false }
    {
    }

    virtual_surface::~virtual_surface()
    {
        set_destroyed();
    }

    i_surface_window& virtual_surface::surface_window() const
    {
        return iWindow;
    }

    i_native_surface& virtual_surface::parent() const
    {
        return surface_window().parent_surface().native_surface();
    }

    render_target_type virtual_surface::target_type() const
    {
        return parent().target_type();
    }

    void* virtual_surface::target_handle() const
    {
        return parent().target_handle();
    }

    void* virtual_surface::target_device_handle() const
    {
        return parent().target_device_handle();
    }

    pixel_format_t virtual_surface::pixel_format() const
    {
        return parent().pixel_format();
    }

    const i_texture& virtual_surface::target_texture() const
    {
        return parent().target_texture();
    }

    point virtual_surface::target_origin() const
    {
        return parent().target_origin();
    }

    size virtual_surface::target_extents() const
    {
        return parent().target_extents();
    }

    dimension virtual_surface::horizontal_dpi() const
    {
        return surface_window().native_window().pixel_density().cx;
    }

    dimension virtual_surface::vertical_dpi() const
    {
        return surface_window().native_window().pixel_density().cy;
    }

    dimension virtual_surface::ppi() const
    {
        return surface_window().native_window().pixel_density().magnitude() / std::sqrt(2.0);
    }

    bool virtual_surface::metrics_available() const
    {
        return true;
    }

    size virtual_surface::extents() const
    {
        return surface_window().native_window().surface_extents();
    }

    dimension virtual_surface::em_size() const
    {
        return 0; /* todo */
    }

    logical_coordinate_system virtual_surface::logical_coordinate_system() const
    {
        return iLogicalCoordinateSystem;
    }

    void virtual_surface::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem)
    {
        iLogicalCoordinateSystem = aSystem;
    }

    logical_coordinates virtual_surface::logical_coordinates() const
    {
        if (iLogicalCoordinates != std::nullopt)
            return *iLogicalCoordinates;
        neogfx::logical_coordinates result;
        switch (iLogicalCoordinateSystem)
        {
        case neogfx::logical_coordinate_system::Specified:
            throw logical_coordinates_not_specified();
            break;
        case neogfx::logical_coordinate_system::AutomaticGui:
            result.bottomLeft = vec2{ 0.0, parent().extents().cy };
            result.topRight = vec2{ parent().extents().cx, 0.0 };
            break;
        case neogfx::logical_coordinate_system::AutomaticGame:
            result.bottomLeft = vec2{ 0.0, 0.0 };
            result.topRight = vec2{ parent().extents().cx, parent().extents().cy };
            break;
        }
        return result;
    }

    void virtual_surface::set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates)
    {
        iLogicalCoordinates = aCoordinates;
    }

    rect_i32 virtual_surface::viewport() const
    {
        return parent().viewport();
    }

    rect_i32 virtual_surface::set_viewport(const rect_i32& aViewport) const
    {
        return parent().set_viewport(aViewport);
    }
    
    bool virtual_surface::target_active() const
    {
        return parent().target_active();
    }

    void virtual_surface::activate_target() const
    {
        parent().activate_target();
    }

    void virtual_surface::deactivate_target() const
    {
        parent().deactivate_target();
    }

    color_space virtual_surface::color_space() const
    {
        return service<i_surface_manager>().display(iWindow).color_space();
    }

    color virtual_surface::read_pixel(const point& aPosition) const
    {
        return parent().read_pixel(surface_window().surface_position() + aPosition);
    }

    std::unique_ptr<i_rendering_context> virtual_surface::create_graphics_context(blending_mode aBlendingMode) const
    {
        return parent().create_graphics_context(aBlendingMode);
    }

    std::unique_ptr<i_rendering_context> virtual_surface::create_graphics_context(const i_widget& aWidget, blending_mode aBlendingMode) const
    {
        return parent().create_graphics_context(aWidget, aBlendingMode);
    }

    std::size_t virtual_surface::ideal_graphics_operation_queue_capacity() const
    {
        return parent().ideal_graphics_operation_queue_capacity();
    }

    void virtual_surface::new_graphics_operation_queue_capacity(std::size_t aCapacity) const
    {
        parent().new_graphics_operation_queue_capacity(aCapacity);
    }

    std::uint64_t virtual_surface::frame_counter() const
    {
        return parent().frame_counter();
    }

    double virtual_surface::fps() const
    {
        return parent().fps();
    }

    double virtual_surface::potential_fps() const
    {
        return parent().potential_fps();
    }

    void virtual_surface::invalidate(const rect& aInvalidatedRect)
    {
        return parent().invalidate(aInvalidatedRect);
    }

    bool virtual_surface::has_invalidated_area() const
    {
        return parent().has_invalidated_area();
    }

    const rect& virtual_surface::invalidated_area() const
    {
        return parent().invalidated_area();
    }

    rect virtual_surface::validate()
    {
        return parent().validate();
    }

    bool virtual_surface::can_render() const
    {
        return parent().can_render();
    }

    void virtual_surface::render(bool aOOBRequest)
    {
        parent().render(aOOBRequest);
    }

    void virtual_surface::pause()
    {
        parent().pause();
    }

    void virtual_surface::resume()
    {
        parent().resume();
    }

    bool virtual_surface::is_rendering() const
    {
        return parent().is_rendering();
    }

    void virtual_surface::debug(bool aEnableDebug)
    {
        iDebug = aEnableDebug;
    }
}