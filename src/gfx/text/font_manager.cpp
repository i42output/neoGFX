// font_manager.cpp
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
#include <filesystem>
#include <neolib/core/string_utils.hpp>
#include <neolib/core/string_utf.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#ifdef u8
#undef u8
#include <harfbuzz\hb.h>
#include <harfbuzz\hb-ft.h>
#include <harfbuzz\hb-ucdn\ucdn.h>
#define u8
#else
#include <harfbuzz\hb.h>
#include <harfbuzz\hb-ot.h>
#endif
#ifdef _WIN32
#include <Shlobj.h>
#endif
#include <neolib/file/file.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gfx/text/font_manager.hpp>
#include <neogfx/gfx/text/text_category_map.hpp>
#include "../../gfx/text/native/native_font_face.hpp"
#include "../../gfx/text/native/native_font.hpp"

template <>
neogfx::i_font_manager& services::start_service<neogfx::i_font_manager>()
{
    return services::service<neogfx::i_rendering_engine>().font_manager();
}

namespace neogfx
{
    neolib::small_cookie item_cookie(const font_manager::id_cache_entry& aEntry)
    {
        return aEntry.id();
    }

    namespace detail
    {
        namespace platform_specific
        {
            optional<font_info> default_system_font_info(system_font_role aRole)
            {
#ifdef WIN32
#if 1 // Has Microsoft (tm) changed their mind on this? (See VS2019 font usage)
                if (service<i_font_manager>().has_font("Segoe UI", "Regular") && (aRole == system_font_role::Caption || aRole == system_font_role::Menu || aRole == system_font_role::StatusBar))
                    return font_info{ "Segoe UI", "Regular", 9 };
#endif
                if (aRole == system_font_role::Widget)
                {
                    std::wstring defaultFontFaceName = L"Microsoft Sans Serif";
                    HKEY hkeyDefaultFont;
                    if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes",
                        0, KEY_READ, &hkeyDefaultFont) == ERROR_SUCCESS)
                    {
                        DWORD dwType;
                        wchar_t byteBuffer[LF_FACESIZE + 1];
                        DWORD dwBufferSize = sizeof(byteBuffer);
                        if (RegQueryValueEx(hkeyDefaultFont, L"MS Shell Dlg 2", NULL, &dwType,
                            (LPBYTE)&byteBuffer, &dwBufferSize) == ERROR_SUCCESS)
                        {
                            defaultFontFaceName = (LPCTSTR)byteBuffer;
                        }
                        else if (RegQueryValueEx(hkeyDefaultFont, L"MS Shell Dlg", NULL, &dwType,
                            (LPBYTE)&byteBuffer, &dwBufferSize) == ERROR_SUCCESS)
                        {
                            defaultFontFaceName = (LPCTSTR)byteBuffer;
                        }
                        ::RegCloseKey(hkeyDefaultFont);
                    }
                    return font_info(neolib::utf16_to_utf8(reinterpret_cast<const char16_t*>(defaultFontFaceName.c_str())), font_style::Normal, 8);
                }
                else
                    return {};
#else
                return {};
#endif
            }

            std::string get_system_font_directory()
            {
#ifdef WIN32
                char szPath[MAX_PATH];
                if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_FONTS, NULL, 0, szPath)))
                    return neolib::tidy_path(szPath);
                else
                    throw std::logic_error("neogfx::detail::platform_specific::get_system_font_directory: Error");
#else
                throw std::logic_error("neogfx::detail::platform_specific::get_system_font_directory: Unknown system");
#endif
            }

            std::string get_local_font_directory()
            {
#ifdef WIN32
                char szPath[MAX_PATH];
                if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)))
                    return neolib::tidy_path(szPath) + "/Microsoft/Windows/Fonts";
                else
                    throw std::logic_error("neogfx::detail::platform_specific::get_system_font_directory: Error");
#else
                throw std::logic_error("neogfx::detail::platform_specific::get_local_font_directory: Unknown system");
#endif
            }

            fallback_font_info default_fallback_font_info()
            {
#ifdef WIN32
                // TODO: Use fallback font info from registry
                return fallback_font_info{ { "Segoe UI Symbol", "Noto Sans CJK JP", "Arial Unicode MS" } };
#else
                throw std::logic_error("neogfx::detail::platform_specific::default_fallback_font_info: Unknown system");
#endif
            }
        }
    }

    fallback_font_info::fallback_font_info(std::vector<string> aFallbackFontFamilies) :
        iFallbackFontFamilies(std::move(aFallbackFontFamilies))
    {
    }

    bool fallback_font_info::has_fallback_for(i_string const& aFontFamilyName) const
    {
        if (iFallbackFontFamilies.empty())
            return false;
        return std::find(iFallbackFontFamilies.begin(), iFallbackFontFamilies.end(), aFontFamilyName) != std::prev(iFallbackFontFamilies.end());
    }

    i_string const& fallback_font_info::fallback_for(i_string const& aFontFamilyName) const
    {
        auto f = std::find(iFallbackFontFamilies.begin(), iFallbackFontFamilies.end(), aFontFamilyName);
        if (f == iFallbackFontFamilies.end())
            return *iFallbackFontFamilies.begin();
        ++f;
        if (f == iFallbackFontFamilies.end())
            throw no_fallback();
        return *f;
    }

    class glyph_text_factory : public i_glyph_text_factory
    {
    public:
        struct cluster
        {
            std::string::size_type from;
            glyph::flags_e flags;
        };
        typedef std::vector<cluster> cluster_map_t;
        typedef std::tuple<const char32_t*, const char32_t*, text_direction, bool, hb_script_t> glyph_run;
        typedef std::vector<glyph_run> run_list;
    public:
        glyph_text create_glyph_text(font const& aFont) override;
        glyph_text to_glyph_text(i_graphics_context const& aContext, char const* aUtf8Begin, char const* aUtf8End, i_font_selector const& aFontSelector) override;
        glyph_text to_glyph_text(i_graphics_context const& aContext, char32_t const* aUtf32Begin, char32_t const* aUtf32End, i_font_selector const& aFontSelector) override;
    private:
        cluster_map_t iClusterMap;
        std::vector<character_type> iTextDirections;
        std::u32string iCodePointsBuffer;
        run_list iRuns;
    };

    class glyph_shapes
    {
    public:
        struct not_using_fallback : std::logic_error { not_using_fallback() : std::logic_error("neogfx::graphics_context::glyph_shapes::not_using_fallback") {} };
    public:
        class glyphs
        {
        public:
            glyphs(const i_graphics_context& aParent, const font& aFont, const glyph_text_factory::glyph_run& aGlyphRun) :
                iParent{ aParent },
                iFont{ static_cast<font_face_handle*>(aFont.native_font_face().handle())->harfbuzzFont },
                iGlyphRun{ aGlyphRun },
                iBuf{ static_cast<font_face_handle*>(aFont.native_font_face().handle())->harfbuzzBuf },
                iGlyphCount{ 0u },
                iGlyphInfo{ nullptr },
                iGlyphPos{ nullptr }
            {
                hb_buffer_set_direction(iBuf, std::get<2>(aGlyphRun) == text_direction::RTL ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
                hb_buffer_set_script(iBuf, std::get<4>(aGlyphRun));
                std::vector<uint32_t> reversed;
                if (std::get<2>(aGlyphRun) != text_direction::None_RTL)
                    hb_buffer_add_utf32(iBuf, reinterpret_cast<const uint32_t*>(std::get<0>(aGlyphRun)), static_cast<int>(std::get<1>(aGlyphRun) - std::get<0>(aGlyphRun)), 0, static_cast<int>(std::get<1>(aGlyphRun) - std::get<0>(aGlyphRun)));
                else
                {
                    reversed.reserve(std::get<1>(aGlyphRun) - std::get<0>(aGlyphRun));
                    for (auto ch = std::get<1>(aGlyphRun); ch != std::get<0>(aGlyphRun); --ch)
                    {
                        switch (*(ch - 1))
                        {
                        case U'(':
                            reversed.push_back(U')');
                            break;
                        case U')':
                            reversed.push_back(U'(');
                            break;
                        case U'[':
                            reversed.push_back(U']');
                            break;
                        case U']':
                            reversed.push_back(U'[');
                            break;
                        case U'{':
                            reversed.push_back(U'}');
                            break;
                        case U'}':
                            reversed.push_back(U'{');
                            break;
                        default:
                            reversed.push_back(*(ch - 1));
                        }
                    }
                    hb_buffer_add_utf32(iBuf, &*reversed.begin(), static_cast<int>(reversed.size()), 0u, static_cast<int>(reversed.size()));
                }
                scoped_kerning sk{ aFont.kerning() };
                hb_shape(iFont, iBuf, NULL, 0);
                unsigned int glyphCount = 0;
                iGlyphInfo = hb_buffer_get_glyph_infos(iBuf, &glyphCount);
                iGlyphPos = hb_buffer_get_glyph_positions(iBuf, &glyphCount);
                iGlyphCount = glyphCount;
                if (std::get<2>(aGlyphRun) == text_direction::None_RTL)
                    for (uint32_t i = 0; i < iGlyphCount; ++i)
                        iGlyphInfo[i].cluster = static_cast<uint32_t>(std::get<1>(aGlyphRun) - std::get<0>(aGlyphRun) - 1 - iGlyphInfo[i].cluster);
            }
            ~glyphs()
            {
                hb_buffer_clear_contents(iBuf);
            }
        public:
            uint32_t glyph_count() const
            {
                return iGlyphCount;
            }
            const hb_glyph_info_t& glyph_info(uint32_t aIndex) const
            {
                return iGlyphInfo[aIndex];
            }
            const hb_glyph_position_t& glyph_position(uint32_t aIndex) const
            {
                return iGlyphPos[aIndex];
            }
            bool needs_fallback_font() const
            {
                for (uint32_t i = 0; i < glyph_count(); ++i)
                {
                    auto const tc = get_text_category(service<i_font_manager>().emoji_atlas(), std::get<0>(iGlyphRun), std::get<1>(iGlyphRun));
                    if (glyph_info(i).codepoint == 0 && tc != text_category::Whitespace && tc != text_category::Emoji)
                        return true;
                }
                return false;
            }
        private:
            const i_graphics_context& iParent;
            hb_font_t* iFont;
            const glyph_text_factory::glyph_run& iGlyphRun;
            hb_buffer_t* iBuf;
            uint32_t iGlyphCount;
            hb_glyph_info_t* iGlyphInfo;
            hb_glyph_position_t* iGlyphPos;
        };
        typedef std::list<glyphs> glyphs_list;
        typedef std::vector<std::pair<glyphs_list::const_iterator, uint32_t>> result_type;
    public:
        glyph_shapes(const i_graphics_context& aParent, const font& aFont, const glyph_text_factory::glyph_run& aGlyphRun)
        {
            thread_local std::vector<font> fontsTried;
            auto tryFont = aFont;
            fontsTried.push_back(aFont);
            iGlyphsList.emplace_back(glyphs{ aParent, tryFont, aGlyphRun });
            while (iGlyphsList.back().needs_fallback_font())
            {
                if (tryFont.has_fallback() && std::find(fontsTried.begin(), fontsTried.end(), tryFont.fallback()) == fontsTried.end())
                {
                    tryFont = tryFont.fallback();
                    fontsTried.push_back(tryFont);
                    iGlyphsList.emplace_back(glyphs{ aParent, tryFont, aGlyphRun });
                }
                else
                {
                    std::u32string lastResort{ std::get<0>(aGlyphRun), std::get<1>(aGlyphRun) };
                    for (uint32_t i = 0; i < iGlyphsList.back().glyph_count(); ++i)
                        if (iGlyphsList.back().glyph_info(i).codepoint == 0)
                            lastResort[iGlyphsList.back().glyph_info(i).cluster] = neolib::INVALID_CHAR32; // replacement character
                    iGlyphsList.emplace_back(glyphs{ aParent, aFont, glyph_text_factory::glyph_run{&lastResort[0], &lastResort[0] + lastResort.size(), std::get<2>(aGlyphRun), std::get<3>(aGlyphRun), std::get<4>(aGlyphRun) } });
                    break;
                }
            }
            fontsTried.clear();
            auto const g = iGlyphsList.begin();
            iResults.reserve(g->glyph_count());
            for (uint32_t i = 0; i < g->glyph_count();)
            {
                auto const& gi = g->glyph_info(i);
                auto tc = get_text_category(service<i_font_manager>().emoji_atlas(), std::get<0>(aGlyphRun) + gi.cluster, std::get<1>(aGlyphRun));
                if (gi.codepoint != 0 || tc == text_category::Whitespace || tc == text_category::Emoji)
                    iResults.push_back(std::make_pair(g, i++));
                else
                {
                    std::vector<uint32_t> clusters;
                    while (i < g->glyph_count() && g->glyph_info(i).codepoint == 0 && tc != text_category::Whitespace && tc != text_category::Emoji)
                    {
                        clusters.push_back(g->glyph_info(i).cluster);
                        ++i;
                    }
                    std::sort(clusters.begin(), clusters.end());
                    auto nextFallback = std::next(g);
                    while (nextFallback != iGlyphsList.end() && !clusters.empty())
                    {
                        auto currentFallback = nextFallback++;
                        auto const& fallbackGlyphs = *currentFallback;
                        for (uint32_t j = 0; j < fallbackGlyphs.glyph_count(); ++j)
                        {
                            if (fallbackGlyphs.glyph_info(j).codepoint != 0)
                            {
                                auto const c = std::find(clusters.begin(), clusters.end(), fallbackGlyphs.glyph_info(j).cluster);
                                if (c != clusters.end())
                                {
                                    iResults.push_back(std::make_pair(currentFallback, j));
                                    clusters.erase(c);
                                }
                            }
                            else
                            {
                                tc = get_text_category(service<i_font_manager>().emoji_atlas(), std::get<0>(aGlyphRun) + fallbackGlyphs.glyph_info(j).cluster, std::get<1>(aGlyphRun));
                                if (tc != text_category::Whitespace && tc != text_category::Emoji)
                                    break;
                                else
                                    goto whitespace_break;
                            }
                        }
                    }
                }
            whitespace_break:
                ;
            }
        }
    public:
        uint32_t glyph_count() const
        {
            return static_cast<uint32_t>(iResults.size());
        }
        const hb_glyph_info_t& glyph_info(uint32_t aIndex) const
        {
            return iResults[aIndex].first->glyph_info(iResults[aIndex].second);
        }
        const hb_glyph_position_t& glyph_position(uint32_t aIndex) const
        {
            return iResults[aIndex].first->glyph_position(iResults[aIndex].second);
        }
        bool using_fallback(uint32_t aIndex) const
        {
            return iResults[aIndex].first != iGlyphsList.begin();
        }
        uint32_t fallback_index(uint32_t aIndex) const
        {
            if (!using_fallback(aIndex))
                throw not_using_fallback();
            return static_cast<uint32_t>(std::distance(iGlyphsList.begin(), iResults[aIndex].first) - 1);
        }
    private:
        glyphs_list iGlyphsList;
        result_type iResults;
    };

    glyph_text glyph_text_factory::create_glyph_text(font const& aFont)
    {
        return *make_ref<glyph_text_content>(aFont);
    }

    glyph_text glyph_text_factory::to_glyph_text(i_graphics_context const& aContext, char const* aUtf8Begin, char const* aUtf8End, i_font_selector const& aFontSelector)
    {
        auto& clusterMap = iClusterMap;
        clusterMap.clear();
        iCodePointsBuffer.clear();
        std::u32string& codePoints = iCodePointsBuffer;

        codePoints = neolib::utf8_to_utf32(std::string_view{ aUtf8Begin, aUtf8End }, [&clusterMap](std::string::size_type aFrom, std::u32string::size_type)
        {
            clusterMap.push_back(glyph_text_factory::cluster{ aFrom });
        });

        if (codePoints.empty())
            return aFontSelector.select_font(0);

        return to_glyph_text(aContext, codePoints.data(), codePoints.data() + codePoints.size(), font_selector{ [&aFontSelector, &clusterMap](std::u32string::size_type aIndex)->font
        {
            return aFontSelector.select_font(clusterMap[aIndex].from);
        } });
    }

    glyph_text glyph_text_factory::to_glyph_text(i_graphics_context const& aContext, char32_t const*  aUtf32Begin, char32_t const* aUtf32End, i_font_selector const& aFontSelector)
    {
        auto refResult = make_ref<glyph_text_content>(aFontSelector.select_font(0));
        auto& result = *refResult;

        if (aUtf32End == aUtf32Begin)
            return result;

        bool hasEmojis = false;

        auto& textDirections = iTextDirections;
        textDirections.clear();

        std::u32string::size_type codePointCount = aUtf32End - aUtf32Begin;

        std::u32string adjustedCodepoints;
        if (aContext.password())
            adjustedCodepoints.assign(codePointCount, neolib::utf8_to_utf32(aContext.password_mask())[0]);
        auto codePoints = adjustedCodepoints.empty() ? &*aUtf32Begin : &adjustedCodepoints[0];

        auto& runs = iRuns;
        runs.clear();
        auto const& emojiAtlas = service<i_font_manager>().emoji_atlas();
        text_category previousCategory = get_text_category(emojiAtlas, codePoints, codePoints + codePointCount);
        if (aContext.mnemonic_set() && codePoints[0] == static_cast<char32_t>(aContext.mnemonic()) && 
            (codePointCount == 1 || codePoints[1] != static_cast<char32_t>(aContext.mnemonic())))
            previousCategory = text_category::Mnemonic;
        text_direction previousDirection = (previousCategory != text_category::RTL ? text_direction::LTR : text_direction::RTL);
        const char32_t* runStart = &codePoints[0];
        std::u32string::size_type lastCodePointIndex = codePointCount - 1;
        font previousFont = aFontSelector.select_font(0);
        hb_script_t previousScript = hb_unicode_script(static_cast<font_face_handle*>(previousFont.native_font_face().handle())->harfbuzzUnicodeFuncs, codePoints[0]);

        std::deque<std::pair<text_direction, bool>> directionStack;
        const char32_t LRE = U'\u202A';
        const char32_t RLE = U'\u202B';
        const char32_t LRO = U'\u202D';
        const char32_t RLO = U'\u202E';
        const char32_t PDF = U'\u202C';

        bool currentLineHasLTR = false;

        for (std::size_t codePointIndex = 0; codePointIndex <= lastCodePointIndex; ++codePointIndex)
        {
            font const currentFont = aFontSelector.select_font(codePointIndex);
            switch (codePoints[codePointIndex])
            {
            case PDF:
                if (!directionStack.empty())
                    directionStack.pop_back();
                break;
            case LRE:
                directionStack.push_back(std::make_pair(text_direction::LTR, false));
                break;
            case RLE:
                directionStack.push_back(std::make_pair(text_direction::RTL, false));
                break;
            case LRO:
                directionStack.push_back(std::make_pair(text_direction::LTR, true));
                break;
            case RLO:
                directionStack.push_back(std::make_pair(text_direction::RTL, true));
                break;
            default:
                break;
            }

            hb_unicode_funcs_t* unicodeFuncs = static_cast<font_face_handle*>(currentFont.native_font_face().handle())->harfbuzzUnicodeFuncs;
            text_category currentCategory = get_text_category(emojiAtlas, codePoints + codePointIndex, codePoints + codePointCount);
            if (aContext.mnemonic_set() && codePoints[codePointIndex] == static_cast<char32_t>(aContext.mnemonic()) &&
                (codePointCount - 1 == codePointIndex || codePoints[codePointIndex + 1] != static_cast<char32_t>(aContext.mnemonic())))
                currentCategory = text_category::Mnemonic;
            text_direction currentDirection = previousDirection;
            if (currentCategory == text_category::LTR)
                currentDirection = text_direction::LTR;
            else if (currentCategory == text_category::RTL)
                currentDirection = text_direction::RTL;

            bool newLine = (codePoints[codePointIndex] == '\r' || codePoints[codePointIndex] == '\n');
            if (newLine)
            {
                currentLineHasLTR = false;
                currentDirection = text_direction::LTR;
            }
            auto bidi_check = [&directionStack](text_category aCategory, text_direction aDirection)
            {
                if (!directionStack.empty())
                {
                    switch (aCategory)
                    {
                    case text_category::LTR:
                    case text_category::RTL:
                    case text_category::Digit:
                    case text_category::Emoji:
                        if (directionStack.back().second == true)
                            return directionStack.back().first;
                        break;
                    case text_category::Mark:
                    case text_category::None:
                    case text_category::Whitespace:
                    case text_category::Mnemonic:
                        return directionStack.back().first;
                        break;
                    default:
                        break;
                    }
                }
                return aDirection;
            };
            currentDirection = bidi_check(currentCategory, currentDirection);
            switch (currentCategory)
            {
            case text_category::LTR:
                currentLineHasLTR = true;
                if (currentDirection == text_direction::None_LTR || currentDirection == text_direction::None_RTL ||
                    currentDirection == text_direction::Digit_LTR || currentDirection == text_direction::Digit_RTL ||
                    currentDirection == text_direction::Emoji_LTR || currentDirection == text_direction::Emoji_RTL)
                    currentDirection = text_direction::LTR;
                break;
            case text_category::RTL:
                if (currentDirection == text_direction::None_LTR || currentDirection == text_direction::None_RTL ||
                    currentDirection == text_direction::Digit_LTR || currentDirection == text_direction::Digit_RTL ||
                    currentDirection == text_direction::Emoji_LTR || currentDirection == text_direction::Emoji_RTL)
                    currentDirection = text_direction::RTL;
                break;
            case text_category::None:
                if (currentDirection == text_direction::LTR)
                    currentDirection = text_direction::None_LTR;
                else if (currentDirection == text_direction::RTL)
                    currentDirection = text_direction::None_RTL;
                break;
            case text_category::Digit:
                if (currentDirection == text_direction::LTR)
                    currentDirection = text_direction::Digit_LTR;
                else if (currentDirection == text_direction::RTL)
                    currentDirection = text_direction::Digit_RTL;
                break;
            case text_category::Emoji:
                if (currentDirection == text_direction::LTR)
                    currentDirection = text_direction::Emoji_LTR;
                else if (currentDirection == text_direction::RTL)
                    currentDirection = text_direction::Emoji_RTL;
                break;
            }
            if (currentDirection == text_direction::None_LTR || currentDirection == text_direction::Digit_LTR) // optimization (less runs for LTR text)
                currentDirection = text_direction::LTR;
            hb_script_t currentScript = hb_unicode_script(unicodeFuncs, codePoints[codePointIndex]);
            if (currentScript == HB_SCRIPT_COMMON || currentScript == HB_SCRIPT_INHERITED)
                currentScript = previousScript;
            bool newRun =
                previousFont != currentFont ||
                (newLine && (previousDirection == text_direction::RTL || previousDirection == text_direction::None_RTL || previousDirection == text_direction::Digit_RTL || previousDirection == text_direction::Emoji_RTL)) ||
                currentCategory == text_category::Mnemonic ||
                previousCategory == text_category::Mnemonic ||
                previousDirection != currentDirection ||
                previousScript != currentScript;
            if (!newRun)
            {
                if ((currentCategory == text_category::Whitespace || currentCategory == text_category::None || currentCategory == text_category::Mnemonic) &&
                    (currentDirection == text_direction::RTL || currentDirection == text_direction::None_RTL || currentDirection == text_direction::Digit_RTL || currentDirection == text_direction::Emoji_RTL))
                {
                    for (std::size_t j = codePointIndex + 1; j <= lastCodePointIndex; ++j)
                    {
                        text_direction nextDirection = bidi_check(get_text_category(emojiAtlas, codePoints + j, codePoints + codePointCount), get_text_direction(emojiAtlas, codePoints + j, codePoints + codePointCount, currentDirection));
                        if (nextDirection == text_direction::RTL)
                            break;
                        else if (nextDirection == text_direction::LTR || (j == lastCodePointIndex && currentLineHasLTR))
                        {
                            newRun = true;
                            currentDirection = text_direction::LTR;
                            break;
                        }
                    }
                }
            }
            textDirections.push_back(character_type{ currentCategory, currentDirection });
            if (currentCategory == text_category::Emoji)
                hasEmojis = true;
            if (newRun && codePointIndex > 0)
            {
                runs.push_back(std::make_tuple(runStart, &codePoints[codePointIndex], previousDirection, previousCategory == text_category::Mnemonic, previousScript));
                runStart = &codePoints[codePointIndex];
            }
            previousDirection = currentDirection;
            previousCategory = currentCategory;
            previousScript = currentScript;
            if (codePointIndex == lastCodePointIndex)
                runs.push_back(std::make_tuple(runStart, &codePoints[codePointIndex + 1], previousDirection, previousCategory == text_category::Mnemonic, previousScript));
            if (newLine && (newRun || codePointIndex == lastCodePointIndex))
            {
                for (auto i = runs.rbegin(); i != runs.rend(); ++i)
                {
                    if (std::get<2>(*i) == text_direction::RTL)
                        break;
                    else
                    {
                        switch (std::get<2>(*i))
                        {
                        case text_direction::None_RTL:
                            std::get<2>(*i) = text_direction::None_LTR;
                            break;
                        case text_direction::Digit_RTL:
                            std::get<2>(*i) = text_direction::LTR;
                            break;
                        case text_direction::Emoji_RTL:
                            std::get<2>(*i) = text_direction::Emoji_LTR;
                            break;
                        }
                    }
                }
            }
            previousFont = currentFont;
        }

        for (std::size_t i = 1; i < runs.size(); ++i)
        {
            std::size_t j = i - 1;
            auto startDirection = std::get<2>(runs[j]);
            do
            {
                auto direction = std::get<2>(runs[i]);
                if ((startDirection == text_direction::RTL || startDirection == text_direction::None_RTL || startDirection == text_direction::Digit_RTL || startDirection == text_direction::Emoji_RTL) &&
                    (direction == text_direction::RTL || direction == text_direction::None_RTL || direction == text_direction::Digit_RTL || direction == text_direction::Emoji_RTL))
                {
                    auto m = runs[i];
                    runs.erase(runs.begin() + i);
                    runs.insert(runs.begin() + j, m);
                    ++i;
                }
                else
                {
                    break;
                }
            } while (i < runs.size());
        }

        for (std::size_t i = 0; i < runs.size(); ++i)
        {
            if (std::get<3>(runs[i]))
                continue;
            bool drawMnemonic = (i > 0 && std::get<3>(runs[i - 1]));
            std::string::size_type sourceClusterRunStart = std::get<0>(runs[i]) - &codePoints[0];
            glyph_shapes shapes{ aContext, aFontSelector.select_font(sourceClusterRunStart), runs[i] };
            for (uint32_t j = 0; j < shapes.glyph_count(); ++j)
            {
                std::u32string::size_type startCluster = shapes.glyph_info(j).cluster;
                std::u32string::size_type endCluster;
                if (std::get<2>(runs[i]) != text_direction::RTL)
                {
                    uint32_t k = j + 1;
                    while (k < shapes.glyph_count() && shapes.glyph_info(k).cluster == startCluster)
                        ++k;
                    endCluster = (k < shapes.glyph_count() ? shapes.glyph_info(k).cluster : startCluster + 1);
                }
                else
                {
                    uint32_t k = j;
                    while (k > 0 && shapes.glyph_info(k).cluster == startCluster)
                        --k;
                    endCluster = (shapes.glyph_info(k).cluster != startCluster ? shapes.glyph_info(k).cluster : startCluster + 1);
                }
                startCluster += (std::get<0>(runs[i]) - &codePoints[0]);
                endCluster += (std::get<0>(runs[i]) - &codePoints[0]);
                neogfx::font selectedFont = aFontSelector.select_font(startCluster);
                neogfx::font font = selectedFont;
                if (shapes.using_fallback(j))
                {
                    font = font.has_fallback() ? font.fallback() : selectedFont;
                    for (auto fi = shapes.fallback_index(j); font != selectedFont && fi > 0; --fi)
                        font = font.has_fallback() ? font.fallback() : selectedFont;
                }
                size advance = textDirections[startCluster].category != text_category::Emoji ?
                    size{ shapes.glyph_position(j).x_advance / 64.0, shapes.glyph_position(j).y_advance / 64.0 } :
                    size{ font.height(), 0.0 };
                auto& newGlyph = result.emplace_back(
                    textDirections[startCluster],
                    shapes.glyph_info(j).codepoint,
                    glyph::flags_e{},
                    glyph::source_type{ static_cast<uint32_t>(startCluster), static_cast<uint32_t>(endCluster) },
                    font.id(),
                    advance, point(shapes.glyph_position(j).x_offset / 64.0, shapes.glyph_position(j).y_offset / 64.0),
                    size{advance.cx, font.height()});
                if (category(newGlyph) == text_category::Whitespace)
                    newGlyph.value = aUtf32Begin[startCluster];
                else if (category(newGlyph) == text_category::Emoji)
                    newGlyph.value = emojiAtlas.emoji(aUtf32Begin[startCluster], font.height());
                if ((selectedFont.style() & font_style::Underline) == font_style::Underline)
                    set_underline(newGlyph, true);
                if ((selectedFont.style() & font_style::Superscript) == font_style::Superscript)
                    set_superscript(newGlyph, true, (selectedFont.style() & font_style::BelowAscenderLine) == font_style::BelowAscenderLine);
                if ((selectedFont.style() & font_style::Subscript) == font_style::Subscript)
                    set_subscript(newGlyph, true, (selectedFont.style() & font_style::AboveBaseline) == font_style::AboveBaseline);
                if (aContext.is_subpixel_rendering_on() && !font.is_bitmap_font())
                    set_subpixel(newGlyph, true);
                if (drawMnemonic && ((j == 0 && std::get<2>(runs[i]) == text_direction::LTR) || (j == shapes.glyph_count() - 1 && std::get<2>(runs[i]) == text_direction::RTL)))
                    set_mnemonic(newGlyph, true);
            }
        }
        if (hasEmojis)
        {
            auto refEmojiResult = make_ref<glyph_text_content>(aFontSelector.select_font(0));
            auto& emojiResult = *refEmojiResult;
            for (auto i = result.begin(); i != result.end(); ++i)
            {
                auto cluster = i->source.first;
                auto chStart = aUtf32Begin[cluster];
                if (category(*i) == text_category::Emoji)
                {
                    if (!emojiResult.empty() && is_emoji(emojiResult.back()) && emojiResult.back().source == i->source)
                    {
                        // probable variant selector fubar'd by harfbuzz
                        auto s = emojiResult.back().source;
                        if (s.second < codePointCount && get_text_category(service<i_font_manager>().emoji_atlas(), aUtf32Begin[s.second]) == text_category::Control)
                        {
                            ++s.first;
                            ++s.second;
                            i->source = s;
                            set_category(*i, text_category::Control);
                            i->advance = size{};
                        }
                    }
                    std::u32string sequence;
                    sequence += chStart;
                    auto j = i + 1;
                    bool absorbNext = false;
                    for (; j != result.end(); ++j)
                    {
                        auto ch = aUtf32Begin[cluster + (j - i)];
                        if (ch == 0x200D)
                            continue;
                        else if (ch == 0xFE0F)
                        {
                            absorbNext = true;
                            break;
                        }
                        else if (service<i_font_manager>().emoji_atlas().is_emoji(sequence + ch))
                            sequence += ch;
                        else
                            break;
                    }
                    if (sequence.size() > 1 && service<i_font_manager>().emoji_atlas().is_emoji(sequence))
                    {
                        auto g = *i;
                        g.value = service<i_font_manager>().emoji_atlas().emoji(sequence, aFontSelector.select_font(cluster).height());
                        g.source = glyph::source_type{ g.source.first, g.source.first + static_cast<uint32_t>(sequence.size()) };
                        emojiResult.push_back(g);
                        i = j - 1;
                    }
                    else
                        emojiResult.push_back(*i);
                    if (absorbNext)
                    {
                        emojiResult.back().source = glyph::source_type{ emojiResult.back().source.first, emojiResult.back().source.first + static_cast<uint32_t>(sequence.size()) + 1u };
                        ++i;
                    }
                }
                else
                    emojiResult.push_back(*i);
            }
            return emojiResult.bottom_justify();
        }
        return result.bottom_justify();
    }

    font_manager::font_manager() :
        iGlyphTextFactory{ std::make_unique<neogfx::glyph_text_factory>() },
        iGlyphAtlas{ size{1024.0, 1024.0} },
        iEmojiAtlas{}
    {
        FT_Error error = FT_Init_FreeType(&iFontLib);
        if (error)
            throw error_initializing_font_library();
        error = FT_Library_SetLcdFilter(iFontLib, FT_LCD_FILTER_NONE);
        if (error)
            throw error_initializing_font_library();
        auto enumerate = [this](const std::string fontsDirectory)
        {
            if (std::filesystem::exists(fontsDirectory))
                for (std::filesystem::directory_iterator file(fontsDirectory); file != std::filesystem::directory_iterator(); ++file)
                {
                    if (!std::filesystem::is_regular_file(file->status()))
                        continue;
                    try
                    {
                        if (is_font_file(string{ file->path().string() }))
                        {
                            auto font = iNativeFonts.emplace(iNativeFonts.end(), iFontLib, file->path().string());
                            iFontFamilies[font->family_name()].push_back(font);
                        }
                    }
                    catch (native_font::failed_to_load_font&)
                    {
                    }
                    catch (...)
                    {
                        throw;
                    }
                }
        };
        enumerate(detail::platform_specific::get_system_font_directory());
        enumerate(detail::platform_specific::get_local_font_directory());
        for (auto& family : iFontFamilies)
        {
            std::optional<native_font_list::iterator> bold;
            std::optional<native_font_list::iterator> italic;
            std::optional<native_font_list::iterator> boldItalic;
            std::vector<native_font_list::iterator> emulatedBold;
            std::vector<native_font_list::iterator> emulatedItalic;
            std::vector<native_font_list::iterator> emulatedBoldItalic;
            for (auto& font : family.second)
            {
                if (font->has_style(font_style::Bold))
                    bold = font;
                if (font->has_style(font_style::Italic))
                    italic = font;
                if (font->has_style(font_style::BoldItalic))
                    boldItalic = font;
                if (font->has_style(font_style::EmulatedBold))
                    emulatedBold.push_back(font);
                if (font->has_style(font_style::EmulatedItalic))
                    emulatedItalic.push_back(font);
                if (font->has_style(font_style::EmulatedBoldItalic))
                    emulatedBoldItalic.push_back(font);
            }
            if (bold)
                for (auto& f : emulatedBold)
                    (*f).remove_style(font_style::EmulatedBold);
            if (italic)
                for (auto& f : emulatedItalic)
                    (*f).remove_style(font_style::EmulatedItalic);
            if (boldItalic)
                for (auto& f : emulatedBoldItalic)
                    (*f).remove_style(font_style::EmulatedBoldItalic);
            std::sort(family.second.begin(), family.second.end(),
                [](auto const& f1, auto const& f2) { return f1->min_style() < f2->min_style() || (f1->min_style() == f2->min_style() && f1->min_weight() < f2->min_weight()); });
        }
    }

    font_manager::~font_manager()
    {
        iIdCache.clear();
        iFontFamilies.clear();
        iNativeFonts.clear();
        FT_Done_FreeType(iFontLib);
    }

    void* font_manager::font_library_handle() const
    {
        return iFontLib;
    }

    i_optional<font_info> const& font_manager::default_system_font_info(system_font_role aRole) const
    {
        if (iDefaultSystemFontInfo[aRole] == std::nullopt)
            iDefaultSystemFontInfo[aRole] = detail::platform_specific::default_system_font_info(aRole);
        return iDefaultSystemFontInfo[aRole];
    }

    const i_fallback_font_info& font_manager::default_fallback_font_info() const
    {
        if (iDefaultFallbackFontInfo == std::nullopt)
            iDefaultFallbackFontInfo = detail::platform_specific::default_fallback_font_info();
        return *iDefaultFallbackFontInfo;
    }

    i_native_font_face& font_manager::create_default_font(const i_device_resolution& aDevice)
    {
        return create_font(service<i_app>().current_style().font_info(), aDevice);
    }

    bool font_manager::has_fallback_font(const i_native_font_face& aExistingFont) const
    {
        return default_fallback_font_info().has_fallback_for(aExistingFont.family_name());
    }
        
    i_native_font_face& font_manager::create_fallback_font(const i_native_font_face& aExistingFont)
    {
        if (!has_fallback_font(aExistingFont))
            throw no_fallback_font();
        if (aExistingFont.fallback_cached())
            return aExistingFont.fallback();
        struct : i_device_resolution
        {
            size iResolution;
            virtual dimension horizontal_dpi() const { return iResolution.cx; }
            virtual dimension vertical_dpi() const { return iResolution.cy; }
            virtual dimension ppi() const { return iResolution.magnitude() / std::sqrt(2.0); }
        } deviceResolution;
        deviceResolution.iResolution = size(aExistingFont.horizontal_dpi(), aExistingFont.vertical_dpi());
        auto& fallbackFont = create_font(default_fallback_font_info().fallback_for(aExistingFont.family_name()), aExistingFont.style_name(), aExistingFont.size(), deviceResolution);
        return fallbackFont;
    }

    i_native_font_face& font_manager::create_font(i_string const& aFamilyName, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice)
    {
        return add_font(find_best_font(aFamilyName, aStyle, aSize).create_face(aStyle, aSize, aDevice));
    }

    i_native_font_face& font_manager::create_font(i_string const& aFamilyName, i_string const& aStyleName, font::point_size aSize, const i_device_resolution& aDevice)
    {
        return add_font(find_font(aFamilyName, aStyleName, aSize).create_face(aStyleName, aSize, aDevice));
    }

    i_native_font_face& font_manager::create_font(const font_info& aInfo, const i_device_resolution& aDevice)
    {
        if (aInfo.style_name_available())
            return create_font(aInfo.family_name(), aInfo.style_name(), aInfo.size(), aDevice);
        else
            return create_font(aInfo.family_name(), aInfo.style(), aInfo.size(), aDevice);
    }

    i_native_font_face& font_manager::create_font(i_native_font& aFont, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice)
    {
        return add_font(aFont.create_face(aStyle, aSize, aDevice));
    }

    i_native_font_face& font_manager::create_font(i_native_font& aFont, i_string const& aStyleName, font::point_size aSize, const i_device_resolution& aDevice)
    {
        return add_font(aFont.create_face(aStyleName, aSize, aDevice));
    }

    bool font_manager::is_font_file(i_string const& aFileName) const
    {
        FT_Face face;
        FT_Error error = FT_New_Face(iFontLib, aFileName.c_str(), 0, &face);
        if (error)
            return false;
        FT_Done_Face(face);
        return true;
    }

    i_native_font_face& font_manager::load_font_from_file(i_string const& aFileName, const i_device_resolution& aDevice)
    {
        throw std::logic_error("neogfx::font_manager::load_font_from_file function overload not yet implemented");
        (void)aFileName;
        (void)aDevice;
    }

    i_native_font_face& font_manager::load_font_from_file(i_string const& aFileName, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice)
    {
        throw std::logic_error("neogfx::font_manager::load_font_from_file function overload not yet implemented");
        (void)aFileName;
        (void)aStyle;
        (void)aSize;
        (void)aDevice;
    }

    i_native_font_face& font_manager::load_font_from_file(i_string const& aFileName, i_string const& aStyleName, font::point_size aSize, const i_device_resolution& aDevice)
    {
        throw std::logic_error("neogfx::font_manager::load_font_from_file function overload not yet implemented");
        (void)aFileName;
        (void)aStyleName;
        (void)aSize;
        (void)aDevice;
    }

    i_native_font_face& font_manager::load_font_from_memory(const void* aData, std::size_t aSizeInBytes, const i_device_resolution& aDevice)
    {
        throw std::logic_error("neogfx::font_manager::load_font_from_memory function overload not yet implemented");
        (void)aData;
        (void)aSizeInBytes;
        (void)aDevice;
    }

    i_native_font_face& font_manager::load_font_from_memory(const void* aData, std::size_t aSizeInBytes, neogfx::font_style aStyle, font::point_size aSize, const i_device_resolution& aDevice)
    {
        throw std::logic_error("neogfx::font_manager::load_font_from_memory function overload not yet implemented");
        (void)aData;
        (void)aSizeInBytes;
        (void)aStyle;
        (void)aSize;
        (void)aDevice;
    }

    i_native_font_face& font_manager::load_font_from_memory(const void* aData, std::size_t aSizeInBytes, i_string const& aStyleName, font::point_size aSize, const i_device_resolution& aDevice)
    {
        throw std::logic_error("neogfx::font_manager::load_font_from_memory function overload not yet implemented");
        (void)aData;
        (void)aSizeInBytes;
        (void)aStyleName;
        (void)aSize;
        (void)aDevice;
    }

    uint32_t font_manager::font_family_count() const
    {
        return static_cast<uint32_t>(iFontFamilies.size());
    }

    i_string const& font_manager::font_family(uint32_t aFamilyIndex) const
    {
        if (aFamilyIndex < font_family_count())
            return std::next(iFontFamilies.begin(), aFamilyIndex)->first;
        throw bad_font_family_index();
    }

    uint32_t font_manager::font_style_count(uint32_t aFamilyIndex) const
    {
        if (aFamilyIndex < font_family_count())
        {
            uint32_t styles = 0;
            for (auto& font : std::next(iFontFamilies.begin(), aFamilyIndex)->second)
                styles += font->style_count();
            return styles;
        }
        throw bad_font_family_index();
    }

    font_style font_manager::font_style(uint32_t aFamilyIndex, uint32_t aStyleIndex) const
    {
        if (aFamilyIndex < font_family_count() && aStyleIndex < font_style_count(aFamilyIndex))
        {
            for (auto& font : std::next(iFontFamilies.begin(), aFamilyIndex)->second)
            {
                if (aStyleIndex < font->style_count())
                    return font->style(aStyleIndex);
                aStyleIndex -= font->style_count();
            }
        }
        throw bad_font_family_index();
    }

    i_string const& font_manager::font_style_name(uint32_t aFamilyIndex, uint32_t aStyleIndex) const
    {
        if (aFamilyIndex < font_family_count() && aStyleIndex < font_style_count(aFamilyIndex))
        {
            for (auto& font : std::next(iFontFamilies.begin(), aFamilyIndex)->second)
            {
                if (aStyleIndex < font->style_count())
                    return font->style_name(aStyleIndex);
                aStyleIndex -= font->style_count();
            }
        }
        throw bad_font_family_index();
    }

    font_id font_manager::allocate_font_id()
    {
        return iIdCache.next_cookie();
    }

    const font& font_manager::font_from_id(font_id aId) const
    {
        return iIdCache[aId];
    }

    i_glyph_text_factory& font_manager::glyph_text_factory() const
    {
        return *iGlyphTextFactory;
    }

    const i_texture_atlas& font_manager::glyph_atlas() const
    {
        return iGlyphAtlas;
    }

    i_texture_atlas& font_manager::glyph_atlas()
    {
        return iGlyphAtlas;
    }

    const i_emoji_atlas& font_manager::emoji_atlas() const
    {
        return iEmojiAtlas;
    }

    i_emoji_atlas& font_manager::emoji_atlas()
    {
        return iEmojiAtlas;
    }

    void font_manager::add_ref(font_id aId)
    {
        font_from_id(aId).native_font_face().add_ref();
    }

    void font_manager::release(font_id aId)
    {
        font_from_id(aId).native_font_face().release();
    }

    long font_manager::use_count(font_id aId) const
    {
        return font_from_id(aId).native_font_face().use_count();
    }

    i_native_font& font_manager::find_font(i_string const& aFamilyName, i_string const& aStyleName, font::point_size aSize)
    {
        auto family = iFontFamilies.find(aFamilyName);
        if (family == iFontFamilies.end() && default_system_font_info(system_font_role::Widget) != std::nullopt)
            family = iFontFamilies.find(default_system_font_info(system_font_role::Widget)->family_name());
        if (family == iFontFamilies.end())
            throw no_matching_font_found();
        std::multimap<uint32_t, native_font_list::iterator> matches;
        for (auto& f : family->second)
        {
            for (uint32_t s = 0; s < f->style_count(); ++s)
                if (neolib::ci_equal_to{}(f->style_name(s), aStyleName))
                    return *f;
        }
        return find_best_font(aFamilyName, font_style::Normal, aSize);
    }

    namespace
    {
        uint32_t matching_bits(uint32_t lhs, uint32_t rhs)
        {
            if (lhs == rhs)
                return 32;
            uint32_t matches = 0;
            uint32_t test = 1;
            while (test != 0)
            {
                if ((lhs & rhs) & test)
                    ++matches;
                test <<= 1;
            }
            return matches;
        }
    }

    i_native_font& font_manager::find_best_font(i_string const& aFamilyName, neogfx::font_style aStyle, font::point_size)
    {
        auto family = iFontFamilies.find(aFamilyName);
        if (family == iFontFamilies.end() && default_system_font_info(system_font_role::Widget) != std::nullopt)
            family = iFontFamilies.find(default_system_font_info(system_font_role::Widget)->family_name());
        if (family == iFontFamilies.end())
            throw no_matching_font_found();
        struct match
        {
            uint32_t matchingBits;
            neogfx::font_style style;
            font_weight weight;
            i_native_font* font;
        };
        std::optional<match> bestNormalFont;
        std::optional<match> bestBoldFont;
        std::optional<match> bestOtherFont;
        for (auto& f : family->second)
        {
            for (uint32_t s = 0; s < f->style_count(); ++s)
            {
                auto const matchingBits = matching_bits(static_cast<uint32_t>(f->style(s)), static_cast<uint32_t>(aStyle));
                auto const& styleName = f->style_name(s);
                auto const weight = font::weight_from_style_name(styleName);
                if (weight <= font_weight::Normal && (
                    bestNormalFont == std::nullopt ||
                    bestNormalFont->matchingBits < matchingBits ||
                    (bestNormalFont->matchingBits == matchingBits && bestNormalFont->weight < weight)))
                {
                    bestNormalFont = match{ matchingBits, f->style(s), weight, &*f };
                }
                else if (weight >= font_weight::Bold && (
                    bestBoldFont == std::nullopt ||
                    bestBoldFont->matchingBits < matchingBits ||
                    (bestBoldFont->style & font_style::Emulated) == font_style::Emulated ||
                    (bestBoldFont->matchingBits == matchingBits && bestBoldFont->weight > weight)))
                {
                    bestBoldFont = match{ matchingBits, f->style(s), weight, &*f };
                }
                else if (bestOtherFont == std::nullopt ||
                    bestOtherFont->matchingBits < matchingBits ||
                    (bestOtherFont->style & font_style::Emulated) == font_style::Emulated ||
                    (bestOtherFont->matchingBits == matchingBits && bestOtherFont->weight < weight))
                {
                    bestOtherFont = match{ matchingBits, f->style(s), weight, &*f };
                }
            }
        }
        if ((aStyle & neogfx::font_style::Bold) != neogfx::font_style::Bold)
        {
            if (bestNormalFont != std::nullopt)
                return *bestNormalFont->font;
            else if (bestOtherFont != std::nullopt)
                return *bestOtherFont->font;
            else if (bestBoldFont != std::nullopt)
                return *bestBoldFont->font;
            else
                throw no_matching_font_found();
        }
        else
        {
            if (bestBoldFont != std::nullopt)
                return *bestBoldFont->font;
            else if (bestOtherFont != std::nullopt)
                return *bestOtherFont->font;
            else if (bestNormalFont != std::nullopt)
                return *bestNormalFont->font;
            else
                throw no_matching_font_found();
        }
    }

    i_native_font_face& font_manager::add_font(const ref_ptr<i_native_font_face>& aNewFont)
    {
        if (!iIdCache.contains(aNewFont->id()))
            iIdCache.add(aNewFont->id(), font{ *aNewFont });
        // cleanup opportunity
        cleanup();
        return *aNewFont;
    }

    void font_manager::cleanup()
    {
        for (auto i = iIdCache.begin(); i != iIdCache.end();)
        {
            auto& cacheEntry = *i;
            if (cacheEntry.native_font_face().use_count() == 1)
                i = iIdCache.erase(i);
            else
                ++i;
        }
    }
}