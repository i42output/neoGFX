// i_style_sheet.hpp
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

#include <neogfx/core/primitives.hpp>

namespace neogfx
{
    using i_style_sheet_value = i_vector<i_pair<i_string, i_string>>;

    template <typename T>
    T const* evaluate_style_sheet_value(i_style_sheet_value const& aValue);

    class i_style_sheet
    {
    public:
        using abstract_type = i_style_sheet;
    public:
        struct failed_to_open_style_sheet : std::runtime_error { failed_to_open_style_sheet() : std::runtime_error("neogfx::i_style_sheet::failed_to_open_style_sheet") {} };
    public:
        virtual ~i_style_sheet() = default;
    public:
        virtual i_string_view& sheet() const = 0;
    public:
        virtual i_style_sheet_value const& value(i_string_view const& aSelector, i_string_view const& aProperty) const = 0;
    public:
        template <typename T>
        T const* evaluate(std::string_view const& aSelector, std::string_view const& aProperty) const
        {
            auto const& resultValue = value(string_view{ aSelector }, string_view{ aProperty });
            if (resultValue.empty())
                return nullptr;
            return evaluate_style_sheet_value<T>(resultValue);
        }
        template <typename T>
        T const* evaluate(std::string_view const& aSelector, std::string_view const& aProperty, i_style_sheet const& aParentStyleSheet) const
        {
            auto result = evaluate<T>(aSelector, aProperty);
            if (result)
                return result;
            if (&aParentStyleSheet != this)
                return aParentStyleSheet.evaluate<T>(aSelector, aProperty);
            return nullptr;
        }
    };

    using i_optional_style_sheet = neolib::i_optional<i_style_sheet>;
}
