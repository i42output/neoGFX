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
        Monochrome         = 4,
        Filter             = 10,
        Ignore             = 99
    };

    enum class shader_filter
    {
        None            = 0,
        GaussianBlur    = 1
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
    typedef std::optional<text_color> optional_text_color;

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
        typedef double auxiliary_parameter;
        typedef std::optional<auxiliary_parameter> optional_auxiliary_parameter;
    public:
        text_effect(text_effect_type aType, const text_color& aColor, const optional_dimension& aWidth = {}, const optional_vec3& aOffset = {}, const optional_auxiliary_parameter& aAux1 = {}) :
            iType{ aType }, iColor{ aColor }, iWidth{ aWidth }, iAux1{ aAux1 }
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
            return *this;
        }
    public:
        bool operator==(const text_effect& aOther) const
        {
            return iType == aOther.iType && iColor == aOther.iColor && iWidth == aOther.iWidth && iOffset == aOther.iOffset && iAux1 == aOther.iAux1;
        }
        bool operator!=(const text_effect& aOther) const
        {
            return !(*this == aOther);
        }
        bool operator<(const text_effect& aRhs) const
        {
            return std::tie(iType, iColor, iWidth, iOffset, iAux1) < std::tie(aRhs.iType, aRhs.iColor, aRhs.iWidth, aRhs.iOffset, aRhs.iAux1);
        }
    public:
        text_effect_type type() const
        {
            return iType;
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
                return {};
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
        text_effect with_alpha(color::component aAlpha) const
        {
            return text_effect{ iType, iColor.with_alpha(aAlpha), iWidth, iOffset, iAux1 };
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
    };
    typedef std::optional<text_effect> optional_text_effect;

    class text_appearance
    {
    public:
        struct no_paper : std::logic_error { no_paper() : std::logic_error("neogfx::text_appearance::no_paper") {} };
        struct no_effect : std::logic_error { no_effect() : std::logic_error("neogfx::text_appearance::no_effect") {} };
    public:
        template <typename InkType, typename PaperType>
        text_appearance(const InkType& aInk, const PaperType& aPaper, const optional_text_effect& aEffect) :
            iInk{ aInk },
            iPaper{ aPaper },
            iEffect{ aEffect },
            iOnlyCalculateEffect{ false }
        {
        }
        template <typename InkType, typename PaperType>
        text_appearance(const InkType& aInk, const PaperType& aPaper, const text_effect& aEffect) :
            iInk{ aInk },
            iPaper{ aPaper },
            iEffect{ aEffect },
            iOnlyCalculateEffect{ false }
        {
        }
        template <typename InkType>
        text_appearance(const InkType& aInk, const optional_text_effect& aEffect) :
            iInk{ aInk },
            iEffect{ aEffect },
            iOnlyCalculateEffect{ false }
        {
        }
        template <typename InkType>
        text_appearance(const InkType& aInk, const text_effect& aEffect) :
            iInk{ aInk },
            iEffect{ aEffect },
            iOnlyCalculateEffect{ false }
        {
        }
        template <typename InkType, typename PaperType>
        text_appearance(const InkType& aInk, const PaperType& aPaper) :
            iInk{ aInk },
            iPaper{ aPaper },
            iOnlyCalculateEffect{ false }
        {
        }
        template <typename InkType>
        text_appearance(const InkType& aInk) :
            iInk{ aInk },
            iOnlyCalculateEffect{ false }
        {
        }
    public:
        bool operator==(const text_appearance& aRhs) const
        {
            return iInk == aRhs.iInk && iPaper == aRhs.iPaper && iEffect == aRhs.iEffect;
        }
        bool operator!=(const text_appearance& aRhs) const
        {
            return !(*this == aRhs);
        }
    public:
        const text_color& ink() const
        {
            return iInk;
        }
        const optional_text_color& paper() const
        {
            return iPaper;
        }
        const optional_text_effect& effect() const
        {
            return iEffect;
        }
        bool only_calculate_effect() const
        {
            return iOnlyCalculateEffect;
        }
    public:
        text_appearance with_ink(const text_color& aInk) const
        {
            return text_appearance{ aInk, iPaper, iEffect };
        }
        text_appearance with_paper(const optional_text_color& aPaper) const
        {
            return text_appearance{ iInk, aPaper, iEffect };
        }
        text_appearance with_effect(const optional_text_effect& aEffect) const
        {
            return text_appearance{ iInk, iPaper, aEffect };
        }
        text_appearance with_alpha(color::component aAlpha) const
        {
            return text_appearance{ iInk.with_alpha(aAlpha), iPaper != std::nullopt ? optional_text_color{ iPaper->with_alpha(aAlpha) } : optional_text_color{}, iEffect != std::nullopt ? iEffect->with_alpha(aAlpha) : optional_text_effect{} };
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
    private:
        text_color iInk;
        optional_text_color iPaper;
        optional_text_effect iEffect;
        bool iOnlyCalculateEffect;
    };

    typedef std::optional<text_appearance> optional_text_appearance;
}