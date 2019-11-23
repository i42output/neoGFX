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
#include <boost/range/iterator_range.hpp>
#include "library.hpp"
#include "app.hpp"
#include "action.hpp"
#include "window.hpp"
#include "menu.hpp"
#include "toolbar.hpp"
#include "menu_bar.hpp"
#include "toolbar.hpp"
#include "status_bar.hpp"
#include "tab_page_container.hpp"
#include "tab_page.hpp"
#include "button.hpp"
#include "group_box.hpp"
#include "layout.hpp"

namespace neogfx::nrc
{
    default_ui_element_library::default_ui_element_library(neolib::i_application& aApplication, const std::string& aPluginPath) :
        iApplication{ aApplication },
        iPluginPath{ aPluginPath },
        iRootElements{
            "app" 
        },
        iChildElements{
            { "action", ui_element_type::HasActions },
            { "menu", ui_element_type::Menu },
            { "window", ui_element_type::None },
            { "menu_bar", ui_element_type::LayoutItem },
            { "toolbar", ui_element_type::LayoutItem },
            { "status_bar", ui_element_type::LayoutItem },
            { "tab_page_container", ui_element_type::LayoutItem },
            { "tab_page", ui_element_type::LayoutItem },
            { "push_button", ui_element_type::LayoutItem },
            { "check_box", ui_element_type::LayoutItem },
            { "radio_button", ui_element_type::LayoutItem },
            { "group_box", ui_element_type::LayoutItem },
            { "vertical_layout", ui_element_type::LayoutItem },
            { "horizontal_layout", ui_element_type::LayoutItem },
            { "grid_layout", ui_element_type::LayoutItem },
            { "flow_layout", ui_element_type::LayoutItem },
            { "stack_layout", ui_element_type::LayoutItem },
            { "border_layout", ui_element_type::LayoutItem }
        }
    {
    }

    bool default_ui_element_library::handles_element(const neolib::i_string& aElementType) const
    {
        return iRootElements.find(aElementType) != iRootElements.end();
    }

    bool default_ui_element_library::handles_element(i_ui_element& aParent, const neolib::i_string& aElementType) const
    {
        auto range = boost::make_iterator_range(iChildElements.container().equal_range(aElementType.to_std_string()));
        for (auto const& ce : range)
        {
            if (ce.second.second() == ui_element_type::None)
                return true;
            else if ((ce.second.second() & aParent.type()) != ui_element_type::None)
                return true;
        }
        return false;
    }

    i_ui_element* default_ui_element_library::do_create_element(const i_ui_element_parser& aParser, const neolib::i_string& aElementType)
    {
        static const std::map<std::string, std::function<i_ui_element*(const i_ui_element_parser&)>> sFactoryMethods =
        {
            { "app", [](const i_ui_element_parser& aParser) -> i_ui_element* { return new app{ aParser }; } }
        };
        auto method = sFactoryMethods.find(aElementType);
        if (method != sFactoryMethods.end())
            return (method->second)(aParser);
        throw unknown_element_type();
    }

    i_ui_element* default_ui_element_library::do_create_element(const i_ui_element_parser& aParser, i_ui_element& aParent, const neolib::i_string& aElementType)
    {
        static const std::map<std::string, std::function<i_ui_element*(const i_ui_element_parser&, i_ui_element&)>> sFactoryMethods =
        {
            { "action", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new action{ aParser, aParent }; } },
            { "window", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new window{ aParser, aParent }; } },
            { "status_bar", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new status_bar{ aParser, aParent }; } },
            { "menu_bar", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new menu_bar{ aParser, aParent }; } },
            { "menu", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new menu{ aParser, aParent }; } },
            { "toolbar", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new toolbar{ aParser, aParent }; } },
            { "tab_page_container", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new tab_page_container{ aParser, aParent }; } },
            { "tab_page", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new tab_page{ aParser, aParent }; } },
            { "push_button", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new push_button{ aParser, aParent }; } },
            { "check_box", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new check_box{ aParser, aParent }; } },
            { "radio_button", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new radio_button{ aParser, aParent }; } },
            { "group_box", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new group_box{ aParser, aParent }; } },
            { "vertical_layout", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new vertical_layout{ aParser, aParent }; } },
            { "horizontal_layout", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new horizontal_layout{ aParser, aParent }; } },
            { "grid_layout", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new grid_layout{ aParser, aParent }; } },
            { "flow_layout", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new flow_layout{ aParser, aParent }; } },
            { "stack_layout", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new stack_layout{ aParser, aParent }; } },
            { "border_layout", [](const i_ui_element_parser& aParser, i_ui_element& aParent) -> i_ui_element* { return new border_layout{ aParser, aParent }; } }
        };
        auto method = sFactoryMethods.find(aElementType);
        if (method != sFactoryMethods.end())
            return (method->second)(aParser, aParent);
        throw unknown_element_type();
    }
}
