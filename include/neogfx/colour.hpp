// colour.hpp
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

#include "neogfx.hpp"
#include "geometry.hpp"
#include "hsl_colour.hpp"

namespace neogfx
{
	class hsl_colour;

	class colour
	{
		// types
	public:
		typedef uint32_t argb;
		typedef uint8_t component;
		struct matrix : std::tr1::array<std::tr1::array<double, 5>, 5>
		{
			matrix() : std::tr1::array<std::tr1::array<double, 5>, 5>() {}
		};
		// constants
	public:
		static const colour AliceBlue;
		static const colour AntiqueWhite;
		static const colour AntiqueWhite1;
		static const colour AntiqueWhite2;
		static const colour AntiqueWhite3;
		static const colour AntiqueWhite4;
		static const colour Aquamarine;
		static const colour Aquamarine1;
		static const colour Aquamarine2;
		static const colour Aquamarine3;
		static const colour Aquamarine4;
		static const colour Azure;
		static const colour Azure1;
		static const colour Azure2;
		static const colour Azure3;
		static const colour Azure4;
		static const colour Beige;
		static const colour Bisque;
		static const colour Bisque1;
		static const colour Bisque2;
		static const colour Bisque3;
		static const colour Bisque4;
		static const colour Black;
		static const colour BlanchedAlmond;
		static const colour Blue;
		static const colour Blue1;
		static const colour Blue2;
		static const colour Blue3;
		static const colour Blue4;
		static const colour BlueViolet;
		static const colour Brown;
		static const colour Brown1;
		static const colour Brown2;
		static const colour Brown3;
		static const colour Brown4;
		static const colour Burlywood;
		static const colour Burlywood1;
		static const colour Burlywood2;
		static const colour Burlywood3;
		static const colour Burlywood4;
		static const colour CadetBlue;
		static const colour CadetBlue1;
		static const colour CadetBlue2;
		static const colour CadetBlue3;
		static const colour CadetBlue4;
		static const colour Chartreuse;
		static const colour Chartreuse1;
		static const colour Chartreuse2;
		static const colour Chartreuse3;
		static const colour Chartreuse4;
		static const colour Chocolate;
		static const colour Chocolate1;
		static const colour Chocolate2;
		static const colour Chocolate3;
		static const colour Chocolate4;
		static const colour Coral;
		static const colour Coral1;
		static const colour Coral2;
		static const colour Coral3;
		static const colour Coral4;
		static const colour CornflowerBlue;
		static const colour Cornsilk;
		static const colour Cornsilk1;
		static const colour Cornsilk2;
		static const colour Cornsilk3;
		static const colour Cornsilk4;
		static const colour Cyan;
		static const colour Cyan1;
		static const colour Cyan2;
		static const colour Cyan3;
		static const colour Cyan4;
		static const colour DarkBlue;
		static const colour DarkCyan;
		static const colour DarkGoldenrod;
		static const colour DarkGoldenrod1;
		static const colour DarkGoldenrod2;
		static const colour DarkGoldenrod3;
		static const colour DarkGoldenrod4;
		static const colour DarkGray;
		static const colour DarkGreen;
		static const colour DarkGrey;
		static const colour DarkKhaki;
		static const colour DarkMagenta;
		static const colour DarkOliveGreen;
		static const colour DarkOliveGreen1;
		static const colour DarkOliveGreen2;
		static const colour DarkOliveGreen3;
		static const colour DarkOliveGreen4;
		static const colour DarkOrange;
		static const colour DarkOrange1;
		static const colour DarkOrange2;
		static const colour DarkOrange3;
		static const colour DarkOrange4;
		static const colour DarkOrchid;
		static const colour DarkOrchid1;
		static const colour DarkOrchid2;
		static const colour DarkOrchid3;
		static const colour DarkOrchid4;
		static const colour DarkRed;
		static const colour DarkSalmon;
		static const colour DarkSeaGreen;
		static const colour DarkSeaGreen1;
		static const colour DarkSeaGreen2;
		static const colour DarkSeaGreen3;
		static const colour DarkSeaGreen4;
		static const colour DarkSlateBlue;
		static const colour DarkSlateGray;
		static const colour DarkSlateGray1;
		static const colour DarkSlateGray2;
		static const colour DarkSlateGray3;
		static const colour DarkSlateGray4;
		static const colour DarkSlateGrey;
		static const colour DarkTurquoise;
		static const colour DarkViolet;
		static const colour DebianRed;
		static const colour DeepPink;
		static const colour DeepPink1;
		static const colour DeepPink2;
		static const colour DeepPink3;
		static const colour DeepPink4;
		static const colour DeepSkyBlue;
		static const colour DeepSkyBlue1;
		static const colour DeepSkyBlue2;
		static const colour DeepSkyBlue3;
		static const colour DeepSkyBlue4;
		static const colour DimGray;
		static const colour DimGrey;
		static const colour DodgerBlue;
		static const colour DodgerBlue1;
		static const colour DodgerBlue2;
		static const colour DodgerBlue3;
		static const colour DodgerBlue4;
		static const colour Firebrick;
		static const colour Firebrick1;
		static const colour Firebrick2;
		static const colour Firebrick3;
		static const colour Firebrick4;
		static const colour FloralWhite;
		static const colour ForestGreen;
		static const colour Gainsboro;
		static const colour GhostWhite;
		static const colour Gold;
		static const colour Gold1;
		static const colour Gold2;
		static const colour Gold3;
		static const colour Gold4;
		static const colour Goldenrod;
		static const colour Goldenrod1;
		static const colour Goldenrod2;
		static const colour Goldenrod3;
		static const colour Goldenrod4;
		static const colour Gray;
		static const colour Gray0;
		static const colour Gray1;
		static const colour Gray10;
		static const colour Gray100;
		static const colour Gray11;
		static const colour Gray12;
		static const colour Gray13;
		static const colour Gray14;
		static const colour Gray15;
		static const colour Gray16;
		static const colour Gray17;
		static const colour Gray18;
		static const colour Gray19;
		static const colour Gray2;
		static const colour Gray20;
		static const colour Gray21;
		static const colour Gray22;
		static const colour Gray23;
		static const colour Gray24;
		static const colour Gray25;
		static const colour Gray26;
		static const colour Gray27;
		static const colour Gray28;
		static const colour Gray29;
		static const colour Gray3;
		static const colour Gray30;
		static const colour Gray31;
		static const colour Gray32;
		static const colour Gray33;
		static const colour Gray34;
		static const colour Gray35;
		static const colour Gray36;
		static const colour Gray37;
		static const colour Gray38;
		static const colour Gray39;
		static const colour Gray4;
		static const colour Gray40;
		static const colour Gray41;
		static const colour Gray42;
		static const colour Gray43;
		static const colour Gray44;
		static const colour Gray45;
		static const colour Gray46;
		static const colour Gray47;
		static const colour Gray48;
		static const colour Gray49;
		static const colour Gray5;
		static const colour Gray50;
		static const colour Gray51;
		static const colour Gray52;
		static const colour Gray53;
		static const colour Gray54;
		static const colour Gray55;
		static const colour Gray56;
		static const colour Gray57;
		static const colour Gray58;
		static const colour Gray59;
		static const colour Gray6;
		static const colour Gray60;
		static const colour Gray61;
		static const colour Gray62;
		static const colour Gray63;
		static const colour Gray64;
		static const colour Gray65;
		static const colour Gray66;
		static const colour Gray67;
		static const colour Gray68;
		static const colour Gray69;
		static const colour Gray7;
		static const colour Gray70;
		static const colour Gray71;
		static const colour Gray72;
		static const colour Gray73;
		static const colour Gray74;
		static const colour Gray75;
		static const colour Gray76;
		static const colour Gray77;
		static const colour Gray78;
		static const colour Gray79;
		static const colour Gray8;
		static const colour Gray80;
		static const colour Gray81;
		static const colour Gray82;
		static const colour Gray83;
		static const colour Gray84;
		static const colour Gray85;
		static const colour Gray86;
		static const colour Gray87;
		static const colour Gray88;
		static const colour Gray89;
		static const colour Gray9;
		static const colour Gray90;
		static const colour Gray91;
		static const colour Gray92;
		static const colour Gray93;
		static const colour Gray94;
		static const colour Gray95;
		static const colour Gray96;
		static const colour Gray97;
		static const colour Gray98;
		static const colour Gray99;
		static const colour Green;
		static const colour Green1;
		static const colour Green2;
		static const colour Green3;
		static const colour Green4;
		static const colour GreenYellow;
		static const colour Grey;
		static const colour Grey0;
		static const colour Grey1;
		static const colour Grey10;
		static const colour Grey100;
		static const colour Grey11;
		static const colour Grey12;
		static const colour Grey13;
		static const colour Grey14;
		static const colour Grey15;
		static const colour Grey16;
		static const colour Grey17;
		static const colour Grey18;
		static const colour Grey19;
		static const colour Grey2;
		static const colour Grey20;
		static const colour Grey21;
		static const colour Grey22;
		static const colour Grey23;
		static const colour Grey24;
		static const colour Grey25;
		static const colour Grey26;
		static const colour Grey27;
		static const colour Grey28;
		static const colour Grey29;
		static const colour Grey3;
		static const colour Grey30;
		static const colour Grey31;
		static const colour Grey32;
		static const colour Grey33;
		static const colour Grey34;
		static const colour Grey35;
		static const colour Grey36;
		static const colour Grey37;
		static const colour Grey38;
		static const colour Grey39;
		static const colour Grey4;
		static const colour Grey40;
		static const colour Grey41;
		static const colour Grey42;
		static const colour Grey43;
		static const colour Grey44;
		static const colour Grey45;
		static const colour Grey46;
		static const colour Grey47;
		static const colour Grey48;
		static const colour Grey49;
		static const colour Grey5;
		static const colour Grey50;
		static const colour Grey51;
		static const colour Grey52;
		static const colour Grey53;
		static const colour Grey54;
		static const colour Grey55;
		static const colour Grey56;
		static const colour Grey57;
		static const colour Grey58;
		static const colour Grey59;
		static const colour Grey6;
		static const colour Grey60;
		static const colour Grey61;
		static const colour Grey62;
		static const colour Grey63;
		static const colour Grey64;
		static const colour Grey65;
		static const colour Grey66;
		static const colour Grey67;
		static const colour Grey68;
		static const colour Grey69;
		static const colour Grey7;
		static const colour Grey70;
		static const colour Grey71;
		static const colour Grey72;
		static const colour Grey73;
		static const colour Grey74;
		static const colour Grey75;
		static const colour Grey76;
		static const colour Grey77;
		static const colour Grey78;
		static const colour Grey79;
		static const colour Grey8;
		static const colour Grey80;
		static const colour Grey81;
		static const colour Grey82;
		static const colour Grey83;
		static const colour Grey84;
		static const colour Grey85;
		static const colour Grey86;
		static const colour Grey87;
		static const colour Grey88;
		static const colour Grey89;
		static const colour Grey9;
		static const colour Grey90;
		static const colour Grey91;
		static const colour Grey92;
		static const colour Grey93;
		static const colour Grey94;
		static const colour Grey95;
		static const colour Grey96;
		static const colour Grey97;
		static const colour Grey98;
		static const colour Grey99;
		static const colour Honeydew;
		static const colour Honeydew1;
		static const colour Honeydew2;
		static const colour Honeydew3;
		static const colour Honeydew4;
		static const colour HotPink;
		static const colour HotPink1;
		static const colour HotPink2;
		static const colour HotPink3;
		static const colour HotPink4;
		static const colour IndianRed;
		static const colour IndianRed1;
		static const colour IndianRed2;
		static const colour IndianRed3;
		static const colour IndianRed4;
		static const colour Ivory;
		static const colour Ivory1;
		static const colour Ivory2;
		static const colour Ivory3;
		static const colour Ivory4;
		static const colour Khaki;
		static const colour Khaki1;
		static const colour Khaki2;
		static const colour Khaki3;
		static const colour Khaki4;
		static const colour Lavender;
		static const colour LavenderBlush;
		static const colour LavenderBlush1;
		static const colour LavenderBlush2;
		static const colour LavenderBlush3;
		static const colour LavenderBlush4;
		static const colour LawnGreen;
		static const colour LemonChiffon;
		static const colour LemonChiffon1;
		static const colour LemonChiffon2;
		static const colour LemonChiffon3;
		static const colour LemonChiffon4;
		static const colour LightBlue;
		static const colour LightBlue1;
		static const colour LightBlue2;
		static const colour LightBlue3;
		static const colour LightBlue4;
		static const colour LightCoral;
		static const colour LightCyan;
		static const colour LightCyan1;
		static const colour LightCyan2;
		static const colour LightCyan3;
		static const colour LightCyan4;
		static const colour LightGoldenrod;
		static const colour LightGoldenrod1;
		static const colour LightGoldenrod2;
		static const colour LightGoldenrod3;
		static const colour LightGoldenrod4;
		static const colour LightGoldenrodYellow;
		static const colour LightGray;
		static const colour LightGreen;
		static const colour LightGrey;
		static const colour LightPink;
		static const colour LightPink1;
		static const colour LightPink2;
		static const colour LightPink3;
		static const colour LightPink4;
		static const colour LightSalmon;
		static const colour LightSalmon1;
		static const colour LightSalmon2;
		static const colour LightSalmon3;
		static const colour LightSalmon4;
		static const colour LightSeaGreen;
		static const colour LightSkyBlue;
		static const colour LightSkyBlue1;
		static const colour LightSkyBlue2;
		static const colour LightSkyBlue3;
		static const colour LightSkyBlue4;
		static const colour LightSlateBlue;
		static const colour LightSlateGray;
		static const colour LightSlateGrey;
		static const colour LightSteelBlue;
		static const colour LightSteelBlue1;
		static const colour LightSteelBlue2;
		static const colour LightSteelBlue3;
		static const colour LightSteelBlue4;
		static const colour LightYellow;
		static const colour LightYellow1;
		static const colour LightYellow2;
		static const colour LightYellow3;
		static const colour LightYellow4;
		static const colour LimeGreen;
		static const colour Linen;
		static const colour Magenta;
		static const colour Magenta1;
		static const colour Magenta2;
		static const colour Magenta3;
		static const colour Magenta4;
		static const colour Maroon;
		static const colour Maroon1;
		static const colour Maroon2;
		static const colour Maroon3;
		static const colour Maroon4;
		static const colour MediumAquamarine;
		static const colour MediumBlue;
		static const colour MediumOrchid;
		static const colour MediumOrchid1;
		static const colour MediumOrchid2;
		static const colour MediumOrchid3;
		static const colour MediumOrchid4;
		static const colour MediumPurple;
		static const colour MediumPurple1;
		static const colour MediumPurple2;
		static const colour MediumPurple3;
		static const colour MediumPurple4;
		static const colour MediumSeaGreen;
		static const colour MediumSlateBlue;
		static const colour MediumSpringGreen;
		static const colour MediumTurquoise;
		static const colour MediumVioletRed;
		static const colour MidnightBlue;
		static const colour MintCream;
		static const colour MistyRose;
		static const colour MistyRose1;
		static const colour MistyRose2;
		static const colour MistyRose3;
		static const colour MistyRose4;
		static const colour Moccasin;
		static const colour NavajoWhite;
		static const colour NavajoWhite1;
		static const colour NavajoWhite2;
		static const colour NavajoWhite3;
		static const colour NavajoWhite4;
		static const colour Navy;
		static const colour NavyBlue;
		static const colour OldLace;
		static const colour OliveDrab;
		static const colour OliveDrab1;
		static const colour OliveDrab2;
		static const colour OliveDrab3;
		static const colour OliveDrab4;
		static const colour Orange;
		static const colour Orange1;
		static const colour Orange2;
		static const colour Orange3;
		static const colour Orange4;
		static const colour OrangeRed;
		static const colour OrangeRed1;
		static const colour OrangeRed2;
		static const colour OrangeRed3;
		static const colour OrangeRed4;
		static const colour Orchid;
		static const colour Orchid1;
		static const colour Orchid2;
		static const colour Orchid3;
		static const colour Orchid4;
		static const colour PaleGoldenrod;
		static const colour PaleGreen;
		static const colour PaleGreen1;
		static const colour PaleGreen2;
		static const colour PaleGreen3;
		static const colour PaleGreen4;
		static const colour PaleTurquoise;
		static const colour PaleTurquoise1;
		static const colour PaleTurquoise2;
		static const colour PaleTurquoise3;
		static const colour PaleTurquoise4;
		static const colour PaleVioletRed;
		static const colour PaleVioletRed1;
		static const colour PaleVioletRed2;
		static const colour PaleVioletRed3;
		static const colour PaleVioletRed4;
		static const colour PapayaWhip;
		static const colour PeachPuff;
		static const colour PeachPuff1;
		static const colour PeachPuff2;
		static const colour PeachPuff3;
		static const colour PeachPuff4;
		static const colour Peru;
		static const colour Pink;
		static const colour Pink1;
		static const colour Pink2;
		static const colour Pink3;
		static const colour Pink4;
		static const colour Plum;
		static const colour Plum1;
		static const colour Plum2;
		static const colour Plum3;
		static const colour Plum4;
		static const colour PowderBlue;
		static const colour Purple;
		static const colour Purple1;
		static const colour Purple2;
		static const colour Purple3;
		static const colour Purple4;
		static const colour Red;
		static const colour Red1;
		static const colour Red2;
		static const colour Red3;
		static const colour Red4;
		static const colour RosyBrown;
		static const colour RosyBrown1;
		static const colour RosyBrown2;
		static const colour RosyBrown3;
		static const colour RosyBrown4;
		static const colour RoyalBlue;
		static const colour RoyalBlue1;
		static const colour RoyalBlue2;
		static const colour RoyalBlue3;
		static const colour RoyalBlue4;
		static const colour SaddleBrown;
		static const colour Salmon;
		static const colour Salmon1;
		static const colour Salmon2;
		static const colour Salmon3;
		static const colour Salmon4;
		static const colour SandyBrown;
		static const colour SeaGreen;
		static const colour SeaGreen1;
		static const colour SeaGreen2;
		static const colour SeaGreen3;
		static const colour SeaGreen4;
		static const colour Seashell;
		static const colour Seashell1;
		static const colour Seashell2;
		static const colour Seashell3;
		static const colour Seashell4;
		static const colour Sienna;
		static const colour Sienna1;
		static const colour Sienna2;
		static const colour Sienna3;
		static const colour Sienna4;
		static const colour SkyBlue;
		static const colour SkyBlue1;
		static const colour SkyBlue2;
		static const colour SkyBlue3;
		static const colour SkyBlue4;
		static const colour SlateBlue;
		static const colour SlateBlue1;
		static const colour SlateBlue2;
		static const colour SlateBlue3;
		static const colour SlateBlue4;
		static const colour SlateGray;
		static const colour SlateGray1;
		static const colour SlateGray2;
		static const colour SlateGray3;
		static const colour SlateGray4;
		static const colour SlateGrey;
		static const colour Snow;
		static const colour Snow1;
		static const colour Snow2;
		static const colour Snow3;
		static const colour Snow4;
		static const colour SpringGreen;
		static const colour SpringGreen1;
		static const colour SpringGreen2;
		static const colour SpringGreen3;
		static const colour SpringGreen4;
		static const colour SteelBlue;
		static const colour SteelBlue1;
		static const colour SteelBlue2;
		static const colour SteelBlue3;
		static const colour SteelBlue4;
		static const colour Tan;
		static const colour Tan1;
		static const colour Tan2;
		static const colour Tan3;
		static const colour Tan4;
		static const colour Thistle;
		static const colour Thistle1;
		static const colour Thistle2;
		static const colour Thistle3;
		static const colour Thistle4;
		static const colour Tomato;
		static const colour Tomato1;
		static const colour Tomato2;
		static const colour Tomato3;
		static const colour Tomato4;
		static const colour Turquoise;
		static const colour Turquoise1;
		static const colour Turquoise2;
		static const colour Turquoise3;
		static const colour Turquoise4;
		static const colour Violet;
		static const colour VioletRed;
		static const colour VioletRed1;
		static const colour VioletRed2;
		static const colour VioletRed3;
		static const colour VioletRed4;
		static const colour Wheat;
		static const colour Wheat1;
		static const colour Wheat2;
		static const colour Wheat3;
		static const colour Wheat4;
		static const colour White;
		static const colour WhiteSmoke;
		static const colour Yellow;
		static const colour Yellow1;
		static const colour Yellow2;
		static const colour Yellow3;
		static const colour Yellow4;
		static const colour YellowGreen;
	public:
		struct not_found : std::logic_error { not_found() : std::logic_error("neogfx::colour::not_found") {} };
	public:
		static const component MinComponetValue = 0x00;
		static const component MaxComponetValue = 0xFF;
	private:
		static const argb AlphaShift = 24;
		static const argb RedShift = 16;
		static const argb GreenShift = 8;
		static const argb BlueShift = 0;
		// construction
	public:
		colour();
		explicit colour(argb aValue);
		colour(component aRed, component aGreen, component aBlue, component aAlpha = 0xFF);
		colour(const std::string& aTextValue);
		// operations
	public:
		static colour from_name(const std::string& aName);
		argb value() const;
		component alpha() const;
		component red() const;
		component green() const;
		component blue() const;
		template <typename T>
		T alpha() const { return static_cast<T>(alpha()) / 0xFF; }
		template <typename T>
		T red() const { return static_cast<T>(red()) / 0xFF; }
		template <typename T>
		T green() const { return static_cast<T>(green()) / 0xFF; }
		template <typename T>
		T blue() const { return static_cast<T>(blue()) / 0xFF; }
		void set_alpha(component aNewValue);
		void set_red(component aNewValue);
		void set_green(component aNewValue);
		void set_blue(component aNewValue);
		hsl_colour to_hsl() const;
		double intensity() const;
		bool similar_intensity(const colour& aOther, double aThreshold = 0.5);
		colour mid(const colour& aOther) const;
		bool light(component aLightIntensity = 0xC0) const;
		bool dark(component aDarkIntensity = 0x40) const;
		colour& lighten(component aDelta);
		colour& darken(component aDelta);
		colour lighter(component aDelta) const;
		colour darker(component aDelta) const;
		colour shade(component aDelta) const;
		colour monochrome() const;
		colour same_lightness_as(const colour& aOther) const;
		colour inverse() const;
		colour& operator+=(component aDelta);
		colour& operator-=(component aDelta);
		colour operator~() const;
		bool operator==(const colour& aOther) const;
		bool operator!=(const colour& aOther) const;
		bool operator<(const colour& aOther) const;
		std::string to_string() const;
		// attributes
	private:
		argb iValue;
	};

	inline colour operator+(const colour& aLeft, colour::component aRight)
	{
		colour ret = aLeft;
		ret += aRight;
		return ret;
	}

	inline colour operator-(const colour& aLeft, colour::component aRight)
	{
		colour ret = aLeft;
		ret -= aRight;
		return ret;
	}

	class gradient
	{
		// types
	public:
		enum direction_e
		{
			Vertical,
			Horizontal,
			Radial
		};
	public:
		struct bad_position : std::logic_error { bad_position() : std::logic_error("neogfx::gradient::bad_position") {} };
		// construction
	public:
		gradient(const colour& aFrom, const colour& aTo, direction_e aDirection = Vertical);
		// operations
	public:
		colour at(coordinate aPos, coordinate aStart, coordinate aEnd) const;
		colour at(double aPos) const;
		direction_e direction() const;
		bool operator<(const gradient& aOther) const;
		// attributes
	private:
		colour iFrom;
		colour iTo;
		direction_e iDirection;
	};

	typedef boost::optional<colour> optional_colour;
}