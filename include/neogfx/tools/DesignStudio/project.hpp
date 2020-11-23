// project.hpp
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

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/gui/view/model.hpp>
#include <neogfx/tools/DesignStudio/i_project.hpp>
#include <neogfx/tools/DesignStudio/element.hpp>

namespace neogfx::DesignStudio
{
    class i_project_manager;

    class project : public reference_counted<model<i_project>>
    {
    public:
        define_declared_event(ElementAdded, element_added, i_element&)
        define_declared_event(ElementRemoved, element_removed, i_element&)
    public:
        typedef i_project abstract_type;
    public:
        project(i_project_manager& aManager);
    public:
        i_project_manager& manager() const override;
    public:
        void create(const i_string& aName, const i_string& aNamespace) override;
        void open(const i_string& aPath) override;
    public:
        const i_string& name() const override;
        const i_string& namespace_() const override;
    public:
        const i_element& root() const override;
        i_element& root() override;
    public:
        i_element& create_element(i_element& aParent, const i_string& aType, const i_string& aElementId = string{}) override;
        void remove_element(i_element& aElement) override;
    private:
        i_project_manager& iManager;
        ref_ptr<i_element> iRoot;
        string iName;
        string iNamespace;
    };
}