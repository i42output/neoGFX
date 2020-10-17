// project_manager.cpp
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

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/tools/DesignStudio/i_element_library.hpp>
#include <neogfx/tools/DesignStudio/project.hpp>
#include <neogfx/tools/DesignStudio/project_manager.hpp>

namespace neogfx::DesignStudio
{
    project_manager::project_manager()
    {
        for (auto const& plugin : service<i_app>().plugin_manager().plugins())
        {
            ref_ptr<i_element_library> elementLibrary;
            plugin->discover(elementLibrary);
            if (elementLibrary)
                iElementLibraries.push_back(elementLibrary);
        }
    }

    project_manager::~project_manager()
    {
        while (!iProjects.empty())
            close_project(*iProjects.back());
    }

    const project_manager::project_list& project_manager::projects() const
    {
        return iProjects;
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

    void project_manager::activate_project(const i_project& aProject)
    {
        if (!project_active() || &active_project() != &aProject)
        {
            auto previouslyActive = iActiveProject;
            iActiveProject = *find_project(aProject);
            ProjectActivated.trigger(*iActiveProject);
            if (previouslyActive)
                ProjectDeactivated.trigger(*previouslyActive);
        }
    }

    void project_manager::deactivate_project()
    {
        if (project_active())
        {
            auto previouslyActive = iActiveProject;
            iActiveProject = nullptr;
            ProjectDeactivated.trigger(*previouslyActive);
        }
        else
            throw no_active_project();
    }

    i_project& project_manager::open_project(const ng::i_string& aProjectFile)
    {
        auto newProject = ng::make_ref<project>(*this);
        newProject->open(aProjectFile);
        iProjects.push_back(newProject);
        ProjectAdded.trigger(*newProject);
        activate_project(*newProject);
        return *newProject;
    }

    i_project& project_manager::create_project(const ng::i_string& aProjectName, const ng::i_string& aProjectNamespace)
    {
        auto newProject = ng::make_ref<project>(*this);
        newProject->create(aProjectName, aProjectNamespace);
        iProjects.push_back(newProject);
        ProjectAdded.trigger(*newProject);
        activate_project(*newProject);
        return *newProject;
    }

    void project_manager::close_project(const i_project& aProject)
    {
        auto removing = find_project(aProject);
        auto removingRef = *removing;
        iProjects.container().erase(removing);
        if (project_active() && &active_project() == &aProject)
        {
            if (!iProjects.empty())
                activate_project(*iProjects.container().back());
            else
                deactivate_project();
        }
        ProjectRemoved.trigger(*removingRef);
    }

    i_element_library& project_manager::library(const ng::i_string& aElementType) const
    {
        auto existing = find_library(aElementType.to_std_string());
        if (existing)
            return *existing;
        throw element_type_not_found(aElementType.to_std_string());
    }

    project_manager::project_list::container_type::const_iterator project_manager::find_project(const i_project& aProject) const
    {
        for (auto p = iProjects.container().begin(); p != iProjects.container().end(); ++p)
            if (*p == &aProject)
                return p;
        throw project_not_found();
    }

    ref_ptr<i_element_library> project_manager::find_library(std::string const& aElementType) const
    {
        for (auto const& library : iElementLibraries)
        {
            if (library->elements().find(string{ aElementType }) != library->elements().end())
                return library;
        }
        return {};
    }
}