// library.hpp
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

#include <neogfx/neogfx.hpp>
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/vector.hpp>
#include <neolib/core/set.hpp>
#include <neolib/core/map.hpp>
#include <neolib/core/string.hpp>
#include <neolib/app/i_application.hpp>
#include <neolib/app/version.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/gui/widget/menu.hpp>
#include <neogfx/tools/DesignStudio/i_element_library.hpp>

namespace neogfx::DesignStudio
{
    class neos_element_library : public neolib::reference_counted<i_element_library>
    {
        // types
    public:
        typedef neolib::set<neolib::string> elements_t;
        typedef neolib::vector<neolib::string> elements_ordered_t;
        // construction
    public:
        neos_element_library(neolib::i_application& aApplication, std::string const& aPluginPath);
        ~neos_element_library();
        //initialisation
    public:
        neolib::i_application& application() const override;
        void ide_ready(i_ide& aIde) override;
        // meta
    public:
        const elements_t& elements() const override;
        const elements_ordered_t& elements_ordered() const override;
        // factory
    public:
        void create_element(i_project& aProject, const neolib::i_string& aElementType, const neolib::i_string& aElementId, neolib::i_ref_ptr<i_element>& aResult) override;
        void create_element(i_element& aParent, const neolib::i_string& aElementType, const neolib::i_string& aElementId, neolib::i_ref_ptr<i_element>& aResult) override;
        DesignStudio::element_group element_group(const neolib::i_string& aElementType) const override;
        i_texture const& element_icon(const neolib::i_string& aElementType) const override;
        // meta
    public:
        static const neolib::uuid& library_id() { static const neolib::uuid sId = neolib::make_uuid("E690CB63-31A3-4BE5-AA3C-D599EEA3F611"); return sId; }
        static std::string const& library_name() { static const std::string sName = "neos"; return sName; }
        static std::string const& library_description() { static const std::string sDescription = "neos element library"; return sDescription; }
        static const neolib::version& library_version() { static const neolib::version sVersion = { 1, 0, 0, 0 }; return sVersion; }
        static std::string const& library_copyright() { static const std::string sCopyright = "Copyright (C) 2021 Leigh Johnston"; return sCopyright; };
        // state
    private:
        neolib::i_application& iApplication;
        std::string iPluginPath;
        elements_ordered_t iElementsOrdered;
        elements_t iElements;
        mutable std::map<color, std::map<std::string, texture>> iIcons;
        action iSeparator;
        action iNewScript;
        action iStartDebugging;
        action iStartWithoutDebugging;
        action iStepInto;
        action iStepOver;
        action iToggleBreakpoint;
        action iNewBreakpoint;
        action iDeleteAllBreakpoints;
        action iDisableAllBreakpoints;
        action iImportScriptingLanguage;
        action iEditScriptingLanguage;
        menu iScriptMenu;
    };
}
