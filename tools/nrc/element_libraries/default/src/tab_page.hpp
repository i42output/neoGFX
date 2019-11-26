// tab_page.hpp
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
    class tab_page : public ui_element<>
    {
    public:
        tab_page(const i_ui_element_parser& aParser, i_ui_element& aParent) :
            ui_element<>{ aParser, aParent, ui_element_type::TabPage },
            iTab{ aParent.parser().get_optional<neolib::string>("tab") },
            iTabText{ aParent.parser().get_optional<neolib::string>("tab_text") },
            iTabImage{ aParent.parser().get_optional<neolib::string>("tab_image") }
        {
            add_data_names({ "tab_text", "tab_image" });
        }
    public:
        const neolib::i_string& header() const override
        {
            static const neolib::string sHeader = "neogfx/gui/widget/tab_page.hpp";
            return sHeader;
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            ui_element<>::parse(aName, aData);
        }
        void parse(const neolib::i_string& aName, const array_data_t& aData) override
        {
            ui_element<>::parse(aName, aData);
        }
    protected:
        void emit() const override
        {
        }
        void emit_preamble() const override
        {
            emit("  tab_page %1%;\n", id());
            ui_element<>::emit_preamble();
        }
        void emit_ctor() const override
        {
            if ((parent().type() & ui_element_type::MASK_RESERVED_SPECIFIC) == ui_element_type::TabPageContainer)
            {
                if (iTab && !iTabText)
                    emit(",\n"
                        "   %1%{ %2%, %3% }", id(), parent().id(), *iTab);
                else if (!iTab && iTabText)
                    emit(",\n"
                        "   %1%{ %2%, \"%3%\"_t }", id(), parent().id(), *iTabText);
                else
                    throw element_ill_formed(id().to_std_string());
            }
            else
                ui_element<>::emit_generic_ctor();
            ui_element<>::emit_ctor();
        }
        void emit_body() const override
        {
            if (iTabImage)
                emit("   %1%.tab().set_image(image{ \"%2%\" });\n", id(), *iTabImage);
            ui_element<>::emit_body();
        }
    protected:
        using ui_element<>::emit;
    private:
        std::optional<neolib::string> iTab;
        std::optional<neolib::string> iTabText;
        std::optional<neolib::string> iTabImage;
    };
}
