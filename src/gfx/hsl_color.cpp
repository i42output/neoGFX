// hsl_color.cpp
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

#include <neolib/core/string_utils.hpp>

#include <neogfx/gfx/color.hpp>

namespace neogfx
{
    hsl_color::hsl_color() :
        iHue{0.0}, iSaturation{0.0}, iLightness{0.0}
    {
    }

    hsl_color::hsl_color(double aHue, double aSaturation, double aLightness, double aAlpha) :
        iHue{aHue}, 
        iSaturation{ std::min(std::max(aSaturation, 0.0), 1.0) }, 
        iLightness{ std::min(std::max(aLightness, 0.0), 1.0) }, 
        iAlpha{aAlpha}
    {
        if (iHue != undefined_hue())
            iHue = std::fmod(iHue, 360.0);
    }

    double hsl_color::hue() const
    {
        if (iHue != undefined_hue())
            return iHue;
        return 0.0;
    }

    double hsl_color::saturation() const
    {
        return iSaturation;
    }

    double hsl_color::lightness() const
    {
        return iLightness;
    }

    double hsl_color::alpha() const
    {
        return iAlpha;
    }

    void hsl_color::set_hue(double aHue)
    {
        iHue = std::fmod(aHue, 360.0);
    }

    void hsl_color::set_saturation(double aSaturation)
    {
        iSaturation = std::min(std::max(aSaturation, 0.0), 1.0);
    }

    void hsl_color::set_lightness(double aLightness)
    {
        iLightness = std::min(std::max(aLightness, 0.0), 1.0);
    }

    void hsl_color::set_alpha(double aAlpha)
    {
        iAlpha = aAlpha;
    }

    bool hsl_color::hue_undefined() const
    {
        return iHue == undefined_hue();
    }

    hsl_color hsl_color::with_hue(double aNewHue) const
    {
        hsl_color result = *this;
        result.set_hue(aNewHue);
        return result;
    }

    hsl_color hsl_color::with_saturation(double aNewSaturation) const
    {
        hsl_color result = *this;
        result.set_saturation(aNewSaturation);
        return result;
    }

    hsl_color hsl_color::with_lightness(double aNewLightness) const
    {
        return lighter(0.0, aNewLightness);
    }

    hsl_color hsl_color::lighter(double aDelta) const
    {
        return lighter(1.0, aDelta);
    }

    hsl_color hsl_color::lighter(double coeffecient, double delta) const
    {
        hsl_color result = *this;
        result.iLightness *= coeffecient;
        result.iLightness += delta;
        result.iLightness = std::min(std::max(result.iLightness, 0.0), 1.0);
        return result;
    }

    hsl_color hsl_color::shade(double aDelta) const
    {
        return lightness() >= 0.5 ? with_lightness(lightness() - aDelta) : with_lightness(lightness() + aDelta);
    }

    hsl_color hsl_color::unshade(double aDelta) const
    {
        return shade(-aDelta);
    }

    void hsl_color::to_rgb(scalar& aRed, scalar& aGreen, scalar& aBlue, scalar& aAlpha) const
    {
        double c = (1.0 - std::abs(2.0 * lightness() - 1.0)) * saturation();
        double h2 = hue() / 60.0;
        double x = c * (1.0 - std::abs(std::fmod(h2, 2.0) - 1.0));
        double r, g, b;
        if (h2 >= 0.0 && h2 < 1.0)
            r = c, g = x, b = 0.0;
        else if (h2 >= 1.0 && h2 < 2.0)
            r = x, g = c, b = 0.0;
        else if (h2 >= 2.0 && h2 < 3.0)
            r = 0.0, g = c, b = x;
        else if (h2 >= 3.0 && h2 < 4.0)
            r = 0.0, g = x, b = c;
        else if (h2 >= 4.0 && h2 < 5.0)
            r = x, g = 0.0, b = c;
        else if (h2 >= 5.0 && h2 < 6.0)
            r = c, g = 0.0, b = x;
        else
            r = g = b = 0.0;
        double m = std::abs(lightness() - 0.5f * c);
        aRed = r + m;
        aGreen = g + m;
        aBlue = b + m;
        aAlpha = alpha();
    }

    hsl_color hsl_color::from_rgb(scalar aRed, scalar aGreen, scalar aBlue, scalar aAlpha)
    {
        double hue, saturation, lightness;
        double r = aRed, g = aGreen, b = aBlue;
        double M = std::max(std::max(r, g), b);
        double m = std::min(std::min(r, g), b);
        double c = M - m;
        double h2;
        if (c == 0.0)
            h2 = undefined_hue();
        else if (M == r)
            h2 = std::fmod((g - b) / c, 6.0);
        else if (M == g)
            h2 = (b - r) / c + 2.0;
        else if (M == b)
            h2 = (r - g) / c + 4.0;
        else
            h2 = undefined_hue();
        if (h2 != undefined_hue())
        {
            hue = 60.0 * h2;
            if (hue < 0.0)
                hue += 360.0;
        }
        else
            hue = undefined_hue();
        lightness = 0.5f * (M + m);
        lightness = std::max(std::min(lightness, 1.0), 0.0);
        if (c == 0.0)
            saturation = 0.0;
        else
            saturation = c / (1.0 - std::abs(2.0 * lightness - 1.0));
        saturation = std::max(std::min(saturation, 1.0), 0.0);
        return hsl_color(hue, saturation, lightness, aAlpha);
    }

    double hsl_color::undefined_hue()
    {
        return -std::numeric_limits<double>::infinity();
    }

    bool hsl_color::operator==(const hsl_color& aOther) const
    {
        return hue() == aOther.hue() &&
            saturation() == aOther.saturation() &&
            lightness() == aOther.lightness() &&
            alpha() == aOther.alpha() &&
            hue_undefined() == aOther.hue_undefined();
    }

    bool hsl_color::operator!=(const hsl_color& aOther) const
    {
        return !(*this == aOther);
    }

    bool hsl_color::operator<(const hsl_color& aOther) const
    {
        return std::make_tuple(hue(), saturation(), lightness(), alpha()) < std::make_tuple(aOther.hue(), aOther.saturation(), aOther.lightness(), aOther.alpha());
    }
}