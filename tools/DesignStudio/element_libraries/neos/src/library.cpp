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
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/utility.hpp>
#include <neogfx/tools/DesignStudio/i_project.hpp>
#include <neogfx/tools/DesignStudio/element.hpp>
#include "library.hpp"

namespace neogfx::DesignStudio
{
    neos_element_library::neos_element_library(neolib::i_application& aApplication, std::string const& aPluginPath) :
        iApplication{ aApplication },
        iPluginPath{ aPluginPath },
        iElementsOrdered
        {
        },
        iElements{ elements_ordered().begin(), elements_ordered().end() },
        iSeparator{},
        iNewScript{ "New Script..."_t },
        iStartDebugging{ "Start Debugging"_t },
        iStartWithoutDebugging{ "Start Without Debugging"_t },
        iStepInto{ "Step Into"_t },
        iStepOver{ "Step Over"_t },
        iToggleBreakpoint{ "Toggle Breakpoint"_t },
        iNewBreakpoint{ "New Breakpoint"_t },
        iDeleteAllBreakpoints{ "Delete All Breakpoints"_t },
        iDisableAllBreakpoints{ "Disable All Breakpoints"_t },
        iImportScriptingLanguage{ "Import Scripting Language..."_t },
        iEditScriptingLanguage{ "Edit Scripting Language..."_t },
        iScriptMenu{ "Script"_t }
    {
        iSeparator.set_group(library_id());
        iNewScript.set_group(library_id());
        iStartDebugging.set_group(library_id()).set_shortcut("F5");
        iStartWithoutDebugging.set_group(library_id()).set_shortcut("Ctrl+F5");
        iStepInto.set_group(library_id()).set_shortcut("F11");
        iStepOver.set_group(library_id()).set_shortcut("F10");
        iToggleBreakpoint.set_group(library_id()).set_shortcut("F9");
        iNewBreakpoint.set_group(library_id());
        iDeleteAllBreakpoints.set_group(library_id());
        iDisableAllBreakpoints.set_group(library_id());
        iImportScriptingLanguage.set_group(library_id());
        iEditScriptingLanguage.set_group(library_id());

        iScriptMenu.set_group(library_id());
        iScriptMenu.add_action(iNewScript);
        iScriptMenu.add_action(iSeparator);
        iScriptMenu.add_action(iStartDebugging);
        iScriptMenu.add_action(iStartWithoutDebugging);
        iScriptMenu.add_action(iSeparator);
        iScriptMenu.add_action(iStepInto);
        iScriptMenu.add_action(iStepOver);
        iScriptMenu.add_action(iSeparator);
        iScriptMenu.add_action(iToggleBreakpoint);
        iScriptMenu.add_action(iNewBreakpoint);
        iScriptMenu.add_action(iDeleteAllBreakpoints);
        iScriptMenu.add_action(iDisableAllBreakpoints);
        iScriptMenu.add_action(iSeparator);
        iScriptMenu.add_action(iImportScriptingLanguage);
        iScriptMenu.add_action(iEditScriptingLanguage);
    }

    neos_element_library::~neos_element_library()
    {
    }

    neolib::i_application& neos_element_library::application() const
    {
        return iApplication;
    }

    void neos_element_library::ide_ready(i_ide& aIde)
    {
        aIde.add_sub_menu(id::MenuBar, iScriptMenu);
    }

    const neos_element_library::elements_t& neos_element_library::elements() const
    {
        return iElements;
    }

    const neos_element_library::elements_ordered_t& neos_element_library::elements_ordered() const
    {
        return iElementsOrdered;
    }

    void neos_element_library::create_element(i_project& aProject, const neolib::i_string& aElementType, const neolib::i_string& aElementId, neolib::i_ref_ptr<i_element>& aResult)
    {
        static const std::map<std::string, std::function<i_element* (i_project& aProject, const neolib::i_string&)>> sFactoryMethods =
        {
            #define MAKE_ROOT_ELEMENT_FACTORY_FUNCTION(Type) { #Type, [this](i_project& aProject, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aProject, #Type, aElementId }; } },
            #define MAKE_NAMED_ROOT_ELEMENT_FACTORY_FUNCTION(Name, Type) { #Name, [this](i_project& aProject, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aProject, #Name, aElementId }; } },
            MAKE_NAMED_ROOT_ELEMENT_FACTORY_FUNCTION(project, i_project)
        };
        auto method = sFactoryMethods.find(aElementType);
        if (method != sFactoryMethods.end())
        {
            aResult.reset((method->second)(aProject, aElementId));
            return;
        }
        throw unknown_element_type();
    }

    void neos_element_library::create_element(i_element& aParent, const neolib::i_string& aElementType, const neolib::i_string& aElementId, neolib::i_ref_ptr<i_element>& aResult)
    {
        static const std::map<std::string, std::function<i_element*(i_element&, const neolib::i_string&, const neolib::i_string&)>> sFactoryMethods =
        {
            #define MAKE_ELEMENT_FACTORY_FUNCTION(Type) { #Type, [this](i_element& aParent, const neolib::i_string& aElementType, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aParent, #Type, aElementId, element_group(aElementType) }; } },
            #define MAKE_NAMED_ELEMENT_FACTORY_FUNCTION(Name, Type) { #Name, [this](i_element& aParent, const neolib::i_string& aElementType, const neolib::i_string& aElementId) -> i_element* { return new element<Type>{ *this, aParent, #Name, aElementId, element_group(aElementType) }; } },
        };
        auto method = sFactoryMethods.find(aElementType);
        if (method != sFactoryMethods.end())
        {
            aResult.reset((method->second)(aParent, aElementType, aElementId));
            return;
        }
        throw unknown_element_type();
    }

    element_group neos_element_library::element_group(const neolib::i_string& aElementType) const
    {
        static const std::map<std::string, DesignStudio::element_group> sElementGroups =
        {
        };
        auto group = sElementGroups.find(aElementType);
        if (group != sElementGroups.end())
        {
            return group->second;
        }
        throw unknown_element_type();
    }

    i_texture const& neos_element_library::element_icon(const neolib::i_string& aElementType) const
    {
        auto& icons = iIcons[service<i_app>().current_style().palette().color(color_role::Text)];
        static std::map<std::string, std::function<void(texture&)>> sIconResources =
        {
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
