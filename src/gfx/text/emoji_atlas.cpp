// emoji_atlas.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/gfx/text/emoji_atlas.hpp>

namespace neogfx
{
	emoji_atlas::emoji_atlas(i_texture_manager& aTextureManager) : iTextureAtlas(aTextureManager.create_texture_atlas())
	{
		// todo
	}

	bool emoji_atlas::is_emoji(const std::u32string& aCodePoints) const
	{
		return m_EmojiMap.find(aCodePoints) != m_EmojiMap.end();
	}

	emoji_atlas::emoji_id emoji_atlas::emoji(const std::u32string& aCodePoints) const
	{
		auto iterEmoji = m_EmojiMap.find(aCodePoints);
		if (iterEmoji == m_EmojiMap.end())
			throw emoji_not_found();
		if (!iterEmoji->second.is_initialized())
			iterEmoji->second = iTextureAtlas->create_sub_texture(neogfx::image("todo")).atlas_id();
		return *iterEmoji->second;
	}

	const i_texture& emoji_atlas::emoji_texture(emoji_id aId) const
	{
		return iTextureAtlas->sub_texture(aId);
	}
}