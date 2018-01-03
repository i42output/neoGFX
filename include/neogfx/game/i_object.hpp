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

	typedef neolib::uuid object_type;

	class i_shape;
	class i_collidable;
	class i_physical_object;

	class i_object
	{
	public:
		struct not_a_shape : std::logic_error { not_a_shape() : std::logic_error("neogfx::i_object::not_a_shape") {} };
		struct not_a_collidable : std::logic_error { not_a_collidable() : std::logic_error("neogfx::i_object::not_a_collidable") {} };
		struct not_a_physical_object : std::logic_error { not_a_physical_object() : std::logic_error("neogfx::i_object::not_a_physical_object") {} };
		struct not_implemented : std::logic_error { not_implemented() : std::logic_error("neogfx::i_object::not_implemented") {} };
	public:
		virtual ~i_object() {}
	public:
		virtual object_category category() const = 0;
		virtual const i_shape& as_shape() const = 0;
		virtual i_shape& as_shape() = 0;
		virtual const i_collidable& as_collidable() const = 0;
		virtual i_collidable& as_collidable() = 0;
		virtual const i_physical_object& as_physical_object() const = 0;
		virtual i_physical_object& as_physical_object() = 0;
		virtual const object_type& type() const { static object_type sNullTypeId = {}; return sNullTypeId; }
		virtual bool killed() const = 0;
		virtual void kill() = 0;
	};
}