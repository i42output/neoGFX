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
        iElements{ elements_ordered().begin(), elements_ordered().end() }
    {
    }

    neos_element_library::~neos_element_library()
    {
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
