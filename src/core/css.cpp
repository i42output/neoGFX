// css.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/neogfx.hpp>

#include <string>

#include <neolib/file/lexer.hpp>
#include <neogfx/core/css.hpp>

namespace neogfx
{
    css::selector::selector(type_e aType, const arguments_type& aArguments) :
        iType(aType), iArguments(aArguments)
    {
    }

    css::selector::type_e css::selector::type() const
    {
        return iType;
    }

    css::declaration::declaration()
    {
    }

    css::css(std::string const& aStyle) : 
        iStyleSheet{ std::make_shared<std::istringstream>(aStyle) }
    {
        parse();
    }

    css::css(std::istream& aStyleSheet) :
        iStyleSheet{ std::shared_ptr<std::istream>{ std::shared_ptr<std::istream>{}, &aStyleSheet } }
    {
        parse();
    }

    void css::accept(i_visitor& aVisitor) const
    {
    }

    const css::rule_list& css::rules() const
    {
        return iRules;
    }

    std::string css::to_string() const
    {
        /* todo */
        return "";
    }

    void css::parse()
    {
    }
}
