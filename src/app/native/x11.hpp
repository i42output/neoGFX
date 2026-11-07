// x11.hpp
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

// Xlib, for the X11 backend's source files, which include it after every other header: Xlib defines macros (e.g.
// None, Bool, True, Success, Always, Above) that would otherwise change the meaning of names that neoGFX (and other
// libraries) use; what is needed of those is here as constants.

#pragma once

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/Xresource.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xcursor/Xcursor.h>

namespace neogfx::native::x11
{
    constexpr long XNone = None;
    constexpr int XTrue = True;
    constexpr int XFalse = False;
    constexpr int XSuccess = Success;
    typedef Bool XBool;
    typedef Status XStatus;
}

#undef None
#undef Bool
#undef Status
#undef True
#undef False
#undef Success
#undef Always
#undef Above
#undef Below
#undef Complex
#undef Convex
#undef CurrentTime
#undef DestroyAll
#undef Expose
#undef FocusIn
#undef FocusOut
#undef KeyPress
#undef KeyRelease
#undef ButtonPress
#undef ButtonRelease

namespace neogfx::native::x11
{
    constexpr ::Time XCurrentTime = 0L;
    // (event types)
    constexpr int XKeyPress = 2;
    constexpr int XKeyRelease = 3;
    constexpr int XButtonPress = 4;
    constexpr int XButtonRelease = 5;
    constexpr int XFocusIn = 9;
    constexpr int XFocusOut = 10;
    constexpr int XExpose = 12;
}
