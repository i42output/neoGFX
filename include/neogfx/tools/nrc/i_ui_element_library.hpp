// i_ui_element_library.hpp
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
#include <neolib/i_set.hpp>
#include <neolib/i_string.hpp>
#include <neolib/i_plugin.hpp>
#include <neogfx/tools/nrc/i_ui_element.hpp>

namespace neogfx::nrc
{
    class i_ui_element_parser;

    class i_ui_element_library : public neolib::i_reference_counted
    {
        // exceptions
    public:
        struct unknown_element_type : std::logic_error { unknown_element_type() : std::logic_error{ "neogfx::nrc::i_ui_element_library::unknown_element_type" } {} };
        // types
    public:
        typedef neolib::i_set<neolib::i_string> elements_t;
        // meta
    public:
        virtual const elements_t& elements() const = 0;
        // factory
    public:
        neolib::ref_ptr<i_ui_element> create_element(const i_ui_element_parser& aParser, const neolib::i_string& aElementType)
        {
            return neolib::ref_ptr<i_ui_element>{ do_create_element(aParser, aElementType) };
        }
        neolib::ref_ptr<i_ui_element> create_element(i_ui_element& aParent, const neolib::i_string& aElementType)
        {
            return neolib::ref_ptr<i_ui_element>{ do_create_element(aParent, aElementType) };
        }
        // implementation
    private:
        virtual i_ui_element* do_create_element(const i_ui_element_parser& aParser, const neolib::i_string& aElementType) = 0;
        virtual i_ui_element* do_create_element(i_ui_element& aParent, const neolib::i_string& aElementType) = 0;
        // interface
    public:
        static const neolib::uuid& iid() { static const neolib::uuid sId = neolib::make_uuid("3D10EA2F-5110-419C-BFF9-93263C618792"); return sId; }
    };
}
