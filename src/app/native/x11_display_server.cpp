// x11_display_server.cpp
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

#include <clocale>
#include <climits>
#include <cstring>
#include <poll.h>
#include <linux/input-event-codes.h>

#include <neogfx/app/i_app.hpp>
#include <neogfx/app/i_clipboard.hpp>
#include <neogfx/hid/i_window_manager.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include "../../gui/window/native/x11_window.hpp"
#include "x11_display_server.hpp"
#include "x11.hpp"
#include <X11/cursorfont.h>

namespace neogfx
{
    namespace native::x11
    {
        namespace
        {
            char const* const sAtomNames[] =
            {
                "WM_PROTOCOLS",
                "WM_DELETE_WINDOW",
                "_NET_WM_PING",
                "_NET_WM_PID",
                "_NET_WM_NAME",
                "_NET_WM_STATE",
                "_NET_WM_STATE_MAXIMIZED_VERT",
                "_NET_WM_STATE_MAXIMIZED_HORZ",
                "_NET_WM_STATE_FULLSCREEN",
                "_NET_WM_STATE_HIDDEN",
                "_NET_WM_STATE_DEMANDS_ATTENTION",
                "_NET_WM_STATE_SKIP_TASKBAR",
                "_NET_WM_STATE_ABOVE",
                "_NET_WM_WINDOW_TYPE",
                "_NET_WM_WINDOW_TYPE_NORMAL",
                "_NET_WM_WINDOW_TYPE_DIALOG",
                "_NET_WM_WINDOW_TYPE_UTILITY",
                "_NET_WM_WINDOW_TYPE_SPLASH",
                "_NET_WM_WINDOW_TYPE_POPUP_MENU",
                "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
                "_NET_WM_WINDOW_TYPE_TOOLTIP",
                "_NET_WM_WINDOW_OPACITY",
                "_NET_WM_MOVERESIZE",
                "_NET_ACTIVE_WINDOW",
                "_NET_SUPPORTED",
                "_NET_WORKAREA",
                "_MOTIF_WM_HINTS",
                "UTF8_STRING",
                "CLIPBOARD",
                "TARGETS",
                "TEXT",
                "NEOGFX_SELECTION"
            };
            static_assert(std::size(sAtomNames) == static_cast<std::size_t>(display_server::atom_id::COUNT));

            int error_handler(Display* aDisplay, XErrorEvent* aEvent)
            {
                // (as Xlib's own handler would, but without exiting: e.g. a request for a window just destroyed)
#ifndef NDEBUG
                char text[256] = {};
                ::XGetErrorText(aDisplay, aEvent->error_code, text, sizeof(text));
                std::cerr << "neogfx::native::x11: X error: " << text << " (request " << static_cast<int>(aEvent->request_code) << ")" << std::endl;
#else
                (void)aDisplay;
                (void)aEvent;
#endif
                return 0;
            }

            mouse_button convert_button(unsigned int aButton)
            {
                switch (aButton)
                {
                case Button1:
                    return mouse_button::Left;
                case Button2:
                    return mouse_button::Middle;
                case Button3:
                    return mouse_button::Right;
                case 8:
                    return mouse_button::X1;
                case 9:
                    return mouse_button::X2;
                default:
                    return mouse_button::None;
                }
            }

            int rr_event_base = -1;
        }

        display_server::native_clipboard::native_clipboard(display_server& aDisplayServer) :
            iDisplayServer{ aDisplayServer }
        {
        }

        bool display_server::native_clipboard::has_text() const
        {
            if (iOwnText)
                return !iOwnText->empty();
            return ::XGetSelectionOwner(iDisplayServer.display(), iDisplayServer.atom(atom_id::CLIPBOARD)) != XNone;
        }

        i_string const& display_server::native_clipboard::text() const
        {
            iText.clear();
            if (iOwnText)
            {
                iText = *iOwnText;
                return iText;
            }
            auto const display = iDisplayServer.display();
            auto const clipboardAtom = iDisplayServer.atom(atom_id::CLIPBOARD);
            if (::XGetSelectionOwner(display, clipboardAtom) == XNone)
                return iText;
            for (Atom const target : { iDisplayServer.atom(atom_id::UTF8_STRING), static_cast<Atom>(XA_STRING) })
            {
                ::XConvertSelection(display, clipboardAtom, target, iDisplayServer.atom(atom_id::NEOGFX_SELECTION), iDisplayServer.iHelperWindow, iDisplayServer.input_time());
                ::XFlush(display);
                // n.b. the selection's owner (another client) converts it; a client that does not is given up on
                XEvent event;
                bool notified = false;
                auto const deadline = std::chrono::steady_clock::now() + std::chrono::seconds{ 1 };
                while (!notified && std::chrono::steady_clock::now() < deadline)
                {
                    if (::XCheckTypedWindowEvent(display, iDisplayServer.iHelperWindow, SelectionNotify, &event))
                        notified = true;
                    else
                    {
                        pollfd pfd{ ConnectionNumber(display), POLLIN, 0 };
                        ::poll(&pfd, 1, 20);
                    }
                }
                if (!notified || event.xselection.property == XNone)
                    continue;
                Atom type = XNone;
                int format = 0;
                unsigned long count = 0ul;
                unsigned long remaining = 0ul;
                unsigned char* data = nullptr;
                if (::XGetWindowProperty(display, iDisplayServer.iHelperWindow, iDisplayServer.atom(atom_id::NEOGFX_SELECTION), 0, LONG_MAX / 4, XTrue,
                    AnyPropertyType, &type, &format, &count, &remaining, &data) == XSuccess && data != nullptr)
                {
                    // (a selection sent incrementally (INCR), as a large one is, is not received)
                    if (format == 8 && type != ::XInternAtom(display, "INCR", XFalse))
                        iText = std::string{ reinterpret_cast<char const*>(data), count };
                    ::XFree(data);
                }
                break;
            }
            return iText;
        }

        void display_server::native_clipboard::set_text(i_string const& aText)
        {
            auto const display = iDisplayServer.display();
            auto const clipboardAtom = iDisplayServer.atom(atom_id::CLIPBOARD);
            iOwnText = aText.to_std_string();
            ::XSetSelectionOwner(display, clipboardAtom, iDisplayServer.iHelperWindow, iDisplayServer.input_time());
            if (::XGetSelectionOwner(display, clipboardAtom) != iDisplayServer.iHelperWindow)
            {
                iOwnText = std::nullopt;
                throw failed_to_set_clipboard_data();
            }
            service<i_clipboard>().updated()();
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

        void display_server::native_clipboard::selection_request(XEvent const& aEvent)
        {
            auto const display = iDisplayServer.display();
            auto const& request = aEvent.xselectionrequest;
            XEvent reply = {};
            reply.xselection.type = SelectionNotify;
            reply.xselection.display = display;
            reply.xselection.requestor = request.requestor;
            reply.xselection.selection = request.selection;
            reply.xselection.target = request.target;
            reply.xselection.property = XNone;
            reply.xselection.time = request.time;
            auto const property = request.property != XNone ? request.property : request.target;
            auto const utf8String = iDisplayServer.atom(atom_id::UTF8_STRING);
            if (request.target == iDisplayServer.atom(atom_id::TARGETS))
            {
                Atom const targets[] = { iDisplayServer.atom(atom_id::TARGETS), utf8String, XA_STRING, iDisplayServer.atom(atom_id::TEXT) };
                ::XChangeProperty(display, request.requestor, property, XA_ATOM, 32, PropModeReplace,
                    reinterpret_cast<unsigned char const*>(targets), static_cast<int>(std::size(targets)));
                reply.xselection.property = property;
            }
            else if (iOwnText && (request.target == utf8String || request.target == XA_STRING || request.target == iDisplayServer.atom(atom_id::TEXT)))
            {
                ::XChangeProperty(display, request.requestor, property, request.target == XA_STRING ? XA_STRING : utf8String, 8, PropModeReplace,
                    reinterpret_cast<unsigned char const*>(iOwnText->data()), static_cast<int>(iOwnText->size()));
                reply.xselection.property = property;
            }
            ::XSendEvent(display, request.requestor, XFalse, 0, &reply);
        }

        void display_server::native_clipboard::selection_clear()
        {
            iOwnText = std::nullopt;
            service<i_clipboard>().updated()();
        }

        display_server::display_server(char const* aDisplayName) :
            iClipboard{ std::make_unique<native_clipboard>(*this) }
        {
            // (for Xutf8LookupString: the input method's text is in the locale's encoding)
            if (auto const locale = std::setlocale(LC_CTYPE, nullptr); locale == nullptr || std::strcmp(locale, "C") == 0)
                std::setlocale(LC_CTYPE, "");
            iDisplay = ::XOpenDisplay(aDisplayName);
            if (iDisplay == nullptr)
                throw failed_to_connect("no X server");
            ::XSetErrorHandler(&error_handler);
            iScreen = DefaultScreen(iDisplay);
            iRoot = RootWindow(iDisplay, iScreen);
            XBool supported = XFalse;
            ::XkbSetDetectableAutoRepeat(iDisplay, XTrue, &supported);
            ::XInternAtoms(iDisplay, const_cast<char**>(sAtomNames), static_cast<int>(std::size(sAtomNames)), XFalse, iAtoms.data());
            {
                Atom type = XNone;
                int format = 0;
                unsigned long count = 0ul;
                unsigned long remaining = 0ul;
                unsigned char* data = nullptr;
                if (::XGetWindowProperty(iDisplay, iRoot, atom(atom_id::NET_SUPPORTED), 0, LONG_MAX / 4, XFalse, XA_ATOM,
                    &type, &format, &count, &remaining, &data) == XSuccess && data != nullptr)
                {
                    auto const atoms = reinterpret_cast<Atom const*>(data);
                    iSupported.insert(atoms, atoms + count);
                    ::XFree(data);
                }
            }
            iHelperWindow = ::XCreateSimpleWindow(iDisplay, iRoot, 0, 0, 1, 1, 0, 0, 0);
            ::XSelectInput(iDisplay, iHelperWindow, PropertyChangeMask);
            if (::XSupportsLocale())
            {
                ::XSetLocaleModifiers("");
                iInputMethod = ::XOpenIM(iDisplay, nullptr, nullptr, nullptr);
            }
            int errorBase = 0;
            if (::XRRQueryExtension(iDisplay, &rr_event_base, &errorBase))
                ::XRRSelectInput(iDisplay, iRoot, RRScreenChangeNotifyMask);
            else
                rr_event_base = -1;
        }

        display_server::~display_server()
        {
            iClipboard.reset();
            for (auto& cursor : iCursors)
                ::XFreeCursor(iDisplay, cursor.second);
            if (iInputMethod != nullptr)
                ::XCloseIM(iInputMethod);
            if (iHelperWindow != 0ul)
                ::XDestroyWindow(iDisplay, iHelperWindow);
            ::XCloseDisplay(iDisplay);
        }

        neogfx::windowing_system display_server::windowing_system() const
        {
            return neogfx::windowing_system::X11;
        }

        void* display_server::native_display() const
        {
            return iDisplay;
        }

        bool display_server::dispatch_events()
        {
            bool didSome = false;
            while (::XPending(iDisplay) > 0)
            {
                XEvent event;
                ::XNextEvent(iDisplay, &event);
                didSome = true;
                // (the input method's: e.g. the keys of a compose sequence)
                if (::XFilterEvent(&event, XNone))
                    continue;
                handle_event(event);
            }
            return didSome;
        }

        void display_server::flush()
        {
            ::XFlush(iDisplay);
        }

        std::uint32_t display_server::output_count() const
        {
            update_outputs();
            return static_cast<std::uint32_t>(iOutputs->size());
        }

        linux::output_info display_server::output(std::uint32_t aIndex) const
        {
            update_outputs();
            return iOutputs->at(aIndex);
        }

        bool display_server::is_key_pressed(std::uint32_t aKeyCode) const
        {
            return iKeysPressed.contains(aKeyCode);
        }

        std::uint32_t display_server::key_symbol(std::uint32_t aKeyCode) const
        {
            // (the layout (group) is that of the latest event's state)
            return static_cast<std::uint32_t>(::XkbKeycodeToKeysym(iDisplay, static_cast<KeyCode>(aKeyCode + 8u), XkbGroupForCoreState(iModifierState), 0));
        }

        key_modifier display_server::modifiers() const
        {
            key_modifier result = key_modifier::None;
            auto const sides = [&](std::uint32_t aLeftKey, std::uint32_t aRightKey, key_modifier aLeft, key_modifier aRight)
            {
                key_modifier pressed = key_modifier::None;
                if (is_key_pressed(aLeftKey))
                    pressed = pressed | aLeft;
                if (is_key_pressed(aRightKey))
                    pressed = pressed | aRight;
                return pressed;
            };
            result = result | sides(KEY_LEFTSHIFT, KEY_RIGHTSHIFT, key_modifier::LSHIFT, key_modifier::RSHIFT);
            result = result | sides(KEY_LEFTCTRL, KEY_RIGHTCTRL, key_modifier::LCTRL, key_modifier::RCTRL);
            result = result | sides(KEY_LEFTALT, KEY_RIGHTALT, key_modifier::LALT, key_modifier::RALT);
            result = result | sides(KEY_LEFTMETA, KEY_RIGHTMETA, key_modifier::LGUI, key_modifier::RGUI);
            auto const lockState = locks();
            if ((lockState & keyboard_locks::CapsLock) == keyboard_locks::CapsLock)
                result = result | key_modifier::CAPS;
            if ((lockState & keyboard_locks::NumLock) == keyboard_locks::NumLock)
                result = result | key_modifier::NUM;
            return result;
        }

        keyboard_locks display_server::locks() const
        {
            keyboard_locks result = keyboard_locks::None;
            if ((iModifierState & LockMask) != 0u)
                result = result | keyboard_locks::CapsLock;
            if ((iModifierState & Mod2Mask) != 0u)
                result = result | keyboard_locks::NumLock;
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
            if (iPointerFocus != nullptr)
                apply_cursor(iPointerFocus->xwindow());
        }

        void display_server::capture_pointer(i_native_window& aWindow)
        {
            for (auto w : iWindows)
                if (static_cast<i_native_window*>(w) == &aWindow)
                {
                    ::XGrabPointer(iDisplay, w->xwindow(), XFalse, ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask,
                        GrabModeAsync, GrabModeAsync, XNone, XNone, XCurrentTime);
                    iCapturing = true;
                }
        }

        void display_server::release_pointer()
        {
            if (iCapturing)
            {
                ::XUngrabPointer(iDisplay, XCurrentTime);
                iCapturing = false;
            }
        }

        i_native_clipboard& display_server::clipboard()
        {
            return *iClipboard;
        }

        void display_server::beep()
        {
            ::XBell(iDisplay, 0);
        }

        color display_server::read_pixel(point const& aPosition) const
        {
            auto const image = ::XGetImage(iDisplay, iRoot, static_cast<int>(aPosition.x), static_cast<int>(aPosition.y), 1u, 1u, AllPlanes, ZPixmap);
            if (image == nullptr)
                return color{};
            auto const pixel = XGetPixel(image, 0, 0);
            auto const visual = DefaultVisual(iDisplay, iScreen);
            auto const component = [&](unsigned long aMask)
            {
                if (aMask == 0ul)
                    return std::uint8_t{};
                int shift = 0;
                while (((aMask >> shift) & 1ul) == 0ul)
                    ++shift;
                auto const maximum = aMask >> shift;
                return static_cast<std::uint8_t>(((pixel & aMask) >> shift) * 255ul / maximum);
            };
            color const result{ component(visual->red_mask), component(visual->green_mask), component(visual->blue_mask) };
            XDestroyImage(image);
            return result;
        }

        Display* display_server::display() const
        {
            return iDisplay;
        }

        int display_server::screen() const
        {
            return iScreen;
        }

        XID display_server::root() const
        {
            return iRoot;
        }

        XIM display_server::input_method() const
        {
            return iInputMethod;
        }

        Atom display_server::atom(atom_id aAtom) const
        {
            return iAtoms[static_cast<std::size_t>(aAtom)];
        }

        bool display_server::supported(atom_id aAtom) const
        {
            return iSupported.contains(atom(aAtom));
        }

        void display_server::send_wm_message(XID aWindow, atom_id aMessage, long aData0, long aData1, long aData2, long aData3, long aData4)
        {
            XEvent event = {};
            event.xclient.type = ClientMessage;
            event.xclient.window = aWindow;
            event.xclient.message_type = atom(aMessage);
            event.xclient.format = 32;
            event.xclient.data.l[0] = aData0;
            event.xclient.data.l[1] = aData1;
            event.xclient.data.l[2] = aData2;
            event.xclient.data.l[3] = aData3;
            event.xclient.data.l[4] = aData4;
            ::XSendEvent(iDisplay, iRoot, XFalse, SubstructureRedirectMask | SubstructureNotifyMask, &event);
        }

        unsigned long display_server::input_time() const
        {
            return iInputTime;
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
                iKeyboardFocus = nullptr;
        }

        window* display_server::keyboard_focus() const
        {
            return iKeyboardFocus;
        }

        void display_server::handle_event(XEvent& aEvent)
        {
            if (rr_event_base >= 0 && aEvent.type == rr_event_base + RRScreenChangeNotify)
            {
                ::XRRUpdateConfiguration(&aEvent);
                iOutputs = std::nullopt;
                return;
            }
            switch (aEvent.type)
            {
            case SelectionRequest:
                if (aEvent.xselectionrequest.owner == iHelperWindow)
                    iClipboard->selection_request(aEvent);
                return;
            case SelectionClear:
                if (aEvent.xselectionclear.window == iHelperWindow)
                    iClipboard->selection_clear();
                return;
            case MappingNotify:
                ::XRefreshKeyboardMapping(&aEvent.xmapping);
                if (service<i_keyboard>().is_enabled())
                    service<i_keyboard>().input_language_changed()();
                return;
            default:
                break;
            }
            auto const w = window_of(aEvent.xany.window);
            if (w == nullptr)
                return;
            switch (aEvent.type)
            {
            case XKeyPress:
            case XKeyRelease:
                key(aEvent, *w);
                break;
            case XButtonPress:
            case XButtonRelease:
                {
                    auto const& e = aEvent.xbutton;
                    bool const pressed = (aEvent.type == XButtonPress);
                    iInputTime = e.time;
                    update_modifier_state(e.state);
                    iPointerPosition = basic_point<int>{ e.x_root, e.y_root };
                    if (e.button >= Button4 && e.button <= 7u)
                    {
                        // (as Windows: positive is away from the user (wheel up) and to the right)
                        if (pressed)
                        {
                            if (e.button == Button4 || e.button == Button5)
                                w->pointer_wheel(mouse_wheel::Vertical, delta{ 0.0, e.button == Button4 ? linux::WHEEL_DELTA : -linux::WHEEL_DELTA });
                            else
                                w->pointer_wheel(mouse_wheel::Horizontal, delta{ e.button == 7u ? linux::WHEEL_DELTA : -linux::WHEEL_DELTA, 0.0 });
                        }
                        break;
                    }
                    auto const button = convert_button(e.button);
                    if (button == mouse_button::None)
                        break;
                    if (pressed)
                        iPointerButtons = iPointerButtons | button;
                    else
                        iPointerButtons = iPointerButtons & ~button;
                    if (!pressed && w->dragging())
                    {
                        w->end_drag();
                        break;
                    }
                    w->pointer_moved(point{ basic_point<int>{ e.x, e.y } });
                    w->pointer_button(button, pressed, 0u);
                }
                break;
            case MotionNotify:
                {
                    // (only the latest of the motion events queued)
                    while (::XCheckTypedWindowEvent(iDisplay, aEvent.xmotion.window, MotionNotify, &aEvent));
                    auto const& e = aEvent.xmotion;
                    iInputTime = e.time;
                    update_modifier_state(e.state);
                    iPointerPosition = basic_point<int>{ e.x_root, e.y_root };
                    if (w->dragging())
                        w->drag(iPointerPosition);
                    else
                        w->pointer_moved(point{ basic_point<int>{ e.x, e.y } });
                }
                break;
            case EnterNotify:
                {
                    auto const& e = aEvent.xcrossing;
                    iPointerPosition = basic_point<int>{ e.x_root, e.y_root };
                    iPointerFocus = w;
                    apply_cursor(w->xwindow());
                    w->pointer_entered(point{ basic_point<int>{ e.x, e.y } });
                }
                break;
            case LeaveNotify:
                {
                    auto const& e = aEvent.xcrossing;
                    // (the window manager has taken the pointer, e.g. to move the window, so the button's release
                    // will not be seen)
                    if (e.mode == NotifyGrab)
                        iPointerButtons = mouse_button::None;
                    if (e.mode == NotifyGrab && iCapturing)
                        break;
                    w->pointer_left();
                    if (iPointerFocus == w)
                        iPointerFocus = nullptr;
                }
                break;
            case XFocusIn:
                if (aEvent.xfocus.mode == NotifyNormal || aEvent.xfocus.mode == NotifyWhileGrabbed)
                {
                    if (aEvent.xfocus.detail == NotifyPointer || aEvent.xfocus.detail == NotifyInferior)
                        break;
                    iKeyboardFocus = w;
                    if (w->input_context() != nullptr)
                        ::XSetICFocus(w->input_context());
                    // (the keys already down, and the lock keys' states)
                    char keys[32] = {};
                    ::XQueryKeymap(iDisplay, keys);
                    iKeysPressed.clear();
                    for (std::uint32_t keyCode = 8u; keyCode < 256u; ++keyCode)
                        if ((keys[keyCode / 8u] & (1 << (keyCode % 8u))) != 0)
                            iKeysPressed.insert(keyCode - 8u);
                    XkbStateRec state = {};
                    ::XkbGetState(iDisplay, XkbUseCoreKbd, &state);
                    iModifierState = state.locked_mods;
                    w->keyboard_focus_gained();
                }
                break;
            case XFocusOut:
                if (aEvent.xfocus.mode == NotifyNormal || aEvent.xfocus.mode == NotifyWhileGrabbed)
                {
                    if (aEvent.xfocus.detail == NotifyPointer || aEvent.xfocus.detail == NotifyInferior)
                        break;
                    if (w->input_context() != nullptr)
                        ::XUnsetICFocus(w->input_context());
                    iKeysPressed.clear();
                    if (iKeyboardFocus == w)
                        iKeyboardFocus = nullptr;
                    w->keyboard_focus_lost();
                }
                break;
            case XExpose:
                {
                    auto const& e = aEvent.xexpose;
                    w->expose(rect{ point{ basic_point<int>{ e.x, e.y } }, size{ basic_size<int>{ e.width, e.height } } });
                }
                break;
            case ConfigureNotify:
                w->configure_notify(aEvent);
                break;
            case MapNotify:
                w->map_notify();
                break;
            case UnmapNotify:
                w->unmap_notify();
                break;
            case PropertyNotify:
                w->property_notify(aEvent.xproperty.atom);
                break;
            case ClientMessage:
                if (static_cast<Atom>(aEvent.xclient.message_type) == atom(atom_id::WM_PROTOCOLS))
                {
                    auto const protocol = static_cast<Atom>(aEvent.xclient.data.l[0]);
                    if (protocol == atom(atom_id::WM_DELETE_WINDOW))
                        w->delete_window();
                    else if (protocol == atom(atom_id::NET_WM_PING))
                    {
                        aEvent.xclient.window = iRoot;
                        ::XSendEvent(iDisplay, iRoot, XFalse, SubstructureNotifyMask | SubstructureRedirectMask, &aEvent);
                    }
                }
                break;
            default:
                break;
            }
        }

        void display_server::key(XEvent& aEvent, window& aWindow)
        {
            auto& e = aEvent.xkey;
            bool const pressed = (aEvent.type == XKeyPress);
            iInputTime = e.time;
            update_modifier_state(e.state);
            // (a key code of zero is the input method's: text it has composed)
            bool const composed = (e.keycode == 0u);
            std::uint32_t const keyCode = composed ? 0u : e.keycode - 8u;
            bool repeat = false;
            if (!composed)
            {
                if (pressed)
                    repeat = !iKeysPressed.insert(keyCode).second;
                else
                    iKeysPressed.erase(keyCode);
            }
            auto const target = keyboard_target() != nullptr ? keyboard_target() : &aWindow;
            if (!target->enabled())
                return;
            auto const scanCode = linux::evdev_to_scan_code(keyCode);
            auto const neogfxKeyCode = service<i_keyboard>().scan_code_to_key_code(scanCode);
            auto modifiers = this->modifiers();
            // as Windows: a key pressed with Alt (but not with Ctrl, as AltGr is) is a "system" key, as F10 is
            bool const systemKey = ((modifiers & key_modifier::ALT) != key_modifier::None && (modifiers & key_modifier::CTRL) == key_modifier::None) || keyCode == KEY_F10;
            if (systemKey)
                modifiers = modifiers | key_modifier::SYSTEM;
            switch (keyCode)
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
            if (pressed)
            {
                std::string text;
                KeySym keySymbol = 0;
                if (aWindow.input_context() != nullptr)
                {
                    char buffer[256] = {};
                    XStatus status = 0;
                    auto const length = ::Xutf8LookupString(aWindow.input_context(), &e, buffer, sizeof(buffer) - 1, &keySymbol, &status);
                    if ((status == XLookupChars || status == XLookupBoth) && length > 0)
                        text.assign(buffer, static_cast<std::size_t>(length));
                }
                else
                {
                    char buffer[32] = {};
                    ::XLookupString(&e, buffer, sizeof(buffer), &keySymbol, nullptr);
                    text = linux::key_symbol_to_utf8(static_cast<std::uint32_t>(keySymbol));
                }
                if (!composed)
                {
                    if (keyCode == KEY_INSERT && !repeat)
                        iInsertLock = !iInsertLock;
                    target->push_event(keyboard_event{
                        keyboard_event_type::KeyPressed, scanCode, neogfxKeyCode, modifiers,
                        static_cast<native_scan_code_e>(keyCode), static_cast<native_key_code_e>(keySymbol) });
                }
                if (!text.empty() && (text.size() > 1 || text[0] >= ' ' || std::isspace(text[0])))
                    target->push_event(keyboard_event{
                        systemKey ? keyboard_event_type::SysTextInput : keyboard_event_type::TextInput,
                        string{ text }, KeyCode_UNKNOWN, modifiers,
                        static_cast<native_scan_code_e>(keyCode), static_cast<native_key_code_e>(keySymbol) });
            }
            else if (!composed)
                target->push_event(keyboard_event{
                    keyboard_event_type::KeyReleased, scanCode, neogfxKeyCode, modifiers,
                    static_cast<native_scan_code_e>(keyCode), static_cast<native_key_code_e>(::XLookupKeysym(&e, 0)) });
            if (!target->is_non_client_capturing())
                service<i_window_manager>().update_mouse_cursor(target->surface_window().as_window());
        }

        window* display_server::keyboard_target() const
        {
            if (iKeyboardFocus == nullptr)
                return nullptr;
            // n.b. a popup (an override-redirect window) does not have the keyboard focus, so the keyboard's events go
            // to the window neoGFX has made active if it is a popup of the window that has the focus
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

        window* display_server::window_of(XID aWindow) const
        {
            for (auto w : iWindows)
                if (w->xwindow() == aWindow)
                    return w;
            return nullptr;
        }

        void display_server::update_outputs() const
        {
            if (iOutputs)
                return;
            iOutputs.emplace();
            // the DPI (and its scaling) and subpixel order are the desktop's (Xft) settings
            double dpi = 96.0;
            auto subpixelFormat = subpixel_format::RGBHorizontal;
            if (auto const resources = ::XResourceManagerString(iDisplay); resources != nullptr)
            {
                auto const database = ::XrmGetStringDatabase(resources);
                if (database != nullptr)
                {
                    char* type = nullptr;
                    XrmValue value = {};
                    if (::XrmGetResource(database, "Xft.dpi", "Xft.Dpi", &type, &value) && value.addr != nullptr)
                        dpi = std::max(48.0, std::atof(value.addr));
                    if (::XrmGetResource(database, "Xft.rgba", "Xft.Rgba", &type, &value) && value.addr != nullptr)
                    {
                        std::string const rgba{ value.addr };
                        if (rgba == "bgr")
                            subpixelFormat = subpixel_format::BGRHorizontal;
                        else if (rgba == "vrgb")
                            subpixelFormat = subpixel_format::RGBVertical;
                        else if (rgba == "vbgr")
                            subpixelFormat = subpixel_format::BGRVertical;
                        else if (rgba == "none")
                            subpixelFormat = subpixel_format::None;
                    }
                    ::XrmDestroyDatabase(database);
                }
            }
            // the work area is the desktop's (that of the first desktop, on which panels are), within each monitor
            std::optional<rect> workArea;
            {
                Atom type = XNone;
                int format = 0;
                unsigned long count = 0ul;
                unsigned long remaining = 0ul;
                unsigned char* data = nullptr;
                if (::XGetWindowProperty(iDisplay, iRoot, atom(atom_id::NET_WORKAREA), 0, 4, XFalse, XA_CARDINAL,
                    &type, &format, &count, &remaining, &data) == XSuccess && data != nullptr)
                {
                    if (count >= 4ul)
                    {
                        auto const values = reinterpret_cast<long const*>(data);
                        workArea = rect{ point{ basic_point<long>{ values[0], values[1] } }, size{ basic_size<long>{ values[2], values[3] } } };
                    }
                    ::XFree(data);
                }
            }
            auto const add = [&](rect const& aMonitor)
            {
                linux::output_info info;
                info.monitor = aMonitor;
                info.workArea = workArea ? workArea->intersection(aMonitor) : aMonitor;
                if (info.workArea.cx <= 0.0 || info.workArea.cy <= 0.0)
                    info.workArea = aMonitor;
                info.dpi = size{ dpi, dpi };
                info.scale = static_cast<std::uint32_t>(std::max(1.0, std::round(dpi / 96.0)));
                info.subpixelFormat = subpixelFormat;
                iOutputs->push_back(info);
            };
            int monitorCount = 0;
            auto const monitors = rr_event_base >= 0 ? ::XRRGetMonitors(iDisplay, iRoot, XTrue, &monitorCount) : nullptr;
            if (monitors != nullptr)
            {
                // (the primary monitor first, as on Windows)
                for (int pass = 0; pass < 2; ++pass)
                    for (int m = 0; m < monitorCount; ++m)
                        if ((monitors[m].primary != 0) == (pass == 0))
                            add(rect{ point{ basic_point<int>{ monitors[m].x, monitors[m].y } }, size{ basic_size<int>{ monitors[m].width, monitors[m].height } } });
                ::XRRFreeMonitors(monitors);
            }
            if (iOutputs->empty())
                add(rect{ point{}, size{ basic_size<int>{ DisplayWidth(iDisplay, iScreen), DisplayHeight(iDisplay, iScreen) } } });
        }

        void display_server::apply_cursor(XID aWindow)
        {
            auto existing = iCursors.find(iCursor);
            if (existing == iCursors.end())
            {
                XID cursor = XNone;
                for (auto name : linux::cursor_names(iCursor))
                    if ((cursor = ::XcursorLibraryLoadCursor(iDisplay, name)) != XNone)
                        break;
                if (cursor == XNone)
                    cursor = ::XCreateFontCursor(iDisplay, XC_left_ptr);
                existing = iCursors.emplace(iCursor, cursor).first;
            }
            ::XDefineCursor(iDisplay, aWindow, existing->second);
        }

        void display_server::update_modifier_state(unsigned int aState)
        {
            iModifierState = aState;
        }
    }
}
