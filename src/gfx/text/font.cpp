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
#include <neogfx/gfx/text/glyph.hpp>
#include <neogfx/gfx/text/i_glyph_texture.hpp>
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

    font_weight font_info::weight_from_style_name(std::string aStyleName, bool aUnknownAsRegular)
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
        boost::algorithm::to_lower(aStyleName);
        auto w = sWeightMap.find(aStyleName);
        if (w != sWeightMap.end())
            return w->second;
        auto aw = sAbbreviatedWeightMap.find(aStyleName);
        if (aw != sAbbreviatedWeightMap.end())
            return aw->second;
        std::optional<decltype(sWeightMap)::value_type*> match;
        for (auto& wme : sWeightMap)
            if (aStyleName.find(wme.first) != std::string::npos)
            {
                if (match == std::nullopt || (**match).first.size() < wme.first.size())
                    match = &wme;
            }
        if (match != std::nullopt)
            return (**match).second;
        return aUnknownAsRegular ? font_weight::Regular : font_weight::Unknown;
    }

    class font_info::instance
    {
        friend class font_info;
    public:
        instance();
        instance(std::string const& aFamilyName, font_style aStyle, point_size aSize);
        instance(std::string const& aFamilyName, std::string const& aStyleName, point_size aSize);
        instance(std::string const& aFamilyName, font_style aStyle, std::string const& aStyleName, point_size aSize);
        instance(std::string const& aFamilyName, const optional_style& aStyle, const optional_style_name& aStyleName, point_size aSize);
        instance(const instance& aOther);
        ~instance();
    public:
        instance& operator=(const instance& aOther);
    private:
        string iFamilyName;
        optional_style iStyle;
        optional_style_name iStyleName;
        bool iUnderline;
        font_weight iWeight;
        point_size iSize;
        bool iKerning;
    };

    font_info::instance::instance() :
        iSize{}, iUnderline{ false }, iWeight{ font_weight::Normal }, iKerning{ false }
    {
    }

    font_info::instance::instance(std::string const& aFamilyName, font_style aStyle, point_size aSize) :
        iFamilyName{ aFamilyName }, iStyle{ aStyle }, iUnderline{ (aStyle & font_style::Underline) == font_style::Underline }, iWeight{ weight_from_style(aStyle) }, iSize{ aSize }, iKerning{ false }
    {
    }

    font_info::instance::instance(std::string const& aFamilyName, std::string const& aStyleName, point_size aSize) :
        iFamilyName{ aFamilyName }, iStyleName{ aStyleName }, iUnderline{ false }, iWeight{ weight_from_style_name(aStyleName) }, iSize{ aSize }, iKerning{ false }
    {
    }

    font_info::instance::instance(std::string const& aFamilyName, font_style aStyle, std::string const& aStyleName, point_size aSize) :
        iFamilyName{ aFamilyName }, iStyle{ aStyle }, iStyleName{ aStyleName }, iUnderline{ (aStyle & font_style::Underline) == font_style::Underline }, iWeight{ weight_from_style_name(aStyleName) }, iSize{ aSize }, iKerning{ false }
    {
    }

    font_info::instance::instance(std::string const& aFamilyName, const optional_style& aStyle, const optional_style_name& aStyleName, point_size aSize) :
        iFamilyName{ aFamilyName },
        iStyle{ aStyle },
        iStyleName{ aStyleName },
        iUnderline{ false },
        iWeight{ aStyleName != std::nullopt ? weight_from_style_name(*aStyleName) : aStyle != std::nullopt ? weight_from_style(*aStyle) : font_weight::Normal },
        iSize{ aSize },
        iKerning{ false }
    {
    }

    font_info::instance::instance(const font_info::instance& aOther) :
        iFamilyName{ aOther.iFamilyName }, iStyle{ aOther.iStyle }, iStyleName{ aOther.iStyleName }, iUnderline{ aOther.iUnderline }, iWeight{ aOther.iWeight }, iSize{ aOther.iSize }, iKerning{ aOther.iKerning }
    {
    }

    font_info::instance::~instance()
    {
    }

    font_info::instance& font_info::instance::operator=(const instance& aOther)
    {
        iFamilyName = aOther.iFamilyName;
        iStyle = aOther.iStyle;
        iStyleName = aOther.iStyleName;
        iUnderline = aOther.iUnderline;
        iWeight = aOther.iWeight;
        iSize = aOther.iSize;
        iKerning = aOther.iKerning;
        return *this;
    }

    font_info::font_info() :
        iInstance{ std::make_shared<instance>() }
    {
    }

    font_info::font_info(std::string const& aFamilyName, font_style aStyle, point_size aSize) :
        iInstance{ std::make_shared<instance>(aFamilyName, aStyle, aSize) }
    {
    }

    font_info::font_info(std::string const& aFamilyName, std::string const& aStyleName, point_size aSize) :
        iInstance{ std::make_shared<instance>(aFamilyName, aStyleName, aSize) }
    {

    }

    font_info::font_info(std::string const& aFamilyName, font_style aStyle, std::string const& aStyleName, point_size aSize) :
        iInstance{ std::make_shared<instance>(aFamilyName, aStyle, aStyleName, aSize) }
    {

    }

    font_info::font_info(std::string const& aFamilyName, const optional_style& aStyle, const optional_style_name& aStyleName, point_size aSize) :
        iInstance{ std::make_shared<instance>(aFamilyName, aStyle, aStyleName, aSize) }
    {
    }

    font_info::font_info(const font_info& aOther) :
        iInstance{ aOther.iInstance }
    {
    }

    font_info::~font_info()
    {
    }

    font_info& font_info::operator=(const font_info& aOther)
    {
        iInstance = aOther.iInstance;
        return *this;
    }

    i_string const& font_info::family_name() const
    {
        return iInstance->iFamilyName;
    }

    bool font_info::style_available() const
    {
        return iInstance->iStyle != std::nullopt;
    }

    font_style font_info::style() const
    {
        if (style_available())
            return *iInstance->iStyle;
        else
            throw unknown_style();
    }

    bool font_info::style_name_available() const
    {
        return iInstance->iStyleName != std::nullopt;
    }

    i_string const& font_info::style_name() const
    {
        if (style_name_available())
            return *iInstance->iStyleName;
        else
            throw unknown_style_name();
    }

    bool font_info::underline() const
    {
        return iInstance->iUnderline || (font_info::style_available() && (font_info::style() & font_style::Underline) == font_style::Underline);
    }

    void font_info::set_underline(bool aUnderline)
    {
        if (iInstance->iUnderline != aUnderline)
        {
            iInstance = std::make_shared<instance>(*iInstance);
            iInstance->iUnderline = aUnderline;
        }
    }

    font_weight font_info::weight() const
    {
        return iInstance->iWeight;
    }

    font::point_size font_info::size() const
    {
        return iInstance->iSize;
    }

    bool font_info::kerning() const
    {
        return iInstance->iKerning;
    }

    void font_info::enable_kerning()
    {
        if (iInstance->iKerning != true)
        {
            iInstance = std::make_shared<instance>(*iInstance);
            iInstance->iKerning = true;
        }
    }

    void font_info::disable_kerning()
    {
        if (iInstance->iKerning != false)
        {
            iInstance = std::make_shared<instance>(*iInstance);
            iInstance->iKerning = false;
        }
    }

    font_info font_info::with_style(font_style aStyle) const
    {
        return font_info(iInstance->iFamilyName, aStyle, optional_style_name{}, iInstance->iSize);
    }

    font_info font_info::with_style_xor(font_style aStyle) const
    {
        return font_info(iInstance->iFamilyName, iInstance->iStyle != std::nullopt ? *iInstance->iStyle ^ aStyle : aStyle, optional_style_name{}, iInstance->iSize);
    }

    font_info font_info::with_underline(bool aUnderline) const
    {
        font_info result = *this;
        result.set_underline(aUnderline);
        return result;
    }

    font_info font_info::with_size(point_size aSize) const
    {
        return font_info(iInstance->iFamilyName, iInstance->iStyle, iInstance->iStyleName, aSize);
    }

    bool font_info::operator==(const font_info& aRhs) const
    {
        return iInstance == aRhs.iInstance ||
            (iInstance->iFamilyName == aRhs.iInstance->iFamilyName &&
                iInstance->iStyle == aRhs.iInstance->iStyle &&
                iInstance->iStyleName == aRhs.iInstance->iStyleName &&
                iInstance->iUnderline == aRhs.iInstance->iUnderline &&
                iInstance->iSize == aRhs.iInstance->iSize &&
                iInstance->iKerning == aRhs.iInstance->iKerning);
    }

    bool font_info::operator!=(const font_info& aRhs) const
    {
        return !operator==(aRhs);
    }

    bool font_info::operator<(const font_info& aRhs) const
    {
        return std::tie(iInstance->iFamilyName, iInstance->iStyle, iInstance->iStyleName, iInstance->iUnderline, iInstance->iSize, iInstance->iKerning) < 
            std::tie(aRhs.iInstance->iFamilyName, aRhs.iInstance->iStyle, aRhs.iInstance->iStyleName, aRhs.iInstance->iUnderline, aRhs.iInstance->iSize, aRhs.iInstance->iKerning);
    }

    class font::instance
    {
    public:
        instance(ref_ptr<i_native_font_face> aNativeFontFace);
        instance(const instance& aOther);
        ~instance();
    public:
        instance& operator=(const instance& aOther);
    public:
        i_native_font_face& native_font_face() const;
        bool has_fallback_font() const;
        font fallback_font() const;
    private:
        ref_ptr<i_native_font_face> iNativeFontFace;
        mutable std::optional<bool> iHasFallbackFont;
        mutable std::optional<font> iFallbackFont;
    };

    font::instance::instance(ref_ptr<i_native_font_face> aNativeFontFace) :
        iNativeFontFace{ aNativeFontFace }
    {
    }

    font::instance::instance(const instance& aOther) :
        iNativeFontFace{ aOther.iNativeFontFace }, iHasFallbackFont{ aOther.iHasFallbackFont }, iFallbackFont{ aOther.iFallbackFont }
    {
    }

    font::instance::~instance()
    {
    }
        
    font::instance& font::instance::operator=(const instance& aOther)
    {
        auto old = *this;
        iNativeFontFace = aOther.iNativeFontFace;
        iHasFallbackFont = aOther.iHasFallbackFont;
        iFallbackFont = aOther.iFallbackFont;
        return *this;
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
        font_info{ service<i_app>().current_style().font_info() }, 
        iInstance{ service<i_app>().current_style().font().iInstance }
    {
    }

    font::font(std::string const& aFamilyName, font_style aStyle, point_size aSize) :
        font_info{ aFamilyName, aStyle, aSize }, 
        iInstance{ std::make_shared<instance>(service<i_font_manager>().create_font(string{ aFamilyName }, aStyle, aSize, service<i_rendering_engine>().default_screen_metrics())) }
    {
    }

    font::font(std::string const& aFamilyName, std::string const& aStyleName, point_size aSize) :
        font_info{ aFamilyName, aStyleName, aSize }, 
        iInstance{ std::make_shared<instance>(service<i_font_manager>().create_font(string{ aFamilyName }, string{ aStyleName }, aSize, service<i_rendering_engine>().default_screen_metrics())) }
    {
    }

    font::font(const font_info& aFontInfo) :
        font_info{ aFontInfo }, 
        iInstance{ std::make_shared<instance>(service<i_font_manager>().create_font(static_cast<font_info>(*this), service<i_rendering_engine>().default_screen_metrics())) }
    {
    }

    font::font(const font& aOther) :
        font_info{ aOther }, 
        iInstance{ aOther.iInstance }
    {
    }
    
    font::font(const font& aOther, font_style aStyle, point_size aSize) :
        font_info{ aOther.native_font_face().family_name(), aStyle, aSize }, 
        iInstance{ std::make_shared<instance>(service<i_font_manager>().create_font(aOther.iInstance->native_font_face().native_font(), aStyle, aSize, service<i_rendering_engine>().default_screen_metrics())) }
    {
    }

    font::font(const font& aOther, std::string const& aStyleName, point_size aSize) :
        font_info{ aOther.native_font_face().family_name(), aStyleName, aSize },
        iInstance{ std::make_shared<instance>(service<i_font_manager>().create_font(aOther.iInstance->native_font_face().native_font(), string{ aStyleName }, aSize, service<i_rendering_engine>().default_screen_metrics())) }
    {
    }

    font::font(i_native_font_face& aNativeFontFace) :
        font_info{ aNativeFontFace.family_name(), aNativeFontFace.style_name(), aNativeFontFace.size() }, 
        iInstance{ std::make_shared<instance>(aNativeFontFace) }
    {
    }

    font::font(i_native_font_face& aNativeFontFace, font_style aStyle) :
        font_info{ aNativeFontFace.family_name(), aStyle, aNativeFontFace.style_name(), aNativeFontFace.size() }, 
        iInstance{ std::make_shared<instance>(aNativeFontFace) }
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
        return font{ service<i_font_manager>().load_font_from_file(string{ aFileName }, string{ aStyleName }, aSize, service<i_rendering_engine>().default_screen_metrics()) };
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
        return font{ service<i_font_manager>().load_font_from_memory(aData, aSizeInBytes, string{ aStyleName }, aSize, service<i_rendering_engine>().default_screen_metrics()) };
    }

    font::~font()
    {
    }

    font& font::operator=(const font& aOther)
    {
        if (&aOther == this)
            return *this;
        font_info::operator=(aOther);
        auto oldFontFaces = iInstance;
        iInstance = aOther.iInstance;
        return *this;
    }

    font_id font::id() const
    {
        return native_font_face().id();
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

    font::point_size font::size() const
    {
        return native_font_face().size();
    }

    dimension font::height() const
    {
        return native_font_face().height();
    }

    dimension font::ascender() const
    {
        return native_font_face().ascender();
    }

    dimension font::descender() const
    {
        return native_font_face().descender();
    }

    dimension font::line_spacing() const
    {
        /* todo */
        return 0;
    }

    dimension font::kerning(uint32_t aLeftGlyphIndex, uint32_t aRightGlyphIndex) const
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

    uint32_t font::num_fixed_sizes() const
    {
        return native_font_face().num_fixed_sizes();
    }

    font::point_size font::fixed_size(uint32_t aFixedSizeIndex) const
    {
        return native_font_face().fixed_size(aFixedSizeIndex);
    }

    const i_glyph_texture& font::glyph_texture(const glyph& aGlyph) const
    {
        return native_font_face().glyph_texture(aGlyph);
    }

    bool font::operator==(const font& aRhs) const
    {
        return iInstance->native_font_face().handle() == aRhs.iInstance->native_font_face().handle() &&
            underline() == aRhs.underline() && kerning() == aRhs.kerning();
    }

    bool font::operator!=(const font& aRhs) const
    {
        return !(*this == aRhs);
    }

    bool font::operator<(const font& aRhs) const
    {
        return font_info::operator<(aRhs);
    }

    i_native_font_face& font::native_font_face() const
    {
        return iInstance->native_font_face();
    }
}