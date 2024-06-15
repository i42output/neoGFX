// css.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.
  
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

#include <neolib/file/parser.hpp>
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
        iStyleSheet{ std::shared_ptr<std::istream>{ std::shared_ptr<std::istream>{},& aStyleSheet } }
    {
        parse();
    }

    void css::accept(i_visitor& aVisitor) const
    {
    }

    std::string css::to_string() const
    {
        /* todo */
        return "";
    }

    namespace css_parser
    {
        enum class symbol
        {
            Sheet,
            Eof,
            Whitespace,
            Comment,
            Rule,
            Selector,
            DeclarationBlock,
            Declaration,
            Property,
            Value,
            Identifier
        };
    }
}

declare_symbols(neogfx::css_parser::symbol)
declare_symbol(neogfx::css_parser::symbol, Sheet)
declare_symbol(neogfx::css_parser::symbol, Eof)
declare_symbol(neogfx::css_parser::symbol, Whitespace)
declare_symbol(neogfx::css_parser::symbol, Comment)
declare_symbol(neogfx::css_parser::symbol, Rule)
declare_symbol(neogfx::css_parser::symbol, Selector)
declare_symbol(neogfx::css_parser::symbol, DeclarationBlock)
declare_symbol(neogfx::css_parser::symbol, Declaration)
declare_symbol(neogfx::css_parser::symbol, Property)
declare_symbol(neogfx::css_parser::symbol, Value)
declare_symbol(neogfx::css_parser::symbol, Identifier)
end_declare_symbols(neogfx::css_parser::symbol)

namespace neogfx
{
    namespace css_parser
    {
        enable_neolib_parser(symbol)

        neolib::parser_rule<symbol> const sRules[] =
        {
            ( symbol::Sheet >> repeat(symbol::Rule) , discard(symbol::Eof) ),
            ( symbol::Rule >> symbol::Selector , symbol::DeclarationBlock ),
            ( symbol::Identifier >> sequence((range('A', 'Z') | range('a', 'z')) , repeat(range('A', 'Z') | range('a', 'z') | range('0' , '9'))) ),
            ( symbol::DeclarationBlock >> '{' , symbol::Declaration , repeat(sequence( ';' , symbol::Declaration )) ),
            ( symbol::Declaration >> symbol::Property , ':', symbol::Value ),

            ( symbol::Eof >> discard(symbol::Whitespace), "" ),
            ( symbol::Whitespace >> (' '_ | '\r' | '\n' | '\t') ),
            ( symbol::Whitespace >> symbol::Comment ),
            ( symbol::Comment >> discard(("/*"_ , repeat(range('\0', '\xFF')) , "*/"_)) ),
            ( symbol::Sheet >> discard(symbol::Whitespace) , symbol::Sheet , discard(symbol::Whitespace) ),
            ( symbol::Rule >> discard(symbol::Whitespace) , symbol::Rule , discard(symbol::Whitespace) ),
            ( symbol::Selector >> discard(symbol::Whitespace) , symbol::Selector , discard(symbol::Whitespace) ),
            ( symbol::DeclarationBlock >> discard(symbol::Whitespace) , symbol::DeclarationBlock , discard(symbol::Whitespace) ),
            ( symbol::Declaration >> discard(symbol::Whitespace) , symbol::Declaration , discard(symbol::Whitespace) ),
            ( symbol::Property >> discard(symbol::Whitespace) , symbol::Property , discard(symbol::Whitespace) ),
            ( symbol::Value >> discard(symbol::Whitespace) , symbol::Value , discard(symbol::Whitespace) )
        };
    }

    void css::parse()
    {
    }
}
