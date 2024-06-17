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

    style_sheet::style_sheet(i_style_sheet const& aSheet) :
        iSheet{ std::make_shared<std::string>(aSheet.sheet().to_std_string_view()) }
    {
        parse();
    }

    style_sheet::style_sheet(std::string const& aSheet) : 
        iSheet{ std::make_shared<std::string>(aSheet) }
    {
        parse();
    }

    style_sheet::style_sheet(std::string_view const& aSheet) :
        iSheet{ std::make_shared<std::string>(aSheet) }
    {
        parse();
    }

    style_sheet::style_sheet(std::istream& aSheet)
    {
        std::ostringstream oss;
        oss << aSheet.rdbuf();
        iSheet = std::make_shared<std::string>(oss.str());
        parse();
    }

    style_sheet& style_sheet::operator=(i_style_sheet const& aSheet)
    {
        iSheet = std::make_shared<std::string>(aSheet.sheet().to_std_string_view());
        iSheetView = std::nullopt;
        parse();
        return *this;
    }

    style_sheet& style_sheet::operator=(std::string const& aSheet)
    {
        iSheet = std::make_shared<std::string>(aSheet);
        iSheetView = std::nullopt;
        parse();
        return *this;
    }

    style_sheet& style_sheet::operator=(std::string_view const& aSheet)
    {
        iSheet = std::make_shared<std::string>(aSheet);
        iSheetView = std::nullopt;
        parse();
        return *this;
    }

    style_sheet& style_sheet::operator=(std::istream& aSheet)
    {
        std::ostringstream oss;
        oss << aSheet.rdbuf();
        iSheet = std::make_shared<std::string>(oss.str());
        iSheetView = std::nullopt;
        parse();
        return *this;
    }

    i_string_view& style_sheet::sheet() const
    {
        if (!iSheetView.has_value())
            iSheetView.emplace(*iSheet);
        return iSheetView.value();
    }

    void style_sheet::accept(i_visitor& aVisitor) const
    {
    }

    std::string style_sheet::to_string() const
    {
        return sheet().to_std_string();
    }

    namespace nss
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
            Declarations,
            Declaration,
            Property,
            Value,
            Identifier,
            Hex3,
            Hex6,
            Hex8,
            HexDigit
        };
    }
}

declare_symbols(neogfx::nss::symbol)
declare_symbol(neogfx::nss::symbol, Sheet)
declare_symbol(neogfx::nss::symbol, Eof)
declare_symbol(neogfx::nss::symbol, Whitespace)
declare_symbol(neogfx::nss::symbol, Comment)
declare_symbol(neogfx::nss::symbol, Rule)
declare_symbol(neogfx::nss::symbol, Selector)
declare_symbol(neogfx::nss::symbol, DeclarationBlock)
declare_symbol(neogfx::nss::symbol, Declarations)
declare_symbol(neogfx::nss::symbol, Declaration)
declare_symbol(neogfx::nss::symbol, Property)
declare_symbol(neogfx::nss::symbol, Value)
declare_symbol(neogfx::nss::symbol, Identifier)
declare_symbol(neogfx::nss::symbol, Hex3)
declare_symbol(neogfx::nss::symbol, Hex6)
declare_symbol(neogfx::nss::symbol, Hex8)
declare_symbol(neogfx::nss::symbol, HexDigit)
end_declare_symbols(neogfx::nss::symbol)

namespace neogfx
{
    namespace nss
    {
        enable_neolib_parser(symbol)

        neolib::parser_rule<symbol> const sRules[] =
        {
            ( symbol::Sheet >> repeat(symbol::Rule) , discard(symbol::Eof) ),
            ( symbol::Rule >> (symbol::Selector <=> "nss.selector"_concept), symbol::DeclarationBlock),
            ( symbol::Rule >> symbol::Declarations ),
            ( symbol::Identifier >> sequence((range('A', 'Z') | range('a', 'z')) , repeat(range('A', 'Z') | range('a', 'z') | range('0' , '9') | '-' ))),
            ( symbol::DeclarationBlock >> '{' , symbol::Declarations, '}' ),
            ( symbol::Declarations >> symbol::Declaration , repeat(sequence(';' , symbol::Declaration)), optional(';')),
            ( symbol::Declaration >> (sequence((symbol::Property <=> "nss.property"_concept), ':', (symbol::Value <=> "nss.value"_concept)) <=> "nss.declaration"_concept) ),
            ( symbol::Selector >> optional('.') , symbol::Identifier ),
            ( symbol::Property >> symbol::Identifier ),
            ( symbol::Value >> symbol::Identifier ),
            ( symbol::Value >> '#'_ , symbol::Hex3 ),
            ( symbol::Value >> '#'_ , symbol::Hex6 ),
            ( symbol::Value >> '#'_ , symbol::Hex8 ),
            ( symbol::Hex3 >> sequence(symbol::HexDigit, symbol::HexDigit, symbol::HexDigit ) ),
            ( symbol::Hex6 >> sequence(symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit) ),
            ( symbol::Hex8 >> sequence(symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit) ),
            ( symbol::HexDigit >> choice(range('0', '9') | range('A', 'F') | range('a', 'f')) ),

            ( symbol::Eof >> discard(symbol::Whitespace), "" ),
            ( symbol::Whitespace >> (' '_ | '\r' | '\n' | '\t' | symbol::Comment) ),
            ( symbol::Comment >> sequence("/*"_ , repeat(range('\0', '\xFF')) , "*/"_) ),
            ( symbol::Sheet >> discard(symbol::Whitespace) , symbol::Sheet , discard(symbol::Whitespace) ),
            ( symbol::Rule >> discard(symbol::Whitespace) , symbol::Rule , discard(symbol::Whitespace) ),
            ( symbol::Identifier >> discard(symbol::Whitespace) , symbol::Identifier , discard(symbol::Whitespace) ),
            ( symbol::Selector >> discard(symbol::Whitespace) , symbol::Selector , discard(symbol::Whitespace) ),
            ( symbol::DeclarationBlock >> discard(symbol::Whitespace) , symbol::DeclarationBlock , discard(symbol::Whitespace) ),
            ( symbol::Declarations >> discard(symbol::Whitespace) , symbol::Declarations , discard(symbol::Whitespace) ),
            ( symbol::Declaration >> discard(symbol::Whitespace) , symbol::Declaration , discard(symbol::Whitespace) ),
            ( symbol::Property >> discard(symbol::Whitespace) , symbol::Property , discard(symbol::Whitespace) ),
            ( symbol::Value >> discard(symbol::Whitespace) , symbol::Value , discard(symbol::Whitespace) )
        };
    }

    void style_sheet::parse()
    {
        neolib::parser<nss::symbol> parser{ nss::sRules };
        parser.set_debug_output(std::cout);
        parser.parse(nss::symbol::Sheet, sheet().to_std_string_view());
        parser.create_ast();
    }
}
