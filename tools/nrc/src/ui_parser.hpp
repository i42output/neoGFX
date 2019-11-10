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

#include <neogfx/neogfx.hpp>
#include <neolib/i_simple_variant.hpp>
#include <neolib/reference_counted.hpp>
#include <neolib/i_plugin_manager.hpp>
#include <neolib/json.hpp>

#include <neogfx/tools/nrc/i_ui_element_parser.hpp>
#include <neogfx/tools/nrc/i_ui_element_library.hpp>

namespace neogfx::nrc
{
    class ui_parser : public neolib::reference_counted<i_ui_element_parser>
    {
    public:
        typedef neolib::simple_variant data_type;
    public:
        ui_parser(const neolib::i_plugin_manager& aPluginManager, const neolib::fjson_string& aNamespace, const neolib::fjson_object& aRoot, std::ofstream& aOutput);
    public:
        using i_ui_element_parser::indent;
        void indent(int32_t aLevel, neolib::i_string& aResult) const override;
        void current_object_data(const neolib::i_string& aKey, neolib::i_simple_variant& aData) const override;
        void emit(const neolib::i_string& aText) const override;
    private:
        void parse(const neolib::fjson_value& aNode);
        void parse(const neolib::fjson_value& aNode, i_ui_element& aElement);
        neolib::ref_ptr<i_ui_element> create_object(const neolib::fjson_value& aNode, neolib::ref_ptr<i_ui_element> aParent = nullptr);
    private:
        const neolib::fjson_object& iRoot;
        std::ofstream& iOutput;
        std::vector<neolib::ref_ptr<i_ui_element_library>> iLibraries;
        std::vector<neolib::ref_ptr<i_ui_element>> iElements;
        mutable const neolib::fjson_value* iCurrentNode;
    };
}
