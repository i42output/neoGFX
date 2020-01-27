// color.hpp
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
#include <type_traits>
#include <neolib/variant.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/gui/layout/i_geometry.hpp>
#include <neogfx/core/hsl_color.hpp>
#include <neogfx/core/hsv_color.hpp>

namespace neogfx
{
    class color
    {
        // types
    public:
        typedef uint32_t argb;
        typedef uint8_t component;
        struct matrix : std::array<std::array<double, 5>, 5>
        {
            matrix() : std::array<std::array<double, 5>, 5>() {}
        };
        // constants
    public:
        static const color AliceBlue;
        static const color AntiqueWhite;
        static const color AntiqueWhite1;
        static const color AntiqueWhite2;
        static const color AntiqueWhite3;
        static const color AntiqueWhite4;
        static const color Aquamarine;
        static const color Aquamarine1;
        static const color Aquamarine2;
        static const color Aquamarine3;
        static const color Aquamarine4;
        static const color Azure;
        static const color Azure1;
        static const color Azure2;
        static const color Azure3;
        static const color Azure4;
        static const color Beige;
        static const color Bisque;
        static const color Bisque1;
        static const color Bisque2;
        static const color Bisque3;
        static const color Bisque4;
        static const color Black;
        static const color BlanchedAlmond;
        static const color Blue;
        static const color Blue1;
        static const color Blue2;
        static const color Blue3;
        static const color Blue4;
        static const color BlueViolet;
        static const color Brown;
        static const color Brown1;
        static const color Brown2;
        static const color Brown3;
        static const color Brown4;
        static const color Burlywood;
        static const color Burlywood1;
        static const color Burlywood2;
        static const color Burlywood3;
        static const color Burlywood4;
        static const color CadetBlue;
        static const color CadetBlue1;
        static const color CadetBlue2;
        static const color CadetBlue3;
        static const color CadetBlue4;
        static const color Chartreuse;
        static const color Chartreuse1;
        static const color Chartreuse2;
        static const color Chartreuse3;
        static const color Chartreuse4;
        static const color Chocolate;
        static const color Chocolate1;
        static const color Chocolate2;
        static const color Chocolate3;
        static const color Chocolate4;
        static const color Coral;
        static const color Coral1;
        static const color Coral2;
        static const color Coral3;
        static const color Coral4;
        static const color CornflowerBlue;
        static const color Cornsilk;
        static const color Cornsilk1;
        static const color Cornsilk2;
        static const color Cornsilk3;
        static const color Cornsilk4;
        static const color Cyan;
        static const color Cyan1;
        static const color Cyan2;
        static const color Cyan3;
        static const color Cyan4;
        static const color DarkBlue;
        static const color DarkCyan;
        static const color DarkGoldenrod;
        static const color DarkGoldenrod1;
        static const color DarkGoldenrod2;
        static const color DarkGoldenrod3;
        static const color DarkGoldenrod4;
        static const color DarkGray;
        static const color DarkGreen;
        static const color DarkGrey;
        static const color DarkKhaki;
        static const color DarkMagenta;
        static const color DarkOliveGreen;
        static const color DarkOliveGreen1;
        static const color DarkOliveGreen2;
        static const color DarkOliveGreen3;
        static const color DarkOliveGreen4;
        static const color DarkOrange;
        static const color DarkOrange1;
        static const color DarkOrange2;
        static const color DarkOrange3;
        static const color DarkOrange4;
        static const color DarkOrchid;
        static const color DarkOrchid1;
        static const color DarkOrchid2;
        static const color DarkOrchid3;
        static const color DarkOrchid4;
        static const color DarkRed;
        static const color DarkSalmon;
        static const color DarkSeaGreen;
        static const color DarkSeaGreen1;
        static const color DarkSeaGreen2;
        static const color DarkSeaGreen3;
        static const color DarkSeaGreen4;
        static const color DarkSlateBlue;
        static const color DarkSlateGray;
        static const color DarkSlateGray1;
        static const color DarkSlateGray2;
        static const color DarkSlateGray3;
        static const color DarkSlateGray4;
        static const color DarkSlateGrey;
        static const color DarkTurquoise;
        static const color DarkViolet;
        static const color DebianRed;
        static const color DeepPink;
        static const color DeepPink1;
        static const color DeepPink2;
        static const color DeepPink3;
        static const color DeepPink4;
        static const color DeepSkyBlue;
        static const color DeepSkyBlue1;
        static const color DeepSkyBlue2;
        static const color DeepSkyBlue3;
        static const color DeepSkyBlue4;
        static const color DimGray;
        static const color DimGrey;
        static const color DodgerBlue;
        static const color DodgerBlue1;
        static const color DodgerBlue2;
        static const color DodgerBlue3;
        static const color DodgerBlue4;
        static const color Firebrick;
        static const color Firebrick1;
        static const color Firebrick2;
        static const color Firebrick3;
        static const color Firebrick4;
        static const color FloralWhite;
        static const color ForestGreen;
        static const color Gainsboro;
        static const color GhostWhite;
        static const color Gold;
        static const color Gold1;
        static const color Gold2;
        static const color Gold3;
        static const color Gold4;
        static const color Goldenrod;
        static const color Goldenrod1;
        static const color Goldenrod2;
        static const color Goldenrod3;
        static const color Goldenrod4;
        static const color Gray;
        static const color Gray0;
        static const color Gray1;
        static const color Gray10;
        static const color Gray100;
        static const color Gray11;
        static const color Gray12;
        static const color Gray13;
        static const color Gray14;
        static const color Gray15;
        static const color Gray16;
        static const color Gray17;
        static const color Gray18;
        static const color Gray19;
        static const color Gray2;
        static const color Gray20;
        static const color Gray21;
        static const color Gray22;
        static const color Gray23;
        static const color Gray24;
        static const color Gray25;
        static const color Gray26;
        static const color Gray27;
        static const color Gray28;
        static const color Gray29;
        static const color Gray3;
        static const color Gray30;
        static const color Gray31;
        static const color Gray32;
        static const color Gray33;
        static const color Gray34;
        static const color Gray35;
        static const color Gray36;
        static const color Gray37;
        static const color Gray38;
        static const color Gray39;
        static const color Gray4;
        static const color Gray40;
        static const color Gray41;
        static const color Gray42;
        static const color Gray43;
        static const color Gray44;
        static const color Gray45;
        static const color Gray46;
        static const color Gray47;
        static const color Gray48;
        static const color Gray49;
        static const color Gray5;
        static const color Gray50;
        static const color Gray51;
        static const color Gray52;
        static const color Gray53;
        static const color Gray54;
        static const color Gray55;
        static const color Gray56;
        static const color Gray57;
        static const color Gray58;
        static const color Gray59;
        static const color Gray6;
        static const color Gray60;
        static const color Gray61;
        static const color Gray62;
        static const color Gray63;
        static const color Gray64;
        static const color Gray65;
        static const color Gray66;
        static const color Gray67;
        static const color Gray68;
        static const color Gray69;
        static const color Gray7;
        static const color Gray70;
        static const color Gray71;
        static const color Gray72;
        static const color Gray73;
        static const color Gray74;
        static const color Gray75;
        static const color Gray76;
        static const color Gray77;
        static const color Gray78;
        static const color Gray79;
        static const color Gray8;
        static const color Gray80;
        static const color Gray81;
        static const color Gray82;
        static const color Gray83;
        static const color Gray84;
        static const color Gray85;
        static const color Gray86;
        static const color Gray87;
        static const color Gray88;
        static const color Gray89;
        static const color Gray9;
        static const color Gray90;
        static const color Gray91;
        static const color Gray92;
        static const color Gray93;
        static const color Gray94;
        static const color Gray95;
        static const color Gray96;
        static const color Gray97;
        static const color Gray98;
        static const color Gray99;
        static const color Green;
        static const color Green1;
        static const color Green2;
        static const color Green3;
        static const color Green4;
        static const color GreenYellow;
        static const color Grey;
        static const color Grey0;
        static const color Grey1;
        static const color Grey10;
        static const color Grey100;
        static const color Grey11;
        static const color Grey12;
        static const color Grey13;
        static const color Grey14;
        static const color Grey15;
        static const color Grey16;
        static const color Grey17;
        static const color Grey18;
        static const color Grey19;
        static const color Grey2;
        static const color Grey20;
        static const color Grey21;
        static const color Grey22;
        static const color Grey23;
        static const color Grey24;
        static const color Grey25;
        static const color Grey26;
        static const color Grey27;
        static const color Grey28;
        static const color Grey29;
        static const color Grey3;
        static const color Grey30;
        static const color Grey31;
        static const color Grey32;
        static const color Grey33;
        static const color Grey34;
        static const color Grey35;
        static const color Grey36;
        static const color Grey37;
        static const color Grey38;
        static const color Grey39;
        static const color Grey4;
        static const color Grey40;
        static const color Grey41;
        static const color Grey42;
        static const color Grey43;
        static const color Grey44;
        static const color Grey45;
        static const color Grey46;
        static const color Grey47;
        static const color Grey48;
        static const color Grey49;
        static const color Grey5;
        static const color Grey50;
        static const color Grey51;
        static const color Grey52;
        static const color Grey53;
        static const color Grey54;
        static const color Grey55;
        static const color Grey56;
        static const color Grey57;
        static const color Grey58;
        static const color Grey59;
        static const color Grey6;
        static const color Grey60;
        static const color Grey61;
        static const color Grey62;
        static const color Grey63;
        static const color Grey64;
        static const color Grey65;
        static const color Grey66;
        static const color Grey67;
        static const color Grey68;
        static const color Grey69;
        static const color Grey7;
        static const color Grey70;
        static const color Grey71;
        static const color Grey72;
        static const color Grey73;
        static const color Grey74;
        static const color Grey75;
        static const color Grey76;
        static const color Grey77;
        static const color Grey78;
        static const color Grey79;
        static const color Grey8;
        static const color Grey80;
        static const color Grey81;
        static const color Grey82;
        static const color Grey83;
        static const color Grey84;
        static const color Grey85;
        static const color Grey86;
        static const color Grey87;
        static const color Grey88;
        static const color Grey89;
        static const color Grey9;
        static const color Grey90;
        static const color Grey91;
        static const color Grey92;
        static const color Grey93;
        static const color Grey94;
        static const color Grey95;
        static const color Grey96;
        static const color Grey97;
        static const color Grey98;
        static const color Grey99;
        static const color Honeydew;
        static const color Honeydew1;
        static const color Honeydew2;
        static const color Honeydew3;
        static const color Honeydew4;
        static const color HotPink;
        static const color HotPink1;
        static const color HotPink2;
        static const color HotPink3;
        static const color HotPink4;
        static const color IndianRed;
        static const color IndianRed1;
        static const color IndianRed2;
        static const color IndianRed3;
        static const color IndianRed4;
        static const color Ivory;
        static const color Ivory1;
        static const color Ivory2;
        static const color Ivory3;
        static const color Ivory4;
        static const color Khaki;
        static const color Khaki1;
        static const color Khaki2;
        static const color Khaki3;
        static const color Khaki4;
        static const color Lavender;
        static const color LavenderBlush;
        static const color LavenderBlush1;
        static const color LavenderBlush2;
        static const color LavenderBlush3;
        static const color LavenderBlush4;
        static const color LawnGreen;
        static const color LemonChiffon;
        static const color LemonChiffon1;
        static const color LemonChiffon2;
        static const color LemonChiffon3;
        static const color LemonChiffon4;
        static const color LightBlue;
        static const color LightBlue1;
        static const color LightBlue2;
        static const color LightBlue3;
        static const color LightBlue4;
        static const color LightCoral;
        static const color LightCyan;
        static const color LightCyan1;
        static const color LightCyan2;
        static const color LightCyan3;
        static const color LightCyan4;
        static const color LightGoldenrod;
        static const color LightGoldenrod1;
        static const color LightGoldenrod2;
        static const color LightGoldenrod3;
        static const color LightGoldenrod4;
        static const color LightGoldenrodYellow;
        static const color LightGray;
        static const color LightGreen;
        static const color LightGrey;
        static const color LightPink;
        static const color LightPink1;
        static const color LightPink2;
        static const color LightPink3;
        static const color LightPink4;
        static const color LightSalmon;
        static const color LightSalmon1;
        static const color LightSalmon2;
        static const color LightSalmon3;
        static const color LightSalmon4;
        static const color LightSeaGreen;
        static const color LightSkyBlue;
        static const color LightSkyBlue1;
        static const color LightSkyBlue2;
        static const color LightSkyBlue3;
        static const color LightSkyBlue4;
        static const color LightSlateBlue;
        static const color LightSlateGray;
        static const color LightSlateGrey;
        static const color LightSteelBlue;
        static const color LightSteelBlue1;
        static const color LightSteelBlue2;
        static const color LightSteelBlue3;
        static const color LightSteelBlue4;
        static const color LightYellow;
        static const color LightYellow1;
        static const color LightYellow2;
        static const color LightYellow3;
        static const color LightYellow4;
        static const color LimeGreen;
        static const color Linen;
        static const color Magenta;
        static const color Magenta1;
        static const color Magenta2;
        static const color Magenta3;
        static const color Magenta4;
        static const color Maroon;
        static const color Maroon1;
        static const color Maroon2;
        static const color Maroon3;
        static const color Maroon4;
        static const color MediumAquamarine;
        static const color MediumBlue;
        static const color MediumOrchid;
        static const color MediumOrchid1;
        static const color MediumOrchid2;
        static const color MediumOrchid3;
        static const color MediumOrchid4;
        static const color MediumPurple;
        static const color MediumPurple1;
        static const color MediumPurple2;
        static const color MediumPurple3;
        static const color MediumPurple4;
        static const color MediumSeaGreen;
        static const color MediumSlateBlue;
        static const color MediumSpringGreen;
        static const color MediumTurquoise;
        static const color MediumVioletRed;
        static const color MidnightBlue;
        static const color MintCream;
        static const color MistyRose;
        static const color MistyRose1;
        static const color MistyRose2;
        static const color MistyRose3;
        static const color MistyRose4;
        static const color Moccasin;
        static const color NavajoWhite;
        static const color NavajoWhite1;
        static const color NavajoWhite2;
        static const color NavajoWhite3;
        static const color NavajoWhite4;
        static const color Navy;
        static const color NavyBlue;
        static const color OldLace;
        static const color OliveDrab;
        static const color OliveDrab1;
        static const color OliveDrab2;
        static const color OliveDrab3;
        static const color OliveDrab4;
        static const color Orange;
        static const color Orange1;
        static const color Orange2;
        static const color Orange3;
        static const color Orange4;
        static const color OrangeRed;
        static const color OrangeRed1;
        static const color OrangeRed2;
        static const color OrangeRed3;
        static const color OrangeRed4;
        static const color Orchid;
        static const color Orchid1;
        static const color Orchid2;
        static const color Orchid3;
        static const color Orchid4;
        static const color PaleGoldenrod;
        static const color PaleGreen;
        static const color PaleGreen1;
        static const color PaleGreen2;
        static const color PaleGreen3;
        static const color PaleGreen4;
        static const color PaleTurquoise;
        static const color PaleTurquoise1;
        static const color PaleTurquoise2;
        static const color PaleTurquoise3;
        static const color PaleTurquoise4;
        static const color PaleVioletRed;
        static const color PaleVioletRed1;
        static const color PaleVioletRed2;
        static const color PaleVioletRed3;
        static const color PaleVioletRed4;
        static const color PapayaWhip;
        static const color PeachPuff;
        static const color PeachPuff1;
        static const color PeachPuff2;
        static const color PeachPuff3;
        static const color PeachPuff4;
        static const color Peru;
        static const color Pink;
        static const color Pink1;
        static const color Pink2;
        static const color Pink3;
        static const color Pink4;
        static const color Plum;
        static const color Plum1;
        static const color Plum2;
        static const color Plum3;
        static const color Plum4;
        static const color PowderBlue;
        static const color Purple;
        static const color Purple1;
        static const color Purple2;
        static const color Purple3;
        static const color Purple4;
        static const color Red;
        static const color Red1;
        static const color Red2;
        static const color Red3;
        static const color Red4;
        static const color RosyBrown;
        static const color RosyBrown1;
        static const color RosyBrown2;
        static const color RosyBrown3;
        static const color RosyBrown4;
        static const color RoyalBlue;
        static const color RoyalBlue1;
        static const color RoyalBlue2;
        static const color RoyalBlue3;
        static const color RoyalBlue4;
        static const color SaddleBrown;
        static const color Salmon;
        static const color Salmon1;
        static const color Salmon2;
        static const color Salmon3;
        static const color Salmon4;
        static const color SandyBrown;
        static const color SeaGreen;
        static const color SeaGreen1;
        static const color SeaGreen2;
        static const color SeaGreen3;
        static const color SeaGreen4;
        static const color Seashell;
        static const color Seashell1;
        static const color Seashell2;
        static const color Seashell3;
        static const color Seashell4;
        static const color Sienna;
        static const color Sienna1;
        static const color Sienna2;
        static const color Sienna3;
        static const color Sienna4;
        static const color SkyBlue;
        static const color SkyBlue1;
        static const color SkyBlue2;
        static const color SkyBlue3;
        static const color SkyBlue4;
        static const color SlateBlue;
        static const color SlateBlue1;
        static const color SlateBlue2;
        static const color SlateBlue3;
        static const color SlateBlue4;
        static const color SlateGray;
        static const color SlateGray1;
        static const color SlateGray2;
        static const color SlateGray3;
        static const color SlateGray4;
        static const color SlateGrey;
        static const color Snow;
        static const color Snow1;
        static const color Snow2;
        static const color Snow3;
        static const color Snow4;
        static const color SpringGreen;
        static const color SpringGreen1;
        static const color SpringGreen2;
        static const color SpringGreen3;
        static const color SpringGreen4;
        static const color SteelBlue;
        static const color SteelBlue1;
        static const color SteelBlue2;
        static const color SteelBlue3;
        static const color SteelBlue4;
        static const color Tan;
        static const color Tan1;
        static const color Tan2;
        static const color Tan3;
        static const color Tan4;
        static const color Thistle;
        static const color Thistle1;
        static const color Thistle2;
        static const color Thistle3;
        static const color Thistle4;
        static const color Tomato;
        static const color Tomato1;
        static const color Tomato2;
        static const color Tomato3;
        static const color Tomato4;
        static const color Turquoise;
        static const color Turquoise1;
        static const color Turquoise2;
        static const color Turquoise3;
        static const color Turquoise4;
        static const color Violet;
        static const color VioletRed;
        static const color VioletRed1;
        static const color VioletRed2;
        static const color VioletRed3;
        static const color VioletRed4;
        static const color Wheat;
        static const color Wheat1;
        static const color Wheat2;
        static const color Wheat3;
        static const color Wheat4;
        static const color White;
        static const color WhiteSmoke;
        static const color Yellow;
        static const color Yellow1;
        static const color Yellow2;
        static const color Yellow3;
        static const color Yellow4;
        static const color YellowGreen;
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
        color();
        color(const color& aOther);
        explicit color(argb aValue);
        color(component aRed, component aGreen, component aBlue, component aAlpha = 0xFF);
        template <typename T>
        color(T aRed, T aGreen, T aBlue, T aAlpha = static_cast<T>(0xFF), typename std::enable_if<std::is_integral<T>::value, void>::type* = nullptr) :
            color(static_cast<component>(aRed), static_cast<component>(aGreen), static_cast<component>(aBlue), static_cast<component>(aAlpha))
        {
        }
        color(const vec4& aValue);
        color(const vec4f& aValue);
        color(const std::string& aTextValue);
        // operations
    public:
        static color from_hsl(double aHue, double aSaturation, double aLightness, double aAlpha = 1.0);
        static color from_hsv(double aHue, double aSaturation, double aValue, double aAlpha = 1.0);
        static std::optional<color> from_name(const std::string& aName);
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
        color& set_alpha(component aNewValue);
        color& set_red(component aNewValue);
        color& set_green(component aNewValue);
        color& set_blue(component aNewValue);
        color with_alpha(component aNewValue) const;
        color with_red(component aNewValue) const;
        color with_green(component aNewValue) const;
        color with_blue(component aNewValue) const;
        color with_combined_alpha(component aNewValue) const;
        color with_combined_alpha(double aCoefficient) const;
        hsl_color to_hsl() const;
        hsv_color to_hsv() const;
        double brightness() const;
        double intensity() const;
        double luma() const;
        bool similar_intensity(const color& aOther, double aThreshold = 0.5);
        color mid(const color& aOther) const;
        bool light(double aThreshold = 0.50) const;
        bool dark(double aThreshold = 0.50) const;
        color& lighten(component aDelta);
        color& darken(component aDelta);
        color lighter(component aDelta) const;
        color darker(component aDelta) const;
        color shade(component aDelta) const;
        color monochrome() const;
        color same_lightness_as(const color& aOther) const;
        color with_lightness(double aLightness) const;
        color inverse() const;
        color& operator+=(component aDelta);
        color& operator-=(component aDelta);
        color operator~() const;
        bool operator==(const color& aOther) const;
        bool operator!=(const color& aOther) const;
        bool operator<(const color& aOther) const;
        std::string to_string() const;
        std::string to_hex_string() const;
        vec4 to_vec4() const;
        vec4f to_vec4f() const;
        // attributes
    private:
        argb iValue;
    };

    inline color operator+(const color& aLeft, color::component aRight)
    {
        color ret = aLeft;
        ret += aRight;
        return ret;
    }

    inline color operator-(const color& aLeft, color::component aRight)
    {
        color ret = aLeft;
        ret -= aRight;
        return ret;
    }

    inline color operator*(const color& aLeft, double aCoefficient)
    {
        return aLeft.to_vec4() *= vec4 { aCoefficient, aCoefficient, aCoefficient, 1.0 };
    }

    template <typename Elem, typename Traits>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const color& aColor)
    {
        aStream << "[A: 0x" << std::hex << (int)aColor.alpha() <<
            ", R: 0x" << std::hex << (int)aColor.red() <<
            ", G: 0x" << std::hex << (int)aColor.green() <<
            ", B: 0x" << std::hex << (int)aColor.blue() << "]";
        return aStream;
    }

    enum class gradient_direction : uint32_t
    {
        Vertical,
        Horizontal,
        Diagonal,
        Rectangular,
        Radial
    };

    enum class gradient_shape : uint32_t
    {
        Ellipse,
        Circle
    };

    enum class gradient_size : uint32_t
    {
        ClosestSide,
        FarthestSide,
        ClosestCorner,
        FarthestCorner
    };
}

template <>
const neolib::enum_enumerators_t<neogfx::gradient_direction> neolib::enum_enumerators_v<neogfx::gradient_direction>
{
    declare_enum_string(neogfx::gradient_direction, Vertical)
    declare_enum_string(neogfx::gradient_direction, Horizontal)
    declare_enum_string(neogfx::gradient_direction, Diagonal)
    declare_enum_string(neogfx::gradient_direction, Rectangular)
    declare_enum_string(neogfx::gradient_direction, Radial)
};

template <>
const neolib::enum_enumerators_t<neogfx::gradient_shape> neolib::enum_enumerators_v<neogfx::gradient_shape>
{
    declare_enum_string(neogfx::gradient_shape, Ellipse)
    declare_enum_string(neogfx::gradient_shape, Circle)
};

template <>
const neolib::enum_enumerators_t<neogfx::gradient_size> neolib::enum_enumerators_v<neogfx::gradient_size>
{
    declare_enum_string(neogfx::gradient_size, ClosestSide)
    declare_enum_string(neogfx::gradient_size, FarthestSide)
    declare_enum_string(neogfx::gradient_size, ClosestCorner)
    declare_enum_string(neogfx::gradient_size, FarthestCorner)
};

namespace neogfx
{
    class gradient
    {
        // constants
    public:
        static const std::uint32_t MaxStops = 256;
        // types
    public:
        typedef neolib::variant<corner, double> orientation_type;
        typedef std::pair<double, color> color_stop;
        typedef std::vector<color_stop> color_stop_list;
        typedef std::pair<double, color::component> alpha_stop;
        typedef std::vector<alpha_stop> alpha_stop_list;
    public:
        struct bad_position : std::logic_error { bad_position() : std::logic_error("neogfx::gradient::bad_position") {} };
        // construction
    public:
        gradient();
        gradient(const gradient& aOther);
        explicit gradient(const color& aColor);
        gradient(const color& aColor, gradient_direction aDirection);
        gradient(const color& aColor1, const color& aColor2, gradient_direction aDirection = gradient_direction::Vertical);
        gradient(const color_stop_list& aColorStops, gradient_direction aDirection = gradient_direction::Vertical);
        gradient(const color_stop_list& aColorStops, const alpha_stop_list& aAlphaStops, gradient_direction aDirection = gradient_direction::Vertical);
        gradient(const std::initializer_list<color>& aColors, gradient_direction aDirection = gradient_direction::Vertical);
        // operations
    public:
        bool use_cache() const;
        void set_cache_usage(bool aUseCache);
        color_stop_list::const_iterator color_begin() const;
        color_stop_list::const_iterator color_end() const;
        alpha_stop_list::const_iterator alpha_begin() const;
        alpha_stop_list::const_iterator alpha_end() const;
        color_stop_list::iterator color_begin();
        color_stop_list::iterator color_end();
        alpha_stop_list::iterator alpha_begin();
        alpha_stop_list::iterator alpha_end();
        color_stop_list::iterator find_color_stop(double aPos, bool aToInsert = false);
        color_stop_list::iterator find_color_stop(double aPos, double aStart, double aEnd, bool aToInsert = false);
        alpha_stop_list::iterator find_alpha_stop(double aPos, bool aToInsert = false);
        alpha_stop_list::iterator find_alpha_stop(double aPos, double aStart, double aEnd, bool aToInsert = false);
        color_stop_list::iterator insert_color_stop(double aPos);
        color_stop_list::iterator insert_color_stop(double aPos, double aStart, double aEnd);
        alpha_stop_list::iterator insert_alpha_stop(double aPos);
        alpha_stop_list::iterator insert_alpha_stop(double aPos, double aStart, double aEnd);
        void erase_stop(color_stop_list::iterator aStop);
        void erase_stop(alpha_stop_list::iterator aStop);
        std::size_t color_stop_count() const;
        std::size_t alpha_stop_count() const;
        color_stop_list combined_stops() const;
        color at(double aPos) const;
        color at(double aPos, double aStart, double aEnd) const;
        color color_at(double aPos) const;
        color color_at(double aPos, double aStart, double aEnd) const;
        color::component alpha_at(double aPos) const;
        color::component alpha_at(double aPos, double aStart, double aEnd) const;
        gradient with_alpha(color::component aAlpha) const;
        gradient with_combined_alpha(color::component aAlpha) const;
        gradient_direction direction() const;
        void set_direction(gradient_direction aDirection);
        gradient with_direction(gradient_direction aDirection) const;
        orientation_type orientation() const;
        void set_orientation(orientation_type aOrientation);
        gradient with_orientation(orientation_type aOrientation) const;
        gradient_shape shape() const;
        void set_shape(gradient_shape aShape);
        gradient with_shape(gradient_shape aShape) const;
        gradient_size size() const;
        void set_size(gradient_size aSize);
        gradient with_size(gradient_size aSize) const;
        const optional_vec2& exponents() const;
        void set_exponents(const optional_vec2& aExponents);
        gradient with_exponents(const optional_vec2& aExponents) const;
        const optional_point& centre() const;
        void set_centre(const optional_point& aCentre);
        gradient with_centre(const optional_point& aCentre) const;
        double smoothness() const;
        void set_smoothness(double aSmoothness);
        gradient with_smoothness(double aSmoothness) const;
    public:
        bool operator==(const gradient& aOther) const;
        bool operator!=(const gradient& aOther) const;
        bool operator<(const gradient& aOther) const;
    public:
        static double normalized_position(double aPos, double aStart, double aEnd);
    private:
        const color_stop_list& color_stops() const;
        color_stop_list& color_stops();
        const alpha_stop_list& alpha_stops() const;
        alpha_stop_list& alpha_stops();
        void fix();
        // attributes
    private:
        bool iUseCache;
        color_stop_list iColorStops;
        alpha_stop_list iAlphaStops;
        gradient_direction iDirection;
        orientation_type iOrientation;
        gradient_shape iShape;
        gradient_size iSize;
        optional_vec2 iExponents;
        optional_point iCentre;
        double iSmoothness;
    };

    typedef std::optional<color> optional_color;
    typedef std::optional<gradient> optional_gradient;

    typedef neolib::variant<color, gradient> color_or_gradient;
    typedef std::optional<color_or_gradient> optional_color_or_gradient;
}