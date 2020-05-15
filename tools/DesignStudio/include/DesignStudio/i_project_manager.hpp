// i_project_manager.hpp
/*
  neogfx C++ App/Game Engine
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

#include <DesignStudio/DesignStudio.hpp>
#include <neolib/i_vector.hpp>
#include <neogfx/core/i_event.hpp>
#include <DesignStudio/i_project.hpp>

namespace design_studio
{
    class i_project_manager
    {
    public:
        declare_event(project_added, i_project&)
        declare_event(project_removed, i_project&)
        declare_event(project_activated, i_project&)
        declare_event(project_deactivated, i_project&)
    public:
        struct project_not_found : std::logic_error { project_not_found() : std::logic_error{ "design_studio::i_project_manager::project_not_found" } {} };
        struct no_active_project : std::logic_error { no_active_project() : std::logic_error{ "design_studio::i_project_manager::no_active_project" } {} };
    public:
        using project_list = neolib::i_vector<ng::i_ref_ptr<i_project>>;
    public:
        virtual ~i_project_manager() = default;
        // interface
    public:
        virtual const project_list& projects() const = 0;
        virtual bool project_active() const = 0;
        virtual i_project& active_project() const = 0;
        virtual void activate_project(const i_project& aProject) = 0;
        virtual void deactivate_project() = 0;
        virtual i_project& create_project(const ng::i_string& aProjectName, const ng::i_string& aProjectNamespace) = 0;
        virtual void close_project(const i_project& aProject) = 0;
        // helpers
    public:
        i_project& create_project(const std::string& aProjectName, const std::string& aProjectNamespace)
        {
            return create_project(ng::string{ aProjectName }, ng::string{ aProjectNamespace });
        }
    };
}