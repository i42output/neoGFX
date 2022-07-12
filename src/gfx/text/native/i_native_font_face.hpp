// i_native_font_face.hpp
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
#include <neolib/core/i_reference_counted.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/gfx/text/font.hpp>

namespace neogfx
{
    class i_native_font;

    struct glyph;
    class i_glyph_texture;

    enum class kerning_method
    {
        Freetype,
        Harfbuzz,
        Disabled
    };

    class i_native_font_face : public neolib::i_reference_counted
    {
    public:
        struct no_fallback_font : std::logic_error { no_fallback_font() : std::logic_error("neogfx::i_native_font_face::no_fallback_font") {} };
        struct bad_fixed_size_index : std::logic_error { bad_fixed_size_index() : std::logic_error("neogfx::i_native_font_face::bad_fixed_size_index") {} };
    public:
        typedef i_native_font_face abstract_type;
        typedef uint32_t glyph_index_t;
    public:
        virtual ~i_native_font_face() = default;
    public:
        virtual font_id id() const = 0;
        virtual i_native_font& native_font() = 0;
        virtual i_string const& family_name() const = 0;
        virtual font_style style() const = 0;
        virtual i_string const& style_name() const = 0;
        virtual font::point_size size() const = 0;
        virtual dimension horizontal_dpi() const = 0;
        virtual dimension vertical_dpi() const = 0;
        virtual neogfx::size em_size() const = 0;
        virtual dimension height() const = 0;
        virtual dimension max_advance() const = 0;
        virtual dimension ascender() const = 0;
        virtual dimension descender() const = 0;
        virtual dimension underline_position() const = 0;
        virtual dimension underline_thickness() const = 0;
        virtual dimension line_spacing() const = 0;
        virtual neogfx::kerning_method kerning_method() const = 0;
        virtual void set_kerning_method(neogfx::kerning_method aKerningMethod) = 0;
        virtual dimension kerning(glyph_index_t aLeftGlyphIndex, glyph_index_t aRightGlyphIndex) const = 0;
        virtual bool is_bitmap_font() const = 0;
        virtual uint32_t num_fixed_sizes() const = 0;
        virtual font::point_size fixed_size(uint32_t aFixedSizeIndex) const = 0;
        virtual bool has_fallback() const = 0;
        virtual bool fallback_cached() const = 0;
        virtual i_native_font_face& fallback() const = 0;
        virtual void* handle() const = 0;
        virtual glyph_index_t glyph_index(char32_t aCodePoint) const = 0;
        virtual i_glyph_texture& glyph_texture(const glyph& aGlyph) const = 0;
    };
}