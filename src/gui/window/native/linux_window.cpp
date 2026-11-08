// linux_window.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_window_manager.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include "../../../app/native/linux_basic_services.hpp"
#include "linux_window.hpp"

namespace neogfx
{
    namespace native::linux
    {
        namespace
        {
            // (as the hit test codes Windows sends non-client mouse messages for: see windows_window.cpp)
            bool is_non_client(widget_part aPart)
            {
                switch (aPart.part)
                {
                case widget_part::Client:
                case widget_part::NonClient:
                case widget_part::NonClientOther:
                case widget_part::Nowhere:
                case widget_part::NowhereError:
                case widget_part::Invalid:
                    return false;
                default:
                    return true;
                }
            }

            bool is_border(widget_part_e aPart)
            {
                switch (aPart)
                {
                case widget_part::BorderLeft:
                case widget_part::BorderTopLeft:
                case widget_part::BorderTop:
                case widget_part::BorderTopRight:
                case widget_part::BorderRight:
                case widget_part::BorderBottomRight:
                case widget_part::BorderBottom:
                case widget_part::BorderBottomLeft:
                case widget_part::GrowBox:
                    return true;
                default:
                    return false;
                }
            }

            mouse_system_cursor non_client_cursor(widget_part_e aPart)
            {
                switch (aPart)
                {
                case widget_part::BorderLeft:
                case widget_part::BorderRight:
                    return mouse_system_cursor::SizeWE;
                case widget_part::BorderTop:
                case widget_part::BorderBottom:
                    return mouse_system_cursor::SizeNS;
                case widget_part::BorderTopLeft:
                case widget_part::BorderBottomRight:
                case widget_part::GrowBox:
                    return mouse_system_cursor::SizeNWSE;
                case widget_part::BorderTopRight:
                case widget_part::BorderBottomLeft:
                    return mouse_system_cursor::SizeNESW;
                default:
                    return mouse_system_cursor::Arrow;
                }
            }
        }

        window::window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window* aParent, window_style aStyle) :
            native_window{ aRenderingEngine, aSurfaceManager, aWindow },
            iParent{ aParent },
            iStyle{ aStyle }
        {
        }

        void window::attach(i_native_surface& aSurface)
        {
            native_window::attach(aSurface);
            iSurfaceSink = aSurface.target_activated([&]()
                {
                    if (iFirstActivation)
                    {
                        iFirstActivation = false;
                        request_paint();
                    }
                });
        }

        void window::detach()
        {
            native_window::detach();
            iFirstActivation = true;
        }

        bool window::has_parent() const
        {
            return iParent != nullptr;
        }

        const i_native_window& window::parent() const
        {
            if (has_parent())
                return *iParent;
            throw no_parent();
        }

        i_native_window& window::parent()
        {
            return const_cast<i_native_window&>(to_const(*this).parent());
        }

        bool window::is_nested() const
        {
            return false;
        }

        bool window::initialising() const
        {
            return !iReady;
        }

        void window::initialisation_complete()
        {
            iReady = true;
        }

        point window::surface_position() const
        {
            return iPosition;
        }

        size window::surface_extents() const
        {
            return iExtents;
        }

        bool window::visible() const
        {
            return iVisible;
        }

        double window::transparency() const
        {
            return 1.0 - opacity();
        }

        void window::set_transparency(double aTransparency)
        {
            set_opacity(1.0 - aTransparency);
        }

        bool window::is_active() const
        {
            return is_effectively_active() && iActive;
        }

        void window::activate()
        {
            if ((surface_window().style() & window_style::NoActivate) == window_style::NoActivate)
                return;
            if (!enabled())
                return;
            if (!visible())
                show();
            // n.b. which window has the keyboard focus is for the display server (and its window manager) to decide
            if (!iActive)
            {
                iActive = true;
                surface_window().as_window().activated()();
                surface_window().as_widget().update(true);
            }
        }

        void window::deactivate()
        {
            if (iActive)
            {
                iActive = false;
                surface_window().as_window().deactivated()();
                surface_window().as_widget().update(true);
            }
        }

        bool window::is_iconic() const
        {
            return iIconic;
        }

        bool window::is_maximized() const
        {
            return iMaximized;
        }

        bool window::is_restored() const
        {
            return !is_iconic() && !is_maximized();
        }

        bool window::is_fullscreen() const
        {
            return (iStyle & window_style::Fullscreen) == window_style::Fullscreen;
        }

        bool window::enabled() const
        {
            return iEnabled;
        }

        void window::enable(bool aEnable)
        {
            if (iEnabled == aEnable)
                return;
            iEnabled = aEnable;
            if (aEnable)
                push_event(window_event(window_event_type::Enabled));
            else
                push_event(window_event(window_event_type::Disabled));
        }

        bool window::is_capturing() const
        {
            return iCapturingMouse;
        }

        void window::set_capture(bool aLockCursor)
        {
            if (!iCapturingMouse)
            {
                iCapturingMouse = true;
                iNonClientCapturing = false;
                service<i_mouse>().capture(surface_window(), aLockCursor);
            }
        }

        void window::release_capture()
        {
            if (iCapturingMouse)
            {
                iCapturingMouse = false;
                iNonClientCapturing = false;
                service<i_mouse>().release_capture();
            }
        }

        bool window::is_non_client_capturing() const
        {
            return iNonClientCapturing;
        }

        void window::non_client_set_capture()
        {
            if (!iCapturingMouse)
            {
                iCapturingMouse = true;
                iNonClientCapturing = true;
                service<i_mouse>().capture_raw(surface_window());
            }
        }

        void window::non_client_release_capture()
        {
            if (iCapturingMouse)
            {
                iCapturingMouse = false;
                iNonClientCapturing = false;
                service<i_mouse>().release_capture();
            }
        }

        border window::border_thickness() const
        {
            if (is_maximized() || is_fullscreen())
                iBorderThickness = border{};
            else if ((surface_window().style() & window_style::Resize) == window_style::Resize)
            {
                // (as the resizable frame of a Windows window (its size frame plus padded border) at the same DPI)
                iBorderThickness = border{ 8.0, 8.0, 8.0, 8.0 } * std::max<scalar>(1.0, std::round(pixel_density().cx / 96.0));
                iBorderThickness += service<i_app>().current_style().padding(padding_role::Window);
            }
            else
                iBorderThickness = border{ 1.0, 1.0, 1.0, 1.0 };
            return iBorderThickness;
        }

        void window::render(bool aOOBRequest)
        {
            if (!can_render())
                return;
            attachment().render(aOOBRequest);
        }

        bool window::is_popup() const
        {
            return iParent != nullptr && (iStyle & window_style::Popup) == window_style::Popup;
        }

        window* window::toplevel()
        {
            auto w = this;
            while (w->is_popup())
                w = w->iParent;
            return w;
        }

        linux::display_server& window::display_server() const
        {
            return static_cast<linux::basic_services&>(service<i_basic_services>()).display_server();
        }

        void window::pointer_entered(point const& aPosition)
        {
            pointer_moved(aPosition);
        }

        void window::pointer_left()
        {
            if (iMouseEntered)
            {
                iMouseEntered = false;
                handle_event(window_event{ window_event_type::Leave });
            }
            if (non_client_entered())
            {
                if (surface_window().has_capturing_widget())
                    surface_window().release_capture(surface_window().capturing_widget());
                else
                    release_capture();
                handle_event(window_event{ window_event_type::NonClientLeave });
            }
        }

        void window::pointer_moved(point const& aPosition)
        {
            iPointerPosition = aPosition;
            if (!enabled())
                return;
            scoped_units su{ surface_window().as_widget(), units::Pixels };
            auto const modifiers = service<i_keyboard>().modifiers();
            bool const nonClient = iNonClientCapturing || (!iCapturingMouse && custom_decoration() && is_non_client(surface_window().native_window_hit_test(aPosition)));
            if (nonClient)
            {
                if (iMouseEntered)
                {
                    iMouseEntered = false;
                    handle_event(window_event{ window_event_type::Leave });
                }
                if (!non_client_entered())
                    push_event(window_event{ window_event_type::NonClientEnter, aPosition });
                push_event(non_client_mouse_event{ mouse_event_type::Moved, aPosition, {}, modifiers });
                // (the cursor DefWindowProc would show on Windows)
                if (!iNonClientCapturing)
                    service<i_window_manager>().set_mouse_cursor(non_client_cursor(surface_window().native_window_hit_test(aPosition).part), false);
                return;
            }
            if (!iMouseEntered)
            {
                iMouseEntered = true;
                handle_event(window_event{ window_event_type::Enter, aPosition });
            }
            push_event(mouse_event{ mouse_event_type::Moved, aPosition, display_server().pointer_buttons(), modifiers });
            service<i_window_manager>().update_mouse_cursor(surface_window().as_window());
        }

        void window::pointer_button(mouse_button aButton, bool aPressed, std::uint32_t aSerial)
        {
            if (!enabled())
                return;
            scoped_units su{ surface_window().as_widget(), units::Pixels };
            auto const position = iPointerPosition;
            auto const modifiers = service<i_keyboard>().modifiers();
            bool const nonClient = iNonClientCapturing || (!iCapturingMouse && custom_decoration() && is_non_client(surface_window().native_window_hit_test(position)));
            if (nonClient)
            {
                handle_non_client_button(aButton, aPressed, position, aSerial);
                return;
            }
            // n.b. the release's click event is made from the press here (not from the native window's previous mouse
            // click event as on Windows) as the press has not necessarily been handled yet (the display server sends
            // events in batches)
            if (aPressed)
            {
                auto const now = std::chrono::steady_clock::now();
                bool const doubleClick = iLastButtonPress && iLastButtonPress->button == aButton &&
                    now - iLastButtonPress->time <= double_click_time() &&
                    std::abs(position.x - iLastButtonPress->position.x) <= DOUBLE_CLICK_DISTANCE &&
                    std::abs(position.y - iLastButtonPress->position.y) <= DOUBLE_CLICK_DISTANCE;
                if (doubleClick)
                    iLastButtonPress = std::nullopt;
                else
                    iLastButtonPress = button_press{ aButton, position, now };
                iPress = press{ aButton, position, doubleClick };
                push_event(mouse_event{ doubleClick ? mouse_event_type::ButtonDoubleClicked : mouse_event_type::ButtonClicked, position, aButton, modifiers });
            }
            else
            {
                if (iPress && iPress->button == aButton &&
                    &widget_for_mouse_event(iPress->position) == &widget_for_mouse_event(position))
                    push_event(mouse_event{ iPress->doubleClick ? mouse_event_type::ButtonDoubleClick : mouse_event_type::ButtonClick, position, aButton, modifiers });
                iPress = std::nullopt;
                push_event(mouse_event{ mouse_event_type::ButtonReleased, position, aButton, modifiers });
            }
        }

        void window::pointer_wheel(mouse_wheel aWheel, delta const& aDelta)
        {
            if (!enabled())
                return;
            push_event(mouse_event{ mouse_event_type::WheelScrolled, iPointerPosition, aWheel, service<i_keyboard>().modifiers(), aDelta });
        }

        void window::keyboard_focus_gained()
        {
            set_iconic(false);
            handle_event(window_event{ window_event_type::FocusGained });
        }

        void window::keyboard_focus_lost()
        {
            if (is_alive())
                handle_event(window_event{ window_event_type::FocusLost });
        }

        void window::set_destroying()
        {
            native_window::set_destroying();
            release_capture();
            surface_window().native_window_closing();
        }

        void window::set_destroyed()
        {
            native_window::set_destroyed();
            surface_window().native_window_closed();
        }

        bool window::custom_decoration() const
        {
            return (surface_window().style() & window_style::TitleBar) == window_style::TitleBar;
        }

        void window::request_paint()
        {
            if (initialising())
                return;
            if (attached())
                attachment().invalidate(rect{ point{}, surface_extents() });
            push_event(window_event{ window_event_type::Paint });
        }

        void window::set_position(point const& aPosition, bool aExternalCause)
        {
            if (iPosition == aPosition)
                return;
            iPosition = aPosition;
            position_changed();
            push_event(window_event{ window_event_type::Moved, iPosition }.with_external_cause(aExternalCause));
        }

        void window::set_extents(size const& aExtents, bool aExternalCause)
        {
            if (iExtents == aExtents)
                return;
            iExtents = aExtents;
            extents_changed();
            push_event(window_event{ window_event_type::Resized, iExtents }.with_external_cause(aExternalCause));
        }

        void window::set_maximized(bool aMaximized)
        {
            if (iMaximized == aMaximized)
                return;
            iMaximized = aMaximized;
            push_event(window_event{ aMaximized ? window_event_type::Maximized : window_event_type::Restored, surface_position() });
        }

        void window::set_iconic(bool aIconic)
        {
            if (iIconic == aIconic)
                return;
            iIconic = aIconic;
            push_event(window_event{ aIconic ? window_event_type::Iconized : window_event_type::Restored, surface_position() });
        }

        void window::handle_non_client_button(mouse_button aButton, bool aPressed, point const& aPosition, std::uint32_t aSerial)
        {
            auto const modifiers = service<i_keyboard>().modifiers();
            auto const part = surface_window().native_window_hit_test(aPosition).part;
            // as the non-client mouse messages on Windows...
            if (aPressed)
            {
                auto const now = std::chrono::steady_clock::now();
                bool const doubleClick = iLastButtonPress && iLastButtonPress->button == aButton &&
                    now - iLastButtonPress->time <= double_click_time() &&
                    std::abs(aPosition.x - iLastButtonPress->position.x) <= DOUBLE_CLICK_DISTANCE &&
                    std::abs(aPosition.y - iLastButtonPress->position.y) <= DOUBLE_CLICK_DISTANCE;
                if (doubleClick)
                    iLastButtonPress = std::nullopt;
                else
                    iLastButtonPress = button_press{ aButton, aPosition, now };
                iPress = press{ aButton, aPosition, doubleClick };
                destroyed_flag destroyed{ *this };
                handle_event(non_client_mouse_event{ doubleClick ? mouse_event_type::ButtonDoubleClicked : mouse_event_type::ButtonClicked, aPosition, aButton, modifiers });
                if (destroyed)
                    return;
                // ...and as DefWindowProc then is
                surface_window().native_window_dismiss_children();
                if (aButton == mouse_button::Left)
                {
                    switch (part)
                    {
                    case widget_part::TitleBar:
                    case widget_part::Grab:
                        if (doubleClick && (iStyle & window_style::Resize) == window_style::Resize)
                        {
                            if (is_maximized())
                                restore();
                            else
                                maximize();
                        }
                        else if (!is_popup())
                            begin_move(aPosition, aSerial);
                        break;
                    case widget_part::SystemMenu:
                        if (doubleClick)
                            handle_event(window_event{ window_event_type::Close });
                        else
                            iClickedWidgetPart = widget_part::SystemMenu;
                        break;
                    case widget_part::MinimizeButton:
                    case widget_part::MaximizeButton:
                    case widget_part::RestoreButton:
                    case widget_part::CloseButton:
                        iClickedWidgetPart = part;
                        break;
                    default:
                        if (is_border(part) && !is_popup() && !is_maximized() && (iStyle & window_style::Resize) == window_style::Resize)
                            begin_resize(part, aPosition, aSerial);
                        break;
                    }
                }
                else if (aButton == mouse_button::Right && (part == widget_part::TitleBar || part == widget_part::Grab))
                    iClickedWidgetPart = widget_part::TitleBar;
            }
            else
            {
                destroyed_flag destroyed{ *this };
                if (iPress && iPress->button == aButton &&
                    &widget_for_mouse_event(iPress->position) == &widget_for_mouse_event(aPosition))
                    handle_event(non_client_mouse_event{ iPress->doubleClick ? mouse_event_type::ButtonDoubleClick : mouse_event_type::ButtonClick, aPosition, aButton, modifiers });
                iPress = std::nullopt;
                if (destroyed)
                    return;
                handle_event(non_client_mouse_event{ mouse_event_type::ButtonReleased, aPosition, aButton, modifiers });
                if (destroyed)
                    return;
                auto const clicked = iClickedWidgetPart;
                iClickedWidgetPart = widget_part::Nowhere;
                if (aButton == mouse_button::Left && clicked == part)
                {
                    switch (part)
                    {
                    case widget_part::MinimizeButton:
                        iconize();
                        break;
                    case widget_part::MaximizeButton:
                    case widget_part::RestoreButton:
                        if (is_maximized())
                            restore();
                        else
                            maximize();
                        break;
                    case widget_part::CloseButton:
                        handle_event(window_event{ window_event_type::Close });
                        break;
                    case widget_part::SystemMenu:
                        show_window_menu(aPosition, aSerial);
                        break;
                    default:
                        break;
                    }
                }
                else if (aButton == mouse_button::Right && clicked == widget_part::TitleBar)
                    show_window_menu(aPosition, aSerial);
            }
        }
    }
}
