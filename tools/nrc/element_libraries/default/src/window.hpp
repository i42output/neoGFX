// window.hpp
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
    class window : public ui_element<>
    {
    public:
        window(i_ui_element& aParent) :
            ui_element<>{ aParent, aParent.parser().get<neolib::i_string>("id"), ui_element_type::Widget }
        {
            if (aParent.parser().data_exists("default_size"))
            {
                auto const& ds = aParent.parser().get_array_data("default_size");
                auto get_length = [](auto&& av) -> length
                {
                    length result;
                    std::visit([&result](auto&& v)
                    {
                        typedef std::remove_const_t<std::remove_reference_t<decltype(v)>> vt;
                        if constexpr (std::is_integral_v<vt>)
                            result = v;
                        else if constexpr (std::is_same_v<vt, neolib::i_string>)
                            result = length::from_string(v.to_std_string());
                        else
                            throw wrong_type();
                    }, av);
                    return result;
                };
                if (ds.size() >= 2u)
                    iDefaultSize.emplace(get_length(ds[0]), get_length(ds[1]));
                else if (ds.size() >= 1u)
                    iDefaultSize.emplace(get_length(ds[0]), get_length(ds[0]));
                else
                    iDefaultSize.emplace();
            }
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
            emit("  neogfx::window %1%;\n", id());
            for (auto const& child : children())
                child->emit_preamble();
        }
        void emit_ctor() const override
        {
            switch(parent().type())
            {
            case ui_element_type::Widget:
                if (iDefaultSize)
                    emit(",\n"
                        "   %1%{ %2%, %3%, %4% }", id(), parent().id(), iDefaultSize->cx, iDefaultSize->cy);
                else
                    emit(",\n"
                        "   %1%{ %2% }", id(), parent().id());
                break;
            default:
                if (iDefaultSize)
                    emit(",\n"
                        "   %1%{ neogfx::size{ %2%, %3% } }", id(), iDefaultSize->cx, iDefaultSize->cy);
                break;
            }
           
        }
        void emit_body() const override
        {
        }
    protected:
        using ui_element<>::emit;
    private:
        std::optional<neogfx::basic_size<length>> iDefaultSize;
    };
}
