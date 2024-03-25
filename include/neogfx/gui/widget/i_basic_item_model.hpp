// i_item_model.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

#include "i_item_model.hpp"

namespace neogfx
{
    template <typename T>
    class i_basic_item_model : public i_item_model
    {
    public:
        typedef T value_type;
    public:
        virtual iterator insert_item(const_iterator aPosition, value_type const& aValue) = 0;
        virtual iterator insert_item(const_iterator aPosition, value_type const& aValue, item_cell_data const& aCellData) = 0;
        virtual iterator insert_item(item_model_index const& aIndex, value_type const& aValue) = 0;
        virtual iterator insert_item(item_model_index const& aIndex, value_type const& aValue, item_cell_data const& aCellData) = 0;
        virtual iterator append_item(value_type const& aValue) = 0;
        virtual iterator append_item(value_type const& aValue, item_cell_data const& aCellData) = 0;
        virtual iterator append_item(const_iterator aParent, value_type const& aValue) = 0;
        virtual iterator append_item(const_iterator aParent, value_type const& aValue, item_cell_data const& aCellData) = 0;
        virtual iterator append_item(item_model_index const& aIndex, value_type const& aValue) = 0;
        virtual iterator append_item(item_model_index const& aIndex, value_type const& aValue, item_cell_data const& aCellData) = 0;
    public:
        virtual value_type& item(item_model_index const& aIndex) = 0;
        virtual value_type const& item(item_model_index const& aIndex) const = 0;
    public:
        value_type& item(const_iterator aItem)
        {
            return item(iterator_to_index(aItem));
        }
        value_type const& item(const_iterator aItem) const
        {
            return item(iterator_to_index(aItem));
        }
        std::optional<item_model_index> find_item_maybe(value_type const& aItem) const
        {
            auto const end = rows();
            for (item_model_index::row_type row = 0; row < end; ++row)
            {
                auto const& existing = item(item_model_index{ row, 0 });
                if (existing == aItem)
                    return item_model_index{ row, 0 };
            }
            return std::nullopt;
        }
        item_model_index find_item(value_type const& aItem) const
        {
            auto result = find_item_maybe(aItem);
            if (result != std::nullopt)
                return *result;
            throw item_not_found();
        }
    };
}