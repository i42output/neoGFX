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

#include <neolib/core/vecarray.hpp>

#include <neogfx/core/i_style_sheet.hpp>

namespace neogfx
{
    using style_sheet_value = vector<pair<string, string>>;

    class style_sheet : public i_style_sheet
    {
    public:
        using property_map = std::unordered_map<string, style_sheet_value>;
        using selector_map = std::unordered_map<string, property_map>;
    public:
        style_sheet();
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
        style_sheet_value const& value(i_string_view const& aSelector, i_string_view const& aProperty) const final;
    public:
        std::string to_string() const;
    private:
        void parse();
    private:
        std::shared_ptr<std::string> iSheet;
        mutable std::optional<string_view> iSheetView;
        selector_map iSelectors;
    };

    using optional_style_sheet = neolib::optional<style_sheet>;
}
