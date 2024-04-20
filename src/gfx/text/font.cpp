// font.cpp
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

#include <unordered_map>
#include <boost/algorithm/string.hpp> 

#include <neogfx/app/i_app.hpp>
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/gfx/text/glyph_text.hpp>
#include <neogfx/gfx/text/i_glyph.hpp>
#include <neogfx/gfx/text/i_font_manager.hpp>
#include "native/i_native_font.hpp"

namespace neogfx
{
    font_weight font_info::weight_from_style(font_style aStyle)
    {
        if ((aStyle & font_style::Bold) == font_style::Bold)
            return font_weight::Bold;
        else
            return font_weight::Normal;
    }

    font_weight font_info::weight_from_style_name(std::string const& aStyleName, bool aUnknownAsRegular)
    {
        static std::unordered_map<std::string, font_weight> sWeightMap =
        {
            { "thin", font_weight::Thin },
            { "extralight", font_weight::Extralight },
            { "extra light", font_weight::Extralight },
            { "ultralight", font_weight::Ultralight },
            { "ultra light", font_weight::Ultralight },
            { "light", font_weight::Light },
            { "normal", font_weight::Normal },
            { "regular", font_weight::Regular },
            { "medium", font_weight::Medium },
            { "semibold", font_weight::Semibold },
            { "semi bold", font_weight::Semibold },
            { "demibold", font_weight::Demibold },
            { "demi bold", font_weight::Demibold },
            { "bold", font_weight::Bold },
            { "bold (emulated)", font_weight::Bold },
            { "extrabold", font_weight::Extrabold },
            { "extra bold", font_weight::Extrabold },
            { "ultrabold", font_weight::Ultrabold },
            { "ultra bold", font_weight::Ultrabold },
            { "heavy", font_weight::Heavy },
            { "black", font_weight::Black },
        };
        static std::unordered_map<std::string, font_weight> sAbbreviatedWeightMap =
        {
            { "t", font_weight::Thin },
            { "el", font_weight::Extralight },
            { "ul", font_weight::Ultralight },
            { "l", font_weight::Light },
            { "n", font_weight::Normal },
            { "r", font_weight::Regular },
            { "m", font_weight::Medium },
            { "sb", font_weight::Semibold },
            { "db", font_weight::Demibold },
            { "b", font_weight::Bold },
            { "eb", font_weight::Extrabold },
            { "ub", font_weight::Ultrabold },
            { "h", font_weight::Heavy },
            { "bl", font_weight::Black },
        };
        auto const key = boost::algorithm::to_lower_copy(aStyleName);
        auto w = sWeightMap.find(key);
        if (w != sWeightMap.end())
            return w->second;
        auto aw = sAbbreviatedWeightMap.find(key);
        if (aw != sAbbreviatedWeightMap.end())
            return aw->second;
        decltype(sWeightMap)::value_type const* match = nullptr;
        for (auto const& wme : sWeightMap)
            if (key.find(wme.first) != std::string::npos)
            {
                if (match == nullptr || match->first.size() < wme.first.size())
                    match = &wme;
            }
        if (match != nullptr)
            return match->second;
        return aUnknownAsRegular ? font_weight::Regular : font_weight::Unknown;
    }

    font_info::font_info() :
        iSize{}, 
        iUnderline{ false }, 
        iWeight{ font_weight::Normal },
        iOutline{ 0.0 },
        iKerning{ true }
    {
    }

    font_info::font_info(std::string const& aFamilyName, font_style aStyle, point_size aSize) :
        iFamilyName{ aFamilyName }, 
        iStyle{ aStyle }, 
        iUnderline{ (aStyle & font_style::Underline) == font_style::Underline }, 
        iWeight{ weight_from_style(aStyle) }, 
        iSize{ aSize }, 
        iOutline{ 0.0 },
        iKerning{ true }
    {
    }

    font_info::font_info(std::string const& aFamilyName, std::string const& aStyleName, point_size aSize) :
        iFamilyName{ aFamilyName }, 
        iStyleName{ aStyleName }, 
        iUnderline{ false }, 
        iWeight{ weight_from_style_name(aStyleName) }, 
        iSize{ aSize }, 
        iOutline{ 0.0 },
        iKerning{ true }
    {
    }

    font_info::font_info(std::string const& aFamilyName, font_style aStyle, std::string const& aStyleName, point_size aSize) :
        iFamilyName{ aFamilyName }, 
        iStyle{ aStyle }, 
        iStyleName{ aStyleName }, 
        iUnderline{ (aStyle & font_style::Underline) == font_style::Underline }, 
        iWeight{ weight_from_style_name(aStyleName) }, 
        iSize{ aSize }, 
        iOutline{ 0.0 },
        iKerning{ true }
    {
    }

    font_info::font_info(std::string const& aFamilyName, const optional_style& aStyle, const optional_style_name& aStyleName, point_size aSize) :
        iFamilyName{ aFamilyName },
        iStyle{ aStyle },
        iStyleName{ aStyleName },
        iUnderline{ false },
        iWeight{ aStyleName != std::nullopt ? weight_from_style_name(*aStyleName) : aStyle != std::nullopt ? weight_from_style(*aStyle) : font_weight::Normal },
        iSize{ aSize },
        iOutline{ 0.0 },
        iKerning{ true }
    {
    }

    font_info::font_info(const font_info& aOther) :
        iFamilyName{ aOther.iFamilyName }, 
        iStyle{ aOther.iStyle }, 
        iStyleName{ aOther.iStyleName }, 
        iUnderline{ aOther.iUnderline }, 
        iWeight{ aOther.iWeight }, 
        iSize{ aOther.iSize }, 
        iOutline{ aOther.iOutline },
        iKerning{ aOther.iKerning }
    {
    }

    font_info::~font_info()
    {
    }

    font_info& font_info::operator=(const font_info& aOther)
    {
        iFamilyName = aOther.iFamilyName;
        iStyle = aOther.iStyle;
        iStyleName = aOther.iStyleName;
        iUnderline = aOther.iUnderline;
        iWeight = aOther.iWeight;
        iSize = aOther.iSize;
        iOutline = aOther.iOutline;
        iKerning = aOther.iKerning;
        return *this;
    }

    i_string const& font_info::family_name() const
    {
        return iFamilyName;
    }

    bool font_info::style_available() const
    {
        return iStyle != std::nullopt;
    }

    font_style font_info::style() const
    {
        if (style_available())
            return *iStyle;
        else
            throw unknown_style();
    }

    font_style font_info::style_maybe() const
    {
        if (style_available())
            return *iStyle;
        else
            return font_style{};
    }

    bool font_info::style_name_available() const
    {
        return iStyleName != std::nullopt;
    }

    i_string const& font_info::style_name() const
    {
        if (style_name_available())
            return *iStyleName;
        else
            throw unknown_style_name();
    }

    bool font_info::underline() const
    {
        return iUnderline || (font_info::style_available() && (font_info::style() & font_style::Underline) == font_style::Underline);
    }

    void font_info::set_underline(bool aUnderline)
    {
        iUnderline = aUnderline;
    }

    font_weight font_info::weight() const
    {
        return iWeight;
    }

    font::point_size font_info::size() const
    {
        return iSize;
    }

    stroke font_info::outline() const
    {
        return iOutline;
    }

    void font_info::set_outline(stroke aOutline)
    {
        iOutline = aOutline;
    }

    bool font_info::kerning() const
    {
        return iKerning;
    }

    void font_info::enable_kerning()
    {
        iKerning = true;
    }

    void font_info::disable_kerning()
    {
        iKerning = false;
    }

    font_info font_info::with_style(font_style aStyle) const
    {
        font_info result{ *this };
        result.iStyle = aStyle;
        return result;
    }

    font_info font_info::with_style_xor(font_style aStyle) const
    {
        auto s = iStyle != std::nullopt ? *iStyle ^ aStyle : aStyle;
        if ((s & (font_style::Bold & font_style::Italic)) == font_style::Invalid)
            s |= font_style::Normal;
        if ((s & font_style::Normal) == font_style::Normal && (s & (font_style::Bold & font_style::Italic)) != font_style::Invalid)
            s &= ~font_style::Normal;
        font_info result{ *this };
        result.iStyle = s;
        return result;
    }

    font_info font_info::with_style_name(std::string const& aStyleName) const
    {
        font_info result{ *this };
        result.iStyleName = aStyleName;
        return result;
    }

    font_info font_info::with_underline(bool aUnderline) const
    {
        font_info result = *this;
        result.set_underline(aUnderline);
        return result;
    }

    font_info font_info::with_size(point_size aSize) const
    {
        return font_info(iFamilyName, iStyle, iStyleName, aSize);
    }

    font_info font_info::with_outline(stroke aOutline) const
    {
        font_info result = *this;
        result.set_outline(aOutline);
        return result;
    }

    class font::instance
    {
    public:
        instance(font_info const& aInfo, ref_ptr<i_native_font_face> aNativeFontFace);
        instance(const instance& aOther);
        ~instance();
    public:
        instance& operator=(instance const& aOther);
    public:
        font_info const& info() const;
        i_native_font_face& native_font_face() const;
        bool has_fallback_font() const;
        font fallback_font() const;
    public:
        auto operator<=>(const instance& aRhs) const = default;
    private:
        font_info iInfo;
        ref_ptr<i_native_font_face> iNativeFontFace;
        mutable std::optional<bool> iHasFallbackFont;
        mutable std::optional<font> iFallbackFont;
    };

    font::instance::instance(font_info const& aInfo, ref_ptr<i_native_font_face> aNativeFontFace) :
        iInfo{ aInfo }, iNativeFontFace { aNativeFontFace }
    {
    }

    font::instance::instance(const instance& aOther) :
        iInfo{ aOther.iInfo }, iNativeFontFace { aOther.iNativeFontFace }, iHasFallbackFont{ aOther.iHasFallbackFont }, iFallbackFont{ aOther.iFallbackFont }
    {
    }

    font::instance::~instance()
    {
    }
        
    font::instance& font::instance::operator=(const instance& aOther)
    {
        auto old = *this;
        iInfo = aOther.iInfo;
        iNativeFontFace = aOther.iNativeFontFace;
        iHasFallbackFont = aOther.iHasFallbackFont;
        iFallbackFont = aOther.iFallbackFont;
        return *this;
    }

    font_info const& font::instance::info() const
    {
        return iInfo;
    }

    i_native_font_face& font::instance::native_font_face() const
    {
        return *iNativeFontFace;
    }

    bool font::instance::has_fallback_font() const
    {
        if (iHasFallbackFont == std::nullopt)
            iHasFallbackFont = service<i_font_manager>().has_fallback_font(native_font_face());
        return *iHasFallbackFont;
    }

    font font::instance::fallback_font() const
    {
        if (iFallbackFont == std::nullopt)
        {
            if (has_fallback_font())
                iFallbackFont = font{ service<i_font_manager>().create_fallback_font(native_font_face()) };
            else
                throw no_fallback_font();
        }
        return *iFallbackFont;
    }

    font::font() :
        iInstance{ service<i_app>().current_style().font().iInstance }
    {
    }

    font::font(std::string const& aFamilyName, font_style aStyle, point_size aSize) :
        iInstance{ std::make_shared<instance>(font_info{ aFamilyName, aStyle, aSize }, service<i_font_manager>().create_font(string{ aFamilyName }, aStyle, aSize, service<i_rendering_engine>().default_screen_metrics())) }
    {
    }

    font::font(std::string const& aFamilyName, std::string const& aStyleName, point_size aSize) :
        iInstance{ std::make_shared<instance>(font_info{ aFamilyName, aStyleName, aSize }, service<i_font_manager>().create_font(string{ aFamilyName }, font_style{}, string{ aStyleName }, aSize, service<i_rendering_engine>().default_screen_metrics())) }
    {
    }

    font::font(const font_info& aFontInfo) :
        iInstance{ std::make_shared<instance>(aFontInfo, service<i_font_manager>().create_font(aFontInfo, service<i_rendering_engine>().default_screen_metrics())) }
    {
    }

    font::font(const font& aOther) :
        iInstance{ aOther.iInstance }
    {
    }
    
    font::font(const font& aOther, font_style aStyle, point_size aSize) :
        iInstance{ std::make_shared<instance>(font_info{ aOther.native_font_face().family_name(), aStyle, aSize }, service<i_font_manager>().create_font(aOther.iInstance->native_font_face().native_font(), aStyle, aSize, service<i_rendering_engine>().default_screen_metrics())) }
    {
    }

    font::font(const font& aOther, std::string const& aStyleName, point_size aSize) :
        iInstance{ std::make_shared<instance>(font_info{ aOther.native_font_face().family_name(), aStyleName, aSize }, service<i_font_manager>().create_font(aOther.iInstance->native_font_face().native_font(), font_style{}, string{ aStyleName }, aSize, service<i_rendering_engine>().default_screen_metrics())) }
    {
    }

    font::font(i_native_font_face& aNativeFontFace) :
        iInstance{ std::make_shared<instance>(font_info{ aNativeFontFace.family_name(), aNativeFontFace.style_name(), aNativeFontFace.size() }, aNativeFontFace) }
    {
    }

    font::font(i_native_font_face& aNativeFontFace, font_style aStyle) :
        iInstance{ std::make_shared<instance>(font_info{ aNativeFontFace.family_name(), aStyle, aNativeFontFace.style_name(), aNativeFontFace.size() }, aNativeFontFace) }
    {
    }

    font font::load_from_file(std::string const& aFileName)
    {
        return font{ service<i_font_manager>().load_font_from_file(string{ aFileName }, service<i_rendering_engine>().default_screen_metrics()) };
    }

    font font::load_from_file(std::string const& aFileName, font_style aStyle, point_size aSize)
    {
        return font{ service<i_font_manager>().load_font_from_file(string{ aFileName }, aStyle, aSize, service<i_rendering_engine>().default_screen_metrics()) };
    }
    
    font font::load_from_file(std::string const& aFileName, std::string const& aStyleName, point_size aSize)
    {
        return font{ service<i_font_manager>().load_font_from_file(string{ aFileName }, font_style{}, string{ aStyleName }, aSize, service<i_rendering_engine>().default_screen_metrics()) };
    }

    font font::load_from_memory(const void* aData, std::size_t aSizeInBytes)
    {
        return font{ service<i_font_manager>().load_font_from_memory(aData, aSizeInBytes, service<i_rendering_engine>().default_screen_metrics()) };
    }

    font font::load_from_memory(const void* aData, std::size_t aSizeInBytes, font_style aStyle, point_size aSize)
    {
        return font{ service<i_font_manager>().load_font_from_memory(aData, aSizeInBytes, aStyle, aSize, service<i_rendering_engine>().default_screen_metrics()) };
    }

    font font::load_from_memory(const void* aData, std::size_t aSizeInBytes, std::string const& aStyleName, point_size aSize)
    {
        return font{ service<i_font_manager>().load_font_from_memory(aData, aSizeInBytes, font_style{}, string{ aStyleName }, aSize, service<i_rendering_engine>().default_screen_metrics()) };
    }

    font::~font()
    {
    }

    font& font::operator=(const font& aOther)
    {
        if (&aOther == this)
            return *this;
        auto old = iInstance;
        iInstance = aOther.iInstance;
        iSize = std::nullopt;
        iEmSize = std::nullopt;
        iHeight = std::nullopt;
        iMaxAdvance = std::nullopt;
        iAscender = std::nullopt;
        iDescender = std::nullopt;
        return *this;
    }

    font font::with_style(font_style aStyle) const
    {
        return info().with_style(aStyle);
    }

    font font::with_style_xor(font_style aStyle) const
    {
        return info().with_style_xor(aStyle);
    }

    font font::with_underline(bool aUnderline) const
    {
        return info().with_underline(aUnderline);
    }

    font font::with_size(point_size aSize) const
    {
        return info().with_size(aSize);
    }

    font font::with_outline(stroke aOutline) const
    {
        return info().with_outline(aOutline);
    }

    font_id font::id() const
    {
        return native_font_face().id();
    }

    font_info const& font::info() const
    {
        return iInstance->info();
    }

    bool font::has_fallback() const
    {
        return iInstance->has_fallback_font();
    }

    font font::fallback() const
    {
        return iInstance->fallback_font();
    }

    i_string const& font::family_name() const
    {
        return native_font_face().family_name();
    }

    font_style font::style() const
    {
        return static_cast<font_style>(native_font_face().style() | (underline() ? font_style::Underline : font_style{}));
    }

    i_string const& font::style_name() const
    {
        return native_font_face().style_name();
    }

    bool font::underline() const
    {
        return info().underline();
    }

    font::point_size font::size() const
    {
        if (iSize == std::nullopt)
            iSize = native_font_face().size();
        return iSize.value();
    }
    
    size font::em_size() const
    {
        if (iEmSize == std::nullopt)
            iEmSize = native_font_face().em_size();
        return iEmSize.value();
    }

    dimension font::height() const
    {
        if (iHeight == std::nullopt)
            iHeight = native_font_face().height();
        return iHeight.value();
    }

    dimension font::max_advance() const
    {
        if (iMaxAdvance == std::nullopt)
            iMaxAdvance = native_font_face().max_advance();
        return iMaxAdvance.value();
    }

    dimension font::ascender() const
    {
        if (iAscender == std::nullopt)
            iAscender = native_font_face().ascender();
        return iAscender.value();
    }

    dimension font::descender() const
    {
        if (iDescender == std::nullopt)
            iDescender = native_font_face().descender();
        return iDescender.value();
    }

    dimension font::line_spacing() const
    {
        /* todo */
        return 0;
    }

    bool font::kerning() const
    {
        return info().kerning();
    }

    dimension font::kerning(std::uint32_t aLeftGlyphIndex, std::uint32_t aRightGlyphIndex) const
    {
        if (kerning())
        {
            dimension result = native_font_face().kerning(aLeftGlyphIndex, aRightGlyphIndex);
            return result < 0.0 ? std::floor(result) : std::ceil(result);
        }
        else
            return 0.0;
    }

    bool font::is_bitmap_font() const
    {
        return native_font_face().is_bitmap_font();
    }

    std::uint32_t font::num_fixed_sizes() const
    {
        return native_font_face().num_fixed_sizes();
    }

    font::point_size font::fixed_size(std::uint32_t aFixedSizeIndex) const
    {
        return native_font_face().fixed_size(aFixedSizeIndex);
    }

    const i_glyph& font::glyph(const glyph_char& aGlyphChar) const
    {
        return native_font_face().glyph(aGlyphChar);
    }

    bool font::operator==(const font& aRhs) const
    {
        if (iInstance == aRhs.iInstance)
            return true;
        return info() == aRhs.info();
    }

    std::partial_ordering font::operator<=>(const font& aRhs) const
    {
        if (iInstance == aRhs.iInstance)
            return iInstance <=> aRhs.iInstance;
        return info() <=> aRhs.info();
    }

    i_native_font_face& font::native_font_face() const
    {
        return iInstance->native_font_face();
    }
}