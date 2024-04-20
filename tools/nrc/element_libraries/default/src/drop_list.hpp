// drop_list.hpp
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

#include <neogfx/gui/widget/drop_list.hpp>
#include <neogfx/tools/nrc/ui_element.hpp>

namespace neogfx::nrc
{
    class drop_list : public ui_element<>
    {
    public:
        drop_list(const i_ui_element_parser& aParser, i_ui_element& aParent) :
            ui_element<>{ aParser, aParent, ui_element_type::DropList }
        {
            add_header("neogfx/gui/widget/drop_list.hpp");
            add_data_names({ "style", "items" });
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            if (aName == "style")
                iStyle.first = get_enum<drop_list_style>(aData);
            else
                ui_element<>::parse(aName, aData);
        }
        void parse(const neolib::i_string& aName, const array_data_t& aData) override
        {
            if (aName == "style")
                iStyle.first = get_enum<drop_list_style>(aData, iStyle.second, "Normal");
            else if (aName == "items")
                iItems.emplace(aData);
            else
                ui_element<>::parse(aName, aData);
        }
    protected:
        void emit() const override
        {
        }
        void emit_preamble() const override
        {
            emit("  %1% %2%;\n", type_name(), id());
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
            if (iStyle.first)
            {
                if (!iStyle.second)
                    emit("   %1%.set_style(%2%);\n", id(), enum_to_string("drop_list_style", *iStyle.first));
                else
                    emit("   %1%.set_style(%1%.style() | %2%);\n", id(), enum_to_string("drop_list_style", *iStyle.first));
            }
            if (iItems)
            {
                for (auto const& item : *iItems)
                {
                    switch (item.which())
                    {
                    case neolib::simple_variant_type::Boolean:
                        emit("   %1%.model().insert_item(%1%.model().end(), %2%);\n", id(), item.get<bool>());
                        break;
                    case neolib::simple_variant_type::Integer:
                        emit("   %1%.model().insert_item(%1%.model().end(), %2%);\n", id(), item.get<std::int64_t>());
                        break;
                    case neolib::simple_variant_type::Real:
                        emit("   %1%.model().insert_item(%1%.model().end(), %2%);\n", id(), item.get<double>());
                        break;
                    case neolib::simple_variant_type::String:
                        emit("   %1%.model().insert_item(%1%.model().end(), \"%2%\");\n", id(), item.get<neogfx::i_string>().to_std_string());
                        break;
                    default:
                        throw unknown_data_type();
                    }
                }
            }
        }
    protected:
        using ui_element<>::emit;
    private:
        std::pair<std::optional<drop_list_style>, bool> iStyle;
        std::optional<concrete_array_data_t> iItems;
    };
}
