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
#include <neogfx/hid/i_window_manager.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include "../../../gfx/native/opengl/opengl_rendering_context.hpp"
#include "virtual_window.hpp"

namespace neogfx
{
    virtual_window::virtual_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_window& aParent, const basic_size<int>& aDimensions, std::string const& aWindowTitle, window_style aStyle) :
        native_window{ aRenderingEngine, aSurfaceManager, aWindow },
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
        surface_window().set_native_window(*this);
    }

    virtual_window::virtual_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_window& aParent, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, std::string const& aWindowTitle, window_style aStyle) :
        native_window{ aRenderingEngine, aSurfaceManager, aWindow },
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
        surface_window().set_native_window(*this);
    }

    virtual_window::~virtual_window()
    {
        set_destroyed();
    }

    void virtual_window::set_destroying()
    {
        if (!is_alive())
            return;
        native_window::set_destroying();
        if (surface_window().native_surface().target_active())
            surface_window().native_surface().deactivate_target();
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

    bool virtual_window::is_nested() const
    {
        return true;
    }

    bool virtual_window::initialising() const
    {
        return !iReady;
    }

    void virtual_window::initialisation_complete()
    {
        iReady = true;
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
        return iPosition + service<i_surface_manager>().find_nest(*this).widget().origin();
    }

    void virtual_window::move_surface(const point& aPosition)
    {
        if (!initialising())
            surface_window().native_surface().invalidate(rect{ surface_position(), surface_extents()});
        iPosition = aPosition;
        as_widget().move(iPosition);
        if (!initialising())
            surface_window().native_surface().invalidate(rect{ surface_position(), surface_extents() });
    }

    size virtual_window::surface_extents() const
    {
        return iExtents;
    }

    void virtual_window::resize_surface(const size& aExtents)
    {
        if (!initialising())
            surface_window().native_surface().invalidate(rect{ surface_position(), surface_extents() });
        iExtents = aExtents;
        as_widget().resize(iExtents);
        if (!initialising())
            surface_window().native_surface().invalidate(rect{ surface_position(), surface_extents() });
    }

    bool virtual_window::resizing_or_moving() const
    {
        // todo
        return false;
    }

    bool virtual_window::can_render() const
    {
        return visible() && parent().can_render();
    }

    void virtual_window::render(bool aOOBRequest)
    {
        parent().render(aOOBRequest);
    }

    void virtual_window::display()
    {
        parent().display();
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
        if (iVisible)
            return;
        iVisible = true;
        surface_window().native_surface().invalidate(rect{ surface_position(), surface_extents() });
        if (aActivate)
            activate();
    }

    void virtual_window::hide()
    {
        if (!iVisible)
            return;
        iVisible = false;
        surface_window().native_surface().invalidate(rect{ surface_position(), surface_extents() });
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
            surface_window().native_surface().invalidate(rect{ surface_position(), surface_extents() });
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

    bool virtual_window::is_effectively_active() const
    {
        return iActive;
    }

    bool virtual_window::is_active() const
    {
        return iActive;
    }

    void virtual_window::activate()
    {
        if ((surface_window().style() & window_style::NoActivate) == window_style::NoActivate)
            return;
        if (!enabled())
            return;
        if (is_active())
            return;
        if (!visible())
            show();

        auto& parentWindow = parent();
        if (!parentWindow.is_effectively_active())
            parentWindow.activate();
        if (!parentWindow.is_effectively_active())
            return;

        iActive = true;
        surface_window().as_window().activated()();
        parentWindow.surface_window().as_widget().update(true);
        surface_window().as_widget().update(true);
    }

    void virtual_window::deactivate()
    {
        iActive = false;
        surface_window().as_window().deactivated()();
        parent().surface_window().as_widget().update(true);
        surface_window().as_widget().update(true);
    }

    bool virtual_window::is_iconic() const
    {
        return iState == window_state::Iconized;
    }

    void virtual_window::iconize()
    {
        if (iState != window_state::Iconized)
        {
            attachment().invalidate(rect{ surface_position(), surface_extents() });
            iState = window_state::Iconized;
            attachment().invalidate(rect{ surface_position(), surface_extents() });
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
            attachment().invalidate(rect{ surface_position(), surface_extents() });
            iState = window_state::Maximized;
            attachment().invalidate(rect{ surface_position(), surface_extents() });
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
            attachment().invalidate(rect{ surface_position(), surface_extents() });
            iState = window_state::Normal;
            attachment().invalidate(rect{ surface_position(), surface_extents() });
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

    void virtual_window::set_title_text(i_string const& aTitleText)
    {
    }

    border virtual_window::border_thickness() const
    {
        iBorderThickness = border{ 1.0, 1.0, 1.0, 1.0 };
        if ((surface_window().style() & window_style::Resize) == window_style::Resize)
        {
            iBorderThickness += service<i_app>().current_style().border(border_role::Window);
            iBorderThickness += service<i_app>().current_style().padding(padding_role::Window);
        }
        return iBorderThickness;
    }

    i_widget const& virtual_window::as_widget() const
    {
        return iSurfaceWindow.as_widget();
    }

    i_widget& virtual_window::as_widget()
    {
        return iSurfaceWindow.as_widget();
    }

    void virtual_window::debug_message(std::string const& aMessage)
    {
#ifdef NEOGFX_DEBUG
        if (iDebug)
            service<debug::logger>() << neolib::logger::severity::Debug << aMessage << std::endl;
#endif // NEOGFX_DEBUG
    }
}