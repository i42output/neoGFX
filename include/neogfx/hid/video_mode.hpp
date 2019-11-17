// video_mode.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/geometrical.hpp>

namespace neogfx
{
    class video_mode
    {
    public:
        typedef basic_size<uint32_t> resolution_t;
    public:
        video_mode(const resolution_t& aResolution, uint32_t aBitsPerPixel = 32) :
            iResolution{ aResolution }, iBitsPerPixel{ aBitsPerPixel }
        {
        };
        video_mode(uint32_t aWidth, uint32_t aHeight, uint32_t aBitsPerPixel = 32) :
            iResolution{ aWidth, aHeight }, iBitsPerPixel{ aBitsPerPixel }
        {
        };
    public:
        const resolution_t& resolution() const
        {
            return iResolution;
        }
        uint32_t bits_per_pixel() const
        {
            return iBitsPerPixel;
        }
    private:
        resolution_t iResolution;
        uint32_t iBitsPerPixel;
    };

    typedef std::optional<video_mode> optional_video_mode;
}