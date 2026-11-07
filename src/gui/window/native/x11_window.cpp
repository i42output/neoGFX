// x11_window.cpp
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

#include <unistd.h>

#include <neolib/core/scoped.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_window_manager.hpp>
#include <neogfx/hid/i_display.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include "../../../app/native/linux_basic_services.hpp"
#include "../../../app/native/x11_display_server.hpp"
#include "../../../gfx/native/linux_renderer.hpp"
#include "x11_window.hpp"
#include "../../../app/native/x11.hpp"

namespace neogfx
{
    namespace native::x11
    {
        namespace
        {
            display_server& the_display_server()
            {
                return static_cast<display_server&>(static_cast<linux::basic_services&>(service<i_basic_services>()).display_server());
            }

            // (as the _NET_WM_MOVERESIZE directions)
            std::optional<long> move_resize_direction(widget_part_e aPart)
            {
                switch (aPart)
                {
                case widget_part::BorderTopLeft:
                    return 0;
                case widget_part::BorderTop:
                    return 1;
                case widget_part::BorderTopRight:
                    return 2;
                case widget_part::BorderRight:
                    return 3;
                case widget_part::BorderBottomRight:
                case widget_part::GrowBox:
                    return 4;
                case widget_part::BorderBottom:
                    return 5;
                case widget_part::BorderBottomLeft:
                    return 6;
                case widget_part::BorderLeft:
                    return 7;
                default:
                    return {};
                }
            }

            constexpr long MOVE_RESIZE_MOVE = 8;

            struct motif_wm_hints
            {
                unsigned long flags;
                unsigned long functions;
                unsigned long decorations;
                long inputMode;
                unsigned long status;
            };
            constexpr unsigned long MWM_HINTS_DECORATIONS = 1ul << 1;

            constexpr long EVENT_MASK = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                EnterWindowMask | LeaveWindowMask | FocusChangeMask | ExposureMask | StructureNotifyMask | PropertyChangeMask;
        }

        window::window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const video_mode& aVideoMode, string const& aWindowTitle, window_style aStyle) :
            linux::window{ aRenderingEngine, aSurfaceManager, aWindow, nullptr, aStyle },
            iDisplayServer{ the_display_server() }
        {
            iExtents = basic_size<std::uint32_t>{ aVideoMode.resolution() };
            init(aWindowTitle);
            enter_fullscreen(aVideoMode);
            if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
                show((aStyle & window_style::NoActivate) != window_style::NoActivate);
        }

        window::window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const basic_size<int>& aDimensions, string const& aWindowTitle, window_style aStyle) :
            linux::window{ aRenderingEngine, aSurfaceManager, aWindow, nullptr, aStyle },
            iDisplayServer{ the_display_server() }
        {
            iPosition = aSurfaceManager.display(0).desktop_rect().center() - point{ size{ aDimensions } / 2.0 };
            iExtents = aDimensions;
            init(aWindowTitle);
            if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
                show((aStyle & window_style::NoActivate) != window_style::NoActivate);
        }

        window::window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, string const& aWindowTitle, window_style aStyle) :
            linux::window{ aRenderingEngine, aSurfaceManager, aWindow, nullptr, aStyle },
            iDisplayServer{ the_display_server() }
        {
            iPosition = aPosition;
            iExtents = aDimensions;
            init(aWindowTitle);
            if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
                show((aStyle & window_style::NoActivate) != window_style::NoActivate);
        }

        window::window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window& aParent, const video_mode& aVideoMode, string const& aWindowTitle, window_style aStyle) :
            linux::window{ aRenderingEngine, aSurfaceManager, aWindow, &aParent, aStyle },
            iDisplayServer{ the_display_server() }
        {
            iExtents = basic_size<std::uint32_t>{ aVideoMode.resolution() };
            init(aWindowTitle);
            enter_fullscreen(aVideoMode);
            if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
                show((aStyle & window_style::NoActivate) != window_style::NoActivate);
        }

        window::window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window& aParent, const basic_size<int>& aDimensions, string const& aWindowTitle, window_style aStyle) :
            linux::window{ aRenderingEngine, aSurfaceManager, aWindow, &aParent, aStyle },
            iDisplayServer{ the_display_server() }
        {
            iPosition = aSurfaceManager.display(0).desktop_rect().center() - point{ size{ aDimensions } / 2.0 };
            iExtents = aDimensions;
            init(aWindowTitle);
            if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
                show((aStyle & window_style::NoActivate) != window_style::NoActivate);
        }

        window::window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window& aParent, const basic_point<int>& aPosition, const basic_size<int>& aDimensions, string const& aWindowTitle, window_style aStyle) :
            linux::window{ aRenderingEngine, aSurfaceManager, aWindow, &aParent, aStyle },
            iDisplayServer{ the_display_server() }
        {
            iPosition = aPosition;
            iExtents = aDimensions;
            init(aWindowTitle);
            if ((aStyle & window_style::InitiallyHidden) != window_style::InitiallyHidden)
                show((aStyle & window_style::NoActivate) != window_style::NoActivate);
        }

        window::~window()
        {
            close(true);
            // (the surface's (Vulkan or EGL) resources go before the window they were made for)
            native_window::detach();
            if (iWindow != 0ul)
            {
                auto& renderer = static_cast<linux::renderer&>(rendering_engine());
                if (!renderer.vulkan())
                    renderer.destroy_window_surface(target_handle());
                ::XDestroyWindow(iDisplayServer.display(), iWindow);
                iDisplayServer.flush();
            }
        }

        void* window::target_handle() const
        {
            return reinterpret_cast<void*>(iWindow);
        }

        void* window::target_device_handle() const
        {
            return iDisplayServer.display();
        }

        void* window::handle() const
        {
            return reinterpret_cast<void*>(iWindow);
        }

        void* window::native_handle() const
        {
            return reinterpret_cast<void*>(iWindow);
        }

        void window::move_surface(const point& aPosition)
        {
            set_position(aPosition, false);
            auto const position = aPosition.as<int>();
            ::XMoveWindow(iDisplayServer.display(), iWindow, position.x, position.y);
        }

        void window::resize_surface(const size& aExtents)
        {
            set_extents(aExtents, false);
            auto const extents = aExtents.max(size{ 1.0, 1.0 }).as<int>();
            if ((iStyle & window_style::Resize) != window_style::Resize)
                update_size_hints();
            ::XResizeWindow(iDisplayServer.display(), iWindow, static_cast<unsigned int>(extents.cx), static_cast<unsigned int>(extents.cy));
        }

        bool window::resizing_or_moving() const
        {
            return iResizingOrMoving;
        }

        void window::close(bool aForce)
        {
            if (iWindow != 0ul && is_alive() && (aForce || surface_window().native_window_can_close()))
            {
                set_destroying();
                if (iInputContext != nullptr)
                {
                    ::XDestroyIC(iInputContext);
                    iInputContext = nullptr;
                }
                ::XUnmapWindow(iDisplayServer.display(), iWindow);
                iDisplayServer.remove_window(*this);
                iDisplayServer.flush();
                set_destroyed();
            }
        }

        void window::show(bool aActivate)
        {
            if (iVisible)
                return;
            iVisible = true;
            update_size_hints();
            ::XMapWindow(iDisplayServer.display(), iWindow);
            if (iOverrideRedirect)
                ::XRaiseWindow(iDisplayServer.display(), iWindow);
            iDisplayServer.flush();
            if (aActivate)
                activate();
        }

        void window::hide()
        {
            if (!iVisible)
                return;
            iVisible = false;
            if (iOverrideRedirect)
                ::XUnmapWindow(iDisplayServer.display(), iWindow);
            else
                ::XWithdrawWindow(iDisplayServer.display(), iWindow, iDisplayServer.screen());
            iDisplayServer.flush();
        }

        double window::opacity() const
        {
            return iOpacity;
        }

        void window::set_opacity(double aOpacity)
        {
            iOpacity = std::clamp(aOpacity, 0.0, 1.0);
            auto const property = iDisplayServer.atom(display_server::atom_id::NET_WM_WINDOW_OPACITY);
            if (iOpacity == 1.0)
                ::XDeleteProperty(iDisplayServer.display(), iWindow, property);
            else
            {
                unsigned long const value = static_cast<unsigned long>(iOpacity * 0xFFFFFFFFul);
                ::XChangeProperty(iDisplayServer.display(), iWindow, property, XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<unsigned char const*>(&value), 1);
            }
        }

        bool window::is_effectively_active() const
        {
            if (is_popup())
                return iParent->is_effectively_active();
            return iDisplayServer.keyboard_focus() == this;
        }

        void window::activate()
        {
            linux::window::activate();
            if (iOverrideRedirect || !visible() || !enabled() || (surface_window().style() & window_style::NoActivate) == window_style::NoActivate)
                return;
            if (iDisplayServer.keyboard_focus() == this)
                return;
            // (asked of the window manager, as an application's request; 1 is "an application" as the source)
            if (iDisplayServer.supported(display_server::atom_id::NET_ACTIVE_WINDOW))
                iDisplayServer.send_wm_message(iWindow, display_server::atom_id::NET_ACTIVE_WINDOW, 1, static_cast<long>(iDisplayServer.input_time()), 0);
            else if (iMapped)
            {
                ::XRaiseWindow(iDisplayServer.display(), iWindow);
                ::XSetInputFocus(iDisplayServer.display(), iWindow, RevertToParent, XCurrentTime);
            }
            iDisplayServer.flush();
        }

        void window::iconize()
        {
            ::XIconifyWindow(iDisplayServer.display(), iWindow, iDisplayServer.screen());
            iDisplayServer.flush();
        }

        void window::maximize()
        {
            set_net_wm_state(true, iDisplayServer.atom(display_server::atom_id::NET_WM_STATE_MAXIMIZED_VERT), iDisplayServer.atom(display_server::atom_id::NET_WM_STATE_MAXIMIZED_HORZ));
        }

        void window::restore()
        {
            if (iMaximized)
                set_net_wm_state(false, iDisplayServer.atom(display_server::atom_id::NET_WM_STATE_MAXIMIZED_VERT), iDisplayServer.atom(display_server::atom_id::NET_WM_STATE_MAXIMIZED_HORZ));
            if (is_fullscreen())
            {
                iStyle &= ~window_style::Fullscreen;
                set_net_wm_state(false, iDisplayServer.atom(display_server::atom_id::NET_WM_STATE_FULLSCREEN));
            }
            if (iIconic)
            {
                ::XMapWindow(iDisplayServer.display(), iWindow);
                iDisplayServer.flush();
            }
        }

        void window::enter_fullscreen(const video_mode& aVideoMode)
        {
            service<i_surface_manager>().display(surface_window()).enter_fullscreen(aVideoMode);
            iStyle |= window_style::Fullscreen;
            set_net_wm_state(true, iDisplayServer.atom(display_server::atom_id::NET_WM_STATE_FULLSCREEN));
        }

        void window::set_title_text(i_string const& aTitleText)
        {
            ::XStoreName(iDisplayServer.display(), iWindow, aTitleText.c_str());
            ::XChangeProperty(iDisplayServer.display(), iWindow, iDisplayServer.atom(display_server::atom_id::NET_WM_NAME), iDisplayServer.atom(display_server::atom_id::UTF8_STRING),
                8, PropModeReplace, reinterpret_cast<unsigned char const*>(aTitleText.c_str()), static_cast<int>(aTitleText.size()));
            native_window::set_title_text(aTitleText);
            handle_event(window_event{ window_event_type::TitleTextChanged });
        }

        void window::alert(window_alert aAlert, std::optional<std::chrono::milliseconds> const& aInterval, std::optional<std::uint32_t> const& aCount)
        {
            native_window::alert(aAlert, aInterval, aCount);
            if (!iOverrideRedirect)
                set_net_wm_state(aAlert != window_alert::None, iDisplayServer.atom(display_server::atom_id::NET_WM_STATE_DEMANDS_ATTENTION));
        }

        bool window::can_render() const
        {
            return visible() && iMapped && attached() && attachment().can_render();
        }

        void window::display()
        {
            // n.b. a Vulkan surface presents itself (see vulkan_surface::do_render)
            auto& renderer = static_cast<linux::renderer&>(rendering_engine());
            if (!renderer.vulkan())
                renderer.swap_buffers(target_handle());
        }

        XID window::xwindow() const
        {
            return iWindow;
        }

        XIC window::input_context() const
        {
            return iInputContext;
        }

        bool window::override_redirect() const
        {
            return iOverrideRedirect;
        }

        bool window::dragging() const
        {
            return iDrag != std::nullopt;
        }

        void window::drag(point const& aRootPosition)
        {
            if (!iDrag)
                return;
            neolib::scoped_flag sf{ iResizingOrMoving };
            auto const offset = aRootPosition - iDrag->start;
            auto r = iDrag->startRect;
            scoped_units su{ surface_window().as_widget(), units::Pixels };
            auto const minimumSize = surface_window().as_widget().minimum_size().max(size{ 1.0, 1.0 });
            auto const left = [&]() { auto const right = r.right(); r.x = std::min(r.x + offset.x, right - minimumSize.cx); r.cx = right - r.x; };
            auto const top = [&]() { auto const bottom = r.bottom(); r.y = std::min(r.y + offset.y, bottom - minimumSize.cy); r.cy = bottom - r.y; };
            auto const right = [&]() { r.cx = std::max(r.cx + offset.x, minimumSize.cx); };
            auto const bottom = [&]() { r.cy = std::max(r.cy + offset.y, minimumSize.cy); };
            switch (iDrag->part)
            {
            case widget_part::BorderLeft: left(); break;
            case widget_part::BorderTopLeft: left(); top(); break;
            case widget_part::BorderTop: top(); break;
            case widget_part::BorderTopRight: top(); right(); break;
            case widget_part::BorderRight: right(); break;
            case widget_part::BorderBottomRight: case widget_part::GrowBox: right(); bottom(); break;
            case widget_part::BorderBottom: bottom(); break;
            case widget_part::BorderBottomLeft: left(); bottom(); break;
            default: r.x += offset.x; r.y += offset.y; break;
            }
            auto const newRect = r.as<int>();
            ::XMoveResizeWindow(iDisplayServer.display(), iWindow, newRect.x, newRect.y, static_cast<unsigned int>(std::max(1, newRect.cx)), static_cast<unsigned int>(std::max(1, newRect.cy)));
            set_position(r.position(), true);
            set_extents(r.extents(), true);
        }

        void window::end_drag()
        {
            iDrag = std::nullopt;
        }

        void window::configure_notify(XEvent const& aEvent)
        {
            auto const& e = aEvent.xconfigure;
            point position{ basic_point<int>{ e.x, e.y } };
            // (a synthetic event (the window manager's) has the position on the screen; a real one has the position
            // within the parent, which is the window manager's frame once it has reparented the window)
            if (!e.send_event && !iOverrideRedirect)
            {
                int x = 0;
                int y = 0;
                ::Window child;
                ::XTranslateCoordinates(iDisplayServer.display(), iWindow, iDisplayServer.root(), 0, 0, &x, &y, &child);
                position = basic_point<int>{ x, y };
            }
            set_position(position, true);
            set_extents(size{ basic_size<int>{ e.width, e.height } }, true);
        }

        void window::map_notify()
        {
            iMapped = true;
            set_iconic(false);
            request_paint();
        }

        void window::unmap_notify()
        {
            iMapped = false;
            if (iVisible && !iOverrideRedirect)
                set_iconic(true);
        }

        void window::property_notify(Atom aProperty)
        {
            if (aProperty != iDisplayServer.atom(display_server::atom_id::NET_WM_STATE))
                return;
            Atom type = XNone;
            int format = 0;
            unsigned long count = 0ul;
            unsigned long remaining = 0ul;
            unsigned char* data = nullptr;
            bool maximizedVertically = false;
            bool maximizedHorizontally = false;
            bool hidden = false;
            if (::XGetWindowProperty(iDisplayServer.display(), iWindow, aProperty, 0, 1024, XFalse, XA_ATOM, &type, &format, &count, &remaining, &data) == XSuccess && data != nullptr)
            {
                auto const states = reinterpret_cast<Atom const*>(data);
                for (unsigned long i = 0ul; i < count; ++i)
                {
                    if (states[i] == iDisplayServer.atom(display_server::atom_id::NET_WM_STATE_MAXIMIZED_VERT))
                        maximizedVertically = true;
                    else if (states[i] == iDisplayServer.atom(display_server::atom_id::NET_WM_STATE_MAXIMIZED_HORZ))
                        maximizedHorizontally = true;
                    else if (states[i] == iDisplayServer.atom(display_server::atom_id::NET_WM_STATE_HIDDEN))
                        hidden = true;
                }
                ::XFree(data);
            }
            set_maximized(maximizedVertically && maximizedHorizontally);
            set_iconic(hidden);
        }

        void window::expose(rect const& aArea)
        {
            if (initialising())
                return;
            if (attached())
                attachment().invalidate(aArea);
            push_event(window_event{ window_event_type::Paint });
        }

        void window::delete_window()
        {
            handle_event(window_event{ window_event_type::Close });
        }

        void window::begin_move(point const& aPosition, std::uint32_t)
        {
            if (iDisplayServer.supported(display_server::atom_id::NET_WM_MOVERESIZE))
                begin_move_resize(MOVE_RESIZE_MOVE, aPosition);
            else
                iDrag = manual_drag{ widget_part::TitleBar, iPosition + aPosition, rect{ iPosition, iExtents } };
        }

        void window::begin_resize(widget_part_e aBorder, point const& aPosition, std::uint32_t)
        {
            auto const direction = move_resize_direction(aBorder);
            if (!direction)
                return;
            if (iDisplayServer.supported(display_server::atom_id::NET_WM_MOVERESIZE))
                begin_move_resize(*direction, aPosition);
            else
                iDrag = manual_drag{ aBorder, iPosition + aPosition, rect{ iPosition, iExtents } };
        }

        void window::show_window_menu(point const&, std::uint32_t)
        {
            // n.b. no window menu that window managers have in common
        }

        void window::init(string const& aWindowTitle)
        {
            auto const display = iDisplayServer.display();
            auto const screen = iDisplayServer.screen();
            auto& renderer = static_cast<linux::renderer&>(rendering_engine());
            // (an EGL window surface must be of a window with the visual of its configuration)
            Visual* visual = DefaultVisual(display, screen);
            int depth = DefaultDepth(display, screen);
            if (auto const visualId = renderer.native_visual_id(); visualId != 0u)
            {
                XVisualInfo visualTemplate = {};
                visualTemplate.visualid = visualId;
                int count = 0;
                if (auto const visualInfo = ::XGetVisualInfo(display, VisualIDMask, &visualTemplate, &count); visualInfo != nullptr)
                {
                    visual = visualInfo->visual;
                    depth = visualInfo->depth;
                    ::XFree(visualInfo);
                }
            }
            iOverrideRedirect = (iStyle & window_style::Popup) == window_style::Popup ||
                ((iStyle & window_style::NoActivate) == window_style::NoActivate && (iStyle & window_style::NoDecoration) == window_style::NoDecoration);
            XSetWindowAttributes attributes = {};
            attributes.colormap = ::XCreateColormap(display, iDisplayServer.root(), visual, AllocNone);
            attributes.border_pixel = 0;
            attributes.background_pixmap = XNone;
            attributes.event_mask = EVENT_MASK;
            attributes.override_redirect = iOverrideRedirect ? XTrue : XFalse;
            auto const position = iPosition.as<int>();
            auto const extents = iExtents.max(size{ 1.0, 1.0 }).as<int>();
            iWindow = ::XCreateWindow(display, iDisplayServer.root(), position.x, position.y, static_cast<unsigned int>(extents.cx), static_cast<unsigned int>(extents.cy),
                0, depth, InputOutput, visual, CWColormap | CWBorderPixel | CWBackPixmap | CWEventMask | CWOverrideRedirect, &attributes);
            if (iWindow == 0ul)
                throw failed_to_create_window("XCreateWindow");
            Atom protocols[] = { iDisplayServer.atom(display_server::atom_id::WM_DELETE_WINDOW), iDisplayServer.atom(display_server::atom_id::NET_WM_PING) };
            ::XSetWMProtocols(display, iWindow, protocols, static_cast<int>(std::size(protocols)));
            long const pid = static_cast<long>(::getpid());
            ::XChangeProperty(display, iWindow, iDisplayServer.atom(display_server::atom_id::NET_WM_PID), XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<unsigned char const*>(&pid), 1);
            std::string const appName = service<i_app>().name().empty() ? std::string{ "neogfx" } : service<i_app>().name().to_std_string();
            XClassHint classHint = {};
            classHint.res_name = const_cast<char*>(appName.c_str());
            classHint.res_class = const_cast<char*>(appName.c_str());
            ::XSetClassHint(display, iWindow, &classHint);
            XWMHints wmHints = {};
            wmHints.flags = InputHint | StateHint;
            wmHints.input = (iStyle & window_style::NoActivate) == window_style::NoActivate ? XFalse : XTrue;
            wmHints.initial_state = NormalState;
            ::XSetWMHints(display, iWindow, &wmHints);
            // what the window is (for the window manager, and the compositor)
            auto windowType = display_server::atom_id::NET_WM_WINDOW_TYPE_NORMAL;
            if ((iStyle & window_style::Menu) == window_style::Menu && (iStyle & window_style::Popup) == window_style::Popup)
                windowType = display_server::atom_id::NET_WM_WINDOW_TYPE_POPUP_MENU;
            else if ((iStyle & window_style::Popup) == window_style::Popup && (iStyle & window_style::Tool) == window_style::Tool)
                windowType = display_server::atom_id::NET_WM_WINDOW_TYPE_TOOLTIP;
            else if (iOverrideRedirect)
                windowType = display_server::atom_id::NET_WM_WINDOW_TYPE_DROPDOWN_MENU;
            else if ((iStyle & window_style::Dialog) == window_style::Dialog)
                windowType = display_server::atom_id::NET_WM_WINDOW_TYPE_DIALOG;
            else if ((iStyle & window_style::Tool) == window_style::Tool)
                windowType = display_server::atom_id::NET_WM_WINDOW_TYPE_UTILITY;
            Atom const windowTypeAtom = iDisplayServer.atom(windowType);
            ::XChangeProperty(display, iWindow, iDisplayServer.atom(display_server::atom_id::NET_WM_WINDOW_TYPE), XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char const*>(&windowTypeAtom), 1);
            if (iParent != nullptr && !iOverrideRedirect)
                ::XSetTransientForHint(display, iWindow, static_cast<window*>(iParent)->xwindow());
            if (custom_decoration() || (iStyle & window_style::NoDecoration) == window_style::NoDecoration)
            {
                motif_wm_hints const hints{ MWM_HINTS_DECORATIONS, 0ul, 0ul, 0l, 0ul };
                auto const motifHints = iDisplayServer.atom(display_server::atom_id::MOTIF_WM_HINTS);
                ::XChangeProperty(display, iWindow, motifHints, motifHints, 32, PropModeReplace, reinterpret_cast<unsigned char const*>(&hints), 5);
            }
            if ((iStyle & window_style::Tool) == window_style::Tool && !iOverrideRedirect)
            {
                Atom const skipTaskbar = iDisplayServer.atom(display_server::atom_id::NET_WM_STATE_SKIP_TASKBAR);
                ::XChangeProperty(display, iWindow, iDisplayServer.atom(display_server::atom_id::NET_WM_STATE), XA_ATOM, 32, PropModeAppend, reinterpret_cast<unsigned char const*>(&skipTaskbar), 1);
            }
            if (iDisplayServer.input_method() != nullptr)
            {
                iInputContext = ::XCreateIC(iDisplayServer.input_method(), XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                    XNClientWindow, iWindow, XNFocusWindow, iWindow, nullptr);
                if (iInputContext != nullptr)
                {
                    long filterEvents = 0;
                    ::XGetICValues(iInputContext, XNFilterEvents, &filterEvents, nullptr);
                    ::XSelectInput(display, iWindow, EVENT_MASK | filterEvents);
                }
            }
            ::XStoreName(display, iWindow, aWindowTitle.c_str());
            ::XChangeProperty(display, iWindow, iDisplayServer.atom(display_server::atom_id::NET_WM_NAME), iDisplayServer.atom(display_server::atom_id::UTF8_STRING),
                8, PropModeReplace, reinterpret_cast<unsigned char const*>(aWindowTitle.c_str()), static_cast<int>(aWindowTitle.size()));
            native_window::set_title_text(aWindowTitle);
            iDisplayServer.add_window(*this);
            if (!renderer.vulkan())
                renderer.create_window_surface(target_handle(), &iWindow);
            surface_window().set_native_window(*this);
        }

        void window::set_net_wm_state(bool aAdd, Atom aState1, Atom aState2)
        {
            if (iMapped)
                iDisplayServer.send_wm_message(iWindow, display_server::atom_id::NET_WM_STATE, aAdd ? 1 : 0, static_cast<long>(aState1), static_cast<long>(aState2), 1);
            else if (aAdd)
            {
                // (before the window is mapped the state is the window's property, which the window manager reads)
                Atom const states[] = { aState1, aState2 };
                ::XChangeProperty(iDisplayServer.display(), iWindow, iDisplayServer.atom(display_server::atom_id::NET_WM_STATE), XA_ATOM, 32, PropModeAppend,
                    reinterpret_cast<unsigned char const*>(states), aState2 != 0ul ? 2 : 1);
            }
            iDisplayServer.flush();
        }

        void window::update_size_hints()
        {
            auto const hints = ::XAllocSizeHints();
            if (hints == nullptr)
                return;
            // (where neoGFX puts the window, rather than where the window manager would)
            hints->flags = PPosition | USPosition;
            auto const position = iPosition.as<int>();
            hints->x = position.x;
            hints->y = position.y;
            if ((iStyle & window_style::Resize) == window_style::Resize)
            {
                scoped_units su{ surface_window().as_widget(), units::Pixels };
                auto const minimumSize = surface_window().as_widget().minimum_size().max(size{ 1.0, 1.0 }).as<int>();
                hints->flags |= PMinSize;
                hints->min_width = minimumSize.cx;
                hints->min_height = minimumSize.cy;
            }
            else
            {
                auto const extents = iExtents.max(size{ 1.0, 1.0 }).as<int>();
                hints->flags |= PMinSize | PMaxSize;
                hints->min_width = hints->max_width = extents.cx;
                hints->min_height = hints->max_height = extents.cy;
            }
            ::XSetWMNormalHints(iDisplayServer.display(), iWindow, hints);
            ::XFree(hints);
        }

        void window::begin_move_resize(int aDirection, point const& aPosition)
        {
            // (the window manager takes the pointer, which the button's press has (implicitly) grabbed)
            auto const rootPosition = (iPosition + aPosition).as<int>();
            ::XUngrabPointer(iDisplayServer.display(), XCurrentTime);
            iDisplayServer.send_wm_message(iWindow, display_server::atom_id::NET_WM_MOVERESIZE, rootPosition.x, rootPosition.y, aDirection, Button1, 1);
            iDisplayServer.flush();
        }
    }
}
