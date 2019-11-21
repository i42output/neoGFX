// menu.hpp
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

#include "action.hpp"

namespace neogfx::nrc
{
    class menu : public ui_element<>
    {
    public:
        menu(const i_ui_element_library& aLibrary, const i_ui_element_parser& aParser, i_ui_element& aParent) :
            ui_element<>{ aLibrary, aParser, aParent, aParser.get_optional<neolib::string>("id"), ui_element_type::Menu },
            iTitle{ aParent.parser().get_optional<neolib::string>("title") }
        {
        }
    public:
        const neolib::i_string& header() const override
        {
            static const neolib::string sHeader = "neogfx/gui/widget/menu.hpp";
            return sHeader;
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            if (aName == "action")
            {
                auto const& reference = aData.get<neolib::i_string>();
                if (reference == "separator")
                    new action_ref{ library(), parser(), *this };
                else
                    new action_ref{ library(), parser(), *this, reference };
            }
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
            emit("  menu %1%;\n", id());
            ui_element<>::emit_preamble();
        }
        void emit_ctor() const override
        {
            if ((parent().type() & ui_element_type::Menu) == ui_element_type::Menu)
            {
                if (iTitle)
                    emit(",\n"
                        "   %1%{ %2%, menu_type::Popup, \"%3%\"_t }", id(), parent().id(), *iTitle);
                else
                    emit(",\n"
                        "   %1%{ %2% }", id(), parent().id());
            }
            else
            {
                if (iTitle)
                    emit(",\n"
                        "   %1%{ menu_type::Popup, \"%3%\"_t }", id(), parent().id(), *iTitle);
            }
            ui_element<>::emit_ctor();
        }
        void emit_body() const override
        {
            ui_element<>::emit_body();
        }
    protected:
        using ui_element<>::emit;
    private:
        std::optional<neolib::string> iTitle;
    };
}
