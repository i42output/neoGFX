// i_emoji_atlas.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gfx/i_texture.hpp>

namespace neogfx
{
    class i_emoji_atlas
    {
    public:
        typedef std::uint32_t emoji_id;
    public:
        struct emoji_not_found : std::logic_error { emoji_not_found() : std::logic_error("neogfx::i_emoji_atlas::emoji_not_found") {} };
    public:
        virtual bool is_emoji(char32_t aCodePoint) const = 0;
        virtual bool is_emoji(const std::u32string& aCodePoints) const = 0;
        virtual bool is_emoji(const std::u32string& aCodePoints, std::u32string& aPartial) const = 0;
        virtual emoji_id emoji(char32_t aCodePoint, dimension aDesiredSize) const = 0;
        virtual emoji_id emoji(const std::u32string& aCodePoints, dimension aDesiredSize) const = 0;
        virtual const i_texture& emoji_texture(emoji_id aId) const = 0;
    };
}