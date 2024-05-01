// keyboard.cpp
// Parts of this file based on Simple DirectMedia Layer Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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
/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <neogfx/neogfx.hpp>

#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/hid/i_native_surface.hpp>
#include <neogfx/gui/window/i_native_window.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include "windows_keyboard.hpp"

namespace neogfx
{
    namespace native::windows
    {
        scan_code_e vk_to_scan_code(WPARAM vkey)
        {
            switch (vkey) 
            {
            case VK_LEFT: return ScanCode_LEFT;
            case VK_UP: return ScanCode_UP;
            case VK_RIGHT: return ScanCode_RIGHT;
            case VK_DOWN: return ScanCode_DOWN;

            case VK_MODECHANGE: return ScanCode_MODE;
            case VK_SELECT: return ScanCode_SELECT;
            case VK_EXECUTE: return ScanCode_EXECUTE;
            case VK_HELP: return ScanCode_HELP;
            case VK_PAUSE: return ScanCode_PAUSE;
            case VK_NUMLOCK: return ScanCode_NUMLOCKCLEAR;

            case VK_F13: return ScanCode_F13;
            case VK_F14: return ScanCode_F14;
            case VK_F15: return ScanCode_F15;
            case VK_F16: return ScanCode_F16;
            case VK_F17: return ScanCode_F17;
            case VK_F18: return ScanCode_F18;
            case VK_F19: return ScanCode_F19;
            case VK_F20: return ScanCode_F20;
            case VK_F21: return ScanCode_F21;
            case VK_F22: return ScanCode_F22;
            case VK_F23: return ScanCode_F23;
            case VK_F24: return ScanCode_F24;

            case VK_OEM_NEC_EQUAL: return ScanCode_KP_EQUALS;
            case VK_BROWSER_BACK: return ScanCode_AC_BACK;
            case VK_BROWSER_FORWARD: return ScanCode_AC_FORWARD;
            case VK_BROWSER_REFRESH: return ScanCode_AC_REFRESH;
            case VK_BROWSER_STOP: return ScanCode_AC_STOP;
            case VK_BROWSER_SEARCH: return ScanCode_AC_SEARCH;
            case VK_BROWSER_FAVORITES: return ScanCode_AC_BOOKMARKS;
            case VK_BROWSER_HOME: return ScanCode_AC_HOME;
            case VK_VOLUME_MUTE: return ScanCode_AUDIOMUTE;
            case VK_VOLUME_DOWN: return ScanCode_VOLUMEDOWN;
            case VK_VOLUME_UP: return ScanCode_VOLUMEUP;

            case VK_MEDIA_NEXT_TRACK: return ScanCode_AUDIONEXT;
            case VK_MEDIA_PREV_TRACK: return ScanCode_AUDIOPREV;
            case VK_MEDIA_STOP: return ScanCode_AUDIOSTOP;
            case VK_MEDIA_PLAY_PAUSE: return ScanCode_AUDIOPLAY;
            case VK_LAUNCH_MAIL: return ScanCode_MAIL;
            case VK_LAUNCH_MEDIA_SELECT: return ScanCode_MEDIASELECT;

            case VK_OEM_102: return ScanCode_NONUSBACKSLASH;

            case VK_ATTN: return ScanCode_SYSREQ;
            case VK_CRSEL: return ScanCode_CRSEL;
            case VK_EXSEL: return ScanCode_EXSEL;
            case VK_OEM_CLEAR: return ScanCode_CLEAR;

            case VK_LAUNCH_APP1: return ScanCode_APP1;
            case VK_LAUNCH_APP2: return ScanCode_APP2;

            default: return ScanCode_UNKNOWN;
            }
        }

        static constexpr std::array<scan_code_e, 128> sWindowsScanCodes =
        {
            ScanCode_UNKNOWN,        ScanCode_ESCAPE,         ScanCode_1,           ScanCode_2,              ScanCode_3,       ScanCode_4,              ScanCode_5,              ScanCode_6,
            ScanCode_7,              ScanCode_8,              ScanCode_9,           ScanCode_0,              ScanCode_MINUS,   ScanCode_EQUALS,         ScanCode_BACKSPACE,      ScanCode_TAB,
            ScanCode_Q,              ScanCode_W,              ScanCode_E,           ScanCode_R,              ScanCode_T,       ScanCode_Y,              ScanCode_U,              ScanCode_I,
            ScanCode_O,              ScanCode_P,              ScanCode_LEFTBRACKET, ScanCode_RIGHTBRACKET,   ScanCode_RETURN,  ScanCode_LCTRL,          ScanCode_A,              ScanCode_S,
            ScanCode_D,              ScanCode_F,              ScanCode_G,           ScanCode_H,              ScanCode_J,       ScanCode_K,              ScanCode_L,              ScanCode_SEMICOLON,
            ScanCode_APOSTROPHE,     ScanCode_GRAVE,          ScanCode_LSHIFT,      ScanCode_BACKSLASH,      ScanCode_Z,       ScanCode_X,              ScanCode_C,              ScanCode_V,
            ScanCode_B,              ScanCode_N,              ScanCode_M,           ScanCode_COMMA,          ScanCode_PERIOD,  ScanCode_SLASH,          ScanCode_RSHIFT,         ScanCode_PRINTSCREEN,
            ScanCode_LALT,           ScanCode_SPACE,          ScanCode_CAPSLOCK,    ScanCode_F1,             ScanCode_F2,      ScanCode_F3,             ScanCode_F4,             ScanCode_F5,
            ScanCode_F6,             ScanCode_F7,             ScanCode_F8,          ScanCode_F9,             ScanCode_F10,     ScanCode_NUMLOCKCLEAR,   ScanCode_SCROLLLOCK,     ScanCode_HOME,
            ScanCode_UP,             ScanCode_PAGEUP,         ScanCode_KP_MINUS,    ScanCode_LEFT,           ScanCode_KP_5,    ScanCode_RIGHT,          ScanCode_KP_PLUS,        ScanCode_END,
            ScanCode_DOWN,           ScanCode_PAGEDOWN,       ScanCode_INSERT,      ScanCode_DELETE,         ScanCode_UNKNOWN, ScanCode_UNKNOWN,        ScanCode_NONUSBACKSLASH, ScanCode_F11,
            ScanCode_F12,            ScanCode_PAUSE,          ScanCode_UNKNOWN,     ScanCode_LGUI,           ScanCode_RGUI,    ScanCode_APPLICATION,    ScanCode_UNKNOWN,        ScanCode_UNKNOWN,
            ScanCode_UNKNOWN,        ScanCode_UNKNOWN,        ScanCode_UNKNOWN,     ScanCode_UNKNOWN,        ScanCode_F13,     ScanCode_F14,            ScanCode_F15,            ScanCode_F16,
            ScanCode_F17,            ScanCode_F18,            ScanCode_F19,         ScanCode_UNKNOWN,        ScanCode_UNKNOWN, ScanCode_UNKNOWN,        ScanCode_UNKNOWN,        ScanCode_UNKNOWN,
            ScanCode_INTERNATIONAL2, ScanCode_UNKNOWN,        ScanCode_UNKNOWN,     ScanCode_INTERNATIONAL1, ScanCode_UNKNOWN, ScanCode_UNKNOWN,        ScanCode_UNKNOWN,        ScanCode_UNKNOWN,
            ScanCode_UNKNOWN,        ScanCode_INTERNATIONAL4, ScanCode_UNKNOWN,     ScanCode_INTERNATIONAL5, ScanCode_UNKNOWN, ScanCode_INTERNATIONAL3, ScanCode_UNKNOWN,        ScanCode_UNKNOWN
        };

        scan_code_e keyboard::scan_code_from_message(LPARAM aLParam, WPARAM aWParam)
        {
            auto const rawScanCode = (aLParam >> 16) & 0xFF;
            bool const extended = ((aLParam & (1 << 24)) != 0);
            auto scanCode = vk_to_scan_code(aWParam);
            if (scanCode == ScanCode_UNKNOWN && rawScanCode <= 127)
            {
                scanCode = sWindowsScanCodes[rawScanCode];
                if (extended)
                {
                    switch (scanCode)
                    {
                    case ScanCode_RETURN:
                        scanCode = ScanCode_KP_ENTER;
                        break;
                    case ScanCode_LALT:
                        scanCode = ScanCode_RALT;
                        break;
                    case ScanCode_LCTRL:
                        scanCode = ScanCode_RCTRL;
                        break;
                    case ScanCode_SLASH:
                        scanCode = ScanCode_KP_DIVIDE;
                        break;
                    case ScanCode_CAPSLOCK:
                        scanCode = ScanCode_KP_PLUS;
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    switch (scanCode)
                    {
                    case ScanCode_HOME:
                        scanCode = ScanCode_KP_7;
                        break;
                    case ScanCode_UP:
                        scanCode = ScanCode_KP_8;
                        break;
                    case ScanCode_PAGEUP:
                        scanCode = ScanCode_KP_9;
                        break;
                    case ScanCode_LEFT:
                        scanCode = ScanCode_KP_4;
                        break;
                    case ScanCode_RIGHT:
                        scanCode = ScanCode_KP_6;
                        break;
                    case ScanCode_END:
                        scanCode = ScanCode_KP_1;
                        break;
                    case ScanCode_DOWN:
                        scanCode = ScanCode_KP_2;
                        break;
                    case ScanCode_PAGEDOWN:
                        scanCode = ScanCode_KP_3;
                        break;
                    case ScanCode_INSERT:
                        scanCode = ScanCode_KP_0;
                        break;
                    case ScanCode_DELETE:
                        scanCode = ScanCode_KP_PERIOD;
                        break;
                    case ScanCode_PRINTSCREEN:
                        scanCode = ScanCode_KP_MULTIPLY;
                        break;
                    default:
                        break;
                    }
                }
            }
            return scanCode;
        }

        keyboard_layout::keyboard_layout(i_keyboard& aKeyboard) : 
            iKeyboard{ aKeyboard }
        {
            iKeyboard.input_language_changed([&]() { input_language_changed(); });

            input_language_changed();

            if (has_ime())
                open_ime();
        }

        keyboard_layout::~keyboard_layout()
        {
        }

        bool keyboard_layout::has_ime() const
        {
            return ImmIsIME(iLayout);
        }

        bool keyboard_layout::ime_open() const
        {
            return iOpen;
        }

        bool keyboard_layout::ime_active() const
        {
            return iContext;
        }

        bool keyboard_layout::ime_active(i_widget const& aInputWidget) const
        {
            return iContext && iInputWidget == &aInputWidget;
        }

        i_widget const& keyboard_layout::input_widget() const
        {
            if (!ime_active())
                throw ime_not_active();
            return *iInputWidget;
        }

        point const& keyboard_layout::position() const
        {
            if (!ime_active())
                throw ime_not_active();
            return iPosition.value();
        }

        void keyboard_layout::open_ime()
        {
            iOpen = true;
        }

        void keyboard_layout::close_ime()
        {
            if (ime_active())
                deactivate_ime(input_widget());
            iOpen = false;
        }

        void keyboard_layout::activate_ime(i_widget const& aInputWidget, optional_point const& aPosition)
        {
            if (!ime_open())
                return;

            auto inputWidget = &aInputWidget;
            auto position = inputWidget->to_window_coordinates(aPosition ?
                aPosition.value() : inputWidget->client_rect(false).bottom_left());

            if (ime_active())
            {
                if (&input_widget() == &aInputWidget)
                {
                    update_ime_position(position);
                    return;
                }
                else
                    deactivate_ime(input_widget());
            }

            auto surface = static_cast<HWND>(inputWidget->surface().as_surface_window().native_window().native_handle());
            auto context = ImmGetContext(surface);

            if (!context ||
                !set_ime_input_area(context, inputWidget->non_client_rect()) ||
                !set_ime_position(context, inputWidget->non_client_rect(), position) ||
                !ImmSetOpenStatus(context, TRUE))
            {
                if (context)
                {
                    ImmSetOpenStatus(context, FALSE);
                    ImmReleaseContext(surface, context);
                }
                throw ime_activation_failure();
            }

            iContext = context;
            iInputWidget = inputWidget;
            iPosition = position;
            iSurface = surface;
            iSink = inputWidget->destroyed([&, inputWidget]()
                {
                    if (ime_active() && &input_widget() == inputWidget)
                        deactivate_ime(input_widget());
                });
        }

        void keyboard_layout::update_ime_position(point const& aPosition)
        {
            if (!ime_active())
                return;
            iPosition = aPosition;
            set_ime_position(iContext, iInputWidget->non_client_rect(), iInputWidget->to_window_coordinates(aPosition));
        }

        void keyboard_layout::deactivate_ime(i_widget const& aInputWidget)
        {
            if (!ime_active())
                return;
            if (&aInputWidget != iInputWidget)
                return;
            ImmNotifyIME(iContext, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
            ImmSetCompositionStringW(iContext, SCS_SETSTR, (LPVOID)L"", sizeof(wchar_t), (LPVOID)L"", sizeof(wchar_t));
            ImmNotifyIME(iContext, NI_CLOSECANDIDATE, 0, 0);
            ImmSetOpenStatus(iContext, FALSE);
            ImmReleaseContext(iSurface, iContext);
            iContext = nullptr;
            iSurface = nullptr;
            iInputWidget = nullptr;
            iPosition = std::nullopt;
        }

        void keyboard_layout::input_language_changed()
        {
            iLayout = GetKeyboardLayout(0);
            if (!has_ime() && ime_active())
                deactivate_ime(input_widget());
            if (ime_active())
            {
                auto inputWidget = iInputWidget;
                auto position = iPosition;
                deactivate_ime(input_widget());
                activate_ime(*inputWidget, position);
            }
        }

        bool keyboard_layout::set_ime_input_area(HIMC aContext, rect const& aArea)
        {
            basic_rect<LONG> wr = aArea;
            COMPOSITIONFORM cof = {};
            cof.dwStyle = CFS_RECT;
            cof.ptCurrentPos.x = wr.bottom_left().x;
            cof.ptCurrentPos.y = wr.bottom_left().y;
            cof.rcArea.left = wr.x;
            cof.rcArea.right = wr.x + wr.width();
            cof.rcArea.top = wr.y;
            cof.rcArea.bottom = wr.y + wr.height();
            return ImmSetCompositionWindow(aContext, &cof);
        }

        bool keyboard_layout::set_ime_position(HIMC aContext, rect const& aArea, point const& aPosition)
        {
            basic_rect<LONG> wr = aArea;
            basic_point<LONG> cp = aPosition;
            CANDIDATEFORM caf = {};
            caf.dwIndex = 0;
            caf.dwStyle = CFS_EXCLUDE;
            caf.ptCurrentPos.x = cp.x;
            caf.ptCurrentPos.y = cp.y;
            caf.rcArea.left = wr.x;
            caf.rcArea.right = wr.x + wr.width();
            caf.rcArea.top = wr.y;
            caf.rcArea.bottom = wr.y + wr.height();
            return ImmSetCandidateWindow(aContext, &caf);
        }

        keyboard::keyboard() :
            iKeymap{},
            iLayout{ new keyboard_layout{ *this } }
        {
            input_language_changed([&]() { update_keymap(); });

            update_keymap();

            BYTE kbdState[256];
            GetKeyboardState(kbdState);
            kbdState[VK_INSERT] |= 0x01;
            SetKeyboardState(kbdState);
        }

        i_keyboard_layout& keyboard::layout() const
        {
            return *iLayout;
        }

        bool keyboard::is_key_pressed(scan_code_e aScanCode) const
        {
            return ::GetAsyncKeyState(iKeymap[aScanCode].vk) >> 1;
        }

        keyboard_locks keyboard::locks() const
        {
            keyboard_locks result = keyboard_locks::None;
            if ((GetKeyState(VK_CAPITAL) & 0x0001) == 0x0001)
                result = result | keyboard_locks::CapsLock;
            if ((GetKeyState(VK_NUMLOCK) & 0x0001) == 0x0001)
                result = result | keyboard_locks::NumLock;
            if ((GetKeyState(VK_SCROLL) & 0x0001) == 0x0001)
                result = result | keyboard_locks::ScrollLock;
            if ((GetKeyState(VK_INSERT) & 0x0001) == 0x0001)
                result = result | keyboard_locks::InsertLock;
            return result;
        }

        // TODO: modifiers for current keyboard event

        key_modifiers_e keyboard::modifiers() const
        {
            key_modifiers_e modifiers = KeyModifier_NONE;
            if (GetAsyncKeyState(VK_LSHIFT) >> 1)
                modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_LSHIFT);
            if (GetAsyncKeyState(VK_RSHIFT) >> 1)
                modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_RSHIFT);
            if (GetAsyncKeyState(VK_LCONTROL) >> 1)
                modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_LCTRL);
            if (GetAsyncKeyState(VK_RCONTROL) >> 1)
                modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_RCTRL);
            if (GetAsyncKeyState(VK_LMENU) >> 1)
                modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_LALT);
            if (GetAsyncKeyState(VK_RMENU) >> 1)
                modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_RALT);
            if (GetAsyncKeyState(VK_LWIN) >> 1)
                modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_LGUI);
            if (GetAsyncKeyState(VK_RWIN) >> 1)
                modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_RGUI);
            if ((locks() & keyboard_locks::CapsLock) == keyboard_locks::CapsLock)
                modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_CAPS);
            if ((locks() & keyboard_locks::NumLock) == keyboard_locks::NumLock)
                modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_NUM);
            if ((locks() & keyboard_locks::ScrollLock) == keyboard_locks::ScrollLock)
                modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_SCROLL);
            return modifiers;
        }

        key_code_e keyboard::scan_code_to_key_code(scan_code_e aScanCode) const
        {
            return iKeymap[aScanCode].keyCode;
        }

        void keyboard::update_keymap()
        {
            keymap_t keymap = {};

            for (std::uint32_t i = 0u; i < sWindowsScanCodes.size(); ++i) 
            {
                auto const scanCode = sWindowsScanCodes[i];
                if (scanCode == ScanCode_UNKNOWN)
                    continue;

                auto vk = MapVirtualKey(i, MAPVK_VSC_TO_VK);
                keymap[scanCode].vk = vk;
                if (vk)
                {
                    auto ch = (MapVirtualKey(vk, MAPVK_VK_TO_CHAR) & 0x7FFF);
                    if (ch)
                    {
                        if (ch >= 'A' && ch <= 'Z')
                            keymap[scanCode].keyCode = static_cast<key_code_e>(KeyCode_a + (ch - 'A'));
                        else
                            keymap[scanCode].keyCode = static_cast<key_code_e>(ch);
                    }
                    else
                        keymap[scanCode].keyCode = static_cast<key_code_e>(NEOGFX_SCANCODE_TO_KEYCODE(scanCode));
                }
                else
                    keymap[scanCode].keyCode = KeyCode_UNKNOWN;
            }

            set_keymap(keymap);
        }

        void keyboard::set_keymap(const keymap_t& aKeymap)
        {
            iKeymap = aKeymap;
        }
    }
}