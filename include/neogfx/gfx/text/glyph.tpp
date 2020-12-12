// glyph.tpp
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
#include <neogfx/gfx/text/glyph.hpp>
#include <neogfx/gfx/text/i_font_manager.hpp>

namespace neogfx
{
    template <typename Container, typename ConstIterator, typename Iterator>
    const font& basic_glyph_text_content<Container, ConstIterator, Iterator>::font_cache::glyph_font() const
    {
        if (!iCache.empty())
            return iCache.begin()->second.second;
        throw cached_font_not_found();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    const font& basic_glyph_text_content<Container, ConstIterator, Iterator>::font_cache::glyph_font(const_reference aGlyph) const
    {
        cache_glyph_font(aGlyph.font);
        auto existing = iCache.find(aGlyph.font);
        if (existing != iCache.end())
            return existing->second.second;
        throw cached_font_not_found();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    void basic_glyph_text_content<Container, ConstIterator, Iterator>::font_cache::cache_glyph_font(font_id aFontId) const
    {
        if (iCache.find(aFontId) == iCache.end())
        {
            auto& fontService = service<i_font_manager>();
            iCache.emplace(aFontId, cache_entry{ neolib::small_cookie_ref_ptr{ fontService, aFontId }, fontService.font_from_id(aFontId) });
        }
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    void basic_glyph_text_content<Container, ConstIterator, Iterator>::font_cache::cache_glyph_font(const font& aFont) const
    {
        if (iCache.find(aFont.id()) == iCache.end())
        {
            auto& fontService = service<i_font_manager>();
            iCache.emplace(aFont.id(), cache_entry{ neolib::small_cookie_ref_ptr{ fontService, aFont.id() }, aFont });
        }
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    void basic_glyph_text_content<Container, ConstIterator, Iterator>::font_cache::clear()
    {
        iCache.clear();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    basic_glyph_text_content<Container, ConstIterator, Iterator>::basic_glyph_text_content() :
        container_type{}
    {
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    basic_glyph_text_content<Container, ConstIterator, Iterator>::basic_glyph_text_content(const self_type& aOther) :
        container_type{ aOther },
        iCache{ aOther.iCache },
        iExtents{ aOther.iExtents }
    {
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    basic_glyph_text_content<Container, ConstIterator, Iterator>::basic_glyph_text_content(self_type&& aOther) :
        container_type{ std::move(aOther) },
        iCache{ std::move(aOther.iCache) },
        iExtents{ aOther.iExtents }
    {
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    basic_glyph_text_content<Container, ConstIterator, Iterator>& basic_glyph_text_content<Container, ConstIterator, Iterator>::operator=(const self_type& aOther)
    {
        if (&aOther == this)
            return *this;
        container_type::operator=(aOther);
        iCache = aOther.iCache;
        iExtents = aOther.iExtents;
        return *this;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    basic_glyph_text_content<Container, ConstIterator, Iterator>& basic_glyph_text_content<Container, ConstIterator, Iterator>::operator=(self_type&& aOther)
    {
        if (&aOther == this)
            return *this;
        container_type::operator=(std::move(aOther));
        iCache = aOther.iCache;
        iExtents = aOther.iExtents;
        return *this;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    bool basic_glyph_text_content<Container, ConstIterator, Iterator>::empty() const
    {
        return container_type::empty();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    typename basic_glyph_text_content<Container, ConstIterator, Iterator>::size_type basic_glyph_text_content<Container, ConstIterator, Iterator>::size() const
    {
        return container_type::size();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    typename basic_glyph_text_content<Container, ConstIterator, Iterator>::const_iterator basic_glyph_text_content<Container, ConstIterator, Iterator>::cbegin() const
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
    typename basic_glyph_text_content<Container, ConstIterator, Iterator>::const_iterator basic_glyph_text_content<Container, ConstIterator, Iterator>::cend() const
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
    typename basic_glyph_text_content<Container, ConstIterator, Iterator>::iterator basic_glyph_text_content<Container, ConstIterator, Iterator>::begin()
    {
        iExtents = std::nullopt;
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
    typename basic_glyph_text_content<Container, ConstIterator, Iterator>::iterator basic_glyph_text_content<Container, ConstIterator, Iterator>::end()
    {
        iExtents = std::nullopt;
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
    typename basic_glyph_text_content<Container, ConstIterator, Iterator>::reference basic_glyph_text_content<Container, ConstIterator, Iterator>::back()
    {
        iExtents = std::nullopt;
        return container_type::back();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    void basic_glyph_text_content<Container, ConstIterator, Iterator>::push_back(const_reference aGlyph)
    {
        container_type::push_back(aGlyph);
        iExtents = std::nullopt;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    void basic_glyph_text_content<Container, ConstIterator, Iterator>::clear()
    {
        container_type::clear();
        iCache.clear();
        iExtents = std::nullopt;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    bool basic_glyph_text_content<Container, ConstIterator, Iterator>::operator==(const self_type& aOther) const
    {
        return static_cast<const container_type&>(*this) == static_cast<const container_type&>(aOther);
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    Container const& basic_glyph_text_content<Container, ConstIterator, Iterator>::container() const
    {
        return *this;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    Container& basic_glyph_text_content<Container, ConstIterator, Iterator>::container()
    {
        return *this;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    size basic_glyph_text_content<Container, ConstIterator, Iterator>::extents() const
    {
        if (iExtents == std::nullopt)
            iExtents = extents(begin(), end());
        return *iExtents;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    size basic_glyph_text_content<Container, ConstIterator, Iterator>::extents(const_reference aGlyph) const
    {
        if (aGlyph.extents != basic_size<float>{})
            return aGlyph.extents;
        if (!has_font_glyph(aGlyph))
            return aGlyph.extents;
        else if (aGlyph.extents == basic_size<float>{})
        {
            auto const& glyphFont = glyph_font(aGlyph);
            if (has_font_glyph(aGlyph))
            {
                auto const& glyphTexture = glyph_texture(aGlyph);
                aGlyph.extents = neogfx::size{ static_cast<float>(offset(aGlyph).cx + glyphTexture.placement().x + glyphTexture.texture().extents().cx), glyphFont.height() };
            }
            else
                aGlyph.extents = neogfx::size{ advance(aGlyph).cx, !is_emoji(aGlyph) ? glyphFont.height() : advance(aGlyph).cx };
        }
        return aGlyph.extents;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    size basic_glyph_text_content<Container, ConstIterator, Iterator>::extents(const_iterator aBegin, const_iterator aEnd, bool aEndIsLineEnd) const
    {
        if (aBegin == aEnd)
            return neogfx::size{ 0.0, 0.0 };
        neogfx::size result;
        for (basic_glyph_text_content<Container, ConstIterator, Iterator>::const_iterator i = aBegin; i != aEnd; ++i)
        {
            auto const& g = *i;
            result.cx += advance(g).cx;
            result.cy = std::max(result.cy, extents(g).cy);
        }
        if (aEndIsLineEnd)
        {
            auto const& lastGlyph = *std::prev(aEnd);
            result.cx += (extents(lastGlyph).cx - advance(lastGlyph).cx);
        }
        return result.ceil();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    void basic_glyph_text_content<Container, ConstIterator, Iterator>::set_extents(const neogfx::size& aExtents)
    {
        iExtents = aExtents;
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    std::pair<typename basic_glyph_text_content<Container, ConstIterator, Iterator>::const_iterator, typename basic_glyph_text_content<Container, ConstIterator, Iterator>::const_iterator> basic_glyph_text_content<Container, ConstIterator, Iterator>::word_break(const_iterator aBegin, const_iterator aFrom) const
    {
        std::pair<const_iterator, const_iterator> result(aFrom, aFrom);
        if (!is_whitespace(*aFrom))
        {
            while (result.first != aBegin && !is_whitespace(*result.first))
                --result.first;
            if (!is_whitespace(*result.first))
            {
                result.first = aFrom;
                while (result.first != aBegin && (result.first - 1)->source == aFrom->source)
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
    const font& basic_glyph_text_content<Container, ConstIterator, Iterator>::glyph_font() const
    {
        return iCache.glyph_font();
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    const font& basic_glyph_text_content<Container, ConstIterator, Iterator>::glyph_font(const_reference aGlyph) const
    {
        return iCache.glyph_font(aGlyph);
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    void basic_glyph_text_content<Container, ConstIterator, Iterator>::cache_glyph_font(font_id aFontId) const
    {
        iCache.cache_glyph_font(aFontId);
    }

    template <typename Container, typename ConstIterator, typename Iterator>
    void basic_glyph_text_content<Container, ConstIterator, Iterator>::cache_glyph_font(const font& aFont) const
    {
        iCache.cache_glyph_font(aFont);
    }
    
    template <typename Container, typename ConstIterator, typename Iterator>
    const i_glyph_texture& basic_glyph_text_content<Container, ConstIterator, Iterator>::glyph_texture(const_reference aGlyph) const
    {
        return glyph_font(aGlyph).glyph_texture(aGlyph);
    }
}