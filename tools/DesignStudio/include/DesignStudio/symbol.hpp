// symbol.hpp
/*
  neogfx C++ App/Game Engine
  Copyright(C) 2016 Leigh Johnston
  
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

#include <DesignStudio/DesignStudio.hpp>
#include <neolib/string_utils.hpp>
#include <DesignStudio/symbol.hpp>

namespace design_studio
{
    enum class naming_convention
    {
        LowerCaseWithUnderscores,
        MixedCaseWithUnderscores,
        UpperCamelCase,
        LowerCamelCase,
        NeoGfx
    };

    enum class named_entity
    {
        LocalVariable,
        ParameterVariable,
        MemberVariable,
        StaticVariable,
        Namespace,
        Function,
        Class
    };

    inline bool named_entity_is_variable(named_entity aNamedEntity)
    {
        switch (aNamedEntity)
        {
        case named_entity::LocalVariable:
        case named_entity::ParameterVariable:
        case named_entity::MemberVariable:
        case named_entity::StaticVariable:
            return true;
        default:
            return false;
        }
    }

    inline std::string to_symbol_name(const std::string& aString, naming_convention aNamingConvention, named_entity aNamedEntity)
    {
        std::vector<std::string> tokens;
        neolib::tokens(aString, std::string{" "}, tokens);
        std::string symbolName;
        switch (aNamingConvention)
        {
        case naming_convention::LowerCaseWithUnderscores:
            for (auto& word : tokens)
                word = neolib::to_lower(word);
            break;
        case naming_convention::MixedCaseWithUnderscores:
        case naming_convention::UpperCamelCase:
            for (auto& word : tokens)
                word = neolib::to_upper(word.substr(0, 1)) + neolib::to_lower(word.substr(1));
            break;
        case naming_convention::LowerCamelCase:
            for (auto& word : tokens)
                if (&word == &tokens[0])
                    word = neolib::to_lower(word);
                else
                    word = neolib::to_upper(word.substr(0, 1)) + neolib::to_lower(word.substr(1));
            break;
        case naming_convention::NeoGfx:
            if (!named_entity_is_variable(aNamedEntity))
            {
                for (auto& word : tokens)
                    word = neolib::to_lower(word);
            }
            else
            {
                for (auto& word : tokens)
                    word = neolib::to_upper(word.substr(0, 1)) + neolib::to_lower(word.substr(1));
                switch (aNamedEntity)
                {
                case named_entity::LocalVariable:
                    tokens[0] = neolib::to_lower(tokens[0]);
                    break;
                case named_entity::ParameterVariable:
                    tokens[0] = "a" + tokens[0];
                    break;
                case named_entity::MemberVariable:
                    tokens[0] = "i" + tokens[0];
                    break;
                case named_entity::StaticVariable:
                    tokens[0] = "s" + tokens[0];
                    break;
                }
            }
            break;
        }
        for (auto& word : tokens)
        {
            if (!symbolName.empty() &&
                (aNamingConvention == naming_convention::LowerCaseWithUnderscores ||
                aNamingConvention == naming_convention::MixedCaseWithUnderscores ||
                (aNamingConvention == naming_convention::NeoGfx && !named_entity_is_variable(aNamedEntity))))
                symbolName += '_';
            symbolName += word;
        }
        return symbolName;
    }
}