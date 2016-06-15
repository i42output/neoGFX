// colour.cpp
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

#include "neogfx.hpp"
#include <tuple>
#include <iomanip>
#include <boost/lexical_cast.hpp>
#include <neolib/vecarray.hpp>
#include <neolib/string_utils.hpp>
#include "colour.hpp"

namespace neogfx
{
	colour::colour() : iValue{}
	{
	}

	colour::colour(argb aValue) : iValue{aValue}
	{
	}

	colour::colour(component aRed, component aGreen, component aBlue, component aAlpha) :
		iValue((static_cast<argb>(aAlpha) << AlphaShift) |
		(static_cast<argb>(aRed) << RedShift) |
		(static_cast<argb>(aGreen) << GreenShift) |
		(static_cast<argb>(aBlue) << BlueShift)) 
	{
	}

	colour::colour(const std::string& aTextValue) : iValue{}
	{
		if (aTextValue.empty())
			*this = Black;
		else if (aTextValue[0] == L'#')
		{
			iValue = neolib::string_to_unsigned_integer(aTextValue.substr(1), 16);
			if (aTextValue.size() <= 7)
				set_alpha(0xFF);
			else
			{
				auto actualAlpha = blue();
				iValue >>= GreenShift;
				set_alpha(actualAlpha);
			}
		}
		else
		{
			auto namedColour = from_name(aTextValue);
			if (namedColour != boost::none)
				*this = *namedColour;
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
								*this = colour(
									static_cast<uint8_t>(boost::lexical_cast<uint32_t>(moreBits[0])),
									static_cast<uint8_t>(boost::lexical_cast<uint32_t>(moreBits[1])),
									static_cast<uint8_t>(boost::lexical_cast<uint32_t>(moreBits[2])));
							}
						}
						else if (bits[0] == "rgba")
						{
							neolib::vecarray<std::string, 4> moreBits;
							neolib::tokens(bits[1], std::string(","), moreBits, 4, false);
							if (moreBits.size() == 4)
							{
								*this = colour(
									static_cast<uint8_t>(boost::lexical_cast<uint32_t>(moreBits[0])),
									static_cast<uint8_t>(boost::lexical_cast<uint32_t>(moreBits[1])),
									static_cast<uint8_t>(boost::lexical_cast<uint32_t>(moreBits[2])),
									static_cast<uint8_t>(boost::lexical_cast<double>(moreBits[3]) * 255.0));
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

	colour::argb colour::value() const 
	{ 
		return iValue; 
	}

	colour::component colour::alpha() const 
	{ 
		return (iValue >> AlphaShift) & 0xFF; 
	}

	colour::component colour::red() const 
	{ 
		return (iValue >> RedShift) & 0xFF; 
	}

	colour::component colour::green() const 
	{ 
		return (iValue >> GreenShift) & 0xFF; 
	}

	colour::component colour::blue() const 
	{ 
		return (iValue >> BlueShift) & 0xFF; 
	}

	colour& colour::set_alpha(component aNewValue)
	{ 
		*this = colour(red(), green(), blue(), aNewValue); 
		return *this;
	}

	colour& colour::set_red(component aNewValue)
	{ 
		*this = colour(aNewValue, green(), blue(), alpha()); 
		return *this;
	}

	colour& colour::set_green(component aNewValue)
	{ 
		*this = colour(red(), aNewValue, blue(), alpha()); 
		return *this;
	}

	colour& colour::set_blue(component aNewValue)
	{ 
		*this = colour(red(), green(), aNewValue, alpha()); 
		return *this;
	}

	colour colour::with_alpha(component aNewValue) const
	{
		return colour(red(), green(), blue(), aNewValue);
	}

	colour colour::with_red(component aNewValue) const
	{
		return colour(aNewValue, green(), blue(), alpha());
	}

	colour colour::with_green(component aNewValue) const
	{
		return colour(red(), aNewValue, blue(), alpha());
	}

	colour colour::with_blue(component aNewValue) const
	{
		return colour(red(), green(), aNewValue, alpha());
	}

	colour colour::with_combined_alpha(component aNewValue) const
	{
		return colour(red(), green(), blue(), static_cast<component>((alpha() / 255.0 * aNewValue / 255.0) * 255));
	}

	hsl_colour colour::to_hsl() const
	{
		return hsl_colour(*this);
	}

	hsv_colour colour::to_hsv() const
	{
		return hsv_colour(*this);
	}

	double colour::intensity() const
	{ 
		return (static_cast<argb>(red()) + static_cast<argb>(green()) + static_cast<argb>(blue())) / (3.0 * 255.0);
	}

	bool colour::similar_intensity(const colour& aOther, double aThreshold)
	{
		return std::abs(intensity() - aOther.intensity()) <= aThreshold;
	}

	colour colour::mid(const colour& aOther) const
	{
		return colour(
			static_cast<component>((red<double>() + aOther.red<double>()) / 2.0 * 0xFF),
			static_cast<component>((green<double>() + aOther.green<double>()) / 2.0 * 0xFF),
			static_cast<component>((blue<double>() + aOther.blue<double>()) / 2.0 * 0xFF),
			static_cast<component>((alpha<double>() + aOther.alpha<double>()) / 2.0 * 0xFF));
	}

	bool colour::light(component aLightIntensity) const 
	{ 
		return intensity() >= (aLightIntensity / 255.0); 
	}

	bool colour::dark(component aDarkIntensity) const 
	{ 
		return intensity() <= (aDarkIntensity / 255.0); 
	}

	colour& colour::lighten(component aDelta) 
	{ 
		*this += aDelta; 
		return *this; 
	}

	colour& colour::darken(component aDelta) 
	{ 
		*this -= aDelta; 
		return *this; 
	}

	colour colour::lighter(component aDelta) const
	{
		colour ret(*this);
		ret += aDelta;
		return ret;
	}

	colour colour::darker(component aDelta) const
	{
		colour ret(*this);
		ret -= aDelta;
		return ret;
	}

	colour colour::shade(component aDelta) const
	{
		if (light())
			return darker(aDelta);
		else
			return lighter(aDelta);
	}
	
	colour colour::monochrome() const
	{
		component i = static_cast<component>(intensity() * 255.0);
		return colour(i, i, i, alpha());
	}

	colour colour::same_lightness_as(const colour& aOther) const
	{
		hsl_colour temp = to_hsl();
		temp.set_lightness(aOther.to_hsl().lightness());
		return temp.to_rgb();
	}

	colour colour::inverse() const
	{
		return colour(0xFF - red(), 0xFF - green(), 0xFF - blue(), alpha());
	}

	colour& colour::operator+=(component aDelta)
	{
		component newRed = MaxComponetValue - red() < aDelta ? MaxComponetValue : red() + aDelta;
		component newGreen = MaxComponetValue - green() < aDelta ? MaxComponetValue : green() + aDelta;
		component newBlue = MaxComponetValue - blue() < aDelta ? MaxComponetValue : blue() + aDelta;
		*this = colour(newRed, newGreen, newBlue, alpha());
		return *this;
	}

	colour& colour::operator-=(component aDelta)
	{
		component newRed = red() < aDelta ? MinComponetValue : red() - aDelta;
		component newGreen = green() < aDelta ? MinComponetValue : green() - aDelta;
		component newBlue = blue() < aDelta ? MinComponetValue : blue() - aDelta;
		*this = colour(newRed, newGreen, newBlue, alpha());
		return *this;
	}
	
	colour colour::operator~() const 
	{ 
		return colour(~red() & 0xFF, ~green() & 0xFF, ~blue() & 0xFF, alpha()); 
	}

	bool colour::operator==(const colour& aOther) const 
	{ 
		return iValue == aOther.iValue; 
	}

	bool colour::operator!=(const colour& aOther) const 
	{ 
		return iValue != aOther.iValue; 
	}

	bool colour::operator<(const colour& aOther) const
	{
		hsv_colour left = to_hsv();
		hsv_colour right = aOther.to_hsv();
		return std::make_tuple(left.hue(), left.saturation(), left.value()) < std::make_tuple(right.hue(), right.saturation(), right.value());
	}

	std::string colour::to_string() const
	{
		std::ostringstream result;
		result << "rgba(" << static_cast<int>(red()) << ", " << static_cast<int>(green()) << ", " << static_cast<int>(blue()) << ", " << alpha() / 255.0 << ");";
		return result.str();
	}

	std::string colour::to_hex_string() const
	{
		std::ostringstream result;
		result << "#" << std::uppercase << std::hex << std::setfill('0') << std::setw(6) << with_alpha(0).value();
		if (alpha() != 0xFF)
			result << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(alpha());
		return result.str();
	}

	gradient::gradient(const colour& aFrom, const colour& aTo, direction_e aDirection) :
		iFrom(aFrom), 
		iTo(aTo), 
		iDirection(aDirection) 
	{
	}

	gradient::gradient(const colour& aFromTo, direction_e aDirection) :
		iFrom(aFromTo),
		iTo(aFromTo),
		iDirection(aDirection)
	{
	}

	colour gradient::at(coordinate aPos, coordinate aStart, coordinate aEnd) const
	{
		if (aEnd - aStart == 0)
			return iFrom;
		colour::component red, green, blue, alpha;
		if (iFrom.red() < iTo.red())
			red = static_cast<colour::component>(static_cast<uint32_t>(iFrom.red() + (iTo.red() - iFrom.red()) * (aPos - aStart) / (aEnd - aStart)));
		else
			red = static_cast<colour::component>(static_cast<uint32_t>(iFrom.red() - (iFrom.red() - iTo.red()) * (aPos - aStart) / (aEnd - aStart)));
		if (iFrom.green() < iTo.green())
			green = static_cast<colour::component>(static_cast<uint32_t>(iFrom.green() + (iTo.green() - iFrom.green()) * (aPos - aStart) / (aEnd - aStart)));
		else
			green = static_cast<colour::component>(static_cast<uint32_t>(iFrom.green() - (iFrom.green() - iTo.green()) * (aPos - aStart) / (aEnd - aStart)));
		if (iFrom.blue() < iTo.blue())
			blue = static_cast<colour::component>(static_cast<uint32_t>(iFrom.blue() + (iTo.blue() - iFrom.blue()) * (aPos - aStart) / (aEnd - aStart)));
		else
			blue = static_cast<colour::component>(static_cast<uint32_t>(iFrom.blue() - (iFrom.blue() - iTo.blue()) * (aPos - aStart) / (aEnd - aStart)));
		if (iFrom.alpha() < iTo.alpha())
			alpha = static_cast<colour::component>(static_cast<uint32_t>(iFrom.alpha() + (iTo.alpha() - iFrom.alpha()) * (aPos - aStart) / (aEnd - aStart)));
		else
			alpha = static_cast<colour::component>(static_cast<uint32_t>(iFrom.alpha() - (iFrom.alpha() - iTo.alpha()) * (aPos - aStart) / (aEnd - aStart)));
		return colour(red, green, blue, alpha);
	}

	colour gradient::at(double aPos) const
	{
		if (aPos < 0.0 || aPos > 1.0)
			throw bad_position();
		return at(static_cast<coordinate>(aPos * colour::MaxComponetValue), 0, colour::MaxComponetValue);
	}

	const colour& gradient::from() const
	{
		return iFrom;
	}

	colour& gradient::from()
	{
		return iFrom;
	}

	const colour& gradient::to() const
	{
		return iTo;
	}

	colour& gradient::to()
	{
		return iTo;
	}

	gradient gradient::with_alpha(colour::component aAlpha) const
	{
		gradient result = *this;
		result.from() = result.from().with_alpha(aAlpha);
		result.to() = result.to().with_alpha(aAlpha);
		return result;
	}

	gradient gradient::with_combined_alpha(colour::component aAlpha) const
	{
		gradient result = *this;
		result.from() = result.from().with_combined_alpha(aAlpha);
		result.to() = result.to().with_combined_alpha(aAlpha);
		return result;
	}

	gradient::direction_e gradient::direction() const
	{ 
		return iDirection; 
	}

	bool gradient::operator==(const gradient& aOther) const
	{
		return iFrom == aOther.iFrom && iTo == aOther.iTo && iDirection == aOther.iDirection;
	}

	bool gradient::operator!=(const gradient& aOther) const
	{
		return !(*this == aOther);
	}

	bool gradient::operator<(const gradient& aOther) const
	{
		return std::tie(iFrom, iTo, iDirection) < std::tie(aOther.iFrom, aOther.iTo, aOther.iDirection);
	}

	const colour colour::AliceBlue = colour(0xF0, 0xF8, 0xFF);
	const colour colour::AntiqueWhite = colour(0xFA, 0xEB, 0xD7);
	const colour colour::AntiqueWhite1 = colour(0xFF, 0xEF, 0xDB);
	const colour colour::AntiqueWhite2 = colour(0xEE, 0xDF, 0xCC);
	const colour colour::AntiqueWhite3 = colour(0xCD, 0xC0, 0xB0);
	const colour colour::AntiqueWhite4 = colour(0x8B, 0x83, 0x78);
	const colour colour::Aquamarine = colour(0x7F, 0xFF, 0xD4);
	const colour colour::Aquamarine1 = colour(0x7F, 0xFF, 0xD4);
	const colour colour::Aquamarine2 = colour(0x76, 0xEE, 0xC6);
	const colour colour::Aquamarine3 = colour(0x66, 0xCD, 0xAA);
	const colour colour::Aquamarine4 = colour(0x45, 0x8B, 0x74);
	const colour colour::Azure = colour(0xF0, 0xFF, 0xFF);
	const colour colour::Azure1 = colour(0xF0, 0xFF, 0xFF);
	const colour colour::Azure2 = colour(0xE0, 0xEE, 0xEE);
	const colour colour::Azure3 = colour(0xC1, 0xCD, 0xCD);
	const colour colour::Azure4 = colour(0x83, 0x8B, 0x8B);
	const colour colour::Beige = colour(0xF5, 0xF5, 0xDC);
	const colour colour::Bisque = colour(0xFF, 0xE4, 0xC4);
	const colour colour::Bisque1 = colour(0xFF, 0xE4, 0xC4);
	const colour colour::Bisque2 = colour(0xEE, 0xD5, 0xB7);
	const colour colour::Bisque3 = colour(0xCD, 0xB7, 0x9E);
	const colour colour::Bisque4 = colour(0x8B, 0x7D, 0x6B);
	const colour colour::Black = colour(0x00, 0x00, 0x00);
	const colour colour::BlanchedAlmond = colour(0xFF, 0xEB, 0xCD);
	const colour colour::Blue = colour(0x00, 0x00, 0xFF);
	const colour colour::Blue1 = colour(0x00, 0x00, 0xFF);
	const colour colour::Blue2 = colour(0x00, 0x00, 0xEE);
	const colour colour::Blue3 = colour(0x00, 0x00, 0xCD);
	const colour colour::Blue4 = colour(0x00, 0x00, 0x8B);
	const colour colour::BlueViolet = colour(0x8A, 0x2B, 0xE2);
	const colour colour::Brown = colour(0xA5, 0x2A, 0x2A);
	const colour colour::Brown1 = colour(0xFF, 0x40, 0x40);
	const colour colour::Brown2 = colour(0xEE, 0x3B, 0x3B);
	const colour colour::Brown3 = colour(0xCD, 0x33, 0x33);
	const colour colour::Brown4 = colour(0x8B, 0x23, 0x23);
	const colour colour::Burlywood = colour(0xDE, 0xB8, 0x87);
	const colour colour::Burlywood1 = colour(0xFF, 0xD3, 0x9B);
	const colour colour::Burlywood2 = colour(0xEE, 0xC5, 0x91);
	const colour colour::Burlywood3 = colour(0xCD, 0xAA, 0x7D);
	const colour colour::Burlywood4 = colour(0x8B, 0x73, 0x55);
	const colour colour::CadetBlue = colour(0x5F, 0x9E, 0xA0);
	const colour colour::CadetBlue1 = colour(0x98, 0xF5, 0xFF);
	const colour colour::CadetBlue2 = colour(0x8E, 0xE5, 0xEE);
	const colour colour::CadetBlue3 = colour(0x7A, 0xC5, 0xCD);
	const colour colour::CadetBlue4 = colour(0x53, 0x86, 0x8B);
	const colour colour::Chartreuse = colour(0x7F, 0xFF, 0x00);
	const colour colour::Chartreuse1 = colour(0x7F, 0xFF, 0x00);
	const colour colour::Chartreuse2 = colour(0x76, 0xEE, 0x00);
	const colour colour::Chartreuse3 = colour(0x66, 0xCD, 0x00);
	const colour colour::Chartreuse4 = colour(0x45, 0x8B, 0x00);
	const colour colour::Chocolate = colour(0xD2, 0x69, 0x1E);
	const colour colour::Chocolate1 = colour(0xFF, 0x7F, 0x24);
	const colour colour::Chocolate2 = colour(0xEE, 0x76, 0x21);
	const colour colour::Chocolate3 = colour(0xCD, 0x66, 0x1D);
	const colour colour::Chocolate4 = colour(0x8B, 0x45, 0x13);
	const colour colour::Coral = colour(0xFF, 0x7F, 0x50);
	const colour colour::Coral1 = colour(0xFF, 0x72, 0x56);
	const colour colour::Coral2 = colour(0xEE, 0x6A, 0x50);
	const colour colour::Coral3 = colour(0xCD, 0x5B, 0x45);
	const colour colour::Coral4 = colour(0x8B, 0x3E, 0x2F);
	const colour colour::CornflowerBlue = colour(0x64, 0x95, 0xED);
	const colour colour::Cornsilk = colour(0xFF, 0xF8, 0xDC);
	const colour colour::Cornsilk1 = colour(0xFF, 0xF8, 0xDC);
	const colour colour::Cornsilk2 = colour(0xEE, 0xE8, 0xCD);
	const colour colour::Cornsilk3 = colour(0xCD, 0xC8, 0xB1);
	const colour colour::Cornsilk4 = colour(0x8B, 0x88, 0x78);
	const colour colour::Cyan = colour(0x00, 0xFF, 0xFF);
	const colour colour::Cyan1 = colour(0x00, 0xFF, 0xFF);
	const colour colour::Cyan2 = colour(0x00, 0xEE, 0xEE);
	const colour colour::Cyan3 = colour(0x00, 0xCD, 0xCD);
	const colour colour::Cyan4 = colour(0x00, 0x8B, 0x8B);
	const colour colour::DarkBlue = colour(0x00, 0x00, 0x8B);
	const colour colour::DarkCyan = colour(0x00, 0x8B, 0x8B);
	const colour colour::DarkGoldenrod = colour(0xB8, 0x86, 0x0B);
	const colour colour::DarkGoldenrod1 = colour(0xFF, 0xB9, 0x0F);
	const colour colour::DarkGoldenrod2 = colour(0xEE, 0xAD, 0x0E);
	const colour colour::DarkGoldenrod3 = colour(0xCD, 0x95, 0x0C);
	const colour colour::DarkGoldenrod4 = colour(0x8B, 0x65, 0x08);
	const colour colour::DarkGray = colour(0xA9, 0xA9, 0xA9);
	const colour colour::DarkGreen = colour(0x00, 0x64, 0x00);
	const colour colour::DarkGrey = colour(0xA9, 0xA9, 0xA9);
	const colour colour::DarkKhaki = colour(0xBD, 0xB7, 0x6B);
	const colour colour::DarkMagenta = colour(0x8B, 0x00, 0x8B);
	const colour colour::DarkOliveGreen = colour(0x55, 0x6B, 0x2F);
	const colour colour::DarkOliveGreen1 = colour(0xCA, 0xFF, 0x70);
	const colour colour::DarkOliveGreen2 = colour(0xBC, 0xEE, 0x68);
	const colour colour::DarkOliveGreen3 = colour(0xA2, 0xCD, 0x5A);
	const colour colour::DarkOliveGreen4 = colour(0x6E, 0x8B, 0x3D);
	const colour colour::DarkOrange = colour(0xFF, 0x8C, 0x00);
	const colour colour::DarkOrange1 = colour(0xFF, 0x7F, 0x00);
	const colour colour::DarkOrange2 = colour(0xEE, 0x76, 0x00);
	const colour colour::DarkOrange3 = colour(0xCD, 0x66, 0x00);
	const colour colour::DarkOrange4 = colour(0x8B, 0x45, 0x00);
	const colour colour::DarkOrchid = colour(0x99, 0x32, 0xCC);
	const colour colour::DarkOrchid1 = colour(0xBF, 0x3E, 0xFF);
	const colour colour::DarkOrchid2 = colour(0xB2, 0x3A, 0xEE);
	const colour colour::DarkOrchid3 = colour(0x9A, 0x32, 0xCD);
	const colour colour::DarkOrchid4 = colour(0x68, 0x22, 0x8B);
	const colour colour::DarkRed = colour(0x8B, 0x00, 0x00);
	const colour colour::DarkSalmon = colour(0xE9, 0x96, 0x7A);
	const colour colour::DarkSeaGreen = colour(0x8F, 0xBC, 0x8F);
	const colour colour::DarkSeaGreen1 = colour(0xC1, 0xFF, 0xC1);
	const colour colour::DarkSeaGreen2 = colour(0xB4, 0xEE, 0xB4);
	const colour colour::DarkSeaGreen3 = colour(0x9B, 0xCD, 0x9B);
	const colour colour::DarkSeaGreen4 = colour(0x69, 0x8B, 0x69);
	const colour colour::DarkSlateBlue = colour(0x48, 0x3D, 0x8B);
	const colour colour::DarkSlateGray = colour(0x2F, 0x4F, 0x4F);
	const colour colour::DarkSlateGray1 = colour(0x97, 0xFF, 0xFF);
	const colour colour::DarkSlateGray2 = colour(0x8D, 0xEE, 0xEE);
	const colour colour::DarkSlateGray3 = colour(0x79, 0xCD, 0xCD);
	const colour colour::DarkSlateGray4 = colour(0x52, 0x8B, 0x8B);
	const colour colour::DarkSlateGrey = colour(0x2F, 0x4F, 0x4F);
	const colour colour::DarkTurquoise = colour(0x00, 0xCE, 0xD1);
	const colour colour::DarkViolet = colour(0x94, 0x00, 0xD3);
	const colour colour::DebianRed = colour(0xD7, 0x07, 0x51);
	const colour colour::DeepPink = colour(0xFF, 0x14, 0x93);
	const colour colour::DeepPink1 = colour(0xFF, 0x14, 0x93);
	const colour colour::DeepPink2 = colour(0xEE, 0x12, 0x89);
	const colour colour::DeepPink3 = colour(0xCD, 0x10, 0x76);
	const colour colour::DeepPink4 = colour(0x8B, 0x0A, 0x50);
	const colour colour::DeepSkyBlue = colour(0x00, 0xBF, 0xFF);
	const colour colour::DeepSkyBlue1 = colour(0x00, 0xBF, 0xFF);
	const colour colour::DeepSkyBlue2 = colour(0x00, 0xB2, 0xEE);
	const colour colour::DeepSkyBlue3 = colour(0x00, 0x9A, 0xCD);
	const colour colour::DeepSkyBlue4 = colour(0x00, 0x68, 0x8B);
	const colour colour::DimGray = colour(0x69, 0x69, 0x69);
	const colour colour::DimGrey = colour(0x69, 0x69, 0x69);
	const colour colour::DodgerBlue = colour(0x1E, 0x90, 0xFF);
	const colour colour::DodgerBlue1 = colour(0x1E, 0x90, 0xFF);
	const colour colour::DodgerBlue2 = colour(0x1C, 0x86, 0xEE);
	const colour colour::DodgerBlue3 = colour(0x18, 0x74, 0xCD);
	const colour colour::DodgerBlue4 = colour(0x10, 0x4E, 0x8B);
	const colour colour::Firebrick = colour(0xB2, 0x22, 0x22);
	const colour colour::Firebrick1 = colour(0xFF, 0x30, 0x30);
	const colour colour::Firebrick2 = colour(0xEE, 0x2C, 0x2C);
	const colour colour::Firebrick3 = colour(0xCD, 0x26, 0x26);
	const colour colour::Firebrick4 = colour(0x8B, 0x1A, 0x1A);
	const colour colour::FloralWhite = colour(0xFF, 0xFA, 0xF0);
	const colour colour::ForestGreen = colour(0x22, 0x8B, 0x22);
	const colour colour::Gainsboro = colour(0xDC, 0xDC, 0xDC);
	const colour colour::GhostWhite = colour(0xF8, 0xF8, 0xFF);
	const colour colour::Gold = colour(0xFF, 0xD7, 0x00);
	const colour colour::Gold1 = colour(0xFF, 0xD7, 0x00);
	const colour colour::Gold2 = colour(0xEE, 0xC9, 0x00);
	const colour colour::Gold3 = colour(0xCD, 0xAD, 0x00);
	const colour colour::Gold4 = colour(0x8B, 0x75, 0x00);
	const colour colour::Goldenrod = colour(0xDA, 0xA5, 0x20);
	const colour colour::Goldenrod1 = colour(0xFF, 0xC1, 0x25);
	const colour colour::Goldenrod2 = colour(0xEE, 0xB4, 0x22);
	const colour colour::Goldenrod3 = colour(0xCD, 0x9B, 0x1D);
	const colour colour::Goldenrod4 = colour(0x8B, 0x69, 0x14);
	const colour colour::Gray = colour(0xBE, 0xBE, 0xBE);
	const colour colour::Gray0 = colour(0x00, 0x00, 0x00);
	const colour colour::Gray1 = colour(0x03, 0x03, 0x03);
	const colour colour::Gray10 = colour(0x1A, 0x1A, 0x1A);
	const colour colour::Gray100 = colour(0xFF, 0xFF, 0xFF);
	const colour colour::Gray11 = colour(0x1C, 0x1C, 0x1C);
	const colour colour::Gray12 = colour(0x1F, 0x1F, 0x1F);
	const colour colour::Gray13 = colour(0x21, 0x21, 0x21);
	const colour colour::Gray14 = colour(0x24, 0x24, 0x24);
	const colour colour::Gray15 = colour(0x26, 0x26, 0x26);
	const colour colour::Gray16 = colour(0x29, 0x29, 0x29);
	const colour colour::Gray17 = colour(0x2B, 0x2B, 0x2B);
	const colour colour::Gray18 = colour(0x2E, 0x2E, 0x2E);
	const colour colour::Gray19 = colour(0x30, 0x30, 0x30);
	const colour colour::Gray2 = colour(0x05, 0x05, 0x05);
	const colour colour::Gray20 = colour(0x33, 0x33, 0x33);
	const colour colour::Gray21 = colour(0x36, 0x36, 0x36);
	const colour colour::Gray22 = colour(0x38, 0x38, 0x38);
	const colour colour::Gray23 = colour(0x3B, 0x3B, 0x3B);
	const colour colour::Gray24 = colour(0x3D, 0x3D, 0x3D);
	const colour colour::Gray25 = colour(0x40, 0x40, 0x40);
	const colour colour::Gray26 = colour(0x42, 0x42, 0x42);
	const colour colour::Gray27 = colour(0x45, 0x45, 0x45);
	const colour colour::Gray28 = colour(0x47, 0x47, 0x47);
	const colour colour::Gray29 = colour(0x4A, 0x4A, 0x4A);
	const colour colour::Gray3 = colour(0x08, 0x08, 0x08);
	const colour colour::Gray30 = colour(0x4D, 0x4D, 0x4D);
	const colour colour::Gray31 = colour(0x4F, 0x4F, 0x4F);
	const colour colour::Gray32 = colour(0x52, 0x52, 0x52);
	const colour colour::Gray33 = colour(0x54, 0x54, 0x54);
	const colour colour::Gray34 = colour(0x57, 0x57, 0x57);
	const colour colour::Gray35 = colour(0x59, 0x59, 0x59);
	const colour colour::Gray36 = colour(0x5C, 0x5C, 0x5C);
	const colour colour::Gray37 = colour(0x5E, 0x5E, 0x5E);
	const colour colour::Gray38 = colour(0x61, 0x61, 0x61);
	const colour colour::Gray39 = colour(0x63, 0x63, 0x63);
	const colour colour::Gray4 = colour(0x0A, 0x0A, 0x0A);
	const colour colour::Gray40 = colour(0x66, 0x66, 0x66);
	const colour colour::Gray41 = colour(0x69, 0x69, 0x69);
	const colour colour::Gray42 = colour(0x6B, 0x6B, 0x6B);
	const colour colour::Gray43 = colour(0x6E, 0x6E, 0x6E);
	const colour colour::Gray44 = colour(0x70, 0x70, 0x70);
	const colour colour::Gray45 = colour(0x73, 0x73, 0x73);
	const colour colour::Gray46 = colour(0x75, 0x75, 0x75);
	const colour colour::Gray47 = colour(0x78, 0x78, 0x78);
	const colour colour::Gray48 = colour(0x7A, 0x7A, 0x7A);
	const colour colour::Gray49 = colour(0x7D, 0x7D, 0x7D);
	const colour colour::Gray5 = colour(0x0D, 0x0D, 0x0D);
	const colour colour::Gray50 = colour(0x7F, 0x7F, 0x7F);
	const colour colour::Gray51 = colour(0x82, 0x82, 0x82);
	const colour colour::Gray52 = colour(0x85, 0x85, 0x85);
	const colour colour::Gray53 = colour(0x87, 0x87, 0x87);
	const colour colour::Gray54 = colour(0x8A, 0x8A, 0x8A);
	const colour colour::Gray55 = colour(0x8C, 0x8C, 0x8C);
	const colour colour::Gray56 = colour(0x8F, 0x8F, 0x8F);
	const colour colour::Gray57 = colour(0x91, 0x91, 0x91);
	const colour colour::Gray58 = colour(0x94, 0x94, 0x94);
	const colour colour::Gray59 = colour(0x96, 0x96, 0x96);
	const colour colour::Gray6 = colour(0x0F, 0x0F, 0x0F);
	const colour colour::Gray60 = colour(0x99, 0x99, 0x99);
	const colour colour::Gray61 = colour(0x9C, 0x9C, 0x9C);
	const colour colour::Gray62 = colour(0x9E, 0x9E, 0x9E);
	const colour colour::Gray63 = colour(0xA1, 0xA1, 0xA1);
	const colour colour::Gray64 = colour(0xA3, 0xA3, 0xA3);
	const colour colour::Gray65 = colour(0xA6, 0xA6, 0xA6);
	const colour colour::Gray66 = colour(0xA8, 0xA8, 0xA8);
	const colour colour::Gray67 = colour(0xAB, 0xAB, 0xAB);
	const colour colour::Gray68 = colour(0xAD, 0xAD, 0xAD);
	const colour colour::Gray69 = colour(0xB0, 0xB0, 0xB0);
	const colour colour::Gray7 = colour(0x12, 0x12, 0x12);
	const colour colour::Gray70 = colour(0xB3, 0xB3, 0xB3);
	const colour colour::Gray71 = colour(0xB5, 0xB5, 0xB5);
	const colour colour::Gray72 = colour(0xB8, 0xB8, 0xB8);
	const colour colour::Gray73 = colour(0xBA, 0xBA, 0xBA);
	const colour colour::Gray74 = colour(0xBD, 0xBD, 0xBD);
	const colour colour::Gray75 = colour(0xBF, 0xBF, 0xBF);
	const colour colour::Gray76 = colour(0xC2, 0xC2, 0xC2);
	const colour colour::Gray77 = colour(0xC4, 0xC4, 0xC4);
	const colour colour::Gray78 = colour(0xC7, 0xC7, 0xC7);
	const colour colour::Gray79 = colour(0xC9, 0xC9, 0xC9);
	const colour colour::Gray8 = colour(0x14, 0x14, 0x14);
	const colour colour::Gray80 = colour(0xCC, 0xCC, 0xCC);
	const colour colour::Gray81 = colour(0xCF, 0xCF, 0xCF);
	const colour colour::Gray82 = colour(0xD1, 0xD1, 0xD1);
	const colour colour::Gray83 = colour(0xD4, 0xD4, 0xD4);
	const colour colour::Gray84 = colour(0xD6, 0xD6, 0xD6);
	const colour colour::Gray85 = colour(0xD9, 0xD9, 0xD9);
	const colour colour::Gray86 = colour(0xDB, 0xDB, 0xDB);
	const colour colour::Gray87 = colour(0xDE, 0xDE, 0xDE);
	const colour colour::Gray88 = colour(0xE0, 0xE0, 0xE0);
	const colour colour::Gray89 = colour(0xE3, 0xE3, 0xE3);
	const colour colour::Gray9 = colour(0x17, 0x17, 0x17);
	const colour colour::Gray90 = colour(0xE5, 0xE5, 0xE5);
	const colour colour::Gray91 = colour(0xE8, 0xE8, 0xE8);
	const colour colour::Gray92 = colour(0xEB, 0xEB, 0xEB);
	const colour colour::Gray93 = colour(0xED, 0xED, 0xED);
	const colour colour::Gray94 = colour(0xF0, 0xF0, 0xF0);
	const colour colour::Gray95 = colour(0xF2, 0xF2, 0xF2);
	const colour colour::Gray96 = colour(0xF5, 0xF5, 0xF5);
	const colour colour::Gray97 = colour(0xF7, 0xF7, 0xF7);
	const colour colour::Gray98 = colour(0xFA, 0xFA, 0xFA);
	const colour colour::Gray99 = colour(0xFC, 0xFC, 0xFC);
	const colour colour::Green = colour(0x00, 0xFF, 0x00);
	const colour colour::Green1 = colour(0x00, 0xFF, 0x00);
	const colour colour::Green2 = colour(0x00, 0xEE, 0x00);
	const colour colour::Green3 = colour(0x00, 0xCD, 0x00);
	const colour colour::Green4 = colour(0x00, 0x8B, 0x00);
	const colour colour::GreenYellow = colour(0xAD, 0xFF, 0x2F);
	const colour colour::Grey = colour(0xBE, 0xBE, 0xBE);
	const colour colour::Grey0 = colour(0x00, 0x00, 0x00);
	const colour colour::Grey1 = colour(0x03, 0x03, 0x03);
	const colour colour::Grey10 = colour(0x1A, 0x1A, 0x1A);
	const colour colour::Grey100 = colour(0xFF, 0xFF, 0xFF);
	const colour colour::Grey11 = colour(0x1C, 0x1C, 0x1C);
	const colour colour::Grey12 = colour(0x1F, 0x1F, 0x1F);
	const colour colour::Grey13 = colour(0x21, 0x21, 0x21);
	const colour colour::Grey14 = colour(0x24, 0x24, 0x24);
	const colour colour::Grey15 = colour(0x26, 0x26, 0x26);
	const colour colour::Grey16 = colour(0x29, 0x29, 0x29);
	const colour colour::Grey17 = colour(0x2B, 0x2B, 0x2B);
	const colour colour::Grey18 = colour(0x2E, 0x2E, 0x2E);
	const colour colour::Grey19 = colour(0x30, 0x30, 0x30);
	const colour colour::Grey2 = colour(0x05, 0x05, 0x05);
	const colour colour::Grey20 = colour(0x33, 0x33, 0x33);
	const colour colour::Grey21 = colour(0x36, 0x36, 0x36);
	const colour colour::Grey22 = colour(0x38, 0x38, 0x38);
	const colour colour::Grey23 = colour(0x3B, 0x3B, 0x3B);
	const colour colour::Grey24 = colour(0x3D, 0x3D, 0x3D);
	const colour colour::Grey25 = colour(0x40, 0x40, 0x40);
	const colour colour::Grey26 = colour(0x42, 0x42, 0x42);
	const colour colour::Grey27 = colour(0x45, 0x45, 0x45);
	const colour colour::Grey28 = colour(0x47, 0x47, 0x47);
	const colour colour::Grey29 = colour(0x4A, 0x4A, 0x4A);
	const colour colour::Grey3 = colour(0x08, 0x08, 0x08);
	const colour colour::Grey30 = colour(0x4D, 0x4D, 0x4D);
	const colour colour::Grey31 = colour(0x4F, 0x4F, 0x4F);
	const colour colour::Grey32 = colour(0x52, 0x52, 0x52);
	const colour colour::Grey33 = colour(0x54, 0x54, 0x54);
	const colour colour::Grey34 = colour(0x57, 0x57, 0x57);
	const colour colour::Grey35 = colour(0x59, 0x59, 0x59);
	const colour colour::Grey36 = colour(0x5C, 0x5C, 0x5C);
	const colour colour::Grey37 = colour(0x5E, 0x5E, 0x5E);
	const colour colour::Grey38 = colour(0x61, 0x61, 0x61);
	const colour colour::Grey39 = colour(0x63, 0x63, 0x63);
	const colour colour::Grey4 = colour(0x0A, 0x0A, 0x0A);
	const colour colour::Grey40 = colour(0x66, 0x66, 0x66);
	const colour colour::Grey41 = colour(0x69, 0x69, 0x69);
	const colour colour::Grey42 = colour(0x6B, 0x6B, 0x6B);
	const colour colour::Grey43 = colour(0x6E, 0x6E, 0x6E);
	const colour colour::Grey44 = colour(0x70, 0x70, 0x70);
	const colour colour::Grey45 = colour(0x73, 0x73, 0x73);
	const colour colour::Grey46 = colour(0x75, 0x75, 0x75);
	const colour colour::Grey47 = colour(0x78, 0x78, 0x78);
	const colour colour::Grey48 = colour(0x7A, 0x7A, 0x7A);
	const colour colour::Grey49 = colour(0x7D, 0x7D, 0x7D);
	const colour colour::Grey5 = colour(0x0D, 0x0D, 0x0D);
	const colour colour::Grey50 = colour(0x7F, 0x7F, 0x7F);
	const colour colour::Grey51 = colour(0x82, 0x82, 0x82);
	const colour colour::Grey52 = colour(0x85, 0x85, 0x85);
	const colour colour::Grey53 = colour(0x87, 0x87, 0x87);
	const colour colour::Grey54 = colour(0x8A, 0x8A, 0x8A);
	const colour colour::Grey55 = colour(0x8C, 0x8C, 0x8C);
	const colour colour::Grey56 = colour(0x8F, 0x8F, 0x8F);
	const colour colour::Grey57 = colour(0x91, 0x91, 0x91);
	const colour colour::Grey58 = colour(0x94, 0x94, 0x94);
	const colour colour::Grey59 = colour(0x96, 0x96, 0x96);
	const colour colour::Grey6 = colour(0x0F, 0x0F, 0x0F);
	const colour colour::Grey60 = colour(0x99, 0x99, 0x99);
	const colour colour::Grey61 = colour(0x9C, 0x9C, 0x9C);
	const colour colour::Grey62 = colour(0x9E, 0x9E, 0x9E);
	const colour colour::Grey63 = colour(0xA1, 0xA1, 0xA1);
	const colour colour::Grey64 = colour(0xA3, 0xA3, 0xA3);
	const colour colour::Grey65 = colour(0xA6, 0xA6, 0xA6);
	const colour colour::Grey66 = colour(0xA8, 0xA8, 0xA8);
	const colour colour::Grey67 = colour(0xAB, 0xAB, 0xAB);
	const colour colour::Grey68 = colour(0xAD, 0xAD, 0xAD);
	const colour colour::Grey69 = colour(0xB0, 0xB0, 0xB0);
	const colour colour::Grey7 = colour(0x12, 0x12, 0x12);
	const colour colour::Grey70 = colour(0xB3, 0xB3, 0xB3);
	const colour colour::Grey71 = colour(0xB5, 0xB5, 0xB5);
	const colour colour::Grey72 = colour(0xB8, 0xB8, 0xB8);
	const colour colour::Grey73 = colour(0xBA, 0xBA, 0xBA);
	const colour colour::Grey74 = colour(0xBD, 0xBD, 0xBD);
	const colour colour::Grey75 = colour(0xBF, 0xBF, 0xBF);
	const colour colour::Grey76 = colour(0xC2, 0xC2, 0xC2);
	const colour colour::Grey77 = colour(0xC4, 0xC4, 0xC4);
	const colour colour::Grey78 = colour(0xC7, 0xC7, 0xC7);
	const colour colour::Grey79 = colour(0xC9, 0xC9, 0xC9);
	const colour colour::Grey8 = colour(0x14, 0x14, 0x14);
	const colour colour::Grey80 = colour(0xCC, 0xCC, 0xCC);
	const colour colour::Grey81 = colour(0xCF, 0xCF, 0xCF);
	const colour colour::Grey82 = colour(0xD1, 0xD1, 0xD1);
	const colour colour::Grey83 = colour(0xD4, 0xD4, 0xD4);
	const colour colour::Grey84 = colour(0xD6, 0xD6, 0xD6);
	const colour colour::Grey85 = colour(0xD9, 0xD9, 0xD9);
	const colour colour::Grey86 = colour(0xDB, 0xDB, 0xDB);
	const colour colour::Grey87 = colour(0xDE, 0xDE, 0xDE);
	const colour colour::Grey88 = colour(0xE0, 0xE0, 0xE0);
	const colour colour::Grey89 = colour(0xE3, 0xE3, 0xE3);
	const colour colour::Grey9 = colour(0x17, 0x17, 0x17);
	const colour colour::Grey90 = colour(0xE5, 0xE5, 0xE5);
	const colour colour::Grey91 = colour(0xE8, 0xE8, 0xE8);
	const colour colour::Grey92 = colour(0xEB, 0xEB, 0xEB);
	const colour colour::Grey93 = colour(0xED, 0xED, 0xED);
	const colour colour::Grey94 = colour(0xF0, 0xF0, 0xF0);
	const colour colour::Grey95 = colour(0xF2, 0xF2, 0xF2);
	const colour colour::Grey96 = colour(0xF5, 0xF5, 0xF5);
	const colour colour::Grey97 = colour(0xF7, 0xF7, 0xF7);
	const colour colour::Grey98 = colour(0xFA, 0xFA, 0xFA);
	const colour colour::Grey99 = colour(0xFC, 0xFC, 0xFC);
	const colour colour::Honeydew = colour(0xF0, 0xFF, 0xF0);
	const colour colour::Honeydew1 = colour(0xF0, 0xFF, 0xF0);
	const colour colour::Honeydew2 = colour(0xE0, 0xEE, 0xE0);
	const colour colour::Honeydew3 = colour(0xC1, 0xCD, 0xC1);
	const colour colour::Honeydew4 = colour(0x83, 0x8B, 0x83);
	const colour colour::HotPink = colour(0xFF, 0x69, 0xB4);
	const colour colour::HotPink1 = colour(0xFF, 0x6E, 0xB4);
	const colour colour::HotPink2 = colour(0xEE, 0x6A, 0xA7);
	const colour colour::HotPink3 = colour(0xCD, 0x60, 0x90);
	const colour colour::HotPink4 = colour(0x8B, 0x3A, 0x62);
	const colour colour::IndianRed = colour(0xCD, 0x5C, 0x5C);
	const colour colour::IndianRed1 = colour(0xFF, 0x6A, 0x6A);
	const colour colour::IndianRed2 = colour(0xEE, 0x63, 0x63);
	const colour colour::IndianRed3 = colour(0xCD, 0x55, 0x55);
	const colour colour::IndianRed4 = colour(0x8B, 0x3A, 0x3A);
	const colour colour::Ivory = colour(0xFF, 0xFF, 0xF0);
	const colour colour::Ivory1 = colour(0xFF, 0xFF, 0xF0);
	const colour colour::Ivory2 = colour(0xEE, 0xEE, 0xE0);
	const colour colour::Ivory3 = colour(0xCD, 0xCD, 0xC1);
	const colour colour::Ivory4 = colour(0x8B, 0x8B, 0x83);
	const colour colour::Khaki = colour(0xF0, 0xE6, 0x8C);
	const colour colour::Khaki1 = colour(0xFF, 0xF6, 0x8F);
	const colour colour::Khaki2 = colour(0xEE, 0xE6, 0x85);
	const colour colour::Khaki3 = colour(0xCD, 0xC6, 0x73);
	const colour colour::Khaki4 = colour(0x8B, 0x86, 0x4E);
	const colour colour::Lavender = colour(0xE6, 0xE6, 0xFA);
	const colour colour::LavenderBlush = colour(0xFF, 0xF0, 0xF5);
	const colour colour::LavenderBlush1 = colour(0xFF, 0xF0, 0xF5);
	const colour colour::LavenderBlush2 = colour(0xEE, 0xE0, 0xE5);
	const colour colour::LavenderBlush3 = colour(0xCD, 0xC1, 0xC5);
	const colour colour::LavenderBlush4 = colour(0x8B, 0x83, 0x86);
	const colour colour::LawnGreen = colour(0x7C, 0xFC, 0x00);
	const colour colour::LemonChiffon = colour(0xFF, 0xFA, 0xCD);
	const colour colour::LemonChiffon1 = colour(0xFF, 0xFA, 0xCD);
	const colour colour::LemonChiffon2 = colour(0xEE, 0xE9, 0xBF);
	const colour colour::LemonChiffon3 = colour(0xCD, 0xC9, 0xA5);
	const colour colour::LemonChiffon4 = colour(0x8B, 0x89, 0x70);
	const colour colour::LightBlue = colour(0xAD, 0xD8, 0xE6);
	const colour colour::LightBlue1 = colour(0xBF, 0xEF, 0xFF);
	const colour colour::LightBlue2 = colour(0xB2, 0xDF, 0xEE);
	const colour colour::LightBlue3 = colour(0x9A, 0xC0, 0xCD);
	const colour colour::LightBlue4 = colour(0x68, 0x83, 0x8B);
	const colour colour::LightCoral = colour(0xF0, 0x80, 0x80);
	const colour colour::LightCyan = colour(0xE0, 0xFF, 0xFF);
	const colour colour::LightCyan1 = colour(0xE0, 0xFF, 0xFF);
	const colour colour::LightCyan2 = colour(0xD1, 0xEE, 0xEE);
	const colour colour::LightCyan3 = colour(0xB4, 0xCD, 0xCD);
	const colour colour::LightCyan4 = colour(0x7A, 0x8B, 0x8B);
	const colour colour::LightGoldenrod = colour(0xEE, 0xDD, 0x82);
	const colour colour::LightGoldenrod1 = colour(0xFF, 0xEC, 0x8B);
	const colour colour::LightGoldenrod2 = colour(0xEE, 0xDC, 0x82);
	const colour colour::LightGoldenrod3 = colour(0xCD, 0xBE, 0x70);
	const colour colour::LightGoldenrod4 = colour(0x8B, 0x81, 0x4C);
	const colour colour::LightGoldenrodYellow = colour(0xFA, 0xFA, 0xD2);
	const colour colour::LightGray = colour(0xD3, 0xD3, 0xD3);
	const colour colour::LightGreen = colour(0x90, 0xEE, 0x90);
	const colour colour::LightGrey = colour(0xD3, 0xD3, 0xD3);
	const colour colour::LightPink = colour(0xFF, 0xB6, 0xC1);
	const colour colour::LightPink1 = colour(0xFF, 0xAE, 0xB9);
	const colour colour::LightPink2 = colour(0xEE, 0xA2, 0xAD);
	const colour colour::LightPink3 = colour(0xCD, 0x8C, 0x95);
	const colour colour::LightPink4 = colour(0x8B, 0x5F, 0x65);
	const colour colour::LightSalmon = colour(0xFF, 0xA0, 0x7A);
	const colour colour::LightSalmon1 = colour(0xFF, 0xA0, 0x7A);
	const colour colour::LightSalmon2 = colour(0xEE, 0x95, 0x72);
	const colour colour::LightSalmon3 = colour(0xCD, 0x81, 0x62);
	const colour colour::LightSalmon4 = colour(0x8B, 0x57, 0x42);
	const colour colour::LightSeaGreen = colour(0x20, 0xB2, 0xAA);
	const colour colour::LightSkyBlue = colour(0x87, 0xCE, 0xFA);
	const colour colour::LightSkyBlue1 = colour(0xB0, 0xE2, 0xFF);
	const colour colour::LightSkyBlue2 = colour(0xA4, 0xD3, 0xEE);
	const colour colour::LightSkyBlue3 = colour(0x8D, 0xB6, 0xCD);
	const colour colour::LightSkyBlue4 = colour(0x60, 0x7B, 0x8B);
	const colour colour::LightSlateBlue = colour(0x84, 0x70, 0xFF);
	const colour colour::LightSlateGray = colour(0x77, 0x88, 0x99);
	const colour colour::LightSlateGrey = colour(0x77, 0x88, 0x99);
	const colour colour::LightSteelBlue = colour(0xB0, 0xC4, 0xDE);
	const colour colour::LightSteelBlue1 = colour(0xCA, 0xE1, 0xFF);
	const colour colour::LightSteelBlue2 = colour(0xBC, 0xD2, 0xEE);
	const colour colour::LightSteelBlue3 = colour(0xA2, 0xB5, 0xCD);
	const colour colour::LightSteelBlue4 = colour(0x6E, 0x7B, 0x8B);
	const colour colour::LightYellow = colour(0xFF, 0xFF, 0xE0);
	const colour colour::LightYellow1 = colour(0xFF, 0xFF, 0xE0);
	const colour colour::LightYellow2 = colour(0xEE, 0xEE, 0xD1);
	const colour colour::LightYellow3 = colour(0xCD, 0xCD, 0xB4);
	const colour colour::LightYellow4 = colour(0x8B, 0x8B, 0x7A);
	const colour colour::LimeGreen = colour(0x32, 0xCD, 0x32);
	const colour colour::Linen = colour(0xFA, 0xF0, 0xE6);
	const colour colour::Magenta = colour(0xFF, 0x00, 0xFF);
	const colour colour::Magenta1 = colour(0xFF, 0x00, 0xFF);
	const colour colour::Magenta2 = colour(0xEE, 0x00, 0xEE);
	const colour colour::Magenta3 = colour(0xCD, 0x00, 0xCD);
	const colour colour::Magenta4 = colour(0x8B, 0x00, 0x8B);
	const colour colour::Maroon = colour(0xB0, 0x30, 0x60);
	const colour colour::Maroon1 = colour(0xFF, 0x34, 0xB3);
	const colour colour::Maroon2 = colour(0xEE, 0x30, 0xA7);
	const colour colour::Maroon3 = colour(0xCD, 0x29, 0x90);
	const colour colour::Maroon4 = colour(0x8B, 0x1C, 0x62);
	const colour colour::MediumAquamarine = colour(0x66, 0xCD, 0xAA);
	const colour colour::MediumBlue = colour(0x00, 0x00, 0xCD);
	const colour colour::MediumOrchid = colour(0xBA, 0x55, 0xD3);
	const colour colour::MediumOrchid1 = colour(0xE0, 0x66, 0xFF);
	const colour colour::MediumOrchid2 = colour(0xD1, 0x5F, 0xEE);
	const colour colour::MediumOrchid3 = colour(0xB4, 0x52, 0xCD);
	const colour colour::MediumOrchid4 = colour(0x7A, 0x37, 0x8B);
	const colour colour::MediumPurple = colour(0x93, 0x70, 0xDB);
	const colour colour::MediumPurple1 = colour(0xAB, 0x82, 0xFF);
	const colour colour::MediumPurple2 = colour(0x9F, 0x79, 0xEE);
	const colour colour::MediumPurple3 = colour(0x89, 0x68, 0xCD);
	const colour colour::MediumPurple4 = colour(0x5D, 0x47, 0x8B);
	const colour colour::MediumSeaGreen = colour(0x3C, 0xB3, 0x71);
	const colour colour::MediumSlateBlue = colour(0x7B, 0x68, 0xEE);
	const colour colour::MediumSpringGreen = colour(0x00, 0xFA, 0x9A);
	const colour colour::MediumTurquoise = colour(0x48, 0xD1, 0xCC);
	const colour colour::MediumVioletRed = colour(0xC7, 0x15, 0x85);
	const colour colour::MidnightBlue = colour(0x19, 0x19, 0x70);
	const colour colour::MintCream = colour(0xF5, 0xFF, 0xFA);
	const colour colour::MistyRose = colour(0xFF, 0xE4, 0xE1);
	const colour colour::MistyRose1 = colour(0xFF, 0xE4, 0xE1);
	const colour colour::MistyRose2 = colour(0xEE, 0xD5, 0xD2);
	const colour colour::MistyRose3 = colour(0xCD, 0xB7, 0xB5);
	const colour colour::MistyRose4 = colour(0x8B, 0x7D, 0x7B);
	const colour colour::Moccasin = colour(0xFF, 0xE4, 0xB5);
	const colour colour::NavajoWhite = colour(0xFF, 0xDE, 0xAD);
	const colour colour::NavajoWhite1 = colour(0xFF, 0xDE, 0xAD);
	const colour colour::NavajoWhite2 = colour(0xEE, 0xCF, 0xA1);
	const colour colour::NavajoWhite3 = colour(0xCD, 0xB3, 0x8B);
	const colour colour::NavajoWhite4 = colour(0x8B, 0x79, 0x5E);
	const colour colour::Navy = colour(0x00, 0x00, 0x80);
	const colour colour::NavyBlue = colour(0x00, 0x00, 0x80);
	const colour colour::OldLace = colour(0xFD, 0xF5, 0xE6);
	const colour colour::OliveDrab = colour(0x6B, 0x8E, 0x23);
	const colour colour::OliveDrab1 = colour(0xC0, 0xFF, 0x3E);
	const colour colour::OliveDrab2 = colour(0xB3, 0xEE, 0x3A);
	const colour colour::OliveDrab3 = colour(0x9A, 0xCD, 0x32);
	const colour colour::OliveDrab4 = colour(0x69, 0x8B, 0x22);
	const colour colour::Orange = colour(0xFF, 0xA5, 0x00);
	const colour colour::Orange1 = colour(0xFF, 0xA5, 0x00);
	const colour colour::Orange2 = colour(0xEE, 0x9A, 0x00);
	const colour colour::Orange3 = colour(0xCD, 0x85, 0x00);
	const colour colour::Orange4 = colour(0x8B, 0x5A, 0x00);
	const colour colour::OrangeRed = colour(0xFF, 0x45, 0x00);
	const colour colour::OrangeRed1 = colour(0xFF, 0x45, 0x00);
	const colour colour::OrangeRed2 = colour(0xEE, 0x40, 0x00);
	const colour colour::OrangeRed3 = colour(0xCD, 0x37, 0x00);
	const colour colour::OrangeRed4 = colour(0x8B, 0x25, 0x00);
	const colour colour::Orchid = colour(0xDA, 0x70, 0xD6);
	const colour colour::Orchid1 = colour(0xFF, 0x83, 0xFA);
	const colour colour::Orchid2 = colour(0xEE, 0x7A, 0xE9);
	const colour colour::Orchid3 = colour(0xCD, 0x69, 0xC9);
	const colour colour::Orchid4 = colour(0x8B, 0x47, 0x89);
	const colour colour::PaleGoldenrod = colour(0xEE, 0xE8, 0xAA);
	const colour colour::PaleGreen = colour(0x98, 0xFB, 0x98);
	const colour colour::PaleGreen1 = colour(0x9A, 0xFF, 0x9A);
	const colour colour::PaleGreen2 = colour(0x90, 0xEE, 0x90);
	const colour colour::PaleGreen3 = colour(0x7C, 0xCD, 0x7C);
	const colour colour::PaleGreen4 = colour(0x54, 0x8B, 0x54);
	const colour colour::PaleTurquoise = colour(0xAF, 0xEE, 0xEE);
	const colour colour::PaleTurquoise1 = colour(0xBB, 0xFF, 0xFF);
	const colour colour::PaleTurquoise2 = colour(0xAE, 0xEE, 0xEE);
	const colour colour::PaleTurquoise3 = colour(0x96, 0xCD, 0xCD);
	const colour colour::PaleTurquoise4 = colour(0x66, 0x8B, 0x8B);
	const colour colour::PaleVioletRed = colour(0xDB, 0x70, 0x93);
	const colour colour::PaleVioletRed1 = colour(0xFF, 0x82, 0xAB);
	const colour colour::PaleVioletRed2 = colour(0xEE, 0x79, 0x9F);
	const colour colour::PaleVioletRed3 = colour(0xCD, 0x68, 0x89);
	const colour colour::PaleVioletRed4 = colour(0x8B, 0x47, 0x5D);
	const colour colour::PapayaWhip = colour(0xFF, 0xEF, 0xD5);
	const colour colour::PeachPuff = colour(0xFF, 0xDA, 0xB9);
	const colour colour::PeachPuff1 = colour(0xFF, 0xDA, 0xB9);
	const colour colour::PeachPuff2 = colour(0xEE, 0xCB, 0xAD);
	const colour colour::PeachPuff3 = colour(0xCD, 0xAF, 0x95);
	const colour colour::PeachPuff4 = colour(0x8B, 0x77, 0x65);
	const colour colour::Peru = colour(0xCD, 0x85, 0x3F);
	const colour colour::Pink = colour(0xFF, 0xC0, 0xCB);
	const colour colour::Pink1 = colour(0xFF, 0xB5, 0xC5);
	const colour colour::Pink2 = colour(0xEE, 0xA9, 0xB8);
	const colour colour::Pink3 = colour(0xCD, 0x91, 0x9E);
	const colour colour::Pink4 = colour(0x8B, 0x63, 0x6C);
	const colour colour::Plum = colour(0xDD, 0xA0, 0xDD);
	const colour colour::Plum1 = colour(0xFF, 0xBB, 0xFF);
	const colour colour::Plum2 = colour(0xEE, 0xAE, 0xEE);
	const colour colour::Plum3 = colour(0xCD, 0x96, 0xCD);
	const colour colour::Plum4 = colour(0x8B, 0x66, 0x8B);
	const colour colour::PowderBlue = colour(0xB0, 0xE0, 0xE6);
	const colour colour::Purple = colour(0xA0, 0x20, 0xF0);
	const colour colour::Purple1 = colour(0x9B, 0x30, 0xFF);
	const colour colour::Purple2 = colour(0x91, 0x2C, 0xEE);
	const colour colour::Purple3 = colour(0x7D, 0x26, 0xCD);
	const colour colour::Purple4 = colour(0x55, 0x1A, 0x8B);
	const colour colour::Red = colour(0xFF, 0x00, 0x00);
	const colour colour::Red1 = colour(0xFF, 0x00, 0x00);
	const colour colour::Red2 = colour(0xEE, 0x00, 0x00);
	const colour colour::Red3 = colour(0xCD, 0x00, 0x00);
	const colour colour::Red4 = colour(0x8B, 0x00, 0x00);
	const colour colour::RosyBrown = colour(0xBC, 0x8F, 0x8F);
	const colour colour::RosyBrown1 = colour(0xFF, 0xC1, 0xC1);
	const colour colour::RosyBrown2 = colour(0xEE, 0xB4, 0xB4);
	const colour colour::RosyBrown3 = colour(0xCD, 0x9B, 0x9B);
	const colour colour::RosyBrown4 = colour(0x8B, 0x69, 0x69);
	const colour colour::RoyalBlue = colour(0x41, 0x69, 0xE1);
	const colour colour::RoyalBlue1 = colour(0x48, 0x76, 0xFF);
	const colour colour::RoyalBlue2 = colour(0x43, 0x6E, 0xEE);
	const colour colour::RoyalBlue3 = colour(0x3A, 0x5F, 0xCD);
	const colour colour::RoyalBlue4 = colour(0x27, 0x40, 0x8B);
	const colour colour::SaddleBrown = colour(0x8B, 0x45, 0x13);
	const colour colour::Salmon = colour(0xFA, 0x80, 0x72);
	const colour colour::Salmon1 = colour(0xFF, 0x8C, 0x69);
	const colour colour::Salmon2 = colour(0xEE, 0x82, 0x62);
	const colour colour::Salmon3 = colour(0xCD, 0x70, 0x54);
	const colour colour::Salmon4 = colour(0x8B, 0x4C, 0x39);
	const colour colour::SandyBrown = colour(0xF4, 0xA4, 0x60);
	const colour colour::SeaGreen = colour(0x2E, 0x8B, 0x57);
	const colour colour::SeaGreen1 = colour(0x54, 0xFF, 0x9F);
	const colour colour::SeaGreen2 = colour(0x4E, 0xEE, 0x94);
	const colour colour::SeaGreen3 = colour(0x43, 0xCD, 0x80);
	const colour colour::SeaGreen4 = colour(0x2E, 0x8B, 0x57);
	const colour colour::Seashell = colour(0xFF, 0xF5, 0xEE);
	const colour colour::Seashell1 = colour(0xFF, 0xF5, 0xEE);
	const colour colour::Seashell2 = colour(0xEE, 0xE5, 0xDE);
	const colour colour::Seashell3 = colour(0xCD, 0xC5, 0xBF);
	const colour colour::Seashell4 = colour(0x8B, 0x86, 0x82);
	const colour colour::Sienna = colour(0xA0, 0x52, 0x2D);
	const colour colour::Sienna1 = colour(0xFF, 0x82, 0x47);
	const colour colour::Sienna2 = colour(0xEE, 0x79, 0x42);
	const colour colour::Sienna3 = colour(0xCD, 0x68, 0x39);
	const colour colour::Sienna4 = colour(0x8B, 0x47, 0x26);
	const colour colour::SkyBlue = colour(0x87, 0xCE, 0xEB);
	const colour colour::SkyBlue1 = colour(0x87, 0xCE, 0xFF);
	const colour colour::SkyBlue2 = colour(0x7E, 0xC0, 0xEE);
	const colour colour::SkyBlue3 = colour(0x6C, 0xA6, 0xCD);
	const colour colour::SkyBlue4 = colour(0x4A, 0x70, 0x8B);
	const colour colour::SlateBlue = colour(0x6A, 0x5A, 0xCD);
	const colour colour::SlateBlue1 = colour(0x83, 0x6F, 0xFF);
	const colour colour::SlateBlue2 = colour(0x7A, 0x67, 0xEE);
	const colour colour::SlateBlue3 = colour(0x69, 0x59, 0xCD);
	const colour colour::SlateBlue4 = colour(0x47, 0x3C, 0x8B);
	const colour colour::SlateGray = colour(0x70, 0x80, 0x90);
	const colour colour::SlateGray1 = colour(0xC6, 0xE2, 0xFF);
	const colour colour::SlateGray2 = colour(0xB9, 0xD3, 0xEE);
	const colour colour::SlateGray3 = colour(0x9F, 0xB6, 0xCD);
	const colour colour::SlateGray4 = colour(0x6C, 0x7B, 0x8B);
	const colour colour::SlateGrey = colour(0x70, 0x80, 0x90);
	const colour colour::Snow = colour(0xFF, 0xFA, 0xFA);
	const colour colour::Snow1 = colour(0xFF, 0xFA, 0xFA);
	const colour colour::Snow2 = colour(0xEE, 0xE9, 0xE9);
	const colour colour::Snow3 = colour(0xCD, 0xC9, 0xC9);
	const colour colour::Snow4 = colour(0x8B, 0x89, 0x89);
	const colour colour::SpringGreen = colour(0x00, 0xFF, 0x7F);
	const colour colour::SpringGreen1 = colour(0x00, 0xFF, 0x7F);
	const colour colour::SpringGreen2 = colour(0x00, 0xEE, 0x76);
	const colour colour::SpringGreen3 = colour(0x00, 0xCD, 0x66);
	const colour colour::SpringGreen4 = colour(0x00, 0x8B, 0x45);
	const colour colour::SteelBlue = colour(0x46, 0x82, 0xB4);
	const colour colour::SteelBlue1 = colour(0x63, 0xB8, 0xFF);
	const colour colour::SteelBlue2 = colour(0x5C, 0xAC, 0xEE);
	const colour colour::SteelBlue3 = colour(0x4F, 0x94, 0xCD);
	const colour colour::SteelBlue4 = colour(0x36, 0x64, 0x8B);
	const colour colour::Tan = colour(0xD2, 0xB4, 0x8C);
	const colour colour::Tan1 = colour(0xFF, 0xA5, 0x4F);
	const colour colour::Tan2 = colour(0xEE, 0x9A, 0x49);
	const colour colour::Tan3 = colour(0xCD, 0x85, 0x3F);
	const colour colour::Tan4 = colour(0x8B, 0x5A, 0x2B);
	const colour colour::Thistle = colour(0xD8, 0xBF, 0xD8);
	const colour colour::Thistle1 = colour(0xFF, 0xE1, 0xFF);
	const colour colour::Thistle2 = colour(0xEE, 0xD2, 0xEE);
	const colour colour::Thistle3 = colour(0xCD, 0xB5, 0xCD);
	const colour colour::Thistle4 = colour(0x8B, 0x7B, 0x8B);
	const colour colour::Tomato = colour(0xFF, 0x63, 0x47);
	const colour colour::Tomato1 = colour(0xFF, 0x63, 0x47);
	const colour colour::Tomato2 = colour(0xEE, 0x5C, 0x42);
	const colour colour::Tomato3 = colour(0xCD, 0x4F, 0x39);
	const colour colour::Tomato4 = colour(0x8B, 0x36, 0x26);
	const colour colour::Turquoise = colour(0x40, 0xE0, 0xD0);
	const colour colour::Turquoise1 = colour(0x00, 0xF5, 0xFF);
	const colour colour::Turquoise2 = colour(0x00, 0xE5, 0xEE);
	const colour colour::Turquoise3 = colour(0x00, 0xC5, 0xCD);
	const colour colour::Turquoise4 = colour(0x00, 0x86, 0x8B);
	const colour colour::Violet = colour(0xEE, 0x82, 0xEE);
	const colour colour::VioletRed = colour(0xD0, 0x20, 0x90);
	const colour colour::VioletRed1 = colour(0xFF, 0x3E, 0x96);
	const colour colour::VioletRed2 = colour(0xEE, 0x3A, 0x8C);
	const colour colour::VioletRed3 = colour(0xCD, 0x32, 0x78);
	const colour colour::VioletRed4 = colour(0x8B, 0x22, 0x52);
	const colour colour::Wheat = colour(0xF5, 0xDE, 0xB3);
	const colour colour::Wheat1 = colour(0xFF, 0xE7, 0xBA);
	const colour colour::Wheat2 = colour(0xEE, 0xD8, 0xAE);
	const colour colour::Wheat3 = colour(0xCD, 0xBA, 0x96);
	const colour colour::Wheat4 = colour(0x8B, 0x7E, 0x66);
	const colour colour::White = colour(0xFF, 0xFF, 0xFF);
	const colour colour::WhiteSmoke = colour(0xF5, 0xF5, 0xF5);
	const colour colour::Yellow = colour(0xFF, 0xFF, 0x00);
	const colour colour::Yellow1 = colour(0xFF, 0xFF, 0x00);
	const colour colour::Yellow2 = colour(0xEE, 0xEE, 0x00);
	const colour colour::Yellow3 = colour(0xCD, 0xCD, 0x00);
	const colour colour::Yellow4 = colour(0x8B, 0x8B, 0x00);
	const colour colour::YellowGreen = colour(0x9A, 0xCD, 0x32);

	optional_colour colour::from_name(const std::string& aName)
	{
		struct named_colours : public std::map<neolib::ci_string, colour>
		{
			named_colours() : std::map<neolib::ci_string, colour>
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
		static const named_colours sNamedColours;
		named_colours::const_iterator theColour = sNamedColours.find(neolib::make_ci_string(aName));
		if (theColour != sNamedColours.end())
			return theColour->second;
		return optional_colour{};
	}
}