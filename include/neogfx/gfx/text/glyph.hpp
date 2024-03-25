// glyph_text.hpp
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

#include <neogfx/gfx/text/i_glyph.hpp>

namespace neogfx
{
    class glyph : public i_glyph
    {
    public:
        glyph(const i_sub_texture& aTexture, bool aSubpixel, const glyph_metrics& aMetrics, glyph_pixel_mode aPixelMode);
        ~glyph();
    public:
        const i_sub_texture& texture() const final;
        bool has_outline_texture() const final;
        const i_sub_texture& outline_texture() const final;
        void set_outline_texture(const i_sub_texture& aOutlineTexture) final;
        bool subpixel() const final;
        const glyph_metrics& metrics() const final;
        glyph_pixel_mode pixel_mode() const final;
    private:
        const i_sub_texture& iTexture;
        const i_sub_texture* iOutlineTexture;
        bool iSubpixel;
        glyph_metrics iMetrics;
        glyph_pixel_mode iPixelMode;
    };
}