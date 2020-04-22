// glyph.cpp
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
#include <neogfx/gfx/text/glyph.hpp>

namespace neogfx
{
    const font& glyph_font_cache::glyph_font(const glyph& aGlyph) const
    {
        cache_glyph_font(aGlyph.font_id());
        auto existing = cache().find(aGlyph.font_id());
        if (existing != cache().end())
            return existing->second.second;
        throw cached_font_not_found();
    }

    void glyph_font_cache::cache_glyph_font(font_id aFontId) const
    {
        if (cache().find(aFontId) == cache().end())
        {
            auto& fontService = service<i_font_manager>();
            cache().emplace(aFontId, cache_entry{ neolib::small_cookie_ref_ptr{ fontService, aFontId }, fontService.font_from_id(aFontId) });
        }
    }

    void glyph_font_cache::cache_glyph_font(const font& aFont) const
    {
        if (cache().find(aFont.id()) == cache().end())
        {
            auto& fontService = service<i_font_manager>();
            cache().emplace(aFont.id(), cache_entry{ neolib::small_cookie_ref_ptr{ fontService, aFont.id() }, aFont });
        }
    }

    void glyph_font_cache::clear()
    {
        iCache.clear();
    }

    glyph_font_cache::font_cache& glyph_font_cache::cache() const
    {
        return iCache;
    }

    glyph_text::glyph_text() :
        container{}
    {
    }

    glyph_text::glyph_text(const glyph_text& aOther) :
        container{ aOther },
        glyph_font_cache{ aOther },
        iExtents{ aOther.iExtents }
    {
    }

    glyph_text::glyph_text(glyph_text&& aOther) :
        container{ std::move(aOther) },
        glyph_font_cache{ std::move(aOther) },
        iExtents{ aOther.iExtents }
    {
    }

    glyph_text& glyph_text::operator=(const glyph_text& aOther)
    {
        if (&aOther == this)
            return *this;
        container::operator=(aOther);
        glyph_font_cache::operator=(aOther);
        iExtents = aOther.iExtents;
        return *this;
    }

    glyph_text& glyph_text::operator=(glyph_text&& aOther)
    {
        if (&aOther == this)
            return *this;
        container::operator=(std::move(aOther));
        glyph_font_cache::operator=(std::move(aOther));
        iExtents = aOther.iExtents;
        return *this;
    }

    glyph_text::iterator glyph_text::begin()
    {
        iExtents = std::nullopt;
        return container::begin();
    }

    glyph_text::iterator glyph_text::end()
    {
        iExtents = std::nullopt;
        return container::end();
    }

    glyph_text::reference glyph_text::back()
    {
        iExtents = std::nullopt;
        return container::back();
    }

    void glyph_text::push_back(const glyph& aGlyph)
    {
        container::push_back(aGlyph);
        iExtents = std::nullopt;
    }

    void glyph_text::clear()
    {
        container::clear();
        glyph_font_cache::clear();
        iExtents = std::nullopt;
    }

    bool glyph_text::operator==(const glyph_text& aOther) const
    {
        return static_cast<const container&>(*this) == static_cast<const container&>(aOther);
    }

    neogfx::size glyph_text::extents(const_iterator aBegin, const_iterator aEnd, bool aEndIsLineEnd) const
    {
        if (aBegin == aEnd)
            return neogfx::size{ 0.0, 0.0 };
        neogfx::size result;
        for (glyph_text::const_iterator i = aBegin; i != aEnd; ++i)
        {
            auto const& g = *i;
            result.cx += g.advance().cx;
            result.cy = std::max(result.cy, g.extents(*this).cy);
        }
        if (aEndIsLineEnd)
        {
            auto const& lastGlyph = *std::prev(aEnd);
            result.cx += (lastGlyph.extents(*this).cx - lastGlyph.advance().cx);
        }
        return result.ceil();
    }

    const neogfx::size& glyph_text::extents() const
    {
        if (iExtents == std::nullopt)
            iExtents = extents(begin(), end());
        return *iExtents;
    }

    void glyph_text::set_extents(const neogfx::size& aExtents)
    {
        iExtents = aExtents;
    }

    std::pair<glyph_text::const_iterator, glyph_text::const_iterator> glyph_text::word_break(const_iterator aBegin, const_iterator aFrom) const
    {
        std::pair<const_iterator, const_iterator> result(aFrom, aFrom);
        if (!aFrom->is_whitespace())
        {
            while (result.first != aBegin && !result.first->is_whitespace())
                --result.first;
            if (!result.first->is_whitespace())
            {
                result.first = aFrom;
                while (result.first != aBegin && (result.first - 1)->source() == aFrom->source())
                    --result.first;
                result.second = result.first;
                return result;
            }
            result.second = result.first;
        }
        while (result.first != aBegin && (result.first - 1)->is_whitespace())
            --result.first;
        while (result.second->is_whitespace() && result.second != end())
            ++result.second;
        return result;
    }

}