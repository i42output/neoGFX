// project_manager.cpp
/*
  neogfx C++ GUI Library
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

#include <DesignStudio/DesignStudio.hpp>
#include <DesignStudio/project.hpp>
#include <DesignStudio/project_manager.hpp>

namespace design_studio
{
    project_manager::project_manager()
    {
    }

    project_manager::~project_manager()
    {
        while (!iProjects.empty())
            close_project(*iProjects.back());
    }

    bool project_manager::project_active() const
    {
        return !!iActiveProject;
    }

    i_project& project_manager::active_project() const
    {
        if (project_active())
            return *iActiveProject;
        throw no_active_project();
    }

    void project_manager::activatate_project(const i_project& aProject)
    {
        if (!iActiveProject || &*iActiveProject != &aProject)
        {
            auto previouslyActive = iActiveProject;
            iActiveProject = *find_project(aProject);
            ProjectActivated.trigger(*iActiveProject);
            if (previouslyActive)
                ProjectDeactivated.trigger(*previouslyActive);
        }
    }

    i_project& project_manager::create_project(const ng::i_string& aProjectName, const ng::i_string& aProjectNamespace)
    {
        auto newProject = ng::make_ref<project>(aProjectName.to_std_string(), aProjectNamespace.to_std_string());
        iProjects.push_back(newProject);
        ProjectAdded.trigger(*newProject);
        activatate_project(*newProject);
        return *newProject;
    }

    void project_manager::close_project(const i_project& aProject)
    {
        auto removing = find_project(aProject);
        auto removingRef = *removing;
        iProjects.container().erase(removing);
        ProjectRemoved.trigger(*removingRef);
    }

    project_manager::project_list::container_type::const_iterator project_manager::find_project(const i_project& aProject) const
    {
        for (auto p = iProjects.container().begin(); p != iProjects.container().end(); ++p)
            if (*p == &aProject)
                return p;
        throw project_not_found();
    }
}