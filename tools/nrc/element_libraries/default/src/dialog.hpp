// dialog.hpp
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
#include <neogfx/gui/dialog/dialog_button_box.hpp>
#include "window.hpp"

namespace neogfx::nrc
{
    class dialog : public window
    {
    public:
        dialog(const i_ui_element_parser& aParser) :
            window{ aParser, ui_element_type::Dialog }
        {
            add_header("neogfx/gui/dialog/dialog.hpp");
            add_data_names({ "standard_layout", "button_box" });
        }
        dialog(const i_ui_element_parser& aParser, i_ui_element& aParent) :
            window{ aParser, aParent, ui_element_type::Dialog }
        {
            add_header("neogfx/gui/dialog/dialog.hpp");
            add_data_names({ "standard_layout", "button_box" });
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            if (aName == "standard_layout")
                iStandardLayout.emplace(get_scalar<length>(aData));
            else if (aName == "button_box")
                iButtonBox = get_enum<standard_button>(aData);
            else
                window::parse(aName, aData);
        }
        void parse(const neolib::i_string& aName, const array_data_t& aData) override
        {
            if (aName == "standard_layout")
                emplace_2<length>("standard_layout", iStandardLayout);
            else if (aName == "button_box")
                iButtonBox = get_enum<standard_button>(aData);
            else
                window::parse(aName, aData);
        }
    protected:
        void emit() const override
        {
            window::emit();
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
            window::emit_ctor();
        }
        void emit_body() const override
        {
            if (iStandardLayout)
                emit("   set_standard_layout(size{ %1%, %2% });\n", iStandardLayout->cx, iStandardLayout->cy);
            if (iButtonBox)
                emit("   button_box().add_buttons(%1%);\n", enum_to_string("standard_button", *iButtonBox));
            window::emit_body();
        }
    protected:
        using ui_element<>::emit;
    private:
        neolib::optional<basic_size<length>> iStandardLayout;
        neolib::optional<standard_button> iButtonBox;
    };
}
