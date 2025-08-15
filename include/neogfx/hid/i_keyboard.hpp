// i_keyboard.hpp
// Parts of this source file based on Simple DirectMedia Layer, Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>
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

#pragma once

#include <neogfx/neogfx.hpp>

#include <optional>

#include <neolib/core/vecarray.hpp>
#include <neolib/core/string_ci.hpp>

#include <neogfx/core/geometrical.hpp>
#include <neogfx/core/event.hpp>
#include <neogfx/hid/i_hid_device.hpp>

namespace neogfx
{
    enum native_scan_code_e : std::uintptr_t {};
    enum native_key_code_e : std::uintptr_t {};

    enum scan_code_e : std::int32_t
    {
        ScanCode_UNKNOWN = 0,
        ScanCode_A = 4,
        ScanCode_B = 5,
        ScanCode_C = 6,
        ScanCode_D = 7,
        ScanCode_E = 8,
        ScanCode_F = 9,
        ScanCode_G = 10,
        ScanCode_H = 11,
        ScanCode_I = 12,
        ScanCode_J = 13,
        ScanCode_K = 14,
        ScanCode_L = 15,
        ScanCode_M = 16,
        ScanCode_N = 17,
        ScanCode_O = 18,
        ScanCode_P = 19,
        ScanCode_Q = 20,
        ScanCode_R = 21,
        ScanCode_S = 22,
        ScanCode_T = 23,
        ScanCode_U = 24,
        ScanCode_V = 25,
        ScanCode_W = 26,
        ScanCode_X = 27,
        ScanCode_Y = 28,
        ScanCode_Z = 29,

        ScanCode_1 = 30,
        ScanCode_2 = 31,
        ScanCode_3 = 32,
        ScanCode_4 = 33,
        ScanCode_5 = 34,
        ScanCode_6 = 35,
        ScanCode_7 = 36,
        ScanCode_8 = 37,
        ScanCode_9 = 38,
        ScanCode_0 = 39,

        ScanCode_RETURN = 40,
        ScanCode_ESCAPE = 41,
        ScanCode_BACKSPACE = 42,
        ScanCode_TAB = 43,
        ScanCode_SPACE = 44,

        ScanCode_MINUS = 45,
        ScanCode_EQUALS = 46,
        ScanCode_LEFTBRACKET = 47,
        ScanCode_RIGHTBRACKET = 48,
        ScanCode_BACKSLASH = 49,

        ScanCode_NONUSHASH = 50,
        ScanCode_SEMICOLON = 51,
        ScanCode_APOSTROPHE = 52,
        ScanCode_GRAVE = 53,
        ScanCode_COMMA = 54,
        ScanCode_PERIOD = 55,
        ScanCode_SLASH = 56,

        ScanCode_CAPSLOCK = 57,

        ScanCode_F1 = 58,
        ScanCode_F2 = 59,
        ScanCode_F3 = 60,
        ScanCode_F4 = 61,
        ScanCode_F5 = 62,
        ScanCode_F6 = 63,
        ScanCode_F7 = 64,
        ScanCode_F8 = 65,
        ScanCode_F9 = 66,
        ScanCode_F10 = 67,
        ScanCode_F11 = 68,
        ScanCode_F12 = 69,

        ScanCode_PRINTSCREEN = 70,
        ScanCode_SCROLLLOCK = 71,
        ScanCode_PAUSE = 72,
        ScanCode_INSERT = 73,
        ScanCode_HOME = 74,
        ScanCode_PAGEUP = 75,
        ScanCode_DELETE = 76,
        ScanCode_END = 77,
        ScanCode_PAGEDOWN = 78,
        ScanCode_RIGHT = 79,
        ScanCode_LEFT = 80,
        ScanCode_DOWN = 81,
        ScanCode_UP = 82,

        ScanCode_NUMLOCKCLEAR = 83,
        ScanCode_KEYPAD_DIVIDE = 84,
        ScanCode_KEYPAD_MULTIPLY = 85,
        ScanCode_KEYPAD_MINUS = 86,
        ScanCode_KEYPAD_PLUS = 87,
        ScanCode_KEYPAD_ENTER = 88,
        ScanCode_KEYPAD_1 = 89,
        ScanCode_KEYPAD_2 = 90,
        ScanCode_KEYPAD_3 = 91,
        ScanCode_KEYPAD_4 = 92,
        ScanCode_KEYPAD_5 = 93,
        ScanCode_KEYPAD_6 = 94,
        ScanCode_KEYPAD_7 = 95,
        ScanCode_KEYPAD_8 = 96,
        ScanCode_KEYPAD_9 = 97,
        ScanCode_KEYPAD_0 = 98,
        ScanCode_KEYPAD_PERIOD = 99,

        ScanCode_NONUSBACKSLASH = 100,
        ScanCode_APPLICATION = 101,
        ScanCode_POWER = 102,

        ScanCode_KEYPAD_EQUALS = 103,
        ScanCode_F13 = 104,
        ScanCode_F14 = 105,
        ScanCode_F15 = 106,
        ScanCode_F16 = 107,
        ScanCode_F17 = 108,
        ScanCode_F18 = 109,
        ScanCode_F19 = 110,
        ScanCode_F20 = 111,
        ScanCode_F21 = 112,
        ScanCode_F22 = 113,
        ScanCode_F23 = 114,
        ScanCode_F24 = 115,
        ScanCode_EXECUTE = 116,
        ScanCode_HELP = 117,
        ScanCode_MENU = 118,
        ScanCode_SELECT = 119,
        ScanCode_STOP = 120,
        ScanCode_AGAIN = 121,
        ScanCode_UNDO = 122,
        ScanCode_CUT = 123,
        ScanCode_COPY = 124,
        ScanCode_PASTE = 125,
        ScanCode_FIND = 126,
        ScanCode_MUTE = 127,
        ScanCode_VOLUMEUP = 128,
        ScanCode_VOLUMEDOWN = 129,

        ScanCode_LOCKING_CAPSLOCK = 130,
        ScanCode_LOCKING_NUMLOCK = 131,
        ScanCode_LOCKING_SCROLLLOCK = 132,

        ScanCode_KEYPAD_COMMA = 133,
        ScanCode_KEYPAD_AS400_EQUALS = 134,

        ScanCode_INTERNATIONAL1 = 135,
        ScanCode_INTERNATIONAL2 = 136,
        ScanCode_INTERNATIONAL3 = 137,
        ScanCode_INTERNATIONAL4 = 138,
        ScanCode_INTERNATIONAL5 = 139,
        ScanCode_INTERNATIONAL6 = 140,
        ScanCode_INTERNATIONAL7 = 141,
        ScanCode_INTERNATIONAL8 = 142,
        ScanCode_INTERNATIONAL9 = 143,
        ScanCode_LANG1 = 144, /**< Hangul/English toggle */
        ScanCode_LANG2 = 145, /**< Hanja conversion */
        ScanCode_LANG3 = 146, /**< Katakana */
        ScanCode_LANG4 = 147, /**< Hiragana */
        ScanCode_LANG5 = 148, /**< Zenkaku/Hankaku */
        ScanCode_LANG6 = 149, /**< reserved */
        ScanCode_LANG7 = 150, /**< reserved */
        ScanCode_LANG8 = 151, /**< reserved */
        ScanCode_LANG9 = 152, /**< reserved */

        ScanCode_ALTERASE = 153,
        ScanCode_SYSREQ = 154,
        ScanCode_CANCEL = 155,
        ScanCode_CLEAR = 156,
        ScanCode_PRIOR = 157,
        ScanCode_RETURN2 = 158,
        ScanCode_SEPARATOR = 159,
        ScanCode_OUT = 160,
        ScanCode_OPER = 161,
        ScanCode_CLEARAGAIN = 162,
        ScanCode_CRSEL = 163,
        ScanCode_EXSEL = 164,

        ScanCode_KEYPAD_00 = 176,
        ScanCode_KEYPAD_000 = 177,
        ScanCode_THOUSANDSSEPARATOR = 178,
        ScanCode_DECIMALSEPARATOR = 179,
        ScanCode_CURRENCYUNIT = 180,
        ScanCode_CURRENCYSUBUNIT = 181,
        ScanCode_KEYPAD_LEFTPAREN = 182,
        ScanCode_KEYPAD_RIGHTPAREN = 183,
        ScanCode_KEYPAD_LEFTBRACE = 184,
        ScanCode_KEYPAD_RIGHTBRACE = 185,
        ScanCode_KEYPAD_TAB = 186,
        ScanCode_KEYPAD_BACKSPACE = 187,
        ScanCode_KEYPAD_A = 188,
        ScanCode_KEYPAD_B = 189,
        ScanCode_KEYPAD_C = 190,
        ScanCode_KEYPAD_D = 191,
        ScanCode_KEYPAD_E = 192,
        ScanCode_KEYPAD_F = 193,
        ScanCode_KEYPAD_XOR = 194,
        ScanCode_KEYPAD_POWER = 195,
        ScanCode_KEYPAD_PERCENT = 196,
        ScanCode_KEYPAD_LESS = 197,
        ScanCode_KEYPAD_GREATER = 198,
        ScanCode_KEYPAD_AMPERSAND = 199,
        ScanCode_KEYPAD_DBLAMPERSAND = 200,
        ScanCode_KEYPAD_VERTICALBAR = 201,
        ScanCode_KEYPAD_DBLVERTICALBAR = 202,
        ScanCode_KEYPAD_COLON = 203,
        ScanCode_KEYPAD_HASH = 204,
        ScanCode_KEYPAD_SPACE = 205,
        ScanCode_KEYPAD_AT = 206,
        ScanCode_KEYPAD_EXCLAM = 207,
        ScanCode_KEYPAD_MEMSTORE = 208,
        ScanCode_KEYPAD_MEMRECALL = 209,
        ScanCode_KEYPAD_MEMCLEAR = 210,
        ScanCode_KEYPAD_MEMADD = 211,
        ScanCode_KEYPAD_MEMSUBTRACT = 212,
        ScanCode_KEYPAD_MEMMULTIPLY = 213,
        ScanCode_KEYPAD_MEMDIVIDE = 214,
        ScanCode_KEYPAD_PLUSMINUS = 215,
        ScanCode_KEYPAD_CLEAR = 216,
        ScanCode_KEYPAD_CLEARENTRY = 217,
        ScanCode_KEYPAD_BINARY = 218,
        ScanCode_KEYPAD_OCTAL = 219,
        ScanCode_KEYPAD_DECIMAL = 220,
        ScanCode_KEYPAD_HEXADECIMAL = 221,

        ScanCode_LCTRL = 224,
        ScanCode_LSHIFT = 225,
        ScanCode_LALT = 226, /**< alt, option */
        ScanCode_LGUI = 227, /**< windows, command (apple), meta */
        ScanCode_RCTRL = 228,
        ScanCode_RSHIFT = 229,
        ScanCode_RALT = 230, /**< alt gr, option */
        ScanCode_RGUI = 231, /**< windows, command (apple), meta */

        ScanCode_MODE = 257,

        ScanCode_AUDIONEXT = 258,
        ScanCode_AUDIOPREV = 259,
        ScanCode_AUDIOSTOP = 260,
        ScanCode_AUDIOPLAY = 261,
        ScanCode_AUDIOMUTE = 262,
        ScanCode_MEDIASELECT = 263,
        ScanCode_WWW = 264,
        ScanCode_MAIL = 265,
        ScanCode_CALCULATOR = 266,
        ScanCode_COMPUTER = 267,
        ScanCode_AC_SEARCH = 268,
        ScanCode_AC_HOME = 269,
        ScanCode_AC_BACK = 270,
        ScanCode_AC_FORWARD = 271,
        ScanCode_AC_STOP = 272,
        ScanCode_AC_REFRESH = 273,
        ScanCode_AC_BOOKMARKS = 274,

        ScanCode_BRIGHTNESSDOWN = 275,
        ScanCode_BRIGHTNESSUP = 276,
        ScanCode_DISPLAYSWITCH = 277,

        ScanCode_KBDILLUMTOGGLE = 278,
        ScanCode_KBDILLUMDOWN = 279,
        ScanCode_KBDILLUMUP = 280,
        ScanCode_EJECT = 281,
        ScanCode_SLEEP = 282,

        ScanCode_APP1 = 283,
        ScanCode_APP2 = 284,

        ScanCode_AUDIOREWIND = 285,
        ScanCode_AUDIOFASTFORWARD = 286,

        ScanCodeCount = 512
    };

    #define NEOGFX_SCANCODE_MASK (1<<30)
    #define NEOGFX_SCANCODE_TO_KEYCODE(X)  (X | NEOGFX_SCANCODE_MASK)

    enum key_code_e : std::int32_t
    {
        KeyCode_UNKNOWN = 0,

        KeyCode_RETURN = '\r',
        KeyCode_ESCAPE = '\033',
        KeyCode_BACKSPACE = '\b',
        KeyCode_TAB = '\t',
        KeyCode_SPACE = ' ',
        KeyCode_EXCLAIM = '!',
        KeyCode_QUOTEDBL = '"',
        KeyCode_HASH = '#',
        KeyCode_PERCENT = '%',
        KeyCode_DOLLAR = '$',
        KeyCode_AMPERSAND = '&',
        KeyCode_QUOTE = '\'',
        KeyCode_LEFTPAREN = '(',
        KeyCode_RIGHTPAREN = ')',
        KeyCode_ASTERISK = '*',
        KeyCode_PLUS = '+',
        KeyCode_COMMA = ',',
        KeyCode_MINUS = '-',
        KeyCode_PERIOD = '.',
        KeyCode_SLASH = '/',
        KeyCode_0 = '0',
        KeyCode_1 = '1',
        KeyCode_2 = '2',
        KeyCode_3 = '3',
        KeyCode_4 = '4',
        KeyCode_5 = '5',
        KeyCode_6 = '6',
        KeyCode_7 = '7',
        KeyCode_8 = '8',
        KeyCode_9 = '9',
        KeyCode_COLON = ':',
        KeyCode_SEMICOLON = ';',
        KeyCode_LESS = '<',
        KeyCode_EQUALS = '=',
        KeyCode_GREATER = '>',
        KeyCode_QUESTION = '?',
        KeyCode_AT = '@',

        KeyCode_LEFTBRACKET = '[',
        KeyCode_BACKSLASH = '\\',
        KeyCode_RIGHTBRACKET = ']',
        KeyCode_CARET = '^',
        KeyCode_UNDERSCORE = '_',
        KeyCode_BACKQUOTE = '`',
        KeyCode_a = 'a',
        KeyCode_b = 'b',
        KeyCode_c = 'c',
        KeyCode_d = 'd',
        KeyCode_e = 'e',
        KeyCode_f = 'f',
        KeyCode_g = 'g',
        KeyCode_h = 'h',
        KeyCode_i = 'i',
        KeyCode_j = 'j',
        KeyCode_k = 'k',
        KeyCode_l = 'l',
        KeyCode_m = 'm',
        KeyCode_n = 'n',
        KeyCode_o = 'o',
        KeyCode_p = 'p',
        KeyCode_q = 'q',
        KeyCode_r = 'r',
        KeyCode_s = 's',
        KeyCode_t = 't',
        KeyCode_u = 'u',
        KeyCode_v = 'v',
        KeyCode_w = 'w',
        KeyCode_x = 'x',
        KeyCode_y = 'y',
        KeyCode_z = 'z',

        KeyCode_CAPSLOCK = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_CAPSLOCK),

        KeyCode_F1 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F1),
        KeyCode_F2 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F2),
        KeyCode_F3 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F3),
        KeyCode_F4 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F4),
        KeyCode_F5 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F5),
        KeyCode_F6 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F6),
        KeyCode_F7 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F7),
        KeyCode_F8 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F8),
        KeyCode_F9 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F9),
        KeyCode_F10 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F10),
        KeyCode_F11 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F11),
        KeyCode_F12 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F12),

        KeyCode_PRINTSCREEN = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_PRINTSCREEN),
        KeyCode_SCROLLLOCK = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_SCROLLLOCK),
        KeyCode_PAUSE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_PAUSE),
        KeyCode_INSERT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_INSERT),
        KeyCode_HOME = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_HOME),
        KeyCode_PAGEUP = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_PAGEUP),
        KeyCode_DELETE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_DELETE),
        KeyCode_END = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_END),
        KeyCode_PAGEDOWN = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_PAGEDOWN),
        KeyCode_RIGHT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_RIGHT),
        KeyCode_LEFT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_LEFT),
        KeyCode_DOWN = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_DOWN),
        KeyCode_UP = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_UP),

        KeyCode_NUMLOCKCLEAR = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_NUMLOCKCLEAR),
        KeyCode_KEYPAD_DIVIDE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_DIVIDE),
        KeyCode_KEYPAD_MULTIPLY = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_MULTIPLY),
        KeyCode_KEYPAD_MINUS = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_MINUS),
        KeyCode_KEYPAD_PLUS = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_PLUS),
        KeyCode_KEYPAD_ENTER = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_ENTER),
        KeyCode_KEYPAD_1 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_1),
        KeyCode_KEYPAD_2 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_2),
        KeyCode_KEYPAD_3 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_3),
        KeyCode_KEYPAD_4 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_4),
        KeyCode_KEYPAD_5 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_5),
        KeyCode_KEYPAD_6 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_6),
        KeyCode_KEYPAD_7 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_7),
        KeyCode_KEYPAD_8 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_8),
        KeyCode_KEYPAD_9 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_9),
        KeyCode_KEYPAD_0 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_0),
        KeyCode_KEYPAD_PERIOD = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_PERIOD),

        KeyCode_APPLICATION = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_APPLICATION),
        KeyCode_POWER = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_POWER),
        KeyCode_KEYPAD_EQUALS = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_EQUALS),
        KeyCode_F13 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F13),
        KeyCode_F14 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F14),
        KeyCode_F15 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F15),
        KeyCode_F16 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F16),
        KeyCode_F17 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F17),
        KeyCode_F18 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F18),
        KeyCode_F19 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F19),
        KeyCode_F20 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F20),
        KeyCode_F21 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F21),
        KeyCode_F22 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F22),
        KeyCode_F23 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F23),
        KeyCode_F24 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_F24),
        KeyCode_EXECUTE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_EXECUTE),
        KeyCode_HELP = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_HELP),
        KeyCode_MENU = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_MENU),
        KeyCode_SELECT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_SELECT),
        KeyCode_STOP = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_STOP),
        KeyCode_AGAIN = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_AGAIN),
        KeyCode_UNDO = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_UNDO),
        KeyCode_CUT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_CUT),
        KeyCode_COPY = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_COPY),
        KeyCode_PASTE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_PASTE),
        KeyCode_FIND = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_FIND),
        KeyCode_MUTE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_MUTE),
        KeyCode_VOLUMEUP = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_VOLUMEUP),
        KeyCode_VOLUMEDOWN = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_VOLUMEDOWN),
        KeyCode_KEYPAD_COMMA = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_COMMA),
        KeyCode_KEYPAD_AS400_EQUALS = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_AS400_EQUALS),

        KeyCode_ALTERASE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_ALTERASE),
        KeyCode_SYSREQ = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_SYSREQ),
        KeyCode_CANCEL = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_CANCEL),
        KeyCode_CLEAR = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_CLEAR),
        KeyCode_PRIOR = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_PRIOR),
        KeyCode_RETURN2 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_RETURN2),
        KeyCode_SEPARATOR = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_SEPARATOR),
        KeyCode_OUT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_OUT),
        KeyCode_OPER = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_OPER),
        KeyCode_CLEARAGAIN = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_CLEARAGAIN),
        KeyCode_CRSEL = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_CRSEL),
        KeyCode_EXSEL = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_EXSEL),

        KeyCode_KEYPAD_00 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_00),
        KeyCode_KEYPAD_000 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_000),
        KeyCode_THOUSANDSSEPARATOR = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_THOUSANDSSEPARATOR),
        KeyCode_DECIMALSEPARATOR = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_DECIMALSEPARATOR),
        KeyCode_CURRENCYUNIT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_CURRENCYUNIT),
        KeyCode_CURRENCYSUBUNIT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_CURRENCYSUBUNIT),
        KeyCode_KEYPAD_LEFTPAREN = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_LEFTPAREN),
        KeyCode_KEYPAD_RIGHTPAREN = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_RIGHTPAREN),
        KeyCode_KEYPAD_LEFTBRACE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_LEFTBRACE),
        KeyCode_KEYPAD_RIGHTBRACE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_RIGHTBRACE),
        KeyCode_KEYPAD_TAB = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_TAB),
        KeyCode_KEYPAD_BACKSPACE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_BACKSPACE),
        KeyCode_KEYPAD_A = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_A),
        KeyCode_KEYPAD_B = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_B),
        KeyCode_KEYPAD_C = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_C),
        KeyCode_KEYPAD_D = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_D),
        KeyCode_KEYPAD_E = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_E),
        KeyCode_KEYPAD_F = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_F),
        KeyCode_KEYPAD_XOR = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_XOR),
        KeyCode_KEYPAD_POWER = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_POWER),
        KeyCode_KEYPAD_PERCENT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_PERCENT),
        KeyCode_KEYPAD_LESS = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_LESS),
        KeyCode_KEYPAD_GREATER = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_GREATER),
        KeyCode_KEYPAD_AMPERSAND = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_AMPERSAND),
        KeyCode_KEYPAD_DBLAMPERSAND = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_DBLAMPERSAND),
        KeyCode_KEYPAD_VERTICALBAR = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_VERTICALBAR),
        KeyCode_KEYPAD_DBLVERTICALBAR = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_DBLVERTICALBAR),
        KeyCode_KEYPAD_COLON = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_COLON),
        KeyCode_KEYPAD_HASH = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_HASH),
        KeyCode_KEYPAD_SPACE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_SPACE),
        KeyCode_KEYPAD_AT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_AT),
        KeyCode_KEYPAD_EXCLAM = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_EXCLAM),
        KeyCode_KEYPAD_MEMSTORE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_MEMSTORE),
        KeyCode_KEYPAD_MEMRECALL = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_MEMRECALL),
        KeyCode_KEYPAD_MEMCLEAR = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_MEMCLEAR),
        KeyCode_KEYPAD_MEMADD = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_MEMADD),
        KeyCode_KEYPAD_MEMSUBTRACT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_MEMSUBTRACT),
        KeyCode_KEYPAD_MEMMULTIPLY = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_MEMMULTIPLY),
        KeyCode_KEYPAD_MEMDIVIDE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_MEMDIVIDE),
        KeyCode_KEYPAD_PLUSMINUS = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_PLUSMINUS),
        KeyCode_KEYPAD_CLEAR = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_CLEAR),
        KeyCode_KEYPAD_CLEARENTRY = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_CLEARENTRY),
        KeyCode_KEYPAD_BINARY = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_BINARY),
        KeyCode_KEYPAD_OCTAL = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_OCTAL),
        KeyCode_KEYPAD_DECIMAL = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_DECIMAL),
        KeyCode_KEYPAD_HEXADECIMAL = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KEYPAD_HEXADECIMAL),

        KeyCode_LCTRL = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_LCTRL),
        KeyCode_LSHIFT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_LSHIFT),
        KeyCode_LALT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_LALT),
        KeyCode_LGUI = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_LGUI),
        KeyCode_RCTRL = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_RCTRL),
        KeyCode_RSHIFT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_RSHIFT),
        KeyCode_RALT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_RALT),
        KeyCode_RGUI = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_RGUI),

        KeyCode_MODE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_MODE),

        KeyCode_AUDIONEXT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_AUDIONEXT),
        KeyCode_AUDIOPREV = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_AUDIOPREV),
        KeyCode_AUDIOSTOP = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_AUDIOSTOP),
        KeyCode_AUDIOPLAY = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_AUDIOPLAY),
        KeyCode_AUDIOMUTE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_AUDIOMUTE),
        KeyCode_MEDIASELECT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_MEDIASELECT),
        KeyCode_WWW = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_WWW),
        KeyCode_MAIL = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_MAIL),
        KeyCode_CALCULATOR = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_CALCULATOR),
        KeyCode_COMPUTER = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_COMPUTER),
        KeyCode_AC_SEARCH = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_AC_SEARCH),
        KeyCode_AC_HOME = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_AC_HOME),
        KeyCode_AC_BACK = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_AC_BACK),
        KeyCode_AC_FORWARD = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_AC_FORWARD),
        KeyCode_AC_STOP = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_AC_STOP),
        KeyCode_AC_REFRESH = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_AC_REFRESH),
        KeyCode_AC_BOOKMARKS = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_AC_BOOKMARKS),

        KeyCode_BRIGHTNESSDOWN = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_BRIGHTNESSDOWN),
        KeyCode_BRIGHTNESSUP = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_BRIGHTNESSUP),
        KeyCode_DISPLAYSWITCH = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_DISPLAYSWITCH),
        KeyCode_KBDILLUMTOGGLE = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KBDILLUMTOGGLE),
        KeyCode_KBDILLUMDOWN = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KBDILLUMDOWN),
        KeyCode_KBDILLUMUP = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_KBDILLUMUP),
        KeyCode_EJECT = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_EJECT),
        KeyCode_SLEEP = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_SLEEP),
        KeyCode_APP1 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_APP1),
        KeyCode_APP2 = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_APP2),
        KeyCode_AUDIOREWIND = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_AUDIOREWIND),
        KeyCode_AUDIOFASTFORWARD = NEOGFX_SCANCODE_TO_KEYCODE(ScanCode_AUDIOFASTFORWARD)
    };

    enum key_modifiers_e : std::uint16_t
    {
        KeyModifier_NONE        = 0x0000,
        KeyModifier_LSHIFT      = 0x0001,
        KeyModifier_RSHIFT      = 0x0002,
        KeyModifier_SHIFT       = KeyModifier_LSHIFT | KeyModifier_RSHIFT,
        KeyModifier_LCTRL       = 0x0004,
        KeyModifier_RCTRL       = 0x0008,
        KeyModifier_CTRL        = KeyModifier_LCTRL | KeyModifier_RCTRL,
        KeyModifier_LALT        = 0x0010,
        KeyModifier_RALT        = 0x0020,
        KeyModifier_ALT         = KeyModifier_LALT | KeyModifier_RALT,
        KeyModifier_LGUI        = 0x0040,
        KeyModifier_RGUI        = 0x0080,
        KeyModifier_GUI         = KeyModifier_LGUI | KeyModifier_RGUI,
        KeyModifier_EXTENDED    = 0x0100,
        KeyModifier_KEYPAD      = KeyModifier_EXTENDED,
        KeyModifier_SYSTEM      = 0x0200,
        KeyModifier_NUM         = 0x1000,
        KeyModifier_CAPS        = 0x2000,
        KeyModifier_SCROLL      = 0x4000,
        KeyModifier_LOCKS       = KeyModifier_NUM | KeyModifier_CAPS | KeyModifier_SCROLL,
    };

    class i_keyboard_handler
    {
    public:
        virtual bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) = 0;
        virtual bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) = 0;
        virtual bool text_input(i_string const& aText) = 0;
        virtual bool sys_text_input(i_string const& aText) = 0;
    };

    enum class keyboard_locks : std::uint32_t
    {
        None        = 0x0000,
        CapsLock    = 0x0001,
        NumLock     = 0x0002,
        ScrollLock  = 0x0004,
        InsertLock  = 0x0008
    };

    constexpr keyboard_locks operator|(keyboard_locks aLhs, keyboard_locks aRhs)
    {
        return static_cast<keyboard_locks>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    constexpr keyboard_locks operator&(keyboard_locks aLhs, keyboard_locks aRhs)
    {
        return static_cast<keyboard_locks>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    class i_widget;

    class i_keyboard_layout
    {
    public:
        struct ime_not_open : std::logic_error { ime_not_open() : std::logic_error("neogfx::i_keyboard_layout::ime_not_open") {} };
        struct ime_not_active : std::logic_error { ime_not_active() : std::logic_error("neogfx::i_keyboard_layout::ime_not_active") {} };
        struct ime_activation_failure : std::runtime_error { ime_activation_failure() : std::runtime_error("neogfx::i_keyboard_layout::ime_activation_failure") {} };
    public:
        virtual ~i_keyboard_layout() = default;
    public:
        virtual bool has_ime() const = 0;
        virtual bool ime_open() const = 0;
        virtual bool ime_active() const = 0;
        virtual bool ime_active(i_widget const& aInputWidget) const = 0;
        virtual i_widget const& input_widget() const = 0;
        virtual point const& position() const = 0;
        virtual void open_ime() = 0;
        virtual void close_ime() = 0;
        virtual void activate_ime(i_widget const& aInputWidget, optional_point const& aPosition = {}) = 0;
        virtual void deactivate_ime(i_widget const& aInputWidget) = 0;
        virtual void update_ime_position(point const& aPosition) = 0;
    };

    class i_keyboard : public i_hid_device, public i_service
    {
    public:
        typedef i_keyboard abstract_type;
    public:
        declare_event(key_pressed, scan_code_e, key_code_e, key_modifiers_e)
        declare_event(key_released, scan_code_e, key_code_e, key_modifiers_e)
        declare_event(text_input, std::string const&)
        declare_event(sys_text_input, std::string const&)
        declare_event(input_language_changed)
    public:
        struct no_grab : std::logic_error { no_grab() : std::logic_error("neogfx::i_keyboard::no_grab") {} };
        struct already_grabbed : std::logic_error { already_grabbed() : std::logic_error("neogfx::i_keyboard::already_grabbed") {} };
    public:
        virtual i_keyboard_layout& layout() const = 0;
    public:
        virtual bool is_key_pressed(scan_code_e aScanCode) const = 0;
        virtual keyboard_locks locks() const = 0;
        virtual key_modifiers_e modifiers() const = 0;
        virtual key_modifiers_e event_modifiers() const = 0;
        virtual void set_event_modifiers(key_modifiers_e aModifiers) = 0;
        virtual void clear_event_modifiers() = 0;
    public:
        virtual bool is_keyboard_grabbed() const = 0;
        virtual bool is_keyboard_grabbed_by(i_keyboard_handler& aKeyboardHandler) const = 0;
        virtual bool is_front_grabber(i_keyboard_handler& aKeyboardHandler) const = 0;
        virtual void grab_keyboard(i_keyboard_handler& aKeyboardHandler) = 0;
        virtual void ungrab_keyboard(i_keyboard_handler& aKeyboardHandler) = 0;
        virtual i_keyboard_handler& grabber() const = 0;
    public:
        virtual key_code_e scan_code_to_key_code(scan_code_e aScanCode) const = 0;
        virtual native_key_code_e native_key_code_to_usb_hid_key_code(native_key_code_e aKeyCode) const = 0;
    public:
        static uuid const& iid() { static uuid const sIid{ 0x878ef7c6, 0x8d4b, 0x4c0d, 0x81dd, { 0x70, 0xd1, 0x74, 0x3c, 0x6b, 0x82 } }; return sIid; }
    };

    class key_sequence
    {
    public:
        enum class match
        {
            No,
            Partial,
            Full,
        };
    private:
        typedef std::pair<key_code_e, std::set<key_modifiers_e>> combo_type;
        typedef std::vector<combo_type> sequence_type;
    public:
        explicit key_sequence(std::string const& aSequence) : iText{ aSequence }
        {
            std::vector<neolib::ci_string> sequenceBits;
            neolib::tokens(neolib::make_ci_string(aSequence), neolib::ci_string(", "), sequenceBits);
            for (auto const& combo : sequenceBits)
            {
                combo_type nextCombo;
                neolib::vecarray<neolib::ci_string, 5> comboBits;
                neolib::tokens(combo, neolib::ci_string("+"), comboBits, comboBits.max_size(), false);
                for (std::size_t i = 0; i < comboBits.size(); ++i)
                {
                    if (i < comboBits.size() - 1)
                    {
                        if (comboBits[i] == "Ctrl")
                            nextCombo.second.insert(KeyModifier_CTRL);
                        else if (comboBits[i] == "Alt")
                            nextCombo.second.insert(KeyModifier_ALT);
                        else if (comboBits[i] == "Shift")
                            nextCombo.second.insert(KeyModifier_SHIFT);
                    }
                    else
                    {
                        static const std::map<neolib::ci_string, key_code_e> sKeyNames
                        {
                            { "Esc", KeyCode_ESCAPE },
                            { "Escape", KeyCode_ESCAPE },
                            { "Return", KeyCode_RETURN },
                            { "Enter", KeyCode_KEYPAD_ENTER },
                            { "Tab", KeyCode_TAB },
                            { "Space", KeyCode_SPACE },
                            { "Backspace", KeyCode_BACKSPACE },
                            { "Ins", KeyCode_INSERT },
                            { "Insert", KeyCode_INSERT },
                            { "Del", KeyCode_DELETE },
                            { "Delete", KeyCode_DELETE },
                            { "Home", KeyCode_HOME },
                            { "End", KeyCode_END },
                            { "Up", KeyCode_UP },
                            { "Arrow Up", KeyCode_UP },
                            { "Down", KeyCode_DOWN },
                            { "Arrow Down", KeyCode_DOWN },
                            { "Left", KeyCode_LEFT },
                            { "Arrow Left", KeyCode_LEFT },
                            { "Right", KeyCode_RIGHT },
                            { "Arrow Right", KeyCode_RIGHT },
                            { "PgUp", KeyCode_PAGEUP },
                            { "Page Up", KeyCode_PAGEUP },
                            { "PgDn", KeyCode_PAGEDOWN },
                            { "PgDown", KeyCode_PAGEDOWN },
                            { "Page Down", KeyCode_PAGEDOWN },
                            { "F1", KeyCode_F1 },
                            { "F2", KeyCode_F2 },
                            { "F3", KeyCode_F3 },
                            { "F4", KeyCode_F4 },
                            { "F5", KeyCode_F5 },
                            { "F6", KeyCode_F6 },
                            { "F7", KeyCode_F7 },
                            { "F8", KeyCode_F8 },
                            { "F9", KeyCode_F9 },
                            { "F10", KeyCode_F10 },
                            { "F11", KeyCode_F11 },
                            { "F12", KeyCode_F12 },
                            { "F13", KeyCode_F13 },
                            { "F14", KeyCode_F14 },
                            { "F15", KeyCode_F15 },
                            { "F16", KeyCode_F16 },
                            { "F17", KeyCode_F17 },
                            { "F18", KeyCode_F18 },
                            { "F19", KeyCode_F19 },
                            { "F20", KeyCode_F20 },
                            { "F21", KeyCode_F21 },
                            { "F22", KeyCode_F22 },
                            { "F23", KeyCode_F23 },
                            { "F24", KeyCode_F24 }
                        };
                        if (!comboBits[i].empty())
                        {
                            auto name = sKeyNames.find(comboBits[i]);
                            if (name == sKeyNames.end())
                                nextCombo.first = static_cast<key_code_e>(neolib::to_lower(neolib::make_string(comboBits[i]))[0]);
                            else
                                nextCombo.first = name->second;
                        }
                        else
                            nextCombo.first = KeyCode_PLUS;
                    }
                }
                iSequence.push_back(nextCombo);
            }
        }
    public:
        bool operator==(const key_sequence& aRhs) const
        {
            return iSequence == aRhs.iSequence;
        }
        bool operator!=(const key_sequence& aRhs) const
        {
            return !(*this == aRhs);
        }
    public:
        template <typename Iter> // iterator to std::pair<key_code_e, key_modifiers_e>
        match matches(Iter aFirst, Iter aLast) const
        {
            std::size_t index = 0;
            for (Iter next = aFirst; next != aLast; ++next, ++index)
                if (matches(index, next->first, next->second) == match::No)
                    return match::No;
            return (index == iSequence.size() ? match::Full : match::Partial);
        }
        match matches(key_code_e aKeyCode, key_modifiers_e aKeyModifiers) const
        {
            return matches(0, aKeyCode, aKeyModifiers);
        }
        const i_string& as_text() const
        {
            return iText;
        }
    private:
        match matches(std::size_t aIndex, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) const
        {
            if (iSequence[aIndex].first != aKeyCode)
                return match::No;
            for (auto m : iSequence[aIndex].second)
                if ((aKeyModifiers & m) != 0)
                    aKeyModifiers = static_cast<key_modifiers_e>(aKeyModifiers & ~m);
                else
                    return match::No;
            if ((aKeyModifiers & ~KeyModifier_LOCKS) == 0)
                return (iSequence.size() == 1 ? match::Full : match::Partial);
            else
                return match::No;
        }
    private:
        string iText;
        sequence_type iSequence;
    };

    typedef std::optional<key_sequence> optional_key_sequence;
}