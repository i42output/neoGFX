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
#include <neolib/file/json.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/tools/DesignStudio/i_element_library.hpp>
#include <neogfx/tools/DesignStudio/project.hpp>
#include <neogfx/tools/DesignStudio/project_manager.hpp>

namespace neogfx::DesignStudio
{
    project_manager::project_manager()
    {
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
        std::vector<ref_ptr<i_element_library>> elementLibraries;
        for (auto const& plugin : service<i_app>().plugin_manager().plugins())
        {
            ref_ptr<i_element_library> elementLibrary;
            plugin->discover(elementLibrary);
            if (elementLibrary)
                elementLibraries.push_back(elementLibrary);
        }
        auto find_library = [&](const std::string& aType) -> ref_ptr<i_element_library>
        {
            for (auto const& library : elementLibraries)
            {
                if (library->elements().find(string{ aType }) != library->elements().end())
                    return library;
            }
            return {};
        };
        boost::filesystem::path const inputFileName{ aProjectFile.to_std_string() };
        neolib::fjson const input{ inputFileName.string() };
        if (!input.has_root())
            throw invalid_project_file("bad root node");
        auto const& ns = input.root().as<neolib::fjson_object>().has("namespace") ? input.root().as<neolib::fjson_object>().at("namespace").text() : "";
        auto newProject = ng::make_ref<project>(inputFileName.stem().string(), ns);
        for (auto const& item : input.root())
        {
            if (item.name() == "resource")
            {
                // todo
            }
            else if (item.name() == "ui")
            {
                std::map<std::string, uint32_t> counters;
                std::function<void(i_element&, neolib::fjson_value const&)> add_node = [&](i_element& aParent, neolib::fjson_value const& aNode)
                {
                    if (aParent.has_parent())
                    {
                        auto elementLibrary = find_library(aNode.name());
                        if (elementLibrary)
                        {
                            ref_ptr<i_element> newNode;
                            switch (aNode.type())
                            {
                            case neolib::json_type::Object:
                                if (aNode.as<neolib::fjson_object>().has("id"))
                                    newNode = elementLibrary->create_element(aParent, aNode.name(), aNode.as<neolib::fjson_object>().at("id").text());
                                else
                                    newNode = elementLibrary->create_element(aParent, aNode.name(), aNode.name() + boost::lexical_cast<std::string>(++counters[aNode.name()]));
                                for (auto const& e : aNode)
                                    add_node(*newNode, e);
                                break;
                            }
                        }
                    }
                    else
                    {
                        switch (aNode.type())
                        {
                        case neolib::json_type::Object:
                            for (auto const& child : aNode)
                            {
                                ref_ptr<i_element> newNode;
                                switch (child.type())
                                {
                                case neolib::json_type::Object:
                                    if (child.as<neolib::fjson_object>().has("id"))
                                    {
                                        auto elementLibrary = find_library(child.name());
                                        if (elementLibrary)
                                            newNode = elementLibrary->create_element(aParent, child.name(), child.as<neolib::fjson_object>().at("id").text());
                                    }
                                    else
                                    {
                                        auto elementLibrary = find_library(child.name());
                                        if (elementLibrary)
                                            newNode = elementLibrary->create_element(aParent, child.name(), child.name() + boost::lexical_cast<std::string>(++counters[child.name()]));
                                    }
                                    for (auto const& e : child)
                                        add_node(*newNode, e);
                                    break;
                                }
                            }
                            break;
                        }
                    }
                };
                for (auto const& child : item)
                    add_node(newProject->root(), child);
            }
        }
        iProjects.push_back(newProject);
        ProjectAdded.trigger(*newProject);
        activate_project(*newProject);
        return *newProject;
    }

    i_project& project_manager::create_project(const ng::i_string& aProjectName, const ng::i_string& aProjectNamespace)
    {
        auto newProject = ng::make_ref<project>(aProjectName.to_std_string(), aProjectNamespace.to_std_string());
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

    project_manager::project_list::container_type::const_iterator project_manager::find_project(const i_project& aProject) const
    {
        for (auto p = iProjects.container().begin(); p != iProjects.container().end(); ++p)
            if (*p == &aProject)
                return p;
        throw project_not_found();
    }
}