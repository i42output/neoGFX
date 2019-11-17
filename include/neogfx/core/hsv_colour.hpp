// hsv_colour.hpp
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

namespace neogfx
{
    class colour;

    class hsv_colour
    {
    public:
        hsv_colour();
        hsv_colour(double aHue, double aSaturation, double aValue, double aAlpha = 1.0);
        hsv_colour(const colour& aColour);
    public:
        double hue() const;
        double saturation() const;
        double value() const;
        double brightness() const;
        double alpha() const;
        void set_hue(double aHue);
        void set_saturation(double aSaturation);
        void set_value(double aValue);
        void set_brightness(double aBrightness);
        void set_alpha(double aAlpha);
        bool hue_undefined() const;
    public:
        hsv_colour with_brightness(double aNewLightness) const;
        hsv_colour brighter(double aDelta) const;
        hsv_colour brighter(double aCoeffecient, double aDelta) const;
        colour to_rgb() const;
        static hsv_colour from_rgb(const colour& aColour);
    public:
        static double undefined_hue();
    public:
        bool operator==(const hsv_colour& aOther) const;
        bool operator!=(const hsv_colour& aOther) const;
        bool operator<(const hsv_colour& aOther) const;
    private:
        double iHue;
        double iSaturation;
        double iValue;
        double iAlpha;
    };
}