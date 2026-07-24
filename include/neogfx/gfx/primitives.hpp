// primitives.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015,2026 Leigh Johnston.  All Rights Reserved.

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
#include <optional>

#include <neolib/core/vecarray.hpp>

#include <neogfx/core/primitives.hpp>
#include <neogfx/core/easing.hpp>
#include <neogfx/gfx/stipple.hpp>
#include <neogfx/gfx/path.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/gfx/sub_texture.hpp>
#include <neogfx/gfx/pen.hpp>

namespace neogfx
{
    enum class front_face : std::uint32_t
    {
        CW,
        CCW
    };

    enum class face_culling : std::uint32_t
    {
        None,
        Front,
        Back,
        FrontAndBack
    };

    enum class blending_mode : std::uint32_t
    {
        Unspecified,
        None,
        Default,
        Sprite,
        Blit,
        Lighten,
        Filter,
        FilterFinish
    };

    enum class smoothing_mode : std::uint32_t
    {
        None,
        AntiAlias
    };

    enum class logical_operation : std::uint32_t
    {
        None,
        Xor
    };

    enum class shader_effect : std::uint32_t
    {
        None = 0,
        Colorize = 1,
        ColorizeAverage = Colorize,
        ColorizeMaximum = 2,
        ColorizeSpot = 3,
        ColorizeAlpha = 4,
        Monochrome = 5,
        Blit = 10,
        MultiplyAlpha = 20,
        Filter = 30,
        Ignore = 99
    };

    enum class shader_filter : std::uint32_t
    {
        None = 0,
        GaussianBlur = 1
    };

    enum class shader_shape : std::uint32_t
    {
        None = 0x00,
        Line = 0x01,
        CubicBezier = 0x02,
        Triangle = 0x03,
        Rect = 0x04,
        Circle = 0x05,
        Ellipse = 0x06,
        Pie = 0x07,
        Arc = 0x08,
        RoundedRect = 0x09,
        EllipseRect = 0x0A,
        Polygon = 0x0B,
        Checkerboard = 0x0C
    };

    enum class blurring_algorithm : std::uint32_t
    {
        None,
        Gaussian
    };

    typedef neolib::variant<color, gradient, texture, neolib::pair<texture, rect>, sub_texture, neolib::pair<sub_texture, rect>> brush;

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
        text_color with_alpha(scalar aAlpha) const
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
        Shadow,
        OutlineGlow,
        OutlineShadow
    };

    inline bool filtered_content_ink_is_effect_color(text_effect_type aTextEffectType)
    {
        switch (aTextEffectType)
        {
        case text_effect_type::Glow:
        case text_effect_type::Shadow:
            return true;
        default:
            return false;
        }
    }

    enum class text_effect_flags : std::uint64_t
    {
        Default     = 0x0000000000000000,
        IgnoreEmoji = 0x0000000000000001,
        Bright      = 0x0000000000010000
    };

    inline constexpr text_effect_flags operator|(text_effect_flags aLhs, text_effect_flags aRhs)
    {
        return static_cast<text_effect_flags>(static_cast<std::uint64_t>(aLhs) | static_cast<std::uint64_t>(aRhs));
    }

    inline constexpr text_effect_flags operator&(text_effect_flags aLhs, text_effect_flags aRhs)
    {
        return static_cast<text_effect_flags>(static_cast<std::uint64_t>(aLhs) & static_cast<std::uint64_t>(aRhs));
    }

    inline constexpr text_effect_flags operator~(text_effect_flags aLhs)
    {
        return static_cast<text_effect_flags>(~static_cast<std::uint64_t>(aLhs));
    }

    enum class text_animation_type : std::uint32_t
    {
        None,
        Flash,
        Blink
    };
}

begin_declare_enum(neogfx::text_effect_type)
declare_enum_string(neogfx::text_effect_type, None)
declare_enum_string(neogfx::text_effect_type, Outline)
declare_enum_string(neogfx::text_effect_type, Glow)
declare_enum_string(neogfx::text_effect_type, Shadow)
declare_enum_string(neogfx::text_effect_type, OutlineGlow)
declare_enum_string(neogfx::text_effect_type, OutlineShadow)
end_declare_enum(neogfx::text_effect_type)

begin_declare_enum(neogfx::text_effect_flags)
declare_enum_string(neogfx::text_effect_flags, Default)
declare_enum_string(neogfx::text_effect_flags, IgnoreEmoji)
declare_enum_string(neogfx::text_effect_flags, Bright)
end_declare_enum(neogfx::text_effect_flags)

begin_declare_enum(neogfx::text_animation_type)
declare_enum_string(neogfx::text_animation_type, None)
declare_enum_string(neogfx::text_animation_type, Flash)
declare_enum_string(neogfx::text_animation_type, Blink)
end_declare_enum(neogfx::text_animation_type)

namespace neogfx
{
    class text_effect
    {
    public:
        using abstract_type = text_effect; // todo
    public:
        using auxiliary_parameter = double;
        using optional_auxiliary_parameter = optional<auxiliary_parameter>;
    public:
        text_effect() :
            iType{ text_effect_type::None }, iFlags{ text_effect_flags::Default }, iColor{}, iWidth{}, iOffset{}, iAux1{}, iAux2{}
        {
        }
        text_effect(text_effect_type aType, const text_color& aColor, const optional_dimension& aWidth = {}, const optional_vec3& aOffset = {}, const optional_auxiliary_parameter& aAux1 = {}, const optional_auxiliary_parameter& aAux2 = {}, text_effect_flags aFlags = text_effect_flags::Default) :
            iType{ aType }, iFlags{ aFlags }, iColor{ aColor }, iWidth{ aWidth }, iOffset{ aOffset }, iAux1{ aAux1 }, iAux2{ aAux2 }
        {
        }
    public:
        text_effect& operator=(const text_effect& aOther)
        {
            if (&aOther == this)
                return *this;
            iType = aOther.iType;
            iFlags = aOther.iFlags;
            iColor = aOther.iColor;
            iWidth = aOther.iWidth;
            iOffset = aOther.iOffset;
            iAux1 = aOther.iAux1;
            iAux2 = aOther.iAux2;
            return *this;
        }
    public:
        bool operator==(const text_effect& that) const noexcept
        {
            return std::forward_as_tuple(iType, iFlags, iColor, iWidth, iOffset, iAux1, iAux2) ==
                std::forward_as_tuple(that.iType, that.iFlags, that.iColor, that.iWidth, that.iOffset, that.iAux1, that.iAux2);
        }
        auto operator<=>(const text_effect& that) const noexcept
        {
            return std::forward_as_tuple(iType, iFlags, iColor, iWidth, iOffset, iAux1, iAux2) <=>
                std::forward_as_tuple(that.iType, that.iFlags, that.iColor, that.iWidth, that.iOffset, that.iAux1, that.iAux2);
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
        text_effect_flags flags() const
        {
            return iFlags;
        }
        void set_flags(text_effect_flags aFlags)
        {
            iFlags = aFlags;
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
                return 5.0;
            }
        }
        void set_aux1(double aAux1)
        {
            iAux1 = aAux1;
        }
        double aux2() const
        {
            if (iAux2 != std::nullopt)
                return *iAux2;
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
        void set_aux2(double aAux2)
        {
            iAux2 = aAux2;
        }
        bool bright() const
        {
            return (flags() & text_effect_flags::Bright) == text_effect_flags::Bright;
        }
        void set_bright(bool aBright)
        {
            if (aBright)
                set_flags(flags() | text_effect_flags::Bright);
            else
                set_flags(flags() & ~text_effect_flags::Bright);
        }
        bool ignore_emoji() const
        {
            return (flags() & text_effect_flags::IgnoreEmoji) == text_effect_flags::IgnoreEmoji;
        }
        void set_ignore_emoji(bool aIgnore)
        {
            if (aIgnore)
                set_flags(flags() | text_effect_flags::IgnoreEmoji);
            else
                set_flags(flags() & ~text_effect_flags::IgnoreEmoji);
        }
        text_effect with_alpha(color::component aAlpha) const
        {
            return text_effect{ iType, iColor.with_alpha(aAlpha), iWidth, iOffset, iAux1, iAux2, iFlags };
        }
        text_effect with_alpha(double aAlpha) const
        {
            return with_alpha(static_cast<color::component>(aAlpha * 255));
        }
    private:
        text_effect_type iType;
        text_effect_flags iFlags;
        text_color iColor;
        optional_dimension iWidth;
        optional_vec3 iOffset;
        optional_auxiliary_parameter iAux1;
        optional_auxiliary_parameter iAux2;
    };

    typedef neolib::optional<text_effect> optional_text_effect;

    inline bool filtered_content_ink_is_effect_color(text_effect const* aBeingFiltered)
    {
        return aBeingFiltered && filtered_content_ink_is_effect_color(aBeingFiltered->type());
    }

    template <typename Elem, typename Traits>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const text_effect& aEffect)
    {
        aStream << '[';
        aStream << aEffect.type();
        aStream << ',';
        aStream << aEffect.flags();
        aStream << ',';
        aStream << aEffect.color();
        aStream << ',';
        aStream << aEffect.width();
        aStream << ',';
        aStream << aEffect.offset();
        aStream << ',';
        aStream << aEffect.aux1();
        aStream << ',';
        aStream << aEffect.aux2();
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
        text_effect_flags flags;
        aStream >> flags;
        aEffect.set_flags(flags);
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
        double aux2;
        aStream >> aux2;
        aEffect.set_aux2(aux2);
        aStream.imbue(previousImbued);
        return aStream;
    }

    class text_animation
    {
    public:
        typedef text_animation abstract_type; // todo
    public:
        static constexpr scalar kDefaultFrequency = 1.5625; ///< Sinclair ZX Spectrum FLASH frequency
    public:
        text_animation() :
            iType{ text_animation_type::None }, iEasing{ neogfx::easing::InStep }, iFrequency{ kDefaultFrequency }
        {
        }
        text_animation(text_animation_type aType, neogfx::easing aEasing = neogfx::easing::InStep, scalar aFrequency = kDefaultFrequency) :
            iType{ aType }, iEasing{ aEasing }, iFrequency{ aFrequency }
        {
        }
    public:
        text_animation& operator=(const text_animation& aOther)
        {
            if (&aOther == this)
                return *this;
            iType = aOther.iType;
            iEasing = aOther.iEasing;
            iFrequency = aOther.iFrequency;
            return *this;
        }
    public:
        bool operator==(const text_animation& that) const noexcept
        {
            return std::forward_as_tuple(iType, iEasing, iFrequency) ==
                std::forward_as_tuple(that.iType, that.iEasing, that.iFrequency);
        }
        auto operator<=>(const text_animation& that) const noexcept
        {
            return std::forward_as_tuple(iType, iEasing, iFrequency) <=>
                std::forward_as_tuple(that.iType, that.iEasing, that.iFrequency);
        }
    public:
        text_animation_type type() const
        {
            return iType;
        }
        void set_type(text_animation_type aType)
        {
            iType = aType;
        }
        neogfx::easing easing() const
        {
            return iEasing;
        }
        void set_easing(neogfx::easing aEasing)
        {
            iEasing = aEasing;
        }
        scalar frequency() const
        {
            return iFrequency;
        }
        void set_frequency(scalar aFrequency)
        {
            iFrequency = aFrequency;
        }
    private:
        text_animation_type iType;
        neogfx::easing iEasing;
        scalar iFrequency;
    };

    typedef neolib::optional<text_animation> optional_text_animation;

    template <typename Elem, typename Traits>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const text_animation& aEffect)
    {
        aStream << '[';
        aStream << aEffect.type();
        aStream << ',';
        aStream << aEffect.easing();
        aStream << ',';
        aStream << aEffect.frequency();
        aStream << ']';
        return aStream;
    }

    template <typename Elem, typename Traits>
    inline std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& aStream, text_animation& aEffect)
    {
        auto previousImbued = aStream.getloc();
        if (typeid(std::use_facet<std::ctype<char>>(previousImbued)) != typeid(neolib::comma_and_brackets_as_whitespace))
            aStream.imbue(std::locale{ previousImbued, new neolib::comma_and_brackets_as_whitespace{} });
        text_animation_type type;
        aStream >> type;
        aEffect.set_type(type);
        easing easing;
        aStream >> easing;
        aEffect.set_easing(easing);
        scalar frequency;
        aStream >> frequency;
        aEffect.set_frequency(frequency);
        aStream.imbue(previousImbued);
        return aStream;
    }

    class text_format
    {
    public:
        struct no_paper : std::logic_error { no_paper() : std::logic_error("neogfx::text_format::no_paper") {} };
        struct no_effect : std::logic_error { no_effect() : std::logic_error("neogfx::text_format::no_effect") {} };
        struct no_animation : std::logic_error { no_animation() : std::logic_error("neogfx::text_format::no_animation") {} };
    public:
        typedef text_format abstract_type; // todo
    private:
        enum class filtered_effect : std::uint8_t
        {
            None,
            Effect1,
            Effect2
        };
    public:
        text_format() :
            iFlags{ text_effect_flags::IgnoreEmoji },
            iSmartUnderline{ false },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ filtered_effect::None }
        {
        }
        text_format(text_format const& aOther) :
            iInk{ aOther.iInk },
            iPaper{ aOther.iPaper },
            iFlags{ aOther.iFlags },
            iSmartUnderline{ aOther.iSmartUnderline },
            iEffect{ aOther.iEffect },
            iEffect2{ aOther.iEffect2 },
            iAnimation{ aOther.iAnimation },
            iOnlyCalculateEffect{ aOther.iOnlyCalculateEffect },
            iBeingFiltered{ aOther.iBeingFiltered }
        {
        }
        template <typename InkType, typename PaperType>
        text_format(InkType const& aInk, PaperType const& aPaper, optional_text_effect const& aEffect, optional_text_animation const& aAnimation = {}) :
            iInk{ aInk },
            iPaper{ aPaper },
            iFlags{ text_effect_flags::IgnoreEmoji },
            iSmartUnderline{ false },
            iEffect{ aEffect },
            iAnimation{ aAnimation },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ filtered_effect::None }
        {
        }
        template <typename InkType, typename PaperType>
        text_format(InkType const& aInk, PaperType const& aPaper, optional_text_effect const& aEffect, optional_text_effect const& aEffect2, optional_text_animation const& aAnimation = {}) :
            iInk{ aInk },
            iPaper{ aPaper },
            iFlags{ text_effect_flags::IgnoreEmoji },
            iSmartUnderline{ false },
            iEffect{ aEffect },
            iEffect2{ aEffect2 },
            iAnimation{ aAnimation },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ filtered_effect::None }
        {
        }
        template <typename InkType, typename PaperType>
        text_format(InkType const& aInk, PaperType const& aPaper, text_effect const& aEffect) :
            iInk{ aInk },
            iPaper{ aPaper },
            iFlags{ text_effect_flags::IgnoreEmoji },
            iSmartUnderline{ false },
            iEffect{ aEffect },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ filtered_effect::None }
        {
        }
        template <typename InkType, typename PaperType>
        text_format(InkType const& aInk, PaperType const& aPaper, text_effect const& aEffect, text_effect const& aEffect2) :
            iInk{ aInk },
            iPaper{ aPaper },
            iFlags{ text_effect_flags::IgnoreEmoji },
            iSmartUnderline{ false },
            iEffect{ aEffect },
            iEffect2{ aEffect2 },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ filtered_effect::None }
        {
        }
        template <typename InkType>
        text_format(InkType const& aInk, optional_text_effect const& aEffect) :
            iInk{ aInk },
            iFlags{ text_effect_flags::IgnoreEmoji },
            iSmartUnderline{ false },
            iEffect{ aEffect },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ filtered_effect::None }
        {
        }
        template <typename InkType>
        text_format(InkType const& aInk, optional_text_effect const& aEffect, optional_text_effect const& aEffect2) :
            iInk{ aInk },
            iFlags{ text_effect_flags::IgnoreEmoji },
            iSmartUnderline{ false },
            iEffect{ aEffect },
            iEffect2{ aEffect2 },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ filtered_effect::None }
        {
        }
        template <typename InkType>
        text_format(InkType const& aInk, text_effect const& aEffect) :
            iInk{ aInk },
            iFlags{ text_effect_flags::IgnoreEmoji },
            iSmartUnderline{ false },
            iEffect{ aEffect },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ filtered_effect::None }
        {
        }
        template <typename InkType>
        text_format(InkType const& aInk, text_effect const& aEffect, text_effect const& aEffect2) :
            iInk{ aInk },
            iFlags{ text_effect_flags::IgnoreEmoji },
            iSmartUnderline{ false },
            iEffect{ aEffect },
            iEffect2{ aEffect2 },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ filtered_effect::None }
        {
        }
        template <typename InkType, typename PaperType>
        text_format(InkType const& aInk, PaperType const& aPaper, optional_text_animation const& aAnimation) :
            iInk{ aInk },
            iPaper{ aPaper },
            iFlags{ text_effect_flags::IgnoreEmoji },
            iSmartUnderline{ false },
            iAnimation{ aAnimation },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ filtered_effect::None }
        {
        }
        template <typename InkType>
        text_format(InkType const& aInk, optional_text_animation const& aAnimation) :
            iInk{ aInk },
            iFlags{ text_effect_flags::IgnoreEmoji },
            iSmartUnderline{ false },
            iAnimation{ aAnimation },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ filtered_effect::None }
        {
        }
        template <typename InkType>
        text_format(InkType const& aInk, text_animation const& aAnimation) :
            iInk{ aInk },
            iFlags{ text_effect_flags::IgnoreEmoji },
            iSmartUnderline{ false },
            iAnimation{ aAnimation },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ filtered_effect::None }
        {
        }
        template <typename InkType, typename PaperType>
        text_format(InkType const& aInk, PaperType const& aPaper) :
            iInk{ aInk },
            iPaper{ aPaper },
            iFlags{ text_effect_flags::IgnoreEmoji },
            iSmartUnderline{ false },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ filtered_effect::None }
        {
        }
        template <typename InkType>
        text_format(InkType const& aInk) :
            iInk{ aInk },
            iFlags{ text_effect_flags::IgnoreEmoji },
            iSmartUnderline{ false },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ filtered_effect::None }
        {
        }
    public:
        bool operator==(text_format const& aRhs) const noexcept
        {
            return std::forward_as_tuple(ink(), paper(), flags(), smart_underline(), effect(), effect2(), animation()) ==
                std::forward_as_tuple(aRhs.ink(), aRhs.paper(), aRhs.flags(), aRhs.smart_underline(), aRhs.effect(), aRhs.effect2(), aRhs.animation());
        }
        auto operator<=>(text_format const& aRhs) const noexcept
        {
            return std::forward_as_tuple(ink(), paper(), flags(), smart_underline(), effect(), effect2(), animation()) <=>
                std::forward_as_tuple(aRhs.ink(), aRhs.paper(), aRhs.flags(), aRhs.smart_underline(), aRhs.effect(), aRhs.effect2(), aRhs.animation());
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
        text_effect_flags flags() const
        {
            return iFlags;
        }
        void set_flags(text_effect_flags aFlags)
        {
            iFlags = aFlags;
        }
        bool bright() const
        {
            return (flags() & text_effect_flags::Bright) == text_effect_flags::Bright;
        }
        void set_bright(bool aBright)
        {
            if (aBright)
                set_flags(flags() | text_effect_flags::Bright);
            else
                set_flags(flags() & ~text_effect_flags::Bright);
        }
        bool ignore_emoji() const
        {
            return (flags() & text_effect_flags::IgnoreEmoji) == text_effect_flags::IgnoreEmoji;
        }
        void set_ignore_emoji(bool aIgnore)
        {
            if (aIgnore)
                set_flags(flags() | text_effect_flags::IgnoreEmoji);
            else
                set_flags(flags() & ~text_effect_flags::IgnoreEmoji);
        }
        bool smart_underline() const
        {
            return iSmartUnderline;
        }
        void set_smart_underline(bool aSmartUnderline)
        {
            iSmartUnderline = aSmartUnderline;
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
        optional_text_effect const& effect2() const
        {
            return iEffect2;
        }
        optional_text_effect& effect2()
        {
            return iEffect2;
        }
        void set_effect2(optional_text_effect const& aEffect2)
        {
            iEffect2 = aEffect2;
        }
        optional_text_animation const& animation() const
        {
            return iAnimation;
        }
        optional_text_animation& animation()
        {
            return iAnimation;
        }
        void set_animation(optional_text_animation const& aAnimation)
        {
            iAnimation = aAnimation;
        }
        bool only_calculate_effect() const
        {
            return iOnlyCalculateEffect;
        }
        text_effect const* being_filtered() const
        {
            switch (iBeingFiltered)
            {
            case filtered_effect::Effect1:
                return iEffect ? &*iEffect : nullptr;
            case filtered_effect::Effect2:
                return iEffect2 ? &*iEffect2 : nullptr;
            default:
                return nullptr;
            }
        }
    public:
        text_format with_ink(text_color const& aInk) const
        {
            return text_format{ aInk, iPaper, iEffect, iEffect2 }.with_flags(flags()).with_smart_underline(smart_underline());
        }
        text_format with_paper(optional_text_color const& aPaper) const
        {
            return text_format{ iInk, aPaper, iEffect, iEffect2 }.with_flags(flags()).with_smart_underline(smart_underline());
        }
        text_format with_flags(text_effect_flags aFlags) const
        {
            auto result = *this;
            result.set_flags(aFlags);
            return result;
        }
        text_format with_smart_underline(bool aSmartUnderline) const
        {
            auto result = *this;
            result.set_smart_underline(aSmartUnderline);
            return result;
        }
        text_format with_bright(bool aBright) const
        {
            auto result = *this;
            result.set_bright(aBright);
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
            return text_format{ iInk, iPaper, aEffect, iEffect2, iAnimation }.with_flags(flags()).with_smart_underline(smart_underline());
        }
        text_format with_effect2(optional_text_effect const& aEffect2) const
        {
            return text_format{ iInk, iPaper, iEffect, aEffect2, iAnimation }.with_flags(flags()).with_smart_underline(smart_underline());
        }
        text_format with_animation(optional_text_animation const& aAnimation) const
        {
            return text_format{ iInk, iPaper, iEffect, iEffect2, aAnimation }.with_flags(flags()).with_smart_underline(smart_underline());
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
                    optional_text_effect{},
                iEffect2 != std::nullopt ?
                    iEffect2->with_alpha(aAlpha) :
                    optional_text_effect{},
            }.with_flags(flags()).with_smart_underline(smart_underline());
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
        text_format as_being_filtered(text_effect const& aTextEffect) const
        {
            auto copy = *this;
            if (iEffect && &aTextEffect == &*iEffect)
                copy.iBeingFiltered = filtered_effect::Effect1;
            else if (iEffect2 && &aTextEffect == &*iEffect2)
                copy.iBeingFiltered = filtered_effect::Effect2;
            else
                throw no_effect();
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
        text_effect_flags iFlags;
        bool iSmartUnderline;
        optional_text_effect iEffect;
        optional_text_effect iEffect2;
        optional_text_animation iAnimation;
        bool iOnlyCalculateEffect;
        filtered_effect iBeingFiltered;
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
        aStream << aTextFormat.flags();
        aStream << ",";
        aStream << aTextFormat.smart_underline();
        aStream << ",";
        aStream << aTextFormat.effect();
        aStream << ",";
        aStream << aTextFormat.effect2();
        aStream << ",";
        aStream << aTextFormat.animation();
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
        optional_text_color paper;
        aStream >> paper;
        aTextFormat.set_paper(paper);
        text_effect_flags flags;
        aStream >> flags;
        aTextFormat.set_flags(flags);
        bool smartUnderline;
        aStream >> smartUnderline;
        aTextFormat.set_smart_underline(smartUnderline);
        optional_text_effect textEffect;
        aStream >> textEffect;
        aTextFormat.set_effect(textEffect);
        optional_text_effect textEffect2;
        aStream >> textEffect2;
        aTextFormat.set_effect2(textEffect2);
        optional_text_animation textAnimation;
        aStream >> textAnimation;
        aTextFormat.set_animation(textAnimation);
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
        using abstract_type = paragraph_format; ///< @todo
    public:
        paragraph_format() {}
        paragraph_format(paragraph_format const& aOther) {}
    public:
        auto operator<=>(paragraph_format const&) const = default;
    };
}