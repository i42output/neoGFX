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
            add_data_names({ "title", "style", "default_size" });
            emplace_2<length>("default_size", iDefaultSize);
        }
        window(const i_ui_element_parser& aParser, i_ui_element& aParent, ui_element_type aElementType = ui_element_type::Window) :
            ui_element<>{ aParser, aParent, aElementType },
            iStyle{ aParser.get_optional_enum<window_style>("style") }
        {
            add_data_names({ "title", "style", "default_size" });
            emplace_2<length>("default_size", iDefaultSize);
        }
    public:
        const neolib::i_string& header() const override
        {
            static const neolib::string sHeader = "neogfx/gui/window/window.hpp";
            return sHeader;
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
                    "  %1%(%2%) :\n", fragment_name().to_std_string_view(), generate_ref_ctor_args(false));
                emit_ctor();
                emit("  {\n");
                emit_body();
                emit("  }\n");
            }
        }
        void emit_preamble() const override
        {
            if (has_parent())
                emit("  window %1%;\n", id());
            else
                emit("  %1%& %2%;\n", type_name(), id());
            ui_element<>::emit_preamble();
        }
        void emit_ctor() const override
        {
            if (has_parent())
            {
                if (iDefaultSize)
                {
                    if (iStyle)
                        emit(",\n"
                            "   %1%{ %2%, size{ %3%, %4% }, %5% }", id(), parent().id(), iDefaultSize->cx, iDefaultSize->cy, enum_to_string("window_style", *iStyle));
                    else
                        emit(",\n"
                            "   %1%{ %2%, size{ %3%, %4% } }", id(), parent().id(), iDefaultSize->cx, iDefaultSize->cy);
                }
                else
                {
                    if (iStyle)
                        emit(",\n"
                            "   %1%{ %2%, %3% }", id(), parent().id(), enum_to_string("window_style", *iStyle));
                    else
                        emit(",\n"
                            "   %1%{ %2% }", id(), parent().id());
                }
            }
            else
            {
                if (iDefaultSize)
                {
                    if (iStyle)
                        emit(
                            "   %1%{ %2%, size{ %3%, %4% }, %5% }", type_name(), iDefaultSize->cx, iDefaultSize->cy, enum_to_string("window_style", *iStyle));
                    else
                        emit(
                            "   %1%{ size{ %2%, %3% } }", type_name(), iDefaultSize->cx, iDefaultSize->cy);
                }
                else
                {
                    if (iStyle)
                        emit(
                            "   %1%{ %2% }", type_name(), enum_to_string("window_style", *iStyle));
                    else
                        emit(
                            "   %1%{}", type_name());
                }
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
                emit("   %1%.set_title(\"%2%\"_t);\n", id(), iTitle->to_std_string_view());
            ui_element<>::emit_body();
        }
    protected:
        using ui_element<>::emit;
    private:
        std::optional<window_style> iStyle;
        std::optional<string> iTitle;
        std::optional<basic_size<length>> iDefaultSize;
    };
}
