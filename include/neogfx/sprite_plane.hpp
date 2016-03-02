// sprite_plane.hpp
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
#include <boost/pool/pool_alloc.hpp>
#include <neolib/timer.hpp>
#include "widget.hpp"
#include "sprite.hpp"

namespace neogfx
{
	class sprite_plane : public widget
	{
	public:
		event<> sprites_updating;
		event<> sprites_updated;
	public:
		typedef std::vector<std::shared_ptr<i_sprite>> sprite_list;
	private:
		typedef std::list<sprite, boost::fast_pool_allocator<sprite>> simple_sprite_list;
	public:
		sprite_plane();
		sprite_plane(i_widget& aParent);
		sprite_plane(i_layout& aLayout);
		~sprite_plane();
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
	public:
		void add_sprite(i_sprite& aSprite);
		void add_sprite(std::shared_ptr<i_sprite> aSprite);
		i_sprite& create_sprite();
		i_sprite& create_sprite(const i_texture& aTexture, const optional_rect& aTextureRect = optional_rect());
		i_sprite& create_sprite(const i_image& aImage, const optional_rect& aTextureRect = optional_rect());
	public:
		const sprite_list& sprites() const;
		sprite_list& sprites();
	private:
		bool update_sprites();
	private:
		sprite_list iSprites;
		simple_sprite_list iSimpleSprites; ///< Simple sprites created by this widget (pointers to which will be available in the main sprite list)
	};
}