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
            switch (LayoutType)
            {
            case ui_element_type::VerticalLayout:
                add_header("neogfx/gui/layout/vertical_layout.hpp");
                break;
            case ui_element_type::HorizontalLayout:
                add_header("neogfx/gui/layout/horizontal_layout.hpp");
                break;
            case ui_element_type::GridLayout:
                add_header("neogfx/gui/layout/grid_layout.hpp");
                break;
            case ui_element_type::FlowLayout:
                add_header("neogfx/gui/layout/flow_layout.hpp");
                break;
            case ui_element_type::StackLayout:
                add_header("neogfx/gui/layout/stack_layout.hpp");
                break;
            case ui_element_type::BorderLayout:
                add_header("neogfx/gui/layout/border.hpp");
                break;
            }
            add_data_names({ "spacing" });
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
            emit("  %1% %2%;\n", type_name(), id());
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
        void emit_ctor() const override
        {
            if (!iRows && !iColumns)
                base_type::emit_ctor();
            else
            {
                emit(",\n"
                    "   %1%{ %2%%3%, %4%, %5% }", id(), parent().id(), layout(), iRows ? *iRows : 1u, iColumns ? *iColumns : 1u);
                ui_element<>::emit_ctor();
            }
        }
        void emit_body() const override
        {
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
