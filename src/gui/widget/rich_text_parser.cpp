// rich_text_parser.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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
#include <array>
#include <charconv>
#include <functional>
#include <map>
#include <utility>

#include "rich_text_parser.hpp"

namespace neogfx::rich_text
{
    namespace
    {
        // ---------------------------------------------------------------------------------
        // Utilities
        // ---------------------------------------------------------------------------------

        // Line prefixes are built from these private use characters; builder::finish() turns them into each paragraph's
        // indent level and list marker (span_format::indent and listMarker) so no spaces are used for indentation.
        constexpr std::string_view QuoteMarker = "\xEE\x80\x80"; // U+E000: one indent level for a block quote
        constexpr std::string_view ListIndent = "\xEE\x80\x81"; // U+E001: one indent level for a list item's continuation
        constexpr std::string_view MarkerBegin = "\xEE\x80\x82"; // U+E002: a list item's marker follows (one indent level) ...
        constexpr std::string_view MarkerEnd = "\xEE\x80\x83"; // U+E003: ... up to here
        constexpr std::string_view ObjectReplacementCharacter = "\xEF\xBF\xBC"; // U+FFFC
        constexpr std::array<std::string_view, 3> Bullets = { "\xE2\x80\xA2", "\xE2\x97\xA6", "\xE2\x96\xAA" }; // U+2022, U+25E6, U+25AA
        constexpr std::string_view TaskUnchecked = "\xE2\x98\x90"; // U+2610
        constexpr std::string_view TaskChecked = "\xE2\x98\x91"; // U+2611
        constexpr std::array<double, 6> HeadingScales = { 2.0, 1.5, 1.17, 1.0, 0.83, 0.67 };
        constexpr double BodyMargin = 8.0; // CSS px
        constexpr std::array<double, 6> HeadingMargins = { 0.67, 0.83, 1.0, 1.33, 1.67, 2.33 }; // in ems of the heading's own font
        constexpr std::size_t MaxLinkTextLength = 1024;
        constexpr std::size_t MaxDelimiterRun = 64;

        void append_utf8(std::string& aOut, char32_t aCodePoint)
        {
            if (aCodePoint == 0 || aCodePoint > 0x10FFFF || (aCodePoint >= 0xD800 && aCodePoint <= 0xDFFF))
                aCodePoint = 0xFFFD;
            if (aCodePoint < 0x80)
                aOut += static_cast<char>(aCodePoint);
            else if (aCodePoint < 0x800)
            {
                aOut += static_cast<char>(0xC0 | (aCodePoint >> 6));
                aOut += static_cast<char>(0x80 | (aCodePoint & 0x3F));
            }
            else if (aCodePoint < 0x10000)
            {
                aOut += static_cast<char>(0xE0 | (aCodePoint >> 12));
                aOut += static_cast<char>(0x80 | ((aCodePoint >> 6) & 0x3F));
                aOut += static_cast<char>(0x80 | (aCodePoint & 0x3F));
            }
            else
            {
                aOut += static_cast<char>(0xF0 | (aCodePoint >> 18));
                aOut += static_cast<char>(0x80 | ((aCodePoint >> 12) & 0x3F));
                aOut += static_cast<char>(0x80 | ((aCodePoint >> 6) & 0x3F));
                aOut += static_cast<char>(0x80 | (aCodePoint & 0x3F));
            }
        }

        bool is_space(char aChar)
        {
            return aChar == ' ' || aChar == '\t' || aChar == '\n' || aChar == '\r' || aChar == '\f' || aChar == '\v';
        }

        bool is_ascii_alpha(char aChar)
        {
            return (aChar >= 'a' && aChar <= 'z') || (aChar >= 'A' && aChar <= 'Z');
        }

        bool is_ascii_digit(char aChar)
        {
            return aChar >= '0' && aChar <= '9';
        }

        bool is_ascii_alnum(char aChar)
        {
            return is_ascii_alpha(aChar) || is_ascii_digit(aChar);
        }

        bool is_ascii_punct(char aChar)
        {
            return (aChar >= '!' && aChar <= '/') || (aChar >= ':' && aChar <= '@') || (aChar >= '[' && aChar <= '`') || (aChar >= '{' && aChar <= '~');
        }

        char to_lower(char aChar)
        {
            return (aChar >= 'A' && aChar <= 'Z') ? static_cast<char>(aChar - 'A' + 'a') : aChar;
        }

        std::string to_lower(std::string_view aText)
        {
            std::string result{ aText };
            for (auto& ch : result)
                ch = to_lower(ch);
            return result;
        }

        std::string_view trim(std::string_view aText)
        {
            while (!aText.empty() && is_space(aText.front()))
                aText.remove_prefix(1);
            while (!aText.empty() && is_space(aText.back()))
                aText.remove_suffix(1);
            return aText;
        }

        bool istarts_with(std::string_view aText, std::string_view aPrefix)
        {
            if (aText.size() < aPrefix.size())
                return false;
            for (std::size_t i = 0; i < aPrefix.size(); ++i)
                if (to_lower(aText[i]) != to_lower(aPrefix[i]))
                    return false;
            return true;
        }

        std::size_t ifind(std::string_view aText, std::string_view aWhat, std::size_t aFrom = 0)
        {
            for (std::size_t i = aFrom; i + aWhat.size() <= aText.size(); ++i)
                if (istarts_with(aText.substr(i), aWhat))
                    return i;
            return std::string_view::npos;
        }

        std::optional<double> parse_number(std::string_view aText, std::size_t& aLength)
        {
            std::size_t i = 0;
            std::string digits;
            if (i < aText.size() && (aText[i] == '+' || aText[i] == '-'))
                digits += aText[i++];
            bool any = false;
            while (i < aText.size() && (is_ascii_digit(aText[i]) || aText[i] == '.'))
            {
                digits += aText[i++];
                any = true;
            }
            if (!any)
                return {};
            aLength = i;
            double value = 0.0;
            auto const first = digits.data() + (digits[0] == '+' ? 1 : 0);
            auto const result = std::from_chars(first, digits.data() + digits.size(), value);
            if (result.ec != std::errc{})
                return {};
            return value;
        }

        // ---------------------------------------------------------------------------------
        // Character references
        // ---------------------------------------------------------------------------------

        std::map<std::string_view, char32_t> const& named_entities()
        {
            static std::map<std::string_view, char32_t> const sEntities = {
                { "amp", U'&' }, { "lt", U'<' }, { "gt", U'>' }, { "quot", U'"' }, { "apos", U'\'' },
                { "nbsp", 0x00A0 }, { "ensp", 0x2002 }, { "emsp", 0x2003 }, { "thinsp", 0x2009 },
                { "zwnj", 0x200C }, { "zwj", 0x200D }, { "shy", 0x00AD },
                { "copy", 0x00A9 }, { "reg", 0x00AE }, { "trade", 0x2122 }, { "hellip", 0x2026 },
                { "mdash", 0x2014 }, { "ndash", 0x2013 }, { "lsquo", 0x2018 }, { "rsquo", 0x2019 },
                { "sbquo", 0x201A }, { "ldquo", 0x201C }, { "rdquo", 0x201D }, { "bdquo", 0x201E },
                { "laquo", 0x00AB }, { "raquo", 0x00BB }, { "lsaquo", 0x2039 }, { "rsaquo", 0x203A },
                { "bull", 0x2022 }, { "middot", 0x00B7 }, { "deg", 0x00B0 }, { "plusmn", 0x00B1 },
                { "times", 0x00D7 }, { "divide", 0x00F7 }, { "minus", 0x2212 }, { "le", 0x2264 },
                { "ge", 0x2265 }, { "ne", 0x2260 }, { "asymp", 0x2248 }, { "infin", 0x221E },
                { "euro", 0x20AC }, { "pound", 0x00A3 }, { "yen", 0x00A5 }, { "cent", 0x00A2 },
                { "curren", 0x00A4 }, { "sect", 0x00A7 }, { "para", 0x00B6 }, { "dagger", 0x2020 },
                { "Dagger", 0x2021 }, { "permil", 0x2030 }, { "prime", 0x2032 }, { "Prime", 0x2033 },
                { "larr", 0x2190 }, { "uarr", 0x2191 }, { "rarr", 0x2192 }, { "darr", 0x2193 },
                { "harr", 0x2194 }, { "lArr", 0x21D0 }, { "rArr", 0x21D2 }, { "hArr", 0x21D4 },
                { "hearts", 0x2665 }, { "spades", 0x2660 }, { "clubs", 0x2663 }, { "diams", 0x2666 },
                { "frac12", 0x00BD }, { "frac14", 0x00BC }, { "frac34", 0x00BE },
                { "sup1", 0x00B9 }, { "sup2", 0x00B2 }, { "sup3", 0x00B3 }, { "micro", 0x00B5 },
                { "iexcl", 0x00A1 }, { "iquest", 0x00BF }, { "ordf", 0x00AA }, { "ordm", 0x00BA },
                { "not", 0x00AC }, { "macr", 0x00AF }, { "acute", 0x00B4 }, { "cedil", 0x00B8 },
                { "uml", 0x00A8 }, { "brvbar", 0x00A6 }, { "alpha", 0x03B1 }, { "beta", 0x03B2 },
                { "gamma", 0x03B3 }, { "delta", 0x03B4 }, { "pi", 0x03C0 }, { "sigma", 0x03C3 },
                { "mu", 0x03BC }, { "lambda", 0x03BB }, { "omega", 0x03C9 }, { "Omega", 0x03A9 },
                { "Delta", 0x0394 }, { "Sigma", 0x03A3 }, { "sum", 0x2211 }, { "prod", 0x220F },
                { "radic", 0x221A }, { "check", 0x2713 },
                { "Agrave", 0x00C0 }, { "Aacute", 0x00C1 }, { "Acirc", 0x00C2 }, { "Atilde", 0x00C3 },
                { "Auml", 0x00C4 }, { "Aring", 0x00C5 }, { "AElig", 0x00C6 }, { "Ccedil", 0x00C7 },
                { "Egrave", 0x00C8 }, { "Eacute", 0x00C9 }, { "Ecirc", 0x00CA }, { "Euml", 0x00CB },
                { "Igrave", 0x00CC }, { "Iacute", 0x00CD }, { "Icirc", 0x00CE }, { "Iuml", 0x00CF },
                { "ETH", 0x00D0 }, { "Ntilde", 0x00D1 }, { "Ograve", 0x00D2 }, { "Oacute", 0x00D3 },
                { "Ocirc", 0x00D4 }, { "Otilde", 0x00D5 }, { "Ouml", 0x00D6 }, { "Oslash", 0x00D8 },
                { "Ugrave", 0x00D9 }, { "Uacute", 0x00DA }, { "Ucirc", 0x00DB }, { "Uuml", 0x00DC },
                { "Yacute", 0x00DD }, { "THORN", 0x00DE }, { "szlig", 0x00DF },
                { "agrave", 0x00E0 }, { "aacute", 0x00E1 }, { "acirc", 0x00E2 }, { "atilde", 0x00E3 },
                { "auml", 0x00E4 }, { "aring", 0x00E5 }, { "aelig", 0x00E6 }, { "ccedil", 0x00E7 },
                { "egrave", 0x00E8 }, { "eacute", 0x00E9 }, { "ecirc", 0x00EA }, { "euml", 0x00EB },
                { "igrave", 0x00EC }, { "iacute", 0x00ED }, { "icirc", 0x00EE }, { "iuml", 0x00EF },
                { "eth", 0x00F0 }, { "ntilde", 0x00F1 }, { "ograve", 0x00F2 }, { "oacute", 0x00F3 },
                { "ocirc", 0x00F4 }, { "otilde", 0x00F5 }, { "ouml", 0x00F6 }, { "oslash", 0x00F8 },
                { "ugrave", 0x00F9 }, { "uacute", 0x00FA }, { "ucirc", 0x00FB }, { "uuml", 0x00FC },
                { "yacute", 0x00FD }, { "thorn", 0x00FE }, { "yuml", 0x00FF }, { "OElig", 0x0152 },
                { "oelig", 0x0153 }, { "Scaron", 0x0160 }, { "scaron", 0x0161 }, { "Yuml", 0x0178 }
            };
            return sEntities;
        }

        // aText starts at '&'; on success aLength is the length of the reference.
        std::optional<std::string> decode_entity(std::string_view aText, std::size_t& aLength)
        {
            if (aText.size() < 3 || aText[0] != '&')
                return {};
            auto const semicolon = aText.find(';', 1);
            if (semicolon == std::string_view::npos || semicolon > 33)
                return {};
            auto const name = aText.substr(1, semicolon - 1);
            std::string result;
            if (name[0] == '#')
            {
                bool const hex = name.size() > 1 && (name[1] == 'x' || name[1] == 'X');
                auto const digits = name.substr(hex ? 2 : 1);
                if (digits.empty() || digits.size() > (hex ? 6u : 7u))
                    return {};
                std::uint32_t value = 0;
                auto const parsed = std::from_chars(digits.data(), digits.data() + digits.size(), value, hex ? 16 : 10);
                if (parsed.ec != std::errc{} || parsed.ptr != digits.data() + digits.size())
                    return {};
                append_utf8(result, static_cast<char32_t>(value));
            }
            else
            {
                auto const existing = named_entities().find(name);
                if (existing == named_entities().end())
                    return {};
                append_utf8(result, existing->second);
            }
            aLength = semicolon + 1;
            return result;
        }

        std::string decode_entities(std::string_view aText)
        {
            std::string result;
            for (std::size_t i = 0; i < aText.size();)
            {
                std::size_t length = 0;
                if (aText[i] == '&')
                    if (auto const decoded = decode_entity(aText.substr(i), length))
                    {
                        result += *decoded;
                        i += length;
                        continue;
                    }
                result += aText[i++];
            }
            return result;
        }

        // Markdown: backslash escapes and character references.
        std::string unescape_markdown(std::string_view aText)
        {
            std::string result;
            for (std::size_t i = 0; i < aText.size();)
            {
                if (aText[i] == '\\' && i + 1 < aText.size() && is_ascii_punct(aText[i + 1]))
                {
                    result += aText[i + 1];
                    i += 2;
                    continue;
                }
                std::size_t length = 0;
                if (aText[i] == '&')
                    if (auto const decoded = decode_entity(aText.substr(i), length))
                    {
                        result += *decoded;
                        i += length;
                        continue;
                    }
                result += aText[i++];
            }
            return result;
        }

        // ---------------------------------------------------------------------------------
        // Output
        // ---------------------------------------------------------------------------------

        class builder
        {
        public:
            void text(std::string_view aText, span_format const& aFormat)
            {
                if (iTrimLeadingSpace)
                {
                    while (!aText.empty() && aText.front() == ' ')
                        aText.remove_prefix(1);
                    if (!aText.empty())
                        iTrimLeadingSpace = false;
                }
                if (aText.empty())
                    return;
                flush_breaks();
                append(aText, aFormat);
            }
            void newline()
            {
                flush_breaks();
                append("\n", span_format{});
            }
            void line_break()
            {
                if (!iDocument.text.empty())
                    iPendingBreak = true;
            }
            // starts a block: its top margin and padding go on the next paragraph (where blocks open together the larger
            // margin wins, as CSS margin collapsing; paddings add)
            void block_break(css_length aMargin, css_length aPadding = {})
            {
                if (!iDocument.text.empty())
                {
                    iPendingBreak = true;
                    iPendingMargin = larger(iPendingMargin, aMargin);
                    iPendingPadding = sum(iPendingPadding, aPadding);
                }
            }
            // ends a block: its bottom margin and padding go on the last paragraph
            void end_block(css_length aMargin, css_length aPadding)
            {
                if (iDocument.text.empty())
                    return;
                if (aMargin != css_length{} || aPadding != css_length{})
                {
                    auto& box = box_of(iDocument.text.back() == '\n' && iParagraph > 0 ? iParagraph - 1 : iParagraph);
                    box.marginBottom = larger(box.marginBottom, aMargin);
                    box.paddingBottom = sum(box.paddingBottom, aPadding);
                }
                iPendingBreak = true;
            }
            bool at_line_start() const
            {
                return iDocument.text.empty() || iDocument.text.back() == '\n' || iPendingBreak;
            }
            void object(inline_object const& aObject, span_format aFormat)
            {
                // display math is a centred line of its own
                if (aObject.kind == inline_object::kind_e::Math && aObject.display)
                {
                    line_break();
                    aFormat.alignment = block_alignment::Center;
                }
                aFormat.object = aObject;
                flush_breaks();
                append(ObjectReplacementCharacter, aFormat);
                if (aObject.kind == inline_object::kind_e::Math && aObject.display)
                {
                    line_break();
                    iTrimLeadingSpace = true;
                }
                else
                    iTrimLeadingSpace = false;
            }
            document finish()
            {
                if (iBoxes.empty() && iDocument.text.find("\xEE\x80") == std::string::npos &&
                    std::none_of(iDocument.runs.begin(), iDocument.runs.end(), [](run const& aRun)
                        { return aRun.format.marginLeft != 0.0 || aRun.format.marginRight != 0.0 || aRun.format.paddingLeft != 0.0 || aRun.format.paddingRight != 0.0; }))
                    return std::move(iDocument);
                // give every run of each paragraph the paragraph's top margin and, from its line prefix, its indent and list marker
                auto const& text = iDocument.text;
                auto const& runs = iDocument.runs;
                auto run_at = [&](std::size_t aOffset) -> run const&
                {
                    return *std::upper_bound(runs.begin(), runs.end(), aOffset, [](std::size_t aValue, run const& aRun) { return aValue < aRun.end; });
                };
                builder result;
                std::size_t paragraphIndex = 0;
                for (std::size_t paragraph = 0; paragraph < text.size(); ++paragraphIndex)
                {
                    auto paragraphEnd = text.find('\n', paragraph);
                    paragraphEnd = (paragraphEnd == std::string::npos ? text.size() : paragraphEnd + 1);
                    auto const box = paragraphIndex < iBoxes.size() ? iBoxes[paragraphIndex] : paragraph_box{};
                    std::uint32_t indent = 0;
                    std::uint32_t rightIndent = 0;
                    std::string marker;
                    std::optional<std::size_t> markerStart;
                    auto i = paragraph;
                    for (;;)
                    {
                        auto const prefix = std::string_view{ text }.substr(i, 3);
                        if (prefix == QuoteMarker || prefix == ListIndent)
                        {
                            ++indent;
                            if (prefix == QuoteMarker)
                                ++rightIndent;
                            i += 3;
                        }
                        else if (prefix == MarkerBegin)
                        {
                            auto const end = text.find(MarkerEnd, i + 3);
                            if (end == std::string::npos || end > paragraphEnd)
                                break;
                            if (!markerStart)
                                markerStart = i;
                            marker.append(text, i + 3, end - (i + 3));
                            ++indent;
                            i = end + 3;
                        }
                        else
                            break;
                    }
                    auto const& first = run_at(paragraph).format;
                    auto paragraph_format = [&](span_format aFormat)
                    {
                        aFormat.indent = indent;
                        aFormat.rightIndent = rightIndent;
                        aFormat.marginTop = box.marginTop;
                        aFormat.marginBottom = box.marginBottom;
                        aFormat.paddingTop = box.paddingTop;
                        aFormat.paddingBottom = box.paddingBottom;
                        aFormat.marginLeft = first.marginLeft;
                        aFormat.marginRight = first.marginRight;
                        aFormat.paddingLeft = first.paddingLeft;
                        aFormat.paddingRight = first.paddingRight;
                        return aFormat;
                    };
                    if (!marker.empty())
                    {
                        auto format = paragraph_format(run_at(*markerStart).format);
                        format.listMarker = true;
                        result.append(marker, format);
                    }
                    while (i < paragraphEnd)
                    {
                        auto const& r = run_at(i);
                        auto const end = std::min(r.end, paragraphEnd);
                        result.append(std::string_view{ text }.substr(i, end - i), paragraph_format(r.format));
                        i = end;
                    }
                    paragraph = paragraphEnd;
                }
                return std::move(result.iDocument);
            }
        private:
            struct paragraph_box
            {
                css_length marginTop;
                css_length marginBottom;
                css_length paddingTop;
                css_length paddingBottom;
            };
            paragraph_box& box_of(std::size_t aParagraph)
            {
                if (iBoxes.size() <= aParagraph)
                    iBoxes.resize(aParagraph + 1);
                return iBoxes[aParagraph];
            }
            static css_length larger(css_length const& aLhs, css_length const& aRhs)
            {
                // only for choosing: an em is taken as 16px
                return aRhs.px + aRhs.em * 16.0 > aLhs.px + aLhs.em * 16.0 ? aRhs : aLhs;
            }
            static css_length sum(css_length const& aLhs, css_length const& aRhs)
            {
                return css_length{ aLhs.px + aRhs.px, aLhs.em + aRhs.em };
            }
            void flush_breaks()
            {
                if (iPendingBreak && !iDocument.text.empty() && iDocument.text.back() != '\n')
                    append("\n", span_format{});
                iPendingBreak = false;
            }
            void append(std::string_view aText, span_format const& aFormat)
            {
                if ((iPendingMargin != css_length{} || iPendingPadding != css_length{}) && (iDocument.text.empty() || iDocument.text.back() == '\n'))
                {
                    // the paragraph starting here has the pending top margin and padding
                    auto& box = box_of(iParagraph);
                    box.marginTop = iPendingMargin;
                    box.paddingTop = iPendingPadding;
                    iPendingMargin = {};
                    iPendingPadding = {};
                }
                iDocument.text += aText;
                iParagraph += static_cast<std::size_t>(std::count(aText.begin(), aText.end(), '\n'));
                if (!iDocument.runs.empty() && iDocument.runs.back().format == aFormat)
                    iDocument.runs.back().end = iDocument.text.size();
                else
                    iDocument.runs.push_back(run{ iDocument.text.size(), aFormat });
            }
        private:
            document iDocument;
            bool iPendingBreak = false;
            css_length iPendingMargin;
            css_length iPendingPadding;
            std::size_t iParagraph = 0; // the paragraph being appended to
            std::vector<paragraph_box> iBoxes; // by paragraph
            bool iTrimLeadingSpace = false;
        };

        span_format prefix_format(span_format const& aFormat)
        {
            span_format result;
            result.alignment = aFormat.alignment;
            result.marginLeft = aFormat.marginLeft;
            result.marginRight = aFormat.marginRight;
            result.paddingLeft = aFormat.paddingLeft;
            result.paddingRight = aFormat.paddingRight;
            return result;
        }

        // ---------------------------------------------------------------------------------
        // HTML tags and the Qt rich text subset of tag/attribute/CSS formatting
        // ---------------------------------------------------------------------------------

        struct html_tag
        {
            std::string name;
            bool closing = false;
            bool selfClosing = false;
            std::map<std::string, std::string> attributes;
            std::size_t end = 0;

            std::optional<std::string> attribute(std::string const& aName) const
            {
                auto const existing = attributes.find(aName);
                if (existing != attributes.end())
                    return existing->second;
                return {};
            }
        };

        // aText[aPos] == '<'
        std::optional<html_tag> parse_tag(std::string_view aText, std::size_t aPos)
        {
            html_tag result;
            std::size_t i = aPos + 1;
            if (i < aText.size() && aText[i] == '/')
            {
                result.closing = true;
                ++i;
            }
            if (i >= aText.size() || !is_ascii_alpha(aText[i]))
                return {};
            while (i < aText.size() && (is_ascii_alnum(aText[i]) || aText[i] == '-' || aText[i] == ':'))
                result.name += to_lower(aText[i++]);
            if (i < aText.size() && !is_space(aText[i]) && aText[i] != '>' && aText[i] != '/')
                return {};
            for (;;)
            {
                while (i < aText.size() && is_space(aText[i]))
                    ++i;
                if (i >= aText.size())
                    return {};
                if (aText[i] == '>')
                {
                    result.end = i + 1;
                    return result;
                }
                if (aText[i] == '/' && i + 1 < aText.size() && aText[i + 1] == '>')
                {
                    result.selfClosing = true;
                    result.end = i + 2;
                    return result;
                }
                std::string name;
                while (i < aText.size() && !is_space(aText[i]) && aText[i] != '>' && aText[i] != '=' && !(aText[i] == '/' && i + 1 < aText.size() && aText[i + 1] == '>'))
                    name += to_lower(aText[i++]);
                if (name.empty())
                {
                    ++i;
                    continue;
                }
                while (i < aText.size() && is_space(aText[i]))
                    ++i;
                std::string value;
                if (i < aText.size() && aText[i] == '=')
                {
                    ++i;
                    while (i < aText.size() && is_space(aText[i]))
                        ++i;
                    if (i < aText.size() && (aText[i] == '"' || aText[i] == '\''))
                    {
                        auto const quote = aText[i++];
                        auto const close = aText.find(quote, i);
                        if (close == std::string_view::npos)
                            return {};
                        value = decode_entities(aText.substr(i, close - i));
                        i = close + 1;
                    }
                    else
                    {
                        auto const start = i;
                        while (i < aText.size() && !is_space(aText[i]) && aText[i] != '>')
                            ++i;
                        value = decode_entities(aText.substr(start, i - start));
                    }
                }
                result.attributes.emplace(std::move(name), std::move(value));
            }
        }

        bool is_void_element(std::string_view aName)
        {
            static constexpr std::array<std::string_view, 14> sVoid = {
                "area", "base", "br", "col", "embed", "hr", "img", "input", "link", "meta", "param", "source", "track", "wbr" };
            return std::find(sVoid.begin(), sVoid.end(), aName) != sVoid.end();
        }

        block_alignment parse_alignment(std::string_view aValue, block_alignment aDefault)
        {
            auto const value = to_lower(trim(aValue));
            if (value == "left" || value == "start")
                return block_alignment::Left;
            if (value == "right" || value == "end")
                return block_alignment::Right;
            if (value == "center" || value == "centre" || value == "middle")
                return block_alignment::Center;
            if (value == "justify")
                return block_alignment::Justify;
            return aDefault;
        }

        void scale_font(span_format& aFormat, double aScale)
        {
            if (aFormat.pointSize)
                *aFormat.pointSize *= aScale;
            else
                aFormat.fontScale *= aScale;
        }

        void apply_font_size(span_format& aFormat, std::string_view aValue)
        {
            auto const value = to_lower(trim(aValue));
            static std::map<std::string, double, std::less<>> const sKeywords = {
                { "xx-small", 0.6 }, { "x-small", 0.75 }, { "small", 0.89 }, { "medium", 1.0 },
                { "large", 1.2 }, { "x-large", 1.5 }, { "xx-large", 2.0 }, { "xxx-large", 3.0 } };
            if (auto const keyword = sKeywords.find(value); keyword != sKeywords.end())
            {
                aFormat.pointSize = std::nullopt;
                aFormat.fontScale = keyword->second;
                return;
            }
            if (value == "smaller")
                return scale_font(aFormat, 0.83);
            if (value == "larger")
                return scale_font(aFormat, 1.2);
            std::size_t length = 0;
            auto const number = parse_number(value, length);
            if (!number || *number <= 0.0)
                return;
            auto const unit = std::string_view{ value }.substr(length);
            if (unit == "pt")
                aFormat.pointSize = *number;
            else if (unit == "px" || unit.empty())
                aFormat.pointSize = *number * 0.75;
            else if (unit == "em" || unit == "rem")
                scale_font(aFormat, *number);
            else if (unit == "%")
                scale_font(aFormat, *number / 100.0);
        }

        void apply_font_family(span_format& aFormat, std::string_view aValue)
        {
            auto first = trim(aValue.substr(0, aValue.find(',')));
            if (first.size() >= 2 && (first.front() == '\'' || first.front() == '"') && first.back() == first.front())
                first = trim(first.substr(1, first.size() - 2));
            if (first.empty())
                return;
            auto const lower = to_lower(first);
            if (lower == "monospace" || lower == "courier" || lower == "courier new")
            {
                aFormat.monospace = true;
                aFormat.fontFamily = std::nullopt;
            }
            else if (lower == "sans-serif" || lower == "serif" || lower == "system-ui" || lower == "inherit")
                aFormat.fontFamily = std::nullopt;
            else
            {
                aFormat.monospace = false;
                aFormat.fontFamily = std::string{ first };
            }
        }

        bool is_color_value(std::string_view aValue)
        {
            auto const value = to_lower(trim(aValue));
            return !value.empty() && value != "transparent" && value != "inherit" && value != "initial" && value != "none";
        }

        void apply_css(span_format& aFormat, bool& aPreserveWhitespace, std::string_view aStyle)
        {
            std::size_t start = 0;
            while (start < aStyle.size())
            {
                auto end = aStyle.find(';', start);
                if (end == std::string_view::npos)
                    end = aStyle.size();
                auto const declaration = aStyle.substr(start, end - start);
                start = end + 1;
                auto const colon = declaration.find(':');
                if (colon == std::string_view::npos)
                    continue;
                auto const property = to_lower(trim(declaration.substr(0, colon)));
                auto valueText = trim(declaration.substr(colon + 1));
                if (auto const important = ifind(valueText, "!important"); important != std::string_view::npos)
                    valueText = trim(valueText.substr(0, important));
                auto const value = to_lower(valueText);
                if (property == "color")
                {
                    if (is_color_value(valueText))
                        aFormat.textColor = std::string{ valueText };
                }
                else if (property == "background-color" || property == "background")
                {
                    auto const color = property == "background" ? trim(valueText.substr(0, valueText.find(' '))) : valueText;
                    if (is_color_value(color) && !istarts_with(color, "url("))
                        aFormat.backgroundColor = std::string{ color };
                }
                else if (property == "font-weight")
                {
                    std::size_t length = 0;
                    auto const weight = parse_number(value, length);
                    aFormat.bold = weight ? *weight >= 600.0 : (value == "bold" || value == "bolder");
                }
                else if (property == "font-style")
                    aFormat.italic = (value == "italic" || value == "oblique");
                else if (property == "text-decoration" || property == "text-decoration-line")
                {
                    if (value.find("none") != std::string::npos)
                        aFormat.underline = aFormat.strike = false;
                    if (value.find("underline") != std::string::npos)
                        aFormat.underline = true;
                    if (value.find("line-through") != std::string::npos)
                        aFormat.strike = true;
                }
                else if (property == "font-size")
                    apply_font_size(aFormat, value);
                else if (property == "font-family")
                    apply_font_family(aFormat, valueText);
                else if (property == "text-align")
                    aFormat.alignment = parse_alignment(value, aFormat.alignment);
                else if (property == "white-space")
                    aPreserveWhitespace = value.starts_with("pre");
                else if (property == "vertical-align")
                {
                    aFormat.superscript = (value == "super");
                    aFormat.subscript = (value == "sub");
                }
            }
        }

        // a CSS length in CSS pixels (96 per inch); em and rem are taken as 16px, the browser default font size
        std::optional<double> parse_css_length(std::string_view aValue)
        {
            auto const value = to_lower(trim(aValue));
            std::size_t length = 0;
            auto const number = parse_number(value, length);
            if (!number)
                return {};
            auto const unit = std::string_view{ value }.substr(length);
            if (unit == "px" || (unit.empty() && *number == 0.0))
                return *number;
            if (unit == "pt")
                return *number * 96.0 / 72.0;
            if (unit == "pc")
                return *number * 16.0;
            if (unit == "in")
                return *number * 96.0;
            if (unit == "cm")
                return *number * 96.0 / 2.54;
            if (unit == "mm")
                return *number * 96.0 / 25.4;
            if (unit == "em" || unit == "rem")
                return *number * 16.0;
            return {};
        }

        // a vertical margin or padding: em relative to the element's font, rem to the base font, other units as CSS pixels
        std::optional<css_length> parse_css_box_length(std::string_view aValue, double aFontScale)
        {
            auto const value = to_lower(trim(aValue));
            std::size_t length = 0;
            auto const number = parse_number(value, length);
            if (!number)
                return {};
            auto const unit = std::string_view{ value }.substr(length);
            if (unit == "em")
                return css_length{ 0.0, *number * aFontScale };
            if (unit == "rem")
                return css_length{ 0.0, *number };
            if (auto const px = parse_css_length(value))
                return css_length{ *px, 0.0 };
            return {};
        }

        struct vertical_box
        {
            std::optional<css_length> marginTop;
            std::optional<css_length> marginBottom;
            std::optional<css_length> paddingTop;
            std::optional<css_length> paddingBottom;
        };

        // A block's margins and padding from its style attribute (longhands and the margin/padding shorthands): left and
        // right are added to those of the blocks enclosing it (aDefaultMargin unless given); top and bottom are returned,
        // as they belong to the block.
        vertical_box apply_css_box(span_format& aFormat, std::string_view aStyle, double aDefaultMargin = 0.0)
        {
            vertical_box result;
            std::optional<double> marginLeft, marginRight, paddingLeft, paddingRight;
            std::size_t start = 0;
            while (start < aStyle.size())
            {
                auto end = aStyle.find(';', start);
                if (end == std::string_view::npos)
                    end = aStyle.size();
                auto const declaration = aStyle.substr(start, end - start);
                start = end + 1;
                auto const colon = declaration.find(':');
                if (colon == std::string_view::npos)
                    continue;
                auto const property = to_lower(trim(declaration.substr(0, colon)));
                auto value = trim(declaration.substr(colon + 1));
                if (auto const important = ifind(value, "!important"); important != std::string_view::npos)
                    value = trim(value.substr(0, important));
                if (property == "margin" || property == "padding")
                {
                    // 1 to 4 values: top [right [bottom [left]]]
                    std::vector<std::string_view> values;
                    for (std::size_t i = 0; i < value.size();)
                    {
                        while (i < value.size() && is_space(value[i]))
                            ++i;
                        auto const first = i;
                        while (i < value.size() && !is_space(value[i]))
                            ++i;
                        if (i > first)
                            values.push_back(value.substr(first, i - first));
                    }
                    if (values.empty() || values.size() > 4)
                        continue;
                    auto const right = parse_css_length(values.size() >= 2 ? values[1] : values[0]);
                    auto const left = parse_css_length(values.size() == 4 ? values[3] : values.size() >= 2 ? values[1] : values[0]);
                    auto const top = parse_css_box_length(values[0], aFormat.fontScale);
                    auto const bottom = parse_css_box_length(values.size() >= 3 ? values[2] : values[0], aFormat.fontScale);
                    bool const margin = (property == "margin");
                    (margin ? marginLeft : paddingLeft) = left;
                    (margin ? marginRight : paddingRight) = right;
                    (margin ? result.marginTop : result.paddingTop) = top;
                    (margin ? result.marginBottom : result.paddingBottom) = bottom;
                }
                else if (property == "margin-top" || property == "margin-block-start")
                    result.marginTop = parse_css_box_length(value, aFormat.fontScale);
                else if (property == "margin-bottom" || property == "margin-block-end")
                    result.marginBottom = parse_css_box_length(value, aFormat.fontScale);
                else if (property == "padding-top" || property == "padding-block-start")
                    result.paddingTop = parse_css_box_length(value, aFormat.fontScale);
                else if (property == "padding-bottom" || property == "padding-block-end")
                    result.paddingBottom = parse_css_box_length(value, aFormat.fontScale);
                else if (property == "margin-left" || property == "margin-inline-start")
                    marginLeft = parse_css_length(value);
                else if (property == "margin-right" || property == "margin-inline-end")
                    marginRight = parse_css_length(value);
                else if (property == "padding-left" || property == "padding-inline-start")
                    paddingLeft = parse_css_length(value);
                else if (property == "padding-right" || property == "padding-inline-end")
                    paddingRight = parse_css_length(value);
            }
            aFormat.marginLeft += marginLeft.value_or(aDefaultMargin);
            aFormat.marginRight += marginRight.value_or(aDefaultMargin);
            aFormat.paddingLeft += paddingLeft.value_or(0.0);
            aFormat.paddingRight += paddingRight.value_or(0.0);
            return result;
        }

        // Inline formatting implied by a tag and its attributes (also used for HTML inside Markdown).
        void apply_tag_format(html_tag const& aTag, span_format& aFormat, bool& aPreserveWhitespace)
        {
            auto const& name = aTag.name;
            if (name == "b" || name == "strong" || name == "th")
                aFormat.bold = true;
            else if (name == "i" || name == "em" || name == "cite" || name == "dfn" || name == "var" || name == "address")
                aFormat.italic = true;
            else if (name == "u" || name == "ins")
                aFormat.underline = true;
            else if (name == "s" || name == "strike" || name == "del")
                aFormat.strike = true;
            else if (name == "code" || name == "tt" || name == "kbd" || name == "samp")
            {
                aFormat.monospace = true;
                aFormat.fontFamily = std::nullopt;
            }
            else if (name == "pre" || name == "xmp" || name == "listing")
            {
                aFormat.monospace = true;
                aFormat.fontFamily = std::nullopt;
                aPreserveWhitespace = true;
            }
            else if (name == "sup")
            {
                aFormat.superscript = true;
                aFormat.subscript = false;
            }
            else if (name == "sub")
            {
                aFormat.subscript = true;
                aFormat.superscript = false;
            }
            else if (name == "big")
                scale_font(aFormat, 1.2);
            else if (name == "small")
                scale_font(aFormat, 0.83);
            else if (name == "mark")
                aFormat.backgroundColor = "yellow";
            else if (name == "center")
                aFormat.alignment = block_alignment::Center;
            else if (name.size() == 2 && name[0] == 'h' && name[1] >= '1' && name[1] <= '6')
            {
                aFormat.bold = true;
                scale_font(aFormat, HeadingScales[name[1] - '1']);
            }
            else if (name == "a")
            {
                if (auto const href = aTag.attribute("href"); href && !trim(*href).empty())
                    aFormat.link = std::string{ trim(*href) };
            }
            else if (name == "font")
            {
                if (auto const color = aTag.attribute("color"); color && is_color_value(*color))
                    aFormat.textColor = *color;
                if (auto const face = aTag.attribute("face"))
                    apply_font_family(aFormat, *face);
                if (auto const size = aTag.attribute("size"))
                {
                    static constexpr std::array<double, 7> sSizes = { 0.63, 0.82, 1.0, 1.13, 1.5, 2.0, 3.0 };
                    auto const text = trim(*size);
                    int value = 0;
                    auto const relative = !text.empty() && (text[0] == '+' || text[0] == '-');
                    auto const digits = relative ? text.substr(1) : text;
                    if (std::from_chars(digits.data(), digits.data() + digits.size(), value).ec == std::errc{})
                    {
                        if (relative)
                            value = 3 + (text[0] == '-' ? -value : value);
                        value = std::clamp(value, 1, 7);
                        aFormat.pointSize = std::nullopt;
                        aFormat.fontScale = sSizes[value - 1];
                    }
                }
            }
            if (auto const align = aTag.attribute("align"))
                aFormat.alignment = parse_alignment(*align, aFormat.alignment);
            if (auto const style = aTag.attribute("style"))
                apply_css(aFormat, aPreserveWhitespace, *style);
        }

        // ---------------------------------------------------------------------------------
        // HTML
        // ---------------------------------------------------------------------------------

        class html_parser
        {
        private:
            enum class element_kind : std::uint8_t
            {
                Inline,
                Line,       // starts and ends on its own line
                Block,      // separated from surrounding blocks by a blank line
                List,
                ListItem,
                Quote,
                Indented,   // <dd>
                Row,
                Cell,
                Ignored     // <head>, <title>
            };
            struct element
            {
                std::string name;
                element_kind kind;
                span_format format;
                bool preserveWhitespace;
                // lists
                bool ordered = false;
                std::string listType;
                long counter = 0;
                int listDepth = 0;
                // list items
                std::string marker;
                bool markerEmitted = false;
                // rows
                int cells = 0;
                // blocks: vertical margins and padding
                css_length marginTop;
                css_length marginBottom;
                css_length paddingTop;
                css_length paddingBottom;
            };
        public:
            html_parser(builder& aOutput, std::string aFirstPrefix = {}, std::string aRestPrefix = {}, int aListDepth = 0) :
                iOutput{ aOutput }, iFirstPrefix{ std::move(aFirstPrefix) }, iRestPrefix{ std::move(aRestPrefix) }, iBaseListDepth{ aListDepth }
            {
            }
        public:
            void parse(std::string_view aHtml)
            {
                aHtml = strip_clipboard_header(aHtml);
                for (std::size_t i = 0; i < aHtml.size();)
                {
                    if (aHtml[i] == '<')
                    {
                        if (aHtml.substr(i).starts_with("<!--"))
                        {
                            auto const end = aHtml.find("-->", i + 4);
                            i = (end == std::string_view::npos ? aHtml.size() : end + 3);
                            continue;
                        }
                        if (aHtml.substr(i).starts_with("<!") || aHtml.substr(i).starts_with("<?"))
                        {
                            auto const end = aHtml.find('>', i);
                            i = (end == std::string_view::npos ? aHtml.size() : end + 1);
                            continue;
                        }
                        if (auto tag = parse_tag(aHtml, i))
                        {
                            i = tag->end;
                            if (!tag->closing && (tag->name == "script" || tag->name == "style" || tag->name == "title" || tag->name == "textarea"))
                            {
                                auto const close = ifind(aHtml, "</" + tag->name, i);
                                if (close == std::string_view::npos)
                                    i = aHtml.size();
                                else
                                {
                                    auto const end = aHtml.find('>', close);
                                    i = (end == std::string_view::npos ? aHtml.size() : end + 1);
                                }
                                continue;
                            }
                            if (tag->closing)
                                end_tag(tag->name);
                            else
                                start_tag(*tag);
                            continue;
                        }
                    }
                    auto const next = aHtml.find('<', i + 1);
                    auto const end = (next == std::string_view::npos ? aHtml.size() : next);
                    text(aHtml.substr(i, end - i));
                    i = end;
                }
            }
        private:
            static std::string_view strip_clipboard_header(std::string_view aHtml)
            {
                // Windows "HTML Format" clipboard data
                if (!aHtml.starts_with("Version:"))
                    return aHtml;
                auto const start = aHtml.find("<!--StartFragment-->");
                auto const end = aHtml.find("<!--EndFragment-->");
                if (start != std::string_view::npos && end != std::string_view::npos && end > start)
                    return aHtml.substr(start + 20, end - (start + 20));
                auto const firstTag = aHtml.find('<');
                return firstTag == std::string_view::npos ? std::string_view{} : aHtml.substr(firstTag);
            }
            span_format const& current_format() const
            {
                static span_format const sDefault;
                return iStack.empty() ? sDefault : iStack.back().format;
            }
            bool preserve_whitespace() const
            {
                return !iStack.empty() && iStack.back().preserveWhitespace;
            }
            bool suppressed() const
            {
                return std::any_of(iStack.begin(), iStack.end(), [](element const& e) { return e.kind == element_kind::Ignored; });
            }
            int list_depth() const
            {
                int depth = iBaseListDepth;
                for (auto const& e : iStack)
                    if (e.kind == element_kind::List)
                        ++depth;
                return depth;
            }
            std::string line_prefix()
            {
                std::string result = iFirstUsed ? iRestPrefix : iFirstPrefix;
                iFirstUsed = true;
                for (auto& e : iStack)
                {
                    if (e.kind == element_kind::Quote)
                        result += QuoteMarker;
                    else if (e.kind == element_kind::Indented)
                        result += ListIndent;
                    else if (e.kind == element_kind::ListItem)
                    {
                        if (!e.markerEmitted)
                        {
                            result += MarkerBegin;
                            result += e.marker;
                            result += ' ';
                            result += MarkerEnd;
                            e.markerEmitted = true;
                        }
                        else
                            result += ListIndent;
                    }
                }
                return result;
            }
            void emit(std::string_view aText, span_format const& aFormat)
            {
                if (aText.empty() || suppressed())
                    return;
                if (iOutput.at_line_start())
                    iOutput.text(line_prefix(), prefix_format(aFormat));
                else if (iPendingSpace)
                    iOutput.text(" ", iPendingSpaceFormat);
                iPendingSpace = false;
                iOutput.text(aText, aFormat);
            }
            void text(std::string_view aRaw)
            {
                if (iCapture)
                {
                    *iCapture += decode_entities(aRaw);
                    return;
                }
                if (suppressed())
                    return;
                if (preserve_whitespace())
                    return text_segment(aRaw);
                // MathJax-style \(...\) and \[...\], and $$...$$, within a text node
                std::size_t position = 0;
                while (position < aRaw.size())
                {
                    auto const inlineOpen = aRaw.find("\\(", position);
                    auto const displayOpen = aRaw.find("\\[", position);
                    auto const dollarsOpen = aRaw.find("$$", position);
                    auto const open = std::min({ inlineOpen, displayOpen, dollarsOpen });
                    if (open == std::string_view::npos)
                        break;
                    std::string_view const close = (open == inlineOpen ? "\\)" : open == displayOpen ? "\\]" : "$$");
                    auto const end = aRaw.find(close, open + 2);
                    if (end == std::string_view::npos)
                        break;
                    text_segment(aRaw.substr(position, open - position));
                    auto const decoded = decode_entities(aRaw.substr(open + 2, end - (open + 2)));
                    auto const source = trim(decoded);
                    if (!source.empty())
                        emit_object(inline_object{ inline_object::kind_e::Math, std::string{ source }, open != inlineOpen });
                    position = end + 2;
                }
                text_segment(aRaw.substr(position));
            }
            void emit_object(inline_object const& aObject)
            {
                if (suppressed())
                    return;
                if (iOutput.at_line_start())
                    iOutput.text(line_prefix(), prefix_format(current_format()));
                else if (iPendingSpace)
                    iOutput.text(" ", iPendingSpaceFormat);
                iPendingSpace = false;
                iOutput.object(aObject, current_format());
            }
            void text_segment(std::string_view aRaw)
            {
                if (aRaw.empty())
                    return;
                auto const decoded = decode_entities(aRaw);
                auto const& format = current_format();
                if (preserve_whitespace())
                {
                    std::string_view rest = decoded;
                    if (iSkipLeadingNewline)
                    {
                        if (rest.starts_with("\r\n"))
                            rest.remove_prefix(2);
                        else if (rest.starts_with("\n"))
                            rest.remove_prefix(1);
                    }
                    iSkipLeadingNewline = false;
                    for (;;)
                    {
                        auto const eol = rest.find('\n');
                        auto line = rest.substr(0, eol);
                        if (!line.empty() && line.back() == '\r')
                            line.remove_suffix(1);
                        if (!line.empty())
                            emit(line, format);
                        else if (iOutput.at_line_start() && eol != std::string_view::npos)
                            iOutput.text(line_prefix(), prefix_format(format));
                        if (eol == std::string_view::npos)
                            break;
                        iOutput.newline();
                        rest.remove_prefix(eol + 1);
                    }
                    return;
                }
                iSkipLeadingNewline = false;
                std::size_t i = 0;
                while (i < decoded.size())
                {
                    if (is_space(decoded[i]))
                    {
                        if (!iOutput.at_line_start())
                        {
                            iPendingSpace = true;
                            iPendingSpaceFormat = format;
                        }
                        ++i;
                        continue;
                    }
                    auto const start = i;
                    while (i < decoded.size() && !is_space(decoded[i]))
                        ++i;
                    emit(std::string_view{ decoded }.substr(start, i - start), format);
                }
            }
            void open_block(element const& aElement)
            {
                iPendingSpace = false;
                iOutput.block_break(aElement.marginTop, aElement.paddingTop);
            }
            void close_block(element const& aElement)
            {
                iPendingSpace = false;
                iOutput.end_block(aElement.marginBottom, aElement.paddingBottom);
            }
            // browser default (HTML spec rendering section) vertical margins, in ems of the base font
            static double block_margin(std::string_view aName, span_format const& aFormat)
            {
                if (aName.size() == 2 && aName[0] == 'h' && aName[1] >= '1' && aName[1] <= '6')
                    return HeadingMargins[aName[1] - '1'] * aFormat.fontScale;
                if (aName == "table")
                    return 0.0;
                return 1.0;
            }
            element_kind kind_of(std::string_view aName) const
            {
                if (aName == "p" || aName == "pre" || aName == "xmp" || aName == "listing" || aName == "table" || aName == "dl" ||
                    (aName.size() == 2 && aName[0] == 'h' && aName[1] >= '1' && aName[1] <= '6'))
                    return element_kind::Block;
                if (aName == "ul" || aName == "ol" || aName == "menu" || aName == "dir")
                    return element_kind::List;
                if (aName == "li")
                    return element_kind::ListItem;
                if (aName == "blockquote" || aName == "figure")
                    return element_kind::Quote; // indented 40px on both sides
                if (aName == "dd")
                    return element_kind::Indented;
                if (aName == "tr")
                    return element_kind::Row;
                if (aName == "td" || aName == "th")
                    return element_kind::Cell;
                if (aName == "head" || aName == "title" || aName == "math" || aName == "svg")
                    return element_kind::Ignored;
                if (aName == "div" || aName == "center" || aName == "address" || aName == "dt" || aName == "caption" || aName == "section" ||
                    aName == "article" || aName == "header" || aName == "footer" || aName == "nav" || aName == "aside" || aName == "main" ||
                    aName == "figcaption" || aName == "details" || aName == "summary" || aName == "form" || aName == "thead" ||
                    aName == "tbody" || aName == "tfoot")
                    return element_kind::Line;
                return element_kind::Inline;
            }
            std::optional<std::size_t> find_open(std::string_view aName, std::initializer_list<std::string_view> aBoundaries = {}) const
            {
                for (std::size_t i = iStack.size(); i-- > 0;)
                {
                    if (iStack[i].name == aName)
                        return i;
                    if (std::find(aBoundaries.begin(), aBoundaries.end(), iStack[i].name) != aBoundaries.end())
                        return {};
                }
                return {};
            }
            void close_to(std::size_t aIndex)
            {
                while (iStack.size() > aIndex)
                {
                    auto const closed = std::move(iStack.back());
                    iStack.pop_back();
                    if (closed.kind != element_kind::Inline && closed.kind != element_kind::Cell && closed.kind != element_kind::Ignored)
                        close_block(closed);
                }
            }
            void implicit_close(std::string_view aName, element_kind aKind)
            {
                if (aKind != element_kind::Inline && aKind != element_kind::Ignored)
                    if (auto const p = find_open("p", { "li", "td", "th", "blockquote", "dd", "ul", "ol" }))
                        close_to(*p);
                if (aName == "li")
                {
                    if (auto const li = find_open("li", { "ul", "ol", "menu", "dir" }))
                        close_to(*li);
                }
                else if (aName == "dt" || aName == "dd")
                {
                    if (auto const dt = find_open("dt", { "dl" }))
                        close_to(*dt);
                    if (auto const dd = find_open("dd", { "dl" }))
                        close_to(*dd);
                }
                else if (aName == "tr")
                {
                    if (auto const tr = find_open("tr", { "table" }))
                        close_to(*tr);
                }
                else if (aName == "td" || aName == "th")
                {
                    if (auto const td = find_open("td", { "tr", "table" }))
                        close_to(*td);
                    if (auto const th = find_open("th", { "tr", "table" }))
                        close_to(*th);
                }
            }
            void start_tag(html_tag const& aTag)
            {
                auto const& name = aTag.name;
                if (name == "br")
                {
                    iPendingSpace = false;
                    if (!suppressed())
                    {
                        if (iOutput.at_line_start())
                            iOutput.text(line_prefix(), prefix_format(current_format()));
                        iOutput.newline();
                    }
                    return;
                }
                if (name == "hr")
                {
                    implicit_close(name, element_kind::Block);
                    iPendingSpace = false;
                    iOutput.block_break(css_length{ 0.0, 0.5 });
                    emit_object(inline_object{ inline_object::kind_e::Rule });
                    iPendingSpace = false;
                    iOutput.end_block(css_length{ 0.0, 0.5 }, {});
                    return;
                }
                if (name == "img")
                {
                    if (aTag.attribute("aria-hidden") == "true")
                        return; // e.g. Wikipedia's rendered formula; its MathML annotation provides the source
                    auto const src = aTag.attribute("src");
                    auto const alt = std::string{ trim(aTag.attribute("alt").value_or("")) };
                    if (src && is_loadable_image(*src))
                    {
                        auto const dimension = [&](char const* aAttribute) -> std::optional<double>
                        {
                            auto const value = aTag.attribute(aAttribute);
                            if (!value || value->find('%') != std::string::npos)
                                return {};
                            std::size_t length = 0;
                            auto const number = parse_number(trim(*value), length);
                            if (number && *number > 0.0)
                                return number;
                            return {};
                        };
                        emit_object(inline_object{ inline_object::kind_e::Image, std::string{ trim(*src) }, false, alt, dimension("width"), dimension("height") });
                    }
                    else if (!alt.empty())
                        emit(alt, current_format());
                    return;
                }
                if (name == "annotation" && !aTag.closing)
                {
                    // the TeX source in MathML (KaTeX, Wikipedia)
                    auto const encoding = to_lower(aTag.attribute("encoding").value_or(""));
                    if (encoding.find("tex") != std::string::npos)
                    {
                        iCapture.emplace();
                        return;
                    }
                }
                if (name == "math")
                    iMathDisplay = (to_lower(aTag.attribute("display").value_or("")) == "block") ||
                        (to_lower(aTag.attribute("mode").value_or("")) == "display");
                if (is_void_element(name))
                    return;
                auto kind = kind_of(name);
                if (aTag.attribute("aria-hidden") == "true")
                    kind = element_kind::Ignored; // a visual duplicate, e.g. KaTeX's rendered HTML beside its MathML
                if (kind == element_kind::Block || kind == element_kind::Line || kind == element_kind::Quote || kind == element_kind::Indented)
                {
                    if (find_open("td", { "table" }) || find_open("th", { "table" }))
                        kind = element_kind::Inline; // keep a table row on one line
                    else if (kind == element_kind::Block && find_open("li", { "ul", "ol" }))
                        kind = element_kind::Line;
                }
                implicit_close(name, kind);
                element e{ name, kind, current_format(), preserve_whitespace() };
                if (kind != element_kind::Inline && kind != element_kind::Cell)
                    e.preserveWhitespace = false;
                apply_tag_format(aTag, e.format, e.preserveWhitespace);
                if (kind == element_kind::Block || kind == element_kind::Quote)
                    e.marginTop = e.marginBottom = css_length{ 0.0, block_margin(name, e.format) };
                else if (kind == element_kind::List && list_depth() == iBaseListDepth)
                    e.marginTop = e.marginBottom = css_length{ 0.0, 1.0 }; // nested lists have none
                if (kind != element_kind::Inline && kind != element_kind::Ignored && kind != element_kind::Row && kind != element_kind::Cell)
                    if (auto const style = aTag.attribute("style"))
                    {
                        auto const box = apply_css_box(e.format, *style);
                        e.marginTop = box.marginTop.value_or(e.marginTop);
                        e.marginBottom = box.marginBottom.value_or(e.marginBottom);
                        e.paddingTop = box.paddingTop.value_or(e.paddingTop);
                        e.paddingBottom = box.paddingBottom.value_or(e.paddingBottom);
                    }
                switch (kind)
                {
                case element_kind::List:
                    e.ordered = (name == "ol");
                    e.listType = aTag.attribute("type").value_or(e.ordered ? "1" : "");
                    e.listDepth = list_depth();
                    if (auto const start = aTag.attribute("start"))
                    {
                        long value = 1;
                        if (std::from_chars(start->data(), start->data() + start->size(), value).ec == std::errc{})
                            e.counter = value - 1;
                    }
                    open_block(e);
                    break;
                case element_kind::ListItem:
                    {
                        element* list = nullptr;
                        for (auto i = iStack.rbegin(); i != iStack.rend(); ++i)
                            if (i->kind == element_kind::List)
                            {
                                list = &*i;
                                break;
                            }
                        if (auto const value = aTag.attribute("value"); list && value)
                        {
                            long v = 0;
                            if (std::from_chars(value->data(), value->data() + value->size(), v).ec == std::errc{})
                                list->counter = v - 1;
                        }
                        e.marker = list ? list_marker(*list) : std::string{ Bullets[0] };
                        open_block(e);
                    }
                    break;
                case element_kind::Row:
                    open_block(e);
                    break;
                case element_kind::Cell:
                    if (auto const row = find_open("tr", { "table" }))
                    {
                        if (iStack[*row].cells++ > 0)
                        {
                            iPendingSpace = false;
                            if (!suppressed())
                            {
                                if (iOutput.at_line_start())
                                    iOutput.text(line_prefix(), prefix_format(e.format));
                                iOutput.text("\t", span_format{});
                            }
                        }
                    }
                    break;
                case element_kind::Inline:
                case element_kind::Ignored:
                    break;
                default:
                    open_block(e);
                    break;
                }
                if (e.preserveWhitespace && kind != element_kind::Inline)
                    iSkipLeadingNewline = true;
                iStack.push_back(std::move(e));
                if (aTag.selfClosing)
                    end_tag(name);
            }
            void end_tag(std::string_view aName)
            {
                if (aName == "annotation" && iCapture)
                {
                    auto source = std::string{ trim(*iCapture) };
                    iCapture.reset();
                    if (!source.empty() && !iPendingMath)
                        iPendingMath = inline_object{ inline_object::kind_e::Math, std::move(source), iMathDisplay };
                    return;
                }
                if (aName == "math")
                {
                    if (auto const open = find_open("math"))
                        close_to(*open);
                    if (iPendingMath)
                    {
                        auto math = std::move(*iPendingMath);
                        iPendingMath.reset();
                        emit_object(math);
                    }
                    return;
                }
                if (aName == "br")
                    return start_tag(html_tag{ "br" });
                if (auto const open = find_open(aName))
                    close_to(*open);
                else if (aName == "p")
                {
                    // </p> without <p> is an empty paragraph, whose margins collapse into those around it
                    iPendingSpace = false;
                    iOutput.block_break(css_length{ 0.0, 1.0 });
                }
            }
            std::string list_marker(element& aList)
            {
                auto const number = ++aList.counter;
                if (!aList.ordered)
                {
                    auto const type = to_lower(aList.listType);
                    if (type == "circle")
                        return std::string{ Bullets[1] };
                    if (type == "square")
                        return std::string{ Bullets[2] };
                    if (type == "disc")
                        return std::string{ Bullets[0] };
                    return std::string{ Bullets[aList.listDepth % Bullets.size()] };
                }
                std::string result;
                if (aList.listType == "a" || aList.listType == "A")
                {
                    for (long n = number; n > 0; n = (n - 1) / 26)
                        result.insert(result.begin(), static_cast<char>((aList.listType == "a" ? 'a' : 'A') + (n - 1) % 26));
                }
                else if (aList.listType == "i" || aList.listType == "I")
                {
                    static constexpr std::array<std::pair<int, std::string_view>, 13> sNumerals = { {
                        { 1000, "m" }, { 900, "cm" }, { 500, "d" }, { 400, "cd" }, { 100, "c" }, { 90, "xc" },
                        { 50, "l" }, { 40, "xl" }, { 10, "x" }, { 9, "ix" }, { 5, "v" }, { 4, "iv" }, { 1, "i" } } };
                    long n = std::clamp<long>(number, 1, 3999);
                    for (auto const& [value, numeral] : sNumerals)
                        for (; n >= value; n -= value)
                            result += numeral;
                    if (aList.listType == "I")
                        for (auto& ch : result)
                            ch = static_cast<char>(ch - 'a' + 'A');
                }
                else
                    result = std::to_string(number);
                return result + ".";
            }
        private:
            builder& iOutput;
            std::string iFirstPrefix;
            std::string iRestPrefix;
            int iBaseListDepth;
            bool iFirstUsed = false;
            std::vector<element> iStack;
            bool iPendingSpace = false;
            span_format iPendingSpaceFormat;
            bool iSkipLeadingNewline = false;
            std::optional<std::string> iCapture;
            std::optional<inline_object> iPendingMath;
            bool iMathDisplay = false;
        };

        // ---------------------------------------------------------------------------------
        // Markdown: inlines
        // ---------------------------------------------------------------------------------

        std::string normalize_label(std::string_view aLabel)
        {
            std::string result;
            bool space = false;
            for (auto ch : trim(aLabel))
            {
                if (is_space(ch))
                    space = true;
                else
                {
                    if (space)
                        result += ' ';
                    space = false;
                    result += to_lower(ch);
                }
            }
            return result;
        }

        using link_definitions = std::map<std::string, std::string>;
        using modifier = std::function<void(span_format&)>;

        struct inline_node
        {
            enum kind_e : std::uint8_t
            {
                Text,
                Delimiter,
                Push,
                Pop,
                HardBreak,
                Object
            } kind;
            std::string text;
            char delimiter = 0;
            int count = 0;
            int originalCount = 0;
            bool canOpen = false;
            bool canClose = false;
            std::size_t modifierId = 0;
            std::optional<inline_object> object;
        };

        using inline_nodes = std::vector<inline_node>;

        class inline_parser
        {
        public:
            inline_parser(link_definitions const& aDefinitions, std::vector<modifier>& aModifiers) :
                iDefinitions{ aDefinitions }, iModifiers{ aModifiers }
            {
            }
        public:
            inline_nodes parse(std::string_view aText, bool aInLink = false)
            {
                inline_nodes result;
                std::string pending;
                std::vector<std::pair<std::string, std::size_t>> openHtml;
                auto flush = [&]()
                {
                    if (!pending.empty())
                    {
                        result.push_back(inline_node{ inline_node::Text, std::move(pending) });
                        pending.clear();
                    }
                };
                auto push = [&](modifier aModifier)
                {
                    flush();
                    iModifiers.push_back(std::move(aModifier));
                    result.push_back(inline_node{ inline_node::Push, {}, 0, 0, 0, false, false, iModifiers.size() - 1 });
                    return iModifiers.size() - 1;
                };
                auto pop = [&](std::size_t aId)
                {
                    flush();
                    result.push_back(inline_node{ inline_node::Pop, {}, 0, 0, 0, false, false, aId });
                };
                auto link = [&](std::string const& aUri, inline_nodes aContent)
                {
                    auto const id = push([aUri](span_format& f) { f.link = aUri; });
                    result.insert(result.end(), std::make_move_iterator(aContent.begin()), std::make_move_iterator(aContent.end()));
                    pop(id);
                };

                std::size_t i = 0;
                while (i < aText.size())
                {
                    char const ch = aText[i];
                    switch (ch)
                    {
                    case '\\':
                        if (i + 1 < aText.size() && aText[i + 1] == '\n')
                        {
                            flush();
                            result.push_back(inline_node{ inline_node::HardBreak });
                            i += 2;
                            while (i < aText.size() && (aText[i] == ' ' || aText[i] == '\t'))
                                ++i;
                            continue;
                        }
                        if (i + 1 < aText.size() && is_ascii_punct(aText[i + 1]))
                        {
                            pending += aText[i + 1];
                            i += 2;
                            continue;
                        }
                        pending += ch;
                        ++i;
                        continue;
                    case '`':
                        {
                            std::size_t run = 0;
                            while (i + run < aText.size() && aText[i + run] == '`')
                                ++run;
                            std::size_t search = i + run;
                            std::optional<std::size_t> close;
                            while (search < aText.size())
                            {
                                auto const next = aText.find('`', search);
                                if (next == std::string_view::npos)
                                    break;
                                std::size_t closeRun = 0;
                                while (next + closeRun < aText.size() && aText[next + closeRun] == '`')
                                    ++closeRun;
                                if (closeRun == run)
                                {
                                    close = next;
                                    break;
                                }
                                search = next + closeRun;
                            }
                            if (!close)
                            {
                                pending.append(run, '`');
                                i += run;
                                continue;
                            }
                            std::string code{ aText.substr(i + run, *close - (i + run)) };
                            std::replace(code.begin(), code.end(), '\n', ' ');
                            if (code.size() >= 2 && code.front() == ' ' && code.back() == ' ' && code.find_first_not_of(' ') != std::string::npos)
                                code = code.substr(1, code.size() - 2);
                            auto const id = push([](span_format& f) { f.monospace = true; f.codeShade = true; });
                            result.push_back(inline_node{ inline_node::Text, std::move(code) });
                            pop(id);
                            i = *close + run;
                            continue;
                        }
                    case '*':
                    case '_':
                    case '~':
                        {
                            std::size_t run = 0;
                            while (i + run < aText.size() && aText[i + run] == ch)
                                ++run;
                            if (ch == '~' && run > 2)
                            {
                                pending.append(run, ch);
                                i += run;
                                continue;
                            }
                            char const before = (i > 0 ? aText[i - 1] : '\n');
                            char const after = (i + run < aText.size() ? aText[i + run] : '\n');
                            bool const beforeSpace = is_space(before);
                            bool const afterSpace = is_space(after);
                            bool const beforePunct = is_ascii_punct(before);
                            bool const afterPunct = is_ascii_punct(after);
                            bool const leftFlanking = !afterSpace && (!afterPunct || beforeSpace || beforePunct);
                            bool const rightFlanking = !beforeSpace && (!beforePunct || afterSpace || afterPunct);
                            inline_node node{ inline_node::Delimiter, {}, ch, static_cast<int>(run), static_cast<int>(run) };
                            if (ch == '_')
                            {
                                node.canOpen = leftFlanking && (!rightFlanking || beforePunct);
                                node.canClose = rightFlanking && (!leftFlanking || afterPunct);
                            }
                            else
                            {
                                node.canOpen = leftFlanking;
                                node.canClose = rightFlanking;
                            }
                            if (run > MaxDelimiterRun)
                                node.canOpen = node.canClose = false; // bound the cost of pathological input
                            flush();
                            result.push_back(std::move(node));
                            i += run;
                            continue;
                        }
                    case '!':
                        if (i + 1 < aText.size() && aText[i + 1] == '[')
                            if (auto const image = match_link(aText, i + 1))
                            {
                                // images text_edit can load become objects; others show their alt text
                                auto alt = plain_text(image->label);
                                if (is_loadable_image(image->destination))
                                {
                                    flush();
                                    inline_node node{ inline_node::Object };
                                    node.object = inline_object{ inline_object::kind_e::Image, image->destination, false, std::move(alt) };
                                    result.push_back(std::move(node));
                                }
                                else
                                    pending += alt;
                                i = image->end;
                                continue;
                            }
                        pending += ch;
                        ++i;
                        continue;
                    case '$':
                        if (auto const math = match_math(aText, i))
                        {
                            flush();
                            inline_node node{ inline_node::Object };
                            node.object = inline_object{ inline_object::kind_e::Math, std::string{ trim(math->source) }, math->display };
                            result.push_back(std::move(node));
                            i = math->end;
                            continue;
                        }
                        // an opening $$ that doesn't match is literal; don't retry its second $
                        if (i + 1 < aText.size() && aText[i + 1] == '$')
                        {
                            pending += "$$";
                            i += 2;
                            continue;
                        }
                        pending += ch;
                        ++i;
                        continue;
                    case '[':
                        if (!aInLink)
                            if (auto const match = match_link(aText, i))
                            {
                                auto content = parse(match->label, true);
                                resolve_emphasis(content);
                                link(match->destination, std::move(content));
                                i = match->end;
                                continue;
                            }
                        pending += ch;
                        ++i;
                        continue;
                    case '<':
                        {
                            auto const close = aText.find('>', i + 1);
                            if (!aInLink && close != std::string_view::npos)
                            {
                                auto const inner = aText.substr(i + 1, close - i - 1);
                                if (auto const uri = match_autolink(inner))
                                {
                                    link(*uri, { inline_node{ inline_node::Text, std::string{ inner } } });
                                    i = close + 1;
                                    continue;
                                }
                            }
                            if (aText.substr(i).starts_with("<!--"))
                            {
                                auto const end = aText.find("-->", i + 4);
                                if (end != std::string_view::npos)
                                {
                                    i = end + 3;
                                    continue;
                                }
                            }
                            if (auto const tag = parse_tag(aText, i))
                            {
                                if (tag->name == "br")
                                {
                                    flush();
                                    result.push_back(inline_node{ inline_node::HardBreak });
                                }
                                else if (!tag->closing)
                                {
                                    if (!is_void_element(tag->name) && !tag->selfClosing)
                                    {
                                        auto const id = push([t = *tag](span_format& f) { bool preserve = false; apply_tag_format(t, f, preserve); });
                                        openHtml.emplace_back(tag->name, id);
                                    }
                                    else if (tag->name == "img")
                                        if (auto const alt = tag->attribute("alt"))
                                            pending += *alt;
                                }
                                else
                                {
                                    for (auto open = openHtml.rbegin(); open != openHtml.rend(); ++open)
                                        if (open->first == tag->name)
                                        {
                                            pop(open->second);
                                            openHtml.erase(std::next(open).base());
                                            break;
                                        }
                                }
                                i = tag->end;
                                continue;
                            }
                            pending += ch;
                            ++i;
                            continue;
                        }
                    case '&':
                        {
                            std::size_t length = 0;
                            if (auto const decoded = decode_entity(aText.substr(i), length))
                            {
                                pending += *decoded;
                                i += length;
                                continue;
                            }
                            pending += ch;
                            ++i;
                            continue;
                        }
                    case '\n':
                        {
                            std::size_t spaces = 0;
                            while (!pending.empty() && pending.back() == ' ')
                            {
                                pending.pop_back();
                                ++spaces;
                            }
                            if (spaces >= 2)
                            {
                                flush();
                                result.push_back(inline_node{ inline_node::HardBreak });
                            }
                            else
                                pending += ' ';
                            ++i;
                            while (i < aText.size() && (aText[i] == ' ' || aText[i] == '\t'))
                                ++i;
                            continue;
                        }
                    default:
                        if (!aInLink && (ch == 'w' || ch == 'h' || ch == 'f' || ch == 'W' || ch == 'H' || ch == 'F'))
                        {
                            char const before = (i > 0 ? aText[i - 1] : ' ');
                            if (is_space(before) || before == '*' || before == '_' || before == '~' || before == '(')
                                if (auto const autolink = match_extended_autolink(aText.substr(i)))
                                {
                                    link(autolink->first, { inline_node{ inline_node::Text, std::string{ aText.substr(i, autolink->second) } } });
                                    i += autolink->second;
                                    continue;
                                }
                        }
                        pending += ch;
                        ++i;
                        continue;
                    }
                }
                flush();
                for (auto open = openHtml.rbegin(); open != openHtml.rend(); ++open)
                    pop(open->second);
                return result;
            }
            void resolve_emphasis(inline_nodes& aNodes)
            {
                for (std::size_t closerIndex = 0; closerIndex < aNodes.size(); ++closerIndex)
                {
                    if (aNodes[closerIndex].kind != inline_node::Delimiter || !aNodes[closerIndex].canClose)
                        continue;
                    while (aNodes[closerIndex].count > 0)
                    {
                        auto& closer = aNodes[closerIndex];
                        std::optional<std::size_t> openerIndex;
                        for (std::size_t j = closerIndex; j-- > 0;)
                        {
                            auto const& opener = aNodes[j];
                            if (opener.kind != inline_node::Delimiter || !opener.canOpen || opener.count == 0 || opener.delimiter != closer.delimiter)
                                continue;
                            if (closer.delimiter == '~')
                            {
                                if (opener.count != closer.count)
                                    continue;
                            }
                            else if ((opener.canClose || closer.canOpen) &&
                                (opener.originalCount + closer.originalCount) % 3 == 0 &&
                                !(opener.originalCount % 3 == 0 && closer.originalCount % 3 == 0))
                                continue;
                            openerIndex = j;
                            break;
                        }
                        if (!openerIndex)
                            break;
                        auto& opener = aNodes[*openerIndex];
                        int const use = (closer.delimiter == '~') ? closer.count : (opener.count >= 2 && closer.count >= 2 ? 2 : 1);
                        opener.count -= use;
                        closer.count -= use;
                        for (auto k = *openerIndex + 1; k < closerIndex; ++k)
                            if (aNodes[k].kind == inline_node::Delimiter)
                                aNodes[k].canOpen = aNodes[k].canClose = false;
                        if (closer.delimiter == '~')
                            iModifiers.push_back([](span_format& f) { f.strike = true; });
                        else if (use == 2)
                            iModifiers.push_back([](span_format& f) { f.bold = true; });
                        else
                            iModifiers.push_back([](span_format& f) { f.italic = true; });
                        auto const id = iModifiers.size() - 1;
                        aNodes.insert(aNodes.begin() + closerIndex, inline_node{ inline_node::Pop, {}, 0, 0, 0, false, false, id });
                        aNodes.insert(aNodes.begin() + *openerIndex + 1, inline_node{ inline_node::Push, {}, 0, 0, 0, false, false, id });
                        closerIndex += 2;
                    }
                }
                // unmatched delimiters are literal text
                for (auto& node : aNodes)
                    if (node.kind == inline_node::Delimiter)
                    {
                        node.kind = inline_node::Text;
                        node.text.assign(static_cast<std::size_t>(node.count), node.delimiter);
                    }
            }
        private:
            struct math_match
            {
                std::string_view source;
                bool display;
                std::size_t end;
            };
            // GitHub's math syntax: $$...$$ (display), $`...`$ and $...$ (inline). An inline $ must be followed by a
            // non-space, and its closing $ preceded by a non-space and not followed by a digit (so "$5 and $10" is text).
            static std::optional<math_match> match_math(std::string_view aText, std::size_t aOpen)
            {
                if (aOpen + 1 < aText.size() && aText[aOpen + 1] == '$')
                {
                    auto const start = aOpen + 2;
                    for (auto j = start; j + 1 < aText.size(); ++j)
                    {
                        if (aText[j] == '\\')
                        {
                            ++j;
                            continue;
                        }
                        if (aText[j] == '$' && aText[j + 1] == '$')
                        {
                            if (trim(aText.substr(start, j - start)).empty())
                                return {};
                            return math_match{ aText.substr(start, j - start), true, j + 2 };
                        }
                    }
                    return {};
                }
                if (aOpen + 1 < aText.size() && aText[aOpen + 1] == '`')
                {
                    auto const close = aText.find("`$", aOpen + 2);
                    if (close == std::string_view::npos || close == aOpen + 2)
                        return {};
                    return math_match{ aText.substr(aOpen + 2, close - (aOpen + 2)), false, close + 2 };
                }
                auto const start = aOpen + 1;
                if (start >= aText.size() || is_space(aText[start]))
                    return {};
                for (auto j = start; j < aText.size(); ++j)
                {
                    if (aText[j] == '\\')
                    {
                        ++j;
                        continue;
                    }
                    if ((aText[j] == '\n' && j + 1 < aText.size() && aText[j + 1] == '\n') || aText[j] == '`')
                        return {}; // code spans take precedence
                    if (aText[j] == '$')
                    {
                        if (j == start || is_space(aText[j - 1]) || (j + 1 < aText.size() && is_ascii_digit(aText[j + 1])))
                            return {};
                        return math_match{ aText.substr(start, j - start), false, j + 1 };
                    }
                }
                return {};
            }
            struct link_match
            {
                std::string_view label;
                std::string destination;
                std::size_t end;
            };
            std::optional<link_match> match_link(std::string_view aText, std::size_t aOpen) const
            {
                int depth = 1;
                std::size_t j = aOpen + 1;
                auto const limit = std::min(aText.size(), aOpen + MaxLinkTextLength);
                for (; j < limit; ++j)
                {
                    if (aText[j] == '\\')
                        ++j;
                    else if (aText[j] == '`')
                    {
                        std::size_t run = 0;
                        while (j + run < aText.size() && aText[j + run] == '`')
                            ++run;
                        auto const close = aText.find(std::string(run, '`'), j + run);
                        j = (close == std::string_view::npos ? j + run : close + run) - 1;
                    }
                    else if (aText[j] == '[')
                        ++depth;
                    else if (aText[j] == ']' && --depth == 0)
                        break;
                }
                if (j >= limit)
                    return {};
                auto const label = aText.substr(aOpen + 1, j - aOpen - 1);
                auto k = j + 1;
                if (k < aText.size() && aText[k] == '(')
                    if (auto const inlineLink = match_inline_destination(aText, k))
                        return link_match{ label, inlineLink->first, inlineLink->second };
                if (k < aText.size() && aText[k] == '[')
                {
                    auto const close = aText.find(']', k + 1);
                    if (close != std::string_view::npos)
                    {
                        auto reference = aText.substr(k + 1, close - k - 1);
                        if (trim(reference).empty())
                            reference = label;
                        if (auto const existing = iDefinitions.find(normalize_label(reference)); existing != iDefinitions.end())
                            return link_match{ label, existing->second, close + 1 };
                        return {};
                    }
                }
                if (auto const existing = iDefinitions.find(normalize_label(label)); existing != iDefinitions.end())
                    return link_match{ label, existing->second, j + 1 };
                return {};
            }
            static std::optional<std::pair<std::string, std::size_t>> match_inline_destination(std::string_view aText, std::size_t aOpen)
            {
                auto p = aOpen + 1;
                auto skipSpace = [&]() { while (p < aText.size() && is_space(aText[p])) ++p; };
                skipSpace();
                std::string destination;
                if (p < aText.size() && aText[p] == '<')
                {
                    auto const close = aText.find('>', p + 1);
                    if (close == std::string_view::npos || aText.substr(p + 1, close - p - 1).find('\n') != std::string_view::npos)
                        return {};
                    destination = aText.substr(p + 1, close - p - 1);
                    p = close + 1;
                }
                else
                {
                    int parens = 0;
                    auto const start = p;
                    for (; p < aText.size() && !is_space(aText[p]); ++p)
                    {
                        if (aText[p] == '\\' && p + 1 < aText.size())
                            ++p;
                        else if (aText[p] == '(')
                            ++parens;
                        else if (aText[p] == ')' && parens-- == 0)
                            break;
                    }
                    destination = aText.substr(start, p - start);
                }
                skipSpace();
                if (p < aText.size() && (aText[p] == '"' || aText[p] == '\'' || aText[p] == '('))
                {
                    char const close = (aText[p] == '(' ? ')' : aText[p]);
                    for (++p; p < aText.size() && aText[p] != close; ++p)
                        if (aText[p] == '\\')
                            ++p;
                    if (p >= aText.size())
                        return {};
                    ++p;
                    skipSpace();
                }
                if (p >= aText.size() || aText[p] != ')')
                    return {};
                return std::make_pair(unescape_markdown(destination), p + 1);
            }
            static std::optional<std::string> match_autolink(std::string_view aInner)
            {
                if (aInner.empty() || aInner.find_first_of(" \t\n<") != std::string_view::npos)
                    return {};
                auto const colon = aInner.find(':');
                if (colon != std::string_view::npos && colon >= 2 && colon <= 32 && is_ascii_alpha(aInner[0]) &&
                    std::all_of(aInner.begin(), aInner.begin() + colon, [](char c) { return is_ascii_alnum(c) || c == '+' || c == '.' || c == '-'; }))
                    return std::string{ aInner };
                auto const at = aInner.find('@');
                if (at != std::string_view::npos && at > 0 && aInner.find('.', at) != std::string_view::npos && aInner.find('@', at + 1) == std::string_view::npos)
                    return "mailto:" + std::string{ aInner };
                return {};
            }
            // GFM extended autolinks: www., http://, https://, ftp://
            static std::optional<std::pair<std::string, std::size_t>> match_extended_autolink(std::string_view aText)
            {
                std::string_view scheme;
                for (auto const s : { std::string_view{ "https://" }, std::string_view{ "http://" }, std::string_view{ "ftp://" }, std::string_view{ "www." } })
                    if (istarts_with(aText, s))
                    {
                        scheme = s;
                        break;
                    }
                if (scheme.empty())
                    return {};
                std::size_t end = 0;
                while (end < aText.size() && !is_space(aText[end]) && aText[end] != '<')
                    ++end;
                // trailing punctuation is not part of the link
                for (bool trimmed = true; trimmed && end > scheme.size();)
                {
                    trimmed = false;
                    char const last = aText[end - 1];
                    if (std::string_view{ "?!.,:*_~'\"" }.find(last) != std::string_view::npos)
                    {
                        --end;
                        trimmed = true;
                    }
                    else if (last == ')')
                    {
                        auto const candidate = aText.substr(0, end);
                        if (std::count(candidate.begin(), candidate.end(), ')') > std::count(candidate.begin(), candidate.end(), '('))
                        {
                            --end;
                            trimmed = true;
                        }
                    }
                    else if (last == ';')
                    {
                        auto amp = end - 1;
                        while (amp > 0 && is_ascii_alnum(aText[amp - 1]))
                            --amp;
                        if (amp > 0 && aText[amp - 1] == '&')
                        {
                            end = amp - 1;
                            trimmed = true;
                        }
                    }
                }
                auto const host = aText.substr(scheme.size(), end - scheme.size());
                auto const hostEnd = host.find_first_of("/?#");
                auto const domain = host.substr(0, hostEnd);
                if (domain.empty() ||
                    !std::all_of(domain.begin(), domain.end(), [](char c) { return is_ascii_alnum(c) || c == '.' || c == '-' || c == '_' || c == ':' || static_cast<unsigned char>(c) >= 0x80; }))
                    return {};
                if (scheme != "www." && domain.find('.') == std::string_view::npos && domain.find(':') == std::string_view::npos && !istarts_with(domain, "localhost"))
                    return {};
                std::string uri{ aText.substr(0, end) };
                if (scheme == "www.")
                    uri = "http://" + uri;
                return std::make_pair(uri, end);
            }
            std::string plain_text(std::string_view aLabel)
            {
                std::string result;
                auto nodes = parse(aLabel, true);
                resolve_emphasis(nodes);
                for (auto const& node : nodes)
                    if (node.kind == inline_node::Text)
                        result += node.text;
                return result;
            }
        private:
            link_definitions const& iDefinitions;
            std::vector<modifier>& iModifiers;
        };

        // ---------------------------------------------------------------------------------
        // Markdown: blocks
        // ---------------------------------------------------------------------------------

        struct block_context
        {
            builder& output;
            std::string firstPrefix;
            std::string restPrefix;
            bool tight = false;
            int listDepth = 0;
            bool firstUsed = false;

            std::string prefix()
            {
                if (!firstUsed)
                {
                    firstUsed = true;
                    return firstPrefix;
                }
                return restPrefix;
            }
            void separate(double aMargin = 1.0)
            {
                if (tight)
                    output.line_break();
                else
                    output.block_break(css_length{ 0.0, aMargin });
            }
        };

        std::size_t indentation(std::string_view aLine)
        {
            std::size_t result = 0;
            while (result < aLine.size() && aLine[result] == ' ')
                ++result;
            return result;
        }

        bool is_blank(std::string_view aLine)
        {
            return trim(aLine).empty();
        }

        std::string_view strip_indent(std::string_view aLine, std::size_t aColumns)
        {
            auto const strip = std::min(aColumns, indentation(aLine));
            return aLine.substr(strip);
        }

        struct fence_info
        {
            char ch;
            std::size_t length;
            std::size_t indent;
        };

        std::optional<fence_info> match_fence(std::string_view aLine)
        {
            auto const indent = indentation(aLine);
            if (indent >= 4 || indent >= aLine.size())
                return {};
            char const ch = aLine[indent];
            if (ch != '`' && ch != '~')
                return {};
            std::size_t length = 0;
            while (indent + length < aLine.size() && aLine[indent + length] == ch)
                ++length;
            if (length < 3)
                return {};
            if (ch == '`' && aLine.substr(indent + length).find('`') != std::string_view::npos)
                return {};
            return fence_info{ ch, length, indent };
        }

        bool is_closing_fence(std::string_view aLine, fence_info const& aFence)
        {
            auto const indent = indentation(aLine);
            if (indent >= 4)
                return false;
            std::size_t length = 0;
            while (indent + length < aLine.size() && aLine[indent + length] == aFence.ch)
                ++length;
            return length >= aFence.length && is_blank(aLine.substr(indent + length));
        }

        std::optional<std::pair<int, std::string_view>> match_atx_heading(std::string_view aLine)
        {
            auto const indent = indentation(aLine);
            if (indent >= 4)
                return {};
            auto text = aLine.substr(indent);
            int level = 0;
            while (level < static_cast<int>(text.size()) && text[level] == '#')
                ++level;
            if (level == 0 || level > 6 || (level < static_cast<int>(text.size()) && text[level] != ' ' && text[level] != '\t'))
                return {};
            text = trim(text.substr(level));
            // optional closing sequence
            auto closing = text.size();
            while (closing > 0 && text[closing - 1] == '#')
                --closing;
            if (closing == 0)
                text = {};
            else if (closing < text.size() && (text[closing - 1] == ' ' || text[closing - 1] == '\t'))
                text = trim(text.substr(0, closing));
            return std::make_pair(level, text);
        }

        bool is_thematic_break(std::string_view aLine)
        {
            if (indentation(aLine) >= 4)
                return false;
            auto const text = trim(aLine);
            if (text.empty() || (text[0] != '*' && text[0] != '-' && text[0] != '_'))
                return false;
            std::size_t count = 0;
            for (auto ch : text)
            {
                if (ch == text[0])
                    ++count;
                else if (ch != ' ' && ch != '\t')
                    return false;
            }
            return count >= 3;
        }

        std::optional<int> match_setext_underline(std::string_view aLine)
        {
            if (indentation(aLine) >= 4)
                return {};
            auto const text = trim(aLine);
            if (text.empty() || (text[0] != '=' && text[0] != '-'))
                return {};
            if (text.find_first_not_of(text[0]) != std::string_view::npos)
                return {};
            return text[0] == '=' ? 1 : 2;
        }

        struct list_marker
        {
            bool ordered;
            char ch; // bullet character or ordered delimiter
            long start;
            std::size_t indent;
            std::size_t contentOffset; // columns from line start to item content
            bool empty;
        };

        std::optional<list_marker> match_list_marker(std::string_view aLine)
        {
            auto const indent = indentation(aLine);
            if (indent >= 4 || indent >= aLine.size())
                return {};
            list_marker result{ false, 0, 1, indent, 0, false };
            std::size_t p = indent;
            if (aLine[p] == '-' || aLine[p] == '+' || aLine[p] == '*')
                result.ch = aLine[p++];
            else
            {
                auto const digitsStart = p;
                while (p < aLine.size() && is_ascii_digit(aLine[p]) && p - digitsStart < 9)
                    ++p;
                if (p == digitsStart || p >= aLine.size() || (aLine[p] != '.' && aLine[p] != ')'))
                    return {};
                std::from_chars(aLine.data() + digitsStart, aLine.data() + p, result.start);
                result.ordered = true;
                result.ch = aLine[p++];
            }
            if (p < aLine.size() && aLine[p] != ' ')
                return {};
            auto spaces = indentation(aLine.substr(p));
            if (p + spaces >= aLine.size())
            {
                result.empty = true;
                spaces = 1;
            }
            else if (spaces > 4)
                spaces = 1;
            result.contentOffset = p + spaces;
            return result;
        }

        bool is_table_delimiter_row(std::string_view aLine, std::size_t& aColumns)
        {
            auto text = trim(aLine);
            if (text.find('-') == std::string_view::npos || indentation(aLine) >= 4)
                return false;
            bool const hasPipe = text.find('|') != std::string_view::npos;
            if (text.starts_with("|"))
                text.remove_prefix(1);
            if (text.ends_with("|"))
                text.remove_suffix(1);
            aColumns = 0;
            std::size_t start = 0;
            while (start <= text.size())
            {
                auto end = text.find('|', start);
                if (end == std::string_view::npos)
                    end = text.size();
                auto const cell = trim(text.substr(start, end - start));
                auto core = cell;
                if (core.starts_with(":"))
                    core.remove_prefix(1);
                if (core.ends_with(":"))
                    core.remove_suffix(1);
                if (core.empty() || core.find_first_not_of('-') != std::string_view::npos)
                    return false;
                ++aColumns;
                start = end + 1;
            }
            return hasPipe || aColumns > 1;
        }

        std::vector<std::string> split_table_row(std::string_view aLine)
        {
            auto text = trim(aLine);
            if (text.starts_with("|"))
                text.remove_prefix(1);
            if (text.ends_with("|") && !text.ends_with("\\|"))
                text.remove_suffix(1);
            std::vector<std::string> cells;
            std::string cell;
            for (std::size_t i = 0; i < text.size(); ++i)
            {
                if (text[i] == '\\' && i + 1 < text.size() && text[i + 1] == '|')
                {
                    cell += '|';
                    ++i;
                }
                else if (text[i] == '|')
                {
                    cells.emplace_back(trim(cell));
                    cell.clear();
                }
                else
                    cell += text[i];
            }
            cells.emplace_back(trim(cell));
            return cells;
        }

        bool is_html_block_start(std::string_view aLine)
        {
            if (indentation(aLine) >= 4)
                return false;
            auto const text = trim(aLine);
            if (!text.starts_with("<"))
                return false;
            if (text.starts_with("<!--") || text.starts_with("<?") || text.starts_with("<!"))
                return true;
            auto const tag = parse_tag(text, 0);
            std::string name;
            if (tag)
                name = tag->name;
            else
            {
                std::size_t i = text.starts_with("</") ? 2 : 1;
                while (i < text.size() && is_ascii_alnum(text[i]))
                    name += to_lower(text[i++]);
                if (i < text.size() && !is_space(text[i]) && text[i] != '>' && text[i] != '/')
                    return false;
            }
            static constexpr std::array<std::string_view, 42> sBlockTags = {
                "address", "article", "aside", "blockquote", "body", "caption", "center", "dd", "details", "dir", "div", "dl", "dt",
                "figcaption", "figure", "footer", "form", "h1", "h2", "h3", "h4", "h5", "h6", "head", "header", "hr", "html", "li",
                "main", "menu", "nav", "ol", "p", "pre", "section", "summary", "table", "tbody", "td", "th", "tr", "ul" };
            return std::find(sBlockTags.begin(), sBlockTags.end(), name) != sBlockTags.end() ||
                name == "script" || name == "style" || name == "thead" || name == "tfoot";
        }

        std::optional<std::pair<std::string, std::string>> match_link_definition(std::string_view aLine)
        {
            if (indentation(aLine) >= 4)
                return {};
            auto text = trim(aLine);
            if (!text.starts_with("[") || text.starts_with("[^"))
                return {};
            auto const close = text.find("]:");
            if (close == std::string_view::npos || close < 2)
                return {};
            auto const label = text.substr(1, close - 1);
            if (label.find_first_of("[]") != std::string_view::npos)
                return {};
            auto rest = trim(text.substr(close + 2));
            if (rest.empty())
                return {};
            std::string_view destination;
            if (rest.starts_with("<"))
            {
                auto const end = rest.find('>');
                if (end == std::string_view::npos)
                    return {};
                destination = rest.substr(1, end - 1);
                rest = trim(rest.substr(end + 1));
            }
            else
            {
                auto const end = std::min(rest.size(), rest.find_first_of(" \t"));
                destination = rest.substr(0, end);
                rest = trim(rest.substr(end));
            }
            if (!rest.empty())
            {
                char const open = rest.front();
                char const closeChar = (open == '(' ? ')' : open);
                if ((open != '"' && open != '\'' && open != '(') || rest.size() < 2 || rest.back() != closeChar)
                    return {};
            }
            return std::make_pair(normalize_label(label), unescape_markdown(destination));
        }

        class markdown_parser
        {
        public:
            explicit markdown_parser(builder& aOutput) :
                iOutput{ aOutput }, iInlines{ iDefinitions, iModifiers }
            {
            }
        public:
            void parse(std::string_view aMarkdown)
            {
                auto const lines = split_lines(aMarkdown);
                for (auto const& line : lines)
                    if (auto const definition = match_link_definition(line))
                        iDefinitions.emplace(definition->first, definition->second);
                block_context root{ iOutput };
                parse_blocks(lines, root);
            }
        private:
            static std::vector<std::string> split_lines(std::string_view aText)
            {
                std::vector<std::string> lines;
                std::string line;
                std::size_t column = 0;
                bool leading = true;
                auto finish = [&]()
                {
                    lines.push_back(std::move(line));
                    line.clear();
                    column = 0;
                    leading = true;
                };
                for (std::size_t i = 0; i < aText.size(); ++i)
                {
                    char const ch = aText[i];
                    if (ch == '\r')
                    {
                        if (i + 1 < aText.size() && aText[i + 1] == '\n')
                            ++i;
                        finish();
                    }
                    else if (ch == '\n')
                        finish();
                    else if (ch == '\t' && leading)
                    {
                        auto const spaces = 4 - (column % 4);
                        line.append(spaces, ' ');
                        column += spaces;
                    }
                    else
                    {
                        if (ch != ' ')
                            leading = false;
                        line += ch;
                        ++column;
                    }
                }
                if (!line.empty())
                    finish();
                return lines;
            }
            static bool interrupts_paragraph(std::string_view aLine)
            {
                if (match_fence(aLine) || match_atx_heading(aLine) || is_thematic_break(aLine) || is_html_block_start(aLine))
                    return true;
                auto const indent = indentation(aLine);
                if (indent < 4 && indent < aLine.size() && aLine[indent] == '>')
                    return true;
                if (auto const marker = match_list_marker(aLine))
                    return !marker->empty && (!marker->ordered || marker->start == 1);
                return false;
            }
            static bool starts_block(std::string_view aLine)
            {
                return is_blank(aLine) || interrupts_paragraph(aLine);
            }
            void parse_blocks(std::vector<std::string> const& aLines, block_context& aContext)
            {
                std::size_t i = 0;
                auto const n = aLines.size();
                while (i < n)
                {
                    std::string_view const line = aLines[i];
                    if (is_blank(line))
                    {
                        ++i;
                        continue;
                    }
                    auto const indent = indentation(line);
                    if (indent >= 4)
                    {
                        std::vector<std::string_view> code;
                        auto j = i;
                        while (j < n && (is_blank(aLines[j]) || indentation(aLines[j]) >= 4))
                            code.push_back(strip_indent(aLines[j++], 4));
                        while (!code.empty() && is_blank(code.back()))
                            code.pop_back();
                        emit_code(code, aContext);
                        i = j;
                        continue;
                    }
                    if (auto const fence = match_fence(line))
                    {
                        std::vector<std::string_view> code;
                        auto j = i + 1;
                        while (j < n && !is_closing_fence(aLines[j], *fence))
                            code.push_back(strip_indent(aLines[j++], fence->indent));
                        auto const info = trim(line.substr(fence->indent + fence->length));
                        if (to_lower(info.substr(0, info.find_first_of(" \t{"))) == "math")
                        {
                            // ```math: display math
                            std::string source;
                            for (auto const& codeLine : code)
                            {
                                source += codeLine;
                                source += '\n';
                            }
                            aContext.separate();
                            aContext.output.object(inline_object{ inline_object::kind_e::Math, std::string{ trim(source) }, true }, span_format{});
                        }
                        else
                            emit_code(code, aContext);
                        i = (j < n ? j + 1 : j);
                        continue;
                    }
                    if (auto const heading = match_atx_heading(line))
                    {
                        emit_heading(heading->first, heading->second, aContext);
                        ++i;
                        continue;
                    }
                    if (is_thematic_break(line))
                    {
                        aContext.separate(0.5);
                        aContext.output.text(aContext.prefix(), span_format{});
                        aContext.output.object(inline_object{ inline_object::kind_e::Rule }, span_format{});
                        aContext.separate(0.5);
                        ++i;
                        continue;
                    }
                    if (line[indent] == '>')
                    {
                        std::vector<std::string> quoted;
                        auto j = i;
                        bool lastWasParagraph = false;
                        while (j < n)
                        {
                            std::string_view const quoteLine = aLines[j];
                            auto const quoteIndent = indentation(quoteLine);
                            if (!is_blank(quoteLine) && quoteIndent < 4 && quoteLine[quoteIndent] == '>')
                            {
                                auto content = quoteLine.substr(quoteIndent + 1);
                                if (content.starts_with(" "))
                                    content.remove_prefix(1);
                                quoted.emplace_back(content);
                                lastWasParagraph = !is_blank(content) && !match_fence(content) && !is_thematic_break(content);
                            }
                            else if (lastWasParagraph && !starts_block(quoteLine))
                                quoted.emplace_back(trim(quoteLine)); // lazy continuation
                            else
                                break;
                            ++j;
                        }
                        aContext.separate();
                        auto const prefix = aContext.prefix();
                        block_context quote{ aContext.output, prefix + std::string{ QuoteMarker }, aContext.restPrefix + std::string{ QuoteMarker }, false, aContext.listDepth };
                        parse_blocks(quoted, quote);
                        aContext.separate();
                        i = j;
                        continue;
                    }
                    if (auto const marker = match_list_marker(line))
                    {
                        i = parse_list(aLines, i, *marker, aContext);
                        continue;
                    }
                    if (is_html_block_start(line))
                    {
                        std::string html;
                        auto j = i;
                        auto const raw = istarts_with(trim(line), "<pre") || istarts_with(trim(line), "<script") || istarts_with(trim(line), "<style");
                        while (j < n && (raw || !is_blank(aLines[j])))
                        {
                            html += aLines[j];
                            html += '\n';
                            if (raw && (ifind(aLines[j], "</pre>") != std::string::npos || ifind(aLines[j], "</script>") != std::string::npos ||
                                ifind(aLines[j], "</style>") != std::string::npos))
                            {
                                ++j;
                                break;
                            }
                            ++j;
                        }
                        aContext.separate();
                        auto const prefix = aContext.prefix();
                        html_parser htmlParser{ aContext.output, prefix, aContext.restPrefix, aContext.listDepth };
                        htmlParser.parse(html);
                        i = j;
                        continue;
                    }
                    std::size_t columns = 0;
                    if (i + 1 < n && line.find('|') != std::string_view::npos && is_table_delimiter_row(aLines[i + 1], columns) &&
                        split_table_row(line).size() == columns)
                    {
                        std::vector<std::vector<std::string>> rows;
                        rows.push_back(split_table_row(line));
                        auto j = i + 2;
                        while (j < n && !starts_block(aLines[j]))
                        {
                            auto row = split_table_row(aLines[j++]);
                            row.resize(columns);
                            rows.push_back(std::move(row));
                        }
                        emit_table(rows, aContext);
                        i = j;
                        continue;
                    }
                    if (match_link_definition(line))
                    {
                        ++i;
                        continue;
                    }
                    // paragraph (or setext heading)
                    std::string paragraph{ trim(line) };
                    auto j = i + 1;
                    std::optional<int> setextLevel;
                    while (j < n && !is_blank(aLines[j]))
                    {
                        if (auto const underline = match_setext_underline(aLines[j]))
                        {
                            setextLevel = underline;
                            ++j;
                            break;
                        }
                        if (interrupts_paragraph(aLines[j]))
                            break;
                        paragraph += '\n';
                        auto continuation = aLines[j++];
                        paragraph += std::string_view{ continuation }.substr(indentation(continuation));
                    }
                    while (!paragraph.empty() && (paragraph.back() == ' ' || paragraph.back() == '\t'))
                        paragraph.pop_back();
                    if (setextLevel)
                        emit_heading(*setextLevel, paragraph, aContext);
                    else
                    {
                        aContext.separate();
                        emit_inlines(paragraph, span_format{}, aContext);
                        aContext.separate();
                    }
                    i = j;
                }
            }
            std::size_t parse_list(std::vector<std::string> const& aLines, std::size_t aStart, list_marker const& aFirst, block_context& aContext)
            {
                struct item
                {
                    std::vector<std::string> lines;
                    list_marker marker;
                };
                std::vector<item> items;
                bool loose = false;
                auto const n = aLines.size();
                auto j = aStart;
                while (j < n)
                {
                    auto const marker = match_list_marker(aLines[j]);
                    if (!marker || marker->ordered != aFirst.ordered || marker->ch != aFirst.ch)
                        break;
                    item current{ {}, *marker };
                    std::string_view const first = aLines[j];
                    current.lines.emplace_back(marker->empty ? std::string_view{} : first.substr(std::min(marker->contentOffset, first.size())));
                    ++j;
                    bool lastBlank = marker->empty;
                    bool blankInside = false;
                    while (j < n)
                    {
                        std::string_view const next = aLines[j];
                        if (is_blank(next))
                        {
                            if (marker->empty && current.lines.size() == 1)
                                break; // an empty item can begin with at most one blank line
                            current.lines.emplace_back();
                            lastBlank = true;
                            ++j;
                            continue;
                        }
                        if (indentation(next) >= marker->contentOffset)
                        {
                            if (lastBlank && current.lines.size() > 1)
                                blankInside = true;
                            current.lines.emplace_back(next.substr(marker->contentOffset));
                            lastBlank = false;
                            ++j;
                            continue;
                        }
                        if (!lastBlank && !starts_block(next) && !match_list_marker(next))
                        {
                            current.lines.emplace_back(trim(next)); // lazy continuation
                            ++j;
                            continue;
                        }
                        break;
                    }
                    std::size_t trailingBlanks = 0;
                    while (!current.lines.empty() && is_blank(current.lines.back()))
                    {
                        current.lines.pop_back();
                        ++trailingBlanks;
                    }
                    if (blankInside)
                        loose = true;
                    items.push_back(std::move(current));
                    if (trailingBlanks > 0)
                    {
                        auto const following = (j < n ? match_list_marker(aLines[j]) : std::nullopt);
                        if (following && following->ordered == aFirst.ordered && following->ch == aFirst.ch)
                            loose = true;
                        else
                            break;
                    }
                }
                aContext.separate();
                long number = aFirst.start;
                for (std::size_t index = 0; index < items.size(); ++index)
                {
                    auto& current = items[index];
                    if (index > 0)
                    {
                        if (loose)
                            aContext.output.block_break(css_length{ 0.0, 1.0 });
                        else
                            aContext.output.line_break();
                    }
                    std::string marker;
                    if (aFirst.ordered)
                        marker = std::to_string(number++) + aFirst.ch;
                    else
                        marker = Bullets[static_cast<std::size_t>(aContext.listDepth) % Bullets.size()];
                    if (!current.lines.empty())
                    {
                        auto& firstLine = current.lines.front();
                        bool const unchecked = firstLine.starts_with("[ ]");
                        bool const checked = firstLine.starts_with("[x]") || firstLine.starts_with("[X]");
                        if ((unchecked || checked) && (firstLine.size() == 3 || firstLine[3] == ' '))
                        {
                            marker = unchecked ? TaskUnchecked : TaskChecked;
                            firstLine.erase(0, std::min<std::size_t>(4, firstLine.size()));
                        }
                    }
                    auto const prefix = aContext.prefix();
                    block_context child{ aContext.output, prefix + std::string{ MarkerBegin } + marker + " " + std::string{ MarkerEnd }, aContext.restPrefix + std::string{ ListIndent }, !loose, aContext.listDepth + 1 };
                    parse_blocks(current.lines, child);
                    if (!child.firstUsed)
                        aContext.output.text(child.firstPrefix, span_format{});
                }
                aContext.separate();
                return j;
            }
            void emit_code(std::vector<std::string_view> const& aLines, block_context& aContext)
            {
                aContext.separate();
                span_format code;
                code.monospace = true;
                code.codeShade = true;
                if (aLines.empty())
                {
                    aContext.output.text(aContext.prefix(), span_format{});
                    return;
                }
                for (std::size_t i = 0; i < aLines.size(); ++i)
                {
                    if (i > 0)
                        aContext.output.newline();
                    aContext.output.text(aContext.prefix(), span_format{});
                    aContext.output.text(aLines[i], code);
                }
            }
            void emit_heading(int aLevel, std::string_view aText, block_context& aContext)
            {
                auto const level = static_cast<std::size_t>(std::clamp(aLevel, 1, 6) - 1);
                aContext.separate(HeadingMargins[level] * HeadingScales[level]);
                span_format format;
                format.bold = true;
                format.fontScale = HeadingScales[level];
                emit_inlines(aText, format, aContext);
                aContext.separate(HeadingMargins[level] * HeadingScales[level]);
            }
            void emit_table(std::vector<std::vector<std::string>> const& aRows, block_context& aContext)
            {
                aContext.separate(0.0); // tables have no margins of their own
                for (std::size_t row = 0; row < aRows.size(); ++row)
                {
                    if (row > 0)
                        aContext.output.newline();
                    span_format format;
                    format.bold = (row == 0);
                    aContext.output.text(aContext.prefix(), span_format{});
                    for (std::size_t column = 0; column < aRows[row].size(); ++column)
                    {
                        if (column > 0)
                            aContext.output.text("\t", span_format{});
                        emit_inline_nodes(aRows[row][column], format, aContext, false);
                    }
                }
                aContext.separate(0.0); // tables have no margins of their own
            }
            void emit_inlines(std::string_view aText, span_format const& aBase, block_context& aContext)
            {
                emit_inline_nodes(aText, aBase, aContext, true);
            }
            void emit_inline_nodes(std::string_view aText, span_format const& aBase, block_context& aContext, bool aEmitPrefix)
            {
                if (aEmitPrefix)
                    aContext.output.text(aContext.prefix(), prefix_format(aBase));
                auto nodes = iInlines.parse(aText);
                iInlines.resolve_emphasis(nodes);
                std::vector<std::size_t> active;
                auto format = aBase;
                auto recompute = [&]()
                {
                    format = aBase;
                    for (auto const id : active)
                        iModifiers[id](format);
                };
                for (auto const& node : nodes)
                {
                    switch (node.kind)
                    {
                    case inline_node::Text:
                        aContext.output.text(node.text, format);
                        break;
                    case inline_node::Delimiter:
                        aContext.output.text(std::string(static_cast<std::size_t>(node.count), node.delimiter), format);
                        break;
                    case inline_node::Push:
                        active.push_back(node.modifierId);
                        recompute();
                        break;
                    case inline_node::Pop:
                        if (auto const existing = std::find(active.rbegin(), active.rend(), node.modifierId); existing != active.rend())
                        {
                            active.erase(std::next(existing).base());
                            recompute();
                        }
                        break;
                    case inline_node::HardBreak:
                        aContext.output.newline();
                        aContext.output.text(aContext.restPrefix, span_format{});
                        break;
                    case inline_node::Object:
                        aContext.output.object(*node.object, format);
                        break;
                    }
                }
            }
        private:
            builder& iOutput;
            link_definitions iDefinitions;
            std::vector<modifier> iModifiers;
            inline_parser iInlines;
        };
    }

    bool is_loadable_image(std::string_view aUri)
    {
        aUri = trim(aUri);
        if (aUri.starts_with(":/") || istarts_with(aUri, "data:image/") || istarts_with(aUri, "file:") ||
            istarts_with(aUri, "http://") || istarts_with(aUri, "https://"))
            return true;
        // absolute paths: /..., \\server\..., C:\... or C:/...
        if (aUri.starts_with("/") || aUri.starts_with("\\\\"))
            return !aUri.starts_with("//");
        return aUri.size() > 2 && is_ascii_alpha(aUri[0]) && aUri[1] == ':' && (aUri[2] == '\\' || aUri[2] == '/');
    }

    document parse_markdown(std::string_view aMarkdown)
    {
        builder output;
        markdown_parser parser{ output };
        parser.parse(aMarkdown);
        return output.finish();
    }

    document parse_html(std::string_view aHtml)
    {
        builder output;
        html_parser parser{ output };
        parser.parse(aHtml);
        auto result = output.finish();
        // the <body>, looked for in all of aHtml: clipboard HTML has it outside the fragment
        std::optional<html_tag> body;
        for (auto pos = ifind(aHtml, "<body"); pos != std::string_view::npos && !body; pos = ifind(aHtml, "<body", pos + 5))
            if (auto tag = parse_tag(aHtml, pos); tag && tag->name == "body" && !tag->closing)
                body = std::move(tag);
        auto const style = body ? body->attribute("style") : std::nullopt;
        if (style)
        {
            span_format horizontal;
            auto const vertical = apply_css_box(horizontal, style.value_or(std::string{}), BodyMargin);
            auto const sum = [](std::optional<css_length> const& aMargin, std::optional<css_length> const& aPadding)
            {
                auto const margin = aMargin.value_or(css_length{ BodyMargin, 0.0 });
                auto const padding = aPadding.value_or(css_length{});
                return css_length{ margin.px + padding.px, margin.em + padding.em };
            };
            result.body = css_box{
                css_length{ horizontal.marginLeft + horizontal.paddingLeft, 0.0 },
                sum(vertical.marginTop, vertical.paddingTop),
                css_length{ horizontal.marginRight + horizontal.paddingRight, 0.0 },
                sum(vertical.marginBottom, vertical.paddingBottom) };
        }
        return result;
    }
}
