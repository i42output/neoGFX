// toolbar.hpp
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
    class toolbar : public ui_element<>
    {
    public:
        toolbar(i_ui_element& aParent) :
            ui_element<>{ aParent, aParent.parser().get_optional<neolib::string>("id"), ui_element_type::MenuBar }
        {
        }
    public:
        const neolib::i_string& header() const override
        {
            static const neolib::string sHeader = "neogfx/gui/widget/toolbar.hpp";
            return sHeader;
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            if (aName == "action")
            {
                auto const& reference = aData.get<neolib::i_string>();
                if (reference == "separator")
                    new action_ref{ *this };
                else
                    new action_ref{ *this, reference };
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
            emit("  toolbar %1%;\n", id());
            ui_element<>::emit_preamble();
        }
        void emit_ctor() const override
        {
            if ((parent().type() & ui_element_type::MASK_RESERVED) == ui_element_type::Window)
                emit(",\n"
                    "   %1%{ %2%.toolbar_layout() }", id(), parent().id());
            else if (is_widget_or_layout(parent().type()))
                emit(",\n"
                    "   %1%{ %2% }", id(), parent().id());
            ui_element<>::emit_ctor();
        }
        void emit_body() const override
        {
            ui_element<>::emit_body();
        }
    protected:
        using ui_element<>::emit;
    private:
    };
}
