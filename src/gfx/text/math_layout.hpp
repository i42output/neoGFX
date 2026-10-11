// math_layout.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

// Private to the font manager (see i_font_manager::typeset_math): typesets a LaTeX math expression using an
// OpenType MATH font (via HarfBuzz), following the TeX layout rules (The TeXbook, Appendix G) with the font's
// MATH table constants. Uses only std types and HarfBuzz.

#include <cstdint>
#include <string_view>
#include <vector>

struct hb_font_t;

namespace neogfx::math_layout
{
    struct glyph_item
    {
        std::uint32_t glyph;    // glyph index in the math font
        float scale;            // font size relative to the base size (script levels)
        float x;                // pen position of the glyph origin
        float y;                // baseline position; positive is below the formula's baseline
    };

    struct rule_item
    {
        float x;
        float y;                // top edge; positive is below the formula's baseline
        float width;
        float height;
    };

    struct layout
    {
        float width = 0.0f;
        float ascent = 0.0f;    // extent above the baseline
        float descent = 0.0f;   // extent below the baseline
        std::vector<glyph_item> glyphs;
        std::vector<rule_item> rules;
        bool error = false;     // the source could not be fully parsed; the layout shows what could be
    };

    // aFont: the math font at the base size (HarfBuzz scale in 26.6 pixels, as neoGFX sets it).
    // Fonts without a MATH table still work using approximate TeX default constants.
    layout typeset(hb_font_t* aFont, std::string_view aLatex, bool aDisplay);
}
