// style_sheet.cpp
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
#include <neogfx/core/style_sheet.hpp>

namespace neogfx
{
    style_sheet::selector::selector(type_e aType, const arguments_type& aArguments) :
        iType(aType), iArguments(aArguments)
    {
    }

    style_sheet::selector::type_e style_sheet::selector::type() const
    {
        return iType;
    }

    style_sheet::declaration::declaration()
    {
    }

    style_sheet::style_sheet(i_style_sheet const& aStyle)
    {
    }

    style_sheet::style_sheet(std::string const& aStyle) :
        iStyleSheet{ std::make_shared<std::istringstream>(aStyle) }
    {
        parse();
    }

    style_sheet::style_sheet(std::string_view const& aStyle) :
        iStyleSheet{ std::make_shared<std::istringstream>(std::string{ aStyle }) }
    {
        parse();
    }

    style_sheet::style_sheet(std::istream& aStyleSheet) :
        iStyleSheet{ std::shared_ptr<std::istream>{ std::shared_ptr<std::istream>{}, &aStyleSheet } }
    {
        parse();
    }

    style_sheet& style_sheet::operator=(std::string const& aStyle)
    {
        iStyleSheet = std::make_shared<std::istringstream>(aStyle);
        parse();
        return *this;
    }

    style_sheet& style_sheet::operator=(std::string_view const& aStyle)
    {
        iStyleSheet = std::make_shared<std::istringstream>(std::string{ aStyle });
        parse();
        return *this;
    }

    style_sheet& style_sheet::operator=(std::istream& aStyleSheet)
    {
        iStyleSheet = std::shared_ptr<std::istream>{ std::shared_ptr<std::istream>{}, &aStyleSheet };
        parse();
        return *this;
    }

    void style_sheet::accept(i_visitor& aVisitor) const
    {
    }

    std::string style_sheet::to_string() const
    {
        /* todo */
        return "";
    }

    namespace style_sheet_parser
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

declare_symbols(neogfx::style_sheet_parser::symbol)
declare_symbol(neogfx::style_sheet_parser::symbol, Sheet)
declare_symbol(neogfx::style_sheet_parser::symbol, Eof)
declare_symbol(neogfx::style_sheet_parser::symbol, Whitespace)
declare_symbol(neogfx::style_sheet_parser::symbol, Comment)
declare_symbol(neogfx::style_sheet_parser::symbol, Rule)
declare_symbol(neogfx::style_sheet_parser::symbol, Selector)
declare_symbol(neogfx::style_sheet_parser::symbol, DeclarationBlock)
declare_symbol(neogfx::style_sheet_parser::symbol, Declaration)
declare_symbol(neogfx::style_sheet_parser::symbol, Property)
declare_symbol(neogfx::style_sheet_parser::symbol, Value)
declare_symbol(neogfx::style_sheet_parser::symbol, Identifier)
end_declare_symbols(neogfx::style_sheet_parser::symbol)

namespace neogfx
{
    namespace style_sheet_parser
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

    void style_sheet::parse()
    {
    }
}
