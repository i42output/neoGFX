// resource_parser.hpp
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
#include <neolib/file/json.hpp>

namespace neogfx::nrc
{
    class resource_parser
    {
    public:
        resource_parser(const boost::filesystem::path& aInputFilename, const neolib::fjson_string& aNamespace, std::ofstream& aOutput);
    public:
        void parse(const neolib::fjson_value& aItem);
    private:
        const boost::filesystem::path iInputFilename;
        const neolib::fjson_string iNamespace;
        std::ofstream& iOutput;
        uint32_t iResourceIndex;
    };
}
