// hsv_color.hpp
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
    class color;

    class hsv_color
    {
    public:
        hsv_color();
        hsv_color(double aHue, double aSaturation, double aValue, double aAlpha = 1.0);
        hsv_color(const color& aColor);
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
        hsv_color with_brightness(double aNewLightness) const;
        hsv_color brighter(double aDelta) const;
        hsv_color brighter(double aCoeffecient, double aDelta) const;
        color to_rgb() const;
        static hsv_color from_rgb(const color& aColor);
    public:
        static double undefined_hue();
    public:
        bool operator==(const hsv_color& aOther) const;
        bool operator!=(const hsv_color& aOther) const;
        bool operator<(const hsv_color& aOther) const;
    private:
        double iHue;
        double iSaturation;
        double iValue;
        double iAlpha;
    };
}