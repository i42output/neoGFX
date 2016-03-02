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

namespace neogfx
{
	class i_sprite
	{
	public:
		typedef std::size_t frame_index;
		typedef scalar time_interval;
		typedef std::vector<std::pair<frame_index, time_interval>> frame_list;
		typedef std::chrono::time_point<std::chrono::steady_clock> time_point;
		typedef boost::optional<time_point> optional_time_point;
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
		virtual const point& origin() const = 0;
		virtual const point& position() const = 0;
		virtual const optional_size& size() const = 0;
		virtual const vector2& scale() const = 0;
		virtual scalar angle_radians() const = 0;
		virtual scalar angle_degrees() const = 0;
		virtual const vector2& velocity() const = 0;
		virtual const vector2& acceleration() const = 0;
		virtual scalar spin_radians() const = 0;
		virtual scalar spin_degrees() const = 0;
		virtual const optional_path& path() const = 0;
		virtual const matrix33& transformation() const = 0;
		virtual void set_animation(const frame_list& aAnimation) = 0;
		virtual void set_current_frame(frame_index aFrameIndex) = 0;
		virtual void set_origin(const point& aOrigin) = 0;
		virtual void set_position(const point& aPosition) = 0;
		virtual void set_size(const optional_size& aSize) = 0;
		virtual void set_scale(const vector2& aScale) = 0;
		virtual void set_angle_radians(scalar aAngle) = 0;
		virtual void set_angle_degrees(scalar aAngle) = 0;
		virtual void set_velocity(const vector2& aVelocity) = 0;
		virtual void set_acceleration(const vector2& aAcceleration) = 0;
		virtual void set_spin_radians(scalar aSpin) = 0;
		virtual void set_spin_degrees(scalar aSpin) = 0;
		virtual void set_path(const optional_path& aPath) = 0;
	public:
		virtual bool update(const optional_time_point& aNow = optional_time_point()) = 0;
	public:
		virtual void paint(graphics_context& aGraphicsContext) const = 0;
	};
}