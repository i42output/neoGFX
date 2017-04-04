// rectangle.hpp
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
#include "shape.hpp"

namespace neogfx
{
	class rectangle : public shape
	{
	public:
		rectangle(i_shape_container& aContainer, const vec3& aPosition, const size& aSize);
		rectangle(i_shape_container& aContainer, const vec3& aPosition, const size& aSize, const colour& aColour);
		rectangle(i_shape_container& aContainer, const vec3& aPosition, const size& aSize, const i_texture& aTexture, const optional_rect& aTextureRect = optional_rect());
		rectangle(i_shape_container& aContainer, const vec3& aPosition, const size& aSize, const i_image& aImage, const optional_rect& aTextureRect = optional_rect());
		rectangle(const rectangle& aOther);
	public:
		virtual std::size_t vertex_count(bool aIncludeCentre = false) const;
		virtual vec3_list vertices(bool aIncludeCentre = false) const;
	};
}