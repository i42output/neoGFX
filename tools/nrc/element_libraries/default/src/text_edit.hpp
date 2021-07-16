// text_edit.hpp
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
#include <neogfx/gfx/color.hpp>
#include <neogfx/tools/nrc/ui_element.hpp>

namespace neogfx::nrc
{
    class text_edit : public ui_element<>
    {
    public:
        text_edit(const i_ui_element_parser& aParser, i_ui_element& aParent, ui_element_type aElementType = ui_element_type::TextEdit) :
            ui_element<>{ aParser, aParent, aElementType }
        {
            add_header("neogfx/gui/widget/text_edit.hpp");
            add_data_names({ "size_hint", "tab_stop_hint", "text_color", "paper_color" });
        }
        text_edit(const i_ui_element_parser& aParser, i_ui_element& aParent, member_element_t, ui_element_type aElementType = ui_element_type::TextEdit) :
            ui_element<>{ aParser, aParent, member_element, aElementType }
        {
            add_header("neogfx/gui/widget/text_edit.hpp");
            add_data_names({ "size_hint", "tab_stop_hint", "text_color", "paper_color" });
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            if (aName == "size_hint")
                iSizeHint = size_hint{ aData.get<neolib::i_string>().to_std_string() };
            else if (aName == "tab_stop_hint")
                iTabStopHint = aData.get<neolib::i_string>();
            else if (aName == "text_color")
                iTextColor = get_color(aData);
            else if (aName == "paper_color")
                iPaperColor = get_color(aData);
            else
                ui_element<>::parse(aName, aData);
        }
        void parse(const neolib::i_string& aName, const array_data_t& aData) override
        {
            if (aName == "size_hint")
            {
                if (aData.size() == 1)
                    iSizeHint = size_hint{ aData[0].get<neolib::i_string>().to_std_string() };
                else if (aData.size() == 2)
                    iSizeHint = size_hint{ aData[0].get<neolib::i_string>().to_std_string(), aData[1].get<neolib::i_string>().to_std_string() };
            }
            else if (aName == "text_color")
                iTextColor = get_color_or_gradient(aData);
            else if (aName == "paper_color")
                iPaperColor = get_color_or_gradient(aData);
            else
                ui_element<>::parse(aName, aData);
        }
    protected:
        void emit() const override
        {
        }
        void emit_preamble() const override
        {
            if (type() == ui_element_type::TextEdit)
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
            ui_element<>::emit_body();
            if (iSizeHint)
            {
                if (iSizeHint->secondary_hint().empty())
                    emit("   %1%.set_size_hint(size_hint{ \"%2%\"_t });\n", id(), iSizeHint->primary_hint());
                else
                    emit("   %1%.set_size_hint(size_hint{ \"%2%\"_t, \"%3%\"_t });\n", id(), iSizeHint->primary_hint(), iSizeHint->secondary_hint());
            }
            if (iTabStopHint)
                emit("   %1%.set_tab_stop_hint(\"%2%\"_s);\n", id(), *iTabStopHint);
            if (iTextColor || iPaperColor)
                emit("   text_edit::style %1%DefaultStyle;\n", id());
            if (iTextColor)
            {
                if (std::holds_alternative<color>(*iTextColor))
                    emit("   %1%DefaultStyle.character().set_text_color(color{ %2% });\n", id(), std::get<color>(*iTextColor));
                else
                    emit("   %1%DefaultStyle.character().set_text_color(gradient{ %2% });\n", id(), std::get<gradient>(*iTextColor));
            }
            if (iPaperColor)
            {
                if (std::holds_alternative<color>(*iPaperColor))
                    emit("   %1%DefaultStyle.character().set_paper_color(color{ %2% });\n", id(), std::get<color>(*iPaperColor));
                else
                    emit("   %1%DefaultStyle.character().set_paper_color(gradient{ %2% });\n", id(), std::get<gradient>(*iPaperColor));
            }
            if (iTextColor || iPaperColor)
                emit("   %1%.set_default_style(%1%DefaultStyle);\n", id());
        }
    protected:
        using ui_element<>::emit;
    private:
        neolib::optional<size_hint> iSizeHint;
        neolib::optional<string> iTabStopHint;
        neolib::optional<color_or_gradient> iTextColor;
        neolib::optional<color_or_gradient> iPaperColor;
    };
}
