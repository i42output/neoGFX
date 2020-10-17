// color.cpp
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
#include <algorithm>
#include <tuple>
#include <iomanip>
#include <boost/lexical_cast.hpp>
#include <neolib/core/vecarray.hpp>
#include <neolib/core/string_ci.hpp>
#include <neogfx/core/numerical.hpp>
#include <neogfx/gfx/color.hpp>

namespace neogfx
{
    scalar sRGB_to_linear(scalar s, scalar scale)
    {
        s /= scale;
        if (s <= 0.04045)
            return s / 12.92 * scale;
        else
            return std::pow(((s + 0.055) / 1.055), 2.4) * scale;
    }

    scalar linear_to_sRGB(scalar l, scalar scale)
    {
        l /= scale;
        if (l <= 0.0031308)
            return l * 12.92 * scale;
        else
            return std::pow(l, 1 / 2.4) * 1.055 - 0.055 * scale;
    }

    sRGB_color sRGB_color::from_linear(const linear_color& aLinear)
    {
        return sRGB_color{ linear_to_sRGB(aLinear.x), linear_to_sRGB(aLinear.y), linear_to_sRGB(aLinear.z), aLinear[3] };
    }

    sRGB_color::sRGB_color() : 
        base_type{}
    {
    }

    sRGB_color::sRGB_color(const sRGB_color& aOther) : 
        base_type{ aOther }
    {
    }

    sRGB_color::sRGB_color(const base_type& aOther) :
        base_type{ aOther }
    {
    }

    sRGB_color::sRGB_color(const linear_color& aLinear) :
        base_type{ from_linear(aLinear) }
    {
    }

    sRGB_color::sRGB_color(std::string const& aTextValue) : 
        base_type{}
    {
        if (aTextValue.empty())
            *this = Black;
        else if (aTextValue[0] == L'#')
        {
            *this = from_argb(neolib::string_to_uint32(aTextValue.substr(1), 16));
            if (aTextValue.size() <= 7)
                set_alpha(0xFF);
            else
            {
                auto actualAlpha = blue();
                *this = from_argb(as_argb() >> GreenShift).with_alpha(actualAlpha);
            }
        }
        else
        {
            auto namedColor = from_name(aTextValue);
            if (namedColor != std::nullopt)
                *this = *namedColor;
            else
            {
                /* todo: parse rgba(x, y, z); */
                neolib::vecarray<std::string, 3> bits;
                neolib::tokens(aTextValue, std::string("()"), bits, 3, false);
                try
                {
                    if ((bits.size() == 3 && bits[2] == ";") || bits.size() == 2)
                    {
                        if (bits[0] == "rgb")
                        {
                            neolib::vecarray<std::string, 3> moreBits;
                            neolib::tokens(bits[1], std::string(","), moreBits, 3, false);
                            if (moreBits.size() == 3)
                            {
                                *this = sRGB_color{
                                    boost::lexical_cast<uint32_t>(moreBits[0]),
                                    boost::lexical_cast<uint32_t>(moreBits[1]),
                                    boost::lexical_cast<uint32_t>(moreBits[2]) };
                            }
                        }
                        else if (bits[0] == "rgba")
                        {
                            neolib::vecarray<std::string, 4> moreBits;
                            neolib::tokens(bits[1], std::string(","), moreBits, 4, false);
                            if (moreBits.size() == 4)
                            {
                                *this = sRGB_color{
                                    boost::lexical_cast<uint32_t>(moreBits[0]),
                                    boost::lexical_cast<uint32_t>(moreBits[1]),
                                    boost::lexical_cast<uint32_t>(moreBits[2]),
                                    static_cast<uint32_t>(boost::lexical_cast<double>(moreBits[3]) * 255.0) };
                            }
                        }
                        else
                            *this = Black;
                    }
                    else
                        *this = Black;
                }
                catch (...)
                {
                    *this = Black;
                }
            }
        }
    }

    sRGB_color& sRGB_color::operator=(const sRGB_color& aOther)
    {
        base_type::operator=(aOther);
        return *this;
    }

    sRGB_color& sRGB_color::operator=(const base_type& aOther)
    {
        base_type::operator=(aOther);
        return *this;
    }

    linear_color sRGB_color::to_linear() const
    {
        return linear_color{ sRGB_to_linear(red<scalar>()), sRGB_to_linear(green<scalar>()), sRGB_to_linear(blue<scalar>()), alpha<scalar>() };
    }

    double sRGB_color::brightness() const
    {
        return std::min(1.0, std::max(0.0, std::sqrt(red<double>() * red<double>() * 0.241 + green<double>() * green<double>() * 0.691 + blue<double>() * blue<double>() * 0.068)));
    }

    double sRGB_color::luma() const
    {
        return red<double>() * 0.21 + green<double>() * 0.72 + blue<double>() * 0.07;
    }

    const sRGB_color sRGB_color::AliceBlue = sRGB_color{ 0xF0, 0xF8, 0xFF };
    const sRGB_color sRGB_color::AntiqueWhite = sRGB_color{ 0xFA, 0xEB, 0xD7 };
    const sRGB_color sRGB_color::AntiqueWhite1 = sRGB_color{ 0xFF, 0xEF, 0xDB };
    const sRGB_color sRGB_color::AntiqueWhite2 = sRGB_color{ 0xEE, 0xDF, 0xCC };
    const sRGB_color sRGB_color::AntiqueWhite3 = sRGB_color{ 0xCD, 0xC0, 0xB0 };
    const sRGB_color sRGB_color::AntiqueWhite4 = sRGB_color{ 0x8B, 0x83, 0x78 };
    const sRGB_color sRGB_color::Aquamarine = sRGB_color{ 0x7F, 0xFF, 0xD4 };
    const sRGB_color sRGB_color::Aquamarine1 = sRGB_color{ 0x7F, 0xFF, 0xD4 };
    const sRGB_color sRGB_color::Aquamarine2 = sRGB_color{ 0x76, 0xEE, 0xC6 };
    const sRGB_color sRGB_color::Aquamarine3 = sRGB_color{ 0x66, 0xCD, 0xAA };
    const sRGB_color sRGB_color::Aquamarine4 = sRGB_color{ 0x45, 0x8B, 0x74 };
    const sRGB_color sRGB_color::Azure = sRGB_color{ 0xF0, 0xFF, 0xFF };
    const sRGB_color sRGB_color::Azure1 = sRGB_color{ 0xF0, 0xFF, 0xFF };
    const sRGB_color sRGB_color::Azure2 = sRGB_color{ 0xE0, 0xEE, 0xEE };
    const sRGB_color sRGB_color::Azure3 = sRGB_color{ 0xC1, 0xCD, 0xCD };
    const sRGB_color sRGB_color::Azure4 = sRGB_color{ 0x83, 0x8B, 0x8B };
    const sRGB_color sRGB_color::Beige = sRGB_color{ 0xF5, 0xF5, 0xDC };
    const sRGB_color sRGB_color::Bisque = sRGB_color{ 0xFF, 0xE4, 0xC4 };
    const sRGB_color sRGB_color::Bisque1 = sRGB_color{ 0xFF, 0xE4, 0xC4 };
    const sRGB_color sRGB_color::Bisque2 = sRGB_color{ 0xEE, 0xD5, 0xB7 };
    const sRGB_color sRGB_color::Bisque3 = sRGB_color{ 0xCD, 0xB7, 0x9E };
    const sRGB_color sRGB_color::Bisque4 = sRGB_color{ 0x8B, 0x7D, 0x6B };
    const sRGB_color sRGB_color::Black = sRGB_color{ 0x00, 0x00, 0x00 };
    const sRGB_color sRGB_color::BlanchedAlmond = sRGB_color{ 0xFF, 0xEB, 0xCD };
    const sRGB_color sRGB_color::Blue = sRGB_color{ 0x00, 0x00, 0xFF };
    const sRGB_color sRGB_color::Blue1 = sRGB_color{ 0x00, 0x00, 0xFF };
    const sRGB_color sRGB_color::Blue2 = sRGB_color{ 0x00, 0x00, 0xEE };
    const sRGB_color sRGB_color::Blue3 = sRGB_color{ 0x00, 0x00, 0xCD };
    const sRGB_color sRGB_color::Blue4 = sRGB_color{ 0x00, 0x00, 0x8B };
    const sRGB_color sRGB_color::BlueViolet = sRGB_color{ 0x8A, 0x2B, 0xE2 };
    const sRGB_color sRGB_color::Brown = sRGB_color{ 0xA5, 0x2A, 0x2A };
    const sRGB_color sRGB_color::Brown1 = sRGB_color{ 0xFF, 0x40, 0x40 };
    const sRGB_color sRGB_color::Brown2 = sRGB_color{ 0xEE, 0x3B, 0x3B };
    const sRGB_color sRGB_color::Brown3 = sRGB_color{ 0xCD, 0x33, 0x33 };
    const sRGB_color sRGB_color::Brown4 = sRGB_color{ 0x8B, 0x23, 0x23 };
    const sRGB_color sRGB_color::Burlywood = sRGB_color{ 0xDE, 0xB8, 0x87 };
    const sRGB_color sRGB_color::Burlywood1 = sRGB_color{ 0xFF, 0xD3, 0x9B };
    const sRGB_color sRGB_color::Burlywood2 = sRGB_color{ 0xEE, 0xC5, 0x91 };
    const sRGB_color sRGB_color::Burlywood3 = sRGB_color{ 0xCD, 0xAA, 0x7D };
    const sRGB_color sRGB_color::Burlywood4 = sRGB_color{ 0x8B, 0x73, 0x55 };
    const sRGB_color sRGB_color::CadetBlue = sRGB_color{ 0x5F, 0x9E, 0xA0 };
    const sRGB_color sRGB_color::CadetBlue1 = sRGB_color{ 0x98, 0xF5, 0xFF };
    const sRGB_color sRGB_color::CadetBlue2 = sRGB_color{ 0x8E, 0xE5, 0xEE };
    const sRGB_color sRGB_color::CadetBlue3 = sRGB_color{ 0x7A, 0xC5, 0xCD };
    const sRGB_color sRGB_color::CadetBlue4 = sRGB_color{ 0x53, 0x86, 0x8B };
    const sRGB_color sRGB_color::Chartreuse = sRGB_color{ 0x7F, 0xFF, 0x00 };
    const sRGB_color sRGB_color::Chartreuse1 = sRGB_color{ 0x7F, 0xFF, 0x00 };
    const sRGB_color sRGB_color::Chartreuse2 = sRGB_color{ 0x76, 0xEE, 0x00 };
    const sRGB_color sRGB_color::Chartreuse3 = sRGB_color{ 0x66, 0xCD, 0x00 };
    const sRGB_color sRGB_color::Chartreuse4 = sRGB_color{ 0x45, 0x8B, 0x00 };
    const sRGB_color sRGB_color::Chocolate = sRGB_color{ 0xD2, 0x69, 0x1E };
    const sRGB_color sRGB_color::Chocolate1 = sRGB_color{ 0xFF, 0x7F, 0x24 };
    const sRGB_color sRGB_color::Chocolate2 = sRGB_color{ 0xEE, 0x76, 0x21 };
    const sRGB_color sRGB_color::Chocolate3 = sRGB_color{ 0xCD, 0x66, 0x1D };
    const sRGB_color sRGB_color::Chocolate4 = sRGB_color{ 0x8B, 0x45, 0x13 };
    const sRGB_color sRGB_color::Coral = sRGB_color{ 0xFF, 0x7F, 0x50 };
    const sRGB_color sRGB_color::Coral1 = sRGB_color{ 0xFF, 0x72, 0x56 };
    const sRGB_color sRGB_color::Coral2 = sRGB_color{ 0xEE, 0x6A, 0x50 };
    const sRGB_color sRGB_color::Coral3 = sRGB_color{ 0xCD, 0x5B, 0x45 };
    const sRGB_color sRGB_color::Coral4 = sRGB_color{ 0x8B, 0x3E, 0x2F };
    const sRGB_color sRGB_color::CornflowerBlue = sRGB_color{ 0x64, 0x95, 0xED };
    const sRGB_color sRGB_color::Cornsilk = sRGB_color{ 0xFF, 0xF8, 0xDC };
    const sRGB_color sRGB_color::Cornsilk1 = sRGB_color{ 0xFF, 0xF8, 0xDC };
    const sRGB_color sRGB_color::Cornsilk2 = sRGB_color{ 0xEE, 0xE8, 0xCD };
    const sRGB_color sRGB_color::Cornsilk3 = sRGB_color{ 0xCD, 0xC8, 0xB1 };
    const sRGB_color sRGB_color::Cornsilk4 = sRGB_color{ 0x8B, 0x88, 0x78 };
    const sRGB_color sRGB_color::Cyan = sRGB_color{ 0x00, 0xFF, 0xFF };
    const sRGB_color sRGB_color::Cyan1 = sRGB_color{ 0x00, 0xFF, 0xFF };
    const sRGB_color sRGB_color::Cyan2 = sRGB_color{ 0x00, 0xEE, 0xEE };
    const sRGB_color sRGB_color::Cyan3 = sRGB_color{ 0x00, 0xCD, 0xCD };
    const sRGB_color sRGB_color::Cyan4 = sRGB_color{ 0x00, 0x8B, 0x8B };
    const sRGB_color sRGB_color::DarkBlue = sRGB_color{ 0x00, 0x00, 0x8B };
    const sRGB_color sRGB_color::DarkCyan = sRGB_color{ 0x00, 0x8B, 0x8B };
    const sRGB_color sRGB_color::DarkGoldenrod = sRGB_color{ 0xB8, 0x86, 0x0B };
    const sRGB_color sRGB_color::DarkGoldenrod1 = sRGB_color{ 0xFF, 0xB9, 0x0F };
    const sRGB_color sRGB_color::DarkGoldenrod2 = sRGB_color{ 0xEE, 0xAD, 0x0E };
    const sRGB_color sRGB_color::DarkGoldenrod3 = sRGB_color{ 0xCD, 0x95, 0x0C };
    const sRGB_color sRGB_color::DarkGoldenrod4 = sRGB_color{ 0x8B, 0x65, 0x08 };
    const sRGB_color sRGB_color::DarkGray = sRGB_color{ 0xA9, 0xA9, 0xA9 };
    const sRGB_color sRGB_color::DarkGreen = sRGB_color{ 0x00, 0x64, 0x00 };
    const sRGB_color sRGB_color::DarkGrey = sRGB_color{ 0xA9, 0xA9, 0xA9 };
    const sRGB_color sRGB_color::DarkKhaki = sRGB_color{ 0xBD, 0xB7, 0x6B };
    const sRGB_color sRGB_color::DarkMagenta = sRGB_color{ 0x8B, 0x00, 0x8B };
    const sRGB_color sRGB_color::DarkOliveGreen = sRGB_color{ 0x55, 0x6B, 0x2F };
    const sRGB_color sRGB_color::DarkOliveGreen1 = sRGB_color{ 0xCA, 0xFF, 0x70 };
    const sRGB_color sRGB_color::DarkOliveGreen2 = sRGB_color{ 0xBC, 0xEE, 0x68 };
    const sRGB_color sRGB_color::DarkOliveGreen3 = sRGB_color{ 0xA2, 0xCD, 0x5A };
    const sRGB_color sRGB_color::DarkOliveGreen4 = sRGB_color{ 0x6E, 0x8B, 0x3D };
    const sRGB_color sRGB_color::DarkOrange = sRGB_color{ 0xFF, 0x8C, 0x00 };
    const sRGB_color sRGB_color::DarkOrange1 = sRGB_color{ 0xFF, 0x7F, 0x00 };
    const sRGB_color sRGB_color::DarkOrange2 = sRGB_color{ 0xEE, 0x76, 0x00 };
    const sRGB_color sRGB_color::DarkOrange3 = sRGB_color{ 0xCD, 0x66, 0x00 };
    const sRGB_color sRGB_color::DarkOrange4 = sRGB_color{ 0x8B, 0x45, 0x00 };
    const sRGB_color sRGB_color::DarkOrchid = sRGB_color{ 0x99, 0x32, 0xCC };
    const sRGB_color sRGB_color::DarkOrchid1 = sRGB_color{ 0xBF, 0x3E, 0xFF };
    const sRGB_color sRGB_color::DarkOrchid2 = sRGB_color{ 0xB2, 0x3A, 0xEE };
    const sRGB_color sRGB_color::DarkOrchid3 = sRGB_color{ 0x9A, 0x32, 0xCD };
    const sRGB_color sRGB_color::DarkOrchid4 = sRGB_color{ 0x68, 0x22, 0x8B };
    const sRGB_color sRGB_color::DarkRed = sRGB_color{ 0x8B, 0x00, 0x00 };
    const sRGB_color sRGB_color::DarkSalmon = sRGB_color{ 0xE9, 0x96, 0x7A };
    const sRGB_color sRGB_color::DarkSeaGreen = sRGB_color{ 0x8F, 0xBC, 0x8F };
    const sRGB_color sRGB_color::DarkSeaGreen1 = sRGB_color{ 0xC1, 0xFF, 0xC1 };
    const sRGB_color sRGB_color::DarkSeaGreen2 = sRGB_color{ 0xB4, 0xEE, 0xB4 };
    const sRGB_color sRGB_color::DarkSeaGreen3 = sRGB_color{ 0x9B, 0xCD, 0x9B };
    const sRGB_color sRGB_color::DarkSeaGreen4 = sRGB_color{ 0x69, 0x8B, 0x69 };
    const sRGB_color sRGB_color::DarkSlateBlue = sRGB_color{ 0x48, 0x3D, 0x8B };
    const sRGB_color sRGB_color::DarkSlateGray = sRGB_color{ 0x2F, 0x4F, 0x4F };
    const sRGB_color sRGB_color::DarkSlateGray1 = sRGB_color{ 0x97, 0xFF, 0xFF };
    const sRGB_color sRGB_color::DarkSlateGray2 = sRGB_color{ 0x8D, 0xEE, 0xEE };
    const sRGB_color sRGB_color::DarkSlateGray3 = sRGB_color{ 0x79, 0xCD, 0xCD };
    const sRGB_color sRGB_color::DarkSlateGray4 = sRGB_color{ 0x52, 0x8B, 0x8B };
    const sRGB_color sRGB_color::DarkSlateGrey = sRGB_color{ 0x2F, 0x4F, 0x4F };
    const sRGB_color sRGB_color::DarkTurquoise = sRGB_color{ 0x00, 0xCE, 0xD1 };
    const sRGB_color sRGB_color::DarkViolet = sRGB_color{ 0x94, 0x00, 0xD3 };
    const sRGB_color sRGB_color::DebianRed = sRGB_color{ 0xD7, 0x07, 0x51 };
    const sRGB_color sRGB_color::DeepPink = sRGB_color{ 0xFF, 0x14, 0x93 };
    const sRGB_color sRGB_color::DeepPink1 = sRGB_color{ 0xFF, 0x14, 0x93 };
    const sRGB_color sRGB_color::DeepPink2 = sRGB_color{ 0xEE, 0x12, 0x89 };
    const sRGB_color sRGB_color::DeepPink3 = sRGB_color{ 0xCD, 0x10, 0x76 };
    const sRGB_color sRGB_color::DeepPink4 = sRGB_color{ 0x8B, 0x0A, 0x50 };
    const sRGB_color sRGB_color::DeepSkyBlue = sRGB_color{ 0x00, 0xBF, 0xFF };
    const sRGB_color sRGB_color::DeepSkyBlue1 = sRGB_color{ 0x00, 0xBF, 0xFF };
    const sRGB_color sRGB_color::DeepSkyBlue2 = sRGB_color{ 0x00, 0xB2, 0xEE };
    const sRGB_color sRGB_color::DeepSkyBlue3 = sRGB_color{ 0x00, 0x9A, 0xCD };
    const sRGB_color sRGB_color::DeepSkyBlue4 = sRGB_color{ 0x00, 0x68, 0x8B };
    const sRGB_color sRGB_color::DimGray = sRGB_color{ 0x69, 0x69, 0x69 };
    const sRGB_color sRGB_color::DimGrey = sRGB_color{ 0x69, 0x69, 0x69 };
    const sRGB_color sRGB_color::DodgerBlue = sRGB_color{ 0x1E, 0x90, 0xFF };
    const sRGB_color sRGB_color::DodgerBlue1 = sRGB_color{ 0x1E, 0x90, 0xFF };
    const sRGB_color sRGB_color::DodgerBlue2 = sRGB_color{ 0x1C, 0x86, 0xEE };
    const sRGB_color sRGB_color::DodgerBlue3 = sRGB_color{ 0x18, 0x74, 0xCD };
    const sRGB_color sRGB_color::DodgerBlue4 = sRGB_color{ 0x10, 0x4E, 0x8B };
    const sRGB_color sRGB_color::Firebrick = sRGB_color{ 0xB2, 0x22, 0x22 };
    const sRGB_color sRGB_color::Firebrick1 = sRGB_color{ 0xFF, 0x30, 0x30 };
    const sRGB_color sRGB_color::Firebrick2 = sRGB_color{ 0xEE, 0x2C, 0x2C };
    const sRGB_color sRGB_color::Firebrick3 = sRGB_color{ 0xCD, 0x26, 0x26 };
    const sRGB_color sRGB_color::Firebrick4 = sRGB_color{ 0x8B, 0x1A, 0x1A };
    const sRGB_color sRGB_color::FloralWhite = sRGB_color{ 0xFF, 0xFA, 0xF0 };
    const sRGB_color sRGB_color::ForestGreen = sRGB_color{ 0x22, 0x8B, 0x22 };
    const sRGB_color sRGB_color::Gainsboro = sRGB_color{ 0xDC, 0xDC, 0xDC };
    const sRGB_color sRGB_color::GhostWhite = sRGB_color{ 0xF8, 0xF8, 0xFF };
    const sRGB_color sRGB_color::Gold = sRGB_color{ 0xFF, 0xD7, 0x00 };
    const sRGB_color sRGB_color::Gold1 = sRGB_color{ 0xFF, 0xD7, 0x00 };
    const sRGB_color sRGB_color::Gold2 = sRGB_color{ 0xEE, 0xC9, 0x00 };
    const sRGB_color sRGB_color::Gold3 = sRGB_color{ 0xCD, 0xAD, 0x00 };
    const sRGB_color sRGB_color::Gold4 = sRGB_color{ 0x8B, 0x75, 0x00 };
    const sRGB_color sRGB_color::Goldenrod = sRGB_color{ 0xDA, 0xA5, 0x20 };
    const sRGB_color sRGB_color::Goldenrod1 = sRGB_color{ 0xFF, 0xC1, 0x25 };
    const sRGB_color sRGB_color::Goldenrod2 = sRGB_color{ 0xEE, 0xB4, 0x22 };
    const sRGB_color sRGB_color::Goldenrod3 = sRGB_color{ 0xCD, 0x9B, 0x1D };
    const sRGB_color sRGB_color::Goldenrod4 = sRGB_color{ 0x8B, 0x69, 0x14 };
    const sRGB_color sRGB_color::Gray = sRGB_color{ 0xBE, 0xBE, 0xBE };
    const sRGB_color sRGB_color::Gray0 = sRGB_color{ 0x00, 0x00, 0x00 };
    const sRGB_color sRGB_color::Gray1 = sRGB_color{ 0x03, 0x03, 0x03 };
    const sRGB_color sRGB_color::Gray10 = sRGB_color{ 0x1A, 0x1A, 0x1A };
    const sRGB_color sRGB_color::Gray100 = sRGB_color{ 0xFF, 0xFF, 0xFF };
    const sRGB_color sRGB_color::Gray11 = sRGB_color{ 0x1C, 0x1C, 0x1C };
    const sRGB_color sRGB_color::Gray12 = sRGB_color{ 0x1F, 0x1F, 0x1F };
    const sRGB_color sRGB_color::Gray13 = sRGB_color{ 0x21, 0x21, 0x21 };
    const sRGB_color sRGB_color::Gray14 = sRGB_color{ 0x24, 0x24, 0x24 };
    const sRGB_color sRGB_color::Gray15 = sRGB_color{ 0x26, 0x26, 0x26 };
    const sRGB_color sRGB_color::Gray16 = sRGB_color{ 0x29, 0x29, 0x29 };
    const sRGB_color sRGB_color::Gray17 = sRGB_color{ 0x2B, 0x2B, 0x2B };
    const sRGB_color sRGB_color::Gray18 = sRGB_color{ 0x2E, 0x2E, 0x2E };
    const sRGB_color sRGB_color::Gray19 = sRGB_color{ 0x30, 0x30, 0x30 };
    const sRGB_color sRGB_color::Gray2 = sRGB_color{ 0x05, 0x05, 0x05 };
    const sRGB_color sRGB_color::Gray20 = sRGB_color{ 0x33, 0x33, 0x33 };
    const sRGB_color sRGB_color::Gray21 = sRGB_color{ 0x36, 0x36, 0x36 };
    const sRGB_color sRGB_color::Gray22 = sRGB_color{ 0x38, 0x38, 0x38 };
    const sRGB_color sRGB_color::Gray23 = sRGB_color{ 0x3B, 0x3B, 0x3B };
    const sRGB_color sRGB_color::Gray24 = sRGB_color{ 0x3D, 0x3D, 0x3D };
    const sRGB_color sRGB_color::Gray25 = sRGB_color{ 0x40, 0x40, 0x40 };
    const sRGB_color sRGB_color::Gray26 = sRGB_color{ 0x42, 0x42, 0x42 };
    const sRGB_color sRGB_color::Gray27 = sRGB_color{ 0x45, 0x45, 0x45 };
    const sRGB_color sRGB_color::Gray28 = sRGB_color{ 0x47, 0x47, 0x47 };
    const sRGB_color sRGB_color::Gray29 = sRGB_color{ 0x4A, 0x4A, 0x4A };
    const sRGB_color sRGB_color::Gray3 = sRGB_color{ 0x08, 0x08, 0x08 };
    const sRGB_color sRGB_color::Gray30 = sRGB_color{ 0x4D, 0x4D, 0x4D };
    const sRGB_color sRGB_color::Gray31 = sRGB_color{ 0x4F, 0x4F, 0x4F };
    const sRGB_color sRGB_color::Gray32 = sRGB_color{ 0x52, 0x52, 0x52 };
    const sRGB_color sRGB_color::Gray33 = sRGB_color{ 0x54, 0x54, 0x54 };
    const sRGB_color sRGB_color::Gray34 = sRGB_color{ 0x57, 0x57, 0x57 };
    const sRGB_color sRGB_color::Gray35 = sRGB_color{ 0x59, 0x59, 0x59 };
    const sRGB_color sRGB_color::Gray36 = sRGB_color{ 0x5C, 0x5C, 0x5C };
    const sRGB_color sRGB_color::Gray37 = sRGB_color{ 0x5E, 0x5E, 0x5E };
    const sRGB_color sRGB_color::Gray38 = sRGB_color{ 0x61, 0x61, 0x61 };
    const sRGB_color sRGB_color::Gray39 = sRGB_color{ 0x63, 0x63, 0x63 };
    const sRGB_color sRGB_color::Gray4 = sRGB_color{ 0x0A, 0x0A, 0x0A };
    const sRGB_color sRGB_color::Gray40 = sRGB_color{ 0x66, 0x66, 0x66 };
    const sRGB_color sRGB_color::Gray41 = sRGB_color{ 0x69, 0x69, 0x69 };
    const sRGB_color sRGB_color::Gray42 = sRGB_color{ 0x6B, 0x6B, 0x6B };
    const sRGB_color sRGB_color::Gray43 = sRGB_color{ 0x6E, 0x6E, 0x6E };
    const sRGB_color sRGB_color::Gray44 = sRGB_color{ 0x70, 0x70, 0x70 };
    const sRGB_color sRGB_color::Gray45 = sRGB_color{ 0x73, 0x73, 0x73 };
    const sRGB_color sRGB_color::Gray46 = sRGB_color{ 0x75, 0x75, 0x75 };
    const sRGB_color sRGB_color::Gray47 = sRGB_color{ 0x78, 0x78, 0x78 };
    const sRGB_color sRGB_color::Gray48 = sRGB_color{ 0x7A, 0x7A, 0x7A };
    const sRGB_color sRGB_color::Gray49 = sRGB_color{ 0x7D, 0x7D, 0x7D };
    const sRGB_color sRGB_color::Gray5 = sRGB_color{ 0x0D, 0x0D, 0x0D };
    const sRGB_color sRGB_color::Gray50 = sRGB_color{ 0x7F, 0x7F, 0x7F };
    const sRGB_color sRGB_color::Gray51 = sRGB_color{ 0x82, 0x82, 0x82 };
    const sRGB_color sRGB_color::Gray52 = sRGB_color{ 0x85, 0x85, 0x85 };
    const sRGB_color sRGB_color::Gray53 = sRGB_color{ 0x87, 0x87, 0x87 };
    const sRGB_color sRGB_color::Gray54 = sRGB_color{ 0x8A, 0x8A, 0x8A };
    const sRGB_color sRGB_color::Gray55 = sRGB_color{ 0x8C, 0x8C, 0x8C };
    const sRGB_color sRGB_color::Gray56 = sRGB_color{ 0x8F, 0x8F, 0x8F };
    const sRGB_color sRGB_color::Gray57 = sRGB_color{ 0x91, 0x91, 0x91 };
    const sRGB_color sRGB_color::Gray58 = sRGB_color{ 0x94, 0x94, 0x94 };
    const sRGB_color sRGB_color::Gray59 = sRGB_color{ 0x96, 0x96, 0x96 };
    const sRGB_color sRGB_color::Gray6 = sRGB_color{ 0x0F, 0x0F, 0x0F };
    const sRGB_color sRGB_color::Gray60 = sRGB_color{ 0x99, 0x99, 0x99 };
    const sRGB_color sRGB_color::Gray61 = sRGB_color{ 0x9C, 0x9C, 0x9C };
    const sRGB_color sRGB_color::Gray62 = sRGB_color{ 0x9E, 0x9E, 0x9E };
    const sRGB_color sRGB_color::Gray63 = sRGB_color{ 0xA1, 0xA1, 0xA1 };
    const sRGB_color sRGB_color::Gray64 = sRGB_color{ 0xA3, 0xA3, 0xA3 };
    const sRGB_color sRGB_color::Gray65 = sRGB_color{ 0xA6, 0xA6, 0xA6 };
    const sRGB_color sRGB_color::Gray66 = sRGB_color{ 0xA8, 0xA8, 0xA8 };
    const sRGB_color sRGB_color::Gray67 = sRGB_color{ 0xAB, 0xAB, 0xAB };
    const sRGB_color sRGB_color::Gray68 = sRGB_color{ 0xAD, 0xAD, 0xAD };
    const sRGB_color sRGB_color::Gray69 = sRGB_color{ 0xB0, 0xB0, 0xB0 };
    const sRGB_color sRGB_color::Gray7 = sRGB_color{ 0x12, 0x12, 0x12 };
    const sRGB_color sRGB_color::Gray70 = sRGB_color{ 0xB3, 0xB3, 0xB3 };
    const sRGB_color sRGB_color::Gray71 = sRGB_color{ 0xB5, 0xB5, 0xB5 };
    const sRGB_color sRGB_color::Gray72 = sRGB_color{ 0xB8, 0xB8, 0xB8 };
    const sRGB_color sRGB_color::Gray73 = sRGB_color{ 0xBA, 0xBA, 0xBA };
    const sRGB_color sRGB_color::Gray74 = sRGB_color{ 0xBD, 0xBD, 0xBD };
    const sRGB_color sRGB_color::Gray75 = sRGB_color{ 0xBF, 0xBF, 0xBF };
    const sRGB_color sRGB_color::Gray76 = sRGB_color{ 0xC2, 0xC2, 0xC2 };
    const sRGB_color sRGB_color::Gray77 = sRGB_color{ 0xC4, 0xC4, 0xC4 };
    const sRGB_color sRGB_color::Gray78 = sRGB_color{ 0xC7, 0xC7, 0xC7 };
    const sRGB_color sRGB_color::Gray79 = sRGB_color{ 0xC9, 0xC9, 0xC9 };
    const sRGB_color sRGB_color::Gray8 = sRGB_color{ 0x14, 0x14, 0x14 };
    const sRGB_color sRGB_color::Gray80 = sRGB_color{ 0xCC, 0xCC, 0xCC };
    const sRGB_color sRGB_color::Gray81 = sRGB_color{ 0xCF, 0xCF, 0xCF };
    const sRGB_color sRGB_color::Gray82 = sRGB_color{ 0xD1, 0xD1, 0xD1 };
    const sRGB_color sRGB_color::Gray83 = sRGB_color{ 0xD4, 0xD4, 0xD4 };
    const sRGB_color sRGB_color::Gray84 = sRGB_color{ 0xD6, 0xD6, 0xD6 };
    const sRGB_color sRGB_color::Gray85 = sRGB_color{ 0xD9, 0xD9, 0xD9 };
    const sRGB_color sRGB_color::Gray86 = sRGB_color{ 0xDB, 0xDB, 0xDB };
    const sRGB_color sRGB_color::Gray87 = sRGB_color{ 0xDE, 0xDE, 0xDE };
    const sRGB_color sRGB_color::Gray88 = sRGB_color{ 0xE0, 0xE0, 0xE0 };
    const sRGB_color sRGB_color::Gray89 = sRGB_color{ 0xE3, 0xE3, 0xE3 };
    const sRGB_color sRGB_color::Gray9 = sRGB_color{ 0x17, 0x17, 0x17 };
    const sRGB_color sRGB_color::Gray90 = sRGB_color{ 0xE5, 0xE5, 0xE5 };
    const sRGB_color sRGB_color::Gray91 = sRGB_color{ 0xE8, 0xE8, 0xE8 };
    const sRGB_color sRGB_color::Gray92 = sRGB_color{ 0xEB, 0xEB, 0xEB };
    const sRGB_color sRGB_color::Gray93 = sRGB_color{ 0xED, 0xED, 0xED };
    const sRGB_color sRGB_color::Gray94 = sRGB_color{ 0xF0, 0xF0, 0xF0 };
    const sRGB_color sRGB_color::Gray95 = sRGB_color{ 0xF2, 0xF2, 0xF2 };
    const sRGB_color sRGB_color::Gray96 = sRGB_color{ 0xF5, 0xF5, 0xF5 };
    const sRGB_color sRGB_color::Gray97 = sRGB_color{ 0xF7, 0xF7, 0xF7 };
    const sRGB_color sRGB_color::Gray98 = sRGB_color{ 0xFA, 0xFA, 0xFA };
    const sRGB_color sRGB_color::Gray99 = sRGB_color{ 0xFC, 0xFC, 0xFC };
    const sRGB_color sRGB_color::Green = sRGB_color{ 0x00, 0xFF, 0x00 };
    const sRGB_color sRGB_color::Green1 = sRGB_color{ 0x00, 0xFF, 0x00 };
    const sRGB_color sRGB_color::Green2 = sRGB_color{ 0x00, 0xEE, 0x00 };
    const sRGB_color sRGB_color::Green3 = sRGB_color{ 0x00, 0xCD, 0x00 };
    const sRGB_color sRGB_color::Green4 = sRGB_color{ 0x00, 0x8B, 0x00 };
    const sRGB_color sRGB_color::GreenYellow = sRGB_color{ 0xAD, 0xFF, 0x2F };
    const sRGB_color sRGB_color::Grey = sRGB_color{ 0xBE, 0xBE, 0xBE };
    const sRGB_color sRGB_color::Grey0 = sRGB_color{ 0x00, 0x00, 0x00 };
    const sRGB_color sRGB_color::Grey1 = sRGB_color{ 0x03, 0x03, 0x03 };
    const sRGB_color sRGB_color::Grey10 = sRGB_color{ 0x1A, 0x1A, 0x1A };
    const sRGB_color sRGB_color::Grey100 = sRGB_color{ 0xFF, 0xFF, 0xFF };
    const sRGB_color sRGB_color::Grey11 = sRGB_color{ 0x1C, 0x1C, 0x1C };
    const sRGB_color sRGB_color::Grey12 = sRGB_color{ 0x1F, 0x1F, 0x1F };
    const sRGB_color sRGB_color::Grey13 = sRGB_color{ 0x21, 0x21, 0x21 };
    const sRGB_color sRGB_color::Grey14 = sRGB_color{ 0x24, 0x24, 0x24 };
    const sRGB_color sRGB_color::Grey15 = sRGB_color{ 0x26, 0x26, 0x26 };
    const sRGB_color sRGB_color::Grey16 = sRGB_color{ 0x29, 0x29, 0x29 };
    const sRGB_color sRGB_color::Grey17 = sRGB_color{ 0x2B, 0x2B, 0x2B };
    const sRGB_color sRGB_color::Grey18 = sRGB_color{ 0x2E, 0x2E, 0x2E };
    const sRGB_color sRGB_color::Grey19 = sRGB_color{ 0x30, 0x30, 0x30 };
    const sRGB_color sRGB_color::Grey2 = sRGB_color{ 0x05, 0x05, 0x05 };
    const sRGB_color sRGB_color::Grey20 = sRGB_color{ 0x33, 0x33, 0x33 };
    const sRGB_color sRGB_color::Grey21 = sRGB_color{ 0x36, 0x36, 0x36 };
    const sRGB_color sRGB_color::Grey22 = sRGB_color{ 0x38, 0x38, 0x38 };
    const sRGB_color sRGB_color::Grey23 = sRGB_color{ 0x3B, 0x3B, 0x3B };
    const sRGB_color sRGB_color::Grey24 = sRGB_color{ 0x3D, 0x3D, 0x3D };
    const sRGB_color sRGB_color::Grey25 = sRGB_color{ 0x40, 0x40, 0x40 };
    const sRGB_color sRGB_color::Grey26 = sRGB_color{ 0x42, 0x42, 0x42 };
    const sRGB_color sRGB_color::Grey27 = sRGB_color{ 0x45, 0x45, 0x45 };
    const sRGB_color sRGB_color::Grey28 = sRGB_color{ 0x47, 0x47, 0x47 };
    const sRGB_color sRGB_color::Grey29 = sRGB_color{ 0x4A, 0x4A, 0x4A };
    const sRGB_color sRGB_color::Grey3 = sRGB_color{ 0x08, 0x08, 0x08 };
    const sRGB_color sRGB_color::Grey30 = sRGB_color{ 0x4D, 0x4D, 0x4D };
    const sRGB_color sRGB_color::Grey31 = sRGB_color{ 0x4F, 0x4F, 0x4F };
    const sRGB_color sRGB_color::Grey32 = sRGB_color{ 0x52, 0x52, 0x52 };
    const sRGB_color sRGB_color::Grey33 = sRGB_color{ 0x54, 0x54, 0x54 };
    const sRGB_color sRGB_color::Grey34 = sRGB_color{ 0x57, 0x57, 0x57 };
    const sRGB_color sRGB_color::Grey35 = sRGB_color{ 0x59, 0x59, 0x59 };
    const sRGB_color sRGB_color::Grey36 = sRGB_color{ 0x5C, 0x5C, 0x5C };
    const sRGB_color sRGB_color::Grey37 = sRGB_color{ 0x5E, 0x5E, 0x5E };
    const sRGB_color sRGB_color::Grey38 = sRGB_color{ 0x61, 0x61, 0x61 };
    const sRGB_color sRGB_color::Grey39 = sRGB_color{ 0x63, 0x63, 0x63 };
    const sRGB_color sRGB_color::Grey4 = sRGB_color{ 0x0A, 0x0A, 0x0A };
    const sRGB_color sRGB_color::Grey40 = sRGB_color{ 0x66, 0x66, 0x66 };
    const sRGB_color sRGB_color::Grey41 = sRGB_color{ 0x69, 0x69, 0x69 };
    const sRGB_color sRGB_color::Grey42 = sRGB_color{ 0x6B, 0x6B, 0x6B };
    const sRGB_color sRGB_color::Grey43 = sRGB_color{ 0x6E, 0x6E, 0x6E };
    const sRGB_color sRGB_color::Grey44 = sRGB_color{ 0x70, 0x70, 0x70 };
    const sRGB_color sRGB_color::Grey45 = sRGB_color{ 0x73, 0x73, 0x73 };
    const sRGB_color sRGB_color::Grey46 = sRGB_color{ 0x75, 0x75, 0x75 };
    const sRGB_color sRGB_color::Grey47 = sRGB_color{ 0x78, 0x78, 0x78 };
    const sRGB_color sRGB_color::Grey48 = sRGB_color{ 0x7A, 0x7A, 0x7A };
    const sRGB_color sRGB_color::Grey49 = sRGB_color{ 0x7D, 0x7D, 0x7D };
    const sRGB_color sRGB_color::Grey5 = sRGB_color{ 0x0D, 0x0D, 0x0D };
    const sRGB_color sRGB_color::Grey50 = sRGB_color{ 0x7F, 0x7F, 0x7F };
    const sRGB_color sRGB_color::Grey51 = sRGB_color{ 0x82, 0x82, 0x82 };
    const sRGB_color sRGB_color::Grey52 = sRGB_color{ 0x85, 0x85, 0x85 };
    const sRGB_color sRGB_color::Grey53 = sRGB_color{ 0x87, 0x87, 0x87 };
    const sRGB_color sRGB_color::Grey54 = sRGB_color{ 0x8A, 0x8A, 0x8A };
    const sRGB_color sRGB_color::Grey55 = sRGB_color{ 0x8C, 0x8C, 0x8C };
    const sRGB_color sRGB_color::Grey56 = sRGB_color{ 0x8F, 0x8F, 0x8F };
    const sRGB_color sRGB_color::Grey57 = sRGB_color{ 0x91, 0x91, 0x91 };
    const sRGB_color sRGB_color::Grey58 = sRGB_color{ 0x94, 0x94, 0x94 };
    const sRGB_color sRGB_color::Grey59 = sRGB_color{ 0x96, 0x96, 0x96 };
    const sRGB_color sRGB_color::Grey6 = sRGB_color{ 0x0F, 0x0F, 0x0F };
    const sRGB_color sRGB_color::Grey60 = sRGB_color{ 0x99, 0x99, 0x99 };
    const sRGB_color sRGB_color::Grey61 = sRGB_color{ 0x9C, 0x9C, 0x9C };
    const sRGB_color sRGB_color::Grey62 = sRGB_color{ 0x9E, 0x9E, 0x9E };
    const sRGB_color sRGB_color::Grey63 = sRGB_color{ 0xA1, 0xA1, 0xA1 };
    const sRGB_color sRGB_color::Grey64 = sRGB_color{ 0xA3, 0xA3, 0xA3 };
    const sRGB_color sRGB_color::Grey65 = sRGB_color{ 0xA6, 0xA6, 0xA6 };
    const sRGB_color sRGB_color::Grey66 = sRGB_color{ 0xA8, 0xA8, 0xA8 };
    const sRGB_color sRGB_color::Grey67 = sRGB_color{ 0xAB, 0xAB, 0xAB };
    const sRGB_color sRGB_color::Grey68 = sRGB_color{ 0xAD, 0xAD, 0xAD };
    const sRGB_color sRGB_color::Grey69 = sRGB_color{ 0xB0, 0xB0, 0xB0 };
    const sRGB_color sRGB_color::Grey7 = sRGB_color{ 0x12, 0x12, 0x12 };
    const sRGB_color sRGB_color::Grey70 = sRGB_color{ 0xB3, 0xB3, 0xB3 };
    const sRGB_color sRGB_color::Grey71 = sRGB_color{ 0xB5, 0xB5, 0xB5 };
    const sRGB_color sRGB_color::Grey72 = sRGB_color{ 0xB8, 0xB8, 0xB8 };
    const sRGB_color sRGB_color::Grey73 = sRGB_color{ 0xBA, 0xBA, 0xBA };
    const sRGB_color sRGB_color::Grey74 = sRGB_color{ 0xBD, 0xBD, 0xBD };
    const sRGB_color sRGB_color::Grey75 = sRGB_color{ 0xBF, 0xBF, 0xBF };
    const sRGB_color sRGB_color::Grey76 = sRGB_color{ 0xC2, 0xC2, 0xC2 };
    const sRGB_color sRGB_color::Grey77 = sRGB_color{ 0xC4, 0xC4, 0xC4 };
    const sRGB_color sRGB_color::Grey78 = sRGB_color{ 0xC7, 0xC7, 0xC7 };
    const sRGB_color sRGB_color::Grey79 = sRGB_color{ 0xC9, 0xC9, 0xC9 };
    const sRGB_color sRGB_color::Grey8 = sRGB_color{ 0x14, 0x14, 0x14 };
    const sRGB_color sRGB_color::Grey80 = sRGB_color{ 0xCC, 0xCC, 0xCC };
    const sRGB_color sRGB_color::Grey81 = sRGB_color{ 0xCF, 0xCF, 0xCF };
    const sRGB_color sRGB_color::Grey82 = sRGB_color{ 0xD1, 0xD1, 0xD1 };
    const sRGB_color sRGB_color::Grey83 = sRGB_color{ 0xD4, 0xD4, 0xD4 };
    const sRGB_color sRGB_color::Grey84 = sRGB_color{ 0xD6, 0xD6, 0xD6 };
    const sRGB_color sRGB_color::Grey85 = sRGB_color{ 0xD9, 0xD9, 0xD9 };
    const sRGB_color sRGB_color::Grey86 = sRGB_color{ 0xDB, 0xDB, 0xDB };
    const sRGB_color sRGB_color::Grey87 = sRGB_color{ 0xDE, 0xDE, 0xDE };
    const sRGB_color sRGB_color::Grey88 = sRGB_color{ 0xE0, 0xE0, 0xE0 };
    const sRGB_color sRGB_color::Grey89 = sRGB_color{ 0xE3, 0xE3, 0xE3 };
    const sRGB_color sRGB_color::Grey9 = sRGB_color{ 0x17, 0x17, 0x17 };
    const sRGB_color sRGB_color::Grey90 = sRGB_color{ 0xE5, 0xE5, 0xE5 };
    const sRGB_color sRGB_color::Grey91 = sRGB_color{ 0xE8, 0xE8, 0xE8 };
    const sRGB_color sRGB_color::Grey92 = sRGB_color{ 0xEB, 0xEB, 0xEB };
    const sRGB_color sRGB_color::Grey93 = sRGB_color{ 0xED, 0xED, 0xED };
    const sRGB_color sRGB_color::Grey94 = sRGB_color{ 0xF0, 0xF0, 0xF0 };
    const sRGB_color sRGB_color::Grey95 = sRGB_color{ 0xF2, 0xF2, 0xF2 };
    const sRGB_color sRGB_color::Grey96 = sRGB_color{ 0xF5, 0xF5, 0xF5 };
    const sRGB_color sRGB_color::Grey97 = sRGB_color{ 0xF7, 0xF7, 0xF7 };
    const sRGB_color sRGB_color::Grey98 = sRGB_color{ 0xFA, 0xFA, 0xFA };
    const sRGB_color sRGB_color::Grey99 = sRGB_color{ 0xFC, 0xFC, 0xFC };
    const sRGB_color sRGB_color::Honeydew = sRGB_color{ 0xF0, 0xFF, 0xF0 };
    const sRGB_color sRGB_color::Honeydew1 = sRGB_color{ 0xF0, 0xFF, 0xF0 };
    const sRGB_color sRGB_color::Honeydew2 = sRGB_color{ 0xE0, 0xEE, 0xE0 };
    const sRGB_color sRGB_color::Honeydew3 = sRGB_color{ 0xC1, 0xCD, 0xC1 };
    const sRGB_color sRGB_color::Honeydew4 = sRGB_color{ 0x83, 0x8B, 0x83 };
    const sRGB_color sRGB_color::HotPink = sRGB_color{ 0xFF, 0x69, 0xB4 };
    const sRGB_color sRGB_color::HotPink1 = sRGB_color{ 0xFF, 0x6E, 0xB4 };
    const sRGB_color sRGB_color::HotPink2 = sRGB_color{ 0xEE, 0x6A, 0xA7 };
    const sRGB_color sRGB_color::HotPink3 = sRGB_color{ 0xCD, 0x60, 0x90 };
    const sRGB_color sRGB_color::HotPink4 = sRGB_color{ 0x8B, 0x3A, 0x62 };
    const sRGB_color sRGB_color::IndianRed = sRGB_color{ 0xCD, 0x5C, 0x5C };
    const sRGB_color sRGB_color::IndianRed1 = sRGB_color{ 0xFF, 0x6A, 0x6A };
    const sRGB_color sRGB_color::IndianRed2 = sRGB_color{ 0xEE, 0x63, 0x63 };
    const sRGB_color sRGB_color::IndianRed3 = sRGB_color{ 0xCD, 0x55, 0x55 };
    const sRGB_color sRGB_color::IndianRed4 = sRGB_color{ 0x8B, 0x3A, 0x3A };
    const sRGB_color sRGB_color::Ivory = sRGB_color{ 0xFF, 0xFF, 0xF0 };
    const sRGB_color sRGB_color::Ivory1 = sRGB_color{ 0xFF, 0xFF, 0xF0 };
    const sRGB_color sRGB_color::Ivory2 = sRGB_color{ 0xEE, 0xEE, 0xE0 };
    const sRGB_color sRGB_color::Ivory3 = sRGB_color{ 0xCD, 0xCD, 0xC1 };
    const sRGB_color sRGB_color::Ivory4 = sRGB_color{ 0x8B, 0x8B, 0x83 };
    const sRGB_color sRGB_color::Khaki = sRGB_color{ 0xF0, 0xE6, 0x8C };
    const sRGB_color sRGB_color::Khaki1 = sRGB_color{ 0xFF, 0xF6, 0x8F };
    const sRGB_color sRGB_color::Khaki2 = sRGB_color{ 0xEE, 0xE6, 0x85 };
    const sRGB_color sRGB_color::Khaki3 = sRGB_color{ 0xCD, 0xC6, 0x73 };
    const sRGB_color sRGB_color::Khaki4 = sRGB_color{ 0x8B, 0x86, 0x4E };
    const sRGB_color sRGB_color::Lavender = sRGB_color{ 0xE6, 0xE6, 0xFA };
    const sRGB_color sRGB_color::LavenderBlush = sRGB_color{ 0xFF, 0xF0, 0xF5 };
    const sRGB_color sRGB_color::LavenderBlush1 = sRGB_color{ 0xFF, 0xF0, 0xF5 };
    const sRGB_color sRGB_color::LavenderBlush2 = sRGB_color{ 0xEE, 0xE0, 0xE5 };
    const sRGB_color sRGB_color::LavenderBlush3 = sRGB_color{ 0xCD, 0xC1, 0xC5 };
    const sRGB_color sRGB_color::LavenderBlush4 = sRGB_color{ 0x8B, 0x83, 0x86 };
    const sRGB_color sRGB_color::LawnGreen = sRGB_color{ 0x7C, 0xFC, 0x00 };
    const sRGB_color sRGB_color::LemonChiffon = sRGB_color{ 0xFF, 0xFA, 0xCD };
    const sRGB_color sRGB_color::LemonChiffon1 = sRGB_color{ 0xFF, 0xFA, 0xCD };
    const sRGB_color sRGB_color::LemonChiffon2 = sRGB_color{ 0xEE, 0xE9, 0xBF };
    const sRGB_color sRGB_color::LemonChiffon3 = sRGB_color{ 0xCD, 0xC9, 0xA5 };
    const sRGB_color sRGB_color::LemonChiffon4 = sRGB_color{ 0x8B, 0x89, 0x70 };
    const sRGB_color sRGB_color::LightBlue = sRGB_color{ 0xAD, 0xD8, 0xE6 };
    const sRGB_color sRGB_color::LightBlue1 = sRGB_color{ 0xBF, 0xEF, 0xFF };
    const sRGB_color sRGB_color::LightBlue2 = sRGB_color{ 0xB2, 0xDF, 0xEE };
    const sRGB_color sRGB_color::LightBlue3 = sRGB_color{ 0x9A, 0xC0, 0xCD };
    const sRGB_color sRGB_color::LightBlue4 = sRGB_color{ 0x68, 0x83, 0x8B };
    const sRGB_color sRGB_color::LightCoral = sRGB_color{ 0xF0, 0x80, 0x80 };
    const sRGB_color sRGB_color::LightCyan = sRGB_color{ 0xE0, 0xFF, 0xFF };
    const sRGB_color sRGB_color::LightCyan1 = sRGB_color{ 0xE0, 0xFF, 0xFF };
    const sRGB_color sRGB_color::LightCyan2 = sRGB_color{ 0xD1, 0xEE, 0xEE };
    const sRGB_color sRGB_color::LightCyan3 = sRGB_color{ 0xB4, 0xCD, 0xCD };
    const sRGB_color sRGB_color::LightCyan4 = sRGB_color{ 0x7A, 0x8B, 0x8B };
    const sRGB_color sRGB_color::LightGoldenrod = sRGB_color{ 0xEE, 0xDD, 0x82 };
    const sRGB_color sRGB_color::LightGoldenrod1 = sRGB_color{ 0xFF, 0xEC, 0x8B };
    const sRGB_color sRGB_color::LightGoldenrod2 = sRGB_color{ 0xEE, 0xDC, 0x82 };
    const sRGB_color sRGB_color::LightGoldenrod3 = sRGB_color{ 0xCD, 0xBE, 0x70 };
    const sRGB_color sRGB_color::LightGoldenrod4 = sRGB_color{ 0x8B, 0x81, 0x4C };
    const sRGB_color sRGB_color::LightGoldenrodYellow = sRGB_color{ 0xFA, 0xFA, 0xD2 };
    const sRGB_color sRGB_color::LightGray = sRGB_color{ 0xD3, 0xD3, 0xD3 };
    const sRGB_color sRGB_color::LightGreen = sRGB_color{ 0x90, 0xEE, 0x90 };
    const sRGB_color sRGB_color::LightGrey = sRGB_color{ 0xD3, 0xD3, 0xD3 };
    const sRGB_color sRGB_color::LightPink = sRGB_color{ 0xFF, 0xB6, 0xC1 };
    const sRGB_color sRGB_color::LightPink1 = sRGB_color{ 0xFF, 0xAE, 0xB9 };
    const sRGB_color sRGB_color::LightPink2 = sRGB_color{ 0xEE, 0xA2, 0xAD };
    const sRGB_color sRGB_color::LightPink3 = sRGB_color{ 0xCD, 0x8C, 0x95 };
    const sRGB_color sRGB_color::LightPink4 = sRGB_color{ 0x8B, 0x5F, 0x65 };
    const sRGB_color sRGB_color::LightSalmon = sRGB_color{ 0xFF, 0xA0, 0x7A };
    const sRGB_color sRGB_color::LightSalmon1 = sRGB_color{ 0xFF, 0xA0, 0x7A };
    const sRGB_color sRGB_color::LightSalmon2 = sRGB_color{ 0xEE, 0x95, 0x72 };
    const sRGB_color sRGB_color::LightSalmon3 = sRGB_color{ 0xCD, 0x81, 0x62 };
    const sRGB_color sRGB_color::LightSalmon4 = sRGB_color{ 0x8B, 0x57, 0x42 };
    const sRGB_color sRGB_color::LightSeaGreen = sRGB_color{ 0x20, 0xB2, 0xAA };
    const sRGB_color sRGB_color::LightSkyBlue = sRGB_color{ 0x87, 0xCE, 0xFA };
    const sRGB_color sRGB_color::LightSkyBlue1 = sRGB_color{ 0xB0, 0xE2, 0xFF };
    const sRGB_color sRGB_color::LightSkyBlue2 = sRGB_color{ 0xA4, 0xD3, 0xEE };
    const sRGB_color sRGB_color::LightSkyBlue3 = sRGB_color{ 0x8D, 0xB6, 0xCD };
    const sRGB_color sRGB_color::LightSkyBlue4 = sRGB_color{ 0x60, 0x7B, 0x8B };
    const sRGB_color sRGB_color::LightSlateBlue = sRGB_color{ 0x84, 0x70, 0xFF };
    const sRGB_color sRGB_color::LightSlateGray = sRGB_color{ 0x77, 0x88, 0x99 };
    const sRGB_color sRGB_color::LightSlateGrey = sRGB_color{ 0x77, 0x88, 0x99 };
    const sRGB_color sRGB_color::LightSteelBlue = sRGB_color{ 0xB0, 0xC4, 0xDE };
    const sRGB_color sRGB_color::LightSteelBlue1 = sRGB_color{ 0xCA, 0xE1, 0xFF };
    const sRGB_color sRGB_color::LightSteelBlue2 = sRGB_color{ 0xBC, 0xD2, 0xEE };
    const sRGB_color sRGB_color::LightSteelBlue3 = sRGB_color{ 0xA2, 0xB5, 0xCD };
    const sRGB_color sRGB_color::LightSteelBlue4 = sRGB_color{ 0x6E, 0x7B, 0x8B };
    const sRGB_color sRGB_color::LightYellow = sRGB_color{ 0xFF, 0xFF, 0xE0 };
    const sRGB_color sRGB_color::LightYellow1 = sRGB_color{ 0xFF, 0xFF, 0xE0 };
    const sRGB_color sRGB_color::LightYellow2 = sRGB_color{ 0xEE, 0xEE, 0xD1 };
    const sRGB_color sRGB_color::LightYellow3 = sRGB_color{ 0xCD, 0xCD, 0xB4 };
    const sRGB_color sRGB_color::LightYellow4 = sRGB_color{ 0x8B, 0x8B, 0x7A };
    const sRGB_color sRGB_color::LimeGreen = sRGB_color{ 0x32, 0xCD, 0x32 };
    const sRGB_color sRGB_color::Linen = sRGB_color{ 0xFA, 0xF0, 0xE6 };
    const sRGB_color sRGB_color::Magenta = sRGB_color{ 0xFF, 0x00, 0xFF };
    const sRGB_color sRGB_color::Magenta1 = sRGB_color{ 0xFF, 0x00, 0xFF };
    const sRGB_color sRGB_color::Magenta2 = sRGB_color{ 0xEE, 0x00, 0xEE };
    const sRGB_color sRGB_color::Magenta3 = sRGB_color{ 0xCD, 0x00, 0xCD };
    const sRGB_color sRGB_color::Magenta4 = sRGB_color{ 0x8B, 0x00, 0x8B };
    const sRGB_color sRGB_color::Maroon = sRGB_color{ 0xB0, 0x30, 0x60 };
    const sRGB_color sRGB_color::Maroon1 = sRGB_color{ 0xFF, 0x34, 0xB3 };
    const sRGB_color sRGB_color::Maroon2 = sRGB_color{ 0xEE, 0x30, 0xA7 };
    const sRGB_color sRGB_color::Maroon3 = sRGB_color{ 0xCD, 0x29, 0x90 };
    const sRGB_color sRGB_color::Maroon4 = sRGB_color{ 0x8B, 0x1C, 0x62 };
    const sRGB_color sRGB_color::MediumAquamarine = sRGB_color{ 0x66, 0xCD, 0xAA };
    const sRGB_color sRGB_color::MediumBlue = sRGB_color{ 0x00, 0x00, 0xCD };
    const sRGB_color sRGB_color::MediumOrchid = sRGB_color{ 0xBA, 0x55, 0xD3 };
    const sRGB_color sRGB_color::MediumOrchid1 = sRGB_color{ 0xE0, 0x66, 0xFF };
    const sRGB_color sRGB_color::MediumOrchid2 = sRGB_color{ 0xD1, 0x5F, 0xEE };
    const sRGB_color sRGB_color::MediumOrchid3 = sRGB_color{ 0xB4, 0x52, 0xCD };
    const sRGB_color sRGB_color::MediumOrchid4 = sRGB_color{ 0x7A, 0x37, 0x8B };
    const sRGB_color sRGB_color::MediumPurple = sRGB_color{ 0x93, 0x70, 0xDB };
    const sRGB_color sRGB_color::MediumPurple1 = sRGB_color{ 0xAB, 0x82, 0xFF };
    const sRGB_color sRGB_color::MediumPurple2 = sRGB_color{ 0x9F, 0x79, 0xEE };
    const sRGB_color sRGB_color::MediumPurple3 = sRGB_color{ 0x89, 0x68, 0xCD };
    const sRGB_color sRGB_color::MediumPurple4 = sRGB_color{ 0x5D, 0x47, 0x8B };
    const sRGB_color sRGB_color::MediumSeaGreen = sRGB_color{ 0x3C, 0xB3, 0x71 };
    const sRGB_color sRGB_color::MediumSlateBlue = sRGB_color{ 0x7B, 0x68, 0xEE };
    const sRGB_color sRGB_color::MediumSpringGreen = sRGB_color{ 0x00, 0xFA, 0x9A };
    const sRGB_color sRGB_color::MediumTurquoise = sRGB_color{ 0x48, 0xD1, 0xCC };
    const sRGB_color sRGB_color::MediumVioletRed = sRGB_color{ 0xC7, 0x15, 0x85 };
    const sRGB_color sRGB_color::MidnightBlue = sRGB_color{ 0x19, 0x19, 0x70 };
    const sRGB_color sRGB_color::MintCream = sRGB_color{ 0xF5, 0xFF, 0xFA };
    const sRGB_color sRGB_color::MistyRose = sRGB_color{ 0xFF, 0xE4, 0xE1 };
    const sRGB_color sRGB_color::MistyRose1 = sRGB_color{ 0xFF, 0xE4, 0xE1 };
    const sRGB_color sRGB_color::MistyRose2 = sRGB_color{ 0xEE, 0xD5, 0xD2 };
    const sRGB_color sRGB_color::MistyRose3 = sRGB_color{ 0xCD, 0xB7, 0xB5 };
    const sRGB_color sRGB_color::MistyRose4 = sRGB_color{ 0x8B, 0x7D, 0x7B };
    const sRGB_color sRGB_color::Moccasin = sRGB_color{ 0xFF, 0xE4, 0xB5 };
    const sRGB_color sRGB_color::NavajoWhite = sRGB_color{ 0xFF, 0xDE, 0xAD };
    const sRGB_color sRGB_color::NavajoWhite1 = sRGB_color{ 0xFF, 0xDE, 0xAD };
    const sRGB_color sRGB_color::NavajoWhite2 = sRGB_color{ 0xEE, 0xCF, 0xA1 };
    const sRGB_color sRGB_color::NavajoWhite3 = sRGB_color{ 0xCD, 0xB3, 0x8B };
    const sRGB_color sRGB_color::NavajoWhite4 = sRGB_color{ 0x8B, 0x79, 0x5E };
    const sRGB_color sRGB_color::Navy = sRGB_color{ 0x00, 0x00, 0x80 };
    const sRGB_color sRGB_color::NavyBlue = sRGB_color{ 0x00, 0x00, 0x80 };
    const sRGB_color sRGB_color::OldLace = sRGB_color{ 0xFD, 0xF5, 0xE6 };
    const sRGB_color sRGB_color::OliveDrab = sRGB_color{ 0x6B, 0x8E, 0x23 };
    const sRGB_color sRGB_color::OliveDrab1 = sRGB_color{ 0xC0, 0xFF, 0x3E };
    const sRGB_color sRGB_color::OliveDrab2 = sRGB_color{ 0xB3, 0xEE, 0x3A };
    const sRGB_color sRGB_color::OliveDrab3 = sRGB_color{ 0x9A, 0xCD, 0x32 };
    const sRGB_color sRGB_color::OliveDrab4 = sRGB_color{ 0x69, 0x8B, 0x22 };
    const sRGB_color sRGB_color::Orange = sRGB_color{ 0xFF, 0xA5, 0x00 };
    const sRGB_color sRGB_color::Orange1 = sRGB_color{ 0xFF, 0xA5, 0x00 };
    const sRGB_color sRGB_color::Orange2 = sRGB_color{ 0xEE, 0x9A, 0x00 };
    const sRGB_color sRGB_color::Orange3 = sRGB_color{ 0xCD, 0x85, 0x00 };
    const sRGB_color sRGB_color::Orange4 = sRGB_color{ 0x8B, 0x5A, 0x00 };
    const sRGB_color sRGB_color::OrangeRed = sRGB_color{ 0xFF, 0x45, 0x00 };
    const sRGB_color sRGB_color::OrangeRed1 = sRGB_color{ 0xFF, 0x45, 0x00 };
    const sRGB_color sRGB_color::OrangeRed2 = sRGB_color{ 0xEE, 0x40, 0x00 };
    const sRGB_color sRGB_color::OrangeRed3 = sRGB_color{ 0xCD, 0x37, 0x00 };
    const sRGB_color sRGB_color::OrangeRed4 = sRGB_color{ 0x8B, 0x25, 0x00 };
    const sRGB_color sRGB_color::Orchid = sRGB_color{ 0xDA, 0x70, 0xD6 };
    const sRGB_color sRGB_color::Orchid1 = sRGB_color{ 0xFF, 0x83, 0xFA };
    const sRGB_color sRGB_color::Orchid2 = sRGB_color{ 0xEE, 0x7A, 0xE9 };
    const sRGB_color sRGB_color::Orchid3 = sRGB_color{ 0xCD, 0x69, 0xC9 };
    const sRGB_color sRGB_color::Orchid4 = sRGB_color{ 0x8B, 0x47, 0x89 };
    const sRGB_color sRGB_color::PaleGoldenrod = sRGB_color{ 0xEE, 0xE8, 0xAA };
    const sRGB_color sRGB_color::PaleGreen = sRGB_color{ 0x98, 0xFB, 0x98 };
    const sRGB_color sRGB_color::PaleGreen1 = sRGB_color{ 0x9A, 0xFF, 0x9A };
    const sRGB_color sRGB_color::PaleGreen2 = sRGB_color{ 0x90, 0xEE, 0x90 };
    const sRGB_color sRGB_color::PaleGreen3 = sRGB_color{ 0x7C, 0xCD, 0x7C };
    const sRGB_color sRGB_color::PaleGreen4 = sRGB_color{ 0x54, 0x8B, 0x54 };
    const sRGB_color sRGB_color::PaleTurquoise = sRGB_color{ 0xAF, 0xEE, 0xEE };
    const sRGB_color sRGB_color::PaleTurquoise1 = sRGB_color{ 0xBB, 0xFF, 0xFF };
    const sRGB_color sRGB_color::PaleTurquoise2 = sRGB_color{ 0xAE, 0xEE, 0xEE };
    const sRGB_color sRGB_color::PaleTurquoise3 = sRGB_color{ 0x96, 0xCD, 0xCD };
    const sRGB_color sRGB_color::PaleTurquoise4 = sRGB_color{ 0x66, 0x8B, 0x8B };
    const sRGB_color sRGB_color::PaleVioletRed = sRGB_color{ 0xDB, 0x70, 0x93 };
    const sRGB_color sRGB_color::PaleVioletRed1 = sRGB_color{ 0xFF, 0x82, 0xAB };
    const sRGB_color sRGB_color::PaleVioletRed2 = sRGB_color{ 0xEE, 0x79, 0x9F };
    const sRGB_color sRGB_color::PaleVioletRed3 = sRGB_color{ 0xCD, 0x68, 0x89 };
    const sRGB_color sRGB_color::PaleVioletRed4 = sRGB_color{ 0x8B, 0x47, 0x5D };
    const sRGB_color sRGB_color::PapayaWhip = sRGB_color{ 0xFF, 0xEF, 0xD5 };
    const sRGB_color sRGB_color::PeachPuff = sRGB_color{ 0xFF, 0xDA, 0xB9 };
    const sRGB_color sRGB_color::PeachPuff1 = sRGB_color{ 0xFF, 0xDA, 0xB9 };
    const sRGB_color sRGB_color::PeachPuff2 = sRGB_color{ 0xEE, 0xCB, 0xAD };
    const sRGB_color sRGB_color::PeachPuff3 = sRGB_color{ 0xCD, 0xAF, 0x95 };
    const sRGB_color sRGB_color::PeachPuff4 = sRGB_color{ 0x8B, 0x77, 0x65 };
    const sRGB_color sRGB_color::Peru = sRGB_color{ 0xCD, 0x85, 0x3F };
    const sRGB_color sRGB_color::Pink = sRGB_color{ 0xFF, 0xC0, 0xCB };
    const sRGB_color sRGB_color::Pink1 = sRGB_color{ 0xFF, 0xB5, 0xC5 };
    const sRGB_color sRGB_color::Pink2 = sRGB_color{ 0xEE, 0xA9, 0xB8 };
    const sRGB_color sRGB_color::Pink3 = sRGB_color{ 0xCD, 0x91, 0x9E };
    const sRGB_color sRGB_color::Pink4 = sRGB_color{ 0x8B, 0x63, 0x6C };
    const sRGB_color sRGB_color::Plum = sRGB_color{ 0xDD, 0xA0, 0xDD };
    const sRGB_color sRGB_color::Plum1 = sRGB_color{ 0xFF, 0xBB, 0xFF };
    const sRGB_color sRGB_color::Plum2 = sRGB_color{ 0xEE, 0xAE, 0xEE };
    const sRGB_color sRGB_color::Plum3 = sRGB_color{ 0xCD, 0x96, 0xCD };
    const sRGB_color sRGB_color::Plum4 = sRGB_color{ 0x8B, 0x66, 0x8B };
    const sRGB_color sRGB_color::PowderBlue = sRGB_color{ 0xB0, 0xE0, 0xE6 };
    const sRGB_color sRGB_color::Purple = sRGB_color{ 0xA0, 0x20, 0xF0 };
    const sRGB_color sRGB_color::Purple1 = sRGB_color{ 0x9B, 0x30, 0xFF };
    const sRGB_color sRGB_color::Purple2 = sRGB_color{ 0x91, 0x2C, 0xEE };
    const sRGB_color sRGB_color::Purple3 = sRGB_color{ 0x7D, 0x26, 0xCD };
    const sRGB_color sRGB_color::Purple4 = sRGB_color{ 0x55, 0x1A, 0x8B };
    const sRGB_color sRGB_color::Red = sRGB_color{ 0xFF, 0x00, 0x00 };
    const sRGB_color sRGB_color::Red1 = sRGB_color{ 0xFF, 0x00, 0x00 };
    const sRGB_color sRGB_color::Red2 = sRGB_color{ 0xEE, 0x00, 0x00 };
    const sRGB_color sRGB_color::Red3 = sRGB_color{ 0xCD, 0x00, 0x00 };
    const sRGB_color sRGB_color::Red4 = sRGB_color{ 0x8B, 0x00, 0x00 };
    const sRGB_color sRGB_color::RosyBrown = sRGB_color{ 0xBC, 0x8F, 0x8F };
    const sRGB_color sRGB_color::RosyBrown1 = sRGB_color{ 0xFF, 0xC1, 0xC1 };
    const sRGB_color sRGB_color::RosyBrown2 = sRGB_color{ 0xEE, 0xB4, 0xB4 };
    const sRGB_color sRGB_color::RosyBrown3 = sRGB_color{ 0xCD, 0x9B, 0x9B };
    const sRGB_color sRGB_color::RosyBrown4 = sRGB_color{ 0x8B, 0x69, 0x69 };
    const sRGB_color sRGB_color::RoyalBlue = sRGB_color{ 0x41, 0x69, 0xE1 };
    const sRGB_color sRGB_color::RoyalBlue1 = sRGB_color{ 0x48, 0x76, 0xFF };
    const sRGB_color sRGB_color::RoyalBlue2 = sRGB_color{ 0x43, 0x6E, 0xEE };
    const sRGB_color sRGB_color::RoyalBlue3 = sRGB_color{ 0x3A, 0x5F, 0xCD };
    const sRGB_color sRGB_color::RoyalBlue4 = sRGB_color{ 0x27, 0x40, 0x8B };
    const sRGB_color sRGB_color::SaddleBrown = sRGB_color{ 0x8B, 0x45, 0x13 };
    const sRGB_color sRGB_color::Salmon = sRGB_color{ 0xFA, 0x80, 0x72 };
    const sRGB_color sRGB_color::Salmon1 = sRGB_color{ 0xFF, 0x8C, 0x69 };
    const sRGB_color sRGB_color::Salmon2 = sRGB_color{ 0xEE, 0x82, 0x62 };
    const sRGB_color sRGB_color::Salmon3 = sRGB_color{ 0xCD, 0x70, 0x54 };
    const sRGB_color sRGB_color::Salmon4 = sRGB_color{ 0x8B, 0x4C, 0x39 };
    const sRGB_color sRGB_color::SandyBrown = sRGB_color{ 0xF4, 0xA4, 0x60 };
    const sRGB_color sRGB_color::SeaGreen = sRGB_color{ 0x2E, 0x8B, 0x57 };
    const sRGB_color sRGB_color::SeaGreen1 = sRGB_color{ 0x54, 0xFF, 0x9F };
    const sRGB_color sRGB_color::SeaGreen2 = sRGB_color{ 0x4E, 0xEE, 0x94 };
    const sRGB_color sRGB_color::SeaGreen3 = sRGB_color{ 0x43, 0xCD, 0x80 };
    const sRGB_color sRGB_color::SeaGreen4 = sRGB_color{ 0x2E, 0x8B, 0x57 };
    const sRGB_color sRGB_color::Seashell = sRGB_color{ 0xFF, 0xF5, 0xEE };
    const sRGB_color sRGB_color::Seashell1 = sRGB_color{ 0xFF, 0xF5, 0xEE };
    const sRGB_color sRGB_color::Seashell2 = sRGB_color{ 0xEE, 0xE5, 0xDE };
    const sRGB_color sRGB_color::Seashell3 = sRGB_color{ 0xCD, 0xC5, 0xBF };
    const sRGB_color sRGB_color::Seashell4 = sRGB_color{ 0x8B, 0x86, 0x82 };
    const sRGB_color sRGB_color::Sienna = sRGB_color{ 0xA0, 0x52, 0x2D };
    const sRGB_color sRGB_color::Sienna1 = sRGB_color{ 0xFF, 0x82, 0x47 };
    const sRGB_color sRGB_color::Sienna2 = sRGB_color{ 0xEE, 0x79, 0x42 };
    const sRGB_color sRGB_color::Sienna3 = sRGB_color{ 0xCD, 0x68, 0x39 };
    const sRGB_color sRGB_color::Sienna4 = sRGB_color{ 0x8B, 0x47, 0x26 };
    const sRGB_color sRGB_color::SkyBlue = sRGB_color{ 0x87, 0xCE, 0xEB };
    const sRGB_color sRGB_color::SkyBlue1 = sRGB_color{ 0x87, 0xCE, 0xFF };
    const sRGB_color sRGB_color::SkyBlue2 = sRGB_color{ 0x7E, 0xC0, 0xEE };
    const sRGB_color sRGB_color::SkyBlue3 = sRGB_color{ 0x6C, 0xA6, 0xCD };
    const sRGB_color sRGB_color::SkyBlue4 = sRGB_color{ 0x4A, 0x70, 0x8B };
    const sRGB_color sRGB_color::SlateBlue = sRGB_color{ 0x6A, 0x5A, 0xCD };
    const sRGB_color sRGB_color::SlateBlue1 = sRGB_color{ 0x83, 0x6F, 0xFF };
    const sRGB_color sRGB_color::SlateBlue2 = sRGB_color{ 0x7A, 0x67, 0xEE };
    const sRGB_color sRGB_color::SlateBlue3 = sRGB_color{ 0x69, 0x59, 0xCD };
    const sRGB_color sRGB_color::SlateBlue4 = sRGB_color{ 0x47, 0x3C, 0x8B };
    const sRGB_color sRGB_color::SlateGray = sRGB_color{ 0x70, 0x80, 0x90 };
    const sRGB_color sRGB_color::SlateGray1 = sRGB_color{ 0xC6, 0xE2, 0xFF };
    const sRGB_color sRGB_color::SlateGray2 = sRGB_color{ 0xB9, 0xD3, 0xEE };
    const sRGB_color sRGB_color::SlateGray3 = sRGB_color{ 0x9F, 0xB6, 0xCD };
    const sRGB_color sRGB_color::SlateGray4 = sRGB_color{ 0x6C, 0x7B, 0x8B };
    const sRGB_color sRGB_color::SlateGrey = sRGB_color{ 0x70, 0x80, 0x90 };
    const sRGB_color sRGB_color::Snow = sRGB_color{ 0xFF, 0xFA, 0xFA };
    const sRGB_color sRGB_color::Snow1 = sRGB_color{ 0xFF, 0xFA, 0xFA };
    const sRGB_color sRGB_color::Snow2 = sRGB_color{ 0xEE, 0xE9, 0xE9 };
    const sRGB_color sRGB_color::Snow3 = sRGB_color{ 0xCD, 0xC9, 0xC9 };
    const sRGB_color sRGB_color::Snow4 = sRGB_color{ 0x8B, 0x89, 0x89 };
    const sRGB_color sRGB_color::SpringGreen = sRGB_color{ 0x00, 0xFF, 0x7F };
    const sRGB_color sRGB_color::SpringGreen1 = sRGB_color{ 0x00, 0xFF, 0x7F };
    const sRGB_color sRGB_color::SpringGreen2 = sRGB_color{ 0x00, 0xEE, 0x76 };
    const sRGB_color sRGB_color::SpringGreen3 = sRGB_color{ 0x00, 0xCD, 0x66 };
    const sRGB_color sRGB_color::SpringGreen4 = sRGB_color{ 0x00, 0x8B, 0x45 };
    const sRGB_color sRGB_color::SteelBlue = sRGB_color{ 0x46, 0x82, 0xB4 };
    const sRGB_color sRGB_color::SteelBlue1 = sRGB_color{ 0x63, 0xB8, 0xFF };
    const sRGB_color sRGB_color::SteelBlue2 = sRGB_color{ 0x5C, 0xAC, 0xEE };
    const sRGB_color sRGB_color::SteelBlue3 = sRGB_color{ 0x4F, 0x94, 0xCD };
    const sRGB_color sRGB_color::SteelBlue4 = sRGB_color{ 0x36, 0x64, 0x8B };
    const sRGB_color sRGB_color::Tan = sRGB_color{ 0xD2, 0xB4, 0x8C };
    const sRGB_color sRGB_color::Tan1 = sRGB_color{ 0xFF, 0xA5, 0x4F };
    const sRGB_color sRGB_color::Tan2 = sRGB_color{ 0xEE, 0x9A, 0x49 };
    const sRGB_color sRGB_color::Tan3 = sRGB_color{ 0xCD, 0x85, 0x3F };
    const sRGB_color sRGB_color::Tan4 = sRGB_color{ 0x8B, 0x5A, 0x2B };
    const sRGB_color sRGB_color::Thistle = sRGB_color{ 0xD8, 0xBF, 0xD8 };
    const sRGB_color sRGB_color::Thistle1 = sRGB_color{ 0xFF, 0xE1, 0xFF };
    const sRGB_color sRGB_color::Thistle2 = sRGB_color{ 0xEE, 0xD2, 0xEE };
    const sRGB_color sRGB_color::Thistle3 = sRGB_color{ 0xCD, 0xB5, 0xCD };
    const sRGB_color sRGB_color::Thistle4 = sRGB_color{ 0x8B, 0x7B, 0x8B };
    const sRGB_color sRGB_color::Tomato = sRGB_color{ 0xFF, 0x63, 0x47 };
    const sRGB_color sRGB_color::Tomato1 = sRGB_color{ 0xFF, 0x63, 0x47 };
    const sRGB_color sRGB_color::Tomato2 = sRGB_color{ 0xEE, 0x5C, 0x42 };
    const sRGB_color sRGB_color::Tomato3 = sRGB_color{ 0xCD, 0x4F, 0x39 };
    const sRGB_color sRGB_color::Tomato4 = sRGB_color{ 0x8B, 0x36, 0x26 };
    const sRGB_color sRGB_color::Turquoise = sRGB_color{ 0x40, 0xE0, 0xD0 };
    const sRGB_color sRGB_color::Turquoise1 = sRGB_color{ 0x00, 0xF5, 0xFF };
    const sRGB_color sRGB_color::Turquoise2 = sRGB_color{ 0x00, 0xE5, 0xEE };
    const sRGB_color sRGB_color::Turquoise3 = sRGB_color{ 0x00, 0xC5, 0xCD };
    const sRGB_color sRGB_color::Turquoise4 = sRGB_color{ 0x00, 0x86, 0x8B };
    const sRGB_color sRGB_color::Violet = sRGB_color{ 0xEE, 0x82, 0xEE };
    const sRGB_color sRGB_color::VioletRed = sRGB_color{ 0xD0, 0x20, 0x90 };
    const sRGB_color sRGB_color::VioletRed1 = sRGB_color{ 0xFF, 0x3E, 0x96 };
    const sRGB_color sRGB_color::VioletRed2 = sRGB_color{ 0xEE, 0x3A, 0x8C };
    const sRGB_color sRGB_color::VioletRed3 = sRGB_color{ 0xCD, 0x32, 0x78 };
    const sRGB_color sRGB_color::VioletRed4 = sRGB_color{ 0x8B, 0x22, 0x52 };
    const sRGB_color sRGB_color::Wheat = sRGB_color{ 0xF5, 0xDE, 0xB3 };
    const sRGB_color sRGB_color::Wheat1 = sRGB_color{ 0xFF, 0xE7, 0xBA };
    const sRGB_color sRGB_color::Wheat2 = sRGB_color{ 0xEE, 0xD8, 0xAE };
    const sRGB_color sRGB_color::Wheat3 = sRGB_color{ 0xCD, 0xBA, 0x96 };
    const sRGB_color sRGB_color::Wheat4 = sRGB_color{ 0x8B, 0x7E, 0x66 };
    const sRGB_color sRGB_color::White = sRGB_color{ 0xFF, 0xFF, 0xFF };
    const sRGB_color sRGB_color::WhiteSmoke = sRGB_color{ 0xF5, 0xF5, 0xF5 };
    const sRGB_color sRGB_color::Yellow = sRGB_color{ 0xFF, 0xFF, 0x00 };
    const sRGB_color sRGB_color::Yellow1 = sRGB_color{ 0xFF, 0xFF, 0x00 };
    const sRGB_color sRGB_color::Yellow2 = sRGB_color{ 0xEE, 0xEE, 0x00 };
    const sRGB_color sRGB_color::Yellow3 = sRGB_color{ 0xCD, 0xCD, 0x00 };
    const sRGB_color sRGB_color::Yellow4 = sRGB_color{ 0x8B, 0x8B, 0x00 };
    const sRGB_color sRGB_color::YellowGreen = sRGB_color{ 0x9A, 0xCD, 0x32 };

    optional_color sRGB_color::from_name(std::string const& aName)
    {
        struct named_colors : public std::map<neolib::ci_string, sRGB_color>
        {
            named_colors() : std::map<neolib::ci_string, sRGB_color>
            ({
                { "alice blue", AliceBlue },
                { "AliceBlue", AliceBlue },
                { "antique white", AntiqueWhite },
                { "AntiqueWhite", AntiqueWhite },
                { "AntiqueWhite1", AntiqueWhite1 },
                { "AntiqueWhite2", AntiqueWhite2 },
                { "AntiqueWhite3", AntiqueWhite3 },
                { "AntiqueWhite4", AntiqueWhite4 },
                { "aquamarine", Aquamarine },
                { "aquamarine1", Aquamarine1 },
                { "aquamarine2", Aquamarine2 },
                { "aquamarine3", Aquamarine3 },
                { "aquamarine4", Aquamarine4 },
                { "azure", Azure },
                { "azure1", Azure1 },
                { "azure2", Azure2 },
                { "azure3", Azure3 },
                { "azure4", Azure4 },
                { "beige", Beige },
                { "bisque", Bisque },
                { "bisque1", Bisque1 },
                { "bisque2", Bisque2 },
                { "bisque3", Bisque3 },
                { "bisque4", Bisque4 },
                { "black", Black },
                { "blanched almond", BlanchedAlmond },
                { "BlanchedAlmond", BlanchedAlmond },
                { "blue violet", BlueViolet },
                { "blue", Blue },
                { "blue1", Blue1 },
                { "blue2", Blue2 },
                { "blue3", Blue3 },
                { "blue4", Blue4 },
                { "BlueViolet", BlueViolet },
                { "brown", Brown },
                { "brown1", Brown1 },
                { "brown2", Brown2 },
                { "brown3", Brown3 },
                { "brown4", Brown4 },
                { "burlywood", Burlywood },
                { "burlywood1", Burlywood1 },
                { "burlywood2", Burlywood2 },
                { "burlywood3", Burlywood3 },
                { "burlywood4", Burlywood4 },
                { "cadet blue", CadetBlue },
                { "CadetBlue", CadetBlue },
                { "CadetBlue1", CadetBlue1 },
                { "CadetBlue2", CadetBlue2 },
                { "CadetBlue3", CadetBlue3 },
                { "CadetBlue4", CadetBlue4 },
                { "chartreuse", Chartreuse },
                { "chartreuse1", Chartreuse1 },
                { "chartreuse2", Chartreuse2 },
                { "chartreuse3", Chartreuse3 },
                { "chartreuse4", Chartreuse4 },
                { "chocolate", Chocolate },
                { "chocolate1", Chocolate1 },
                { "chocolate2", Chocolate2 },
                { "chocolate3", Chocolate3 },
                { "chocolate4", Chocolate4 },
                { "coral", Coral },
                { "coral1", Coral1 },
                { "coral2", Coral2 },
                { "coral3", Coral3 },
                { "coral4", Coral4 },
                { "cornflower blue", CornflowerBlue },
                { "CornflowerBlue", CornflowerBlue },
                { "cornsilk", Cornsilk },
                { "cornsilk1", Cornsilk1 },
                { "cornsilk2", Cornsilk2 },
                { "cornsilk3", Cornsilk3 },
                { "cornsilk4", Cornsilk4 },
                { "cyan", Cyan },
                { "cyan1", Cyan1 },
                { "cyan2", Cyan2 },
                { "cyan3", Cyan3 },
                { "cyan4", Cyan4 },
                { "dark blue", DarkBlue },
                { "dark cyan", DarkCyan },
                { "dark goldenrod", DarkGoldenrod },
                { "dark gray", DarkGray },
                { "dark green", DarkGreen },
                { "dark grey", DarkGrey },
                { "dark khaki", DarkKhaki },
                { "dark magenta", DarkMagenta },
                { "dark olive green", DarkOliveGreen },
                { "dark orange", DarkOrange },
                { "dark orchid", DarkOrchid },
                { "dark red", DarkRed },
                { "dark salmon", DarkSalmon },
                { "dark sea green", DarkSeaGreen },
                { "dark slate blue", DarkSlateBlue },
                { "dark slate gray", DarkSlateGray },
                { "dark slate grey", DarkSlateGrey },
                { "dark turquoise", DarkTurquoise },
                { "dark violet", DarkViolet },
                { "DarkBlue", DarkBlue },
                { "DarkCyan", DarkCyan },
                { "DarkGoldenrod", DarkGoldenrod },
                { "DarkGoldenrod1", DarkGoldenrod1 },
                { "DarkGoldenrod2", DarkGoldenrod2 },
                { "DarkGoldenrod3", DarkGoldenrod3 },
                { "DarkGoldenrod4", DarkGoldenrod4 },
                { "DarkGray", DarkGray },
                { "DarkGreen", DarkGreen },
                { "DarkGrey", DarkGrey },
                { "DarkKhaki", DarkKhaki },
                { "DarkMagenta", DarkMagenta },
                { "DarkOliveGreen", DarkOliveGreen },
                { "DarkOliveGreen1", DarkOliveGreen1 },
                { "DarkOliveGreen2", DarkOliveGreen2 },
                { "DarkOliveGreen3", DarkOliveGreen3 },
                { "DarkOliveGreen4", DarkOliveGreen4 },
                { "DarkOrange", DarkOrange },
                { "DarkOrange1", DarkOrange1 },
                { "DarkOrange2", DarkOrange2 },
                { "DarkOrange3", DarkOrange3 },
                { "DarkOrange4", DarkOrange4 },
                { "DarkOrchid", DarkOrchid },
                { "DarkOrchid1", DarkOrchid1 },
                { "DarkOrchid2", DarkOrchid2 },
                { "DarkOrchid3", DarkOrchid3 },
                { "DarkOrchid4", DarkOrchid4 },
                { "DarkRed", DarkRed },
                { "DarkSalmon", DarkSalmon },
                { "DarkSeaGreen", DarkSeaGreen },
                { "DarkSeaGreen1", DarkSeaGreen1 },
                { "DarkSeaGreen2", DarkSeaGreen2 },
                { "DarkSeaGreen3", DarkSeaGreen3 },
                { "DarkSeaGreen4", DarkSeaGreen4 },
                { "DarkSlateBlue", DarkSlateBlue },
                { "DarkSlateGray", DarkSlateGray },
                { "DarkSlateGray1", DarkSlateGray1 },
                { "DarkSlateGray2", DarkSlateGray2 },
                { "DarkSlateGray3", DarkSlateGray3 },
                { "DarkSlateGray4", DarkSlateGray4 },
                { "DarkSlateGrey", DarkSlateGrey },
                { "DarkTurquoise", DarkTurquoise },
                { "DarkViolet", DarkViolet },
                { "DebianRed", DebianRed },
                { "deep pink", DeepPink },
                { "deep sky blue", DeepSkyBlue },
                { "DeepPink", DeepPink },
                { "DeepPink1", DeepPink1 },
                { "DeepPink2", DeepPink2 },
                { "DeepPink3", DeepPink3 },
                { "DeepPink4", DeepPink4 },
                { "DeepSkyBlue", DeepSkyBlue },
                { "DeepSkyBlue1", DeepSkyBlue1 },
                { "DeepSkyBlue2", DeepSkyBlue2 },
                { "DeepSkyBlue3", DeepSkyBlue3 },
                { "DeepSkyBlue4", DeepSkyBlue4 },
                { "dim gray", DimGray },
                { "dim grey", DimGrey },
                { "DimGray", DimGray },
                { "DimGrey", DimGrey },
                { "dodger blue", DodgerBlue },
                { "DodgerBlue", DodgerBlue },
                { "DodgerBlue1", DodgerBlue1 },
                { "DodgerBlue2", DodgerBlue2 },
                { "DodgerBlue3", DodgerBlue3 },
                { "DodgerBlue4", DodgerBlue4 },
                { "firebrick", Firebrick },
                { "firebrick1", Firebrick1 },
                { "firebrick2", Firebrick2 },
                { "firebrick3", Firebrick3 },
                { "firebrick4", Firebrick4 },
                { "floral white", FloralWhite },
                { "FloralWhite", FloralWhite },
                { "forest green", ForestGreen },
                { "ForestGreen", ForestGreen },
                { "gainsboro", Gainsboro },
                { "ghost white", GhostWhite },
                { "GhostWhite", GhostWhite },
                { "gold", Gold },
                { "gold1", Gold1 },
                { "gold2", Gold2 },
                { "gold3", Gold3 },
                { "gold4", Gold4 },
                { "goldenrod", Goldenrod },
                { "goldenrod1", Goldenrod1 },
                { "goldenrod2", Goldenrod2 },
                { "goldenrod3", Goldenrod3 },
                { "goldenrod4", Goldenrod4 },
                { "gray", Gray },
                { "gray0", Gray0 },
                { "gray1", Gray1 },
                { "gray10", Gray10 },
                { "gray100", Gray100 },
                { "gray11", Gray11 },
                { "gray12", Gray12 },
                { "gray13", Gray13 },
                { "gray14", Gray14 },
                { "gray15", Gray15 },
                { "gray16", Gray16 },
                { "gray17", Gray17 },
                { "gray18", Gray18 },
                { "gray19", Gray19 },
                { "gray2", Gray2 },
                { "gray20", Gray20 },
                { "gray21", Gray21 },
                { "gray22", Gray22 },
                { "gray23", Gray23 },
                { "gray24", Gray24 },
                { "gray25", Gray25 },
                { "gray26", Gray26 },
                { "gray27", Gray27 },
                { "gray28", Gray28 },
                { "gray29", Gray29 },
                { "gray3", Gray3 },
                { "gray30", Gray30 },
                { "gray31", Gray31 },
                { "gray32", Gray32 },
                { "gray33", Gray33 },
                { "gray34", Gray34 },
                { "gray35", Gray35 },
                { "gray36", Gray36 },
                { "gray37", Gray37 },
                { "gray38", Gray38 },
                { "gray39", Gray39 },
                { "gray4", Gray4 },
                { "gray40", Gray40 },
                { "gray41", Gray41 },
                { "gray42", Gray42 },
                { "gray43", Gray43 },
                { "gray44", Gray44 },
                { "gray45", Gray45 },
                { "gray46", Gray46 },
                { "gray47", Gray47 },
                { "gray48", Gray48 },
                { "gray49", Gray49 },
                { "gray5", Gray5 },
                { "gray50", Gray50 },
                { "gray51", Gray51 },
                { "gray52", Gray52 },
                { "gray53", Gray53 },
                { "gray54", Gray54 },
                { "gray55", Gray55 },
                { "gray56", Gray56 },
                { "gray57", Gray57 },
                { "gray58", Gray58 },
                { "gray59", Gray59 },
                { "gray6", Gray6 },
                { "gray60", Gray60 },
                { "gray61", Gray61 },
                { "gray62", Gray62 },
                { "gray63", Gray63 },
                { "gray64", Gray64 },
                { "gray65", Gray65 },
                { "gray66", Gray66 },
                { "gray67", Gray67 },
                { "gray68", Gray68 },
                { "gray69", Gray69 },
                { "gray7", Gray7 },
                { "gray70", Gray70 },
                { "gray71", Gray71 },
                { "gray72", Gray72 },
                { "gray73", Gray73 },
                { "gray74", Gray74 },
                { "gray75", Gray75 },
                { "gray76", Gray76 },
                { "gray77", Gray77 },
                { "gray78", Gray78 },
                { "gray79", Gray79 },
                { "gray8", Gray8 },
                { "gray80", Gray80 },
                { "gray81", Gray81 },
                { "gray82", Gray82 },
                { "gray83", Gray83 },
                { "gray84", Gray84 },
                { "gray85", Gray85 },
                { "gray86", Gray86 },
                { "gray87", Gray87 },
                { "gray88", Gray88 },
                { "gray89", Gray89 },
                { "gray9", Gray9 },
                { "gray90", Gray90 },
                { "gray91", Gray91 },
                { "gray92", Gray92 },
                { "gray93", Gray93 },
                { "gray94", Gray94 },
                { "gray95", Gray95 },
                { "gray96", Gray96 },
                { "gray97", Gray97 },
                { "gray98", Gray98 },
                { "gray99", Gray99 },
                { "green yellow", GreenYellow },
                { "green", Green },
                { "green1", Green1 },
                { "green2", Green2 },
                { "green3", Green3 },
                { "green4", Green4 },
                { "GreenYellow", GreenYellow },
                { "grey", Grey },
                { "grey0", Grey0 },
                { "grey1", Grey1 },
                { "grey10", Grey10 },
                { "grey100", Grey100 },
                { "grey11", Grey11 },
                { "grey12", Grey12 },
                { "grey13", Grey13 },
                { "grey14", Grey14 },
                { "grey15", Grey15 },
                { "grey16", Grey16 },
                { "grey17", Grey17 },
                { "grey18", Grey18 },
                { "grey19", Grey19 },
                { "grey2", Grey2 },
                { "grey20", Grey20 },
                { "grey21", Grey21 },
                { "grey22", Grey22 },
                { "grey23", Grey23 },
                { "grey24", Grey24 },
                { "grey25", Grey25 },
                { "grey26", Grey26 },
                { "grey27", Grey27 },
                { "grey28", Grey28 },
                { "grey29", Grey29 },
                { "grey3", Grey3 },
                { "grey30", Grey30 },
                { "grey31", Grey31 },
                { "grey32", Grey32 },
                { "grey33", Grey33 },
                { "grey34", Grey34 },
                { "grey35", Grey35 },
                { "grey36", Grey36 },
                { "grey37", Grey37 },
                { "grey38", Grey38 },
                { "grey39", Grey39 },
                { "grey4", Grey4 },
                { "grey40", Grey40 },
                { "grey41", Grey41 },
                { "grey42", Grey42 },
                { "grey43", Grey43 },
                { "grey44", Grey44 },
                { "grey45", Grey45 },
                { "grey46", Grey46 },
                { "grey47", Grey47 },
                { "grey48", Grey48 },
                { "grey49", Grey49 },
                { "grey5", Grey5 },
                { "grey50", Grey50 },
                { "grey51", Grey51 },
                { "grey52", Grey52 },
                { "grey53", Grey53 },
                { "grey54", Grey54 },
                { "grey55", Grey55 },
                { "grey56", Grey56 },
                { "grey57", Grey57 },
                { "grey58", Grey58 },
                { "grey59", Grey59 },
                { "grey6", Grey6 },
                { "grey60", Grey60 },
                { "grey61", Grey61 },
                { "grey62", Grey62 },
                { "grey63", Grey63 },
                { "grey64", Grey64 },
                { "grey65", Grey65 },
                { "grey66", Grey66 },
                { "grey67", Grey67 },
                { "grey68", Grey68 },
                { "grey69", Grey69 },
                { "grey7", Grey7 },
                { "grey70", Grey70 },
                { "grey71", Grey71 },
                { "grey72", Grey72 },
                { "grey73", Grey73 },
                { "grey74", Grey74 },
                { "grey75", Grey75 },
                { "grey76", Grey76 },
                { "grey77", Grey77 },
                { "grey78", Grey78 },
                { "grey79", Grey79 },
                { "grey8", Grey8 },
                { "grey80", Grey80 },
                { "grey81", Grey81 },
                { "grey82", Grey82 },
                { "grey83", Grey83 },
                { "grey84", Grey84 },
                { "grey85", Grey85 },
                { "grey86", Grey86 },
                { "grey87", Grey87 },
                { "grey88", Grey88 },
                { "grey89", Grey89 },
                { "grey9", Grey9 },
                { "grey90", Grey90 },
                { "grey91", Grey91 },
                { "grey92", Grey92 },
                { "grey93", Grey93 },
                { "grey94", Grey94 },
                { "grey95", Grey95 },
                { "grey96", Grey96 },
                { "grey97", Grey97 },
                { "grey98", Grey98 },
                { "grey99", Grey99 },
                { "honeydew", Honeydew },
                { "honeydew1", Honeydew1 },
                { "honeydew2", Honeydew2 },
                { "honeydew3", Honeydew3 },
                { "honeydew4", Honeydew4 },
                { "hot pink", HotPink },
                { "HotPink", HotPink },
                { "HotPink1", HotPink1 },
                { "HotPink2", HotPink2 },
                { "HotPink3", HotPink3 },
                { "HotPink4", HotPink4 },
                { "indian red", IndianRed },
                { "IndianRed", IndianRed },
                { "IndianRed1", IndianRed1 },
                { "IndianRed2", IndianRed2 },
                { "IndianRed3", IndianRed3 },
                { "IndianRed4", IndianRed4 },
                { "ivory", Ivory },
                { "ivory1", Ivory1 },
                { "ivory2", Ivory2 },
                { "ivory3", Ivory3 },
                { "ivory4", Ivory4 },
                { "khaki", Khaki },
                { "khaki1", Khaki1 },
                { "khaki2", Khaki2 },
                { "khaki3", Khaki3 },
                { "khaki4", Khaki4 },
                { "lavender blush", LavenderBlush },
                { "lavender", Lavender },
                { "LavenderBlush", LavenderBlush },
                { "LavenderBlush1", LavenderBlush1 },
                { "LavenderBlush2", LavenderBlush2 },
                { "LavenderBlush3", LavenderBlush3 },
                { "LavenderBlush4", LavenderBlush4 },
                { "lawn green", LawnGreen },
                { "LawnGreen", LawnGreen },
                { "lemon chiffon", LemonChiffon },
                { "LemonChiffon", LemonChiffon },
                { "LemonChiffon1", LemonChiffon1 },
                { "LemonChiffon2", LemonChiffon2 },
                { "LemonChiffon3", LemonChiffon3 },
                { "LemonChiffon4", LemonChiffon4 },
                { "light blue", LightBlue },
                { "light coral", LightCoral },
                { "light cyan", LightCyan },
                { "light goldenrod yellow", LightGoldenrodYellow },
                { "light goldenrod", LightGoldenrod },
                { "light gray", LightGray },
                { "light green", LightGreen },
                { "light grey", LightGrey },
                { "light pink", LightPink },
                { "light salmon", LightSalmon },
                { "light sea green", LightSeaGreen },
                { "light sky blue", LightSkyBlue },
                { "light slate blue", LightSlateBlue },
                { "light slate gray", LightSlateGray },
                { "light slate grey", LightSlateGrey },
                { "light steel blue", LightSteelBlue },
                { "light yellow", LightYellow },
                { "LightBlue", LightBlue },
                { "LightBlue1", LightBlue1 },
                { "LightBlue2", LightBlue2 },
                { "LightBlue3", LightBlue3 },
                { "LightBlue4", LightBlue4 },
                { "LightCoral", LightCoral },
                { "LightCyan", LightCyan },
                { "LightCyan1", LightCyan1 },
                { "LightCyan2", LightCyan2 },
                { "LightCyan3", LightCyan3 },
                { "LightCyan4", LightCyan4 },
                { "LightGoldenrod", LightGoldenrod },
                { "LightGoldenrod1", LightGoldenrod1 },
                { "LightGoldenrod2", LightGoldenrod2 },
                { "LightGoldenrod3", LightGoldenrod3 },
                { "LightGoldenrod4", LightGoldenrod4 },
                { "LightGoldenrodYellow", LightGoldenrodYellow },
                { "LightGray", LightGray },
                { "LightGreen", LightGreen },
                { "LightGrey", LightGrey },
                { "LightPink", LightPink },
                { "LightPink1", LightPink1 },
                { "LightPink2", LightPink2 },
                { "LightPink3", LightPink3 },
                { "LightPink4", LightPink4 },
                { "LightSalmon", LightSalmon },
                { "LightSalmon1", LightSalmon1 },
                { "LightSalmon2", LightSalmon2 },
                { "LightSalmon3", LightSalmon3 },
                { "LightSalmon4", LightSalmon4 },
                { "LightSeaGreen", LightSeaGreen },
                { "LightSkyBlue", LightSkyBlue },
                { "LightSkyBlue1", LightSkyBlue1 },
                { "LightSkyBlue2", LightSkyBlue2 },
                { "LightSkyBlue3", LightSkyBlue3 },
                { "LightSkyBlue4", LightSkyBlue4 },
                { "LightSlateBlue", LightSlateBlue },
                { "LightSlateGray", LightSlateGray },
                { "LightSlateGrey", LightSlateGrey },
                { "LightSteelBlue", LightSteelBlue },
                { "LightSteelBlue1", LightSteelBlue1 },
                { "LightSteelBlue2", LightSteelBlue2 },
                { "LightSteelBlue3", LightSteelBlue3 },
                { "LightSteelBlue4", LightSteelBlue4 },
                { "LightYellow", LightYellow },
                { "LightYellow1", LightYellow1 },
                { "LightYellow2", LightYellow2 },
                { "LightYellow3", LightYellow3 },
                { "LightYellow4", LightYellow4 },
                { "lime green", LimeGreen },
                { "LimeGreen", LimeGreen },
                { "linen", Linen },
                { "magenta", Magenta },
                { "magenta1", Magenta1 },
                { "magenta2", Magenta2 },
                { "magenta3", Magenta3 },
                { "magenta4", Magenta4 },
                { "maroon", Maroon },
                { "maroon1", Maroon1 },
                { "maroon2", Maroon2 },
                { "maroon3", Maroon3 },
                { "maroon4", Maroon4 },
                { "medium aquamarine", MediumAquamarine },
                { "medium blue", MediumBlue },
                { "medium orchid", MediumOrchid },
                { "medium purple", MediumPurple },
                { "medium sea green", MediumSeaGreen },
                { "medium slate blue", MediumSlateBlue },
                { "medium spring green", MediumSpringGreen },
                { "medium turquoise", MediumTurquoise },
                { "medium violet red", MediumVioletRed },
                { "MediumAquamarine", MediumAquamarine },
                { "MediumBlue", MediumBlue },
                { "MediumOrchid", MediumOrchid },
                { "MediumOrchid1", MediumOrchid1 },
                { "MediumOrchid2", MediumOrchid2 },
                { "MediumOrchid3", MediumOrchid3 },
                { "MediumOrchid4", MediumOrchid4 },
                { "MediumPurple", MediumPurple },
                { "MediumPurple1", MediumPurple1 },
                { "MediumPurple2", MediumPurple2 },
                { "MediumPurple3", MediumPurple3 },
                { "MediumPurple4", MediumPurple4 },
                { "MediumSeaGreen", MediumSeaGreen },
                { "MediumSlateBlue", MediumSlateBlue },
                { "MediumSpringGreen", MediumSpringGreen },
                { "MediumTurquoise", MediumTurquoise },
                { "MediumVioletRed", MediumVioletRed },
                { "midnight blue", MidnightBlue },
                { "MidnightBlue", MidnightBlue },
                { "mint cream", MintCream },
                { "MintCream", MintCream },
                { "misty rose", MistyRose },
                { "MistyRose", MistyRose },
                { "MistyRose1", MistyRose1 },
                { "MistyRose2", MistyRose2 },
                { "MistyRose3", MistyRose3 },
                { "MistyRose4", MistyRose4 },
                { "moccasin", Moccasin },
                { "navajo white", NavajoWhite },
                { "NavajoWhite", NavajoWhite },
                { "NavajoWhite1", NavajoWhite1 },
                { "NavajoWhite2", NavajoWhite2 },
                { "NavajoWhite3", NavajoWhite3 },
                { "NavajoWhite4", NavajoWhite4 },
                { "navy blue", NavyBlue },
                { "navy", Navy },
                { "NavyBlue", NavyBlue },
                { "old lace", OldLace },
                { "OldLace", OldLace },
                { "olive drab", OliveDrab },
                { "OliveDrab", OliveDrab },
                { "OliveDrab1", OliveDrab1 },
                { "OliveDrab2", OliveDrab2 },
                { "OliveDrab3", OliveDrab3 },
                { "OliveDrab4", OliveDrab4 },
                { "orange red", OrangeRed },
                { "orange", Orange },
                { "orange1", Orange1 },
                { "orange2", Orange2 },
                { "orange3", Orange3 },
                { "orange4", Orange4 },
                { "OrangeRed", OrangeRed },
                { "OrangeRed1", OrangeRed1 },
                { "OrangeRed2", OrangeRed2 },
                { "OrangeRed3", OrangeRed3 },
                { "OrangeRed4", OrangeRed4 },
                { "orchid", Orchid },
                { "orchid1", Orchid1 },
                { "orchid2", Orchid2 },
                { "orchid3", Orchid3 },
                { "orchid4", Orchid4 },
                { "pale goldenrod", PaleGoldenrod },
                { "pale green", PaleGreen },
                { "pale turquoise", PaleTurquoise },
                { "pale violet red", PaleVioletRed },
                { "PaleGoldenrod", PaleGoldenrod },
                { "PaleGreen", PaleGreen },
                { "PaleGreen1", PaleGreen1 },
                { "PaleGreen2", PaleGreen2 },
                { "PaleGreen3", PaleGreen3 },
                { "PaleGreen4", PaleGreen4 },
                { "PaleTurquoise", PaleTurquoise },
                { "PaleTurquoise1", PaleTurquoise1 },
                { "PaleTurquoise2", PaleTurquoise2 },
                { "PaleTurquoise3", PaleTurquoise3 },
                { "PaleTurquoise4", PaleTurquoise4 },
                { "PaleVioletRed", PaleVioletRed },
                { "PaleVioletRed1", PaleVioletRed1 },
                { "PaleVioletRed2", PaleVioletRed2 },
                { "PaleVioletRed3", PaleVioletRed3 },
                { "PaleVioletRed4", PaleVioletRed4 },
                { "papaya whip", PapayaWhip },
                { "PapayaWhip", PapayaWhip },
                { "peach puff", PeachPuff },
                { "PeachPuff", PeachPuff },
                { "PeachPuff1", PeachPuff1 },
                { "PeachPuff2", PeachPuff2 },
                { "PeachPuff3", PeachPuff3 },
                { "PeachPuff4", PeachPuff4 },
                { "peru", Peru },
                { "pink", Pink },
                { "pink1", Pink1 },
                { "pink2", Pink2 },
                { "pink3", Pink3 },
                { "pink4", Pink4 },
                { "plum", Plum },
                { "plum1", Plum1 },
                { "plum2", Plum2 },
                { "plum3", Plum3 },
                { "plum4", Plum4 },
                { "powder blue", PowderBlue },
                { "PowderBlue", PowderBlue },
                { "purple", Purple },
                { "purple1", Purple1 },
                { "purple2", Purple2 },
                { "purple3", Purple3 },
                { "purple4", Purple4 },
                { "red", Red },
                { "red1", Red1 },
                { "red2", Red2 },
                { "red3", Red3 },
                { "red4", Red4 },
                { "rosy brown", RosyBrown },
                { "RosyBrown", RosyBrown },
                { "RosyBrown1", RosyBrown1 },
                { "RosyBrown2", RosyBrown2 },
                { "RosyBrown3", RosyBrown3 },
                { "RosyBrown4", RosyBrown4 },
                { "royal blue", RoyalBlue },
                { "RoyalBlue", RoyalBlue },
                { "RoyalBlue1", RoyalBlue1 },
                { "RoyalBlue2", RoyalBlue2 },
                { "RoyalBlue3", RoyalBlue3 },
                { "RoyalBlue4", RoyalBlue4 },
                { "saddle brown", SaddleBrown },
                { "SaddleBrown", SaddleBrown },
                { "salmon", Salmon },
                { "salmon1", Salmon1 },
                { "salmon2", Salmon2 },
                { "salmon3", Salmon3 },
                { "salmon4", Salmon4 },
                { "sandy brown", SandyBrown },
                { "SandyBrown", SandyBrown },
                { "sea green", SeaGreen },
                { "SeaGreen", SeaGreen },
                { "SeaGreen1", SeaGreen1 },
                { "SeaGreen2", SeaGreen2 },
                { "SeaGreen3", SeaGreen3 },
                { "SeaGreen4", SeaGreen4 },
                { "seashell", Seashell },
                { "seashell1", Seashell1 },
                { "seashell2", Seashell2 },
                { "seashell3", Seashell3 },
                { "seashell4", Seashell4 },
                { "sienna", Sienna },
                { "sienna1", Sienna1 },
                { "sienna2", Sienna2 },
                { "sienna3", Sienna3 },
                { "sienna4", Sienna4 },
                { "sky blue", SkyBlue },
                { "SkyBlue", SkyBlue },
                { "SkyBlue1", SkyBlue1 },
                { "SkyBlue2", SkyBlue2 },
                { "SkyBlue3", SkyBlue3 },
                { "SkyBlue4", SkyBlue4 },
                { "slate blue", SlateBlue },
                { "slate gray", SlateGray },
                { "slate grey", SlateGrey },
                { "SlateBlue", SlateBlue },
                { "SlateBlue1", SlateBlue1 },
                { "SlateBlue2", SlateBlue2 },
                { "SlateBlue3", SlateBlue3 },
                { "SlateBlue4", SlateBlue4 },
                { "SlateGray", SlateGray },
                { "SlateGray1", SlateGray1 },
                { "SlateGray2", SlateGray2 },
                { "SlateGray3", SlateGray3 },
                { "SlateGray4", SlateGray4 },
                { "SlateGrey", SlateGrey },
                { "snow", Snow },
                { "snow1", Snow1 },
                { "snow2", Snow2 },
                { "snow3", Snow3 },
                { "snow4", Snow4 },
                { "spring green", SpringGreen },
                { "SpringGreen", SpringGreen },
                { "SpringGreen1", SpringGreen1 },
                { "SpringGreen2", SpringGreen2 },
                { "SpringGreen3", SpringGreen3 },
                { "SpringGreen4", SpringGreen4 },
                { "steel blue", SteelBlue },
                { "SteelBlue", SteelBlue },
                { "SteelBlue1", SteelBlue1 },
                { "SteelBlue2", SteelBlue2 },
                { "SteelBlue3", SteelBlue3 },
                { "SteelBlue4", SteelBlue4 },
                { "tan", Tan },
                { "tan1", Tan1 },
                { "tan2", Tan2 },
                { "tan3", Tan3 },
                { "tan4", Tan4 },
                { "thistle", Thistle },
                { "thistle1", Thistle1 },
                { "thistle2", Thistle2 },
                { "thistle3", Thistle3 },
                { "thistle4", Thistle4 },
                { "tomato", Tomato },
                { "tomato1", Tomato1 },
                { "tomato2", Tomato2 },
                { "tomato3", Tomato3 },
                { "tomato4", Tomato4 },
                { "turquoise", Turquoise },
                { "turquoise1", Turquoise1 },
                { "turquoise2", Turquoise2 },
                { "turquoise3", Turquoise3 },
                { "turquoise4", Turquoise4 },
                { "violet red", VioletRed },
                { "violet", Violet },
                { "VioletRed", VioletRed },
                { "VioletRed1", VioletRed1 },
                { "VioletRed2", VioletRed2 },
                { "VioletRed3", VioletRed3 },
                { "VioletRed4", VioletRed4 },
                { "wheat", Wheat },
                { "wheat1", Wheat1 },
                { "wheat2", Wheat2 },
                { "wheat3", Wheat3 },
                { "wheat4", Wheat4 },
                { "white smoke", WhiteSmoke },
                { "white", White },
                { "WhiteSmoke", WhiteSmoke },
                { "yellow green", YellowGreen },
                { "yellow", Yellow },
                { "yellow1", Yellow1 },
                { "yellow2", Yellow2 },
                { "yellow3", Yellow3 },
                { "yellow4", Yellow4 },
                { "YellowGreen", YellowGreen }
            }) {}
        };
        static const named_colors sNamedColors;
        named_colors::const_iterator theColor = sNamedColors.find(neolib::make_ci_string(aName));
        if (theColor != sNamedColors.end())
            return theColor->second;
        return optional_color{};
    }
}