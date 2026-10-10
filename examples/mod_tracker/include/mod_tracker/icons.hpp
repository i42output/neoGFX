/*
neogfx C++ App/Game Engine - Examples - MOD Tracker
Copyright(C) 2026 Leigh Johnston

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

#include <mod_tracker/mod_tracker.hpp>

#include <string>

#include <neogfx/gfx/image.hpp>

namespace mod_tracker::icons
{
    // the toolbar's icons, in the style of Material Design's (on a 24 x 24 grid, drawn 20 x 20), as SVG drawn in
    // currentColor
    extern char const* const OPEN;
    extern char const* const PLAY;
    extern char const* const PAUSE;
    extern char const* const STOP;
    extern char const* const PREVIOUS;
    extern char const* const NEXT;
    extern char const* const REPEAT;

    // an icon as an image, its currentColor aColor
    ng::image make(std::string const& aName, char const* aSvg, ng::color const& aColor, ng::dimension aDpiScaleFactor);
}
