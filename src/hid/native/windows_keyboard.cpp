// keyboard.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include "windows_keyboard.hpp"

namespace neogfx
{
    namespace native::windows
    {
        scan_code_e keyboard::scan_code_from_message(LPARAM aLParam, WPARAM aWParam)
        {
            auto const rawScanCode = static_cast<scan_code_e>(aLParam >> 16) & 0xFF;
            bool const extended = ((aLParam & (1 << 24)) != 0);
            auto scanCode = static_cast<scan_code_e>(MapVirtualKey(static_cast<UINT>(aWParam), MAPVK_VK_TO_VSC));
            if (scanCode == ScanCode_UNKNOWN && rawScanCode <= 127)
            {
                static constexpr scan_code_e sWindowsScanCodes[] =
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

        static const int32_t sDefaultKeyMap[ScanCodeCount] = {
            0, 0, 0, 0,
            'a',
            'b',
            'c',
            'd',
            'e',
            'f',
            'g',
            'h',
            'i',
            'j',
            'k',
            'l',
            'm',
            'n',
            'o',
            'p',
            'q',
            'r',
            's',
            't',
            'u',
            'v',
            'w',
            'x',
            'y',
            'z',
            '1',
            '2',
            '3',
            '4',
            '5',
            '6',
            '7',
            '8',
            '9',
            '0',
            KeyCode_RETURN,
            KeyCode_ESCAPE,
            KeyCode_BACKSPACE,
            KeyCode_TAB,
            KeyCode_SPACE,
            '-',
            '=',
            '[',
            ']',
            '\\',
            '#',
            ';',
            '\'',
            '`',
            ',',
            '.',
            '/',
            KeyCode_CAPSLOCK,
            KeyCode_F1,
            KeyCode_F2,
            KeyCode_F3,
            KeyCode_F4,
            KeyCode_F5,
            KeyCode_F6,
            KeyCode_F7,
            KeyCode_F8,
            KeyCode_F9,
            KeyCode_F10,
            KeyCode_F11,
            KeyCode_F12,
            KeyCode_PRINTSCREEN,
            KeyCode_SCROLLLOCK,
            KeyCode_PAUSE,
            KeyCode_INSERT,
            KeyCode_HOME,
            KeyCode_PAGEUP,
            KeyCode_DELETE,
            KeyCode_END,
            KeyCode_PAGEDOWN,
            KeyCode_RIGHT,
            KeyCode_LEFT,
            KeyCode_DOWN,
            KeyCode_UP,
            KeyCode_NUMLOCKCLEAR,
            KeyCode_KP_DIVIDE,
            KeyCode_KP_MULTIPLY,
            KeyCode_KP_MINUS,
            KeyCode_KP_PLUS,
            KeyCode_KP_ENTER,
            KeyCode_KP_1,
            KeyCode_KP_2,
            KeyCode_KP_3,
            KeyCode_KP_4,
            KeyCode_KP_5,
            KeyCode_KP_6,
            KeyCode_KP_7,
            KeyCode_KP_8,
            KeyCode_KP_9,
            KeyCode_KP_0,
            KeyCode_KP_PERIOD,
            0,
            KeyCode_APPLICATION,
            KeyCode_POWER,
            KeyCode_KP_EQUALS,
            KeyCode_F13,
            KeyCode_F14,
            KeyCode_F15,
            KeyCode_F16,
            KeyCode_F17,
            KeyCode_F18,
            KeyCode_F19,
            KeyCode_F20,
            KeyCode_F21,
            KeyCode_F22,
            KeyCode_F23,
            KeyCode_F24,
            KeyCode_EXECUTE,
            KeyCode_HELP,
            KeyCode_MENU,
            KeyCode_SELECT,
            KeyCode_STOP,
            KeyCode_AGAIN,
            KeyCode_UNDO,
            KeyCode_CUT,
            KeyCode_COPY,
            KeyCode_PASTE,
            KeyCode_FIND,
            KeyCode_MUTE,
            KeyCode_VOLUMEUP,
            KeyCode_VOLUMEDOWN,
            0, 0, 0,
            KeyCode_KP_COMMA,
            KeyCode_KP_EQUALSAS400,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            KeyCode_ALTERASE,
            KeyCode_SYSREQ,
            KeyCode_CANCEL,
            KeyCode_CLEAR,
            KeyCode_PRIOR,
            KeyCode_RETURN2,
            KeyCode_SEPARATOR,
            KeyCode_OUT,
            KeyCode_OPER,
            KeyCode_CLEARAGAIN,
            KeyCode_CRSEL,
            KeyCode_EXSEL,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            KeyCode_KP_00,
            KeyCode_KP_000,
            KeyCode_THOUSANDSSEPARATOR,
            KeyCode_DECIMALSEPARATOR,
            KeyCode_CURRENCYUNIT,
            KeyCode_CURRENCYSUBUNIT,
            KeyCode_KP_LEFTPAREN,
            KeyCode_KP_RIGHTPAREN,
            KeyCode_KP_LEFTBRACE,
            KeyCode_KP_RIGHTBRACE,
            KeyCode_KP_TAB,
            KeyCode_KP_BACKSPACE,
            KeyCode_KP_A,
            KeyCode_KP_B,
            KeyCode_KP_C,
            KeyCode_KP_D,
            KeyCode_KP_E,
            KeyCode_KP_F,
            KeyCode_KP_XOR,
            KeyCode_KP_POWER,
            KeyCode_KP_PERCENT,
            KeyCode_KP_LESS,
            KeyCode_KP_GREATER,
            KeyCode_KP_AMPERSAND,
            KeyCode_KP_DBLAMPERSAND,
            KeyCode_KP_VERTICALBAR,
            KeyCode_KP_DBLVERTICALBAR,
            KeyCode_KP_COLON,
            KeyCode_KP_HASH,
            KeyCode_KP_SPACE,
            KeyCode_KP_AT,
            KeyCode_KP_EXCLAM,
            KeyCode_KP_MEMSTORE,
            KeyCode_KP_MEMRECALL,
            KeyCode_KP_MEMCLEAR,
            KeyCode_KP_MEMADD,
            KeyCode_KP_MEMSUBTRACT,
            KeyCode_KP_MEMMULTIPLY,
            KeyCode_KP_MEMDIVIDE,
            KeyCode_KP_PLUSMINUS,
            KeyCode_KP_CLEAR,
            KeyCode_KP_CLEARENTRY,
            KeyCode_KP_BINARY,
            KeyCode_KP_OCTAL,
            KeyCode_KP_DECIMAL,
            KeyCode_KP_HEXADECIMAL,
            0, 0,
            KeyCode_LCTRL,
            KeyCode_LSHIFT,
            KeyCode_LALT,
            KeyCode_LGUI,
            KeyCode_RCTRL,
            KeyCode_RSHIFT,
            KeyCode_RALT,
            KeyCode_RGUI,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            KeyCode_MODE,
            KeyCode_AUDIONEXT,
            KeyCode_AUDIOPREV,
            KeyCode_AUDIOSTOP,
            KeyCode_AUDIOPLAY,
            KeyCode_AUDIOMUTE,
            KeyCode_MEDIASELECT,
            KeyCode_WWW,
            KeyCode_MAIL,
            KeyCode_CALCULATOR,
            KeyCode_COMPUTER,
            KeyCode_AC_SEARCH,
            KeyCode_AC_HOME,
            KeyCode_AC_BACK,
            KeyCode_AC_FORWARD,
            KeyCode_AC_STOP,
            KeyCode_AC_REFRESH,
            KeyCode_AC_BOOKMARKS,
            KeyCode_BRIGHTNESSDOWN,
            KeyCode_BRIGHTNESSUP,
            KeyCode_DISPLAYSWITCH,
            KeyCode_KBDILLUMTOGGLE,
            KeyCode_KBDILLUMDOWN,
            KeyCode_KBDILLUMUP,
            KeyCode_EJECT,
            KeyCode_SLEEP,
            KeyCode_APP1,
            KeyCode_APP2,
            KeyCode_AUDIOREWIND,
            KeyCode_AUDIOFASTFORWARD,
        };

        key_code_e keyboard::scan_code_to_key_code(scan_code_e aScanCode)
        {
            return static_cast<key_code_e>(sDefaultKeyMap[aScanCode]);
        }

        keyboard::keyboard()
        {
            BYTE kbdState[256];
            GetKeyboardState(kbdState);
            kbdState[VK_INSERT] |= 0x01;
            SetKeyboardState(kbdState);
        }

        bool keyboard::is_key_pressed(scan_code_e aScanCode) const
        {
            BYTE kbdState[256];
            GetKeyboardState(kbdState);
            return kbdState[MapVirtualKey(aScanCode, MAPVK_VSC_TO_VK_EX)];
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

        key_modifiers_e keyboard::modifiers()
        {
            key_modifiers_e modifiers = KeyModifier_NONE;
            if (GetKeyState(VK_SHIFT) >> 1)
                modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_SHIFT);
            if (GetKeyState(VK_CONTROL) >> 1)
                modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_CTRL);
            if (GetKeyState(VK_MENU) >> 1)
                modifiers = static_cast<key_modifiers_e>(modifiers | KeyModifier_ALT);
            return modifiers;
        }
    }
}