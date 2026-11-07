// x11_display_server.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

#include <set>

#include "i_native_clipboard.hpp"
#include "linux_display_server.hpp"

// (Xlib's types, without Xlib's macros: see x11.hpp)
typedef struct _XDisplay Display;
typedef struct _XIM* XIM;
typedef struct _XIC* XIC;
typedef union _XEvent XEvent;
typedef unsigned long XID;
typedef unsigned long Atom;

namespace neogfx
{
    namespace native::x11
    {
        class window;

        class display_server : public linux::display_server
        {
        public:
            enum class atom_id : std::uint32_t
            {
                WM_PROTOCOLS,
                WM_DELETE_WINDOW,
                NET_WM_PING,
                NET_WM_PID,
                NET_WM_NAME,
                NET_WM_STATE,
                NET_WM_STATE_MAXIMIZED_VERT,
                NET_WM_STATE_MAXIMIZED_HORZ,
                NET_WM_STATE_FULLSCREEN,
                NET_WM_STATE_HIDDEN,
                NET_WM_STATE_DEMANDS_ATTENTION,
                NET_WM_STATE_SKIP_TASKBAR,
                NET_WM_STATE_ABOVE,
                NET_WM_WINDOW_TYPE,
                NET_WM_WINDOW_TYPE_NORMAL,
                NET_WM_WINDOW_TYPE_DIALOG,
                NET_WM_WINDOW_TYPE_UTILITY,
                NET_WM_WINDOW_TYPE_SPLASH,
                NET_WM_WINDOW_TYPE_POPUP_MENU,
                NET_WM_WINDOW_TYPE_DROPDOWN_MENU,
                NET_WM_WINDOW_TYPE_TOOLTIP,
                NET_WM_WINDOW_OPACITY,
                NET_WM_MOVERESIZE,
                NET_ACTIVE_WINDOW,
                NET_SUPPORTED,
                NET_WORKAREA,
                MOTIF_WM_HINTS,
                UTF8_STRING,
                CLIPBOARD,
                TARGETS,
                TEXT,
                NEOGFX_SELECTION,
                COUNT
            };
        private:
            class native_clipboard : public i_native_clipboard
            {
            public:
                native_clipboard(display_server& aDisplayServer);
            public:
                bool has_text() const final;
                i_string const& text() const final;
                void set_text(i_string const& aText) final;
                bool has_image() const final;
                neogfx::image image() const final;
                void set_image(const neogfx::image& aImage) final;
            public:
                void selection_request(XEvent const& aEvent);
                void selection_clear();
            private:
                display_server& iDisplayServer;
                std::optional<std::string> iOwnText;
                mutable string iText;
            };
        public:
            display_server(char const* aDisplayName = nullptr);
            ~display_server();
        public:
            neogfx::windowing_system windowing_system() const final;
            void* native_display() const final;
            bool dispatch_events() final;
            void flush() final;
        public:
            std::uint32_t output_count() const final;
            linux::output_info output(std::uint32_t aIndex) const final;
        public:
            bool is_key_pressed(std::uint32_t aKeyCode) const final;
            std::uint32_t key_symbol(std::uint32_t aKeyCode) const final;
            key_modifier modifiers() const final;
            keyboard_locks locks() const final;
        public:
            point pointer_position() const final;
            mouse_button pointer_buttons() const final;
            i_native_window* window_under_pointer() const final;
            void set_cursor(mouse_system_cursor aCursor) final;
            void capture_pointer(i_native_window& aWindow) final;
            void release_pointer() final;
        public:
            i_native_clipboard& clipboard() final;
            void beep() final;
            color read_pixel(point const& aPosition) const final;
        public:
            Display* display() const;
            int screen() const;
            XID root() const;
            XIM input_method() const;
            Atom atom(atom_id aAtom) const;
            bool supported(atom_id aAtom) const;
            void send_wm_message(XID aWindow, atom_id aMessage, long aData0, long aData1 = 0, long aData2 = 0, long aData3 = 0, long aData4 = 0);
            // the latest (X server) time of an input event
            unsigned long input_time() const;
        public:
            void add_window(window& aWindow);
            void remove_window(window& aWindow);
            window* keyboard_focus() const;
        private:
            void handle_event(XEvent& aEvent);
            void key(XEvent& aEvent, window& aWindow);
            window* keyboard_target() const;
            window* window_of(XID aWindow) const;
            void update_outputs() const;
            void apply_cursor(XID aWindow);
            void update_modifier_state(unsigned int aState);
        private:
            Display* iDisplay = nullptr;
            int iScreen = 0;
            XID iRoot = 0;
            XID iHelperWindow = 0;
            XIM iInputMethod = nullptr;
            std::array<Atom, static_cast<std::size_t>(atom_id::COUNT)> iAtoms = {};
            std::set<Atom> iSupported;
            std::vector<window*> iWindows;
            mutable std::optional<std::vector<linux::output_info>> iOutputs;
            // keyboard
            window* iKeyboardFocus = nullptr;
            std::set<std::uint32_t> iKeysPressed;
            unsigned int iModifierState = 0u;
            bool iInsertLock = true; // (as Windows, where neoGFX turns it on at startup)
            // pointer
            window* iPointerFocus = nullptr;
            point iPointerPosition;
            mouse_button iPointerButtons = mouse_button::None;
            unsigned long iInputTime = 0ul;
            bool iCapturing = false;
            mouse_system_cursor iCursor = mouse_system_cursor::Arrow;
            std::map<mouse_system_cursor, XID> iCursors;
            // clipboard
            std::unique_ptr<native_clipboard> iClipboard;
        };
    }
}
