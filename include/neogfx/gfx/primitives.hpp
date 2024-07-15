// graphics_context.hpp
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

#include <memory>
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4459 ) // declaration of 'name' hides global declaration
#endif
#ifdef _WIN32
#pragma warning( pop )
#endif
#include <optional>

#include <neolib/core/vecarray.hpp>

#include <neogfx/core/primitives.hpp>
#include <neogfx/gfx/path.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/gfx/sub_texture.hpp>
#include <neogfx/gfx/pen.hpp>

namespace neogfx
{
    enum class blending_mode
    {
        None,
        Default, // todo
        Blit
    };

    enum class smoothing_mode
    {
        None,
        AntiAlias
    };

    enum class logical_operation
    {
        None,
        Xor
    };

    enum class shader_effect
    {
        None            = 0,
        Colorize        = 1,
        ColorizeAverage = Colorize,
        ColorizeMaximum = 2,
        ColorizeSpot    = 3,
        ColorizeAlpha   = 4,
        Monochrome      = 5,
        Filter          = 10,
        Ignore          = 99
    };

    enum class shader_filter
    {
        None            = 0,
        GaussianBlur    = 1
    };

    enum class shader_shape
    {
        None        = 0x00,
        Line        = 0x01,
        CubicBezier = 0x02,
        Triangle    = 0x03,
        Rect        = 0x04,
        Circle      = 0x05,
        Ellipse     = 0x06,
        Pie         = 0x07,
        Arc         = 0x08,
        RoundedRect = 0x09,
        EllipseRect = 0x0A
    };

    enum class blurring_algorithm
    {
        None,
        Gaussian
    };

    typedef neolib::variant<color, gradient, texture, std::pair<texture, rect>, sub_texture, std::pair<sub_texture, rect>> brush;

    inline brush to_brush(const color_or_gradient& aEffectColor)
    {
        if (std::holds_alternative<color>(aEffectColor))
            return std::get<color>(aEffectColor);
        else if (std::holds_alternative<gradient>(aEffectColor))
            return std::get<gradient>(aEffectColor);
        else
            return color{};
    }

    class text_color : public color_or_gradient
    {
    public:
        using color_or_gradient::color_or_gradient;
        text_color(color_or_gradient const& aOther) :
            color_or_gradient{ aOther }
        {
        }
    public:
        using color_or_gradient::operator=;
        text_color& operator=(color_or_gradient const& aOther)
        {
            color_or_gradient::operator=(aOther);
            return *this;
        }
    public:
        color::component alpha() const
        {
            if (std::holds_alternative<color>(*this))
                return std::get<color>(*this).alpha();
            else
                return 255;
        }
        text_color with_alpha(color::component aAlpha) const
        {
            if (std::holds_alternative<color>(*this))
                return std::get<color>(*this).with_alpha(aAlpha);
            if (std::holds_alternative<gradient>(*this))
                return std::get<gradient>(*this).with_combined_alpha(aAlpha);
            else
                return text_color{};
        }
    };

    inline bool operator==(color_or_gradient const& lhs, color const& rhs) noexcept
    {
        return std::holds_alternative<color>(lhs) && std::get<color>(lhs) == rhs;
    }

    inline bool operator==(color const& lhs, color_or_gradient const& rhs) noexcept
    {
        return std::holds_alternative<color>(rhs) && std::get<color>(rhs) == lhs;
    }

    inline bool operator!=(color_or_gradient const& lhs, color const& rhs) noexcept
    {
        return !std::holds_alternative<color>(lhs) || std::get<color>(lhs) != rhs;
    }

    inline bool operator!=(color const& lhs, color_or_gradient const& rhs) noexcept
    {
        return !std::holds_alternative<color>(rhs) && std::get<color>(rhs) != lhs;
    }

    inline bool operator==(color_or_gradient const& lhs, gradient const& rhs) noexcept
    {
        return std::holds_alternative<gradient>(lhs) && std::get<gradient>(lhs) == rhs;
    }

    inline bool operator==(gradient const& lhs, color_or_gradient const& rhs) noexcept
    {
        return std::holds_alternative<gradient>(rhs) && std::get<gradient>(rhs) == lhs;
    }

    inline bool operator!=(color_or_gradient const& lhs, gradient const& rhs) noexcept
    {
        return !std::holds_alternative<gradient>(lhs) || std::get<gradient>(lhs) != rhs;
    }

    inline bool operator!=(gradient const& lhs, color_or_gradient const& rhs) noexcept
    {
        return !std::holds_alternative<gradient>(rhs) && std::get<gradient>(rhs) != lhs;
    }

    typedef neolib::optional<text_color> optional_text_color;

    enum class text_effect_type : std::uint32_t
    {
        None,
        Outline,
        Glow,
        Shadow
    };
}

begin_declare_enum(neogfx::text_effect_type)
declare_enum_string(neogfx::text_effect_type, None)
declare_enum_string(neogfx::text_effect_type, Outline)
declare_enum_string(neogfx::text_effect_type, Glow)
declare_enum_string(neogfx::text_effect_type, Shadow)
end_declare_enum(neogfx::text_effect_type)

namespace neogfx
{
    class text_effect
    {
    public:
        typedef text_effect abstract_type; // todo
    public:
        typedef double auxiliary_parameter;
        typedef optional<auxiliary_parameter> optional_auxiliary_parameter;
    public:
        text_effect() :
            iType{ text_effect_type::None }, iColor{}, iWidth{}, iAux1{}, iIgnoreEmoji{}
        {
        }
        text_effect(text_effect_type aType, const text_color& aColor, const optional_dimension& aWidth = {}, const optional_vec3& aOffset = {}, const optional_auxiliary_parameter& aAux1 = {}, bool aIgnoreEmoji = false) :
            iType{ aType }, iColor{ aColor }, iWidth{ aWidth }, iAux1{ aAux1 }, iIgnoreEmoji{ aIgnoreEmoji }
        {
        }
    public:
        text_effect& operator=(const text_effect& aOther)
        {
            if (&aOther == this)
                return *this;
            iType = aOther.iType;
            iColor = aOther.iColor;
            iWidth = aOther.iWidth;
            iOffset = aOther.iOffset;
            iAux1 = aOther.iAux1;
            iIgnoreEmoji = aOther.iIgnoreEmoji;
            return *this;
        }
    public:
        bool operator==(const text_effect& that) const noexcept
        {
            return std::forward_as_tuple(iType, iColor, iWidth, iOffset, iAux1, iIgnoreEmoji) ==
                std::forward_as_tuple(that.iType, that.iColor, that.iWidth, that.iOffset, that.iAux1, that.iIgnoreEmoji);
        }
        auto operator<=>(const text_effect& that) const noexcept
        {
            return std::forward_as_tuple(iType, iColor, iWidth, iOffset, iAux1, iIgnoreEmoji) <=>
                std::forward_as_tuple(that.iType, that.iColor, that.iWidth, that.iOffset, that.iAux1, that.iIgnoreEmoji);
        }
    public:
        text_effect_type type() const
        {
            return iType;
        }
        void set_type(text_effect_type aType)
        {
            iType = aType;
        }
        text_color color() const
        {
            if (iColor != neolib::none)
                return iColor;
            switch (type())
            {
            case text_effect_type::Shadow:
                return color::Black.with_alpha(0.5);
            default:
                return color::White;
            }
        }
        void set_color(text_color const& aColor)
        {
            iColor = aColor;
        }
        bool has_width() const
        {
            return iWidth != std::nullopt;
        }
        dimension width() const
        {
            if (iWidth != std::nullopt)
                return *iWidth;
            switch (type())
            {
            case text_effect_type::None:
            default:
                return 0.0;
            case text_effect_type::Outline:
                return 1.0;
            case text_effect_type::Glow:
            case text_effect_type::Shadow:
                return 4.0;
            }
        }
        void set_width(optional_dimension const& aWidth)
        {
            iWidth = aWidth;
        }
        vec3 offset() const
        {
            if (iOffset != std::nullopt)
                return *iOffset;
            switch (type())
            {
            case text_effect_type::None:
            default:
                return {};
            case text_effect_type::Outline:
                return { -width(), -width() };
            case text_effect_type::Glow:
                return {};
            case text_effect_type::Shadow:
                return { std::max(1.0, width() / 2.0), std::max(1.0, width() / 2.0) };
            }
        }
        void set_offset(vec3 const& aOffset)
        {
            iOffset = aOffset;
        }
        double aux1() const
        {
            if (iAux1 != std::nullopt)
                return *iAux1;
            switch (type())
            {
            case text_effect_type::None:
            default:
                return 0.0;
            case text_effect_type::Outline:
                return 0.0;
            case text_effect_type::Glow:
            case text_effect_type::Shadow:
                return 1.0;
            }
        }
        void set_aux1(double aAux1)
        {
            iAux1 = aAux1;
        }
        bool ignore_emoji() const
        {
            return iIgnoreEmoji;
        }
        void set_ignore_emoji(bool aIgnore)
        {
            iIgnoreEmoji = aIgnore;
        }
        text_effect with_alpha(color::component aAlpha) const
        {
            return text_effect{ iType, iColor.with_alpha(aAlpha), iWidth, iOffset, iAux1, iIgnoreEmoji };
        }
        text_effect with_alpha(double aAlpha) const
        {
            return with_alpha(static_cast<color::component>(aAlpha * 255));
        }
    private:
        text_effect_type iType;
        text_color iColor;
        optional_dimension iWidth;
        optional_vec3 iOffset;
        optional_auxiliary_parameter iAux1;
        bool iIgnoreEmoji;
    };

    typedef neolib::optional<text_effect> optional_text_effect;

    template <typename Elem, typename Traits>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const text_effect& aEffect)
    {
        aStream << '[';
        aStream << aEffect.type();
        aStream << ',';
        aStream << aEffect.color();
        aStream << ',';
        aStream << aEffect.width();
        aStream << ',';
        aStream << aEffect.offset();
        aStream << ',';
        aStream << aEffect.aux1();
        aStream << ',';
        aStream << aEffect.ignore_emoji();
        aStream << ']';
        return aStream;
    }

    template <typename Elem, typename Traits>
    inline std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& aStream, text_effect& aEffect)
    {
        auto previousImbued = aStream.getloc();
        if (typeid(std::use_facet<std::ctype<char>>(previousImbued)) != typeid(neolib::comma_and_brackets_as_whitespace))
            aStream.imbue(std::locale{ previousImbued, new neolib::comma_and_brackets_as_whitespace{} });
        text_effect_type type;
        aStream >> type;
        aEffect.set_type(type);
        text_color color;
        aStream >> color;
        aEffect.set_color(color);
        dimension width;
        aStream >> width;
        aEffect.set_width(width);
        vec3 offset;
        aStream >> offset;
        aEffect.set_offset(offset);
        double aux1;
        aStream >> aux1;
        aEffect.set_aux1(aux1);
        bool ignoreEmoji;
        aStream >> ignoreEmoji;
        aEffect.set_ignore_emoji(ignoreEmoji);
        aStream.imbue(previousImbued);
        return aStream;
    }

    class text_format
    {
    public:
        struct no_paper : std::logic_error { no_paper() : std::logic_error("neogfx::text_format::no_paper") {} };
        struct no_effect : std::logic_error { no_effect() : std::logic_error("neogfx::text_format::no_effect") {} };
    public:
        typedef text_format abstract_type; // todo
    public:
        text_format() :
            iSmartUnderline{ false },
            iIgnoreEmoji{ true },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ false }
        {
        }
        text_format(text_format const& aOther) :
            iInk{ aOther.iInk },
            iPaper{ aOther.iPaper },
            iSmartUnderline{ aOther.iSmartUnderline },
            iIgnoreEmoji{ aOther.iIgnoreEmoji },
            iEffect{ aOther.iEffect },
            iOnlyCalculateEffect{ aOther.iOnlyCalculateEffect },
            iBeingFiltered{ aOther.iBeingFiltered }
        {
        }
        template <typename InkType, typename PaperType>
        text_format(InkType const& aInk, PaperType const& aPaper, optional_text_effect const& aEffect) :
            iInk{ aInk },
            iPaper{ aPaper },
            iSmartUnderline{ false },
            iIgnoreEmoji{ true },
            iEffect{ aEffect },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ false }
        {
        }
        template <typename InkType, typename PaperType>
        text_format(InkType const& aInk, PaperType const& aPaper, text_effect const& aEffect) :
            iInk{ aInk },
            iPaper{ aPaper },
            iSmartUnderline{ false },
            iIgnoreEmoji{ true },
            iEffect{ aEffect },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ false }
        {
        }
        template <typename InkType>
        text_format(InkType const& aInk, optional_text_effect const& aEffect) :
            iInk{ aInk },
            iSmartUnderline{ false },
            iIgnoreEmoji{ true },
            iEffect{ aEffect },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ false }
        {
        }
        template <typename InkType>
        text_format(InkType const& aInk, text_effect const& aEffect) :
            iInk{ aInk },
            iSmartUnderline{ false },
            iIgnoreEmoji{ true },
            iEffect{ aEffect },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ false }
        {
        }
        template <typename InkType, typename PaperType>
        text_format(InkType const& aInk, PaperType const& aPaper) :
            iInk{ aInk },
            iPaper{ aPaper },
            iSmartUnderline{ false },
            iIgnoreEmoji{ true },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ false }
        {
        }
        template <typename InkType>
        text_format(InkType const& aInk) :
            iInk{ aInk },
            iSmartUnderline{ false },
            iIgnoreEmoji{ true },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ false }
        {
        }
    public:
        bool operator==(text_format const& aRhs) const noexcept
        {
            return std::forward_as_tuple(ink(), paper(), smart_underline(), ignore_emoji(), effect()) ==
                std::forward_as_tuple(aRhs.ink(), aRhs.paper(), aRhs.smart_underline(), aRhs.ignore_emoji(), aRhs.effect());
        }
        auto operator<=>(text_format const& aRhs) const noexcept
        {
            return std::forward_as_tuple(ink(), paper(), smart_underline(), ignore_emoji(), effect()) <=>
                std::forward_as_tuple(aRhs.ink(), aRhs.paper(), aRhs.smart_underline(), aRhs.ignore_emoji(), aRhs.effect());
        }
    public:
        text_color const& ink() const
        {
            return iInk;
        }
        void set_ink(text_color const& aInk)
        {
            iInk = aInk;
        }
        optional_text_color const& paper() const
        {
            return iPaper;
        }
        void set_paper(optional_text_color const& aPaper)
        {
            iPaper = aPaper;
        }
        bool smart_underline() const
        {
            return iSmartUnderline;
        }
        void set_smart_underline(bool aSmartUnderline)
        {
            iSmartUnderline = aSmartUnderline;
        }
        bool ignore_emoji() const
        {
            return iIgnoreEmoji;
        }
        void set_ignore_emoji(bool aIgnore)
        {
            iIgnoreEmoji = aIgnore;
        }
        optional_text_effect const& effect() const
        {
            return iEffect;
        }
        optional_text_effect& effect()
        {
            return iEffect;
        }
        void set_effect(optional_text_effect const& aEffect)
        {
            iEffect = aEffect;
        }
        bool only_calculate_effect() const
        {
            return iOnlyCalculateEffect;
        }
        bool being_filtered() const
        {
            return iBeingFiltered;
        }
    public:
        text_format with_ink(text_color const& aInk) const
        {
            return text_format{ aInk, iPaper, iEffect }.with_smart_underline(smart_underline()).with_emoji_ignored(ignore_emoji());
        }
        text_format with_paper(optional_text_color const& aPaper) const
        {
            return text_format{ iInk, aPaper, iEffect }.with_smart_underline(smart_underline()).with_emoji_ignored(ignore_emoji());
        }
        text_format with_smart_underline(bool aSmartUnderline) const
        {
            auto result = *this;
            result.set_smart_underline(aSmartUnderline);
            return result;
        }
        text_format with_emoji_ignored(bool aIgnored) const
        {
            auto result = *this;
            result.set_ignore_emoji(aIgnored);
            return result;
        }
        text_format with_effect(optional_text_effect const& aEffect) const
        {
            return text_format{ iInk, iPaper, aEffect }.with_smart_underline(smart_underline()).with_emoji_ignored(ignore_emoji());
        }
        text_format with_alpha(color::component aAlpha) const
        {
            return text_format{
                iInk.with_alpha(aAlpha),
                iPaper != std::nullopt ?
                    optional_text_color{ iPaper->with_alpha(aAlpha) } :
                    optional_text_color{},
                iEffect != std::nullopt ?
                    iEffect->with_alpha(aAlpha) :
                    optional_text_effect{} }.with_smart_underline(smart_underline()).with_emoji_ignored(ignore_emoji());
        }
        text_format with_alpha(double aAlpha) const
        {
            return with_alpha(static_cast<color::component>(aAlpha * 255));
        }
        text_format with_only_effect_calculation() const
        {
            auto copy = *this;
            copy.iOnlyCalculateEffect = true;
            return copy;
        }
        text_format as_being_filtered() const
        {
            auto copy = *this;
            copy.iBeingFiltered = true;
            return copy;
        }
        font apply(font const& aFont) const
        {
            if (iEffect && iEffect.value().type() == text_effect_type::Outline && aFont.info().outline().radius == 0.0)
                return aFont.with_outline(stroke{ iEffect.value().width() });
            else
                return aFont;
        }
    private:
        text_color iInk;
        optional_text_color iPaper;
        bool iSmartUnderline;
        bool iIgnoreEmoji;
        optional_text_effect iEffect;
        bool iOnlyCalculateEffect;
        bool iBeingFiltered;
    };

    typedef neolib::optional<text_format> optional_text_format;

    template <typename Elem, typename Traits>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const text_format& aTextFormat)
    {
        aStream << "[";
        aStream << aTextFormat.ink();
        aStream << ",";
        aStream << aTextFormat.paper();
        aStream << ",";
        aStream << aTextFormat.effect();
        aStream << ",";
        aStream << aTextFormat.smart_underline();
        aStream << ",";
        aStream << aTextFormat.ignore_emoji();
        aStream << "]";
        return aStream;
    }

    template <typename Elem, typename Traits>
    inline std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& aStream, text_format& aTextFormat)
    {
        auto previousImbued = aStream.getloc();
        if (typeid(std::use_facet<std::ctype<char>>(previousImbued)) != typeid(neolib::comma_and_brackets_as_whitespace))
            aStream.imbue(std::locale{ previousImbued, new neolib::comma_and_brackets_as_whitespace{} });
        text_color ink;
        aStream >> ink;
        aTextFormat.set_ink(ink);
        optional_text_color temp;
        aStream >> temp;
        aTextFormat.set_paper(temp);
        optional_text_effect textEffect;
        aStream >> textEffect;
        aTextFormat.set_effect(textEffect);
        bool smartUnderline;
        aStream >> smartUnderline;
        aTextFormat.set_smart_underline(smartUnderline);
        bool ignoreEmoji;
        aStream >> ignoreEmoji;
        aTextFormat.set_ignore_emoji(ignoreEmoji);
        aStream.imbue(previousImbued);
        return aStream;
    }
}

define_setting_type(neogfx::text_format)

namespace neogfx
{
    struct text_format_span
    {
        std::ptrdiff_t start;
        std::ptrdiff_t end;
        text_format attributes;
    };

    class text_format_spans
    {
    public:
        typedef neolib::vecarray<text_format_span, 1, -1> spans;
    public:
        text_format_spans()
        {
        }
        text_format_spans(text_format_span const& aSpan) :
            iSpans{ aSpan }
        {
        }
    public:
        void clear()
        {
            iSpans.clear();
        }
        void add(std::ptrdiff_t aGlyphTextIndex, text_format const& aTextFormat)
        {
            if (iSpans.empty() || iSpans.back().attributes != aTextFormat)
                iSpans.emplace_back(aGlyphTextIndex, aGlyphTextIndex + 1, aTextFormat);
            else
                iSpans.back().end = aGlyphTextIndex + 1;
        }
        template <typename... Args>
        void add(std::ptrdiff_t aGlyphtTextIndex, Args&&... aArgs)
        {
            text_format const attributes{ std::forward<Args>(aArgs)... };
            add(aGlyphtTextIndex, attributes);
        }
    public:
        spans::const_iterator begin() const
        {
            return iSpans.begin();
        }
        spans::const_iterator end() const
        {
            return iSpans.end();
        }
    private:
        spans iSpans;
    };

    class paragraph_format
    {
    public:
        using self_type = paragraph_format;
        using abstract_type = self_type; ///< @todo
    public:
        paragraph_format() {}
        paragraph_format(paragraph_format const& aOther) {}
    public:
        auto operator<=>(self_type const&) const = default;
    };
}