// hsl_colour.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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

	class hsl_colour
	{
	public:
		hsl_colour();
		hsl_colour(double aHue, double aSaturation, double aLightness, double aAlpha = 1.0);
		hsl_colour(const colour& aColour);
	public:
		double hue() const;
		double saturation() const;
		double lightness() const;
		double alpha() const;
		void set_hue(double aHue);
		void set_saturation(double aSaturation);
		void set_lightness(double aLightness);
		void set_alpha(double aAlpha);
		bool hue_undefined() const;
	public:
		hsl_colour with_lightness(double aNewLightness) const;
		hsl_colour lighter(double aDelta) const;
		hsl_colour lighter(double aCoeffecient, double aDelta) const;
		colour to_rgb() const;
		static hsl_colour from_rgb(const colour& aColour);
	public:
		static double undefined_hue();
	public:
		bool operator==(const hsl_colour& aOther) const;
		bool operator!=(const hsl_colour& aOther) const;
		bool operator<(const hsl_colour& aOther) const;
	private:
		double iHue;
		double iSaturation;
		double iLightness;
		double iAlpha;
	};
}