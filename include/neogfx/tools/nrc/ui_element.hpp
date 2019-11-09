// ui_element.hpp
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
#include <neolib/reference_counted.hpp>
#include <neogfx/tools/nrc/i_ui_element.hpp>

namespace neogfx::nrc
{
    template <typename Base = i_ui_element>
    class ui_element : public neolib::reference_counted<Base>
    {
    public:
        ui_element(i_ui_element_parser& aParser, const neolib::i_string& aName, ui_element_type aType) :
            iParser{ aParser }, iParent { nullptr }, iName{ aName }, iType{ aType }
        {
        }
        ui_element(i_ui_element& aParent, const neolib::i_string& aName, ui_element_type aType) :
            iParser{ aParent.parser() }, iParent{ &aParent }, iName{ aName }, iType{ aType }
        {
        }
    public:
        const i_ui_element_parser& parser() const override
        {
            return iParser;
        }
        i_ui_element_parser& parser() override
        {
            return iParser;
        }
    public:
        const neolib::i_string& name() const override
        {
            return iName;
        }
        ui_element_type type() const override
        {
            return iType;
        }
    public:
        bool has_parent() const override
        {
            return iParent != nullptr;
        }
        const i_ui_element& parent() const override
        {
            if (has_parent())
                return *iParent;
            throw no_parent();
        }
        i_ui_element& parent() override
        {
            return const_cast<i_ui_element&>(to_const(*this).parent());
        }
    public:
        i_ui_element& parse(const neolib::i_string& aName) override
        {
        }
    public:
        void instantiate(i_app& aApp) override
        {
        }
        void instantiate(i_widget& aWidget) override
        {
        }
        void instantiate(i_layout& aLayout) override
        {
        }
    private:
        i_ui_element_parser& iParser;
        i_ui_element* iParent;
        neolib::string iName;
        ui_element_type iType;
    }
}
