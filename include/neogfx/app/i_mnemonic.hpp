// i_mnemonic.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/string_utils.hpp>

namespace neogfx
{
    class i_widget;

    class i_mnemonic
    {
    public:
        virtual std::string mnemonic() const = 0;
        virtual void mnemonic_execute() = 0;
        virtual i_widget& mnemonic_widget() = 0;
    };

    inline std::string mnemonic_from_text(const std::string& aText, char aMnemonicPrefix = '&')
    {
        auto u = neolib::utf8_to_utf32(aText);
        for (std::size_t i = 0; i < u.size(); ++i)
        {
            if (u[i] == static_cast<char32_t>(aMnemonicPrefix) && i < u.size() - 1 && u[i + 1] != static_cast<char32_t>(aMnemonicPrefix))
            {
                return neolib::utf32_to_utf8(u.substr(i + 1, 1));
            }
        }
        return std::string{};
    }
}