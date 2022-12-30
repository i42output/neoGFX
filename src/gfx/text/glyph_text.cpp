// glyph_char.cpp
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
#include <neogfx/gfx/text/i_font_manager.hpp>
#include <neogfx/gfx/text/glyph_text.ipp>

namespace neogfx
{
    template class basic_glyph_text_content<neolib::vecarray<glyph_char, SMALL_OPTIMIZATION_GLYPH_TEXT_GLYPH_COUNT, -1>, glyph_char const*, glyph_char*>;

    glyph_text::glyph_text(font const& aFont) :
        iContent{ service<i_font_manager>().glyph_text_factory().create_glyph_text(aFont).content() }
    {
    }

    glyph_text::glyph_text(i_glyph_text& aContents) :
        iContent{ aContents }
    {
    }

    glyph_text::glyph_text(glyph_text const& aOther) :
        iContent{ aOther.iContent }
    {
    }

    glyph_text glyph_text::operator=(const glyph_text& aOther)
    {
        iContent = aOther.iContent;
        return *this;
    }

    i_glyph_text& glyph_text::content() const
    {
        return *iContent;
    }

    const font& glyph_text::glyph_font() const
    {
        return content().glyph_font();
    }

    const font& glyph_text::glyph_font(const_reference aGlyphChar) const
    {
        return content().glyph_font(aGlyphChar);
    }

    const i_glyph& glyph_text::glyph(const_reference aGlyphChar) const
    {
        return content().glyph(aGlyphChar);
    }

    bool glyph_text::empty() const
    {
        return content().empty();
    }

    glyph_text::size_type glyph_text::size() const
    {
        return content().size();
    }

    void glyph_text::clear()
    {
        content().clear();
    }

    size glyph_text::extents() const
    {
        return content().extents();
    }

    size glyph_text::extents(const_reference aGlyphChar) const
    {
        return content().extents(aGlyphChar);
    }

    size glyph_text::extents(const_iterator aBegin, const_iterator aEnd) const
    {
        return content().extents(aBegin, aEnd);
    }

    void glyph_text::set_extents(const neogfx::size& aExtents)
    {
        content().set_extents(aExtents);
    }

    std::pair<glyph_text::const_iterator, glyph_text::const_iterator> glyph_text::word_break(const_iterator aBegin, const_iterator aFrom) const
    {
        return content().word_break(aBegin, aFrom);
    }

    std::pair<glyph_text::iterator, glyph_text::iterator> glyph_text::word_break(const_iterator aBegin, const_iterator aFrom)
    {
        return content().word_break(aBegin, aFrom);
    }

    glyph_text::const_reference glyph_text::back() const
    {
        return *std::prev(content().cend());
    }

    glyph_text::reference glyph_text::back()
    {
        return *std::prev(content().end());
    }

    glyph_text::const_iterator glyph_text::cbegin() const
    {
        return content().cbegin();
    }

    glyph_text::const_iterator glyph_text::cend() const
    {
        return content().cend();
    }

    glyph_text::const_iterator glyph_text::begin() const
    {
        return cbegin();
    }

    glyph_text::const_iterator glyph_text::end() const
    {
        return cend();
    }

    glyph_text::iterator glyph_text::begin()
    {
        return content().begin();
    }

    glyph_text::iterator glyph_text::end()
    {
        return content().end();
    }
}