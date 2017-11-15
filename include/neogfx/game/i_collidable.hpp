// i_collidable.hpp
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
#include <neogfx/core/numerical.hpp>

namespace neogfx
{
	class i_collidable
	{
	public:
		struct not_implemented : std::logic_error { not_implemented() : std::logic_error("neogfx::i_collidable::not_implemented") {} };
	public:
		virtual neogfx::aabb aabb() const = 0;
		virtual void* collision_tree_link() const = 0;
		virtual void set_collision_tree_link(void* aLink) = 0;
		virtual uint64_t collision_mask() const { return 0ull; }
		virtual void set_collision_mask(uint64_t) { throw not_implemented(); }
		virtual bool has_collided(const i_collidable& aOther) const = 0;
		virtual void collided(i_collidable& aOther) = 0;
	public:
		template <typename T> const T& as() const { return static_cast<const T&>(*this); }
		template <typename T> T& as() { return static_cast<T&>(*this); }
	};
}