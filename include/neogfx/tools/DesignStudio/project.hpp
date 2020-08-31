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
    class project : public ng::reference_counted<ng::model<i_project>>
    {
    public:
        typedef i_project abstract_type;
    public:
        project(const std::string& aName, const std::string& aNamespace);
    public:
        const ng::i_string& name() const override;
        const ng::i_string& namespace_() const override;
    public:
        const i_element& root() const override;
        i_element& root() override;
    private:
        ng::string iName;
        ng::string iNamespace;
        element<> iApp;
    };
}