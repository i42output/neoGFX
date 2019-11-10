// app.hpp
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
#include <boost/format.hpp>
#include <neogfx/tools/nrc/ui_element.hpp>

namespace neogfx::nrc
{
    class app : public ui_element<>
    {
    public:
        app(const i_ui_element_parser& aParser) :
            ui_element<>{ aParser, aParser.current_object_data(neolib::string{"id"}).value_as_string(), ui_element_type::App }
        {
            aParser.emit(neolib::string{ 
                (boost::format(
                    "  neogfx::app %1%;\n"
                    "\n"
                    "  ui(int argc, char* argv[], const std::string& aName = std::string{}) :\n"
                    "   %1%{ argc, argv, aName } {}\n") % id()
                ).str()});
        }
    public:
        void parse(const neolib::i_string& aName, const data_type& aData) override
        {
        }
        void parse(const neolib::i_string& aName, const array_data_type& aData) override
        {
        }
    };
}
