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

#include <neogfx/neogfx.hpp>

#include <neogfx/gfx/text/i_font_manager.hpp>
#include <neogfx/gfx/text/glyph_text.hpp>
#include <neogfx/gfx/text/glyph_text.ipp>

namespace neogfx
{
    template class basic_glyph_text_content<glyph_text_container, const glyph_char*, glyph_char*>;

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

    glyph_text glyph_text::clone() const
    {
        return glyph_text{ *content().clone() };
    }

    i_glyph_text& glyph_text::content() const
    {
        return *iContent;
    }

    const font& glyph_text::cached_font(font_id aFontId) const
    {
        return content().cached_font(aFontId);
    }

    const font& glyph_text::glyph_font(const_reference aGlyphChar) const
    {
        return content().glyph_font(aGlyphChar);
    }

    const i_glyph& glyph_text::glyph(const_reference aGlyphChar) const
    {
        return content().glyph(aGlyphChar);
    }

    const font& glyph_text::major_font() const
    {
        return content().major_font();
    }

    void glyph_text::set_major_font(const font& aFont)
    {
        content().set_major_font(aFont);
    }

    scalar glyph_text::baseline() const
    {
        return content().baseline();
    }

    void glyph_text::set_baseline(scalar aBaseline)
    {
        content().set_baseline(aBaseline);
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
        
    glyph_text::const_reference glyph_text::operator[](size_type aIndex) const
    {
        return content()[aIndex];
    }

    void glyph_text::push_back(const_reference aGlyphChar)
    {
        content().push_back(aGlyphChar);
    }

    glyph_text::iterator glyph_text::insert(const_iterator aPos, const_iterator aFirst, const_iterator aLast)
    {
        return content().insert(aPos, aFirst, aLast);
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

    glyph_text& glyph_text::align_baselines()
    {
        content().align_baselines();
        return *this;
    }

    glyph_text::align_baselines_result glyph_text::align_baselines(iterator aBegin, iterator aEnd, bool aJustCalculate)
    {
        return content().align_baselines(aBegin, aEnd, aJustCalculate);
    }

    i_vector<glyph_text::size_type> const& glyph_text::line_breaks() const
    {
        return content().line_breaks();
    }

    i_vector<glyph_text::size_type>& glyph_text::line_breaks()
    {
        return content().line_breaks();
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
        return content().begin();
    }

    glyph_text::const_iterator glyph_text::end() const
    {
        return content().end();
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