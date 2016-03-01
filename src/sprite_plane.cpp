// sprite_plane.cpp
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

#include "neogfx.hpp"
#include "sprite_plane.hpp"

namespace neogfx
{
	sprite_plane::sprite_plane()
	{
	}

	sprite_plane::sprite_plane(i_widget& aParent) :
		widget(aParent)
	{
	}

	sprite_plane::sprite_plane(i_layout& aLayout) : 
		widget(aLayout)
	{
	}

	void sprite_plane::paint(graphics_context& aGraphicsContext) const
	{
		for (const auto& s : iSprites)
			s->paint(aGraphicsContext);
	}

	void sprite_plane::add_sprite(i_sprite& aSprite)
	{
		iSprites.push_back(std::shared_ptr<i_sprite>(std::shared_ptr<i_sprite>(), &aSprite));
	}

	void sprite_plane::add_sprite(std::shared_ptr<i_sprite> aSprite)
	{
		iSprites.push_back(aSprite);
	}

	i_sprite& sprite_plane::create_sprite()
	{
		iSimpleSprites.push_back(sprite());
		add_sprite(iSimpleSprites.back());
		return iSimpleSprites.back();
	}

	i_sprite& sprite_plane::create_sprite(const i_texture& aTexture, const optional_rect& aTextureRect)
	{
		iSimpleSprites.emplace_back(aTexture, aTextureRect);
		add_sprite(iSimpleSprites.back());
		return iSimpleSprites.back();
	}

	i_sprite& sprite_plane::create_sprite(const i_image& aImage, const optional_rect& aTextureRect)
	{
		iSimpleSprites.emplace_back(aImage, aTextureRect);
		add_sprite(iSimpleSprites.back());
		return iSimpleSprites.back();
	}

	const sprite_plane::sprite_list& sprite_plane::sprites() const
	{
		return iSprites;
	}

	sprite_plane::sprite_list& sprite_plane::sprites()
	{
		return iSprites;
	}
}