// library.hpp
/*
neoGFX Resource Compiler
Copyright(C) 2019 Leigh Johnston

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
#include <neolib/reference_counted.hpp>
#include <neolib/set.hpp>
#include <neolib/string.hpp>
#include <neolib/i_application.hpp>
#include <neolib/version.hpp>
#include <neogfx/tools/nrc/i_ui_element_library.hpp>

namespace neogfx::nrc
{
    class default_ui_element_library : public neolib::reference_counted<i_ui_element_library>
    {
        // types
    public:
        typedef neolib::set<neolib::string> elements_t;
        // construction
    public:
        default_ui_element_library(neolib::i_application& aApplication, const std::string& aPluginPath);
        // meta
    public:
        bool handles_element(const neolib::i_string& aElementType) const override;
        bool handles_element(i_ui_element& aParent, const neolib::i_string& aElementType) const override;
        // implementation
    private:
        i_ui_element* do_create_element(const i_ui_element_parser& aParser, const neolib::i_string& aElementType) override;
        i_ui_element* do_create_element(const i_ui_element_parser& aParser, i_ui_element& aParent, const neolib::i_string& aElementType) override;
        // meta
    public:
        static const neolib::uuid& library_id() { static const neolib::uuid sId = neolib::make_uuid("6942704C-043E-43A2-9290-CD58E106E65E"); return sId; }
        static const std::string& library_name() { static const std::string sName = "default"; return sName; }
        static const std::string& library_description() { static const std::string sDescription = "Default widget library"; return sDescription; }
        static const neolib::version& library_version() { static const neolib::version sVersion = { 1, 0, 0, 0 }; return sVersion; }
        static const std::string& library_copyright() { static const std::string sCopyright = "Copyright (C) 2019 Leigh Johnston"; return sCopyright; };
        // state
    private:
        neolib::i_application& iApplication;
        std::string iPluginPath;
        elements_t iRootElements;
        elements_t iChildElements;
    };
}
