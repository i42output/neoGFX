// library.cpp
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
#include <neogfx/app/app.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/widget/text_widget.hpp>
#include <neogfx/gui/widget/image_widget.hpp>
#include <neogfx/gui/widget/menu_bar.hpp>
#include <neogfx/gui/widget/toolbar.hpp>
#include <neogfx/gui/widget/status_bar.hpp>
#include <neogfx/gui/widget/tab_page_container.hpp>
#include <neogfx/gui/widget/tab_page.hpp>
#include <neogfx/game/canvas.hpp>
#include <neogfx/gui/widget/push_button.hpp>
#include <neogfx/gui/widget/check_box.hpp>
#include <neogfx/gui/widget/radio_button.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/text_edit.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/widget/text_field.hpp>
#include <neogfx/gui/widget/drop_list.hpp>
#include <neogfx/gui/widget/table_view.hpp>
#include <neogfx/gui/widget/tree_view.hpp>
#include <neogfx/gui/widget/group_box.hpp>
#include <neogfx/gui/widget/slider.hpp>
#include <neogfx/gui/widget/spin_box.hpp>
#include <neogfx/gui/widget/gradient_widget.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/grid_layout.hpp>
#include <neogfx/gui/layout/flow_layout.hpp>
#include <neogfx/gui/layout/stack_layout.hpp>
#include <neogfx/gui/layout/border_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/utility.hpp>
#include <neogfx/tools/DesignStudio/i_project.hpp>
#include <neogfx/tools/DesignStudio/element.hpp>
#include "library.hpp"

namespace neogfx::DesignStudio
{
    default_element_library::default_element_library(neolib::i_application& aApplication, std::string const& aPluginPath) :
        iApplication{ aApplication },
        iPluginPath{ aPluginPath },
        iElementsOrdered
        {
            { "project" },
            { "code" },
            { "script" },
            { "node" },
            { "user_interface" },
            { "app" },
            { "MVC_app" },
            { "dialog_app" },
            { "2D_game" },
            { "2.5D_game" },
            { "3D_game" },
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
        },
        iElements{ elements_ordered().begin(), elements_ordered().end() }
    {
    }

    default_element_library::~default_element_library()
    {
    }

    const default_element_library::elements_t& default_element_library::elements() const
    {
        return iElements;
    }

    const default_element_library::elements_ordered_t& default_element_library::elements_ordered() const
    {
        return iElementsOrdered;
    }

    void default_element_library::create_element(const neolib::i_string& aElementType, const neolib::i_string& aElementId, neolib::i_ref_ptr<i_element>& aResult)
    {
        static const std::map<std::string, std::function<i_element* (const neolib::i_string&)>> sFactoryMethods =
        {
            #define MAKE_ROOT_ELEMENT_FACTORY_FUNCTION(Type) { #Type, [this](const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, #Type, aElementId }; } },
            #define MAKE_NAMED_ROOT_ELEMENT_FACTORY_FUNCTION(Name, Type) { #Name, [this](const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, #Name, aElementId }; } },
            MAKE_NAMED_ROOT_ELEMENT_FACTORY_FUNCTION(project, i_project)
        };
        auto method = sFactoryMethods.find(aElementType);
        if (method != sFactoryMethods.end())
        {
            aResult.reset((method->second)(aElementId));
            return;
        }
        throw unknown_element_type();
    }

    void default_element_library::create_element(i_element& aParent, const neolib::i_string& aElementType, const neolib::i_string& aElementId, neolib::i_ref_ptr<i_element>& aResult)
    {
        static const std::map<std::string, std::function<i_element*(i_element&, const neolib::i_string&)>> sFactoryMethods =
        {
            #define MAKE_ELEMENT_FACTORY_FUNCTION(Type) { #Type, [this](i_element& aParent, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aParent, #Type, aElementId }; } },
            #define MAKE_NAMED_ELEMENT_FACTORY_FUNCTION(Name, Type) { #Name, [this](i_element& aParent, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aParent, #Name, aElementId }; } },
            MAKE_ELEMENT_FACTORY_FUNCTION(user_interface)
            MAKE_NAMED_ELEMENT_FACTORY_FUNCTION(app, app)
            MAKE_NAMED_ELEMENT_FACTORY_FUNCTION(MVC_app, app)
            MAKE_NAMED_ELEMENT_FACTORY_FUNCTION(dialog_app, app)
            MAKE_NAMED_ELEMENT_FACTORY_FUNCTION(2D_game, app)
            MAKE_NAMED_ELEMENT_FACTORY_FUNCTION(2.5D_game, app)
            MAKE_NAMED_ELEMENT_FACTORY_FUNCTION(3D_game, app)
            MAKE_ELEMENT_FACTORY_FUNCTION(action)
            MAKE_ELEMENT_FACTORY_FUNCTION(window)
            MAKE_ELEMENT_FACTORY_FUNCTION(widget)
            MAKE_ELEMENT_FACTORY_FUNCTION(text_widget)
            MAKE_ELEMENT_FACTORY_FUNCTION(image_widget)
            MAKE_ELEMENT_FACTORY_FUNCTION(status_bar)
            MAKE_ELEMENT_FACTORY_FUNCTION(menu_bar)
            MAKE_ELEMENT_FACTORY_FUNCTION(menu)
            MAKE_ELEMENT_FACTORY_FUNCTION(toolbar)
            MAKE_ELEMENT_FACTORY_FUNCTION(tab_page_container)
            MAKE_ELEMENT_FACTORY_FUNCTION(tab_page)
            MAKE_ELEMENT_FACTORY_FUNCTION(canvas)
            MAKE_ELEMENT_FACTORY_FUNCTION(push_button)
            MAKE_ELEMENT_FACTORY_FUNCTION(check_box)
            MAKE_ELEMENT_FACTORY_FUNCTION(radio_button)
            MAKE_ELEMENT_FACTORY_FUNCTION(label)
            MAKE_ELEMENT_FACTORY_FUNCTION(text_edit)
            MAKE_ELEMENT_FACTORY_FUNCTION(line_edit)
            MAKE_ELEMENT_FACTORY_FUNCTION(text_field)
            MAKE_ELEMENT_FACTORY_FUNCTION(drop_list)
            MAKE_ELEMENT_FACTORY_FUNCTION(table_view)
            MAKE_ELEMENT_FACTORY_FUNCTION(tree_view)
            MAKE_ELEMENT_FACTORY_FUNCTION(group_box)
            MAKE_ELEMENT_FACTORY_FUNCTION(slider)
            MAKE_ELEMENT_FACTORY_FUNCTION(double_slider)
            MAKE_ELEMENT_FACTORY_FUNCTION(spin_box)
            MAKE_ELEMENT_FACTORY_FUNCTION(double_spin_box)
            MAKE_ELEMENT_FACTORY_FUNCTION(gradient_widget)
            MAKE_ELEMENT_FACTORY_FUNCTION(vertical_layout)
            MAKE_ELEMENT_FACTORY_FUNCTION(horizontal_layout)
            MAKE_ELEMENT_FACTORY_FUNCTION(grid_layout)
            MAKE_ELEMENT_FACTORY_FUNCTION(flow_layout)
            MAKE_ELEMENT_FACTORY_FUNCTION(stack_layout)
            MAKE_ELEMENT_FACTORY_FUNCTION(border_layout)
            MAKE_ELEMENT_FACTORY_FUNCTION(spacer)
            MAKE_ELEMENT_FACTORY_FUNCTION(vertical_spacer)
            MAKE_ELEMENT_FACTORY_FUNCTION(horizontal_spacer)
        };
        auto method = sFactoryMethods.find(aElementType);
        if (method != sFactoryMethods.end())
        {
            aResult.reset((method->second)(aParent, aElementId));
            return;
        }
        throw unknown_element_type();
    }

    element_group default_element_library::element_group(const neolib::i_string& aElementType) const
    {
        static const std::map<std::string, DesignStudio::element_group> sElementGroups =
        {
            { "project", DesignStudio::element_group::Project },
            { "code", DesignStudio::element_group::Code },
            { "script", DesignStudio::element_group::Script },
            { "node", DesignStudio::element_group::Node },
            { "user_interface", DesignStudio::element_group::UserInterface },
            { "app", DesignStudio::element_group::App },
            { "MVC_app", DesignStudio::element_group::App },
            { "dialog_app", DesignStudio::element_group::App },
            { "2D_game", DesignStudio::element_group::App },
            { "2.5D_game", DesignStudio::element_group::App },
            { "3D_game", DesignStudio::element_group::App },
            { "action", DesignStudio::element_group::Action },
            { "window", DesignStudio::element_group::Widget },
            { "widget", DesignStudio::element_group::Widget },
            { "text_widget", DesignStudio::element_group::Widget },
            { "image_widget", DesignStudio::element_group::Widget },
            { "status_bar", DesignStudio::element_group::Widget },
            { "menu_bar", DesignStudio::element_group::Menu },
            { "menu", DesignStudio::element_group::Menu },
            { "toolbar", DesignStudio::element_group::Widget },
            { "tab_page_container", DesignStudio::element_group::Widget },
            { "tab_page", DesignStudio::element_group::Widget },
            { "canvas", DesignStudio::element_group::Widget },
            { "push_button", DesignStudio::element_group::Widget },
            { "check_box", DesignStudio::element_group::Widget },
            { "radio_button", DesignStudio::element_group::Widget },
            { "label", DesignStudio::element_group::Widget },
            { "text_edit", DesignStudio::element_group::Widget },
            { "line_edit", DesignStudio::element_group::Widget },
            { "text_field", DesignStudio::element_group::Widget },
            { "drop_list", DesignStudio::element_group::Widget },
            { "table_view", DesignStudio::element_group::Widget },
            { "tree_view", DesignStudio::element_group::Widget },
            { "group_box", DesignStudio::element_group::Widget },
            { "slider", DesignStudio::element_group::Widget },
            { "double_slider", DesignStudio::element_group::Widget },
            { "spin_box", DesignStudio::element_group::Widget },
            { "double_spin_box", DesignStudio::element_group::Widget },
            { "gradient_widget", DesignStudio::element_group::Widget },
            { "vertical_layout", DesignStudio::element_group::Layout },
            { "horizontal_layout", DesignStudio::element_group::Layout },
            { "grid_layout", DesignStudio::element_group::Layout },
            { "flow_layout", DesignStudio::element_group::Layout },
            { "stack_layout", DesignStudio::element_group::Layout },
            { "border_layout", DesignStudio::element_group::Layout },
            { "spacer", DesignStudio::element_group::Layout },
            { "vertical_spacer", DesignStudio::element_group::Layout },
            { "horizontal_spacer", DesignStudio::element_group::Layout }
        };
        auto group = sElementGroups.find(aElementType);
        if (group != sElementGroups.end())
        {
            return group->second;
        }
        throw unknown_element_type();
    }

    i_texture const& default_element_library::element_icon(const neolib::i_string& aElementType) const
    {
        auto& icons = iIcons[service<i_app>().current_style().palette().color(color_role::Text)];
        static std::map<std::string, std::function<void(texture&)>> sIconResources =
        {
            {
                "project",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/project.png" }, color::Coral);
                }
            },
            {
                "user_interface",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/ui.png" }, color::PaleGreen);
                }
            },
            {
                "code",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/logic.png" }, color::DeepSkyBlue);
                }
            },
            {
                "script",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/scripttext.png" }, color::DeepSkyBlue);
                }
            },
            {
                "node",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/node.png" }, color::DeepSkyBlue);
                }
            },
            {
                "app",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/app.png" }, color::BlanchedAlmond );
                }
            },
            {
                "MVC_app",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/app.png" }, color::BlanchedAlmond);
                }
            },
            {
                "dialog_app",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/app.png" }, color::BlanchedAlmond);
                }
            },
            {
                "2D_game",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/gamepad.png" }, color::BlanchedAlmond);
                }
            },
            {
                "2.5D_game",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/gamepad.png" }, color::BlanchedAlmond);
                }
            },
            {
                "3D_game",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/gamepad.png" }, color::BlanchedAlmond);
                }
            },
            {
                "action",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/action.png" });
                }
            },
            {
                "window",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/window.png" });
                }
            },
            {
                "toolbar",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/toolbar.png" });
                }
            },
            {
                "status_bar",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/statusbar.png" });
                }
            },
            {
                "canvas",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/canvas.png" });
                }
            },
            {
                "widget",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/widget.png" });
                }
            },
            {
                "menu_bar",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/menubar.png" });
                }
            },
            {
                "menu",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/menu.png" });
                }
            },
            {
                "group_box",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/group.png" });
                }
            },
            {
                "push_button",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/button.png" });
                }
            },
            {
                "check_box",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/check.png" });
                }
            },
            {
                "radio_button",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/radio.png" });
                }
            },
            {
                "label",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/label.png" });
                }
            },
            {
                "text_widget",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/text.png" });
                }
            },
            {
                "image_widget",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/image.png" });
                }
            },
            {
                "table_view",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/table.png" });
                }
            },
            {
                "tree_view",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/tree.png" });
                }
            },
            {
                "list_view",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/list.png" });
                }
            },
            {
                "drop_list",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/droplist.png" });
                }
            },
            {
                "tab_page",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/tabpage.png" });
                }
            },
            {
                "tab_page_container",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/tabpagecontainer.png" });
                }
            },
            {
                "line_edit",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/lineedit.png" });
                }
            },
            {
                "text_field",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/lineedit.png" });
                }
            },
            {
                "text_edit",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/textedit.png" });
                }
            },
            {
                "spin_box",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/spinbox.png" });
                }
            },
            {
                "double_spin_box",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/spinbox.png" });
                }
            },
            {
                "slider",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/slider.png" });
                }
            },
            {
                "double_slider",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/slider.png" });
                }
            },
            {
                "gradient_widget",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/gradient.png" });
                }
            },
            {
                "spacer",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/spacer.png" }, color::PowderBlue.lighter(0x20));
                }
            },
            {
                "vertical_spacer",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/verticalspacer.png" }, color::PowderBlue.lighter(0x20));
                }
            },
            {
                "horizontal_spacer",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/horizontalspacer.png" }, color::PowderBlue.lighter(0x20));
                }
            },
            {
                "border_layout",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/borderlayout.png" }, color::PowderBlue.lighter(0x20));
                }
            },
            {
                "flow_layout",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/flowlayout.png" }, color::PowderBlue.lighter(0x20));
                }
            },
            {
                "stack_layout",
                [](texture& aTexture)
                {
                    aTexture = colored_icon(image{ ":/neogfx/DesignStudio/default_nel/resources/stackedlayout.png" }, color::PowderBlue.lighter(0x20));
                }
            },
            {
                "vertical_layout",
                // todo: create a .png asset for this...
                [](texture& aTexture)
                {
                    texture newTexture{ size{ 128, 128 }, 1.0, ng::texture_sampling::Multisample };
                    graphics_context gc{ newTexture };
                    gc.fill_rect(rect{ point{ 4.0, 12.0 }, size{ 120.0, 24.0 } }, color::White);
                    gc.fill_rect(rect{ point{ 4.0, 12.0 + 24.0 + 16.0 }, size{ 120.0, 24.0 } }, color::White);
                    gc.fill_rect(rect{ point{ 4.0, 12.0 + 24.0 * 2 + 16.0 * 2 }, size{ 120.0, 24.0 } }, color::White);
                    aTexture = colored_icon(newTexture, color::PowderBlue.lighter(0x20));
                }
            },
            {
                "horizontal_layout",
                // todo: create a .png asset for this...
                [](texture& aTexture)
                {
                    texture newTexture{ size{ 128, 128 }, 1.0, ng::texture_sampling::Multisample };
                    graphics_context gc{ newTexture };
                    gc.fill_rect(rect{ point{ 12.0, 4.0 }, size{ 24.0, 120.0 } }, color::White);
                    gc.fill_rect(rect{ point{ 12.0 + 24.0 + 16.0, 4.0 }, size{ 24.0, 120.0 } }, color::White);
                    gc.fill_rect(rect{ point{ 12.0 + 24.0 * 2 + 16.0 * 2, 4.0 }, size{ 24.0, 120.0 } }, color::White);
                    aTexture = colored_icon(newTexture, color::PowderBlue.lighter(0x20));
                }
            },
            {
                "grid_layout",
                // todo: create a .png asset for this...
                [](texture& aTexture)
                {
                    texture newTexture{ size{ 128, 128 }, 1.0, ng::texture_sampling::Multisample };
                    graphics_context gc{ newTexture };
                    gc.fill_rect(rect{ point{ 12.0, 12.0 }, size{ 24.0, 24.0 } }, color::White);
                    gc.fill_rect(rect{ point{ 12.0 + 24.0 + 16.0, 12.0 }, size{ 24.0, 24.0 } }, color::White);
                    gc.fill_rect(rect{ point{ 12.0 + 24.0 * 2 + 16.0 * 2, 12.0 }, size{ 24.0, 24.0 } }, color::White);
                    gc.fill_rect(rect{ point{ 12.0, 12.0 + 24.0 + 16.0 }, size{ 24.0, 24.0 } }, color::White);
                    gc.fill_rect(rect{ point{ 12.0 + 24.0 + 16.0, 12.0 + 24.0 + 16.0 }, size{ 24.0, 24.0 } }, color::White);
                    gc.fill_rect(rect{ point{ 12.0 + 24.0 * 2 + 16.0 * 2, 12.0 + 24.0 + 16.0 }, size{ 24.0, 24.0 } }, color::White);
                    gc.fill_rect(rect{ point{ 12.0, 12.0 + 24.0 * 2 + 16.0 * 2 }, size{ 24.0, 24.0 } }, color::White);
                    gc.fill_rect(rect{ point{ 12.0 + 24.0 + 16.0, 12.0 + 24.0 * 2 + 16.0 * 2 }, size{ 24.0, 24.0 } }, color::White);
                    gc.fill_rect(rect{ point{ 12.0 + 24.0 * 2 + 16.0 * 2, 12.0 + 24.0 * 2 + 16.0 * 2 }, size{ 24.0, 24.0 } }, color::White);
                    aTexture = colored_icon(newTexture, color::PowderBlue.lighter(0x20));
                }
            }
        };
        auto existing = sIconResources.find(aElementType.to_std_string());
        if (existing != sIconResources.end())
        {
            if (icons.find(aElementType.to_std_string()) == icons.end())
                existing->second(icons[aElementType.to_std_string()]);
            return icons[aElementType.to_std_string()];
        }
        if (icons.find("default") == icons.end())
            icons["default"] = texture{ size{32.0, 32.0} };
        return icons["default"];
    }
}
