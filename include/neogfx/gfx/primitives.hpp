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
#include <boost/multi_array.hpp>
#ifdef _WIN32
#pragma warning( pop )
#endif
#include <optional>
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
        None               = 0,
        Colorize           = 1,
        ColorizeAverage    = Colorize,
        ColorizeMaximum    = 2,
        ColorizeSpot       = 3,
        ColorizeAlpha      = 4,
        Monochrome         = 5,
        Filter             = 10,
        Ignore             = 99
    };

    enum class shader_filter
    {
        None            = 0,
        GaussianBlur    = 1
    };

    enum class shader_shape
    {
        None            = 0x00,
        Line            = 0x01,
        CubicBezier     = 0x02,
        Circle          = 0x03,
        Pie             = 0x04,
        Arc             = 0x05,
        RoundedRect     = 0x06
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
        text_color() : color_or_gradient{}
        {
        }
        text_color(const text_color& aOther) : color_or_gradient{ aOther }
        {
        }
        text_color(text_color&& aOther) : color_or_gradient{ std::move(aOther) }
        {
        }
        template <typename T>
        text_color(T&& aOther) : color_or_gradient{ std::forward<T>(aOther) }
        {
        }
    public:
        text_color& operator=(const text_color& aOther)
        {
            if (&aOther == this)
                return *this;
            color_or_gradient::operator=(aOther);
            return *this;
        }
        text_color& operator=(text_color&& aOther)
        {
            if (&aOther == this)
                return *this;
            color_or_gradient::operator=(std::move(aOther));
            return *this;
        }
        template <typename T>
        text_color& operator=(T&& aOther)
        {
            color_or_gradient::operator=(std::forward<T>(aOther));
            return *this;
        }
    public:
        using color_or_gradient::operator==;
        using color_or_gradient::operator!=;
        using color_or_gradient::operator<;
        using color_or_gradient::operator<=;
        using color_or_gradient::operator>;
        using color_or_gradient::operator>=;
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
    typedef neolib::optional<text_color> optional_text_color;

    enum class text_effect_type : uint32_t
    {
        None,
        Outline,
        Glow,
        Shadow
    };

    class text_effect
    {
    public:
        typedef text_effect abstract_type; // todo
    public:
        typedef double auxiliary_parameter;
        typedef optional<auxiliary_parameter> optional_auxiliary_parameter;
    public:
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
        bool operator==(const text_effect& aOther) const
        {
            return iType == aOther.iType && iColor == aOther.iColor && iWidth == aOther.iWidth && iOffset == aOther.iOffset && iAux1 == aOther.iAux1 && iIgnoreEmoji == aOther.iIgnoreEmoji;
        }
        bool operator!=(const text_effect& aOther) const
        {
            return !(*this == aOther);
        }
        bool operator<(const text_effect& aRhs) const
        {
            return std::tie(iType, iColor, iWidth, iOffset, iAux1, iIgnoreEmoji) < std::tie(aRhs.iType, aRhs.iColor, aRhs.iWidth, aRhs.iOffset, aRhs.iAux1, aRhs.iIgnoreEmoji);
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

    class text_appearance
    {
    public:
        struct no_paper : std::logic_error { no_paper() : std::logic_error("neogfx::text_appearance::no_paper") {} };
        struct no_effect : std::logic_error { no_effect() : std::logic_error("neogfx::text_appearance::no_effect") {} };
    public:
        typedef text_appearance abstract_type; // todo
    public:
        text_appearance() :
            iIgnoreEmoji{ true },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ false }
        {
        }
        text_appearance(text_appearance const& aOther) :
            iInk{ aOther.iInk },
            iPaper{ aOther.iPaper },
            iIgnoreEmoji{ aOther.iIgnoreEmoji },
            iEffect{ aOther.iEffect },
            iOnlyCalculateEffect{ aOther.iOnlyCalculateEffect },
            iBeingFiltered{ aOther.iBeingFiltered }
        {
        }
        template <typename InkType, typename PaperType>
        text_appearance(InkType const& aInk, PaperType const& aPaper, optional_text_effect const& aEffect) :
            iInk{ aInk },
            iPaper{ aPaper },
            iIgnoreEmoji{ true },
            iEffect{ aEffect },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ false }
        {
        }
        template <typename InkType, typename PaperType>
        text_appearance(InkType const& aInk, PaperType const& aPaper, text_effect const& aEffect) :
            iInk{ aInk },
            iPaper{ aPaper },
            iIgnoreEmoji{ true },
            iEffect{ aEffect },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ false }
        {
        }
        template <typename InkType>
        text_appearance(InkType const& aInk, optional_text_effect const& aEffect) :
            iInk{ aInk },
            iIgnoreEmoji{ true },
            iEffect{ aEffect },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ false }
        {
        }
        template <typename InkType>
        text_appearance(InkType const& aInk, text_effect const& aEffect) :
            iInk{ aInk },
            iIgnoreEmoji{ true },
            iEffect{ aEffect },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ false }
        {
        }
        template <typename InkType, typename PaperType>
        text_appearance(InkType const& aInk, PaperType const& aPaper) :
            iInk{ aInk },
            iPaper{ aPaper },
            iIgnoreEmoji{ true },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ false }
        {
        }
        template <typename InkType>
        text_appearance(InkType const& aInk) :
            iInk{ aInk },
            iIgnoreEmoji{ true },
            iOnlyCalculateEffect{ false },
            iBeingFiltered{ false }
        {
        }
    public:
        bool operator==(text_appearance const& aRhs) const
        {
            return iInk == aRhs.iInk && iPaper == aRhs.iPaper && iIgnoreEmoji == aRhs.iIgnoreEmoji && iEffect == aRhs.iEffect;
        }
        bool operator!=(text_appearance const& aRhs) const
        {
            return !(*this == aRhs);
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
        text_appearance with_ink(text_color const& aInk) const
        {
            return text_appearance{ aInk, iPaper, iEffect }.with_emoji_ignored(ignore_emoji());
        }
        text_appearance with_paper(optional_text_color const& aPaper) const
        {
            return text_appearance{ iInk, aPaper, iEffect }.with_emoji_ignored(ignore_emoji());
        }
        text_appearance with_emoji_ignored(bool aIgnored) const
        {
            auto result = *this;
            result.set_ignore_emoji(aIgnored);
            return result;
        }
        text_appearance with_effect(optional_text_effect const& aEffect) const
        {
            return text_appearance{ iInk, iPaper, aEffect }.with_emoji_ignored(ignore_emoji());
        }
        text_appearance with_alpha(color::component aAlpha) const
        {
            return text_appearance{ 
                iInk.with_alpha(aAlpha), 
                iPaper != std::nullopt ? 
                    optional_text_color{ iPaper->with_alpha(aAlpha) } : 
                    optional_text_color{}, 
                iEffect != std::nullopt ? 
                    iEffect->with_alpha(aAlpha) : 
                    optional_text_effect{} }.with_emoji_ignored(ignore_emoji());
        }
        text_appearance with_alpha(double aAlpha) const
        {
            return with_alpha(static_cast<color::component>(aAlpha * 255));
        }
        text_appearance with_only_effect_calculation() const
        {
            auto copy = *this;
            copy.iOnlyCalculateEffect = true;
            return copy;
        }
        text_appearance as_being_filtered() const
        {
            auto copy = *this;
            copy.iBeingFiltered = true;
            return copy;
        }
    private:
        text_color iInk;
        optional_text_color iPaper;
        bool iIgnoreEmoji;
        optional_text_effect iEffect;
        bool iOnlyCalculateEffect;
        bool iBeingFiltered;
    };

    typedef neolib::optional<text_appearance> optional_text_appearance;
}