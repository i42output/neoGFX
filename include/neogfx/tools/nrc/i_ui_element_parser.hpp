// i_ui_element_parser.hpp
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
#include <neolib/i_reference_counted.hpp>
#include <neolib/string.hpp>
#include <neolib/i_vector.hpp>
#include <neolib/simple_variant.hpp>

namespace neogfx::nrc
{
    class i_ui_element;

    class i_ui_element_parser : public neolib::i_reference_counted
    {
        // exceptions
    public:
        struct element_type_not_found : std::runtime_error { element_type_not_found(const std::string& aType) : std::runtime_error{ "nrc: Element type '" + aType + "' not found." } {} };
        // types
    public:
        typedef neolib::i_simple_variant data_type;
        typedef neolib::i_vector<neolib::i_simple_variant> array_data_type;
        // operations
    public:
        virtual void indent(int32_t aLevel, neolib::i_string& aResult) const = 0;
        virtual void current_object_data(const neolib::i_string& aKey, neolib::i_simple_variant& aData) const = 0;
        virtual void emit(const neolib::i_string& aText) const = 0;
        // helpers
    public:
        std::string indent(int32_t aLevel) const
        {
            neolib::string result;
            indent(aLevel, result);
            return result.to_std_string();
        }
        neolib::simple_variant current_object_data(const neolib::i_string& aKey) const
        {
            neolib::simple_variant data;
            current_object_data(aKey, data);
            return data;
        }
    };
}
