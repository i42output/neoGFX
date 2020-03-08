// color.cpp
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

#include <neogfx/neogfx.hpp>
#include <algorithm>
#include <tuple>
#include <iomanip>
#include <boost/lexical_cast.hpp>
#include <neolib/vecarray.hpp>
#include <neolib/string_ci.hpp>
#include <neogfx/core/numerical.hpp>
#include <neogfx/core/color.hpp>

namespace neogfx
{
    color color::from_hsl(double aHue, double aSaturation, double aLightness, double aAlpha)
    {
        return hsl_color{ aHue, aSaturation, aLightness, aAlpha }.to_rgb();
    }

    color color::from_hsv(double aHue, double aSaturation, double aValue, double aAlpha)
    {
        return hsv_color{ aHue, aSaturation, aValue, aAlpha }.to_rgb();
    }

    color::color() : iValue{}
    {
    }

    color::color(const color& aOther) : iValue{ aOther.iValue }
    {
    }

    color::color(argb aValue) : iValue{ aValue }
    {
    }

    color::color(component aRed, component aGreen, component aBlue, component aAlpha) :
        color{
            (static_cast<argb>(aAlpha) << AlphaShift) |
            (static_cast<argb>(aRed) << RedShift) |
            (static_cast<argb>(aGreen) << GreenShift) |
            (static_cast<argb>(aBlue) << BlueShift) }
    {
    }

    color::color(const vec3& aValue) :
        color{
            static_cast<component>(aValue[0] * 0xFF),
            static_cast<component>(aValue[1] * 0xFF),
            static_cast<component>(aValue[2] * 0xFF),
            static_cast<component>(1.0 * 0xFF) }
    {
    }

    color::color(const vec3f& aValue) :
        color{
            static_cast<component>(aValue[0] * 0xFF),
            static_cast<component>(aValue[1] * 0xFF),
            static_cast<component>(aValue[2] * 0xFF),
            static_cast<component>(1.0 * 0xFF) }
    {
    }

    color::color(const vec4& aValue) :
        color{
            static_cast<component>(aValue[0] * 0xFF),
            static_cast<component>(aValue[1] * 0xFF),
            static_cast<component>(aValue[2] * 0xFF),
            static_cast<component>(aValue[3] * 0xFF) }
    {
    }

    color::color(const vec4f& aValue) :
        color{
            static_cast<component>(aValue[0] * 0xFF),
            static_cast<component>(aValue[1] * 0xFF),
            static_cast<component>(aValue[2] * 0xFF),
            static_cast<component>(aValue[3] * 0xFF) }
    {
    }

    color::color(const std::string& aTextValue) : iValue{}
    {
        if (aTextValue.empty())
            *this = Black;
        else if (aTextValue[0] == L'#')
        {
            iValue = neolib::string_to_uint32(aTextValue.substr(1), 16);
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
                                *this = color(
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
                                *this = color(
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

    color::argb color::value() const 
    { 
        return iValue; 
    }

    color::component color::alpha() const 
    { 
        return (iValue >> AlphaShift) & 0xFF; 
    }

    color::component color::red() const 
    { 
        return (iValue >> RedShift) & 0xFF; 
    }

    color::component color::green() const 
    { 
        return (iValue >> GreenShift) & 0xFF; 
    }

    color::component color::blue() const 
    { 
        return (iValue >> BlueShift) & 0xFF; 
    }

    color& color::set_alpha(component aNewValue)
    { 
        *this = color(red(), green(), blue(), aNewValue); 
        return *this;
    }

    color& color::set_red(component aNewValue)
    { 
        *this = color(aNewValue, green(), blue(), alpha()); 
        return *this;
    }

    color& color::set_green(component aNewValue)
    { 
        *this = color(red(), aNewValue, blue(), alpha()); 
        return *this;
    }

    color& color::set_blue(component aNewValue)
    { 
        *this = color(red(), green(), aNewValue, alpha()); 
        return *this;
    }

    color color::with_alpha(component aNewValue) const
    {
        return color(red(), green(), blue(), aNewValue);
    }

    color color::with_red(component aNewValue) const
    {
        return color(aNewValue, green(), blue(), alpha());
    }

    color color::with_green(component aNewValue) const
    {
        return color(red(), aNewValue, blue(), alpha());
    }

    color color::with_blue(component aNewValue) const
    {
        return color(red(), green(), aNewValue, alpha());
    }

    color color::with_combined_alpha(component aNewValue) const
    {
        return color(red(), green(), blue(), static_cast<component>((alpha() / 255.0 * aNewValue / 255.0) * 255));
    }
    
    color color::with_combined_alpha(double aCoefficient) const
    {
        return color(red(), green(), blue(), static_cast<component>((alpha() / 255.0 * aCoefficient) * 255));
    }

    hsl_color color::to_hsl() const
    {
        return hsl_color(*this);
    }

    hsv_color color::to_hsv() const
    {
        return hsv_color(*this);
    }

    double color::brightness() const
    {
        return std::min(1.0, std::max(0.0, std::sqrt(red<double>() * red<double>() * 0.241 + green<double>() * green<double>() * 0.691 + blue<double>() * blue<double>() * 0.068)));
    }

    double color::intensity() const
    { 
        return (red<double>() + green<double>() + blue<double>()) / 3.0;
    }

    double color::luma() const
    {
        return red<double>() * 0.21 + green<double>() * 0.72 + blue<double>() * 0.07;
    }

    bool color::similar_intensity(const color& aOther, double aThreshold)
    {
        return std::abs(intensity() - aOther.intensity()) <= aThreshold;
    }

    color color::mid(const color& aOther) const
    {
        return color(
            static_cast<component>((red<double>() + aOther.red<double>()) / 2.0 * 0xFF),
            static_cast<component>((green<double>() + aOther.green<double>()) / 2.0 * 0xFF),
            static_cast<component>((blue<double>() + aOther.blue<double>()) / 2.0 * 0xFF),
            static_cast<component>((alpha<double>() + aOther.alpha<double>()) / 2.0 * 0xFF));
    }

    bool color::light(double aThreshold) const
    { 
        return to_hsl().lightness() >= aThreshold;
    }

    bool color::dark(double aThreshold) const
    { 
        return to_hsl().lightness() < aThreshold;
    }

    color& color::lighten(component aDelta) 
    { 
        *this += aDelta; 
        return *this; 
    }

    color& color::darken(component aDelta) 
    { 
        *this -= aDelta; 
        return *this; 
    }

    color color::lighter(component aDelta) const
    {
        color ret(*this);
        ret += aDelta;
        return ret;
    }

    color color::darker(component aDelta) const
    {
        color ret(*this);
        ret -= aDelta;
        return ret;
    }

    color color::shade(component aDelta) const
    {
        if (light())
            return darker(aDelta);
        else
            return lighter(aDelta);
    }
    
    color color::monochrome() const
    {
        component i = static_cast<component>(intensity() * 255.0);
        return color(i, i, i, alpha());
    }

    color color::same_lightness_as(const color& aOther) const
    {
        hsl_color temp = to_hsl();
        temp.set_lightness(aOther.to_hsl().lightness());
        return temp.to_rgb();
    }

    color color::with_lightness(double aLightness) const
    {
        hsl_color temp = to_hsl();
        temp.set_lightness(std::min(aLightness, 1.0));
        return temp.to_rgb();
    }

    color color::inverse() const
    {
        return color(0xFF - red(), 0xFF - green(), 0xFF - blue(), alpha());
    }

    color& color::operator+=(component aDelta)
    {
        component newRed = MaxComponetValue - red() < aDelta ? MaxComponetValue : red() + aDelta;
        component newGreen = MaxComponetValue - green() < aDelta ? MaxComponetValue : green() + aDelta;
        component newBlue = MaxComponetValue - blue() < aDelta ? MaxComponetValue : blue() + aDelta;
        *this = color(newRed, newGreen, newBlue, alpha());
        return *this;
    }

    color& color::operator-=(component aDelta)
    {
        component newRed = red() < aDelta ? MinComponetValue : red() - aDelta;
        component newGreen = green() < aDelta ? MinComponetValue : green() - aDelta;
        component newBlue = blue() < aDelta ? MinComponetValue : blue() - aDelta;
        *this = color(newRed, newGreen, newBlue, alpha());
        return *this;
    }
    
    color color::operator~() const 
    { 
        return color(~red() & 0xFF, ~green() & 0xFF, ~blue() & 0xFF, alpha()); 
    }

    bool color::operator==(const color& aOther) const 
    { 
        return iValue == aOther.iValue; 
    }

    bool color::operator!=(const color& aOther) const 
    { 
        return iValue != aOther.iValue; 
    }

    bool color::operator<(const color& aOther) const
    {
        hsv_color left = to_hsv();
        hsv_color right = aOther.to_hsv();
        return std::make_tuple(left.hue(), left.saturation(), left.value()) < std::make_tuple(right.hue(), right.saturation(), right.value());
    }

    std::string color::to_string() const
    {
        std::ostringstream result;
        result << "rgba(" << static_cast<int>(red()) << ", " << static_cast<int>(green()) << ", " << static_cast<int>(blue()) << ", " << alpha() / 255.0 << ");";
        return result.str();
    }

    std::string color::to_hex_string() const
    {
        std::ostringstream result;
        result << "#" << std::uppercase << std::hex << std::setfill('0') << std::setw(6) << with_alpha(0).value();
        if (alpha() != 0xFF)
            result << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(alpha());
        return result.str();
    }

    vec4 color::to_vec4() const
    {
        return vec4{ red<double>(), green<double>(), blue<double>(), alpha<double>() };
    }

    vec4f color::to_vec4f() const
    {
        return vec4{ red<float>(), green<float>(), blue<float>(), alpha<float>() };
    }

    gradient::gradient() :
        iUseCache{ true },
        iColorStops{ {0.0, color::Black}, {1.0, color::Black} },
        iAlphaStops{ {0.0, 255_u8}, {1.0, 255_u8} },
        iDirection{ gradient_direction::Vertical },
        iOrientation{ corner::TopLeft },
        iShape{ gradient_shape::Ellipse },
        iSize{ gradient_size::FarthestCorner },
        iSmoothness{ 0.0 }
    {
    }

    gradient::gradient(const gradient& aOther) : 
        iUseCache{ aOther.iUseCache },
        iColorStops{ aOther.iColorStops },
        iAlphaStops{ aOther.iAlphaStops },
        iDirection{ aOther.iDirection },
        iOrientation{ aOther.iOrientation },
        iShape{ aOther.iShape },
        iSize{ aOther.iSize },
        iExponents{ aOther.iExponents },
        iCentre{ aOther.iCentre },
        iSmoothness{ aOther.iSmoothness }
    {
    }

    gradient::gradient(const color& aColor) :
        gradient{ aColor, gradient_direction::Vertical }
    {
    }

    gradient::gradient(const color& aColor, gradient_direction aDirection) :
        gradient{ aColor, aColor, aDirection }
    {
    }

    gradient::gradient(const color& aColor1, const color& aColor2, gradient_direction aDirection) :
        iUseCache{ true },
        iColorStops{ {0.0, aColor1}, {1.0, aColor2} },
        iAlphaStops{ {0.0, 255_u8}, {1.0, 255_u8} },
        iDirection{ aDirection },
        iOrientation{ corner::TopLeft },
        iShape{ gradient_shape::Ellipse },
        iSize{ gradient_size::FarthestCorner },
        iSmoothness{ 0.0 }
    { 
    }

    gradient::gradient(const color_stop_list& aColorStops, gradient_direction aDirection) :
        iUseCache{ true },
        iColorStops{ !aColorStops.empty() ? aColorStops : color_stop_list{ {0.0, color::Black}, {1.0, color::Black} } },
        iAlphaStops{ {0.0, 255_u8}, {1.0, 255_u8} },
        iDirection{ aDirection },
        iOrientation{ corner::TopLeft },
        iShape{ gradient_shape::Ellipse },
        iSize{ gradient_size::FarthestCorner },
        iSmoothness{ 0.0 }
    {
        fix();
    }
    
    gradient::gradient(const std::initializer_list<color>& aColors, gradient_direction aDirection) :
        gradient{}
    {
        set_direction(aDirection);
        color_stop_list temp;
        double pos = 0.0;
        for (auto const& c : aColors)
        {
            temp.push_back(color_stop{ pos, c });
            if (aColors.size() > 1)
                pos += (1.0 / (aColors.size() - 1));
        }
        color_stops() = temp;
        fix();
    }


    gradient::gradient(const color_stop_list& aColorStops, const alpha_stop_list& aAlphaStops, gradient_direction aDirection) :
        iUseCache{ true },
        iColorStops{ !aColorStops.empty() ? aColorStops : color_stop_list{ {0.0, color::Black}, {1.0, color::Black} } },
        iAlphaStops{ !aAlphaStops.empty() ? aAlphaStops : alpha_stop_list{ {0.0, 255_u8}, {1.0, 255_u8} } },
        iDirection{ aDirection },
        iOrientation{ corner::TopLeft },
        iShape{ gradient_shape::Ellipse },
        iSize{ gradient_size::FarthestCorner },
        iSmoothness{ 0.0 }
    {
        fix();
    }

    gradient::gradient(const gradient& aOther, const color_stop_list& aColorStops, const alpha_stop_list& aAlphaStops) :
        iUseCache{ aOther.iUseCache },
        iColorStops{ aColorStops },
        iAlphaStops{ aAlphaStops },
        iDirection{ aOther.iDirection },
        iOrientation{ aOther.iOrientation },
        iShape{ aOther.iShape },
        iSize{ aOther.iSize },
        iExponents{ aOther.iExponents },
        iCentre{ aOther.iCentre },
        iSmoothness{ aOther.iSmoothness }
    {
    }

    bool gradient::use_cache() const
    {
        return iUseCache;
    }

    void gradient::set_cache_usage(bool aUseCache)
    {
        iUseCache = aUseCache;
    }

    gradient::color_stop_list::const_iterator gradient::color_begin() const
    {
        return color_stops().begin();
    }

    gradient::color_stop_list::const_iterator gradient::color_end() const
    {
        return color_stops().end();
    }

    gradient::alpha_stop_list::const_iterator gradient::alpha_begin() const
    {
        return alpha_stops().begin();
    }

    gradient::alpha_stop_list::const_iterator gradient::alpha_end() const
    {
        return alpha_stops().end();
    }

    gradient::color_stop_list::iterator gradient::color_begin()
    {
        return color_stops().begin();
    }

    gradient::color_stop_list::iterator gradient::color_end()
    {
        return color_stops().end();
    }

    gradient::alpha_stop_list::iterator gradient::alpha_begin()
    {
        return alpha_stops().begin();
    }

    gradient::alpha_stop_list::iterator gradient::alpha_end()
    {
        return alpha_stops().end();
    }

    gradient::color_stop_list gradient::combined_stops() const
    {
        color_stop_list results;
        results.reserve(color_stops().size() + alpha_stops().size() + 2);
        results.push_back(std::make_pair(0.0, at(0.0)));
        for (const auto& colorStop : color_stops())
            results.push_back(std::make_pair(colorStop.first, at(colorStop.first)));
        for (const auto& alphaStops : alpha_stops())
            results.push_back(std::make_pair(alphaStops.first, at(alphaStops.first)));
        results.push_back(std::make_pair(1.0, at(1.0)));
        std::sort(results.begin(), results.end(), [](const color_stop& aLeft, const color_stop& aRight)
        {
            return aLeft.first < aRight.first;
        });
        results.erase(std::unique(results.begin(), results.end(), [](const color_stop& aLeft, const color_stop& aRight)
        {
            return aLeft.first == aRight.first;
        }), results.end());
        return results;
    }
    
    gradient::color_stop_list::iterator gradient::find_color_stop(double aPos, bool aToInsert)
    {
        auto colorStop = std::lower_bound(color_stops().begin(), color_stops().end(), color_stop{ aPos, color{} },
            [](const color_stop& aLeft, const color_stop& aRight)
        {
            return aLeft.first < aRight.first;
        });
        if (colorStop == color_stops().end() && !aToInsert)
            --colorStop;
        return colorStop;
    }

    gradient::color_stop_list::iterator gradient::find_color_stop(double aPos, double aStart, double aEnd, bool aToInsert)
    {
        return find_color_stop(normalized_position(aPos, aStart, aEnd), aToInsert);
    }

    gradient::alpha_stop_list::iterator gradient::find_alpha_stop(double aPos, bool aToInsert)
    {
        auto alphaStop = std::lower_bound(alpha_stops().begin(), alpha_stops().end(), alpha_stop{ aPos, 255_u8 },
            [](const alpha_stop& aLeft, const alpha_stop& aRight)
        {
            return aLeft.first < aRight.first;
        });
        if (alphaStop == alpha_stops().end() && !aToInsert)
            --alphaStop;
        return alphaStop;
    }

    gradient::alpha_stop_list::iterator gradient::find_alpha_stop(double aPos, double aStart, double aEnd, bool aToInsert)
    {
        return find_alpha_stop(normalized_position(aPos, aStart, aEnd), aToInsert);
    }

    gradient::color_stop_list::iterator gradient::insert_color_stop(double aPos)
    {
        return color_stops().insert(find_color_stop(aPos, true), color_stop(aPos, color_at(aPos)));
    }

    gradient::color_stop_list::iterator gradient::insert_color_stop(double aPos, double aStart, double aEnd)
    {
        return insert_color_stop(normalized_position(aPos, aStart, aEnd));
    }

    gradient::alpha_stop_list::iterator gradient::insert_alpha_stop(double aPos)
    {
        return alpha_stops().insert(find_alpha_stop(aPos, true), alpha_stop(aPos, alpha_at(aPos)));
    }

    gradient::alpha_stop_list::iterator gradient::insert_alpha_stop(double aPos, double aStart, double aEnd)
    {
        return insert_alpha_stop(normalized_position(aPos, aStart, aEnd));
    }

    void gradient::erase_stop(color_stop_list::iterator aStop)
    {
        color_stops().erase(aStop);
        fix();
    }

    void gradient::erase_stop(alpha_stop_list::iterator aStop)
    {
        alpha_stops().erase(aStop);
        fix();
    }

    std::size_t gradient::color_stop_count() const
    {
        return color_stops().size();
    }

    std::size_t gradient::alpha_stop_count() const
    {
        return alpha_stops().size();
    }
    
    color gradient::at(double aPos) const
    {
        color result = color_at(aPos);
        return result.with_combined_alpha(alpha_at(aPos));
    }

    color gradient::at(double aPos, double aStart, double aEnd) const
    {
        return at(normalized_position(aPos, aStart, aEnd));
    }

    color gradient::color_at(double aPos) const
    {
        if (aPos < 0.0 || aPos > 1.0)
            throw bad_position();
        color::component red{}, green{}, blue{}, alpha{};
        auto colorStop = std::lower_bound(color_stops().begin(), color_stops().end(), color_stop{ aPos, color{} },
            [](const color_stop& aLeft, const color_stop& aRight)
        {
            return aLeft.first < aRight.first;
        });
        if (colorStop == color_stops().end())
            --colorStop;
        if (colorStop->first >= aPos && colorStop != color_stops().begin())
            --colorStop;
        auto left = colorStop;
        auto right = colorStop + 1;
        if (right == color_stops().end())
            --right;
        aPos = std::min(std::max(left->first, aPos), right->first);
        double nc = (left != right ? (aPos - left->first) / (right->first - left->first) : 0.0);
        red = lerp(left->second.red(), right->second.red(), nc);
        green = lerp(left->second.green(), right->second.green(), nc);
        blue = lerp(left->second.blue(), right->second.blue(), nc);
        alpha = lerp(left->second.alpha(), right->second.alpha(), nc);
        return color{ red, green, blue, alpha };
    }

    color gradient::color_at(double aPos, double aStart, double aEnd) const
    {
        return color_at(normalized_position(aPos, aStart, aEnd));
    }

    color::component gradient::alpha_at(double aPos) const
    {
        auto alphaStop = std::lower_bound(alpha_stops().begin(), alpha_stops().end(), alpha_stop{ aPos, 255_u8 },
            [](const alpha_stop& aLeft, const alpha_stop& aRight)
        {
            return aLeft.first < aRight.first;
        });
        if (alphaStop == alpha_stops().end())
            --alphaStop;
        if (alphaStop->first >= aPos && alphaStop != alpha_stops().begin())
            --alphaStop;
        auto left = alphaStop;
        auto right= alphaStop + 1;
        if (right == alpha_stops().end())
            --right;
        aPos = std::min(std::max(left->first, aPos), right->first);
        double na = (left != right ? (aPos - left->first) / (right->first - left->first) : 0.0);
        color::component alpha = static_cast<color::component>((lerp(left->second, right->second, na) / 255.0) * 255.0);
        return alpha;
    }

    color::component gradient::alpha_at(double aPos, double aStart, double aEnd) const
    {
        return alpha_at(normalized_position(aPos, aStart, aEnd));
    }

    gradient gradient::with_alpha(color::component aAlpha) const
    {
        gradient result{color_stops(), {{0.0, aAlpha}, {1.0, aAlpha}}, direction()};
        for (auto& stop : result.color_stops())
            stop.second.set_alpha(255);
        return result;
    }

    gradient gradient::with_combined_alpha(color::component aAlpha) const
    {
        gradient result{ color_stops(), alpha_stops(), direction() };
        for (auto& stop : result.alpha_stops())
            stop.second = static_cast<color::component>((stop.second / 255.0 * aAlpha / 255.0) * 255.0);
        return result;
    }

    gradient_direction gradient::direction() const
    { 
        return iDirection; 
    }

    void gradient::set_direction(gradient_direction aDirection)
    {
        iDirection = aDirection;
        if (iDirection != gradient_direction::Diagonal)
            iOrientation = corner::TopLeft;
        if (iDirection != gradient_direction::Radial)
        {
            iExponents = optional_vec2{};
            iCentre = optional_point{};
        }
    }

    gradient gradient::with_direction(gradient_direction aDirection) const
    {
        gradient result = *this;
        result.set_direction(aDirection);
        return result;
    }

    gradient::orientation_type gradient::orientation() const
    {
        return iOrientation;
    }

    void gradient::set_orientation(orientation_type aOrientation)
    {
        iOrientation = aOrientation;
    }

    gradient gradient::with_orientation(orientation_type aOrientation) const
    {
        gradient result = *this;
        result.set_orientation(aOrientation);
        return result;
    }

    gradient_shape gradient::shape() const
    {
        return iShape;
    }

    void gradient::set_shape(gradient_shape aShape)
    {
        iShape = aShape;
    }

    gradient gradient::with_shape(gradient_shape aShape) const
    {
        gradient result = *this;
        result.set_shape(aShape);
        return result;
    }

    gradient_size gradient::size() const
    {
        return iSize;
    }

    void gradient::set_size(gradient_size aSize)
    {
        iSize = aSize;
    }

    gradient gradient::with_size(gradient_size aSize) const
    {
        gradient result = *this;
        result.set_size(aSize);
        return result;
    }

    const optional_vec2& gradient::exponents() const
    {
        return iExponents;
    }

    void gradient::set_exponents(const optional_vec2& aExponents)
    {
        iExponents = aExponents;
        if (iExponents != optional_vec2{})
            iExponents = iExponents->max(vec2{0.0, 0.0});
    }

    gradient gradient::with_exponents(const optional_vec2& aExponents) const
    {
        gradient result = *this;
        result.set_exponents(aExponents);
        return result;
    }

    const optional_point& gradient::centre() const
    {
        return iCentre;
    }

    void gradient::set_centre(const optional_point& aCentre)
    {
        iCentre = aCentre;
        if (iCentre != optional_point{})
            iCentre = iCentre->min(point{ 1.0, 1.0 }).max(point{ -1.0, -1.0 });
    }

    gradient gradient::with_centre(const optional_point& aCentre) const
    {
        gradient result = *this;
        result.set_centre(aCentre);
        return result;
    }

    double gradient::smoothness() const
    {
        return iSmoothness;
    }

    void gradient::set_smoothness(double aSmoothness)
    {
        iSmoothness = aSmoothness;
    }

    gradient gradient::with_smoothness(double aSmoothness) const
    {
        gradient result = *this;
        result.set_smoothness(aSmoothness);
        return result;
    }

    bool gradient::operator==(const gradient& aOther) const
    {
        return color_stops() == aOther.color_stops() &&
            alpha_stops() == aOther.alpha_stops() &&
            direction() == aOther.direction() &&
            orientation() == aOther.orientation() &&
            shape() == aOther.shape() &&
            size() == aOther.size() &&
            exponents() == aOther.exponents() &&
            centre() == aOther.centre() &&
            smoothness() == aOther.smoothness();
    }

    bool gradient::operator!=(const gradient& aOther) const
    {
        return !(*this == aOther);
    }

    bool gradient::operator<(const gradient& aOther) const
    {
        return std::tie(iColorStops, iAlphaStops, iDirection, iOrientation, iShape, iSize, iExponents, iCentre, iSmoothness) < 
            std::tie(aOther.iColorStops, aOther.iAlphaStops, aOther.iDirection, aOther.iOrientation, aOther.iShape, aOther.iSize, aOther.iExponents, aOther.iCentre, aOther.iSmoothness);
    }

    double gradient::normalized_position(double aPos, double aStart, double aEnd)
    {
        if (aStart != aEnd)
            return std::max(0.0, std::min(1.0, (aPos - aStart) / (aEnd - aStart)));
        else
            return 0.0;
    }

    const gradient::color_stop_list& gradient::color_stops() const
    {
        return iColorStops;
    }

    gradient::color_stop_list& gradient::color_stops()
    {
        return iColorStops;
    }

    const gradient::alpha_stop_list& gradient::alpha_stops() const
    {
        return iAlphaStops;
    }

    gradient::alpha_stop_list& gradient::alpha_stops()
    {
        return iAlphaStops;
    }

    void gradient::fix()
    {
        if (color_stops().empty())
        {
            color_stops().assign({ { 0.0, color::Black }, { 1.0, color::Black } });
        }
        else if (color_stops().size() == 1)
        {
            auto stop = color_stops().back();
            color_stops().push_back(stop);
            if (color_stops()[0].first < 1.0)
                color_stops()[1].first = 1.0;
            else
                color_stops()[0].first = 0.0;
        }
        if (alpha_stops().empty())
        {
            alpha_stops().assign({ { 0.0, 255_u8 }, { 1.0, 255_u8 } });
        }
        else if (alpha_stops().size() == 1)
        {
            auto stop = alpha_stops().back();
            alpha_stops().push_back(stop);
            if (alpha_stops()[0].first < 1.0)
                alpha_stops()[1].first = 1.0;
            else
                alpha_stops()[0].first = 0.0;
        }
    }

    const color color::AliceBlue = color(0xF0, 0xF8, 0xFF);
    const color color::AntiqueWhite = color(0xFA, 0xEB, 0xD7);
    const color color::AntiqueWhite1 = color(0xFF, 0xEF, 0xDB);
    const color color::AntiqueWhite2 = color(0xEE, 0xDF, 0xCC);
    const color color::AntiqueWhite3 = color(0xCD, 0xC0, 0xB0);
    const color color::AntiqueWhite4 = color(0x8B, 0x83, 0x78);
    const color color::Aquamarine = color(0x7F, 0xFF, 0xD4);
    const color color::Aquamarine1 = color(0x7F, 0xFF, 0xD4);
    const color color::Aquamarine2 = color(0x76, 0xEE, 0xC6);
    const color color::Aquamarine3 = color(0x66, 0xCD, 0xAA);
    const color color::Aquamarine4 = color(0x45, 0x8B, 0x74);
    const color color::Azure = color(0xF0, 0xFF, 0xFF);
    const color color::Azure1 = color(0xF0, 0xFF, 0xFF);
    const color color::Azure2 = color(0xE0, 0xEE, 0xEE);
    const color color::Azure3 = color(0xC1, 0xCD, 0xCD);
    const color color::Azure4 = color(0x83, 0x8B, 0x8B);
    const color color::Beige = color(0xF5, 0xF5, 0xDC);
    const color color::Bisque = color(0xFF, 0xE4, 0xC4);
    const color color::Bisque1 = color(0xFF, 0xE4, 0xC4);
    const color color::Bisque2 = color(0xEE, 0xD5, 0xB7);
    const color color::Bisque3 = color(0xCD, 0xB7, 0x9E);
    const color color::Bisque4 = color(0x8B, 0x7D, 0x6B);
    const color color::Black = color(0x00, 0x00, 0x00);
    const color color::BlanchedAlmond = color(0xFF, 0xEB, 0xCD);
    const color color::Blue = color(0x00, 0x00, 0xFF);
    const color color::Blue1 = color(0x00, 0x00, 0xFF);
    const color color::Blue2 = color(0x00, 0x00, 0xEE);
    const color color::Blue3 = color(0x00, 0x00, 0xCD);
    const color color::Blue4 = color(0x00, 0x00, 0x8B);
    const color color::BlueViolet = color(0x8A, 0x2B, 0xE2);
    const color color::Brown = color(0xA5, 0x2A, 0x2A);
    const color color::Brown1 = color(0xFF, 0x40, 0x40);
    const color color::Brown2 = color(0xEE, 0x3B, 0x3B);
    const color color::Brown3 = color(0xCD, 0x33, 0x33);
    const color color::Brown4 = color(0x8B, 0x23, 0x23);
    const color color::Burlywood = color(0xDE, 0xB8, 0x87);
    const color color::Burlywood1 = color(0xFF, 0xD3, 0x9B);
    const color color::Burlywood2 = color(0xEE, 0xC5, 0x91);
    const color color::Burlywood3 = color(0xCD, 0xAA, 0x7D);
    const color color::Burlywood4 = color(0x8B, 0x73, 0x55);
    const color color::CadetBlue = color(0x5F, 0x9E, 0xA0);
    const color color::CadetBlue1 = color(0x98, 0xF5, 0xFF);
    const color color::CadetBlue2 = color(0x8E, 0xE5, 0xEE);
    const color color::CadetBlue3 = color(0x7A, 0xC5, 0xCD);
    const color color::CadetBlue4 = color(0x53, 0x86, 0x8B);
    const color color::Chartreuse = color(0x7F, 0xFF, 0x00);
    const color color::Chartreuse1 = color(0x7F, 0xFF, 0x00);
    const color color::Chartreuse2 = color(0x76, 0xEE, 0x00);
    const color color::Chartreuse3 = color(0x66, 0xCD, 0x00);
    const color color::Chartreuse4 = color(0x45, 0x8B, 0x00);
    const color color::Chocolate = color(0xD2, 0x69, 0x1E);
    const color color::Chocolate1 = color(0xFF, 0x7F, 0x24);
    const color color::Chocolate2 = color(0xEE, 0x76, 0x21);
    const color color::Chocolate3 = color(0xCD, 0x66, 0x1D);
    const color color::Chocolate4 = color(0x8B, 0x45, 0x13);
    const color color::Coral = color(0xFF, 0x7F, 0x50);
    const color color::Coral1 = color(0xFF, 0x72, 0x56);
    const color color::Coral2 = color(0xEE, 0x6A, 0x50);
    const color color::Coral3 = color(0xCD, 0x5B, 0x45);
    const color color::Coral4 = color(0x8B, 0x3E, 0x2F);
    const color color::CornflowerBlue = color(0x64, 0x95, 0xED);
    const color color::Cornsilk = color(0xFF, 0xF8, 0xDC);
    const color color::Cornsilk1 = color(0xFF, 0xF8, 0xDC);
    const color color::Cornsilk2 = color(0xEE, 0xE8, 0xCD);
    const color color::Cornsilk3 = color(0xCD, 0xC8, 0xB1);
    const color color::Cornsilk4 = color(0x8B, 0x88, 0x78);
    const color color::Cyan = color(0x00, 0xFF, 0xFF);
    const color color::Cyan1 = color(0x00, 0xFF, 0xFF);
    const color color::Cyan2 = color(0x00, 0xEE, 0xEE);
    const color color::Cyan3 = color(0x00, 0xCD, 0xCD);
    const color color::Cyan4 = color(0x00, 0x8B, 0x8B);
    const color color::DarkBlue = color(0x00, 0x00, 0x8B);
    const color color::DarkCyan = color(0x00, 0x8B, 0x8B);
    const color color::DarkGoldenrod = color(0xB8, 0x86, 0x0B);
    const color color::DarkGoldenrod1 = color(0xFF, 0xB9, 0x0F);
    const color color::DarkGoldenrod2 = color(0xEE, 0xAD, 0x0E);
    const color color::DarkGoldenrod3 = color(0xCD, 0x95, 0x0C);
    const color color::DarkGoldenrod4 = color(0x8B, 0x65, 0x08);
    const color color::DarkGray = color(0xA9, 0xA9, 0xA9);
    const color color::DarkGreen = color(0x00, 0x64, 0x00);
    const color color::DarkGrey = color(0xA9, 0xA9, 0xA9);
    const color color::DarkKhaki = color(0xBD, 0xB7, 0x6B);
    const color color::DarkMagenta = color(0x8B, 0x00, 0x8B);
    const color color::DarkOliveGreen = color(0x55, 0x6B, 0x2F);
    const color color::DarkOliveGreen1 = color(0xCA, 0xFF, 0x70);
    const color color::DarkOliveGreen2 = color(0xBC, 0xEE, 0x68);
    const color color::DarkOliveGreen3 = color(0xA2, 0xCD, 0x5A);
    const color color::DarkOliveGreen4 = color(0x6E, 0x8B, 0x3D);
    const color color::DarkOrange = color(0xFF, 0x8C, 0x00);
    const color color::DarkOrange1 = color(0xFF, 0x7F, 0x00);
    const color color::DarkOrange2 = color(0xEE, 0x76, 0x00);
    const color color::DarkOrange3 = color(0xCD, 0x66, 0x00);
    const color color::DarkOrange4 = color(0x8B, 0x45, 0x00);
    const color color::DarkOrchid = color(0x99, 0x32, 0xCC);
    const color color::DarkOrchid1 = color(0xBF, 0x3E, 0xFF);
    const color color::DarkOrchid2 = color(0xB2, 0x3A, 0xEE);
    const color color::DarkOrchid3 = color(0x9A, 0x32, 0xCD);
    const color color::DarkOrchid4 = color(0x68, 0x22, 0x8B);
    const color color::DarkRed = color(0x8B, 0x00, 0x00);
    const color color::DarkSalmon = color(0xE9, 0x96, 0x7A);
    const color color::DarkSeaGreen = color(0x8F, 0xBC, 0x8F);
    const color color::DarkSeaGreen1 = color(0xC1, 0xFF, 0xC1);
    const color color::DarkSeaGreen2 = color(0xB4, 0xEE, 0xB4);
    const color color::DarkSeaGreen3 = color(0x9B, 0xCD, 0x9B);
    const color color::DarkSeaGreen4 = color(0x69, 0x8B, 0x69);
    const color color::DarkSlateBlue = color(0x48, 0x3D, 0x8B);
    const color color::DarkSlateGray = color(0x2F, 0x4F, 0x4F);
    const color color::DarkSlateGray1 = color(0x97, 0xFF, 0xFF);
    const color color::DarkSlateGray2 = color(0x8D, 0xEE, 0xEE);
    const color color::DarkSlateGray3 = color(0x79, 0xCD, 0xCD);
    const color color::DarkSlateGray4 = color(0x52, 0x8B, 0x8B);
    const color color::DarkSlateGrey = color(0x2F, 0x4F, 0x4F);
    const color color::DarkTurquoise = color(0x00, 0xCE, 0xD1);
    const color color::DarkViolet = color(0x94, 0x00, 0xD3);
    const color color::DebianRed = color(0xD7, 0x07, 0x51);
    const color color::DeepPink = color(0xFF, 0x14, 0x93);
    const color color::DeepPink1 = color(0xFF, 0x14, 0x93);
    const color color::DeepPink2 = color(0xEE, 0x12, 0x89);
    const color color::DeepPink3 = color(0xCD, 0x10, 0x76);
    const color color::DeepPink4 = color(0x8B, 0x0A, 0x50);
    const color color::DeepSkyBlue = color(0x00, 0xBF, 0xFF);
    const color color::DeepSkyBlue1 = color(0x00, 0xBF, 0xFF);
    const color color::DeepSkyBlue2 = color(0x00, 0xB2, 0xEE);
    const color color::DeepSkyBlue3 = color(0x00, 0x9A, 0xCD);
    const color color::DeepSkyBlue4 = color(0x00, 0x68, 0x8B);
    const color color::DimGray = color(0x69, 0x69, 0x69);
    const color color::DimGrey = color(0x69, 0x69, 0x69);
    const color color::DodgerBlue = color(0x1E, 0x90, 0xFF);
    const color color::DodgerBlue1 = color(0x1E, 0x90, 0xFF);
    const color color::DodgerBlue2 = color(0x1C, 0x86, 0xEE);
    const color color::DodgerBlue3 = color(0x18, 0x74, 0xCD);
    const color color::DodgerBlue4 = color(0x10, 0x4E, 0x8B);
    const color color::Firebrick = color(0xB2, 0x22, 0x22);
    const color color::Firebrick1 = color(0xFF, 0x30, 0x30);
    const color color::Firebrick2 = color(0xEE, 0x2C, 0x2C);
    const color color::Firebrick3 = color(0xCD, 0x26, 0x26);
    const color color::Firebrick4 = color(0x8B, 0x1A, 0x1A);
    const color color::FloralWhite = color(0xFF, 0xFA, 0xF0);
    const color color::ForestGreen = color(0x22, 0x8B, 0x22);
    const color color::Gainsboro = color(0xDC, 0xDC, 0xDC);
    const color color::GhostWhite = color(0xF8, 0xF8, 0xFF);
    const color color::Gold = color(0xFF, 0xD7, 0x00);
    const color color::Gold1 = color(0xFF, 0xD7, 0x00);
    const color color::Gold2 = color(0xEE, 0xC9, 0x00);
    const color color::Gold3 = color(0xCD, 0xAD, 0x00);
    const color color::Gold4 = color(0x8B, 0x75, 0x00);
    const color color::Goldenrod = color(0xDA, 0xA5, 0x20);
    const color color::Goldenrod1 = color(0xFF, 0xC1, 0x25);
    const color color::Goldenrod2 = color(0xEE, 0xB4, 0x22);
    const color color::Goldenrod3 = color(0xCD, 0x9B, 0x1D);
    const color color::Goldenrod4 = color(0x8B, 0x69, 0x14);
    const color color::Gray = color(0xBE, 0xBE, 0xBE);
    const color color::Gray0 = color(0x00, 0x00, 0x00);
    const color color::Gray1 = color(0x03, 0x03, 0x03);
    const color color::Gray10 = color(0x1A, 0x1A, 0x1A);
    const color color::Gray100 = color(0xFF, 0xFF, 0xFF);
    const color color::Gray11 = color(0x1C, 0x1C, 0x1C);
    const color color::Gray12 = color(0x1F, 0x1F, 0x1F);
    const color color::Gray13 = color(0x21, 0x21, 0x21);
    const color color::Gray14 = color(0x24, 0x24, 0x24);
    const color color::Gray15 = color(0x26, 0x26, 0x26);
    const color color::Gray16 = color(0x29, 0x29, 0x29);
    const color color::Gray17 = color(0x2B, 0x2B, 0x2B);
    const color color::Gray18 = color(0x2E, 0x2E, 0x2E);
    const color color::Gray19 = color(0x30, 0x30, 0x30);
    const color color::Gray2 = color(0x05, 0x05, 0x05);
    const color color::Gray20 = color(0x33, 0x33, 0x33);
    const color color::Gray21 = color(0x36, 0x36, 0x36);
    const color color::Gray22 = color(0x38, 0x38, 0x38);
    const color color::Gray23 = color(0x3B, 0x3B, 0x3B);
    const color color::Gray24 = color(0x3D, 0x3D, 0x3D);
    const color color::Gray25 = color(0x40, 0x40, 0x40);
    const color color::Gray26 = color(0x42, 0x42, 0x42);
    const color color::Gray27 = color(0x45, 0x45, 0x45);
    const color color::Gray28 = color(0x47, 0x47, 0x47);
    const color color::Gray29 = color(0x4A, 0x4A, 0x4A);
    const color color::Gray3 = color(0x08, 0x08, 0x08);
    const color color::Gray30 = color(0x4D, 0x4D, 0x4D);
    const color color::Gray31 = color(0x4F, 0x4F, 0x4F);
    const color color::Gray32 = color(0x52, 0x52, 0x52);
    const color color::Gray33 = color(0x54, 0x54, 0x54);
    const color color::Gray34 = color(0x57, 0x57, 0x57);
    const color color::Gray35 = color(0x59, 0x59, 0x59);
    const color color::Gray36 = color(0x5C, 0x5C, 0x5C);
    const color color::Gray37 = color(0x5E, 0x5E, 0x5E);
    const color color::Gray38 = color(0x61, 0x61, 0x61);
    const color color::Gray39 = color(0x63, 0x63, 0x63);
    const color color::Gray4 = color(0x0A, 0x0A, 0x0A);
    const color color::Gray40 = color(0x66, 0x66, 0x66);
    const color color::Gray41 = color(0x69, 0x69, 0x69);
    const color color::Gray42 = color(0x6B, 0x6B, 0x6B);
    const color color::Gray43 = color(0x6E, 0x6E, 0x6E);
    const color color::Gray44 = color(0x70, 0x70, 0x70);
    const color color::Gray45 = color(0x73, 0x73, 0x73);
    const color color::Gray46 = color(0x75, 0x75, 0x75);
    const color color::Gray47 = color(0x78, 0x78, 0x78);
    const color color::Gray48 = color(0x7A, 0x7A, 0x7A);
    const color color::Gray49 = color(0x7D, 0x7D, 0x7D);
    const color color::Gray5 = color(0x0D, 0x0D, 0x0D);
    const color color::Gray50 = color(0x7F, 0x7F, 0x7F);
    const color color::Gray51 = color(0x82, 0x82, 0x82);
    const color color::Gray52 = color(0x85, 0x85, 0x85);
    const color color::Gray53 = color(0x87, 0x87, 0x87);
    const color color::Gray54 = color(0x8A, 0x8A, 0x8A);
    const color color::Gray55 = color(0x8C, 0x8C, 0x8C);
    const color color::Gray56 = color(0x8F, 0x8F, 0x8F);
    const color color::Gray57 = color(0x91, 0x91, 0x91);
    const color color::Gray58 = color(0x94, 0x94, 0x94);
    const color color::Gray59 = color(0x96, 0x96, 0x96);
    const color color::Gray6 = color(0x0F, 0x0F, 0x0F);
    const color color::Gray60 = color(0x99, 0x99, 0x99);
    const color color::Gray61 = color(0x9C, 0x9C, 0x9C);
    const color color::Gray62 = color(0x9E, 0x9E, 0x9E);
    const color color::Gray63 = color(0xA1, 0xA1, 0xA1);
    const color color::Gray64 = color(0xA3, 0xA3, 0xA3);
    const color color::Gray65 = color(0xA6, 0xA6, 0xA6);
    const color color::Gray66 = color(0xA8, 0xA8, 0xA8);
    const color color::Gray67 = color(0xAB, 0xAB, 0xAB);
    const color color::Gray68 = color(0xAD, 0xAD, 0xAD);
    const color color::Gray69 = color(0xB0, 0xB0, 0xB0);
    const color color::Gray7 = color(0x12, 0x12, 0x12);
    const color color::Gray70 = color(0xB3, 0xB3, 0xB3);
    const color color::Gray71 = color(0xB5, 0xB5, 0xB5);
    const color color::Gray72 = color(0xB8, 0xB8, 0xB8);
    const color color::Gray73 = color(0xBA, 0xBA, 0xBA);
    const color color::Gray74 = color(0xBD, 0xBD, 0xBD);
    const color color::Gray75 = color(0xBF, 0xBF, 0xBF);
    const color color::Gray76 = color(0xC2, 0xC2, 0xC2);
    const color color::Gray77 = color(0xC4, 0xC4, 0xC4);
    const color color::Gray78 = color(0xC7, 0xC7, 0xC7);
    const color color::Gray79 = color(0xC9, 0xC9, 0xC9);
    const color color::Gray8 = color(0x14, 0x14, 0x14);
    const color color::Gray80 = color(0xCC, 0xCC, 0xCC);
    const color color::Gray81 = color(0xCF, 0xCF, 0xCF);
    const color color::Gray82 = color(0xD1, 0xD1, 0xD1);
    const color color::Gray83 = color(0xD4, 0xD4, 0xD4);
    const color color::Gray84 = color(0xD6, 0xD6, 0xD6);
    const color color::Gray85 = color(0xD9, 0xD9, 0xD9);
    const color color::Gray86 = color(0xDB, 0xDB, 0xDB);
    const color color::Gray87 = color(0xDE, 0xDE, 0xDE);
    const color color::Gray88 = color(0xE0, 0xE0, 0xE0);
    const color color::Gray89 = color(0xE3, 0xE3, 0xE3);
    const color color::Gray9 = color(0x17, 0x17, 0x17);
    const color color::Gray90 = color(0xE5, 0xE5, 0xE5);
    const color color::Gray91 = color(0xE8, 0xE8, 0xE8);
    const color color::Gray92 = color(0xEB, 0xEB, 0xEB);
    const color color::Gray93 = color(0xED, 0xED, 0xED);
    const color color::Gray94 = color(0xF0, 0xF0, 0xF0);
    const color color::Gray95 = color(0xF2, 0xF2, 0xF2);
    const color color::Gray96 = color(0xF5, 0xF5, 0xF5);
    const color color::Gray97 = color(0xF7, 0xF7, 0xF7);
    const color color::Gray98 = color(0xFA, 0xFA, 0xFA);
    const color color::Gray99 = color(0xFC, 0xFC, 0xFC);
    const color color::Green = color(0x00, 0xFF, 0x00);
    const color color::Green1 = color(0x00, 0xFF, 0x00);
    const color color::Green2 = color(0x00, 0xEE, 0x00);
    const color color::Green3 = color(0x00, 0xCD, 0x00);
    const color color::Green4 = color(0x00, 0x8B, 0x00);
    const color color::GreenYellow = color(0xAD, 0xFF, 0x2F);
    const color color::Grey = color(0xBE, 0xBE, 0xBE);
    const color color::Grey0 = color(0x00, 0x00, 0x00);
    const color color::Grey1 = color(0x03, 0x03, 0x03);
    const color color::Grey10 = color(0x1A, 0x1A, 0x1A);
    const color color::Grey100 = color(0xFF, 0xFF, 0xFF);
    const color color::Grey11 = color(0x1C, 0x1C, 0x1C);
    const color color::Grey12 = color(0x1F, 0x1F, 0x1F);
    const color color::Grey13 = color(0x21, 0x21, 0x21);
    const color color::Grey14 = color(0x24, 0x24, 0x24);
    const color color::Grey15 = color(0x26, 0x26, 0x26);
    const color color::Grey16 = color(0x29, 0x29, 0x29);
    const color color::Grey17 = color(0x2B, 0x2B, 0x2B);
    const color color::Grey18 = color(0x2E, 0x2E, 0x2E);
    const color color::Grey19 = color(0x30, 0x30, 0x30);
    const color color::Grey2 = color(0x05, 0x05, 0x05);
    const color color::Grey20 = color(0x33, 0x33, 0x33);
    const color color::Grey21 = color(0x36, 0x36, 0x36);
    const color color::Grey22 = color(0x38, 0x38, 0x38);
    const color color::Grey23 = color(0x3B, 0x3B, 0x3B);
    const color color::Grey24 = color(0x3D, 0x3D, 0x3D);
    const color color::Grey25 = color(0x40, 0x40, 0x40);
    const color color::Grey26 = color(0x42, 0x42, 0x42);
    const color color::Grey27 = color(0x45, 0x45, 0x45);
    const color color::Grey28 = color(0x47, 0x47, 0x47);
    const color color::Grey29 = color(0x4A, 0x4A, 0x4A);
    const color color::Grey3 = color(0x08, 0x08, 0x08);
    const color color::Grey30 = color(0x4D, 0x4D, 0x4D);
    const color color::Grey31 = color(0x4F, 0x4F, 0x4F);
    const color color::Grey32 = color(0x52, 0x52, 0x52);
    const color color::Grey33 = color(0x54, 0x54, 0x54);
    const color color::Grey34 = color(0x57, 0x57, 0x57);
    const color color::Grey35 = color(0x59, 0x59, 0x59);
    const color color::Grey36 = color(0x5C, 0x5C, 0x5C);
    const color color::Grey37 = color(0x5E, 0x5E, 0x5E);
    const color color::Grey38 = color(0x61, 0x61, 0x61);
    const color color::Grey39 = color(0x63, 0x63, 0x63);
    const color color::Grey4 = color(0x0A, 0x0A, 0x0A);
    const color color::Grey40 = color(0x66, 0x66, 0x66);
    const color color::Grey41 = color(0x69, 0x69, 0x69);
    const color color::Grey42 = color(0x6B, 0x6B, 0x6B);
    const color color::Grey43 = color(0x6E, 0x6E, 0x6E);
    const color color::Grey44 = color(0x70, 0x70, 0x70);
    const color color::Grey45 = color(0x73, 0x73, 0x73);
    const color color::Grey46 = color(0x75, 0x75, 0x75);
    const color color::Grey47 = color(0x78, 0x78, 0x78);
    const color color::Grey48 = color(0x7A, 0x7A, 0x7A);
    const color color::Grey49 = color(0x7D, 0x7D, 0x7D);
    const color color::Grey5 = color(0x0D, 0x0D, 0x0D);
    const color color::Grey50 = color(0x7F, 0x7F, 0x7F);
    const color color::Grey51 = color(0x82, 0x82, 0x82);
    const color color::Grey52 = color(0x85, 0x85, 0x85);
    const color color::Grey53 = color(0x87, 0x87, 0x87);
    const color color::Grey54 = color(0x8A, 0x8A, 0x8A);
    const color color::Grey55 = color(0x8C, 0x8C, 0x8C);
    const color color::Grey56 = color(0x8F, 0x8F, 0x8F);
    const color color::Grey57 = color(0x91, 0x91, 0x91);
    const color color::Grey58 = color(0x94, 0x94, 0x94);
    const color color::Grey59 = color(0x96, 0x96, 0x96);
    const color color::Grey6 = color(0x0F, 0x0F, 0x0F);
    const color color::Grey60 = color(0x99, 0x99, 0x99);
    const color color::Grey61 = color(0x9C, 0x9C, 0x9C);
    const color color::Grey62 = color(0x9E, 0x9E, 0x9E);
    const color color::Grey63 = color(0xA1, 0xA1, 0xA1);
    const color color::Grey64 = color(0xA3, 0xA3, 0xA3);
    const color color::Grey65 = color(0xA6, 0xA6, 0xA6);
    const color color::Grey66 = color(0xA8, 0xA8, 0xA8);
    const color color::Grey67 = color(0xAB, 0xAB, 0xAB);
    const color color::Grey68 = color(0xAD, 0xAD, 0xAD);
    const color color::Grey69 = color(0xB0, 0xB0, 0xB0);
    const color color::Grey7 = color(0x12, 0x12, 0x12);
    const color color::Grey70 = color(0xB3, 0xB3, 0xB3);
    const color color::Grey71 = color(0xB5, 0xB5, 0xB5);
    const color color::Grey72 = color(0xB8, 0xB8, 0xB8);
    const color color::Grey73 = color(0xBA, 0xBA, 0xBA);
    const color color::Grey74 = color(0xBD, 0xBD, 0xBD);
    const color color::Grey75 = color(0xBF, 0xBF, 0xBF);
    const color color::Grey76 = color(0xC2, 0xC2, 0xC2);
    const color color::Grey77 = color(0xC4, 0xC4, 0xC4);
    const color color::Grey78 = color(0xC7, 0xC7, 0xC7);
    const color color::Grey79 = color(0xC9, 0xC9, 0xC9);
    const color color::Grey8 = color(0x14, 0x14, 0x14);
    const color color::Grey80 = color(0xCC, 0xCC, 0xCC);
    const color color::Grey81 = color(0xCF, 0xCF, 0xCF);
    const color color::Grey82 = color(0xD1, 0xD1, 0xD1);
    const color color::Grey83 = color(0xD4, 0xD4, 0xD4);
    const color color::Grey84 = color(0xD6, 0xD6, 0xD6);
    const color color::Grey85 = color(0xD9, 0xD9, 0xD9);
    const color color::Grey86 = color(0xDB, 0xDB, 0xDB);
    const color color::Grey87 = color(0xDE, 0xDE, 0xDE);
    const color color::Grey88 = color(0xE0, 0xE0, 0xE0);
    const color color::Grey89 = color(0xE3, 0xE3, 0xE3);
    const color color::Grey9 = color(0x17, 0x17, 0x17);
    const color color::Grey90 = color(0xE5, 0xE5, 0xE5);
    const color color::Grey91 = color(0xE8, 0xE8, 0xE8);
    const color color::Grey92 = color(0xEB, 0xEB, 0xEB);
    const color color::Grey93 = color(0xED, 0xED, 0xED);
    const color color::Grey94 = color(0xF0, 0xF0, 0xF0);
    const color color::Grey95 = color(0xF2, 0xF2, 0xF2);
    const color color::Grey96 = color(0xF5, 0xF5, 0xF5);
    const color color::Grey97 = color(0xF7, 0xF7, 0xF7);
    const color color::Grey98 = color(0xFA, 0xFA, 0xFA);
    const color color::Grey99 = color(0xFC, 0xFC, 0xFC);
    const color color::Honeydew = color(0xF0, 0xFF, 0xF0);
    const color color::Honeydew1 = color(0xF0, 0xFF, 0xF0);
    const color color::Honeydew2 = color(0xE0, 0xEE, 0xE0);
    const color color::Honeydew3 = color(0xC1, 0xCD, 0xC1);
    const color color::Honeydew4 = color(0x83, 0x8B, 0x83);
    const color color::HotPink = color(0xFF, 0x69, 0xB4);
    const color color::HotPink1 = color(0xFF, 0x6E, 0xB4);
    const color color::HotPink2 = color(0xEE, 0x6A, 0xA7);
    const color color::HotPink3 = color(0xCD, 0x60, 0x90);
    const color color::HotPink4 = color(0x8B, 0x3A, 0x62);
    const color color::IndianRed = color(0xCD, 0x5C, 0x5C);
    const color color::IndianRed1 = color(0xFF, 0x6A, 0x6A);
    const color color::IndianRed2 = color(0xEE, 0x63, 0x63);
    const color color::IndianRed3 = color(0xCD, 0x55, 0x55);
    const color color::IndianRed4 = color(0x8B, 0x3A, 0x3A);
    const color color::Ivory = color(0xFF, 0xFF, 0xF0);
    const color color::Ivory1 = color(0xFF, 0xFF, 0xF0);
    const color color::Ivory2 = color(0xEE, 0xEE, 0xE0);
    const color color::Ivory3 = color(0xCD, 0xCD, 0xC1);
    const color color::Ivory4 = color(0x8B, 0x8B, 0x83);
    const color color::Khaki = color(0xF0, 0xE6, 0x8C);
    const color color::Khaki1 = color(0xFF, 0xF6, 0x8F);
    const color color::Khaki2 = color(0xEE, 0xE6, 0x85);
    const color color::Khaki3 = color(0xCD, 0xC6, 0x73);
    const color color::Khaki4 = color(0x8B, 0x86, 0x4E);
    const color color::Lavender = color(0xE6, 0xE6, 0xFA);
    const color color::LavenderBlush = color(0xFF, 0xF0, 0xF5);
    const color color::LavenderBlush1 = color(0xFF, 0xF0, 0xF5);
    const color color::LavenderBlush2 = color(0xEE, 0xE0, 0xE5);
    const color color::LavenderBlush3 = color(0xCD, 0xC1, 0xC5);
    const color color::LavenderBlush4 = color(0x8B, 0x83, 0x86);
    const color color::LawnGreen = color(0x7C, 0xFC, 0x00);
    const color color::LemonChiffon = color(0xFF, 0xFA, 0xCD);
    const color color::LemonChiffon1 = color(0xFF, 0xFA, 0xCD);
    const color color::LemonChiffon2 = color(0xEE, 0xE9, 0xBF);
    const color color::LemonChiffon3 = color(0xCD, 0xC9, 0xA5);
    const color color::LemonChiffon4 = color(0x8B, 0x89, 0x70);
    const color color::LightBlue = color(0xAD, 0xD8, 0xE6);
    const color color::LightBlue1 = color(0xBF, 0xEF, 0xFF);
    const color color::LightBlue2 = color(0xB2, 0xDF, 0xEE);
    const color color::LightBlue3 = color(0x9A, 0xC0, 0xCD);
    const color color::LightBlue4 = color(0x68, 0x83, 0x8B);
    const color color::LightCoral = color(0xF0, 0x80, 0x80);
    const color color::LightCyan = color(0xE0, 0xFF, 0xFF);
    const color color::LightCyan1 = color(0xE0, 0xFF, 0xFF);
    const color color::LightCyan2 = color(0xD1, 0xEE, 0xEE);
    const color color::LightCyan3 = color(0xB4, 0xCD, 0xCD);
    const color color::LightCyan4 = color(0x7A, 0x8B, 0x8B);
    const color color::LightGoldenrod = color(0xEE, 0xDD, 0x82);
    const color color::LightGoldenrod1 = color(0xFF, 0xEC, 0x8B);
    const color color::LightGoldenrod2 = color(0xEE, 0xDC, 0x82);
    const color color::LightGoldenrod3 = color(0xCD, 0xBE, 0x70);
    const color color::LightGoldenrod4 = color(0x8B, 0x81, 0x4C);
    const color color::LightGoldenrodYellow = color(0xFA, 0xFA, 0xD2);
    const color color::LightGray = color(0xD3, 0xD3, 0xD3);
    const color color::LightGreen = color(0x90, 0xEE, 0x90);
    const color color::LightGrey = color(0xD3, 0xD3, 0xD3);
    const color color::LightPink = color(0xFF, 0xB6, 0xC1);
    const color color::LightPink1 = color(0xFF, 0xAE, 0xB9);
    const color color::LightPink2 = color(0xEE, 0xA2, 0xAD);
    const color color::LightPink3 = color(0xCD, 0x8C, 0x95);
    const color color::LightPink4 = color(0x8B, 0x5F, 0x65);
    const color color::LightSalmon = color(0xFF, 0xA0, 0x7A);
    const color color::LightSalmon1 = color(0xFF, 0xA0, 0x7A);
    const color color::LightSalmon2 = color(0xEE, 0x95, 0x72);
    const color color::LightSalmon3 = color(0xCD, 0x81, 0x62);
    const color color::LightSalmon4 = color(0x8B, 0x57, 0x42);
    const color color::LightSeaGreen = color(0x20, 0xB2, 0xAA);
    const color color::LightSkyBlue = color(0x87, 0xCE, 0xFA);
    const color color::LightSkyBlue1 = color(0xB0, 0xE2, 0xFF);
    const color color::LightSkyBlue2 = color(0xA4, 0xD3, 0xEE);
    const color color::LightSkyBlue3 = color(0x8D, 0xB6, 0xCD);
    const color color::LightSkyBlue4 = color(0x60, 0x7B, 0x8B);
    const color color::LightSlateBlue = color(0x84, 0x70, 0xFF);
    const color color::LightSlateGray = color(0x77, 0x88, 0x99);
    const color color::LightSlateGrey = color(0x77, 0x88, 0x99);
    const color color::LightSteelBlue = color(0xB0, 0xC4, 0xDE);
    const color color::LightSteelBlue1 = color(0xCA, 0xE1, 0xFF);
    const color color::LightSteelBlue2 = color(0xBC, 0xD2, 0xEE);
    const color color::LightSteelBlue3 = color(0xA2, 0xB5, 0xCD);
    const color color::LightSteelBlue4 = color(0x6E, 0x7B, 0x8B);
    const color color::LightYellow = color(0xFF, 0xFF, 0xE0);
    const color color::LightYellow1 = color(0xFF, 0xFF, 0xE0);
    const color color::LightYellow2 = color(0xEE, 0xEE, 0xD1);
    const color color::LightYellow3 = color(0xCD, 0xCD, 0xB4);
    const color color::LightYellow4 = color(0x8B, 0x8B, 0x7A);
    const color color::LimeGreen = color(0x32, 0xCD, 0x32);
    const color color::Linen = color(0xFA, 0xF0, 0xE6);
    const color color::Magenta = color(0xFF, 0x00, 0xFF);
    const color color::Magenta1 = color(0xFF, 0x00, 0xFF);
    const color color::Magenta2 = color(0xEE, 0x00, 0xEE);
    const color color::Magenta3 = color(0xCD, 0x00, 0xCD);
    const color color::Magenta4 = color(0x8B, 0x00, 0x8B);
    const color color::Maroon = color(0xB0, 0x30, 0x60);
    const color color::Maroon1 = color(0xFF, 0x34, 0xB3);
    const color color::Maroon2 = color(0xEE, 0x30, 0xA7);
    const color color::Maroon3 = color(0xCD, 0x29, 0x90);
    const color color::Maroon4 = color(0x8B, 0x1C, 0x62);
    const color color::MediumAquamarine = color(0x66, 0xCD, 0xAA);
    const color color::MediumBlue = color(0x00, 0x00, 0xCD);
    const color color::MediumOrchid = color(0xBA, 0x55, 0xD3);
    const color color::MediumOrchid1 = color(0xE0, 0x66, 0xFF);
    const color color::MediumOrchid2 = color(0xD1, 0x5F, 0xEE);
    const color color::MediumOrchid3 = color(0xB4, 0x52, 0xCD);
    const color color::MediumOrchid4 = color(0x7A, 0x37, 0x8B);
    const color color::MediumPurple = color(0x93, 0x70, 0xDB);
    const color color::MediumPurple1 = color(0xAB, 0x82, 0xFF);
    const color color::MediumPurple2 = color(0x9F, 0x79, 0xEE);
    const color color::MediumPurple3 = color(0x89, 0x68, 0xCD);
    const color color::MediumPurple4 = color(0x5D, 0x47, 0x8B);
    const color color::MediumSeaGreen = color(0x3C, 0xB3, 0x71);
    const color color::MediumSlateBlue = color(0x7B, 0x68, 0xEE);
    const color color::MediumSpringGreen = color(0x00, 0xFA, 0x9A);
    const color color::MediumTurquoise = color(0x48, 0xD1, 0xCC);
    const color color::MediumVioletRed = color(0xC7, 0x15, 0x85);
    const color color::MidnightBlue = color(0x19, 0x19, 0x70);
    const color color::MintCream = color(0xF5, 0xFF, 0xFA);
    const color color::MistyRose = color(0xFF, 0xE4, 0xE1);
    const color color::MistyRose1 = color(0xFF, 0xE4, 0xE1);
    const color color::MistyRose2 = color(0xEE, 0xD5, 0xD2);
    const color color::MistyRose3 = color(0xCD, 0xB7, 0xB5);
    const color color::MistyRose4 = color(0x8B, 0x7D, 0x7B);
    const color color::Moccasin = color(0xFF, 0xE4, 0xB5);
    const color color::NavajoWhite = color(0xFF, 0xDE, 0xAD);
    const color color::NavajoWhite1 = color(0xFF, 0xDE, 0xAD);
    const color color::NavajoWhite2 = color(0xEE, 0xCF, 0xA1);
    const color color::NavajoWhite3 = color(0xCD, 0xB3, 0x8B);
    const color color::NavajoWhite4 = color(0x8B, 0x79, 0x5E);
    const color color::Navy = color(0x00, 0x00, 0x80);
    const color color::NavyBlue = color(0x00, 0x00, 0x80);
    const color color::OldLace = color(0xFD, 0xF5, 0xE6);
    const color color::OliveDrab = color(0x6B, 0x8E, 0x23);
    const color color::OliveDrab1 = color(0xC0, 0xFF, 0x3E);
    const color color::OliveDrab2 = color(0xB3, 0xEE, 0x3A);
    const color color::OliveDrab3 = color(0x9A, 0xCD, 0x32);
    const color color::OliveDrab4 = color(0x69, 0x8B, 0x22);
    const color color::Orange = color(0xFF, 0xA5, 0x00);
    const color color::Orange1 = color(0xFF, 0xA5, 0x00);
    const color color::Orange2 = color(0xEE, 0x9A, 0x00);
    const color color::Orange3 = color(0xCD, 0x85, 0x00);
    const color color::Orange4 = color(0x8B, 0x5A, 0x00);
    const color color::OrangeRed = color(0xFF, 0x45, 0x00);
    const color color::OrangeRed1 = color(0xFF, 0x45, 0x00);
    const color color::OrangeRed2 = color(0xEE, 0x40, 0x00);
    const color color::OrangeRed3 = color(0xCD, 0x37, 0x00);
    const color color::OrangeRed4 = color(0x8B, 0x25, 0x00);
    const color color::Orchid = color(0xDA, 0x70, 0xD6);
    const color color::Orchid1 = color(0xFF, 0x83, 0xFA);
    const color color::Orchid2 = color(0xEE, 0x7A, 0xE9);
    const color color::Orchid3 = color(0xCD, 0x69, 0xC9);
    const color color::Orchid4 = color(0x8B, 0x47, 0x89);
    const color color::PaleGoldenrod = color(0xEE, 0xE8, 0xAA);
    const color color::PaleGreen = color(0x98, 0xFB, 0x98);
    const color color::PaleGreen1 = color(0x9A, 0xFF, 0x9A);
    const color color::PaleGreen2 = color(0x90, 0xEE, 0x90);
    const color color::PaleGreen3 = color(0x7C, 0xCD, 0x7C);
    const color color::PaleGreen4 = color(0x54, 0x8B, 0x54);
    const color color::PaleTurquoise = color(0xAF, 0xEE, 0xEE);
    const color color::PaleTurquoise1 = color(0xBB, 0xFF, 0xFF);
    const color color::PaleTurquoise2 = color(0xAE, 0xEE, 0xEE);
    const color color::PaleTurquoise3 = color(0x96, 0xCD, 0xCD);
    const color color::PaleTurquoise4 = color(0x66, 0x8B, 0x8B);
    const color color::PaleVioletRed = color(0xDB, 0x70, 0x93);
    const color color::PaleVioletRed1 = color(0xFF, 0x82, 0xAB);
    const color color::PaleVioletRed2 = color(0xEE, 0x79, 0x9F);
    const color color::PaleVioletRed3 = color(0xCD, 0x68, 0x89);
    const color color::PaleVioletRed4 = color(0x8B, 0x47, 0x5D);
    const color color::PapayaWhip = color(0xFF, 0xEF, 0xD5);
    const color color::PeachPuff = color(0xFF, 0xDA, 0xB9);
    const color color::PeachPuff1 = color(0xFF, 0xDA, 0xB9);
    const color color::PeachPuff2 = color(0xEE, 0xCB, 0xAD);
    const color color::PeachPuff3 = color(0xCD, 0xAF, 0x95);
    const color color::PeachPuff4 = color(0x8B, 0x77, 0x65);
    const color color::Peru = color(0xCD, 0x85, 0x3F);
    const color color::Pink = color(0xFF, 0xC0, 0xCB);
    const color color::Pink1 = color(0xFF, 0xB5, 0xC5);
    const color color::Pink2 = color(0xEE, 0xA9, 0xB8);
    const color color::Pink3 = color(0xCD, 0x91, 0x9E);
    const color color::Pink4 = color(0x8B, 0x63, 0x6C);
    const color color::Plum = color(0xDD, 0xA0, 0xDD);
    const color color::Plum1 = color(0xFF, 0xBB, 0xFF);
    const color color::Plum2 = color(0xEE, 0xAE, 0xEE);
    const color color::Plum3 = color(0xCD, 0x96, 0xCD);
    const color color::Plum4 = color(0x8B, 0x66, 0x8B);
    const color color::PowderBlue = color(0xB0, 0xE0, 0xE6);
    const color color::Purple = color(0xA0, 0x20, 0xF0);
    const color color::Purple1 = color(0x9B, 0x30, 0xFF);
    const color color::Purple2 = color(0x91, 0x2C, 0xEE);
    const color color::Purple3 = color(0x7D, 0x26, 0xCD);
    const color color::Purple4 = color(0x55, 0x1A, 0x8B);
    const color color::Red = color(0xFF, 0x00, 0x00);
    const color color::Red1 = color(0xFF, 0x00, 0x00);
    const color color::Red2 = color(0xEE, 0x00, 0x00);
    const color color::Red3 = color(0xCD, 0x00, 0x00);
    const color color::Red4 = color(0x8B, 0x00, 0x00);
    const color color::RosyBrown = color(0xBC, 0x8F, 0x8F);
    const color color::RosyBrown1 = color(0xFF, 0xC1, 0xC1);
    const color color::RosyBrown2 = color(0xEE, 0xB4, 0xB4);
    const color color::RosyBrown3 = color(0xCD, 0x9B, 0x9B);
    const color color::RosyBrown4 = color(0x8B, 0x69, 0x69);
    const color color::RoyalBlue = color(0x41, 0x69, 0xE1);
    const color color::RoyalBlue1 = color(0x48, 0x76, 0xFF);
    const color color::RoyalBlue2 = color(0x43, 0x6E, 0xEE);
    const color color::RoyalBlue3 = color(0x3A, 0x5F, 0xCD);
    const color color::RoyalBlue4 = color(0x27, 0x40, 0x8B);
    const color color::SaddleBrown = color(0x8B, 0x45, 0x13);
    const color color::Salmon = color(0xFA, 0x80, 0x72);
    const color color::Salmon1 = color(0xFF, 0x8C, 0x69);
    const color color::Salmon2 = color(0xEE, 0x82, 0x62);
    const color color::Salmon3 = color(0xCD, 0x70, 0x54);
    const color color::Salmon4 = color(0x8B, 0x4C, 0x39);
    const color color::SandyBrown = color(0xF4, 0xA4, 0x60);
    const color color::SeaGreen = color(0x2E, 0x8B, 0x57);
    const color color::SeaGreen1 = color(0x54, 0xFF, 0x9F);
    const color color::SeaGreen2 = color(0x4E, 0xEE, 0x94);
    const color color::SeaGreen3 = color(0x43, 0xCD, 0x80);
    const color color::SeaGreen4 = color(0x2E, 0x8B, 0x57);
    const color color::Seashell = color(0xFF, 0xF5, 0xEE);
    const color color::Seashell1 = color(0xFF, 0xF5, 0xEE);
    const color color::Seashell2 = color(0xEE, 0xE5, 0xDE);
    const color color::Seashell3 = color(0xCD, 0xC5, 0xBF);
    const color color::Seashell4 = color(0x8B, 0x86, 0x82);
    const color color::Sienna = color(0xA0, 0x52, 0x2D);
    const color color::Sienna1 = color(0xFF, 0x82, 0x47);
    const color color::Sienna2 = color(0xEE, 0x79, 0x42);
    const color color::Sienna3 = color(0xCD, 0x68, 0x39);
    const color color::Sienna4 = color(0x8B, 0x47, 0x26);
    const color color::SkyBlue = color(0x87, 0xCE, 0xEB);
    const color color::SkyBlue1 = color(0x87, 0xCE, 0xFF);
    const color color::SkyBlue2 = color(0x7E, 0xC0, 0xEE);
    const color color::SkyBlue3 = color(0x6C, 0xA6, 0xCD);
    const color color::SkyBlue4 = color(0x4A, 0x70, 0x8B);
    const color color::SlateBlue = color(0x6A, 0x5A, 0xCD);
    const color color::SlateBlue1 = color(0x83, 0x6F, 0xFF);
    const color color::SlateBlue2 = color(0x7A, 0x67, 0xEE);
    const color color::SlateBlue3 = color(0x69, 0x59, 0xCD);
    const color color::SlateBlue4 = color(0x47, 0x3C, 0x8B);
    const color color::SlateGray = color(0x70, 0x80, 0x90);
    const color color::SlateGray1 = color(0xC6, 0xE2, 0xFF);
    const color color::SlateGray2 = color(0xB9, 0xD3, 0xEE);
    const color color::SlateGray3 = color(0x9F, 0xB6, 0xCD);
    const color color::SlateGray4 = color(0x6C, 0x7B, 0x8B);
    const color color::SlateGrey = color(0x70, 0x80, 0x90);
    const color color::Snow = color(0xFF, 0xFA, 0xFA);
    const color color::Snow1 = color(0xFF, 0xFA, 0xFA);
    const color color::Snow2 = color(0xEE, 0xE9, 0xE9);
    const color color::Snow3 = color(0xCD, 0xC9, 0xC9);
    const color color::Snow4 = color(0x8B, 0x89, 0x89);
    const color color::SpringGreen = color(0x00, 0xFF, 0x7F);
    const color color::SpringGreen1 = color(0x00, 0xFF, 0x7F);
    const color color::SpringGreen2 = color(0x00, 0xEE, 0x76);
    const color color::SpringGreen3 = color(0x00, 0xCD, 0x66);
    const color color::SpringGreen4 = color(0x00, 0x8B, 0x45);
    const color color::SteelBlue = color(0x46, 0x82, 0xB4);
    const color color::SteelBlue1 = color(0x63, 0xB8, 0xFF);
    const color color::SteelBlue2 = color(0x5C, 0xAC, 0xEE);
    const color color::SteelBlue3 = color(0x4F, 0x94, 0xCD);
    const color color::SteelBlue4 = color(0x36, 0x64, 0x8B);
    const color color::Tan = color(0xD2, 0xB4, 0x8C);
    const color color::Tan1 = color(0xFF, 0xA5, 0x4F);
    const color color::Tan2 = color(0xEE, 0x9A, 0x49);
    const color color::Tan3 = color(0xCD, 0x85, 0x3F);
    const color color::Tan4 = color(0x8B, 0x5A, 0x2B);
    const color color::Thistle = color(0xD8, 0xBF, 0xD8);
    const color color::Thistle1 = color(0xFF, 0xE1, 0xFF);
    const color color::Thistle2 = color(0xEE, 0xD2, 0xEE);
    const color color::Thistle3 = color(0xCD, 0xB5, 0xCD);
    const color color::Thistle4 = color(0x8B, 0x7B, 0x8B);
    const color color::Tomato = color(0xFF, 0x63, 0x47);
    const color color::Tomato1 = color(0xFF, 0x63, 0x47);
    const color color::Tomato2 = color(0xEE, 0x5C, 0x42);
    const color color::Tomato3 = color(0xCD, 0x4F, 0x39);
    const color color::Tomato4 = color(0x8B, 0x36, 0x26);
    const color color::Turquoise = color(0x40, 0xE0, 0xD0);
    const color color::Turquoise1 = color(0x00, 0xF5, 0xFF);
    const color color::Turquoise2 = color(0x00, 0xE5, 0xEE);
    const color color::Turquoise3 = color(0x00, 0xC5, 0xCD);
    const color color::Turquoise4 = color(0x00, 0x86, 0x8B);
    const color color::Violet = color(0xEE, 0x82, 0xEE);
    const color color::VioletRed = color(0xD0, 0x20, 0x90);
    const color color::VioletRed1 = color(0xFF, 0x3E, 0x96);
    const color color::VioletRed2 = color(0xEE, 0x3A, 0x8C);
    const color color::VioletRed3 = color(0xCD, 0x32, 0x78);
    const color color::VioletRed4 = color(0x8B, 0x22, 0x52);
    const color color::Wheat = color(0xF5, 0xDE, 0xB3);
    const color color::Wheat1 = color(0xFF, 0xE7, 0xBA);
    const color color::Wheat2 = color(0xEE, 0xD8, 0xAE);
    const color color::Wheat3 = color(0xCD, 0xBA, 0x96);
    const color color::Wheat4 = color(0x8B, 0x7E, 0x66);
    const color color::White = color(0xFF, 0xFF, 0xFF);
    const color color::WhiteSmoke = color(0xF5, 0xF5, 0xF5);
    const color color::Yellow = color(0xFF, 0xFF, 0x00);
    const color color::Yellow1 = color(0xFF, 0xFF, 0x00);
    const color color::Yellow2 = color(0xEE, 0xEE, 0x00);
    const color color::Yellow3 = color(0xCD, 0xCD, 0x00);
    const color color::Yellow4 = color(0x8B, 0x8B, 0x00);
    const color color::YellowGreen = color(0x9A, 0xCD, 0x32);

    optional_color color::from_name(const std::string& aName)
    {
        struct named_colors : public std::map<neolib::ci_string, color>
        {
            named_colors() : std::map<neolib::ci_string, color>
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