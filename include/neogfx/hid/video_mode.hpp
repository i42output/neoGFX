// video_mode.hpp
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

namespace neogfx
{
	class video_mode
	{
	public:
		video_mode(uint32_t aWidth, uint32_t aHeight, uint32_t aBitsPerPixel = 32) : 
			iWidth(aWidth), iHeight(aHeight), iBitsPerPixel(aBitsPerPixel)
		{
		};
	public:
		uint32_t width() const
		{
			return iWidth;
		}
		uint32_t height() const
		{
			return iHeight;
		}
		uint32_t bits_per_pixel() const
		{
			return iBitsPerPixel;
		}
	private:
		uint32_t iWidth;
		uint32_t iHeight;
		uint32_t iBitsPerPixel;
	};
}