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
#include <boost/functional/hash.hpp>

#include <neolib/file/parser.hpp>
#include <neogfx/core/style_sheet.hpp>
#include <neogfx/gfx/color.hpp>

namespace neogfx
{
    template <>
    bool evaluate_style_sheet_value(i_style_sheet_value const& aValue, color& aResult)
    {
        thread_local std::unordered_map<std::pair<std::string_view, std::string_view>, color, boost::hash<std::pair<std::string_view, std::string_view>>> tCache;

        for (auto const& v : aValue)
        {
            auto const key = std::make_pair(v.first().to_std_string_view(), v.second().to_std_string_view());
            auto existing = tCache.find(key);
            if (existing != tCache.end())
            {
                aResult = existing->second;
                return true;
            }

            if (v.first() == "nss.color")
            {
                if (v.second().empty())
                    continue;
                if (v.second()[0] == '#')
                {
                    aResult = v.second().to_std_string();
                    tCache[key] = aResult;
                    return true;
                }
            }
        }
        return false;
    }

    style_sheet::style_sheet()
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

    style_sheet_value const& style_sheet::value(i_string_view const& aSelector, i_string_view const& aProperty) const
    {
        static style_sheet_value const tNoResult;

        auto selector = iSelectors.find(aSelector.to_std_string_view());
        if (selector != iSelectors.end())
        {
            auto property = selector->second.find(aProperty.to_std_string_view());
            if (property != selector->second.end())
                return property->second;
        }

        return tNoResult;
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
            BeginDeclarationBlock,
            EndDeclarationBlock,
            Declarations,
            Declaration,
            DeclarationSeparator,
            EndDeclaration,
            Property,
            Value,
            ValuePart,
            Identifier,
            Integer,
            Digit,
            Hex3,
            Hex4,
            Hex6,
            Hex8,
            HexDigit,
            Length
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
declare_symbol(neogfx::nss::symbol, BeginDeclarationBlock)
declare_symbol(neogfx::nss::symbol, EndDeclarationBlock)
declare_symbol(neogfx::nss::symbol, Declarations)
declare_symbol(neogfx::nss::symbol, Declaration)
declare_symbol(neogfx::nss::symbol, DeclarationSeparator)
declare_symbol(neogfx::nss::symbol, EndDeclaration)
declare_symbol(neogfx::nss::symbol, Property)
declare_symbol(neogfx::nss::symbol, Value)
declare_symbol(neogfx::nss::symbol, ValuePart)
declare_symbol(neogfx::nss::symbol, Identifier)
declare_symbol(neogfx::nss::symbol, Integer)
declare_symbol(neogfx::nss::symbol, Digit)
declare_symbol(neogfx::nss::symbol, Hex3)
declare_symbol(neogfx::nss::symbol, Hex4)
declare_symbol(neogfx::nss::symbol, Hex6)
declare_symbol(neogfx::nss::symbol, Hex8)
declare_symbol(neogfx::nss::symbol, HexDigit)
declare_symbol(neogfx::nss::symbol, Length)
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
            ( symbol::Identifier >> sequence(+(range('A', 'Z') | range('a', 'z')) , repeat(range('A', 'Z') | range('a', 'z') | range('0' , '9') | '-' ))),
            ( symbol::DeclarationBlock >> sequence(symbol::BeginDeclarationBlock , symbol::Declarations, symbol::EndDeclarationBlock) ),
            ( symbol::BeginDeclarationBlock >> '{'_ ),
            ( symbol::EndDeclarationBlock >> '}'_ ),
            ( symbol::Declarations >> sequence(symbol::Declaration , repeat(sequence(symbol::EndDeclaration , symbol::Declaration)), optional(symbol::EndDeclaration)) ),
            ( symbol::Declaration >> (sequence(symbol::Property <=> "nss.property"_concept), symbol::DeclarationSeparator, (symbol::Value <=> "nss.value"_concept)) ),
            ( symbol::DeclarationSeparator >> ':'_ ),
            ( symbol::EndDeclaration >> ';'_ ),
            ( symbol::Selector >> optional('.') , symbol::Identifier ),
            ( symbol::Property >> symbol::Identifier ),
            ( symbol::Value >> +repeat(symbol::ValuePart) ),
            ( symbol::ValuePart >> symbol::Identifier ),
            ( symbol::ValuePart >> (symbol::Length <=> "nss.length"_concept) ),
            ( symbol::ValuePart >> (symbol::Integer <=> "nss.integer"_concept) ),
            ( symbol::ValuePart >> (('#'_ , symbol::Hex3) <=> "nss.color"_concept) ),
            ( symbol::ValuePart >> (('#'_ , symbol::Hex4) <=> "nss.color"_concept) ),
            ( symbol::ValuePart >> (('#'_ , symbol::Hex6) <=> "nss.color"_concept) ),
            ( symbol::ValuePart >> (('#'_ , symbol::Hex8) <=> "nss.color"_concept) ),
            ( symbol::Length >> sequence(symbol::Integer, choice("cm"_ | "mm"_ | "in"_ | "px"_ | "pt"_ | "pc"_ | "em"_ | "ex"_ | "ch"_ | "rem"_ | "vw"_ | "vh"_ | "vmin"_ | "vmax"_ | "%"_ )) ),
            ( symbol::Integer >> +repeat(symbol::Digit) ),
            ( symbol::Digit >> range('0', '9') ),
            ( symbol::Hex3 >> sequence(symbol::HexDigit, symbol::HexDigit, symbol::HexDigit) ),
            ( symbol::Hex4 >> sequence(symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit) ),
            ( symbol::Hex6 >> sequence(symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit) ),
            ( symbol::Hex8 >> sequence(symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit) ),
            ( symbol::HexDigit >> choice(range('0', '9') | range('A', 'F') | range('a', 'f')) ),

            ( symbol::Eof >> discard(optional(symbol::Whitespace)), "" ),
            ( symbol::Whitespace >> +(' '_ | '\r' | '\n' | '\t' | symbol::Comment) ),
            ( symbol::Comment >> sequence("/*"_ , repeat(range('\0', '\xFF')) , "*/"_) ),
            ( symbol::Sheet >> discard(optional(symbol::Whitespace)) , symbol::Sheet , discard(optional(symbol::Whitespace)) ),
            ( symbol::Rule >> discard(optional(symbol::Whitespace)) , symbol::Rule , discard(optional(symbol::Whitespace)) ),
            ( symbol::Identifier >> discard(optional(symbol::Whitespace)) , symbol::Identifier , discard(optional(symbol::Whitespace)) ),
            ( symbol::Selector >> discard(optional(symbol::Whitespace)) , symbol::Selector , discard(optional(symbol::Whitespace)) ),
            ( symbol::DeclarationBlock >> discard(optional(symbol::Whitespace)) , symbol::DeclarationBlock , discard(optional(symbol::Whitespace)) ),
            ( symbol::BeginDeclarationBlock >> discard(optional(symbol::Whitespace)) , symbol::BeginDeclarationBlock , discard(optional(symbol::Whitespace)) ),
            ( symbol::EndDeclarationBlock >> discard(optional(symbol::Whitespace)) , symbol::EndDeclarationBlock , discard(optional(symbol::Whitespace)) ),
            ( symbol::Declarations >> discard(optional(symbol::Whitespace)) , symbol::Declarations , discard(optional(symbol::Whitespace)) ),
            ( symbol::Declaration >> discard(optional(symbol::Whitespace)) , symbol::Declaration , discard(optional(symbol::Whitespace)) ),
            ( symbol::DeclarationSeparator >> discard(optional(symbol::Whitespace)) , symbol::DeclarationSeparator , discard(optional(symbol::Whitespace)) ),
            ( symbol::EndDeclaration >> discard(optional(symbol::Whitespace)) , symbol::EndDeclaration , discard(optional(symbol::Whitespace)) ),
            ( symbol::Property >> discard(optional(symbol::Whitespace)) , symbol::Property , discard(optional(symbol::Whitespace)) ),
            ( symbol::Value >> discard(optional(symbol::Whitespace)) , symbol::Value , discard(optional(symbol::Whitespace)) ),
            ( symbol::ValuePart >> discard(optional(symbol::Whitespace)) , symbol::ValuePart , discard(optional(symbol::Whitespace)) )
        };
    }

    namespace
    {
        void create_values(style_sheet::selector_map& aSelectors, neolib::parser<nss::symbol>::ast_node const& aNode)
        {
            thread_local std::optional<style_sheet::selector_map::iterator> selector;
            thread_local std::optional<style_sheet::property_map::iterator> property;
            if (aNode.parent == nullptr)
            {
                selector = std::nullopt;
                property = std::nullopt;
            }
            if (aNode.c.has_value() && aNode.c.value() == "nss.selector")
                selector = aSelectors.emplace(aNode.value, style_sheet::property_map{}).first;
            else if (selector.has_value() && aNode.c.has_value() && aNode.c.value() == "nss.property")
                property = selector.value()->second.emplace(string{ aNode.value }, style_sheet_value{}).first;
            else if (property.has_value())
                property.value()->second.emplace_back(string{ aNode.c.value() }, string{ aNode.value });
            for (auto const& child : aNode.children)
                create_values(aSelectors, *child);
        }
    }

    void style_sheet::parse()
    {
        neolib::parser<nss::symbol> parser{ nss::sRules };
        parser.set_debug_output(std::cout);
        parser.parse(nss::symbol::Sheet, sheet().to_std_string_view());
        parser.create_ast();
        create_values(iSelectors, parser.ast());
    }
}
