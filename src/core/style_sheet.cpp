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
    namespace
    {
        template <typename T>
        using value_map_t = std::unordered_map<std::pair<std::string_view, std::string_view>, T, boost::hash<std::pair<std::string_view, std::string_view>>>;

        template <typename T>
        value_map_t<T>& value_map()
        {
            thread_local value_map_t<T> tValueMap;
            return tValueMap;
        }

        template <typename T>
        typename value_map_t<T>::iterator value_map_find(typename value_map_t<T>::key_type aKey)
        {
            return value_map<T>().find(aKey);
        }

        template <typename T>
        typename value_map_t<T>::iterator value_map_find(std::string_view const& aSelector, std::string_view const& aProperty)
        {
            return value_map<T>().find(typename value_map_t<T>::key_type{ aSelector, aProperty });
        }

        template <typename T>
        typename T& value_map_entry(std::string_view const& aSelector, std::string_view const& aProperty)
        {
            return value_map<T>()[typename value_map_t<T>::key_type{aSelector, aProperty}];
        }
    }

    template <>
    color const* evaluate_style_sheet_value<color>(i_style_sheet_value const& aValue)
    {
        using value_type = color;

        for (auto const& v : aValue)
        {
            auto existing = value_map_find<value_type>(v.first().to_std_string_view(), v.second().to_std_string_view());
            if (existing != value_map<value_type>().end())
                return &existing->second;

            if (v.first() == "nss.color")
            {
                if (v.second().empty())
                    continue;
                if (v.second()[0] == '#')
                    return &(value_map_entry<value_type>(v.first().to_std_string_view(), v.second().to_std_string_view()) = v.second().to_std_string());
            }
        }

        return nullptr;
    }

    template <>
    std::vector<length> const* evaluate_style_sheet_value<std::vector<length>>(i_style_sheet_value const& aValue)
    {
        using value_type = std::vector<length>;

        value_type* result = nullptr;

        for (auto const& v : aValue)
        {
            auto existing = value_map_find<value_type>(v.first().to_std_string_view(), v.second().to_std_string_view());
            if (existing != value_map<value_type>().end())
                return &existing->second;

            if (v.first() == "nss.value")
            {
                if (v.second().empty())
                    continue;
                auto& lengths = value_map_entry<value_type>(v.first().to_std_string_view(), v.second().to_std_string_view());
                result = &lengths;
            }
            else if (v.first() == "nss.length" && result)
            {
                if (v.second().empty())
                    continue;
                result->push_back(length::from_string(v.second()));
            }
        }

        return result;
    }

    template <>
    std::tuple<std::optional<color>, std::optional<length>, std::optional<border_style>> const* evaluate_style_sheet_value<std::tuple<std::optional<color>, std::optional<length>, std::optional<border_style>>>(i_style_sheet_value const& aValue)
    {
        using value_type = std::tuple<std::optional<color>, std::optional<length>, std::optional<border_style>>;

        value_type* result = nullptr;

        for (auto const& v : aValue)
        {
            auto existing = value_map_find<value_type>(v.first().to_std_string_view(), v.second().to_std_string_view());
            if (existing != value_map<value_type>().end())
                return &existing->second;

            if (v.second().empty())
                continue;

            if (v.first() == "nss.value")
            {
                auto& args = value_map_entry<value_type>(v.first().to_std_string_view(), v.second().to_std_string_view());
                result = &args;
            }
            else if (v.first() == "nss.length" && result)
            {
                std::get<1>(*result) = length::from_string(v.second());
            }
            else if (v.first() == "nss.color" && result)
            {
                std::get<0>(*result) = v.second().to_std_string();
            }
            else if (v.first() == "nss.border_style" && result)
            {
                std::get<2>(*result) = neolib::string_to_enum<border_style>(v.second().to_std_string());
            }
        }

        return result;
    }

    template <>
    std::optional<border_style> const* evaluate_style_sheet_value<std::optional<border_style>>(i_style_sheet_value const& aValue)
    {
        using value_type = std::optional<border_style>;

        value_type* result = nullptr;

        for (auto const& v : aValue)
        {
            auto existing = value_map_find<value_type>(v.first().to_std_string_view(), v.second().to_std_string_view());
            if (existing != value_map<value_type>().end())
                return &existing->second;

            if (v.second().empty())
                continue;

            // todo: four cardinal borders

            if (v.first() == "nss.value")
            {
                auto& borderStyle = value_map_entry<value_type>(v.first().to_std_string_view(), v.second().to_std_string_view());
                result = &borderStyle;
                *result = neolib::string_to_enum<border_style>(v.second().to_std_string());
            }
        }

        return result;
    }

    template <>
    std::optional<std::array<std::array<length, 2u>, 4u>> const* evaluate_style_sheet_value<std::optional<std::array<std::array<length, 2u>, 4u>>>(i_style_sheet_value const& aValue)
    {    
        using value_type = std::optional<std::array<std::array<length, 2u>, 4u>>;

        value_type* result = nullptr;

        std::pair<std::vector<length>, std::vector<length>> partialResult;
        bool secondPair = false;

        for (auto const& v : aValue)
        {
            auto existing = value_map_find<value_type>(v.first().to_std_string_view(), v.second().to_std_string_view());
            if (existing != value_map<value_type>().end())
                return &existing->second;

            if (v.first() == "nss.value" || v.first() == "nss.paired_values")
            {
                if (v.second().empty())
                    continue;
                auto& lengths = value_map_entry<value_type>(v.first().to_std_string_view(), v.second().to_std_string_view());
                result = &lengths;
            }
            else if (v.first() == "nss.pair_separator" && result)
            {
                secondPair = true;
            }
            else if (v.first() == "nss.length" && result)
            {
                if (v.second().empty())
                    continue;
                if (!secondPair)
                    partialResult.first.push_back(length::from_string(v.second()));
                else
                    partialResult.second.push_back(length::from_string(v.second()));
            }
        }

        if (result)
        {
            std::array<std::array<std::size_t, 4u>, 4u> constexpr indices =
            { {
                { 0, 0, 0, 0 },
                { 0, 1, 0, 1 },
                { 0, 1, 2, 1 },
                { 0, 1, 2, 3 }
            } };
            if (!partialResult.first.empty())
            {
                if (!result->has_value())
                    result->emplace();
                result->value()[0][0] = partialResult.first[indices[partialResult.first.size() - 1][0]];
                result->value()[1][0] = partialResult.first[indices[partialResult.first.size() - 1][1]];
                result->value()[2][0] = partialResult.first[indices[partialResult.first.size() - 1][2]];
                result->value()[3][0] = partialResult.first[indices[partialResult.first.size() - 1][3]];
            }
            if (result->has_value() && !partialResult.second.empty())
            {
                result->value()[0][1] = partialResult.second[indices[partialResult.second.size() - 1][0]];
                result->value()[1][1] = partialResult.second[indices[partialResult.second.size() - 1][1]];
                result->value()[2][1] = partialResult.second[indices[partialResult.second.size() - 1][2]];
                result->value()[3][1] = partialResult.second[indices[partialResult.second.size() - 1][3]];
            }
            else if (result->has_value())
            {
                result->value()[0][1] = result->value()[0][0];
                result->value()[1][1] = result->value()[1][0];
                result->value()[2][1] = result->value()[2][0];
                result->value()[3][1] = result->value()[3][0];
            }
        }

        return result;
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

        thread_local std::vector<std::pair<i_string_view::const_iterator, i_string_view::const_iterator>> tBits;
        tBits.clear();
        neolib::tokens(aSelector[0] == '.' ? std::next(aSelector.begin()) : aSelector.begin(), aSelector.end(), ":"s, tBits);
        for (auto const& bit : tBits)
        {
            auto selector = iSelectors.find("." + std::string{ bit.first, bit.second });
            if (selector != iSelectors.end())
            {
                auto property = selector->second.find(aProperty);
                if (property != selector->second.end())
                    return property->second;
            }
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
            PairedValues,
            PairSeparator,
            ValuePart,
            Identifier,
            Integer,
            Digit,
            Hex3,
            Hex4,
            Hex6,
            Hex8,
            HexDigit,
            Length,
            BorderStyle
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
declare_symbol(neogfx::nss::symbol, PairedValues)
declare_symbol(neogfx::nss::symbol, PairSeparator)
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
declare_symbol(neogfx::nss::symbol, BorderStyle)
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
            ( symbol::Identifier >> sequence(+repeat(range('A', 'Z') | range('a', 'z')) , repeat(range('A', 'Z') | range('a', 'z') | range('0' , '9') | '-' ))),
            ( symbol::DeclarationBlock >> sequence(symbol::BeginDeclarationBlock , symbol::Declarations, symbol::EndDeclarationBlock) ),
            ( symbol::BeginDeclarationBlock >> '{'_ ),
            ( symbol::EndDeclarationBlock >> '}'_ ),
            ( symbol::Declarations >> sequence(symbol::Declaration , repeat(sequence(symbol::EndDeclaration , symbol::Declaration)), optional(symbol::EndDeclaration)) ),
            ( symbol::Declaration >> (sequence(symbol::Property <=> "nss.property"_concept), symbol::DeclarationSeparator, (symbol::Value <=> "nss.value"_concept)) ),
            ( symbol::Declaration >> (sequence(symbol::Property <=> "nss.property"_concept), symbol::DeclarationSeparator, (symbol::PairedValues <=> "nss.paired_values"_concept)) ),
            ( symbol::DeclarationSeparator >> ':'_ ),
            ( symbol::EndDeclaration >> ';'_ ),
            ( symbol::Selector >> optional('.') , symbol::Identifier ),
            ( symbol::Property >> symbol::Identifier ),
            ( symbol::Value >> +repeat(symbol::ValuePart) ),
            ( symbol::PairedValues >> sequence( +repeat(symbol::ValuePart), symbol::PairSeparator <=> "nss.pair_separator"_concept, +repeat(symbol::ValuePart))),
            ( symbol::PairSeparator >> '/'_ ),
            ( symbol::ValuePart >> (symbol::BorderStyle <=> "nss.border_style"_concept) ),
            ( symbol::ValuePart >> (symbol::Length <=> "nss.length"_concept) ),
            ( symbol::ValuePart >> (symbol::Integer <=> "nss.integer"_concept) ),
            ( symbol::ValuePart >> (('#'_ , symbol::Hex3) <=> "nss.color"_concept) ),
            ( symbol::ValuePart >> (('#'_ , symbol::Hex4) <=> "nss.color"_concept) ),
            ( symbol::ValuePart >> (('#'_ , symbol::Hex6) <=> "nss.color"_concept) ),
            ( symbol::ValuePart >> (('#'_ , symbol::Hex8) <=> "nss.color"_concept) ),
            ( symbol::ValuePart >> symbol::Identifier ),
            ( symbol::Length >> sequence(symbol::Integer, ("cm"_ | "mm"_ | "in"_ | "px"_ | "pt"_ | "pc"_ | "em"_ | "ex"_ | "ch"_ | "rem"_ | "vw"_ | "vh"_ | "vmin"_ | "vmax"_ | "%"_ )) ),
            ( symbol::BorderStyle >> ("dotted"_ | "dashed"_ | "solid"_ | "double"_ | "groove"_ | "ridge"_ | "inset"_ | "outset"_ | "none"_ | "hidden"_ ) ),
            ( symbol::Integer >> +repeat(symbol::Digit) ),
            ( symbol::Digit >> range('0', '9') ),
            ( symbol::Hex3 >> sequence(symbol::HexDigit, symbol::HexDigit, symbol::HexDigit) ),
            ( symbol::Hex4 >> sequence(symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit) ),
            ( symbol::Hex6 >> sequence(symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit) ),
            ( symbol::Hex8 >> sequence(symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit, symbol::HexDigit) ),
            ( symbol::HexDigit >> (range('0', '9') | range('A', 'F') | range('a', 'f')) ),

            ( symbol::Eof >> discard(optional(symbol::Whitespace)), "" ),
            ( symbol::Whitespace >> +repeat(' '_ | '\r' | '\n' | '\t' | symbol::Comment) ),
            ( symbol::Comment >> sequence("/*"_ , repeat(range('\0', '\xFF')) , "*/"_) ),
            ( symbol::Sheet >> discard(optional(symbol::Whitespace)) , symbol::Sheet , discard(optional(symbol::Whitespace)) ),
            ( symbol::Rule >> discard(optional(symbol::Whitespace)) , symbol::Rule , discard(optional(symbol::Whitespace)) ),
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
            ( symbol::PairedValues >> discard(optional(symbol::Whitespace)) , symbol::PairedValues , discard(optional(symbol::Whitespace))),
            ( symbol::PairSeparator >> discard(optional(symbol::Whitespace)) , symbol::PairSeparator, discard(optional(symbol::Whitespace))),
            ( symbol::ValuePart >> discard(optional(symbol::Whitespace)) , symbol::ValuePart , discard(optional(symbol::Whitespace)) ),
            ( symbol::Identifier >> discard(optional(symbol::Whitespace)) , symbol::Identifier , discard(optional(symbol::Whitespace)) )
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
        parser.parse(nss::symbol::Sheet, sheet().to_std_string_view());
        parser.set_debug_output(std::cout);
        parser.create_ast();
        create_values(iSelectors, parser.ast());
    }
}
