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
#include <neolib/vector.hpp>
#include <neolib/simple_variant.hpp>
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
        typedef neolib::simple_variant data_t;
        typedef neolib::vector<neolib::simple_variant> array_data_t;
    public:
        ui_parser(const neolib::i_plugin_manager& aPluginManager, const neolib::fjson_string& aNamespace, const neolib::fjson_object& aRoot, std::ofstream& aOutput);
    public:
        using i_ui_element_parser::indent;
        void indent(int32_t aLevel, neolib::i_string& aResult) const override;
        void emit(const neolib::i_string& aText) const override;
    private:
        bool do_data_exists(const neolib::i_string& aKey) const override;
        const data_t& do_get_data(const neolib::i_string& aKey) const override;
        data_t& do_get_data(const neolib::i_string& aKey) override;
        const array_data_t& do_get_array_data(const neolib::i_string& aKey) const override;
        array_data_t& do_get_array_data(const neolib::i_string& aKey) override;
    private:
        neolib::ref_ptr<i_ui_element> create_element(const neolib::i_string& aElementType);
        neolib::ref_ptr<i_ui_element> create_element(i_ui_element& aParent, const neolib::i_string& aElementType);
        void parse(const neolib::fjson_value& aNode);
        void parse(const neolib::fjson_value& aNode, i_ui_element& aElement);
    private:
        const neolib::fjson_object& iRoot;
        std::ofstream& iOutput;
        std::vector<neolib::ref_ptr<i_ui_element_library>> iLibraries;
        std::vector<neolib::ref_ptr<i_ui_element>> iRootElements;
        mutable const neolib::fjson_value* iCurrentNode;
        mutable std::map<std::pair<const neolib::fjson_value*, std::string>, data_t> iDataCache;
        mutable std::map<std::pair<const neolib::fjson_value*, std::string>, array_data_t> iArrayDataCache;
    };
}
