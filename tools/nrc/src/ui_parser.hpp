// ui_parser.hpp
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

#include <fstream>
#include <neolib/json.hpp>

namespace neogfx::nrc
{
    class ui_parser
    {
    public:
        ui_parser(const neolib::fjson_object& aRoot, std::ofstream& aOutput);
    private:
        const neolib::fjson_object& iRoot;
        std::ofstream& iOutput;
    };
}
