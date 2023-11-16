// glyph_char.ipp
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
#include <neogfx/gfx/text/glyph_text.hpp>
#include <neogfx/gfx/text/i_font_manager.hpp>

namespace neogfx
{
    template <typename Container, typename ConstIterator, typename Iterator>
    inline const font& basic_glyph_text_content<Container, ConstIterator, Iterator>::font_cache::glyph_font() const
    {
        if (!iCache.empty())
            return iCache.begin()->second.second;
        throw cached_font_not_found();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline const font& basic_glyph_text_content<Container, ConstIterator, Iterator>::font_cache::glyph_font(font_id aFontId) const
    {
        cache_glyph_font(aFontId);
        auto existing = iCache.find(aFontId);
        if (existing != iCache.end())
            return existing->second.second;
        throw cached_font_not_found();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline const font& basic_glyph_text_content<Container, ConstIterator, Iterator>::font_cache::glyph_font(const_reference aGlyphChar) const
    {
        return glyph_font(aGlyphChar.font);
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline void basic_glyph_text_content<Container, ConstIterator, Iterator>::font_cache::cache_glyph_font(font_id aFontId) const
    {
        if (iCache.find(aFontId) == iCache.end())
        {
            auto& fontService = service<i_font_manager>();
            iCache.emplace(aFontId, cache_entry{ neolib::small_cookie_ref_ptr{ fontService, aFontId }, fontService.font_from_id(aFontId) });
        }
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline void basic_glyph_text_content<Container, ConstIterator, Iterator>::font_cache::cache_glyph_font(const font& aFont) const
    {
        if (iCache.find(aFont.id()) == iCache.end())
        {
            auto& fontService = service<i_font_manager>();
            iCache.emplace(aFont.id(), cache_entry{ neolib::small_cookie_ref_ptr{ fontService, aFont.id() }, aFont });
        }
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline void basic_glyph_text_content<Container, ConstIterator, Iterator>::font_cache::clear()
    {
        iCache.clear();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline basic_glyph_text_content<Container, ConstIterator, Iterator>::basic_glyph_text_content() :
        container_type{},
        iMajorFont{},
        iBaseline{}
    {
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline basic_glyph_text_content<Container, ConstIterator, Iterator>::basic_glyph_text_content(font const& aFont) :
        container_type{},
        iMajorFont{ aFont.id() },
        iBaseline{}
    {
        cache_glyph_font(aFont.id());
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline basic_glyph_text_content<Container, ConstIterator, Iterator>::basic_glyph_text_content(const self_type& aOther) :
        container_type{ aOther },
        iCache{ aOther.iCache },
        iExtents{ aOther.iExtents },
        iMajorFont{ aOther.iMajorFont },
        iBaseline{ aOther.iBaseline }
    {
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline basic_glyph_text_content<Container, ConstIterator, Iterator>::basic_glyph_text_content(self_type&& aOther) :
        container_type{ std::move(aOther) },
        iCache{ std::move(aOther.iCache) },
        iExtents{ aOther.iExtents },
        iMajorFont{ aOther.iMajorFont },
        iBaseline{ aOther.iBaseline }
    {
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline basic_glyph_text_content<Container, ConstIterator, Iterator>& basic_glyph_text_content<Container, ConstIterator, Iterator>::operator=(const self_type& aOther)
    {
        if (&aOther == this)
            return *this;
        container_type::operator=(aOther);
        iCache = aOther.iCache;
        iExtents = aOther.iExtents;
        iMajorFont = aOther.iMajorFont;
        iBaseline = aOther.iBaseline;
        return *this;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline basic_glyph_text_content<Container, ConstIterator, Iterator>& basic_glyph_text_content<Container, ConstIterator, Iterator>::operator=(self_type&& aOther)
    {
        if (&aOther == this)
            return *this;
        container_type::operator=(std::move(aOther));
        iCache = aOther.iCache;
        iExtents = aOther.iExtents;
        iMajorFont = aOther.iMajorFont;
        iBaseline = aOther.iBaseline;
        return *this;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline void basic_glyph_text_content<Container, ConstIterator, Iterator>::clone(i_ref_ptr<abstract_type>& aClone) const
    {
        aClone = make_ref<self_type>(*this);
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline bool basic_glyph_text_content<Container, ConstIterator, Iterator>::empty() const
    {
        return container_type::empty();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline typename basic_glyph_text_content<Container, ConstIterator, Iterator>::size_type basic_glyph_text_content<Container, ConstIterator, Iterator>::size() const
    {
        return container_type::size();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline typename basic_glyph_text_content<Container, ConstIterator, Iterator>::const_iterator basic_glyph_text_content<Container, ConstIterator, Iterator>::cbegin() const
    {
        if constexpr (std::is_same_v<const_iterator, typename container_type::const_iterator>)
            return container_type::cbegin();
        else
        {
            if (!empty())
                return &*container_type::cbegin();
            else
                return nullptr;
        }
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline typename basic_glyph_text_content<Container, ConstIterator, Iterator>::const_iterator basic_glyph_text_content<Container, ConstIterator, Iterator>::cend() const
    {
        if constexpr (std::is_same_v<const_iterator, typename container_type::const_iterator>)
            return container_type::cend();
        else
        {
            if (!empty())
                return std::next(&*std::prev(container_type::cend()));
            else
                return nullptr;
        }
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline typename basic_glyph_text_content<Container, ConstIterator, Iterator>::iterator basic_glyph_text_content<Container, ConstIterator, Iterator>::begin()
    {
        iExtents = invalid;
        if constexpr (std::is_same_v<iterator, typename container_type::iterator>)
            return container_type::begin();
        else
        {
            if (!empty())
                return &*container_type::begin();
            else
                return nullptr;
        }
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline typename basic_glyph_text_content<Container, ConstIterator, Iterator>::iterator basic_glyph_text_content<Container, ConstIterator, Iterator>::end()
    {
        iExtents = invalid;
        if constexpr (std::is_same_v<iterator, typename container_type::iterator>)
            return container_type::end();
        else
        {
            if (!empty())
                return std::next(&*std::prev(container_type::end()));
            else
                return nullptr;
        }
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline typename basic_glyph_text_content<Container, ConstIterator, Iterator>::reference basic_glyph_text_content<Container, ConstIterator, Iterator>::back()
    {
        iExtents = invalid;
        return container_type::back();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline void basic_glyph_text_content<Container, ConstIterator, Iterator>::push_back(const_reference aGlyphChar)
    {
        container_type::push_back(aGlyphChar);
        iExtents = invalid;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline void basic_glyph_text_content<Container, ConstIterator, Iterator>::clear()
    {
        container_type::clear();
        iCache.clear();
        iExtents = invalid;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline bool basic_glyph_text_content<Container, ConstIterator, Iterator>::operator==(const self_type& aOther) const
    {
        return static_cast<const container_type&>(*this) == static_cast<const container_type&>(aOther);
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline Container const& basic_glyph_text_content<Container, ConstIterator, Iterator>::container() const
    {
        return *this;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline Container& basic_glyph_text_content<Container, ConstIterator, Iterator>::container()
    {
        return *this;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline size basic_glyph_text_content<Container, ConstIterator, Iterator>::extents() const
    {
        if (!iExtents)
            iExtents = extents(begin(), end());
        return *iExtents;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline size basic_glyph_text_content<Container, ConstIterator, Iterator>::extents(const_reference aGlyphChar) const
    {
        return rect{ to_aabb_2d(aGlyphChar.cell.begin(), aGlyphChar.cell.end()) }.extents().ceil();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline size basic_glyph_text_content<Container, ConstIterator, Iterator>::extents(const_iterator aBegin, const_iterator aEnd) const
    {
        if (aBegin == aEnd)
            return neogfx::size{ 0.0, glyph_font().height() };
        auto const& firstGlyph = *aBegin;
        auto const& lastGlyph = *std::prev(aEnd);
        quad_2d const quadExtents{ 
            firstGlyph.cell[0],
            lastGlyph.cell[1],
            lastGlyph.cell[3],
            firstGlyph.cell[3] };
        rect const boundingRect{ to_aabb_2d(quadExtents.begin(), quadExtents.end()) };
        return boundingRect.extents().ceil();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline void basic_glyph_text_content<Container, ConstIterator, Iterator>::set_extents(const neogfx::size& aExtents)
    {
        iExtents = aExtents;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline typename basic_glyph_text_content<Container, ConstIterator, Iterator>::self_type& basic_glyph_text_content<Container, ConstIterator, Iterator>::align_baselines()
    {
        auto result = align_baselines(begin(), end());
        iMajorFont = result.majorFont;
        iBaseline = result.baseline;
        return *this;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline basic_glyph_text_content<Container, ConstIterator, Iterator>::align_baselines_result basic_glyph_text_content<Container, ConstIterator, Iterator>::align_baselines(iterator aBegin, iterator aEnd, bool aJustCalculate)
    {
        align_baselines_result result = {};
        float cyMax = 0.0f;
        float yMax = 0.0f;
        if (aBegin != aEnd)
        {
            for (auto const& g : std::ranges::subrange(aBegin, aEnd))
            {
                auto const& gf = glyph_font(g);
                auto const existingExtents = quad_extents(g.cell);
                result.yExtent = std::max(result.yExtent, existingExtents.y);
                float cy = existingExtents.y + static_cast<float>(gf.descender());
                if (cy > cyMax)
                {
                    cyMax = cy;
                    result.majorFont = gf.id();
                    result.baseline = cyMax;
                }
                yMax = std::max(yMax, existingExtents.y);
            }
        }
        else
        {
            result.majorFont = major_font().id();
            result.yExtent = static_cast<float>(major_font().height());
        }
        if (aJustCalculate)
            return result;
        for (auto& g : std::ranges::subrange(aBegin, aEnd))
        {
            auto const& gf = glyph_font(g);
            auto const existingExtents = quad_extents(g.cell);
            auto shapeOffset = vec2f{ 0.0f, cyMax - (existingExtents.y + static_cast<float>(gf.descender())) };
            g.cell[2].y = g.cell[1].y + yMax;
            g.cell[3].y = g.cell[0].y + yMax;
            if ((g.flags & (glyph_char::Superscript | glyph_char::Subscript)) != glyph_char::Default)
            {
                scalar const ascender = gf.ascender();
                scalar const descender = gf.descender();
                scalar const cy = ascender - descender;
                scalar const dyLarge = cy / 0.58 * 0.33; // todo: make configurable attributes
                scalar const dySmall = dyLarge * 0.5; // todo: make configurable attributes
                if ((g.flags & glyph_char::Superscript) == glyph_char::Superscript)
                {
                    auto const belowAscenderDelta = static_cast<float>(dySmall);
                    auto const aboveAscenderDelta = static_cast<float>(dyLarge);
                    if ((g.flags & glyph_char::BelowAscenderLine) == glyph_char::BelowAscenderLine)
                        shapeOffset -= vec2f{ 0.0f, std::ceil(belowAscenderDelta) };
                    else
                        shapeOffset -= vec2f{ 0.0f, std::ceil(aboveAscenderDelta) };
                }
                else if ((g.flags & glyph_char::Subscript) == glyph_char::Subscript)
                {
                    auto const aboveBaselineDelta = 0.0f;
                    auto const belowBaselineDelta = static_cast<float>(dyLarge + descender / 0.58);
                    if ((g.flags & glyph_char::AboveBaseline) == glyph_char::AboveBaseline)
                        shapeOffset += vec2f{ 0.0f, aboveBaselineDelta };
                    else
                        shapeOffset += vec2f{ 0.0f, belowBaselineDelta };
                }
            }
            g.shape += shapeOffset;
            if (g.outlineShape)
                g.outlineShape.value() += shapeOffset;
        }
        return result;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline std::pair<typename basic_glyph_text_content<Container, ConstIterator, Iterator>::const_iterator, typename basic_glyph_text_content<Container, ConstIterator, Iterator>::const_iterator> basic_glyph_text_content<Container, ConstIterator, Iterator>::word_break(const_iterator aBegin, const_iterator aFrom) const
    {
        std::pair<const_iterator, const_iterator> result(aFrom, aFrom);
        if (!is_whitespace(*aFrom))
        {
            while (result.first != aBegin && !is_whitespace(*result.first))
                --result.first;
            if (!is_whitespace(*result.first))
            {
                result.first = aFrom;
                while (result.first != aBegin && (result.first - 1)->clusters == aFrom->clusters)
                    --result.first;
                result.second = result.first;
                return result;
            }
            result.second = result.first;
        }
        while (result.first != aBegin && is_whitespace(*(result.first - 1)))
            --result.first;
        while (is_whitespace(*result.second) && result.second != end())
            ++result.second;
        return result;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline std::pair<typename basic_glyph_text_content<Container, ConstIterator, Iterator>::iterator, typename basic_glyph_text_content<Container, ConstIterator, Iterator>::iterator> basic_glyph_text_content<Container, ConstIterator, Iterator>::word_break(const_iterator aBegin, const_iterator aFrom)
    {
        auto result = const_cast<const self_type&>(*this).word_break(aBegin, aFrom);
        return std::make_pair(std::next(begin(), std::distance(cbegin(), result.first)), std::next(begin(), std::distance(cbegin(), result.second)));
    }
        
    template <typename Container, typename ConstIterator, typename Iterator>
    inline vector<typename basic_glyph_text_content<Container, ConstIterator, Iterator>::size_type> const& basic_glyph_text_content<Container, ConstIterator, Iterator>::line_breaks() const
    {
        return iLineBreaks;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline vector<typename basic_glyph_text_content<Container, ConstIterator, Iterator>::size_type>& basic_glyph_text_content<Container, ConstIterator, Iterator>::line_breaks()
    {
        return iLineBreaks;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline const font& basic_glyph_text_content<Container, ConstIterator, Iterator>::glyph_font() const
    {
        return iCache.glyph_font();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline const font& basic_glyph_text_content<Container, ConstIterator, Iterator>::glyph_font(const_reference aGlyphChar) const
    {
        return iCache.glyph_font(aGlyphChar);
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline void basic_glyph_text_content<Container, ConstIterator, Iterator>::cache_glyph_font(font_id aFontId) const
    {
        iCache.cache_glyph_font(aFontId);
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline void basic_glyph_text_content<Container, ConstIterator, Iterator>::cache_glyph_font(const font& aFont) const
    {
        iCache.cache_glyph_font(aFont);
    }
    
    template <typename Container, typename ConstIterator, typename Iterator>
    inline const i_glyph& basic_glyph_text_content<Container, ConstIterator, Iterator>::glyph(const_reference aGlyphChar) const
    {
        return glyph_font(aGlyphChar).glyph(aGlyphChar);
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline void basic_glyph_text_content<Container, ConstIterator, Iterator>::set_major_font(const font& aFont)
    {
        iMajorFont = aFont.id();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline const font& basic_glyph_text_content<Container, ConstIterator, Iterator>::major_font() const
    {
        return iCache.glyph_font(iMajorFont);
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline scalar basic_glyph_text_content<Container, ConstIterator, Iterator>::baseline() const
    {
        return iBaseline;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    inline void basic_glyph_text_content<Container, ConstIterator, Iterator>::set_baseline(scalar aBaseline)
    {
        iBaseline = aBaseline;
    }
}