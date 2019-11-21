// library.hpp
/*
neoGFX Resource Compiler
Copyright(C) 2019 Leigh Johnston

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
#include "library.hpp"
#include "app.hpp"
#include "action.hpp"
#include "window.hpp"
#include "status_bar.hpp"
#include "menu_bar.hpp"
#include "menu.hpp"
#include "toolbar.hpp"
#include "tab_page_container.hpp"
#include "tab_page.hpp"
#include "toolbar.hpp"
#include "layout.hpp"

namespace neogfx::nrc
{
    default_ui_element_library::default_ui_element_library(neolib::i_application& aApplication, const std::string& aPluginPath) :
        iApplication{ aApplication },
        iPluginPath{ aPluginPath },
        iRootElements{
            "app" },
        iChildElements{
            "action", "window", "status_bar", "menu_bar", "menu", "toolbar", "tab_page_container", "tab_page", 
            "vertical_layout", "horizontal_layout", "grid_layout", "flow_layout", "stack_layout", "border_layout" }
    {
    }

    bool default_ui_element_library::handles_element(const neolib::i_string& aElementType) const
    {
        return iRootElements.find(aElementType) != iRootElements.end();
    }

    bool default_ui_element_library::handles_element(i_ui_element& aParent, const neolib::i_string& aElementType) const
    {
        // todo: this logic is too rudimentary for extensibility; check on type category instead.
        return (aParent.type() & ui_element_type::MASK_RESERVED) != ui_element_type::Invalid && iChildElements.find(aElementType) != iChildElements.end();
    }

    i_ui_element* default_ui_element_library::do_create_element(const i_ui_element_parser& aParser, const neolib::i_string& aElementType)
    {
        static const std::map<std::string, std::function<i_ui_element*(const i_ui_element_library&, const i_ui_element_parser&)>> sFactoryMethods =
        {
            { "app", [](const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser) -> i_ui_element* { return new app{ aLibrary, aParser }; } }
        };
        auto method = sFactoryMethods.find(aElementType);
        if (method != sFactoryMethods.end())
            return (method->second)(*this, aParser);
        throw unknown_element_type();
    }

    i_ui_element* default_ui_element_library::do_create_element(const i_ui_element_parser& aParser, i_ui_element& aParent, const neolib::i_string& aElementType)
    {
        static const std::map<std::string, std::function<i_ui_element*(const i_ui_element_library&, const i_ui_element_parser&, i_ui_element&)>> sFactoryMethods =
        {
            { "action", [](const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new action{ aLibrary, aParser, aParent }; } },
            { "window", [](const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new window{ aLibrary, aParser, aParent }; } },
            { "status_bar", [](const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new status_bar{ aLibrary, aParser, aParent }; } },
            { "menu_bar", [](const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new menu_bar{ aLibrary, aParser, aParent }; } },
            { "menu", [](const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new menu{ aLibrary, aParser, aParent }; } },
            { "toolbar", [](const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new toolbar{ aLibrary, aParser, aParent }; } },
            { "tab_page_container", [](const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new tab_page_container{ aLibrary, aParser, aParent }; } },
            { "tab_page", [](const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new tab_page{ aLibrary, aParser, aParent }; } },
            { "vertical_layout", [](const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new vertical_layout{ aLibrary, aParser, aParent }; } },
            { "horizontal_layout", [](const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new horizontal_layout{ aLibrary, aParser, aParent }; } },
            { "grid_layout", [](const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new grid_layout{ aLibrary, aParser, aParent }; } },
            { "flow_layout", [](const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new flow_layout{ aLibrary, aParser, aParent }; } },
            { "stack_layout", [](const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new stack_layout{ aLibrary, aParser, aParent }; } },
            { "border_layout", [](const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new border_layout{ aLibrary, aParser, aParent }; } }
        };
        auto method = sFactoryMethods.find(aElementType);
        if (method != sFactoryMethods.end())
            return (method->second)(*this, aParser, aParent);
        throw unknown_element_type();
    }
}
