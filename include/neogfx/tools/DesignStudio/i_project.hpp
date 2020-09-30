// i_project.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2020 Leigh Johnston
  
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

#include <neogfx/core/i_event.hpp>
#include <neogfx/gui/view/i_model.hpp>
#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>

namespace neogfx::DesignStudio
{
    class i_project_manager;

    class i_project : public ng::i_model, public ng::i_reference_counted
    {
    public:
        declare_event(element_added, i_element&);
    public:
        struct invalid_project_file : std::runtime_error { invalid_project_file(const std::string& aReason) : std::runtime_error{ "neogfx::DesignStudio::i_project::invalid_project_file: " + aReason } {} };
    public:
        typedef i_project abstract_type;
    public:
        virtual i_project_manager& manager() const = 0;
    public:
        virtual void create(const i_string& aName, const i_string& aNamespace) = 0;
        virtual void open(const i_string& aPath) = 0;
    public:
        virtual const i_string& name() const = 0;
        virtual const i_string& namespace_() const = 0;
    public:
        virtual const i_element& root() const = 0;
        virtual i_element& root() = 0;
    public:
        virtual i_element& create_element(i_element& aParent, const i_string& aType, const i_string& aElementId = string{}) = 0;
    };
}