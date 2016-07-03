// i_item_model.hpp
/*
  neogfx C++ GUI Library
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
		virtual iterator insert_item(const_iterator aPosition, const value_type& aValue) = 0;
		virtual iterator insert_item(const_iterator aPosition, const value_type& aValue, const cell_data_type& aCellData) = 0;
		virtual iterator append_item(const_iterator aParent, const value_type& aValue) = 0;
		virtual iterator append_item(const_iterator aParent, const value_type& aValue, const cell_data_type& aCellData) = 0;
	public:
		virtual value_type& item(const item_model_index& aIndex) = 0;
		virtual const value_type& item(const item_model_index& aIndex) const = 0;
	};

	typedef i_basic_item_model<void*> i_default_item_model;
}