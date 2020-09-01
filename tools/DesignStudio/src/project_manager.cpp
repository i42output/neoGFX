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
                std::function<void(i_element& aParent, neolib::fjson_value const& aNode)> add_node = [&](i_element& aParent, neolib::fjson_value const& aNode)
                {
                    static std::map<std::string, std::function<neolib::ref_ptr<i_element>(i_element&, const std::string&)>> factory =
                    {
                        { "menu", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "menu", aId); } },
                        { "window", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "window", aId); } },
                        { "dialog", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "dialog", aId); } },
                        { "widget", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "widget", aId); } },
                        { "text_widget", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "text_widget", aId); } },
                        { "image_widget", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "image_widget", aId); } },
                        { "menu_bar", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "menu_bar", aId); } },
                        { "toolbar", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "toolbar", aId); } },
                        { "status_bar", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "status_bar", aId); } },
                        { "tab_page_container", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "tab_page_container", aId); } },
                        { "tab_page", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "tab_page", aId); } },
                        { "canvas", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "canvas", aId); } },
                        { "push_button", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "push_button", aId); } },
                        { "check_box", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "check_box", aId); } },
                        { "radio_button", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "radio_button", aId); } },
                        { "label", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "label", aId); } },
                        { "text_edit", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "text_edit", aId); } },
                        { "line_edit", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "line_edit", aId); } },
                        { "text_field", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "text_field", aId); } },
                        { "drop_list", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "drop_list", aId); } },
                        { "table_view", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "table_view", aId); } },
                        { "tree_view", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "tree_view", aId); } },
                        { "group_box", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "group_box", aId); } },
                        { "slider", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "slider", aId); } },
                        { "double_slider", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "double_slider", aId); } },
                        { "spin_box", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "spin_box", aId); } },
                        { "double_spin_box", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "double_spin_box", aId); } },
                        { "slider_box", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "slider_box", aId); } },
                        { "double_slider_box", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "double_slider_box", aId); } },
                        { "gradient_widget", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Widget, "gradient_widget", aId); } },
                        { "vertical_layout", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Layout, "vertical_layout", aId); } },
                        { "horizontal_layout", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Layout, "horizontal_layout", aId); } },
                        { "grid_layout", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Layout, "grid_layout", aId); } },
                        { "flow_layout", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Layout, "flow_layout", aId); } },
                        { "stack_layout", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Layout, "stack_layout", aId); } },
                        { "border_layout", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Layout, "border_layout", aId); } },
                        { "spacer", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Layout, "spacer", aId); } },
                        { "vertical_spacer", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Layout, "vertical_spacer", aId); } },
                        { "horizontal_spacer", [](i_element& aParent, const std::string& aId) { return ng::make_ref<element<>>(aParent, element_group::Layout, "horizontal_spacer", aId); } }
                    };
                    neolib::ref_ptr<i_element> newNode;
                    auto existing = factory.find(aNode.name());
                    if (existing != factory.end())
                    {
                        switch (aNode.type())
                        {
                        case neolib::json_type::Object:
                            if (aNode.as<neolib::fjson_object>().has("id"))
                                newNode = existing->second(aParent, aNode.as<neolib::fjson_object>().at("id").text());
                            else
                                newNode = existing->second(aParent, aNode.name() + boost::lexical_cast<std::string>(++counters[aNode.name()]));
                            for (auto const& e : aNode)
                                add_node(*newNode, e);
                            break;
                        }
                    }
                    else if (!aParent.has_parent())
                    {
                        switch (aNode.type())
                        {
                        case neolib::json_type::Object:
                            for (auto const& child : aNode)
                            {
                                existing = factory.find(child.name());
                                if (existing != factory.end())
                                {
                                    switch (child.type())
                                    {
                                    case neolib::json_type::Object:
                                        if (child.as<neolib::fjson_object>().has("id"))
                                            newNode = existing->second(aParent, child.as<neolib::fjson_object>().at("id").text());
                                        else
                                            newNode = existing->second(aParent, child.name() + boost::lexical_cast<std::string>(++counters[child.name()]));
                                        for (auto const& e : child)
                                            add_node(*newNode, e);
                                        break;
                                    }
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