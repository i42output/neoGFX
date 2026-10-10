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

#include <mod_tracker/mod_tracker.hpp>

#include <format>

#include <neogfx/app/i_resource_manager.hpp>

#include <mod_tracker/icons.hpp>

namespace mod_tracker::icons
{
    #define ICON_SVG(elements) \
        R"(<svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">)" \
        elements "</svg>"

    char const* const OPEN = ICON_SVG(
        R"(<polygon points="3,5 9,5 11,7 21,7 21,19 3,19"/>)"
        R"(<line x1="3" y1="10" x2="21" y2="10"/>)");
    char const* const PLAY = ICON_SVG(
        R"(<polygon points="8,5 8,19 19,12" fill="currentColor" stroke="none"/>)");
    char const* const PAUSE = ICON_SVG(
        R"(<rect x="6" y="5" width="4" height="14" fill="currentColor" stroke="none"/>)"
        R"(<rect x="14" y="5" width="4" height="14" fill="currentColor" stroke="none"/>)");
    char const* const STOP = ICON_SVG(
        R"(<rect x="6" y="6" width="12" height="12" fill="currentColor" stroke="none"/>)");
    char const* const PREVIOUS = ICON_SVG(
        R"(<rect x="6" y="6" width="2" height="12" fill="currentColor" stroke="none"/>)"
        R"(<polygon points="9.5,12 18,18 18,6" fill="currentColor" stroke="none"/>)");
    char const* const NEXT = ICON_SVG(
        R"(<polygon points="6,6 6,18 14.5,12" fill="currentColor" stroke="none"/>)"
        R"(<rect x="16" y="6" width="2" height="12" fill="currentColor" stroke="none"/>)");
    char const* const REPEAT = ICON_SVG(
        R"(<polyline points="6,13 6,6 17,6"/>)"
        R"(<polygon points="17,2 21,6 17,10" fill="currentColor" stroke="none"/>)"
        R"(<polyline points="18,11 18,18 7,18"/>)"
        R"(<polygon points="7,14 3,18 7,22" fill="currentColor" stroke="none"/>)");

    #undef ICON_SVG

    ng::image make(std::string const& aName, char const* aSvg, ng::color const& aColor, ng::dimension aDpiScaleFactor)
    {
        // images are loaded from resources: each icon is added as one in each colour it is wanted in
        auto const hex = std::format("{:02X}{:02X}{:02X}",
            static_cast<unsigned>(aColor.red()), static_cast<unsigned>(aColor.green()), static_cast<unsigned>(aColor.blue()));
        std::string svg{ aSvg };
        for (auto at = svg.find("currentColor"); at != std::string::npos; at = svg.find("currentColor", at))
            svg.replace(at, std::string_view{ "currentColor" }.size(), "#" + hex);
        std::string const uri = ":/mod_tracker/icons/" + aName + "_" + hex + ".svg";
        ng::service<ng::i_resource_manager>().add_resource(uri, svg.data(), svg.size());
        return ng::image{ ng::string{ uri }, aDpiScaleFactor, ng::texture_sampling::Scaled };
    }
}
