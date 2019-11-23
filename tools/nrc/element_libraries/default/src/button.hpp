// basic_button.hpp
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
#include <neogfx/gui/widget/i_button.hpp>
#include <neogfx/tools/nrc/ui_element.hpp>

namespace neogfx::nrc
{
    template<ui_element_type ButtonType>
    class basic_button : public ui_element<>
    {
    public:
        basic_button(const i_ui_element_parser& aParser, i_ui_element& aParent) :
            ui_element<>{ aParser, aParent, aParser.get_optional<neolib::string>("id"), ButtonType }
        {
        }
    public:
        const neolib::i_string& header() const override
        {
            switch (ButtonType)
            {
            case ui_element_type::PushButton:
            {
                static const neolib::string sHeader = "neogfx/gui/widget/push_button.hpp";
                return sHeader;
            }
            case ui_element_type::CheckBox:
            {
                static const neolib::string sHeader = "neogfx/gui/widget/check_box.hpp";
                return sHeader;
            }
            case ui_element_type::RadioButton:
            {
                static const neolib::string sHeader = "neogfx/gui/widget/radio_button.hpp";
                return sHeader;
            }
            default:
                throw wrong_type();
            }
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            if (aName == "text")
                iText = aData.get<neolib::i_string>();
            else if (aName == "checkable")
                iCheckable = parser().get_optional_enum<button_checkable>(aName);
            else
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
            emit("  basic_button %1%;\n", id());
            ui_element<>::emit_preamble();
        }
        void emit_ctor() const override
        {
            if ((parent().type() & ui_element_type::MASK_RESERVED_GENERIC) == ui_element_type::Window)
            {
                if (iText)
                    emit(",\n"
                        "   %1%{ %2%.client_layout(), \"%3%\"_t }", id(), parent().id(), *iText);
                else
                    emit(",\n"
                        "   %1%{ %2%.client_layout() }", id(), parent().id());
            }
            else if (is_widget_or_layout(parent().type()))
            {
                if (iText)
                    emit(",\n"
                        "   %1%{ %2%, \"%3%\"_t }", id(), parent().id(), *iText);
                else
                    emit(",\n"
                        "   %1%{ %2% }", id(), parent().id());
            }
            ui_element<>::emit_ctor();
        }
        void emit_body() const override
        {
            if (iCheckable)
                emit("   %1%.set_checkable(button_checkable::%2%);\n", id(), neolib::enum_to_string(*iCheckable));
            ui_element<>::emit_body();
        }
    protected:
        using ui_element<>::emit;
    private:
        std::optional<neolib::string> iText;
        std::optional<button_checkable> iCheckable;
    };

    typedef basic_button<ui_element_type::PushButton> push_button;
    typedef basic_button<ui_element_type::CheckBox> check_box;
    typedef basic_button<ui_element_type::RadioButton> radio_button;
}
