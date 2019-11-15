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
#include <boost/format.hpp>
#include <neogfx/tools/nrc/ui_element.hpp>

namespace neogfx::nrc
{
    class app : public ui_element<>
    {
    public:
        app(const i_ui_element_parser& aParser) :
            ui_element<>{ aParser, aParser.get<neolib::i_string>("id"), ui_element_type::App },
            iName{ aParser.get_optional<neolib::string>("name")}
        {
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
        }
        void parse(const neolib::i_string& aName, const array_data_t& aData) override
        {
        }
        void emit() const override
        {
            emit_preamble();
            parser().emit(neolib::string{
                (boost::format(
                    "\n"
                    "  ui(int argc, char* argv[], const std::string& aName = {%1%}) :\n") % (iName ? "\"" + *iName + "\"" : "")
                ).str() });
            emit_ctor();
            parser().emit(neolib::string{
                (boost::format(
                    "  {\n")
                ).str() });
            emit_body();
            parser().emit(neolib::string{
                (boost::format(
                    "  }\n")
                ).str() });
        }
        void emit_preamble() const override
        {
            parser().emit(neolib::string{
                (boost::format(
                    "  neogfx::app %1%;\n") % id()
                ).str() });
            for (auto const& child : children())
                child->emit_preamble();
        }
        void emit_ctor() const override
        {
            parser().emit(neolib::string{
                (boost::format(
                    "   %1%{ argc, argv, aName }\n") % id()
                ).str() });
            for (auto const& child : children())
                child->emit_ctor();
        }
        void emit_body() const override
        {
            for (auto const& child : children())
                child->emit_body();
        }
    private:
        std::optional<neolib::string> iName;
    };
}
