// project_manager.hpp
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
#include <neolib/core/vector.hpp>
#include <neogfx/tools/DesignStudio/i_project_manager.hpp>
#include <neogfx/tools/DesignStudio/project.hpp>

namespace neogfx::DesignStudio
{
    class project_manager : public i_project_manager
    {
    public:
        define_declared_event(ProjectAdded, project_added, i_project&)
        define_declared_event(ProjectRemoved, project_removed, i_project&)
        define_declared_event(ProjectActivated, project_activated, i_project&)
        define_declared_event(ProjectDeactivated, project_deactivated, i_project&)
    public:
        using project_list = neolib::vector<ng::ref_ptr<project>>;
    public:
        project_manager();
        ~project_manager();
    public:
        const project_list& projects() const override;
        bool project_active() const override;
        i_project& active_project() const override;
        void activate_project(const i_project& aProject) override;
        void deactivate_project() override;
        i_project& open_project(const ng::i_string& aProjectFile) override;
        i_project& create_project(const ng::i_string& aProjectName, const ng::i_string& aProjectNamespace) override;
        void close_project(const i_project& aProject) override;
    public:
        i_element_library& library(const ng::i_string& aElementType) const override;
    public:
        using i_project_manager::open_project;
        using i_project_manager::create_project;
    private:
        project_list::std_type::const_iterator find_project(const i_project& aProject) const;
        ref_ptr<i_element_library> find_library(std::string const& aElementType) const;
    private:
        project_list iProjects;
        ng::ref_ptr<project> iActiveProject;
        std::vector<ref_ptr<i_element_library>> iElementLibraries;
    };
}