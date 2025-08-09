// hsv_color.hpp
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

namespace neogfx
{
    template <color_space ColorSpace, typename BaseComponent, typename ViewComponent, typename Derived>
    class basic_rgb_color;

    class hsv_color
    {
    public:
        hsv_color();
        hsv_color(double aHue, double aSaturation, double aValue, double aAlpha = 1.0);
        template <color_space ColorSpace, typename BaseComponent, typename ViewComponent = BaseComponent, typename Derived = void>
        hsv_color(const basic_rgb_color<ColorSpace, BaseComponent, ViewComponent, Derived>& aColor) :
            hsv_color{ from_rgb(aColor) } {}
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
        hsv_color with_hue(double aNewHue) const;
        hsv_color with_saturation(double aNewSaturation) const;
        hsv_color with_value(double aNewValue) const;
        hsv_color with_brightness(double aNewLightness) const;
        hsv_color brighter(double aDelta) const;
        hsv_color brighter(double aCoeffecient, double aDelta) const;
        void to_rgb(scalar& aRed, scalar& aGreen, scalar& aBlue, scalar& aAlpha) const;
        template <typename RgbColor>
        typename RgbColor::return_type to_rgb() const
        {
            vec4 components;
            to_rgb(components.x, components.y, components.z, components[3]);
            typename RgbColor::return_type result;
            result.template set_red<scalar>(components.x);
            result.template set_green<scalar>(components.y);
            result.template set_blue<scalar>(components.z);
            result.template set_alpha<scalar>(components[3]);
            return result;
        }
        static hsv_color from_rgb(scalar aRed, scalar aGreen, scalar aBlue, scalar aAlpha = 1.0);
        template <color_space ColorSpace, typename BaseComponent, typename ViewComponent = BaseComponent, typename Derived = void>
        static hsv_color from_rgb(const basic_rgb_color<ColorSpace, BaseComponent, ViewComponent, Derived>& aColor)
        {
            return from_rgb(aColor.template red<scalar>(), aColor.template green<scalar>(), aColor.template blue<scalar>(), aColor.template alpha<scalar>());
        }
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