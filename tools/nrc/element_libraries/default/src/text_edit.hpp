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
#include <neogfx/core/colour.hpp>
#include <neogfx/tools/nrc/ui_element.hpp>

namespace neogfx::nrc
{
    class text_edit : public ui_element<>
    {
    public:
        text_edit(const i_ui_element_parser& aParser, i_ui_element& aParent, ui_element_type aElementType = ui_element_type::TextEdit) :
            ui_element<>{ aParser, aParent, aElementType }
        {
            add_data_names({ "tab_stop_hint", "text_colour" });
        }
    public:
        const neolib::i_string& header() const override
        {
            static const neolib::string sHeader = "neogfx/gui/widget/text_edit.hpp";
            return sHeader;
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            if (aName == "tab_stop_hint")
                iTabStopHint = aData.get<neolib::i_string>();
            else if (aName == "text_colour")
                iTextColour = get_colour(aData);
            else if (aName == "background_colour")
                iBackgroundColour = get_colour(aData);
            else
                ui_element<>::parse(aName, aData);
        }
        void parse(const neolib::i_string& aName, const array_data_t& aData) override
        {
            if (aName == "text_colour")
                iTextColour = get_colour_or_gradient(aData);
            else if (aName == "background_colour")
                iBackgroundColour = get_colour_or_gradient(aData);
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
                emit("  text_edit %1%;\n", id());
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
            if (iTabStopHint)
                emit("   %1%.set_tab_stop_hint(\"%2%\");\n", id(), *iTabStopHint);
            if (iTextColour || iBackgroundColour)
                emit("   text_edit::style %1%DefaultStyle;\n", id());
            if (iTextColour)
            {
                if (std::holds_alternative<colour>(*iTextColour))
                    emit("   %1%DefaultStyle.set_text_colour(colour{ %2% });\n", id(), std::get<colour>(*iTextColour));
                else
                    emit("   %1%DefaultStyle.set_text_colour(gradient{ %2% });\n", id(), std::get<gradient>(*iTextColour));
            }
            if (iBackgroundColour)
            {
                if (std::holds_alternative<colour>(*iBackgroundColour))
                    emit("   %1%DefaultStyle.set_background_colour(colour{ %2% });\n", id(), std::get<colour>(*iBackgroundColour));
                else
                    emit("   %1%DefaultStyle.set_background_colour(gradient{ %2% });\n", id(), std::get<gradient>(*iBackgroundColour));
            }
            if (iTextColour || iBackgroundColour)
                emit("   %1%.set_default_style(%1%DefaultStyle);\n", id());
        }
    protected:
        using ui_element<>::emit;
    private:
        std::optional<string> iTabStopHint;
        std::optional<colour_or_gradient> iTextColour;
        std::optional<colour_or_gradient> iBackgroundColour;
    };
}
