// project.cpp
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

#include <filesystem>
#include <boost/lexical_cast.hpp>

#include <neolib/file/json.hpp>
#include <neogfx/tools/DesignStudio/project.hpp>
#include <neogfx/tools/DesignStudio/i_project_manager.hpp>
#include <neogfx/tools/DesignStudio/i_element_library.hpp>

namespace neogfx::DesignStudio
{
    project::project(i_project_manager& aManager) :
        iManager{ aManager }
    {
    }

    i_project_manager& project::manager() const
    {
        return iManager;
    }

    void project::create(const i_string& aName, const i_string& aNamespace)
    {
        iRoot = manager().library("project"_s).create_element(*this, "project", aName);
    }

    void project::open(const i_string& aPath)
    {
        std::filesystem::path const inputFileName{ aPath.to_std_string() };
        neolib::fjson const input{ inputFileName.string() };
        if (!input.has_root())
            throw invalid_project_file("bad root node");
        iName = inputFileName.stem().string();
        iNamespace = input.root().as<neolib::fjson_object>().has("namespace") ? input.root().as<neolib::fjson_object>().at("namespace").text() : "";
        iRoot = manager().library("project"_s).create_element(*this, "project", inputFileName.stem().string());
        auto userInterface = manager().library("user_interface"_s).create_element(*iRoot, "user_interface", "User Interface"_t);
        for (auto const& item : input.root())
        {
            if (item.name() == "resource")
            {
                // todo
            }
            else if (item.name() == "ui")
            {
                std::map<std::string, std::uint32_t> counters;
                std::function<void(i_element&, neolib::fjson_value const&)> add_node = [&](i_element& aParent, neolib::fjson_value const& aNode)
                {
                    if (aParent.has_parent() && aParent.parent().has_parent())
                    {
                        ref_ptr<i_element> newNode;
                        switch (aNode.type())
                        {
                        case neolib::json_type::Object:
                            try
                            {
                                if (aNode.as<neolib::fjson_object>().has("id"))
                                    newNode = create_element(aParent, string{ aNode.name() }, string{ aNode.as<neolib::fjson_object>().at("id").text() });
                                else
                                    newNode = create_element(aParent, string{ aNode.name() }, string{ aNode.name() + boost::lexical_cast<std::string>(++counters[aNode.name()]) });
                                for (auto const& e : aNode)
                                    add_node(*newNode, e);
                            }
                            catch (...)
                            {
                            }
                            break;
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
                                    try
                                    {
                                        if (child.as<neolib::fjson_object>().has("id"))
                                            newNode = create_element(aParent, string{ child.name() }, string{ child.as<neolib::fjson_object>().at("id").text() });
                                        else
                                            newNode = create_element(aParent, string{ child.name() }, string{ child.name() + boost::lexical_cast<std::string>(++counters[child.name()]) });
                                        for (auto const& e : child)
                                            add_node(*newNode, e);
                                    }
                                    catch (...)
                                    {
                                    }
                                    break;
                                }
                            }
                            break;
                        }
                    }
                };
                for (auto const& child : item)
                    add_node(*userInterface, child);
            }
        }
    }

    const i_string& project::name() const
    {
        return iName;
    }

    const i_string& project::namespace_() const
    {
        return iNamespace;
    }

    const i_element& project::root() const
    {
        return *iRoot;
    }

    i_element& project::root()
    {
        return *iRoot;
    }

    i_element& project::create_element(i_element& aParent, const i_string& aType, const i_string& aElementId)
    {
        auto result = manager().library(aType).create_element(aParent, aType, aElementId);
        ElementAdded(*result);
        result->create_default_children();
        return *result;
    }

    void project::remove_element(i_element& aElement)
    {
        ref_ptr<i_element> temp = aElement;
        aElement.parent().remove_child(aElement);
        ElementRemoved(*temp);
    }
}