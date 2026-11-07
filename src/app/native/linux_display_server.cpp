// linux_display_server.cpp
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

#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon.h>

#include "linux_display_server.hpp"

namespace neogfx
{
    namespace native::linux
    {
        scan_code_e evdev_to_scan_code(std::uint32_t aKeyCode)
        {
            switch (aKeyCode)
            {
            case KEY_ESC: return ScanCode_ESCAPE;
            case KEY_1: return ScanCode_1;
            case KEY_2: return ScanCode_2;
            case KEY_3: return ScanCode_3;
            case KEY_4: return ScanCode_4;
            case KEY_5: return ScanCode_5;
            case KEY_6: return ScanCode_6;
            case KEY_7: return ScanCode_7;
            case KEY_8: return ScanCode_8;
            case KEY_9: return ScanCode_9;
            case KEY_0: return ScanCode_0;
            case KEY_MINUS: return ScanCode_MINUS;
            case KEY_EQUAL: return ScanCode_EQUALS;
            case KEY_BACKSPACE: return ScanCode_BACKSPACE;
            case KEY_TAB: return ScanCode_TAB;
            case KEY_Q: return ScanCode_Q;
            case KEY_W: return ScanCode_W;
            case KEY_E: return ScanCode_E;
            case KEY_R: return ScanCode_R;
            case KEY_T: return ScanCode_T;
            case KEY_Y: return ScanCode_Y;
            case KEY_U: return ScanCode_U;
            case KEY_I: return ScanCode_I;
            case KEY_O: return ScanCode_O;
            case KEY_P: return ScanCode_P;
            case KEY_LEFTBRACE: return ScanCode_LEFTBRACKET;
            case KEY_RIGHTBRACE: return ScanCode_RIGHTBRACKET;
            case KEY_ENTER: return ScanCode_RETURN;
            case KEY_LEFTCTRL: return ScanCode_LCTRL;
            case KEY_A: return ScanCode_A;
            case KEY_S: return ScanCode_S;
            case KEY_D: return ScanCode_D;
            case KEY_F: return ScanCode_F;
            case KEY_G: return ScanCode_G;
            case KEY_H: return ScanCode_H;
            case KEY_J: return ScanCode_J;
            case KEY_K: return ScanCode_K;
            case KEY_L: return ScanCode_L;
            case KEY_SEMICOLON: return ScanCode_SEMICOLON;
            case KEY_APOSTROPHE: return ScanCode_APOSTROPHE;
            case KEY_GRAVE: return ScanCode_GRAVE;
            case KEY_LEFTSHIFT: return ScanCode_LSHIFT;
            case KEY_BACKSLASH: return ScanCode_BACKSLASH;
            case KEY_Z: return ScanCode_Z;
            case KEY_X: return ScanCode_X;
            case KEY_C: return ScanCode_C;
            case KEY_V: return ScanCode_V;
            case KEY_B: return ScanCode_B;
            case KEY_N: return ScanCode_N;
            case KEY_M: return ScanCode_M;
            case KEY_COMMA: return ScanCode_COMMA;
            case KEY_DOT: return ScanCode_PERIOD;
            case KEY_SLASH: return ScanCode_SLASH;
            case KEY_RIGHTSHIFT: return ScanCode_RSHIFT;
            case KEY_KPASTERISK: return ScanCode_KEYPAD_MULTIPLY;
            case KEY_LEFTALT: return ScanCode_LALT;
            case KEY_SPACE: return ScanCode_SPACE;
            case KEY_CAPSLOCK: return ScanCode_CAPSLOCK;
            case KEY_F1: return ScanCode_F1;
            case KEY_F2: return ScanCode_F2;
            case KEY_F3: return ScanCode_F3;
            case KEY_F4: return ScanCode_F4;
            case KEY_F5: return ScanCode_F5;
            case KEY_F6: return ScanCode_F6;
            case KEY_F7: return ScanCode_F7;
            case KEY_F8: return ScanCode_F8;
            case KEY_F9: return ScanCode_F9;
            case KEY_F10: return ScanCode_F10;
            case KEY_NUMLOCK: return ScanCode_NUMLOCKCLEAR;
            case KEY_SCROLLLOCK: return ScanCode_SCROLLLOCK;
            case KEY_KP7: return ScanCode_KEYPAD_7;
            case KEY_KP8: return ScanCode_KEYPAD_8;
            case KEY_KP9: return ScanCode_KEYPAD_9;
            case KEY_KPMINUS: return ScanCode_KEYPAD_MINUS;
            case KEY_KP4: return ScanCode_KEYPAD_4;
            case KEY_KP5: return ScanCode_KEYPAD_5;
            case KEY_KP6: return ScanCode_KEYPAD_6;
            case KEY_KPPLUS: return ScanCode_KEYPAD_PLUS;
            case KEY_KP1: return ScanCode_KEYPAD_1;
            case KEY_KP2: return ScanCode_KEYPAD_2;
            case KEY_KP3: return ScanCode_KEYPAD_3;
            case KEY_KP0: return ScanCode_KEYPAD_0;
            case KEY_KPDOT: return ScanCode_KEYPAD_PERIOD;
            case KEY_ZENKAKUHANKAKU: return ScanCode_LANG5;
            case KEY_102ND: return ScanCode_NONUSBACKSLASH;
            case KEY_F11: return ScanCode_F11;
            case KEY_F12: return ScanCode_F12;
            case KEY_RO: return ScanCode_INTERNATIONAL1;
            case KEY_KATAKANA: return ScanCode_LANG3;
            case KEY_HIRAGANA: return ScanCode_LANG4;
            case KEY_HENKAN: return ScanCode_INTERNATIONAL4;
            case KEY_KATAKANAHIRAGANA: return ScanCode_INTERNATIONAL2;
            case KEY_MUHENKAN: return ScanCode_INTERNATIONAL5;
            case KEY_KPJPCOMMA: return ScanCode_INTERNATIONAL6;
            case KEY_KPENTER: return ScanCode_KEYPAD_ENTER;
            case KEY_RIGHTCTRL: return ScanCode_RCTRL;
            case KEY_KPSLASH: return ScanCode_KEYPAD_DIVIDE;
            case KEY_SYSRQ: return ScanCode_PRINTSCREEN;
            case KEY_RIGHTALT: return ScanCode_RALT;
            case KEY_HOME: return ScanCode_HOME;
            case KEY_UP: return ScanCode_UP;
            case KEY_PAGEUP: return ScanCode_PAGEUP;
            case KEY_LEFT: return ScanCode_LEFT;
            case KEY_RIGHT: return ScanCode_RIGHT;
            case KEY_END: return ScanCode_END;
            case KEY_DOWN: return ScanCode_DOWN;
            case KEY_PAGEDOWN: return ScanCode_PAGEDOWN;
            case KEY_INSERT: return ScanCode_INSERT;
            case KEY_DELETE: return ScanCode_DELETE;
            case KEY_MUTE: return ScanCode_AUDIOMUTE;
            case KEY_VOLUMEDOWN: return ScanCode_VOLUMEDOWN;
            case KEY_VOLUMEUP: return ScanCode_VOLUMEUP;
            case KEY_POWER: return ScanCode_POWER;
            case KEY_KPEQUAL: return ScanCode_KEYPAD_EQUALS;
            case KEY_KPPLUSMINUS: return ScanCode_KEYPAD_PLUSMINUS;
            case KEY_PAUSE: return ScanCode_PAUSE;
            case KEY_KPCOMMA: return ScanCode_KEYPAD_COMMA;
            case KEY_HANGEUL: return ScanCode_LANG1;
            case KEY_HANJA: return ScanCode_LANG2;
            case KEY_YEN: return ScanCode_INTERNATIONAL3;
            case KEY_LEFTMETA: return ScanCode_LGUI;
            case KEY_RIGHTMETA: return ScanCode_RGUI;
            case KEY_COMPOSE: return ScanCode_APPLICATION;
            case KEY_STOP: return ScanCode_STOP;
            case KEY_AGAIN: return ScanCode_AGAIN;
            case KEY_UNDO: return ScanCode_UNDO;
            case KEY_COPY: return ScanCode_COPY;
            case KEY_PASTE: return ScanCode_PASTE;
            case KEY_FIND: return ScanCode_FIND;
            case KEY_CUT: return ScanCode_CUT;
            case KEY_HELP: return ScanCode_HELP;
            case KEY_MENU: return ScanCode_MENU;
            case KEY_CALC: return ScanCode_CALCULATOR;
            case KEY_SLEEP: return ScanCode_SLEEP;
            case KEY_WWW: return ScanCode_WWW;
            case KEY_MAIL: return ScanCode_MAIL;
            case KEY_BOOKMARKS: return ScanCode_AC_BOOKMARKS;
            case KEY_COMPUTER: return ScanCode_COMPUTER;
            case KEY_BACK: return ScanCode_AC_BACK;
            case KEY_FORWARD: return ScanCode_AC_FORWARD;
            case KEY_EJECTCD: return ScanCode_EJECT;
            case KEY_NEXTSONG: return ScanCode_AUDIONEXT;
            case KEY_PLAYPAUSE: return ScanCode_AUDIOPLAY;
            case KEY_PREVIOUSSONG: return ScanCode_AUDIOPREV;
            case KEY_STOPCD: return ScanCode_AUDIOSTOP;
            case KEY_REWIND: return ScanCode_AUDIOREWIND;
            case KEY_HOMEPAGE: return ScanCode_AC_HOME;
            case KEY_REFRESH: return ScanCode_AC_REFRESH;
            case KEY_KPLEFTPAREN: return ScanCode_KEYPAD_LEFTPAREN;
            case KEY_KPRIGHTPAREN: return ScanCode_KEYPAD_RIGHTPAREN;
            case KEY_F13: return ScanCode_F13;
            case KEY_F14: return ScanCode_F14;
            case KEY_F15: return ScanCode_F15;
            case KEY_F16: return ScanCode_F16;
            case KEY_F17: return ScanCode_F17;
            case KEY_F18: return ScanCode_F18;
            case KEY_F19: return ScanCode_F19;
            case KEY_F20: return ScanCode_F20;
            case KEY_F21: return ScanCode_F21;
            case KEY_F22: return ScanCode_F22;
            case KEY_F23: return ScanCode_F23;
            case KEY_F24: return ScanCode_F24;
            case KEY_FASTFORWARD: return ScanCode_AUDIOFASTFORWARD;
            case KEY_SEARCH: return ScanCode_AC_SEARCH;
            case KEY_BRIGHTNESSDOWN: return ScanCode_BRIGHTNESSDOWN;
            case KEY_BRIGHTNESSUP: return ScanCode_BRIGHTNESSUP;
            case KEY_MEDIA: return ScanCode_MEDIASELECT;
            case KEY_SWITCHVIDEOMODE: return ScanCode_DISPLAYSWITCH;
            case KEY_KBDILLUMTOGGLE: return ScanCode_KBDILLUMTOGGLE;
            case KEY_KBDILLUMDOWN: return ScanCode_KBDILLUMDOWN;
            case KEY_KBDILLUMUP: return ScanCode_KBDILLUMUP;
            default: return ScanCode_UNKNOWN;
            }
        }

        std::uint32_t scan_code_to_evdev(scan_code_e aScanCode)
        {
            static std::array<std::uint32_t, ScanCodeCount> const sReverse = []()
            {
                std::array<std::uint32_t, ScanCodeCount> result = {};
                for (std::uint32_t keyCode = KEY_MAX; keyCode > 0u; --keyCode)
                {
                    auto const scanCode = evdev_to_scan_code(keyCode);
                    if (scanCode != ScanCode_UNKNOWN && scanCode < ScanCodeCount)
                        result[scanCode] = keyCode;
                }
                return result;
            }();
            if (aScanCode < ScanCodeCount)
                return sReverse[aScanCode];
            return 0u;
        }

        std::string key_symbol_to_utf8(std::uint32_t aKeySymbol)
        {
            char buffer[64] = {};
            auto const length = ::xkb_keysym_to_utf8(aKeySymbol, buffer, sizeof(buffer));
            if (length <= 1) // (the count includes the terminating null)
                return {};
            return std::string{ buffer };
        }

        std::vector<char const*> const& cursor_names(mouse_system_cursor aCursor)
        {
            static std::map<mouse_system_cursor, std::vector<char const*>> const sNames =
            {
                { mouse_system_cursor::Arrow, { "default", "left_ptr" } },
                { mouse_system_cursor::IBeam, { "text", "xterm" } },
                { mouse_system_cursor::Wait, { "wait", "watch" } },
                { mouse_system_cursor::Crosshair, { "crosshair", "cross" } },
                { mouse_system_cursor::WaitArrow, { "progress", "left_ptr_watch" } },
                { mouse_system_cursor::SizeNWSE, { "nwse-resize", "bottom_right_corner", "size_fdiag" } },
                { mouse_system_cursor::SizeNESW, { "nesw-resize", "bottom_left_corner", "size_bdiag" } },
                { mouse_system_cursor::SizeWE, { "ew-resize", "sb_h_double_arrow", "size_hor" } },
                { mouse_system_cursor::SizeNS, { "ns-resize", "sb_v_double_arrow", "size_ver" } },
                { mouse_system_cursor::SizeAll, { "move", "fleur", "size_all" } },
                { mouse_system_cursor::No, { "not-allowed", "crossed_circle", "forbidden" } },
                { mouse_system_cursor::Hand, { "pointer", "hand2", "hand1" } }
            };
            auto const existing = sNames.find(aCursor);
            if (existing != sNames.end())
                return existing->second;
            return sNames.at(mouse_system_cursor::Arrow);
        }

        std::chrono::milliseconds double_click_time()
        {
            // (as GTK's default (gtk-double-click-time) and Windows')
            return std::chrono::milliseconds{ 500 };
        }
    }
}
