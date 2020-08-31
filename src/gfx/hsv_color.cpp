// hsv_color.cpp
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
    hsv_color::hsv_color() :
        iHue{0.0}, iSaturation{0.0}, iValue{0.0}, iAlpha{1.0}
    {
    }

    hsv_color::hsv_color(double aHue, double aSaturation, double aValue, double aAlpha) :
        iHue{aHue}, iSaturation{ aSaturation }, iValue{ aValue }, iAlpha{ aAlpha }
    {
        if (iHue != undefined_hue())
            iHue = std::fmod(iHue, 360.0);
    }

    double hsv_color::hue() const
    {
        if (iHue != undefined_hue())
            return iHue;
        return 0.0;
    }

    double hsv_color::saturation() const
    {
        return iSaturation;
    }

    double hsv_color::value() const
    {
        return iValue;
    }

    double hsv_color::brightness() const
    {
        return value();
    }

    double hsv_color::alpha() const
    {
        return iAlpha;
    }

    void hsv_color::set_hue(double aHue)
    {
        iHue = std::fmod(aHue, 360.0);
    }

    void hsv_color::set_saturation(double aSaturation)
    {
        iSaturation = aSaturation;
    }

    void hsv_color::set_value(double aValue)
    {
        iValue = aValue;
    }

    void hsv_color::set_brightness(double aBrightness)
    {
        set_value(aBrightness);
    }

    void hsv_color::set_alpha(double aAlpha)
    {
        iAlpha = aAlpha;
    }

    bool hsv_color::hue_undefined() const
    {
        return iHue == undefined_hue();
    }

    hsv_color hsv_color::with_hue(double aNewHue) const
    {
        hsv_color result = *this;
        result.iHue = aNewHue;
        return result;
    }

    hsv_color hsv_color::with_saturation(double aNewSaturation) const
    {
        hsv_color result = *this;
        result.iSaturation = aNewSaturation;
        return result;
    }

    hsv_color hsv_color::with_value(double aNewValue) const
    {
        hsv_color result = *this;
        result.iValue = aNewValue;
        return result;
    }

    hsv_color hsv_color::with_brightness(double aNewBrightness) const
    {
        return brighter(0.0, aNewBrightness);
    }

    hsv_color hsv_color::brighter(double aDelta) const
    {
        return brighter(1.0, aDelta);
    }

    hsv_color hsv_color::brighter(double coeffecient, double delta) const
    {
        hsv_color result = *this;
        result.iValue *= coeffecient;
        result.iValue += delta;
        result.iValue = std::min(std::max(result.iValue, 0.0), 1.0);
        return result;
    }

    void hsv_color::to_rgb(scalar& aRed, scalar& aGreen, scalar& aBlue, scalar& aAlpha) const
    {
        double c = value() * saturation();
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
        double m = std::abs(value() - c);
        aRed = r + m;
        aGreen = g + m;
        aBlue = b + m;
        aAlpha = alpha();
    }

    hsv_color hsv_color::from_rgb(scalar aRed, scalar aGreen, scalar aBlue, scalar aAlpha)
    {
        double hue, saturation, value;
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
        value = M;
        value = std::max(std::min(value, 1.0), 0.0);
        if (c == 0.0)
            saturation = 0.0;
        else
            saturation = c / value;
        saturation = std::max(std::min(saturation, 1.0), 0.0);
        return hsv_color(hue, saturation, value, aAlpha);
    }

    double hsv_color::undefined_hue()
    {
        return -std::numeric_limits<double>::max();
    }


    bool hsv_color::operator==(const hsv_color& aOther) const
    {
        return hue() == aOther.hue() &&
            saturation() == aOther.saturation() &&
            value() == aOther.value() &&
            alpha() == aOther.alpha() &&
            hue_undefined() == aOther.hue_undefined();
    }

    bool hsv_color::operator!=(const hsv_color& aOther) const
    {
        return !(*this == aOther);
    }

    bool hsv_color::operator<(const hsv_color& aOther) const
    {
        return std::make_tuple(hue(), saturation(), value(), alpha()) < std::make_tuple(aOther.hue(), aOther.saturation(), aOther.value(), aOther.alpha());
    }
}