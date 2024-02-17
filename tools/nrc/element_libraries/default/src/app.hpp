// app.hpp
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
    class app : public ui_element<>
    {
    public:
        app(const i_ui_element_parser& aParser) :
            ui_element<>{ aParser, ui_element_type::App, aParser.get<neolib::string>("id") },
            iName{ aParser.get_optional<neolib::string>("name") },
            iDefaultWindowIcon{ aParser.get_optional<neolib::string>("default_window_icon") }
        {
            add_header("neogfx/app/app.hpp");
            add_data_names({ "name", "default_window_icon" });
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
            emit_preamble();
            emit("\n"
                "  template <typename... Args>\n"
                "  %1%(Args&&... aArgs) :\n", fragment_name().to_std_string_view());
            emit_ctor();
            emit("  {\n");
            emit_body();
            emit("  }\n");
        }
        void emit_preamble() const override
        {
            emit("  %1%& %2%;\n", type_name(), id());
            ui_element<>::emit_preamble();
        }
        void emit_ctor() const override
        {
            emit(
                "   %1%{ std::forward<Args>(aArgs)... }", type_name());
            emit(",\n"
                "   %1%{ *this }", id());

            ui_element<>::emit_ctor();
            emit("\n");
        }
        void emit_body() const override
        {
            if (iDefaultWindowIcon)
                emit("   set_default_window_icon(image{ \"%1%\" });\n", *iDefaultWindowIcon);
            ui_element<>::emit_body();
        }
    protected:
        using ui_element<>::emit;
    private:
        neolib::optional<neolib::string> iName;
        neolib::optional<neolib::string> iDefaultWindowIcon;
    };
}
