// virtual_window.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include "virtual_window.hpp"

namespace neogfx
{
    virtual_window::virtual_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_window& aParent, const basic_size<int>& aDimensions, std::string const& aWindowTitle, window_style aStyle) :
        native_window{ aRenderingEngine, aSurfaceManager },
        iSurfaceWindow{ aWindow },
        iParent{ aParent },
        iLogicalCoordinateSystem{ neogfx::logical_coordinate_system::AutomaticGui },
        iPosition{},
        iExtents{ aDimensions },
        iEnabled{ true },
        iVisible{ (aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden },
        iOpacity{ 1.0 },
        iActive{ false },
        iState{ window_state::Normal },
        iMouseEntered{ false },
        iCapturingMouse{ false },
        iNonClientCapturing{ false },
        iReady{ false },
        iClickedWidgetPart{ widget_part::Nowhere },
        iSystemMenuOpen{ false },
        iDebug{ false }
    {
    }

    virtual_window::virtual_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_window& aParent, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, std::string const& aWindowTitle, window_style aStyle) :
        native_window{ aRenderingEngine, aSurfaceManager },
        iSurfaceWindow{ aWindow },
        iParent{ aParent },
        iLogicalCoordinateSystem{ neogfx::logical_coordinate_system::AutomaticGui },
        iPosition{ aPosition },
        iExtents{ aDimensions },
        iEnabled{ true },
        iVisible{ (aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden },
        iOpacity{ 1.0 },
        iActive{ false },
        iState{ window_state::Normal },
        iMouseEntered{ false },
        iCapturingMouse{ false },
        iNonClientCapturing{ false },
        iReady{ false },
        iClickedWidgetPart{ widget_part::Nowhere },
        iSystemMenuOpen{ false },
        iDebug{ false }
    {
    }

    virtual_window::~virtual_window()
    {
        set_destroyed();
    }

    render_target_type virtual_window::target_type() const
    {
        return render_target_type::Surface;
    }

    const i_texture& virtual_window::target_texture() const
    {
        return parent().target_texture();
    }

    point virtual_window::target_origin() const
    {
        return iPosition + parent().target_origin();
    }

    size virtual_window::target_extents() const
    {
        return iExtents;
    }

    void virtual_window::activate_target() const
    {
        parent().activate_target();
    }

    bool virtual_window::target_active() const
    {
        return parent().target_active();
    }

    void virtual_window::deactivate_target() const
    {
        parent().deactivate_target();
    }

    color_space virtual_window::color_space() const
    {
        return surface_manager().display(surface_window()).color_space();
    }

    color virtual_window::read_pixel(const point& aPosition) const
    {
        return parent().read_pixel(aPosition + surface_position());
    }

    neogfx::logical_coordinate_system virtual_window::logical_coordinate_system() const
    {
        return iLogicalCoordinateSystem;
    }

    void virtual_window::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem)
    {
        iLogicalCoordinateSystem = aSystem;
    }

    logical_coordinates virtual_window::logical_coordinates() const
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
            result.bottomLeft = vec2{ 0.0, extents().cy };
            result.topRight = vec2{ extents().cx, 0.0 };
            break;
        case neogfx::logical_coordinate_system::AutomaticGame:
            result.bottomLeft = vec2{ 0.0, 0.0 };
            result.topRight = vec2{ extents().cx, extents().cy };
            break;
        }
        return result;
    }

    void virtual_window::set_logical_coordinates(const neogfx::logical_coordinates& aCoordinates)
    {
        iLogicalCoordinates = aCoordinates;
    }

    uint64_t virtual_window::frame_counter() const
    {
        return parent().frame_counter();
    }

    double virtual_window::fps() const
    {
        return parent().fps();
    }

    double virtual_window::potential_fps() const
    {
        return parent().potential_fps();
    }

    void virtual_window::invalidate(const rect& aInvalidatedRect)
    {
        parent().invalidate(aInvalidatedRect);
    }

    bool virtual_window::has_invalidated_area() const
    {
        return parent().has_invalidated_area();
    }

    const rect& virtual_window::invalidated_area() const
    {
        return parent().invalidated_area();
    }

    rect virtual_window::validate()
    {
        return parent().validate();
    }

    void virtual_window::render(bool aOOBRequest)
    {
        parent().render(aOOBRequest);
    }

    bool virtual_window::is_rendering() const
    {
        return parent().is_rendering();
    }

    void virtual_window::debug(bool aEnableDebug)
    {
        iDebug = aEnableDebug;
    }

    bool virtual_window::metrics_available() const
    {
        return true;
    }

    size virtual_window::extents() const
    {
        return surface_extents();
    }

    i_surface_window& virtual_window::surface_window() const
    {
        return iSurfaceWindow;
    }

    void virtual_window::set_destroying()
    {
        if (!is_alive())
            return;
        native_window::set_destroying();
        if (target_active())
            deactivate_target();
        release_capture();
        surface_window().native_window_closing();
    }

    void virtual_window::set_destroyed()
    {
        native_window::set_destroyed();
        surface_window().native_window_closed();
    }

    void* virtual_window::target_handle() const
    {
        return parent().target_handle();
    }

    void* virtual_window::target_device_handle() const
    {
        return parent().target_device_handle();
    }

    pixel_format_t virtual_window::pixel_format() const
    {
        return parent().pixel_format();
    }

    bool virtual_window::has_parent() const
    {
        return true;
    }

    const i_native_window& virtual_window::parent() const
    {
        return iParent;
    }

    i_native_window& virtual_window::parent()
    {
        return const_cast<i_native_window&>(to_const(*this).parent());
    }

    bool virtual_window::initialising() const
    {
        return false;
    }

    void virtual_window::initialisation_complete()
    {
    }

    void* virtual_window::handle() const
    {
        return parent().handle();
    }

    void* virtual_window::native_handle() const
    {
        return parent().native_handle();
    }

    point virtual_window::surface_position() const
    {
        return parent().surface_position();
    }

    void virtual_window::move_surface(const point& aPosition)
    {
        invalidate(rect{ surface_position(), surface_extents() });
        iPosition = aPosition;
        invalidate(rect{ surface_position(), surface_extents() });
    }

    size virtual_window::surface_extents() const
    {
        return iExtents;
    }

    void virtual_window::resize_surface(const size& aExtents)
    {
        invalidate(rect{ surface_position(), surface_extents() });
        iExtents = aExtents;
        invalidate(rect{ surface_position(), surface_extents() });
    }

    bool virtual_window::can_render() const
    {
        return visible() && native_window::can_render();
    }

    std::unique_ptr<i_rendering_context> virtual_window::create_graphics_context(blending_mode aBlendingMode) const
    {
        auto graphicsContext = parent().create_graphics_context(aBlendingMode);
        return graphicsContext;
    }

    std::unique_ptr<i_rendering_context> virtual_window::create_graphics_context(const i_widget& aWidget, blending_mode aBlendingMode) const
    {
        auto graphicsContext = parent().create_graphics_context(aWidget, aBlendingMode);
        return graphicsContext;
    }

    void virtual_window::close(bool aForce)
    {
        set_destroying();
        set_destroyed();
    }

    bool virtual_window::visible() const
    {
        return iVisible;
    }

    void virtual_window::show(bool aActivate)
    {
        iVisible = true;
        invalidate(rect{ surface_position(), surface_extents() });
    }

    void virtual_window::hide()
    {
        iVisible = false;
        invalidate(rect{ surface_position(), surface_extents() });
    }

    double virtual_window::opacity() const
    {
        return iOpacity;
    }

    void virtual_window::set_opacity(double aOpacity)
    {
        if (iOpacity != aOpacity)
        { 
            iOpacity = aOpacity;
            invalidate(rect{ surface_position(), surface_extents() });
        }
    }

    double virtual_window::transparency() const
    {
        return 1.0 - opacity();
    }

    void virtual_window::set_transparency(double aTransparency)
    {
        set_opacity(1.0 - aTransparency);
    }

    bool virtual_window::is_active() const
    {
        return parent().is_active() && iActive;
    }

    void virtual_window::activate()
    {
        if (!enabled())
            return;
        parent().activate();
        iActive = true;
    }

    bool virtual_window::is_iconic() const
    {
        return iState == window_state::Iconized;
    }

    void virtual_window::iconize()
    {
        if (iState != window_state::Iconized)
        {
            invalidate(rect{ surface_position(), surface_extents() });
            iState = window_state::Iconized;
            invalidate(rect{ surface_position(), surface_extents() });
        }
    }

    bool virtual_window::is_maximized() const
    {
        return iState == window_state::Maximized;
    }

    void virtual_window::maximize()
    {
        if (iState != window_state::Maximized)
        {
            invalidate(rect{ surface_position(), surface_extents() });
            iState = window_state::Maximized;
            invalidate(rect{ surface_position(), surface_extents() });
        }
    }

    bool virtual_window::is_restored() const
    {
        return iState == window_state::Normal;
    }

    void virtual_window::restore()
    {
        if (iState != window_state::Normal)
        {
            invalidate(rect{ surface_position(), surface_extents() });
            iState = window_state::Normal;
            invalidate(rect{ surface_position(), surface_extents() });
        }
    }

    bool virtual_window::is_fullscreen() const
    {
        return false;
    }

    void virtual_window::enter_fullscreen(const video_mode& aVideoMode)
    {
    }

    bool virtual_window::enabled() const
    {
        return iEnabled;
    }

    void virtual_window::enable(bool aEnable)
    {
        if (iEnabled != aEnable)
        {
            iEnabled = aEnable;
            if (aEnable)
                push_event(window_event(window_event_type::Enabled));
            else
                push_event(window_event(window_event_type::Disabled));
        }
    }

    bool virtual_window::is_capturing() const
    {
        return iCapturingMouse;
    }

    void virtual_window::set_capture()
    {
        if (!iCapturingMouse)
        {
            iCapturingMouse = true;
            iNonClientCapturing = false;
            service<i_mouse>().capture(surface_window());
        }
    }

    void virtual_window::release_capture()
    {
        if (iCapturingMouse)
        {
            iCapturingMouse = false;
            iNonClientCapturing = false;
            service<i_mouse>().release_capture();
        }
    }

    void virtual_window::non_client_set_capture()
    {
        if (!iCapturingMouse)
        {
            iCapturingMouse = true;
            iNonClientCapturing = true;
            service<i_mouse>().capture_raw(surface_window());
        }
    }

    void virtual_window::non_client_release_capture()
    {
        if (iCapturingMouse)
        {
            iCapturingMouse = false;
            iNonClientCapturing = false;
            service<i_mouse>().release_capture();
        }
    }

    void virtual_window::set_title_text(std::string const& aTitleText)
    {
    }

    padding virtual_window::border_thickness() const
    {
        iBorderThickness = padding{ 1.0, 1.0, 1.0, 1.0 };
        if ((surface_window().style() & window_style::Resize) == window_style::Resize)
            iBorderThickness += service<i_app>().current_style().padding(padding_role::Window);
        return iBorderThickness;
    }

    void virtual_window::debug_message(std::string const& aMessage)
    {
#ifdef NEOGFX_DEBUG
        if (iDebug)
            service<debug::logger>() << aMessage << endl;
#endif // NEOGFX_DEBUG
    }
}