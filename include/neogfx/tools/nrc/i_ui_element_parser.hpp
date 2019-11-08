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
#include <neolib/i_string.hpp>
#include <neolib/i_vector.hpp>
#include <neolib/i_simple_variant.hpp>

namespace neogfx::nrc
{
    class i_ui_element;

    class i_ui_element_parser : public neolib::i_reference_counted
    {
    public:
        typedef neolib::i_simple_variant data_type;
        typedef neolib::i_vector<neolib::i_simple_variant> array_data_type;
    public:
        virtual void emit(const neolib::i_string& aText) = 0;
    };
}
