// layout.hpp
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
#include <neogfx/tools/nrc/ui_element.hpp>

namespace neogfx::nrc
{
    template<ui_element_type LayoutType>
    class basic_layout : public ui_element<>
    {
    public:
        basic_layout(const i_ui_element_parser& aParser, i_ui_element& aParent) :
            ui_element<>{ aParser, aParent, LayoutType }
        {
            add_data_names({ "spacing" });
        }
    public:
        const neolib::i_string& header() const override
        {
            switch (LayoutType)
            {
            case ui_element_type::VerticalLayout:
                {
                    static const neolib::string sHeader = "neogfx/gui/layout/vertical_layout.hpp";
                    return sHeader;
                }
            case ui_element_type::HorizontalLayout:
                {
                    static const neolib::string sHeader = "neogfx/gui/layout/horizontal_layout.hpp";
                    return sHeader;
                }
            case ui_element_type::GridLayout:
                {
                    static const neolib::string sHeader = "neogfx/gui/layout/grid_layout.hpp";
                    return sHeader;
                }
            case ui_element_type::FlowLayout:
                {
                    static const neolib::string sHeader = "neogfx/gui/layout/flow_layout.hpp";
                    return sHeader;
                }
            case ui_element_type::StackLayout:
                {
                    static const neolib::string sHeader = "neogfx/gui/layout/stack_layout.hpp";
                    return sHeader;
                }
            case ui_element_type::BorderLayout:
                {
                    static const neolib::string sHeader = "neogfx/gui/layout/border.hpp";
                    return sHeader;
                }
            default:
                throw wrong_type();
            }
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            ui_element<>::parse(aName, aData);
            if (aName == "spacing")
                iSpacing.emplace(get_scalar<length>(aData));
        }
        void parse(const neolib::i_string& aName, const array_data_t& aData) override
        {
            ui_element<>::parse(aName, aData);
            if (aName == "spacing")
                emplace_2<length>("spacing", iSpacing);
        }
    protected:
        void emit() const override
        {
        }
        void emit_preamble() const override
        {
            switch (LayoutType)
            {
            case ui_element_type::VerticalLayout:
                emit("  vertical_layout %1%;\n", id());
                break;
            case ui_element_type::HorizontalLayout:
                emit("  horizontal_layout %1%;\n", id());
                break;
            case ui_element_type::GridLayout:
                emit("  grid_layout %1%;\n", id());
                break;
            case ui_element_type::FlowLayout:
                emit("  flow_layout %1%;\n", id());
                break;
            case ui_element_type::StackLayout:
                emit("  stack_layout %1%;\n", id());
                break;
            case ui_element_type::BorderLayout:
                emit("  border_layout %1%;\n", id());
                break;
            default:
                throw wrong_type();
            }
            ui_element<>::emit_preamble();
        }
        void emit_ctor() const override
        {
            ui_element<>::emit_generic_ctor();
            ui_element<>::emit_ctor();
        }
        void emit_body() const override
        {
            if (iSpacing)
                emit("   %1%.set_spacing(size{ %2%, %3% });\n", id(), iSpacing->cx, iSpacing->cy);
            ui_element<>::emit_body();
        }
    protected:
        using ui_element<>::emit;
    private:
        std::optional<basic_size<length>> iSpacing;
    };

    class grid_layout : public basic_layout<ui_element_type::GridLayout>
    {
        typedef basic_layout<ui_element_type::GridLayout> base_type;
    public:
        grid_layout(const i_ui_element_parser& aParser, i_ui_element& aParent) :
            base_type{ aParser, aParent },
            iRows{ aParser.get_optional<uint32_t>("rows") },
            iColumns{ aParser.get_optional<uint32_t>("columns") }
        {
            add_data_names({ "rows", "columns" });
        }
    protected:
        void emit_body() const override
        {
            if (iRows || iColumns)
                emit("   %1%.set_dimensions(%2%, %3%);\n", id(), iRows ? *iRows : 1u, iColumns ? *iColumns : 1u);
            base_type::emit_body();
        }
    public:
        std::optional<uint32_t> iRows;
        std::optional<uint32_t> iColumns;
    };

    typedef basic_layout<ui_element_type::VerticalLayout> vertical_layout;
    typedef basic_layout<ui_element_type::HorizontalLayout> horizontal_layout;
    typedef basic_layout<ui_element_type::FlowLayout> flow_layout;
    typedef basic_layout<ui_element_type::StackLayout> stack_layout;
    typedef basic_layout<ui_element_type::BorderLayout> border_layout;
}
