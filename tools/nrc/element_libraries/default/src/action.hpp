// action.hpp
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
    class action : public ui_element<>
    {
    public:
        action(i_ui_element& aParent) :
            ui_element<>{ aParent, aParent.parser().get<neolib::i_string>("id"), ui_element_type::Action },
            iCheckable{ aParent.parser().get_optional<bool>("checkable") },
            iText{ aParent.parser().get_optional<neolib::string>("text") },
            iImage{ aParent.parser().get_optional<neolib::string>("image") },
            iShortcut{ aParent.parser().get_optional<neolib::string>("shortcut") },
            iCheckedImage{ aParent.parser().get_optional<neolib::string>("checked_image") }
        {
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
        }
        void parse(const neolib::i_string& aName, const array_data_t& aData) override
        {
        }
    protected:
        void emit() const override
        {
        }
        void emit_preamble() const override
        {
            emit("  neogfx::action %1%;\n", id());
            ui_element<>::emit_preamble();
        }
        void emit_ctor() const override
        {
            if (iText)
                emit(",\n"
                    "   %1%{ \"%2%\"_t }", id(), *iText);
            ui_element<>::emit_ctor();
        }
        void emit_body() const override
        {
            if (iCheckable)
                emit("   %1%.set_checkable(%2%);\n", id(), *iCheckable);
            if (iImage)
                emit("   %1%.set_image(\"%2%\");\n", id(), *iImage);
            if (iShortcut)
                emit("   %1%.set_shortcut(\"%2%\");\n", id(), *iShortcut);
            if (iCheckedImage)
                emit("   %1%.set_checked_image(\"%2%\");\n", id(), *iCheckedImage);
            emit("   %1%.add_action(%2%);\n", parent().id(), id());
            ui_element<>::emit_body();
        }
    protected:
        using ui_element<>::emit;
    private:
        std::optional<bool> iCheckable;
        std::optional<neolib::string> iText;
        std::optional<neolib::string> iImage;
        std::optional<neolib::string> iShortcut;
        std::optional<neolib::string> iCheckedImage;
    };
}
