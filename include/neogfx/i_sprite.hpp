// i_sprite.hpp
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

#include "neogfx.hpp"
#include <chrono>
#include <boost/optional.hpp>
#include "geometry.hpp"
#include "graphics_context.hpp"
#include "i_texture.hpp"
#include "i_physical_object.hpp"

namespace neogfx
{
	class i_sprite
	{
	public:
		typedef std::size_t frame_index;
		typedef scalar time_interval;
		typedef std::vector<std::pair<frame_index, time_interval>> frame_list;
		typedef i_physical_object::time_point time_point;
		typedef i_physical_object::optional_time_point optional_time_point;
	public:
		struct bad_frame_index : std::logic_error { bad_frame_index() : std::logic_error("neogfx::i_sprite::bad_frame_index") {} };
	public:
		virtual ~i_sprite() {}
	public:
		virtual void add_frame(const i_texture& aTexture, const optional_rect& aTextureRect) = 0;
		virtual void replace_frame(frame_index aFrameIndex, const i_texture& aTexture, const optional_rect& aTextureRect) = 0;
		virtual void remove_frame(frame_index aFrameIndex) = 0;
		virtual void set_texture_rect(frame_index aFrameIndex, const optional_rect& aTextureRect) = 0;
		virtual void set_texture_rect_for_all_frames(const optional_rect& aTextureRect) = 0;
	public:
		virtual const frame_list& animation() const = 0;
		virtual frame_index current_frame() const = 0;
		virtual point position() const = 0;
		virtual neogfx::size size() const = 0;
		virtual const vector2& scale() const = 0;
		virtual const optional_path& path() const = 0;
		virtual matrix33 transformation() const = 0;
		virtual void set_animation(const frame_list& aAnimation) = 0;
		virtual void set_current_frame(frame_index aFrameIndex) = 0;
		virtual void set_position(const point& aPosition) = 0;
		virtual void set_size(const optional_size& aSize, bool aCentreOrigin = true) = 0;
		virtual void set_scale(const vector2& aScale) = 0;
		virtual void set_path(const optional_path& aPath) = 0;
		virtual void set_transformation(const optional_matrix33& aTransformation) = 0;
	public:
		virtual const i_physical_object& physics() const = 0;
		virtual i_physical_object& physics() = 0;
	public:
		virtual bool update(const optional_time_point& aNow = optional_time_point()) = 0;
	public:
		virtual void paint(graphics_context& aGraphicsContext) const = 0;
	};
}