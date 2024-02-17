// video_mode.hpp
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

#include <neogfx/core/geometrical.hpp>

namespace neogfx
{
    typedef size_u32 video_mode_resolution;

    class video_mode
    {
    public:
        video_mode(uint32_t aWidth, uint32_t aHeight, uint32_t aBitsPerPixel = 32) :
            iResolution{ aWidth, aHeight }, iBitsPerPixel{ aBitsPerPixel }
        {
        };
        template <typename T>
        explicit video_mode(const basic_size<T>& aExtents, uint32_t aBitsPerPixel = 32) :
            iResolution{ aExtents }, iBitsPerPixel{ aBitsPerPixel }
        {
        };
    public:
        const video_mode_resolution& resolution() const
        {
            return iResolution;
        }
        uint32_t bits_per_pixel() const
        {
            return iBitsPerPixel;
        }
    private:
        video_mode_resolution iResolution;
        uint32_t iBitsPerPixel;
    };

    typedef std::optional<video_mode> optional_video_mode;
}