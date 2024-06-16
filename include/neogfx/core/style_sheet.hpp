// style_sheet.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

#include <neolib/core/variant.hpp>
#include <neolib/core/vecarray.hpp>

#include <neogfx/core/i_style_sheet.hpp>

namespace neogfx
{
    class style_sheet : public i_style_sheet
    {
    public:
        class selector
        {
        public:
            typedef std::string element;
            typedef std::optional<element> optional_element;
            enum type_e
            {
                Universal,
                Type,
                Descendant,
                Child,
                AdjacentSibling,
                GeneralSibling,
                Class,
                ID,
                SimpleAttribute,
                ExactAttributeValue,
                PartialAttributeValue,
                BeginniingSubstringAttributeValue,
                EndingSubstringAttributeValue,
                ArbitrarySubstringAttributeValue,
                LanguageAttribute
            };
            enum pseudo_class_e
            {
                // Structural
                Empty,
                FirstChild,
                FirstOfType,
                Lang,
                LastChild,
                LastOfType,
                NthChild,
                NthLastChild,
                NthLastOfType,
                NthOfType,
                OnlyChild,
                OnlyOfType,
                Root,
                // Negation
                Not,
                // Interaction
                Active,
                Checked,
                Disabled,
                Enabled,
                Focus,
                Hover,
                Link,
                Target,
                Visited
            };
            typedef std::variant<std::monostate, std::string> pseudo_class_argument;
            typedef std::variant<std::monostate, std::string, std::pair<pseudo_class_e, pseudo_class_argument>> argument;
            typedef neolib::vecarray<argument, 4, -1> arguments_type;
            typedef std::pair<type_e, arguments_type> value_type;
        public:
            selector(type_e aType, const arguments_type& aArguments = arguments_type());
        public:
            type_e type() const;
            const arguments_type& arguments() const;
        private:
            type_e iType;
            arguments_type iArguments;
        };
        class declaration
        {
        public:
            declaration();
        };
        typedef std::vector<declaration> declaration_block;
        typedef std::list<selector> selector_list;
        typedef std::list<declaration_block> declaration_block_list;
        typedef std::vector<std::pair<selector_list::const_iterator, declaration_block_list::const_iterator>> rule_list;
    public:
        class i_visitor
        {
        public:
            virtual std::string const& element_name() = 0;
            virtual std::string const& id() = 0;
            virtual bool has_parent() = 0;
            virtual i_visitor& parent() = 0;
        public:
            virtual void apply(const declaration_block& aDeclarations) = 0;
        };
    public:
        style_sheet(i_style_sheet const& aSheet);
        style_sheet(std::string const& aSheet);
        style_sheet(std::string_view const& aSheet);
        style_sheet(std::istream& aSheet);
    public:
        style_sheet& operator=(i_style_sheet const& aSheet);
        style_sheet& operator=(std::string const& aSheet);
        style_sheet& operator=(std::string_view const& aSheet);
        style_sheet& operator=(std::istream& aSheet);
    public:
        i_string_view& sheet() const final;
    public:
        void accept(i_visitor& aVisitor) const;
        std::string to_string() const;
    private:
        void parse();
    private:
        std::shared_ptr<std::string> iSheet;
        mutable std::optional<string_view> iSheetView;
    };

    using optional_style_sheet = neolib::optional<style_sheet>;
}
