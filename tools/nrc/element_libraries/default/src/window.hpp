// window.hpp
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
#include <neogfx/gui/window/window_bits.hpp>
#include <neogfx/tools/nrc/ui_element.hpp>

namespace neogfx::nrc
{
    class window : public ui_element<>
    {
    public:
        window(const i_ui_element_parser& aParser, ui_element_type aElementType = ui_element_type::Window) :
            ui_element<>{ aParser, aElementType },
            iStyle{ aParser.get_optional_enum<window_style>("style") }
        {
            add_header("neogfx/gui/window/window.hpp");
            add_data_names({ "title", "style", "default_size" });
            emplace_2<length>("default_size", iDefaultSize);
        }
        window(const i_ui_element_parser& aParser, i_ui_element& aParent, ui_element_type aElementType = ui_element_type::Window) :
            ui_element<>{ aParser, aParent, aElementType },
            iStyle{ aParser.get_optional_enum<window_style>("style") }
        {
            add_header("neogfx/gui/window/window.hpp");
            add_data_names({ "title", "style", "default_size" });
            emplace_2<length>("default_size", iDefaultSize);
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            if (aName == "title")
                iTitle = aData.get<neolib::i_string>();
            else
                ui_element<>::parse(aName, aData);
        }
        void parse(const neolib::i_string& aName, const array_data_t& aData) override
        {
            if (aName == "style")
                iStyle = get_enum<window_style>(aData);
            else
                ui_element<>::parse(aName, aData);
        }
    protected:
        void emit() const override
        {
            if (!has_parent())
            {
                emit_preamble();
                emit("\n"
                    "  %1%(%2%) :\n", fragment_name().to_std_string_view(), generate_ctor_params(false));
                emit_ctor();
                emit("  {\n");
                emit_body();
                emit("  }\n");
            }
        }
        void emit_preamble() const override
        {
            if (has_parent())
                emit("  %1% %2%;\n", type_name(), id());
            else
                emit("  %1%& %2%;\n", type_name(), id());
            ui_element<>::emit_preamble();
        }
        void emit_ctor() const override
        {
            if (iDefaultSize)
            {
                if (iStyle)
                    emit(
                        "   %1%{ %2%%3%, size{ %4%, %5% }, %6% }", type_name(), generate_base_ctor_args(true), iDefaultSize->cx, iDefaultSize->cy, enum_to_string("window_style", *iStyle));
                else
                    emit(
                        "   %1%{ %2%size{ %3%, %4% } }", type_name(), generate_base_ctor_args(true), iDefaultSize->cx, iDefaultSize->cy);
            }
            else
            {
                if (iStyle)
                    emit(
                        "   %1%{ %2%%3% }", type_name(), generate_base_ctor_args(true), enum_to_string("window_style", *iStyle));
                else
                    emit(
                        "   %1%{%2%}", type_name(), generate_base_ctor_args(false) );
            }
            if (!has_parent())
                emit(",\n"
                    "   %1%{ *this }", id());
            ui_element<>::emit_ctor();
            emit("\n");
        }
        void emit_body() const override
        {
            if (iTitle)
                emit("   %1%.set_title_text(\"%2%\"_t);\n", id(), iTitle->to_std_string_view());
            ui_element<>::emit_body();
            if (!iStyle || (*iStyle & window_style::InitiallyCentered) == window_style::InitiallyCentered)
            {
                if ((type() & ui_element_type::Dialog) == ui_element_type::Dialog)
                {
                    emit(
                        "   center_on_parent();\n"
                        "   set_ready_to_render(true);\n");
                }
            }
        }
    protected:
        using ui_element<>::emit;
    private:
        neolib::optional<window_style> iStyle;
        neolib::optional<string> iTitle;
        neolib::optional<basic_size<length>> iDefaultSize;
    };
}
