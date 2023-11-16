// glyph_text.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/text/glyph.hpp>

namespace neogfx
{
    glyph::glyph(const i_sub_texture& aTexture, bool aSubpixel, const glyph_metrics& aMetrics, glyph_pixel_mode aPixelMode) :
        iTexture(aTexture), iOutlineTexture{ nullptr }, iSubpixel{ aSubpixel }, iMetrics{ aMetrics }, iPixelMode{ aPixelMode }
    {
    }

    glyph::~glyph()
    {
    }

    const i_sub_texture& glyph::texture() const
    {
        return iTexture;
    }

    bool glyph::has_outline_texture() const
    {
        return iOutlineTexture != nullptr;
    }

    const i_sub_texture& glyph::outline_texture() const
    {
        if (has_outline_texture())
            return *iOutlineTexture;
        throw no_outline_texture();
    }

    void glyph::set_outline_texture(const i_sub_texture& aOutlineTexture)
    {
        iOutlineTexture = &aOutlineTexture;
    }

    bool glyph::subpixel() const
    {
        return iSubpixel && iPixelMode == glyph_pixel_mode::LCD;
    }

    const glyph_metrics& glyph::metrics() const
    {
        return iMetrics;
    }

    glyph_pixel_mode glyph::pixel_mode() const
    {
        return iPixelMode;
    }
}