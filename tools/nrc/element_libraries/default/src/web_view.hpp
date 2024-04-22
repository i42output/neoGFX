// web_view.hpp
/*
neoGFX Resource Compiler
Copyright(C) 2024 Leigh Johnston

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
    class web_view : public ui_element<>
    {
    public:
        web_view(const i_ui_element_parser& aParser, i_ui_element& aParent, ui_element_type aElementType = ui_element_type::WebView) :
            ui_element<>{ aParser, aParent, aElementType }
        {
            add_header("neogfx/gui/widget/web_view.hpp");
        }
        web_view(const i_ui_element_parser& aParser, i_ui_element& aParent, member_element_t, ui_element_type aElementType = ui_element_type::WebView) :
            ui_element<>{ aParser, aParent, member_element, aElementType }
        {
            add_header("neogfx/gui/widget/web_view.hpp");
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
            if (type() == ui_element_type::WebView)
            {
                emit("  %1% %2%;\n", type_name(), id());
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
            ui_element<>::emit_body();
        }
    protected:
        using ui_element<>::emit;
    };
}
