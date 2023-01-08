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
#include <optional>
#include <neolib/core/allocator.hpp>
#include <neolib/core/vecarray.hpp>
#include <neolib/core/string_ci.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/gfx/text/glyph.hpp>
#include <neogfx/gfx/text/font.hpp>

namespace neogfx
{
    // case insensitive text
    typedef std::basic_string<char, neolib::ci_char_traits<std::char_traits<char> > > ci_string;

    enum class text_category : uint8_t
    {
        Unknown     = 0x00,
        None        = 0x01,
        Whitespace  = 0x02,
        Digit       = 0x03,
        LTR         = 0x04,
        RTL         = 0x05,
        Mark        = 0x06,
        Mnemonic    = 0x07,
        Emoji       = 0x08,
        Control     = 0x09
    };

    enum class text_direction : uint8_t
    {
        Invalid     = 0x00,
        LTR         = 0x01,
        RTL         = 0x02,
        None_LTR    = 0x04 | LTR,
        None_RTL    = 0x04 | RTL,
        Mark_LTR    = 0x08 | LTR,
        Mark_RTL    = 0x08 | RTL,
        Digit_LTR   = 0x10 | LTR, // unused
        Digit_RTL   = 0x10 | RTL,
        Emoji_LTR   = 0x20 | LTR,
        Emoji_RTL   = 0x20 | RTL,
        BaseMask    = LTR | RTL
    };

    inline text_direction base_text_direction(text_direction aDirection)
    {
        return static_cast<text_direction>(static_cast<uint8_t>(aDirection) & static_cast<uint8_t>(text_direction::BaseMask));
    }

    class character_type
    {
    public:
        text_category category;
        text_direction direction;
    public:
        character_type() :
            category(text_category::Unknown), direction(text_direction::LTR) {}
        character_type(text_category aCategory, text_direction aDirection) :
            category(aCategory), direction(aDirection) {}
        character_type(text_category aCategory) :
            category(aCategory), direction(text_direction::LTR) {}
    public:
        bool operator==(const character_type& aRhs) const { return category == aRhs.category && direction == aRhs.direction; }
        bool operator!=(const character_type& aRhs) const { return !(*this == aRhs); }
    };

    struct glyph_char
    {
        using value_type = std::uint32_t;
        using cluster_index = std::uint32_t;
        using cluster_range = std::pair<cluster_index, cluster_index>;
        enum flags_e : uint8_t
        {
            Default             = 0x00,
            Underline           = 0x01,
            Superscript         = 0x02,
            Subscript           = 0x04,
            BelowAscenderLine   = 0x08,
            AboveBaseline       = 0x08,
            Mnemonic            = 0x40,
            Subpixel            = 0x80,
        };

        value_type value;
        cluster_range clusters;
        character_type type;
        flags_e flags;
        font_id font;
        quadf_2d cell;
        quadf_2d shape;
    };

    inline bool operator==(const glyph_char& lhs, const glyph_char& rhs)
    {
        return lhs.type.category == rhs.type.category && lhs.value == rhs.value;
    }

    inline bool has_font(glyph_char const& g)
    {
        return g.font != font_id{};
    }

    inline text_category category(glyph_char const& g)
    {
        return g.type.category;
    }

    inline void set_category(glyph_char& g, text_category c)
    {
        g.type.category = c;
    }

    inline text_direction direction(glyph_char const& g)
    {
        return g.type.direction;
    }

    inline bool is_whitespace(glyph_char const& g)
    { 
        return category(g) == text_category::Whitespace; 
    };

    inline bool is_non_line_breaking_whitespace(glyph_char const& g)
    { 
        return is_whitespace(g) && g.value != U'\n';
    }

    inline bool is_line_breaking_whitespace(glyph_char const& g)
    { 
        return is_whitespace(g) && g.value == U'\n';
    }

    inline bool is_digit(glyph_char const& g)
    { 
        return category(g) == text_category::Digit;
    }

    inline bool is_emoji(glyph_char const& g)
    { 
        return category(g) == text_category::Emoji;
    }

    inline bool has_font_glyph(glyph_char const& g)
    {
        return has_font(g) && !is_whitespace(g) && !is_emoji(g);
    }

    inline bool left_to_right(glyph_char const& g)
    { 
        return direction(g) == text_direction::LTR; 
    }
    
    inline bool right_to_left(glyph_char const& g)
    { 
        return direction(g) == text_direction::RTL; 
    }
    
    inline bool category_has_no_direction(glyph_char const& g)
    { 
        return category(g) != text_category::LTR && category(g) != text_category::RTL;
    }

    inline bool underline(glyph_char const& g)
    { 
        return (g.flags & glyph_char::Underline) == glyph_char::Underline;
    }
    
    inline void set_underline(glyph_char& g, bool aUnderline)
    { 
        g.flags = static_cast<glyph_char::flags_e>(aUnderline ? g.flags | glyph_char::Underline : g.flags & ~glyph_char::Underline);
    }
    
    inline bool superscript(glyph_char const& g)
    {
        return (g.flags & glyph_char::Superscript) == glyph_char::Superscript;
    }

    inline bool below_ascender_line(glyph_char const& g)
    {
        return (g.flags & glyph_char::BelowAscenderLine) == glyph_char::BelowAscenderLine;
    }

    inline void set_superscript(glyph_char& g, bool aSuperscript, bool aBelowAscenderLine = false)
    {
        g.flags = static_cast<glyph_char::flags_e>(aSuperscript ? g.flags | glyph_char::Superscript : g.flags & ~glyph_char::Superscript);
        g.flags = static_cast<glyph_char::flags_e>(aBelowAscenderLine ? g.flags | glyph_char::BelowAscenderLine : g.flags & ~glyph_char::BelowAscenderLine);
    }

    inline bool subscript(glyph_char const& g)
    {
        return (g.flags & glyph_char::Subscript) == glyph_char::Subscript;
    }

    inline bool above_baseline(glyph_char const& g)
    {
        return (g.flags & glyph_char::AboveBaseline) == glyph_char::AboveBaseline;
    }

    inline void set_subscript(glyph_char& g, bool aSubscript, bool aAboveBaseline = false)
    {
        g.flags = static_cast<glyph_char::flags_e>(aSubscript ? g.flags | glyph_char::Subscript : g.flags & ~glyph_char::Subscript);
        g.flags = static_cast<glyph_char::flags_e>(aAboveBaseline ? g.flags | glyph_char::AboveBaseline : g.flags & ~glyph_char::AboveBaseline);
    }

    inline bool subpixel(glyph_char const& g)
    { 
        return (g.flags & glyph_char::Subpixel) == glyph_char::Subpixel;
    }
    
    inline void set_subpixel(glyph_char& g, bool aSubpixel)
    { 
        g.flags = static_cast<glyph_char::flags_e>(aSubpixel ? g.flags | glyph_char::Subpixel : g.flags & ~glyph_char::Subpixel);
    }

    inline bool mnemonic(glyph_char const& g)
    { 
        return (g.flags & glyph_char::Mnemonic) == glyph_char::Mnemonic;
    }

    inline void set_mnemonic(glyph_char& g, bool aMnemonic)
    { 
        g.flags = static_cast<glyph_char::flags_e>(aMnemonic ? g.flags | glyph_char::Mnemonic : g.flags & ~glyph_char::Mnemonic);
    }

    template <typename GlyphT, typename ConstIterator = GlyphT const*, typename Iterator = GlyphT*>
    class i_basic_glyph_text : public i_reference_counted
    {
        typedef i_basic_glyph_text<GlyphT, ConstIterator, Iterator> self_type;
    public:
        typedef self_type abstract_type;
    public:
        typedef GlyphT value_type;
        typedef value_type const* const_pointer;
        typedef value_type* pointer;
        typedef value_type const& const_reference;
        typedef value_type& reference;
        typedef ConstIterator const_iterator;
        typedef Iterator iterator;
        typedef std::size_t size_type;
        typedef std::ptrdiff_t difference_type;
        struct align_baselines_result
        {
            float yExtent;
            font_id majorFont;
            float baseline;
        };
    public:
        virtual void clone(i_ref_ptr<self_type>& aClone) const = 0;
        ref_ptr<self_type> clone() const
        {
            ref_ptr<self_type> result;
            clone(result);
            return result;
        }
    public:
        virtual const font& glyph_font() const = 0;
        virtual const font& glyph_font(const_reference aGlyphChar) const = 0;
        virtual void cache_glyph_font(font_id aFontId) const = 0;
        virtual void cache_glyph_font(const font& aFont) const = 0;
        virtual const i_glyph& glyph(const_reference aGlyphChar) const = 0;
    public:
        virtual const font& major_font() const = 0;
        virtual void set_major_font(const font& aFont) = 0;
        virtual scalar baseline() const = 0;
        virtual void set_baseline(scalar aBaseline) = 0;
    public:
        virtual bool empty() const = 0;
        virtual size_type size() const = 0;
        virtual void clear() = 0;
    public:
        virtual void push_back(const_reference aGlyphChar) = 0;
    public:
        virtual neogfx::size extents() const = 0;
        virtual neogfx::size extents(const_reference aGlyphChar) const = 0;
        virtual neogfx::size extents(const_iterator aBegin, const_iterator aEnd) const = 0;
    public:
        virtual void set_extents(const neogfx::size& aExtents) = 0;
        virtual self_type& align_baselines() = 0;
        virtual align_baselines_result align_baselines(iterator aBegin, iterator aEnd, bool aJustCalculate = false) = 0;
        virtual std::pair<const_iterator, const_iterator> word_break(const_iterator aBegin, const_iterator aFrom) const = 0;
        virtual std::pair<iterator, iterator> word_break(const_iterator aBegin, const_iterator aFrom) = 0;
    public:
        virtual i_vector<size_type> const& line_breaks() const = 0;
        virtual i_vector<size_type>& line_breaks() = 0;
    public:
        virtual const_iterator cbegin() const = 0;
        virtual const_iterator cend() const = 0;
        const_iterator begin() const
        {
            return cbegin();
        }
        const_iterator end() const
        {
            return cend();
        }
        virtual iterator begin() = 0;
        virtual iterator end() = 0;
    };

    using i_glyph_text = i_basic_glyph_text<glyph_char>;

    template <typename Container, typename ConstIterator = typename Container::const_iterator, typename Iterator = typename Container::iterator>
    class basic_glyph_text_content : public reference_counted<i_basic_glyph_text<typename Container::value_type, ConstIterator, Iterator>>, public Container
    {
        typedef basic_glyph_text_content<Container, ConstIterator, Iterator> self_type;
        typedef Container base_type;
    public:
        typedef i_basic_glyph_text<typename Container::value_type, ConstIterator, Iterator> abstract_type;
    public:
        using typename abstract_type::value_type;
        using typename abstract_type::const_pointer;
        using typename abstract_type::pointer;
        using typename abstract_type::const_reference;
        using typename abstract_type::reference;
        using typename abstract_type::const_iterator;
        using typename abstract_type::iterator;
        using typename abstract_type::size_type;
        using typename abstract_type::difference_type;
        using typename i_basic_glyph_text<typename Container::value_type, ConstIterator, Iterator>::align_baselines_result;
    private:
        static constexpr std::size_t SMALL_OPTIMIZATION_FONT_COUNT = 4;
        typedef Container container_type;
        class font_cache
        {
        private:
            typedef std::pair<neolib::small_cookie_ref_ptr, font> cache_entry;
            typedef std::unordered_map<font_id, cache_entry, std::hash<font_id>, std::equal_to<font_id>, neolib::fast_pool_allocator<std::pair<const font_id, cache_entry>>> cache;
        public:
            struct cached_font_not_found : std::logic_error { cached_font_not_found() : std::logic_error("neogfx::glyph_text_content::font_cache::cached_font_not_found") {} };
        public:
            const font& glyph_font() const;
            const font& glyph_font(font_id aFontId) const;
            const font& glyph_font(const_reference aGlyphChar) const;
            void cache_glyph_font(font_id aFontId) const;
            void cache_glyph_font(const font& aFont) const;
        public:
            void clear();
        private:
            mutable cache iCache;
        };
    public:
        basic_glyph_text_content();
        basic_glyph_text_content(font const& aFont);
        template <typename Iter>
        basic_glyph_text_content(Iter aBegin, Iter aEnd) :
            container_type{ aBegin, aEnd },
            iMajorFont{},
            iBaseline{}
        {
        }
        basic_glyph_text_content(const self_type& aOther);
        basic_glyph_text_content(self_type&& aOther);
    public:
        self_type& operator=(const self_type& aOther);
        self_type& operator=(self_type&& aOther);
    public:
        container_type const& container() const;
        container_type& container();
    public:
        void clone(i_ref_ptr<abstract_type>& aClone) const override;
    public:
        bool empty() const override;
        size_type size() const override;
        void clear() override;
    public:
        template< class... Args >
        reference emplace_back(Args&&... args)
        {
            auto& result = container_type::emplace_back(std::forward<Args>(args)...);
            iExtents = std::nullopt;
            return result;
        }
        void push_back(const_reference aGlyphChar) override;
    public:
        using container_type::back;
        reference back();
    public:
        const_iterator cbegin() const override;
        const_iterator cend() const override;
        using abstract_type::begin;
        using abstract_type::end;
        iterator begin() override;
        iterator end() override;
    public:
        bool operator==(const self_type& aOther) const;
    public:
        neogfx::size extents() const override;
        neogfx::size extents(const_reference aGlyphChar) const override;
        neogfx::size extents(const_iterator aBegin, const_iterator aEnd) const override;
    public:
        void set_extents(const neogfx::size& aExtents) override;
        self_type& align_baselines() override;
        align_baselines_result align_baselines(iterator aBegin, iterator aEnd, bool aJustCalculate = false) override;
        std::pair<const_iterator, const_iterator> word_break(const_iterator aBegin, const_iterator aFrom) const override;
        std::pair<iterator, iterator> word_break(const_iterator aBegin, const_iterator aFrom) override;
    public:
        vector<size_type> const& line_breaks() const override;
        vector<size_type>& line_breaks() override;
    public:
        const font& glyph_font() const override;
        const font& glyph_font(const_reference aGlyphChar) const override;
        void cache_glyph_font(font_id aFontId) const override;
        void cache_glyph_font(const font& aFont) const override;
        const i_glyph& glyph(const_reference aGlyphChar) const override;
    public:
        const font& major_font() const override;
        void set_major_font(const font& aFont) override;
        scalar baseline() const override;
        void set_baseline(scalar aBaseline) override;
    private:
        font_cache iCache;
        mutable std::optional<neogfx::size> iExtents;
        vector<size_type> iLineBreaks;
        font_id iMajorFont;
        scalar iBaseline;
    };

    constexpr std::size_t SMALL_OPTIMIZATION_GLYPH_TEXT_GLYPH_COUNT = 16;
    extern template class basic_glyph_text_content<neolib::vecarray<glyph_char, SMALL_OPTIMIZATION_GLYPH_TEXT_GLYPH_COUNT, -1>, glyph_char const*, glyph_char*>;
    
    using glyph_text_content = basic_glyph_text_content<neolib::vecarray<glyph_char, SMALL_OPTIMIZATION_GLYPH_TEXT_GLYPH_COUNT, -1>, glyph_char const*, glyph_char*>;

    class glyph_text
    {
    public:
        typedef i_glyph_text::const_pointer const_pointer;
        typedef i_glyph_text::pointer pointer;
        typedef i_glyph_text::const_reference const_reference;
        typedef i_glyph_text::reference reference;
        typedef i_glyph_text::const_iterator const_iterator;
        typedef i_glyph_text::iterator iterator;
        typedef i_glyph_text::size_type size_type;
        typedef i_glyph_text::difference_type difference_type;
    public:
        glyph_text();
        glyph_text(font const& aFont);
        glyph_text(i_glyph_text& aContents);
        glyph_text(glyph_text const& aOther);
        template <typename GlyphIter>
        glyph_text(GlyphIter aGlyphsBegin, GlyphIter aGlyphsEnd) :
            glyph_text{}
        {
            for (auto const& glyph : std::ranges::subrange(aGlyphsBegin, aGlyphsEnd))
            {
                content().push_back(glyph);
                content().cache_glyph_font(glyph.font);
            }
        }
    public:
        glyph_text operator=(const glyph_text& aOther);
        glyph_text clone() const;
    public:
        i_glyph_text& content() const;
    public:
        const font& glyph_font() const;
        const font& glyph_font(const_reference aGlyphChar) const;
        const i_glyph& glyph(const_reference aGlyphChar) const;
    public:
        bool empty() const;
        size_type size() const;
        void clear();
    public:
        neogfx::size extents() const;
        neogfx::size extents(const_reference aGlyphChar) const;
        neogfx::size extents(const_iterator aBegin, const_iterator aEnd) const;
    public:
        void set_extents(const neogfx::size& aExtents);
        std::pair<const_iterator, const_iterator> word_break(const_iterator aBegin, const_iterator aFrom) const;
        std::pair<iterator, iterator> word_break(const_iterator aBegin, const_iterator aFrom);
    public:
        const_reference back() const;
        reference back();
    public:
        const_iterator cbegin() const;
        const_iterator cend() const;
        const_iterator begin() const;
        const_iterator end() const;
        iterator begin();
        iterator end();
    private:
        mutable ref_ptr<i_glyph_text> iContent;
    };

    struct multiline_glyph_text
    {
        glyph_text glyphText;
        quad bbox;
        struct line
        {
            quad bbox;
            glyph_text::difference_type begin;
            glyph_text::difference_type end;
        };
        typedef neolib::vecarray<line, 8, -1> lines_t;
        lines_t lines;
    };

    template <typename Iter>
    inline text_direction glyph_text_direction(Iter aBegin, Iter aEnd)
    {
        text_direction result = text_direction::LTR;
        bool gotOne = false;
        for (Iter i = aBegin; i != aEnd; ++i)
        {
            if (!is_whitespace(*i) && !category_has_no_direction(*i))
            {
                if (!gotOne)
                {
                    gotOne = true;
                    result = direction(*i);
                }
                else
                {
                    if (result != direction(*i))
                        result = text_direction::LTR;
                }
            }
        }
        return result;
    }

    typedef std::optional<glyph_text> optional_glyph_text;

    class i_graphics_context;

    class i_font_selector
    {
    public:
        virtual ~i_font_selector() = default;
    public:
        virtual font select_font(std::size_t CharacterPos) const = 0;
    };

    class font_selector : public i_font_selector
    {
        typedef std::function<font(std::size_t)> function_type;
    public:
        explicit font_selector(function_type const& aSelectorFunction) :
            iSelectorFunction{ aSelectorFunction }
        {
        }
    public:
        font select_font(std::size_t CharacterPos) const override
        {
            return iSelectorFunction(CharacterPos);
        }
    private:
        function_type iSelectorFunction;
    };
    
    class i_glyph_text_factory
    {
    public:
        virtual ~i_glyph_text_factory() = default;
    public:
        virtual glyph_text create_glyph_text() = 0;
        virtual glyph_text create_glyph_text(font const& aFont) = 0;
        virtual glyph_text to_glyph_text(i_graphics_context const& aContext, char32_t const* aUtf32Begin, char32_t const* aUtf32End, i_font_selector const& aFontSelector, bool aAlignBaselines = true) = 0;
        virtual glyph_text to_glyph_text(i_graphics_context const& aContext, char const* aUtf8Begin, char const* aUtf8End, i_font_selector const& aFontSelector, bool aAlignBaselines = true) = 0;
    public:
        glyph_text to_glyph_text(i_graphics_context const& aContext, char32_t const* aUtf32Begin, char32_t const* aUtf32End, std::function<font(std::size_t)> aFontSelector, bool aAlignBaselines = true)
        {
            return to_glyph_text(aContext, aUtf32Begin, aUtf32End, font_selector{ aFontSelector }, aAlignBaselines);
        }
        glyph_text to_glyph_text(i_graphics_context const& aContext, std::u32string_view const& aString, std::function<font(std::size_t)> aFontSelector, bool aAlignBaselines = true)
        {
            return to_glyph_text(aContext, aString.data(), aString.data() + aString.size(), font_selector{ aFontSelector }, aAlignBaselines);
        }
        glyph_text to_glyph_text(i_graphics_context const& aContext, char const* aUtf8Begin, char const* aUtf8End, std::function<font(std::size_t)> aFontSelector, bool aAlignBaselines = true)
        {
            return to_glyph_text(aContext, aUtf8Begin, aUtf8End, font_selector{ aFontSelector }, aAlignBaselines);
        }
        glyph_text to_glyph_text(i_graphics_context const& aContext, std::string_view const& aString, std::function<font(std::size_t)> aFontSelector, bool aAlignBaselines = true)
        {
            return to_glyph_text(aContext, aString.data(), aString.data() + aString.size(), font_selector{ aFontSelector }, aAlignBaselines);
        }
        glyph_text to_glyph_text(i_graphics_context const& aContext, i_string const& aString, std::function<font(std::size_t)> aFontSelector, bool aAlignBaselines = true)
        {
            return to_glyph_text(aContext, aString.c_str(), aString.c_str() + aString.size(), font_selector{ aFontSelector }, aAlignBaselines);
        }
    };
}