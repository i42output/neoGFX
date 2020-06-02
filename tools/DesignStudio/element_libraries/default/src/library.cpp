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

#include <neogfx/neogfx.hpp>
#include <boost/range/iterator_range.hpp>
#include <neolib/core/string_ci.hpp>
#include "library.hpp"

namespace neogfx::DesignStudio
{
    default_element_library::default_element_library(neolib::i_application& aApplication, const std::string& aPluginPath) :
        iApplication{ aApplication },
        iPluginPath{ aPluginPath },
        iRootElements
        {
            { "app" },
        },
        iElements
        {
            { "action" },
            { "menu" },
            { "window" },
            { "widget" },
            { "text_widget" },
            { "image_widget" },
            { "menu_bar" },
            { "toolbar" },
            { "status_bar" },
            { "tab_page_container" },
            { "tab_page" },
            { "canvas" },
            { "push_button" },
            { "check_box" },
            { "radio_button" },
            { "label" },
            { "text_edit" },
            { "line_edit" },
            { "text_field" },
            { "drop_list" },
            { "table_view" },
            { "tree_view" },
            { "group_box" },
            { "slider" },
            { "double_slider" },
            { "spin_box" },
            { "double_spin_box" },
            { "gradient_widget" },
            { "vertical_layout" },
            { "horizontal_layout" },
            { "grid_layout" },
            { "flow_layout" },
            { "stack_layout" },
            { "border_layout" },
            { "spacer" },
            { "vertical_spacer" },
            { "horizontal_spacer" }
        }
    {
    }

    const default_element_library::elements_t& default_element_library::root_elements() const
    {
        return iRootElements;
    }

    const default_element_library::elements_t& default_element_library::elements() const
    {
        return iElements;
    }

    void default_element_library::create_element(const neolib::i_string& aElementType, neolib::i_ref_ptr<i_element>& aResult)
    {
#if 0
        static const std::map<std::string, std::function<i_element*()>> sFactoryMethods =
        {
            { "app", []() -> i_element* { return new app{}; } }
        };
        auto method = sFactoryMethods.find(aElementType);
        if (method != sFactoryMethods.end())
        {
            aResult.reset((method->second)());
            return;
        }
        throw unknown_element_type();
#endif
    }

    void default_element_library::create_element(i_element& aParent, const neolib::i_string& aElementType, neolib::i_ref_ptr<i_element>& aResult)
    {
#if 0
        static const std::map<std::string, std::function<i_element*(i_element&)>> sFactoryMethods =
        {
            { "action", [](i_element& aParent) -> i_element* { return new action{ aParent }; } },
            { "window", [](i_element& aParent) -> i_element* { return new window{ aParent }; } },
            { "widget", [](i_element& aParent) -> i_element* { return new widget{ aParent }; } },
            { "text_widget", [](i_element& aParent) -> i_element* { return new text_widget{ aParent }; } },
            { "image_widget", [](i_element& aParent) -> i_element* { return new image_widget{ aParent }; } },
            { "status_bar", [](i_element& aParent) -> i_element* { return new status_bar{ aParent }; } },
            { "menu_bar", [](i_element& aParent) -> i_element* { return new menu_bar{ aParent }; } },
            { "menu", [](i_element& aParent) -> i_element* { return new menu{ aParent }; } },
            { "toolbar", [](i_element& aParent) -> i_element* { return new toolbar{ aParent }; } },
            { "tab_page_container", [](i_element& aParent) -> i_element* { return new tab_page_container{ aParent }; } },
            { "tab_page", [](i_element& aParent) -> i_element* { return new tab_page{ aParent }; } },
            { "canvas", [](i_element& aParent) -> i_element* { return new canvas{ aParent }; } },
            { "push_button", [](i_element& aParent) -> i_element* { return new push_button{ aParent }; } },
            { "check_box", [](i_element& aParent) -> i_element* { return new check_box{ aParent }; } },
            { "radio_button", [](i_element& aParent) -> i_element* { return new radio_button{ aParent }; } },
            { "label", [](i_element& aParent) -> i_element* { return new label{ aParent }; } },
            { "text_edit", [](i_element& aParent) -> i_element* { return new text_edit{ aParent }; } },
            { "line_edit", [](i_element& aParent) -> i_element* { return new line_edit{ aParent }; } },
            { "text_field", [](i_element& aParent) -> i_element* { return new text_field{ aParent }; } },
            { "drop_list", [](i_element& aParent) -> i_element* { return new drop_list{ aParent }; } },
            { "table_view", [](i_element& aParent) -> i_element* { return new table_view{ aParent }; } },
            { "tree_view", [](i_element& aParent) -> i_element* { return new tree_view{ aParent }; } },
            { "group_box", [](i_element& aParent) -> i_element* { return new group_box{ aParent }; } },
            { "slider", [](i_element& aParent) -> i_element* { return new slider{ aParent }; } },
            { "double_slider", [](i_element& aParent) -> i_element* { return new double_slider{ aParent }; } },
            { "spin_box", [](i_element& aParent) -> i_element* { return new spin_box{ aParent }; } },
            { "double_spin_box", [](i_element& aParent) -> i_element* { return new double_spin_box{ aParent }; } },
            { "gradient_widget", [](i_element& aParent) -> i_element* { return new gradient_widget{ aParent }; } },
            { "vertical_layout", [](i_element& aParent) -> i_element* { return new vertical_layout{ aParent }; } },
            { "horizontal_layout", [](i_element& aParent) -> i_element* { return new horizontal_layout{ aParent }; } },
            { "grid_layout", [](i_element& aParent) -> i_element* { return new grid_layout{ aParent }; } },
            { "flow_layout", [](i_element& aParent) -> i_element* { return new flow_layout{ aParent }; } },
            { "stack_layout", [](i_element& aParent) -> i_element* { return new stack_layout{ aParent }; } },
            { "border_layout", [](i_element& aParent) -> i_element* { return new border_layout{ aParent }; } },
            { "spacer", [](i_element& aParent) -> i_element* { return new spacer{ aParent }; } },
            { "vertical_spacer", [](i_element& aParent) -> i_element* { return new vertical_spacer{ aParent }; } },
            { "horizontal_spacer", [](i_element& aParent) -> i_element* { return new horizontal_spacer{ aParent }; } }
        };
        auto method = sFactoryMethods.find(aElementType);
        if (method != sFactoryMethods.end())
        {
            aResult.reset((method->second)(aParent));
            return;
        }
        throw unknown_element_type();
#endif
    }
}
