// line_edit.hpp
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
#include "text_edit.hpp"

namespace neogfx::nrc
{
    class line_edit : public text_edit
    {
    public:
        line_edit(const i_ui_element_parser& aParser, i_ui_element& aParent) :
            text_edit{ aParser, aParent, ui_element_type::LineEdit }
        {
            add_header("neogfx/gui/widget/line_edit.hpp");
        }
        line_edit(const i_ui_element_parser& aParser, i_ui_element& aParent, member_element_t) :
            text_edit{ aParser, aParent, member_element, ui_element_type::LineEdit }
        {
            add_header("neogfx/gui/widget/line_edit.hpp");
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            text_edit::parse(aName, aData);
        }
        void parse(const neolib::i_string& aName, const array_data_t& aData) override
        {
            text_edit::parse(aName, aData);
        }
    protected:
        void emit() const override
        {
        }
        void emit_preamble() const override
        {
            if (!is_member_element())
            {
                if (!iCaps)
                    emit("  %1% %2%;\n", type_name(), id());
                else
                    emit("  %1% %2%{ %3 };\n", type_name(), id(), enum_to_string("text_edit_caps", *iCaps));
            }
            text_edit::emit_preamble();
        }
        void emit_ctor() const override
        {
            text_edit::emit_ctor();
        }
        void emit_body() const override
        {
            text_edit::emit_body();
        }
    protected:
        using ui_element<>::emit;
    private:
    };
}
