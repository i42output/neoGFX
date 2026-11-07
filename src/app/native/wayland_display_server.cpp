// wayland_display_server.cpp
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

#include <cstring>
#include <clocale>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/input-event-codes.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"

#include <neogfx/app/i_app.hpp>
#include <neogfx/app/i_clipboard.hpp>
#include <neogfx/hid/i_window_manager.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include "../../gui/window/native/wayland_window.hpp"
#include "wayland_display_server.hpp"

namespace neogfx
{
    namespace native::wayland
    {
        namespace
        {
            wl_registry_listener const sRegistryListener =
            {
                .global = [](void* aData, wl_registry* aRegistry, std::uint32_t aName, char const* aInterface, std::uint32_t aVersion)
                    { static_cast<display_server*>(aData)->registry_global(aRegistry, aName, aInterface, aVersion); },
                .global_remove = [](void* aData, wl_registry* aRegistry, std::uint32_t aName)
                    { static_cast<display_server*>(aData)->registry_global_remove(aRegistry, aName); }
            };

            xdg_wm_base_listener const sWmBaseListener =
            {
                .ping = [](void*, xdg_wm_base* aWmBase, std::uint32_t aSerial) { ::xdg_wm_base_pong(aWmBase, aSerial); }
            };

            wl_seat_listener const sSeatListener =
            {
                .capabilities = [](void* aData, wl_seat*, std::uint32_t aCapabilities)
                    { static_cast<display_server*>(aData)->seat_capabilities(aCapabilities); },
                .name = [](void*, wl_seat*, char const*) {}
            };

            wl_output_listener const sOutputListener =
            {
                .geometry = [](void* aData, wl_output* aOutput, std::int32_t aX, std::int32_t aY, std::int32_t aPhysicalWidth, std::int32_t aPhysicalHeight, std::int32_t aSubpixel, char const*, char const*, std::int32_t)
                    { static_cast<display_server*>(aData)->output_geometry(aOutput, aX, aY, aPhysicalWidth, aPhysicalHeight, aSubpixel); },
                .mode = [](void* aData, wl_output* aOutput, std::uint32_t aFlags, std::int32_t aWidth, std::int32_t aHeight, std::int32_t)
                    { static_cast<display_server*>(aData)->output_mode(aOutput, aFlags, aWidth, aHeight); },
                .done = [](void* aData, wl_output* aOutput)
                    { static_cast<display_server*>(aData)->output_done(aOutput); },
                .scale = [](void* aData, wl_output* aOutput, std::int32_t aScale)
                    { static_cast<display_server*>(aData)->output_scale_changed(aOutput, aScale); },
#ifdef WL_OUTPUT_NAME_SINCE_VERSION
                .name = [](void*, wl_output*, char const*) {},
                .description = [](void*, wl_output*, char const*) {}
#endif
            };

            wl_pointer_listener const sPointerListener =
            {
                .enter = [](void* aData, wl_pointer*, std::uint32_t aSerial, wl_surface* aSurface, wl_fixed_t aX, wl_fixed_t aY)
                    { static_cast<display_server*>(aData)->pointer_enter(aSerial, aSurface, ::wl_fixed_to_double(aX), ::wl_fixed_to_double(aY)); },
                .leave = [](void* aData, wl_pointer*, std::uint32_t aSerial, wl_surface* aSurface)
                    { static_cast<display_server*>(aData)->pointer_leave(aSerial, aSurface); },
                .motion = [](void* aData, wl_pointer*, std::uint32_t, wl_fixed_t aX, wl_fixed_t aY)
                    { static_cast<display_server*>(aData)->pointer_motion(::wl_fixed_to_double(aX), ::wl_fixed_to_double(aY)); },
                .button = [](void* aData, wl_pointer*, std::uint32_t aSerial, std::uint32_t, std::uint32_t aButton, std::uint32_t aState)
                    { static_cast<display_server*>(aData)->pointer_button(aSerial, aButton, aState); },
                .axis = [](void* aData, wl_pointer*, std::uint32_t, std::uint32_t aAxis, wl_fixed_t aValue)
                    { static_cast<display_server*>(aData)->pointer_axis(aAxis, ::wl_fixed_to_double(aValue)); },
                .frame = [](void* aData, wl_pointer*)
                    { static_cast<display_server*>(aData)->pointer_frame(); },
                .axis_source = [](void*, wl_pointer*, std::uint32_t) {},
                .axis_stop = [](void*, wl_pointer*, std::uint32_t, std::uint32_t) {},
                .axis_discrete = [](void* aData, wl_pointer*, std::uint32_t aAxis, std::int32_t aDiscrete)
                    { static_cast<display_server*>(aData)->pointer_axis_discrete(aAxis, aDiscrete); },
#ifdef WL_POINTER_AXIS_VALUE120_SINCE_VERSION
                .axis_value120 = [](void* aData, wl_pointer*, std::uint32_t aAxis, std::int32_t aValue120)
                    { static_cast<display_server*>(aData)->pointer_axis_value120(aAxis, aValue120); },
#endif
#ifdef WL_POINTER_AXIS_RELATIVE_DIRECTION_SINCE_VERSION
                .axis_relative_direction = [](void*, wl_pointer*, std::uint32_t, std::uint32_t) {}
#endif
            };

            wl_keyboard_listener const sKeyboardListener =
            {
                .keymap = [](void* aData, wl_keyboard*, std::uint32_t aFormat, std::int32_t aFd, std::uint32_t aSize)
                    { static_cast<display_server*>(aData)->keyboard_keymap(aFormat, aFd, aSize); },
                .enter = [](void* aData, wl_keyboard*, std::uint32_t aSerial, wl_surface* aSurface, wl_array*)
                    { static_cast<display_server*>(aData)->keyboard_enter(aSerial, aSurface); },
                .leave = [](void* aData, wl_keyboard*, std::uint32_t aSerial, wl_surface* aSurface)
                    { static_cast<display_server*>(aData)->keyboard_leave(aSerial, aSurface); },
                .key = [](void* aData, wl_keyboard*, std::uint32_t aSerial, std::uint32_t, std::uint32_t aKey, std::uint32_t aState)
                    { static_cast<display_server*>(aData)->keyboard_key(aSerial, aKey, aState); },
                .modifiers = [](void* aData, wl_keyboard*, std::uint32_t aSerial, std::uint32_t aDepressed, std::uint32_t aLatched, std::uint32_t aLocked, std::uint32_t aGroup)
                    { static_cast<display_server*>(aData)->keyboard_modifiers(aSerial, aDepressed, aLatched, aLocked, aGroup); },
                .repeat_info = [](void* aData, wl_keyboard*, std::int32_t aRate, std::int32_t aDelay)
                    { static_cast<display_server*>(aData)->keyboard_repeat_info(aRate, aDelay); }
            };

            wl_data_offer_listener const sDataOfferListener =
            {
                .offer = [](void* aData, wl_data_offer* aOffer, char const* aMimeType)
                    { static_cast<display_server*>(aData)->data_offer_offer(aOffer, aMimeType); },
                .source_actions = [](void*, wl_data_offer*, std::uint32_t) {},
                .action = [](void*, wl_data_offer*, std::uint32_t) {}
            };

            wl_data_device_listener const sDataDeviceListener =
            {
                .data_offer = [](void* aData, wl_data_device*, wl_data_offer* aOffer)
                    { static_cast<display_server*>(aData)->data_device_offer(aOffer); },
                .enter = [](void*, wl_data_device*, std::uint32_t, wl_surface*, wl_fixed_t, wl_fixed_t, wl_data_offer*) {},
                .leave = [](void*, wl_data_device*) {},
                .motion = [](void*, wl_data_device*, std::uint32_t, wl_fixed_t, wl_fixed_t) {},
                .drop = [](void*, wl_data_device*) {},
                .selection = [](void* aData, wl_data_device*, wl_data_offer* aOffer)
                    { static_cast<display_server*>(aData)->data_device_selection(aOffer); }
            };

            wl_data_source_listener const sDataSourceListener =
            {
                .target = [](void*, wl_data_source*, char const*) {},
                .send = [](void* aData, wl_data_source* aSource, char const* aMimeType, std::int32_t aFd)
                    { static_cast<display_server*>(aData)->data_source_send(aSource, aMimeType, aFd); },
                .cancelled = [](void* aData, wl_data_source* aSource)
                    { static_cast<display_server*>(aData)->data_source_cancelled(aSource); },
                .dnd_drop_performed = [](void*, wl_data_source*) {},
                .dnd_finished = [](void*, wl_data_source*) {},
                .action = [](void*, wl_data_source*, std::uint32_t) {}
            };

            char const* const sTextMimeTypes[] = { "text/plain;charset=utf-8", "UTF8_STRING", "text/plain", "TEXT", "STRING" };

            mouse_button convert_button(std::uint32_t aButton)
            {
                switch (aButton)
                {
                case BTN_LEFT:
                    return mouse_button::Left;
                case BTN_RIGHT:
                    return mouse_button::Right;
                case BTN_MIDDLE:
                    return mouse_button::Middle;
                case BTN_SIDE:
                case BTN_BACK:
                    return mouse_button::X1;
                case BTN_EXTRA:
                case BTN_FORWARD:
                    return mouse_button::X2;
                default:
                    return mouse_button::None;
                }
            }
        }

        display_server::native_clipboard::native_clipboard(display_server& aDisplayServer) :
            iDisplayServer{ aDisplayServer }
        {
        }

        display_server::native_clipboard::~native_clipboard()
        {
            for (auto& offer : iOffers)
                ::wl_data_offer_destroy(offer.first);
            if (iSource != nullptr)
                ::wl_data_source_destroy(iSource);
        }

        bool display_server::native_clipboard::has_text() const
        {
            if (iSource != nullptr)
                return !iSourceText.empty();
            return text_mime_type() != std::nullopt;
        }

        i_string const& display_server::native_clipboard::text() const
        {
            iText.clear();
            if (iSource != nullptr)
            {
                iText = iSourceText;
                return iText;
            }
            auto const mimeType = text_mime_type();
            if (!mimeType)
                return iText;
            int fds[2];
            if (::pipe2(fds, O_CLOEXEC) != 0)
                throw failed_to_get_clipboard_data();
            ::wl_data_offer_receive(iSelection, mimeType->c_str(), fds[1]);
            ::close(fds[1]);
            iDisplayServer.flush();
            // n.b. the selection's owner (another client) writes it; a client that does not is given up on
            std::string result;
            char buffer[4096];
            for (;;)
            {
                pollfd pfd{ fds[0], POLLIN, 0 };
                if (::poll(&pfd, 1, 1000) <= 0)
                    break;
                auto const count = ::read(fds[0], buffer, sizeof(buffer));
                if (count <= 0)
                    break;
                result.append(buffer, static_cast<std::size_t>(count));
            }
            ::close(fds[0]);
            iText = result;
            return iText;
        }

        void display_server::native_clipboard::set_text(i_string const& aText)
        {
            if (iDisplayServer.iDataDeviceManager == nullptr || iDisplayServer.iDataDevice == nullptr)
                throw failed_to_set_clipboard_data();
            if (iSource != nullptr)
                ::wl_data_source_destroy(iSource);
            iSource = ::wl_data_device_manager_create_data_source(iDisplayServer.iDataDeviceManager);
            ::wl_data_source_add_listener(iSource, &sDataSourceListener, &iDisplayServer);
            for (auto mimeType : sTextMimeTypes)
                ::wl_data_source_offer(iSource, mimeType);
            iSourceText = aText.to_std_string();
            ::wl_data_device_set_selection(iDisplayServer.iDataDevice, iSource, iDisplayServer.input_serial());
            iDisplayServer.flush();
        }

        bool display_server::native_clipboard::has_image() const
        {
            return false; // todo
        }

        neogfx::image display_server::native_clipboard::image() const
        {
            throw unsupported_clipboard_operation(); // todo
        }

        void display_server::native_clipboard::set_image(const neogfx::image&)
        {
            throw unsupported_clipboard_operation(); // todo
        }

        void display_server::native_clipboard::offer(wl_data_offer* aOffer)
        {
            iOffers[aOffer];
            ::wl_data_offer_add_listener(aOffer, &sDataOfferListener, &iDisplayServer);
        }

        void display_server::native_clipboard::offer_mime_type(wl_data_offer* aOffer, char const* aMimeType)
        {
            iOffers[aOffer].insert(aMimeType);
        }

        void display_server::native_clipboard::selection(wl_data_offer* aOffer)
        {
            if (iSelection != nullptr && iSelection != aOffer)
            {
                iOffers.erase(iSelection);
                ::wl_data_offer_destroy(iSelection);
            }
            iSelection = aOffer;
            // (offers made for drag and drop, which neoGFX does not take, are let go of here too)
            for (auto o = iOffers.begin(); o != iOffers.end();)
            {
                if (o->first != iSelection)
                {
                    ::wl_data_offer_destroy(o->first);
                    o = iOffers.erase(o);
                }
                else
                    ++o;
            }
            service<i_clipboard>().updated()();
        }

        void display_server::native_clipboard::send(wl_data_source* aSource, char const*, int aFd)
        {
            if (aSource == iSource)
            {
                char const* data = iSourceText.data();
                std::size_t remaining = iSourceText.size();
                while (remaining > 0u)
                {
                    auto const written = ::write(aFd, data, remaining);
                    if (written <= 0)
                        break;
                    data += written;
                    remaining -= static_cast<std::size_t>(written);
                }
            }
            ::close(aFd);
        }

        void display_server::native_clipboard::cancelled(wl_data_source* aSource)
        {
            if (aSource == iSource)
            {
                iSource = nullptr;
                iSourceText.clear();
            }
            ::wl_data_source_destroy(aSource);
        }

        std::optional<std::string> display_server::native_clipboard::text_mime_type() const
        {
            if (iSelection == nullptr)
                return {};
            auto const existing = iOffers.find(iSelection);
            if (existing == iOffers.end())
                return {};
            for (auto mimeType : sTextMimeTypes)
                if (existing->second.contains(mimeType))
                    return mimeType;
            return {};
        }

        display_server::display_server(char const* aDisplayName) :
            iClipboard{ std::make_unique<native_clipboard>(*this) }
        {
            iDisplay = ::wl_display_connect(aDisplayName);
            if (iDisplay == nullptr)
                throw failed_to_connect("no Wayland compositor");
            iXkbContext = ::xkb_context_new(XKB_CONTEXT_NO_FLAGS);
            iRegistry = ::wl_display_get_registry(iDisplay);
            ::wl_registry_add_listener(iRegistry, &sRegistryListener, this);
            // (the globals, then the events of those bound (outputs' and the seat's))
            ::wl_display_roundtrip(iDisplay);
            ::wl_display_roundtrip(iDisplay);
            if (iCompositor == nullptr || iWmBase == nullptr || iShm == nullptr)
            {
                std::string const missing = iCompositor == nullptr ? "wl_compositor" : iWmBase == nullptr ? "xdg_wm_base" : "wl_shm";
                release();
                throw failed_to_connect("the Wayland compositor has no " + missing);
            }
        }

        display_server::~display_server()
        {
            release();
        }

        void display_server::release()
        {
            if (iDisplay == nullptr)
                return;
            iRepeatTimer.reset();
            iClipboard.reset();
            if (iCursorSurface != nullptr)
                ::wl_surface_destroy(iCursorSurface);
            if (iCursorTheme != nullptr)
                ::wl_cursor_theme_destroy(iCursorTheme);
            if (iDataDevice != nullptr)
                ::wl_data_device_destroy(iDataDevice);
            if (iPointer != nullptr)
                ::wl_pointer_destroy(iPointer);
            if (iKeyboard != nullptr)
                ::wl_keyboard_destroy(iKeyboard);
            if (iSeat != nullptr)
                ::wl_seat_destroy(iSeat);
            for (auto& o : iOutputs)
                ::wl_output_destroy(o.handle);
            if (iDataDeviceManager != nullptr)
                ::wl_data_device_manager_destroy(iDataDeviceManager);
            if (iDecorationManager != nullptr)
                ::zxdg_decoration_manager_v1_destroy(iDecorationManager);
            if (iWmBase != nullptr)
                ::xdg_wm_base_destroy(iWmBase);
            if (iShm != nullptr)
                ::wl_shm_destroy(iShm);
            if (iCompositor != nullptr)
                ::wl_compositor_destroy(iCompositor);
            if (iRegistry != nullptr)
                ::wl_registry_destroy(iRegistry);
            if (iXkbComposeState != nullptr)
                ::xkb_compose_state_unref(iXkbComposeState);
            if (iXkbComposeTable != nullptr)
                ::xkb_compose_table_unref(iXkbComposeTable);
            if (iXkbState != nullptr)
                ::xkb_state_unref(iXkbState);
            if (iXkbKeymap != nullptr)
                ::xkb_keymap_unref(iXkbKeymap);
            if (iXkbContext != nullptr)
                ::xkb_context_unref(iXkbContext);
            ::wl_display_disconnect(iDisplay);
            iDisplay = nullptr;
        }

        neogfx::windowing_system display_server::windowing_system() const
        {
            return neogfx::windowing_system::Wayland;
        }

        void* display_server::native_display() const
        {
            return iDisplay;
        }

        bool display_server::dispatch_events()
        {
            bool didSome = false;
            while (::wl_display_prepare_read(iDisplay) != 0)
                didSome = (::wl_display_dispatch_pending(iDisplay) > 0) || didSome;
            ::wl_display_flush(iDisplay);
            pollfd pfd{ ::wl_display_get_fd(iDisplay), POLLIN, 0 };
            if (::poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN) != 0)
                ::wl_display_read_events(iDisplay);
            else
                ::wl_display_cancel_read(iDisplay);
            didSome = (::wl_display_dispatch_pending(iDisplay) > 0) || didSome;
            if (auto const error = ::wl_display_get_error(iDisplay); error != 0)
                throw std::runtime_error{ "neogfx::native::wayland::display_server: connection to the Wayland compositor lost: " + std::string{ std::strerror(error) } };
            return didSome;
        }

        void display_server::flush()
        {
            ::wl_display_flush(iDisplay);
        }

        std::uint32_t display_server::output_count() const
        {
            return std::max<std::uint32_t>(1u, static_cast<std::uint32_t>(iOutputs.size()));
        }

        linux::output_info display_server::output(std::uint32_t aIndex) const
        {
            linux::output_info result;
            if (iOutputs.empty())
            {
                // (no outputs (yet), e.g. a headless compositor)
                result.monitor = rect{ point{}, size{ 1920.0, 1080.0 } };
                result.workArea = result.monitor;
                result.dpi = size{ 96.0, 96.0 };
                result.subpixelFormat = subpixel_format::RGBHorizontal;
                return result;
            }
            auto const& o = iOutputs.at(aIndex);
            result.scale = o.scale;
            result.monitor = rect{ o.position * static_cast<scalar>(o.scale), o.extents };
            result.workArea = result.monitor; // n.b. Wayland does not say what the work area is
            result.dpi = size{ 96.0 * o.scale, 96.0 * o.scale };
            switch (o.subpixel)
            {
            case WL_OUTPUT_SUBPIXEL_NONE:
                result.subpixelFormat = subpixel_format::None;
                break;
            case WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR:
                result.subpixelFormat = subpixel_format::BGRHorizontal;
                break;
            case WL_OUTPUT_SUBPIXEL_VERTICAL_RGB:
                result.subpixelFormat = subpixel_format::RGBVertical;
                break;
            case WL_OUTPUT_SUBPIXEL_VERTICAL_BGR:
                result.subpixelFormat = subpixel_format::BGRVertical;
                break;
            default:
                result.subpixelFormat = subpixel_format::RGBHorizontal;
                break;
            }
            return result;
        }

        bool display_server::is_key_pressed(std::uint32_t aKeyCode) const
        {
            return iKeysPressed.contains(aKeyCode);
        }

        std::uint32_t display_server::key_symbol(std::uint32_t aKeyCode) const
        {
            if (iXkbKeymap == nullptr)
                return XKB_KEY_NoSymbol;
            xkb_keysym_t const* symbols = nullptr;
            auto const layout = iXkbState != nullptr ? ::xkb_state_key_get_layout(iXkbState, aKeyCode + 8u) : 0u;
            auto const count = ::xkb_keymap_key_get_syms_by_level(iXkbKeymap, aKeyCode + 8u, layout == XKB_LAYOUT_INVALID ? 0u : layout, 0u, &symbols);
            if (count > 0)
                return symbols[0];
            return XKB_KEY_NoSymbol;
        }

        key_modifier display_server::modifiers() const
        {
            key_modifier result = key_modifier::None;
            if (iXkbState == nullptr)
                return result;
            auto const active = [&](char const* aName)
            {
                return ::xkb_state_mod_name_is_active(iXkbState, aName, XKB_STATE_MODS_EFFECTIVE) > 0;
            };
            auto const either = [&](std::uint32_t aLeftKey, std::uint32_t aRightKey, key_modifier aLeft, key_modifier aRight)
            {
                key_modifier sides = key_modifier::None;
                if (is_key_pressed(aLeftKey))
                    sides = sides | aLeft;
                if (is_key_pressed(aRightKey))
                    sides = sides | aRight;
                return sides != key_modifier::None ? sides : aLeft;
            };
            if (active(XKB_MOD_NAME_SHIFT))
                result = result | either(KEY_LEFTSHIFT, KEY_RIGHTSHIFT, key_modifier::LSHIFT, key_modifier::RSHIFT);
            if (active(XKB_MOD_NAME_CTRL))
                result = result | either(KEY_LEFTCTRL, KEY_RIGHTCTRL, key_modifier::LCTRL, key_modifier::RCTRL);
            if (active(XKB_MOD_NAME_ALT))
                result = result | either(KEY_LEFTALT, KEY_RIGHTALT, key_modifier::LALT, key_modifier::RALT);
            if (active(XKB_MOD_NAME_LOGO))
                result = result | either(KEY_LEFTMETA, KEY_RIGHTMETA, key_modifier::LGUI, key_modifier::RGUI);
            auto const lockState = locks();
            if ((lockState & keyboard_locks::CapsLock) == keyboard_locks::CapsLock)
                result = result | key_modifier::CAPS;
            if ((lockState & keyboard_locks::NumLock) == keyboard_locks::NumLock)
                result = result | key_modifier::NUM;
            if ((lockState & keyboard_locks::ScrollLock) == keyboard_locks::ScrollLock)
                result = result | key_modifier::SCROLL;
            return result;
        }

        keyboard_locks display_server::locks() const
        {
            keyboard_locks result = keyboard_locks::None;
            if (iXkbState != nullptr)
            {
                if (::xkb_state_mod_name_is_active(iXkbState, XKB_MOD_NAME_CAPS, XKB_STATE_MODS_LOCKED) > 0)
                    result = result | keyboard_locks::CapsLock;
                if (::xkb_state_mod_name_is_active(iXkbState, XKB_MOD_NAME_NUM, XKB_STATE_MODS_LOCKED) > 0)
                    result = result | keyboard_locks::NumLock;
                if (::xkb_state_led_name_is_active(iXkbState, XKB_LED_NAME_SCROLL) > 0)
                    result = result | keyboard_locks::ScrollLock;
            }
            if (iInsertLock)
                result = result | keyboard_locks::InsertLock;
            return result;
        }

        point display_server::pointer_position() const
        {
            return iPointerPosition;
        }

        mouse_button display_server::pointer_buttons() const
        {
            return iPointerButtons;
        }

        i_native_window* display_server::window_under_pointer() const
        {
            return iPointerFocus;
        }

        void display_server::set_cursor(mouse_system_cursor aCursor)
        {
            if (iCursor == aCursor)
                return;
            iCursor = aCursor;
            update_cursor();
        }

        void display_server::capture_pointer(i_native_window& aWindow)
        {
            // n.b. a Wayland compositor carries on sending a surface pointer events, wherever the pointer is, for
            // as long as a button is held down (implicit grab), which is all the capturing a client can do
            for (auto w : iWindows)
                if (static_cast<i_native_window*>(w) == &aWindow)
                    iCapture = w;
        }

        void display_server::release_pointer()
        {
            iCapture = std::nullopt;
        }

        i_native_clipboard& display_server::clipboard()
        {
            return *iClipboard;
        }

        void display_server::beep()
        {
            // n.b. no Wayland protocol for this (yet) that compositors have in common
            std::cout << '\a' << std::flush;
        }

        color display_server::read_pixel(point const&) const
        {
            // n.b. a Wayland client can only see its own surfaces
            return color{};
        }

        wl_display* display_server::display() const
        {
            return iDisplay;
        }

        wl_compositor* display_server::compositor() const
        {
            return iCompositor;
        }

        xdg_wm_base* display_server::wm_base() const
        {
            return iWmBase;
        }

        std::uint32_t display_server::wm_base_version() const
        {
            return iWmBaseVersion;
        }

        zxdg_decoration_manager_v1* display_server::decoration_manager() const
        {
            return iDecorationManager;
        }

        wl_seat* display_server::seat() const
        {
            return iSeat;
        }

        std::uint32_t display_server::input_serial() const
        {
            return iInputSerial;
        }

        std::uint32_t display_server::pointer_button_serial() const
        {
            return iPointerButtonSerial;
        }

        std::optional<std::uint32_t> display_server::output_scale(wl_output* aOutput) const
        {
            for (auto const& o : iOutputs)
                if (o.handle == aOutput)
                    return o.scale;
            return {};
        }

        std::uint32_t display_server::max_output_scale() const
        {
            std::uint32_t result = 1u;
            for (auto const& o : iOutputs)
                result = std::max(result, o.scale);
            return result;
        }

        bool display_server::has_preferred_buffer_scale() const
        {
#ifdef WL_SURFACE_PREFERRED_BUFFER_SCALE_SINCE_VERSION
            return iCompositorVersion >= WL_SURFACE_PREFERRED_BUFFER_SCALE_SINCE_VERSION;
#else
            return false;
#endif
        }

        void display_server::add_window(window& aWindow)
        {
            iWindows.push_back(&aWindow);
        }

        void display_server::remove_window(window& aWindow)
        {
            iWindows.erase(std::remove(iWindows.begin(), iWindows.end(), &aWindow), iWindows.end());
            if (iPointerFocus == &aWindow)
                iPointerFocus = nullptr;
            if (iKeyboardFocus == &aWindow)
            {
                iKeyboardFocus = nullptr;
                stop_key_repeat();
            }
            if (iCapture == &aWindow)
                iCapture = std::nullopt;
        }

        window* display_server::keyboard_focus() const
        {
            return iKeyboardFocus;
        }

        void display_server::registry_global(wl_registry* aRegistry, std::uint32_t aName, char const* aInterface, std::uint32_t aVersion)
        {
            std::string_view const interface{ aInterface };
            if (interface == ::wl_compositor_interface.name)
            {
                iCompositorVersion = std::min<std::uint32_t>(aVersion, 6u);
                iCompositor = static_cast<wl_compositor*>(::wl_registry_bind(aRegistry, aName, &::wl_compositor_interface, iCompositorVersion));
            }
            else if (interface == ::wl_shm_interface.name)
                iShm = static_cast<wl_shm*>(::wl_registry_bind(aRegistry, aName, &::wl_shm_interface, 1u));
            else if (interface == ::xdg_wm_base_interface.name)
            {
                iWmBaseVersion = std::min<std::uint32_t>(aVersion, 5u);
                iWmBase = static_cast<xdg_wm_base*>(::wl_registry_bind(aRegistry, aName, &::xdg_wm_base_interface, iWmBaseVersion));
                ::xdg_wm_base_add_listener(iWmBase, &sWmBaseListener, this);
            }
            else if (interface == ::zxdg_decoration_manager_v1_interface.name)
                iDecorationManager = static_cast<zxdg_decoration_manager_v1*>(::wl_registry_bind(aRegistry, aName, &::zxdg_decoration_manager_v1_interface, 1u));
            else if (interface == ::wl_data_device_manager_interface.name)
            {
                iDataDeviceManager = static_cast<wl_data_device_manager*>(::wl_registry_bind(aRegistry, aName, &::wl_data_device_manager_interface, std::min<std::uint32_t>(aVersion, 3u)));
                if (iSeat != nullptr && iDataDevice == nullptr)
                {
                    iDataDevice = ::wl_data_device_manager_get_data_device(iDataDeviceManager, iSeat);
                    ::wl_data_device_add_listener(iDataDevice, &sDataDeviceListener, this);
                }
            }
            else if (interface == ::wl_seat_interface.name && iSeat == nullptr)
            {
#ifdef WL_POINTER_AXIS_VALUE120_SINCE_VERSION
                iSeatVersion = std::min<std::uint32_t>(aVersion, 8u);
#else
                iSeatVersion = std::min<std::uint32_t>(aVersion, 7u);
#endif
                iSeat = static_cast<wl_seat*>(::wl_registry_bind(aRegistry, aName, &::wl_seat_interface, iSeatVersion));
                ::wl_seat_add_listener(iSeat, &sSeatListener, this);
                if (iDataDeviceManager != nullptr && iDataDevice == nullptr)
                {
                    iDataDevice = ::wl_data_device_manager_get_data_device(iDataDeviceManager, iSeat);
                    ::wl_data_device_add_listener(iDataDevice, &sDataDeviceListener, this);
                }
            }
            else if (interface == ::wl_output_interface.name)
            {
                output_state newOutput;
                newOutput.name = aName;
                newOutput.handle = static_cast<wl_output*>(::wl_registry_bind(aRegistry, aName, &::wl_output_interface, std::min<std::uint32_t>(aVersion, 2u)));
                ::wl_output_add_listener(newOutput.handle, &sOutputListener, this);
                iOutputs.push_back(newOutput);
            }
        }

        void display_server::registry_global_remove(wl_registry*, std::uint32_t aName)
        {
            for (auto o = iOutputs.begin(); o != iOutputs.end(); ++o)
                if (o->name == aName)
                {
                    ::wl_output_destroy(o->handle);
                    iOutputs.erase(o);
                    for (auto w : iWindows)
                        w->outputs_changed();
                    break;
                }
        }

        void display_server::seat_capabilities(std::uint32_t aCapabilities)
        {
            if ((aCapabilities & WL_SEAT_CAPABILITY_POINTER) != 0 && iPointer == nullptr)
            {
                iPointer = ::wl_seat_get_pointer(iSeat);
                ::wl_pointer_add_listener(iPointer, &sPointerListener, this);
            }
            else if ((aCapabilities & WL_SEAT_CAPABILITY_POINTER) == 0 && iPointer != nullptr)
            {
                ::wl_pointer_destroy(iPointer);
                iPointer = nullptr;
                iPointerFocus = nullptr;
            }
            if ((aCapabilities & WL_SEAT_CAPABILITY_KEYBOARD) != 0 && iKeyboard == nullptr)
            {
                iKeyboard = ::wl_seat_get_keyboard(iSeat);
                ::wl_keyboard_add_listener(iKeyboard, &sKeyboardListener, this);
            }
            else if ((aCapabilities & WL_SEAT_CAPABILITY_KEYBOARD) == 0 && iKeyboard != nullptr)
            {
                ::wl_keyboard_destroy(iKeyboard);
                iKeyboard = nullptr;
                iKeyboardFocus = nullptr;
                stop_key_repeat();
            }
        }

        void display_server::output_geometry(wl_output* aOutput, std::int32_t aX, std::int32_t aY, std::int32_t aPhysicalWidth, std::int32_t aPhysicalHeight, std::int32_t aSubpixel)
        {
            for (auto& o : iOutputs)
                if (o.handle == aOutput)
                {
                    o.position = basic_point<std::int32_t>{ aX, aY };
                    o.physicalSize = basic_size<std::int32_t>{ aPhysicalWidth, aPhysicalHeight };
                    o.subpixel = aSubpixel;
                }
        }

        void display_server::output_mode(wl_output* aOutput, std::uint32_t aFlags, std::int32_t aWidth, std::int32_t aHeight)
        {
            if ((aFlags & WL_OUTPUT_MODE_CURRENT) == 0)
                return;
            for (auto& o : iOutputs)
                if (o.handle == aOutput)
                    o.extents = basic_size<std::int32_t>{ aWidth, aHeight };
        }

        void display_server::output_scale_changed(wl_output* aOutput, std::int32_t aScale)
        {
            for (auto& o : iOutputs)
                if (o.handle == aOutput)
                    o.scale = static_cast<std::uint32_t>(std::max(1, aScale));
        }

        void display_server::output_done(wl_output*)
        {
            for (auto w : iWindows)
                w->outputs_changed();
        }

        namespace
        {
            window* window_of(std::vector<window*> const& aWindows, wl_surface* aSurface)
            {
                if (aSurface == nullptr)
                    return nullptr;
                for (auto w : aWindows)
                    if (w->surface() == aSurface)
                        return w;
                return nullptr;
            }
        }

        void display_server::pointer_enter(std::uint32_t aSerial, wl_surface* aSurface, double aX, double aY)
        {
            iPointerEnterSerial = aSerial;
            iPointerFocus = window_of(iWindows, aSurface);
            update_cursor();
            if (iPointerFocus != nullptr)
            {
                point const position{ aX * iPointerFocus->scale(), aY * iPointerFocus->scale() };
                iPointerPosition = iPointerFocus->surface_position() + position;
                iPointerFocus->pointer_entered(position);
            }
        }

        void display_server::pointer_leave(std::uint32_t, wl_surface* aSurface)
        {
            auto const left = window_of(iWindows, aSurface);
            // (no button is down when the pointer leaves, other than when the compositor has taken it, e.g. to move
            // the window, in which case the button's release will not be seen)
            iPointerButtons = mouse_button::None;
            if (left != nullptr)
                left->pointer_left();
            if (left == iPointerFocus)
                iPointerFocus = nullptr;
        }

        void display_server::pointer_motion(double aX, double aY)
        {
            if (iPointerFocus == nullptr)
                return;
            point const position{ aX * iPointerFocus->scale(), aY * iPointerFocus->scale() };
            iPointerPosition = iPointerFocus->surface_position() + position;
            iPointerFocus->pointer_moved(position);
        }

        void display_server::pointer_button(std::uint32_t aSerial, std::uint32_t aButton, std::uint32_t aState)
        {
            auto const button = convert_button(aButton);
            if (button == mouse_button::None)
                return;
            bool const pressed = (aState == WL_POINTER_BUTTON_STATE_PRESSED);
            if (pressed)
            {
                iPointerButtons = iPointerButtons | button;
                iPointerButtonSerial = aSerial;
                iInputSerial = aSerial;
            }
            else
                iPointerButtons = iPointerButtons & ~button;
            if (iPointerFocus != nullptr)
                iPointerFocus->pointer_button(button, pressed, aSerial);
        }

        void display_server::pointer_axis(std::uint32_t aAxis, double aValue)
        {
            if (aAxis == WL_POINTER_AXIS_VERTICAL_SCROLL)
                iAxis.dy += aValue;
            else
                iAxis.dx += aValue;
            if (iSeatVersion < WL_POINTER_FRAME_SINCE_VERSION)
                pointer_frame();
        }

        void display_server::pointer_axis_discrete(std::uint32_t aAxis, std::int32_t aDiscrete)
        {
            if (iAxisValue120)
                return;
            if (aAxis == WL_POINTER_AXIS_VERTICAL_SCROLL)
                iAxisDiscrete.dy += aDiscrete * linux::WHEEL_DELTA;
            else
                iAxisDiscrete.dx += aDiscrete * linux::WHEEL_DELTA;
        }

        void display_server::pointer_axis_value120(std::uint32_t aAxis, std::int32_t aValue120)
        {
            iAxisValue120 = true;
            if (aAxis == WL_POINTER_AXIS_VERTICAL_SCROLL)
                iAxisDiscrete.dy += aValue120;
            else
                iAxisDiscrete.dx += aValue120;
        }

        void display_server::pointer_frame()
        {
            // n.b. a wheel's notches are discrete steps; a touchpad scrolls continuously, by (about) ten surface
            // pixels a notch
            constexpr double PIXELS_PER_NOTCH = 10.0;
            delta wheel;
            if (iAxisDiscrete.dx != 0.0 || iAxisDiscrete.dy != 0.0)
            {
                wheel = iAxisDiscrete;
                iAxis = {};
            }
            else
            {
                auto const notchesX = std::trunc(iAxis.dx / PIXELS_PER_NOTCH);
                auto const notchesY = std::trunc(iAxis.dy / PIXELS_PER_NOTCH);
                wheel = delta{ notchesX * linux::WHEEL_DELTA, notchesY * linux::WHEEL_DELTA };
                iAxis.dx -= notchesX * PIXELS_PER_NOTCH;
                iAxis.dy -= notchesY * PIXELS_PER_NOTCH;
            }
            iAxisDiscrete = {};
            if (iPointerFocus == nullptr)
                return;
            // (as Windows: positive is away from the user (wheel up) and to the right)
            if (wheel.dy != 0.0)
                iPointerFocus->pointer_wheel(mouse_wheel::Vertical, delta{ 0.0, -wheel.dy });
            if (wheel.dx != 0.0)
                iPointerFocus->pointer_wheel(mouse_wheel::Horizontal, delta{ wheel.dx, 0.0 });
        }

        void display_server::keyboard_keymap(std::uint32_t aFormat, std::int32_t aFd, std::uint32_t aSize)
        {
            if (aFormat != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
            {
                ::close(aFd);
                return;
            }
            auto const map = ::mmap(nullptr, aSize, PROT_READ, MAP_PRIVATE, aFd, 0);
            ::close(aFd);
            if (map == MAP_FAILED)
                return;
            auto const keymap = ::xkb_keymap_new_from_string(iXkbContext, static_cast<char const*>(map), XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
            ::munmap(map, aSize);
            if (keymap == nullptr)
                return;
            if (iXkbState != nullptr)
                ::xkb_state_unref(iXkbState);
            if (iXkbKeymap != nullptr)
                ::xkb_keymap_unref(iXkbKeymap);
            iXkbKeymap = keymap;
            iXkbState = ::xkb_state_new(iXkbKeymap);
            if (iXkbComposeTable == nullptr)
            {
                char const* locale = std::getenv("LC_ALL");
                if (locale == nullptr || *locale == '\0')
                    locale = std::getenv("LC_CTYPE");
                if (locale == nullptr || *locale == '\0')
                    locale = std::getenv("LANG");
                if (locale == nullptr || *locale == '\0')
                    locale = "C";
                iXkbComposeTable = ::xkb_compose_table_new_from_locale(iXkbContext, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
                if (iXkbComposeTable != nullptr)
                    iXkbComposeState = ::xkb_compose_state_new(iXkbComposeTable, XKB_COMPOSE_STATE_NO_FLAGS);
            }
            if (service<i_keyboard>().is_enabled())
                service<i_keyboard>().input_language_changed()();
        }

        void display_server::keyboard_enter(std::uint32_t aSerial, wl_surface* aSurface)
        {
            iInputSerial = aSerial;
            iKeyboardFocus = window_of(iWindows, aSurface);
            if (iKeyboardFocus != nullptr)
                iKeyboardFocus->keyboard_focus_gained();
        }

        void display_server::keyboard_leave(std::uint32_t, wl_surface* aSurface)
        {
            stop_key_repeat();
            iKeysPressed.clear();
            if (iXkbComposeState != nullptr)
                ::xkb_compose_state_reset(iXkbComposeState);
            auto const left = window_of(iWindows, aSurface);
            if (left == iKeyboardFocus)
                iKeyboardFocus = nullptr;
            if (left != nullptr)
                left->keyboard_focus_lost();
        }

        void display_server::keyboard_key(std::uint32_t aSerial, std::uint32_t aKey, std::uint32_t aState)
        {
            iInputSerial = aSerial;
            bool const pressed = (aState == WL_KEYBOARD_KEY_STATE_PRESSED);
            if (pressed)
                iKeysPressed.insert(aKey);
            else
                iKeysPressed.erase(aKey);
            key(aKey, pressed, false);
            if (pressed)
            {
                if (iRepeatRate > 0 && iXkbKeymap != nullptr && ::xkb_keymap_key_repeats(iXkbKeymap, aKey + 8u))
                {
                    iRepeatKey = aKey;
                    iRepeatTimer.emplace(service<i_async_task>(), [this](neolib::callback_timer& aTimer)
                    {
                        if (!iRepeatKey)
                            return;
                        key(*iRepeatKey, true, true);
                        aTimer.set_duration(std::chrono::duration<double>{ 1.0 / iRepeatRate });
                        aTimer.again();
                    }, std::chrono::duration<double>{ iRepeatDelay / 1000.0 });
                }
            }
            else if (iRepeatKey == aKey)
                stop_key_repeat();
        }

        void display_server::keyboard_modifiers(std::uint32_t, std::uint32_t aDepressed, std::uint32_t aLatched, std::uint32_t aLocked, std::uint32_t aGroup)
        {
            if (iXkbState == nullptr)
                return;
            auto const previousLayout = ::xkb_state_serialize_layout(iXkbState, XKB_STATE_LAYOUT_EFFECTIVE);
            ::xkb_state_update_mask(iXkbState, aDepressed, aLatched, aLocked, 0u, 0u, aGroup);
            if (::xkb_state_serialize_layout(iXkbState, XKB_STATE_LAYOUT_EFFECTIVE) != previousLayout && service<i_keyboard>().is_enabled())
                service<i_keyboard>().input_language_changed()();
        }

        void display_server::keyboard_repeat_info(std::int32_t aRate, std::int32_t aDelay)
        {
            iRepeatRate = aRate;
            iRepeatDelay = aDelay;
        }

        void display_server::data_device_offer(wl_data_offer* aOffer)
        {
            iClipboard->offer(aOffer);
        }

        void display_server::data_device_selection(wl_data_offer* aOffer)
        {
            iClipboard->selection(aOffer);
        }

        void display_server::data_offer_offer(wl_data_offer* aOffer, char const* aMimeType)
        {
            iClipboard->offer_mime_type(aOffer, aMimeType);
        }

        void display_server::data_source_send(wl_data_source* aSource, char const* aMimeType, std::int32_t aFd)
        {
            iClipboard->send(aSource, aMimeType, aFd);
        }

        void display_server::data_source_cancelled(wl_data_source* aSource)
        {
            iClipboard->cancelled(aSource);
        }

        void display_server::key(std::uint32_t aKey, bool aPressed, bool aRepeat)
        {
            auto const target = keyboard_target();
            if (target == nullptr || !target->enabled())
                return;
            auto const scanCode = linux::evdev_to_scan_code(aKey);
            auto const keyCode = service<i_keyboard>().scan_code_to_key_code(scanCode);
            xkb_keycode_t const xkbKeyCode = aKey + 8u;
            auto const keySymbol = iXkbState != nullptr ? ::xkb_state_key_get_one_sym(iXkbState, xkbKeyCode) : XKB_KEY_NoSymbol;
            auto modifiers = this->modifiers();
            // as Windows: a key pressed with Alt (but not with Ctrl, as AltGr is) is a "system" key, as F10 is
            bool const systemKey = ((modifiers & key_modifier::ALT) != key_modifier::None && (modifiers & key_modifier::CTRL) == key_modifier::None) || aKey == KEY_F10;
            if (systemKey)
                modifiers = modifiers | key_modifier::SYSTEM;
            switch (aKey)
            {
            case KEY_KPENTER:
            case KEY_RIGHTCTRL:
            case KEY_RIGHTALT:
            case KEY_KPSLASH:
            case KEY_HOME:
            case KEY_UP:
            case KEY_PAGEUP:
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_END:
            case KEY_DOWN:
            case KEY_PAGEDOWN:
            case KEY_INSERT:
            case KEY_DELETE:
                modifiers = modifiers | key_modifier::EXTENDED;
                break;
            default:
                break;
            }
            if (aPressed)
            {
                if (aKey == KEY_INSERT && !aRepeat)
                    iInsertLock = !iInsertLock;
                target->push_event(keyboard_event{
                    keyboard_event_type::KeyPressed, scanCode, keyCode, modifiers,
                    static_cast<native_scan_code_e>(aKey), static_cast<native_key_code_e>(keySymbol) });
                std::string text;
                if (iXkbComposeState != nullptr && !aRepeat)
                {
                    ::xkb_compose_state_feed(iXkbComposeState, keySymbol);
                    switch (::xkb_compose_state_get_status(iXkbComposeState))
                    {
                    case XKB_COMPOSE_COMPOSING:
                        break;
                    case XKB_COMPOSE_COMPOSED:
                        {
                            char buffer[64] = {};
                            ::xkb_compose_state_get_utf8(iXkbComposeState, buffer, sizeof(buffer));
                            text = buffer;
                            ::xkb_compose_state_reset(iXkbComposeState);
                        }
                        break;
                    case XKB_COMPOSE_CANCELLED:
                        ::xkb_compose_state_reset(iXkbComposeState);
                        break;
                    case XKB_COMPOSE_NOTHING:
                    default:
                        if (iXkbState != nullptr)
                        {
                            char buffer[64] = {};
                            ::xkb_state_key_get_utf8(iXkbState, xkbKeyCode, buffer, sizeof(buffer));
                            text = buffer;
                        }
                        break;
                    }
                }
                else if (iXkbState != nullptr)
                {
                    char buffer[64] = {};
                    ::xkb_state_key_get_utf8(iXkbState, xkbKeyCode, buffer, sizeof(buffer));
                    text = buffer;
                }
                if (!text.empty() && (text.size() > 1 || text[0] >= ' ' || std::isspace(text[0])))
                    target->push_event(keyboard_event{
                        systemKey ? keyboard_event_type::SysTextInput : keyboard_event_type::TextInput,
                        string{ text }, KeyCode_UNKNOWN, modifiers,
                        static_cast<native_scan_code_e>(aKey), static_cast<native_key_code_e>(keySymbol) });
            }
            else
                target->push_event(keyboard_event{
                    keyboard_event_type::KeyReleased, scanCode, keyCode, modifiers,
                    static_cast<native_scan_code_e>(aKey), static_cast<native_key_code_e>(keySymbol) });
            if (!target->is_non_client_capturing())
                service<i_window_manager>().update_mouse_cursor(target->surface_window().as_window());
        }

        void display_server::stop_key_repeat()
        {
            iRepeatKey = std::nullopt;
            iRepeatTimer.reset();
        }

        window* display_server::keyboard_target() const
        {
            if (iKeyboardFocus == nullptr)
                return nullptr;
            // n.b. a popup cannot have the keyboard focus on Wayland (without a grab, which neoGFX's popups (as
            // its windows on Windows) do not need), so the keyboard's events go to the window neoGFX has made
            // active if it is a popup of the window that has the focus
            auto& windowManager = service<i_window_manager>();
            if (windowManager.window_activated())
            {
                auto& active = windowManager.active_window();
                if (active.has_native_surface())
                {
                    auto const& activeNativeWindow = active.native_window();
                    for (auto w : iWindows)
                        if (static_cast<i_native_window const*>(w) == &activeNativeWindow && w->toplevel() == iKeyboardFocus)
                            return w;
                }
            }
            return iKeyboardFocus;
        }

        void display_server::update_cursor()
        {
            if (iPointer == nullptr || iPointerFocus == nullptr || iShm == nullptr)
                return;
            auto const scale = iPointerFocus->scale();
            if (iCursorTheme == nullptr || iCursorThemeScale != scale)
                update_cursor_theme(scale);
            if (iCursorTheme == nullptr)
                return;
            wl_cursor* cursor = nullptr;
            for (auto name : linux::cursor_names(iCursor))
                if ((cursor = ::wl_cursor_theme_get_cursor(iCursorTheme, name)) != nullptr)
                    break;
            if (cursor == nullptr || cursor->image_count == 0u)
                return;
            auto const image = cursor->images[0];
            auto const buffer = ::wl_cursor_image_get_buffer(image);
            if (buffer == nullptr)
                return;
            if (iCursorSurface == nullptr)
                iCursorSurface = ::wl_compositor_create_surface(iCompositor);
            ::wl_pointer_set_cursor(iPointer, iPointerEnterSerial, iCursorSurface,
                static_cast<std::int32_t>(image->hotspot_x / scale), static_cast<std::int32_t>(image->hotspot_y / scale));
            ::wl_surface_set_buffer_scale(iCursorSurface, static_cast<std::int32_t>(scale));
            ::wl_surface_attach(iCursorSurface, buffer, 0, 0);
            ::wl_surface_damage(iCursorSurface, 0, 0, static_cast<std::int32_t>(image->width), static_cast<std::int32_t>(image->height));
            ::wl_surface_commit(iCursorSurface);
        }

        void display_server::update_cursor_theme(std::uint32_t aScale)
        {
            if (iCursorTheme != nullptr)
                ::wl_cursor_theme_destroy(iCursorTheme);
            int size = 24;
            if (auto const sizeVariable = std::getenv("XCURSOR_SIZE"); sizeVariable != nullptr && std::atoi(sizeVariable) > 0)
                size = std::atoi(sizeVariable);
            iCursorTheme = ::wl_cursor_theme_load(std::getenv("XCURSOR_THEME"), size * static_cast<int>(aScale), iShm);
            iCursorThemeScale = aScale;
        }
    }
}
