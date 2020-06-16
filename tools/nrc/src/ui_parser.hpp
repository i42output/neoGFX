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
#include <map>
#include <neolib/core/vector.hpp>
#include <neolib/core/reference_counted.hpp>
#include <neolib/file/json.hpp>
#include <neolib/plugin/simple_variant.hpp>
#include <neolib/plugin/i_plugin_manager.hpp>

#include <neogfx/tools/nrc/i_ui_element_parser.hpp>
#include <neogfx/tools/nrc/i_ui_element_library.hpp>

namespace neogfx::nrc
{
    class ui_parser : public neolib::reference_counted<i_ui_element_parser>
    {
        typedef neolib::reference_counted<i_ui_element_parser> base_type;
    public:
        typedef std::map<std::string, const i_ui_element*> index_t;
        typedef neolib::simple_variant data_t;
        typedef neolib::vector<neolib::simple_variant> array_data_t;
    public:
        ui_parser(const boost::filesystem::path& aInputFilename, const neolib::i_plugin_manager& aPluginManager, const neolib::fjson_string& aNamespace, const neolib::fjson_object& aRoot, std::ofstream& aOutput);
    public:
        const neolib::i_string& element_namespace() const override;
        const neolib::i_string& current_fragment() const override;
        const neolib::i_string& current_element() const override;
        void index(const neolib::i_string& aId, const i_ui_element& aElement) const override;
        const i_ui_element* find(const neolib::i_string& aId) const override;
        const i_ui_element& at(const neolib::i_string& aId) const override;
        using base_type::generate_anonymous_id;
        void generate_anonymous_id(neolib::i_string& aNewAnonymousId) const override;
        using i_ui_element_parser::indent;
        void indent(int32_t aLevel, neolib::i_string& aResult) const override;
        using base_type::emit;
        void emit(const neolib::i_string& aText) const override;
    private:
        void do_source_location(neolib::i_string& aLocation) const override;
        bool do_data_exists(const neolib::i_string& aKey) const override;
        bool do_array_data_exists(const neolib::i_string& aKey) const override;
        const data_t& do_get_data(const neolib::i_string& aKey) const override;
        data_t& do_get_data(const neolib::i_string& aKey) override;
        const array_data_t& do_get_array_data(const neolib::i_string& aKey) const override;
        array_data_t& do_get_array_data(const neolib::i_string& aKey) override;
        void to_data(const neolib::fjson_value& aNode, data_t& aResult) const;
        void to_array_data(const neolib::fjson_value& aNode, array_data_t& aResult) const;
    private:
        std::string headers() const;
        void next_header(const i_ui_element& aElement, std::set<std::string>& aHeaders) const;
        neolib::ref_ptr<i_ui_element> create_element(const neolib::i_string& aElementType);
        neolib::ref_ptr<i_ui_element> create_element(i_ui_element& aParent, const neolib::i_string& aElementType);
        const neolib::fjson_object& current_object() const;
        void parse(const neolib::fjson_value& aNode);
        void parse(const neolib::fjson_value& aNode, i_ui_element& aElement);
    private:
        const boost::filesystem::path& iInputFilename;
        const neolib::fjson_object& iRoot;
        std::ofstream& iOutput;
        std::vector<neolib::ref_ptr<i_ui_element_library>> iLibraries;
        neolib::string iNamespace;
        std::vector<std::pair<std::string, neolib::ref_ptr<i_ui_element>>> iRootElements;
        mutable const neolib::fjson_value* iCurrentNode;
        mutable const neolib::fjson_value* iCurrentFragment;
        mutable std::map<std::pair<const neolib::fjson_object*, std::string>, data_t> iDataCache;
        mutable std::map<std::pair<const neolib::fjson_object*, std::string>, array_data_t> iArrayDataCache;
        mutable uint32_t iAnonymousIdCounter;
        mutable index_t iIndex;
    };
}
