// i_object.hpp
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
#include <neolib/uuid.hpp>

namespace neogfx
{
	enum class object_category
	{
		Sprite,
		PhysicalObject,
		Custom,
		Shape // Shape must sort last
	};

	class i_object
	{
	public:
		typedef neolib::uuid uuid;
	public:
		struct not_implemented : std::logic_error { not_implemented() : std::logic_error("neogfx::i_object::not_implemented") {} };
	public:
		virtual ~i_object() {}
	public:
		virtual object_category category() const = 0;
		virtual const uuid& type() const { static uuid sNullTypeId = {}; return sNullTypeId; }
		virtual uint64_t collision_mask() const { return 0ull; }
		virtual void set_collision_mask(uint64_t) { throw not_implemented(); }
		virtual bool destroyed() const { return false; }
	};
}