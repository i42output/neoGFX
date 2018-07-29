// i_component_data.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/numerical.hpp>

namespace neogfx
{
	enum class component_data_field_type : uint32_t
	{
		Bool		= 0x00000000,
		Int8		= 0x00000001,
		Uint8		= 0x00000002,
		Int16		= 0x00000003,
		Uint16		= 0x00000004,
		Int32		= 0x00000005,
		Uint32		= 0x00000006,
		Int64		= 0x00000007,
		Uint64		= 0x00000008,
		Float32		= 0x00000009,
		Float64		= 0x0000000A,
		Vec2		= 0x00100000,
		Vec3		= 0x00200000,
		Vec4		= 0x00300000,
		Mat22		= 0x00400000,
		Mat33		= 0x00500000,
		Mat44		= 0x00600000,
		Array		= 0x08000000,
		Internal	= 0x80000000
	};

	inline constexpr component_data_field_type operator|(component_data_field_type aLhs, component_data_field_type aRhs)
	{
		return static_cast<component_data_field_type>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline constexpr component_data_field_type operator&(component_data_field_type aLhs, component_data_field_type aRhs)
	{
		return static_cast<component_data_field_type>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
	}

	inline constexpr component_data_field_type operator|=(component_data_field_type& aLhs, component_data_field_type aRhs)
	{
		return aLhs = static_cast<component_data_field_type>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline constexpr component_data_field_type operator&=(component_data_field_type& aLhs, component_data_field_type aRhs)
	{
		return aLhs = static_cast<component_data_field_type>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
	}

	struct i_component_data
	{
		struct meta
		{
			static bool shared_data()
			{
				return false;
			}
		};
	};
}