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
        None                = 0,
        Colourize           = 1,
        ColourizeAverage    = Colourize,
        Colorize            = Colourize,
        ColorizeAverage     = ColourizeAverage,
        ColourizeMaximum    = 2,
        ColorizeMaximum     = ColourizeMaximum,
        ColourizeSpot       = 3,
        ColorizeSpot        = ColourizeSpot,
        Monochrome          = 4,
        Ignore              = 5
    };

    enum class blurring_algorithm
    {
        None,
        Gaussian
    };

    typedef neolib::variant<colour, gradient, texture, std::pair<texture, rect>, sub_texture, std::pair<sub_texture, rect>> brush;

    inline brush to_brush(const colour_or_gradient& aEffectColour)
    {
        if (std::holds_alternative<colour>(aEffectColour))
            return std::get<colour>(aEffectColour);
        else if (std::holds_alternative<gradient>(aEffectColour))
            return std::get<gradient>(aEffectColour);
        else
            return colour{};
    }

    class text_colour : public colour_or_gradient
    {
    public:
        text_colour() : colour_or_gradient{}
        {
        }
        text_colour(const text_colour& aOther) : colour_or_gradient{ aOther }
        {
        }
        text_colour(text_colour&& aOther) : colour_or_gradient{ std::move(aOther) }
        {
        }
        template <typename T>
        text_colour(T&& aOther) : colour_or_gradient{ std::forward<T>(aOther) }
        {
        }
    public:
        text_colour& operator=(const text_colour& aOther)
        {
            if (&aOther == this)
                return *this;
            colour_or_gradient::operator=(aOther);
            return *this;
        }
        text_colour& operator=(text_colour&& aOther)
        {
            if (&aOther == this)
                return *this;
            colour_or_gradient::operator=(std::move(aOther));
            return *this;
        }
        template <typename T>
        text_colour& operator=(T&& aOther)
        {
            colour_or_gradient::operator=(std::forward<T>(aOther));
            return *this;
        }
    public:
        colour::component alpha() const
        {
            if (std::holds_alternative<colour>(*this))
                return std::get<colour>(*this).alpha();
            else
                return 255;
        }
        text_colour with_alpha(colour::component aAlpha) const
        {
            if (std::holds_alternative<colour>(*this))
                return std::get<colour>(*this).with_alpha(aAlpha);
            if (std::holds_alternative<gradient>(*this))
                return std::get<gradient>(*this).with_combined_alpha(aAlpha);
            else
                return text_colour{};
        }
    };
    typedef std::optional<text_colour> optional_text_colour;

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
        text_effect(text_effect_type aType, const text_colour& aColour, const optional_dimension& aWidth = optional_dimension{}, const optional_auxiliary_parameter& aAux1 = optional_auxiliary_parameter{}) :
            iType{ aType }, iColour{ aColour }, iWidth{ aWidth }, iAux1{ aAux1 }
        {
        }
    public:
        text_effect& operator=(const text_effect& aOther)
        {
            if (&aOther == this)
                return *this;
            iType = aOther.iType;
            iColour = aOther.iColour;
            iWidth = aOther.iWidth;
            iAux1 = aOther.iAux1;
            return *this;
        }
    public:
        bool operator==(const text_effect& aOther) const
        {
            return iType == aOther.iType && iColour == aOther.iColour && iWidth == aOther.iWidth && iAux1 == aOther.iAux1;
        }
        bool operator!=(const text_effect& aOther) const
        {
            return iType != aOther.iType || iColour != aOther.iColour || iWidth != aOther.iWidth || iAux1 != aOther.iAux1;
        }
        bool operator<(const text_effect& aRhs) const
        {
            return std::tie(iType, iColour, iWidth, iAux1) < std::tie(aRhs.iType, aRhs.iColour, aRhs.iWidth, aRhs.iAux1);
        }
    public:
        text_effect_type type() const
        {
            return iType;
        }
        const text_colour& colour() const
        {
            return iColour;
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
        text_effect with_alpha(colour::component aAlpha) const
        {
            return text_effect{ iType, iColour.with_alpha(aAlpha), iWidth, iAux1 };
        }
        text_effect with_alpha(double aAlpha) const
        {
            return with_alpha(static_cast<colour::component>(aAlpha * 255));
        }
    private:
        text_effect_type iType;
        text_colour iColour;
        optional_dimension iWidth;
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
        const text_colour& ink() const
        {
            return iInk;
        }
        const optional_text_colour& paper() const
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
        text_appearance with_alpha(colour::component aAlpha) const
        {
            return text_appearance{ iInk.with_alpha(aAlpha), iPaper != std::nullopt ? optional_text_colour{ iPaper->with_alpha(aAlpha) } : optional_text_colour{}, iEffect != std::nullopt ? iEffect->with_alpha(aAlpha) : optional_text_effect{} };
        }
        text_appearance with_alpha(double aAlpha) const
        {
            return with_alpha(static_cast<colour::component>(aAlpha * 255));
        }
        text_appearance with_only_effect_calculation() const
        {
            auto copy = *this;
            copy.iOnlyCalculateEffect = true;
            return copy;
        }
    private:
        text_colour iInk;
        optional_text_colour iPaper;
        optional_text_effect iEffect;
        bool iOnlyCalculateEffect;
    };

    typedef std::optional<text_appearance> optional_text_appearance;
}