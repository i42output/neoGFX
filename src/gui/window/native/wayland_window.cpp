// wayland_window.cpp
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

#include <wayland-client.h>
#include <wayland-egl.h>
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"

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
#include "../../../app/native/wayland_display_server.hpp"
#include "../../../gfx/native/linux_renderer.hpp"
#include "wayland_window.hpp"

namespace neogfx
{
    namespace native::wayland
    {
        namespace
        {
            wl_surface_listener const sSurfaceListener =
            {
                .enter = [](void* aData, wl_surface*, wl_output* aOutput) { static_cast<window*>(aData)->surface_enter(aOutput); },
                .leave = [](void* aData, wl_surface*, wl_output* aOutput) { static_cast<window*>(aData)->surface_leave(aOutput); },
#ifdef WL_SURFACE_PREFERRED_BUFFER_SCALE_SINCE_VERSION
                .preferred_buffer_scale = [](void* aData, wl_surface*, std::int32_t aScale) { static_cast<window*>(aData)->surface_preferred_buffer_scale(aScale); },
                .preferred_buffer_transform = [](void*, wl_surface*, std::uint32_t) {}
#endif
            };

            xdg_surface_listener const sXdgSurfaceListener =
            {
                .configure = [](void* aData, xdg_surface*, std::uint32_t aSerial) { static_cast<window*>(aData)->xdg_surface_configure(aSerial); }
            };

            xdg_toplevel_listener const sToplevelListener =
            {
                .configure = [](void* aData, xdg_toplevel*, std::int32_t aWidth, std::int32_t aHeight, wl_array* aStates)
                {
                    std::vector<std::uint32_t> states;
                    auto const begin = static_cast<std::uint32_t const*>(aStates->data);
                    states.assign(begin, begin + aStates->size / sizeof(std::uint32_t));
                    static_cast<window*>(aData)->toplevel_configure(aWidth, aHeight, states);
                },
                .close = [](void* aData, xdg_toplevel*) { static_cast<window*>(aData)->toplevel_close(); },
                .configure_bounds = [](void*, xdg_toplevel*, std::int32_t, std::int32_t) {},
                .wm_capabilities = [](void*, xdg_toplevel*, wl_array*) {}
            };

            xdg_popup_listener const sPopupListener =
            {
                .configure = [](void* aData, xdg_popup*, std::int32_t aX, std::int32_t aY, std::int32_t aWidth, std::int32_t aHeight)
                    { static_cast<window*>(aData)->popup_configure(aX, aY, aWidth, aHeight); },
                .popup_done = [](void* aData, xdg_popup*) { static_cast<window*>(aData)->popup_done(); },
                .repositioned = [](void*, xdg_popup*, std::uint32_t) {}
            };

            zxdg_toplevel_decoration_v1_listener const sDecorationListener =
            {
                .configure = [](void*, zxdg_toplevel_decoration_v1*, std::uint32_t) {}
            };

            std::optional<std::uint32_t> resize_edges(widget_part_e aPart)
            {
                switch (aPart)
                {
                case widget_part::BorderLeft:
                    return XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
                case widget_part::BorderTopLeft:
                    return XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
                case widget_part::BorderTop:
                    return XDG_TOPLEVEL_RESIZE_EDGE_TOP;
                case widget_part::BorderTopRight:
                    return XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT;
                case widget_part::BorderRight:
                    return XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
                case widget_part::BorderBottomRight:
                case widget_part::GrowBox:
                    return XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
                case widget_part::BorderBottom:
                    return XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
                case widget_part::BorderBottomLeft:
                    return XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
                default:
                    return {};
                }
            }

            display_server& the_display_server()
            {
                return static_cast<display_server&>(static_cast<linux::basic_services&>(service<i_basic_services>()).display_server());
            }
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
            // (the surface's (Vulkan or EGL) resources go before the surface they were made for)
            native_window::detach();
            auto& renderer = static_cast<linux::renderer&>(rendering_engine());
            if (iSurface != nullptr)
            {
                if (!renderer.vulkan())
                    renderer.destroy_window_surface(iSurface);
                if (iEglWindow != nullptr)
                    ::wl_egl_window_destroy(iEglWindow);
                ::wl_surface_destroy(iSurface);
                iDisplayServer.flush();
            }
        }

        void* window::target_handle() const
        {
            return iSurface;
        }

        void* window::target_device_handle() const
        {
            return iDisplayServer.display();
        }

        void* window::handle() const
        {
            return iSurface;
        }

        void* window::native_handle() const
        {
            return iSurface;
        }

        void window::move_surface(const point& aPosition)
        {
            // n.b. a toplevel cannot be moved by its client, so its position is only what neoGFX makes of it
            set_position(aPosition, false);
        }

        void window::resize_surface(const size& aExtents)
        {
            // n.b. rounded as extents_changed rounds it, so that asking for a size that rounds to the current one changes nothing
            // (else a popup sized by its content, e.g. 201 pixels at a scale of 2, is resized to 202, resizes itself back, and
            // so on forever, its popup made again each time)
            auto const scale = static_cast<scalar>(iScale);
            set_extents((aExtents / scale).ceil() * scale, false);
        }

        bool window::resizing_or_moving() const
        {
            return iResizing;
        }

        void window::close(bool aForce)
        {
            if (iSurface != nullptr && is_alive() && (aForce || surface_window().native_window_can_close()))
            {
                set_destroying();
                destroy_role();
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
            create_role();
            if (aActivate)
                activate();
        }

        void window::hide()
        {
            if (!iVisible)
                return;
            iVisible = false;
            destroy_role();
        }

        double window::opacity() const
        {
            return iOpacity;
        }

        void window::set_opacity(double aOpacity)
        {
            // n.b. Wayland has no window opacity of its own (a window is as opaque as what it draws)
            iOpacity = aOpacity;
        }

        bool window::is_effectively_active() const
        {
            if (is_popup())
                return wayland_parent()->is_effectively_active();
            return iDisplayServer.keyboard_focus() == this || iActivated;
        }

        void window::iconize()
        {
            if (iToplevel != nullptr)
            {
                ::xdg_toplevel_set_minimized(iToplevel);
                // (the compositor says nothing of a toplevel being minimized; it is restored when next activated)
                set_iconic(true);
            }
        }

        void window::maximize()
        {
            if (iToplevel != nullptr)
                ::xdg_toplevel_set_maximized(iToplevel);
        }

        void window::restore()
        {
            if (iToplevel != nullptr)
            {
                if (iMaximized)
                    ::xdg_toplevel_unset_maximized(iToplevel);
                if (iFullscreen)
                    ::xdg_toplevel_unset_fullscreen(iToplevel);
            }
            set_iconic(false);
        }

        void window::enter_fullscreen(const video_mode& aVideoMode)
        {
            service<i_surface_manager>().display(surface_window()).enter_fullscreen(aVideoMode);
            iStyle |= window_style::Fullscreen;
            if (iToplevel != nullptr)
                ::xdg_toplevel_set_fullscreen(iToplevel, nullptr);
        }

        void window::set_title_text(i_string const& aTitleText)
        {
            if (iToplevel != nullptr)
                ::xdg_toplevel_set_title(iToplevel, aTitleText.c_str());
            native_window::set_title_text(aTitleText);
            handle_event(window_event{ window_event_type::TitleTextChanged });
        }

        void window::alert(window_alert aAlert, std::optional<std::chrono::milliseconds> const& aInterval, std::optional<std::uint32_t> const& aCount)
        {
            // n.b. there is no Wayland protocol for this that compositors have in common
            native_window::alert(aAlert, aInterval, aCount);
        }

        bool window::can_render() const
        {
            return visible() && iConfigured && attached() && attachment().can_render();
        }

        void window::display()
        {
            // n.b. a Vulkan surface presents itself (see vulkan_surface::do_render)
            auto& renderer = static_cast<linux::renderer&>(rendering_engine());
            if (!renderer.vulkan())
                renderer.swap_buffers(iSurface);
        }

        wayland::display_server& window::wayland_display_server() const
        {
            return iDisplayServer;
        }

        window* window::wayland_parent() const
        {
            return static_cast<window*>(iParent);
        }

        wl_surface* window::surface() const
        {
            return iSurface;
        }

        std::uint32_t window::scale() const
        {
            return iScale;
        }

        void window::surface_enter(wl_output* aOutput)
        {
            iOutputs.push_back(aOutput);
            update_scale();
        }

        void window::surface_leave(wl_output* aOutput)
        {
            iOutputs.erase(std::remove(iOutputs.begin(), iOutputs.end(), aOutput), iOutputs.end());
            update_scale();
        }

        void window::surface_preferred_buffer_scale(std::int32_t aScale)
        {
            iPreferredScale = static_cast<std::uint32_t>(std::max(1, aScale));
            update_scale();
        }

        void window::xdg_surface_configure(std::uint32_t aSerial)
        {
            ::xdg_surface_ack_configure(iXdgSurface, aSerial);
            bool const firstConfigure = !iConfigured;
            iConfigured = true;
            if (iToplevel != nullptr)
            {
                auto const has_state = [&](std::uint32_t aState)
                {
                    return std::find(iConfigureStates.begin(), iConfigureStates.end(), aState) != iConfigureStates.end();
                };
                iResizing = has_state(XDG_TOPLEVEL_STATE_RESIZING);
                iActivated = has_state(XDG_TOPLEVEL_STATE_ACTIVATED);
                iFullscreen = has_state(XDG_TOPLEVEL_STATE_FULLSCREEN);
                if (iConfigureSize && iConfigureSize->cx > 0 && iConfigureSize->cy > 0)
                    set_extents(size{ basic_size<std::int32_t>{ iConfigureSize->cx * static_cast<std::int32_t>(iScale), iConfigureSize->cy * static_cast<std::int32_t>(iScale) } }, true);
                iConfigureSize = std::nullopt;
                set_maximized(has_state(XDG_TOPLEVEL_STATE_MAXIMIZED));
            }
            if (firstConfigure)
            {
                update_min_size();
                request_paint();
            }
        }

        void window::toplevel_configure(std::int32_t aWidth, std::int32_t aHeight, std::vector<std::uint32_t> const& aStates)
        {
            iConfigureSize = basic_size<std::int32_t>{ aWidth, aHeight };
            iConfigureStates = aStates;
        }

        void window::toplevel_close()
        {
            handle_event(window_event{ window_event_type::Close });
        }

        void window::popup_configure(std::int32_t aX, std::int32_t aY, std::int32_t, std::int32_t)
        {
            auto const parent = wayland_parent();
            if (parent == nullptr)
                return;
            // (where the compositor has put the popup, e.g. having slid it to keep it on the screen)
            neolib::scoped_flag sf{ iRepositioning };
            set_position(parent->surface_position() + point{ basic_point<std::int32_t>{ aX, aY } } * static_cast<scalar>(parent->scale()), true);
        }

        void window::popup_done()
        {
            // the compositor has dismissed the popup (as Windows would on a click outside it)
            if (iParent != nullptr)
                iParent->surface_window().native_window_dismiss_children();
            if (iPopup != nullptr && is_alive())
                hide();
        }

        void window::outputs_changed()
        {
            update_scale();
        }

        void window::begin_move(point const&, std::uint32_t aSerial)
        {
            if (iToplevel != nullptr)
                ::xdg_toplevel_move(iToplevel, iDisplayServer.seat(), aSerial);
        }

        void window::begin_resize(widget_part_e aBorder, point const&, std::uint32_t aSerial)
        {
            if (auto const edges = resize_edges(aBorder); edges && iToplevel != nullptr)
                ::xdg_toplevel_resize(iToplevel, iDisplayServer.seat(), aSerial, *edges);
        }

        void window::show_window_menu(point const& aPosition, std::uint32_t)
        {
            if (iToplevel != nullptr)
                ::xdg_toplevel_show_window_menu(iToplevel, iDisplayServer.seat(), iDisplayServer.pointer_button_serial(),
                    static_cast<std::int32_t>(aPosition.x / iScale), static_cast<std::int32_t>(aPosition.y / iScale));
        }

        void window::extents_changed()
        {
            // n.b. a buffer's extents must be a multiple of the surface's buffer scale (else the compositor ends the connection
            // with a protocol error), so the window's are too (a whole number of logical pixels)
            auto const scale = static_cast<scalar>(iScale);
            if (auto const wholeLogical = (iExtents / scale).ceil() * scale; wholeLogical != iExtents)
            {
                set_extents(wholeLogical, false);
                return;
            }
            if (iEglWindow != nullptr)
            {
                auto const extents = ((iExtents.max(size{ 1.0, 1.0 }) / scale).ceil() * scale).as<int>();
                ::wl_egl_window_resize(iEglWindow, extents.cx, extents.cy, 0, 0);
            }
            if (iPopup != nullptr && !iRepositioning)
                reposition_popup();
        }

        void window::position_changed()
        {
            if (iPopup != nullptr && !iRepositioning)
                reposition_popup();
        }

        void window::init(string const& aWindowTitle)
        {
            iSurface = ::wl_compositor_create_surface(iDisplayServer.compositor());
            if (iSurface == nullptr)
                throw failed_to_create_window("wl_compositor_create_surface");
            ::wl_surface_add_listener(iSurface, &sSurfaceListener, this);
            iDisplayServer.add_window(*this);
            if (wayland_parent() != nullptr)
                iScale = wayland_parent()->scale();
            else
                // (the outputs' scale, as the window's extents (see linux::display), until the compositor says which scale it
                // prefers (wl_surface.preferred_buffer_scale), which is only after the first commit: a first buffer at scale 1
                // would be twice the size (on a scale 2 output), and e.g. GNOME maximizes a new window that large)
                iScale = iDisplayServer.max_output_scale();
            if (iScale != 1u)
                ::wl_surface_set_buffer_scale(iSurface, static_cast<std::int32_t>(iScale));
            auto& renderer = static_cast<linux::renderer&>(rendering_engine());
            if (!renderer.vulkan())
            {
                // (a multiple of the buffer scale: see extents_changed)
                auto const scale = static_cast<scalar>(iScale);
                auto const extents = ((iExtents.max(size{ 1.0, 1.0 }) / scale).ceil() * scale).as<int>();
                iEglWindow = ::wl_egl_window_create(iSurface, extents.cx, extents.cy);
                if (iEglWindow == nullptr)
                    throw failed_to_create_window("wl_egl_window_create");
                renderer.create_window_surface(iSurface, iEglWindow);
            }
            native_window::set_title_text(aWindowTitle);
            surface_window().set_native_window(*this);
        }

        void window::create_role()
        {
            if (iXdgSurface != nullptr)
                return;
            iConfigured = false;
            iXdgSurface = ::xdg_wm_base_get_xdg_surface(iDisplayServer.wm_base(), iSurface);
            ::xdg_surface_add_listener(iXdgSurface, &sXdgSurfaceListener, this);
            auto const parent = wayland_parent();
            if (is_popup() && parent->iXdgSurface != nullptr)
            {
                auto const positioner = ::xdg_wm_base_create_positioner(iDisplayServer.wm_base());
                auto const parentScale = static_cast<scalar>(parent->scale());
                auto const logicalExtents = (iExtents / static_cast<scalar>(iScale)).ceil().max(size{ 1.0, 1.0 }).as<std::int32_t>();
                auto const parentExtents = (parent->surface_extents() / parentScale).floor().max(size{ 1.0, 1.0 }).as<std::int32_t>();
                auto const relative = ((iPosition - parent->surface_position()) / parentScale).round().as<std::int32_t>();
                // (the anchor (a point) is within the parent; where the popup is relative to that is its offset)
                basic_point<std::int32_t> const anchor{ std::clamp(relative.x, 0, parentExtents.cx - 1), std::clamp(relative.y, 0, parentExtents.cy - 1) };
                ::xdg_positioner_set_size(positioner, logicalExtents.cx, logicalExtents.cy);
                ::xdg_positioner_set_anchor_rect(positioner, anchor.x, anchor.y, 1, 1);
                ::xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_TOP_LEFT);
                ::xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);
                ::xdg_positioner_set_offset(positioner, relative.x - anchor.x, relative.y - anchor.y);
                ::xdg_positioner_set_constraint_adjustment(positioner,
                    XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X | XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y);
                iPopup = ::xdg_surface_get_popup(iXdgSurface, parent->iXdgSurface, positioner);
                ::xdg_positioner_destroy(positioner);
                ::xdg_popup_add_listener(iPopup, &sPopupListener, this);
            }
            else
            {
                iToplevel = ::xdg_surface_get_toplevel(iXdgSurface);
                ::xdg_toplevel_add_listener(iToplevel, &sToplevelListener, this);
                ::xdg_toplevel_set_title(iToplevel, title_text().c_str());
                auto const& appName = service<i_app>().name();
                if (!appName.empty())
                    ::xdg_toplevel_set_app_id(iToplevel, appName.c_str());
                if (parent != nullptr && parent->iToplevel != nullptr)
                    ::xdg_toplevel_set_parent(iToplevel, parent->iToplevel);
                if (iDisplayServer.decoration_manager() != nullptr)
                {
                    iDecoration = ::zxdg_decoration_manager_v1_get_toplevel_decoration(iDisplayServer.decoration_manager(), iToplevel);
                    ::zxdg_toplevel_decoration_v1_add_listener(iDecoration, &sDecorationListener, this);
                    bool const serverSide = !custom_decoration() && (iStyle & (window_style::NoDecoration | window_style::Popup)) == window_style::Invalid;
                    ::zxdg_toplevel_decoration_v1_set_mode(iDecoration, serverSide ?
                        ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE : ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);
                }
                if ((iStyle & window_style::Resize) != window_style::Resize)
                {
                    auto const logicalExtents = (iExtents / static_cast<scalar>(iScale)).ceil().as<std::int32_t>();
                    ::xdg_toplevel_set_min_size(iToplevel, logicalExtents.cx, logicalExtents.cy);
                    ::xdg_toplevel_set_max_size(iToplevel, logicalExtents.cx, logicalExtents.cy);
                }
                if (is_fullscreen())
                    ::xdg_toplevel_set_fullscreen(iToplevel, nullptr);
            }
            ::wl_surface_commit(iSurface);
            iDisplayServer.flush();
        }

        void window::destroy_role()
        {
            if (iXdgSurface == nullptr)
                return;
            // (unmapped: a surface may not have a buffer when it is next given a role)
            ::wl_surface_attach(iSurface, nullptr, 0, 0);
            ::wl_surface_commit(iSurface);
            if (iDecoration != nullptr)
            {
                ::zxdg_toplevel_decoration_v1_destroy(iDecoration);
                iDecoration = nullptr;
            }
            if (iPopup != nullptr)
            {
                ::xdg_popup_destroy(iPopup);
                iPopup = nullptr;
            }
            if (iToplevel != nullptr)
            {
                ::xdg_toplevel_destroy(iToplevel);
                iToplevel = nullptr;
            }
            ::xdg_surface_destroy(iXdgSurface);
            iXdgSurface = nullptr;
            iConfigured = false;
            iDisplayServer.flush();
        }

        void window::reposition_popup()
        {
            if (iPopup == nullptr)
                return;
            // (made again where it now is: xdg_popup.reposition needs xdg_wm_base version 3, which not every
            // compositor has)
            destroy_role();
            create_role();
        }

        void window::set_scale(std::uint32_t aScale)
        {
            if (iScale == aScale)
                return;
            auto const previousScale = iScale;
            iScale = aScale;
            ::wl_surface_set_buffer_scale(iSurface, static_cast<std::int32_t>(iScale));
            // (the same logical size, so more (or fewer) pixels)
            set_extents((iExtents * static_cast<scalar>(iScale) / static_cast<scalar>(previousScale)).ceil(), true);
            handle_dpi_changed();
        }

        void window::update_scale()
        {
            if (is_popup())
            {
                set_scale(wayland_parent()->scale());
                return;
            }
            if (iPreferredScale)
            {
                set_scale(*iPreferredScale);
                return;
            }
            std::uint32_t scale = 0u;
            for (auto output : iOutputs)
                scale = std::max(scale, iDisplayServer.output_scale(output).value_or(1u));
            if (scale == 0u)
                scale = iDisplayServer.max_output_scale();
            set_scale(scale);
        }

        void window::update_min_size()
        {
            if (iToplevel == nullptr || (iStyle & window_style::Resize) != window_style::Resize)
                return;
            scoped_units su{ surface_window().as_widget(), units::Pixels };
            auto const minimumSize = (surface_window().as_widget().minimum_size() / static_cast<scalar>(iScale)).ceil().as<std::int32_t>();
            ::xdg_toplevel_set_min_size(iToplevel, std::max(1, minimumSize.cx), std::max(1, minimumSize.cy));
        }
    }
}
