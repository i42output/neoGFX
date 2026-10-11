// rich_text_parser.hpp
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

#pragma once

// Private to text_edit: converts GitHub Flavored Markdown or basic HTML (the Qt rich text
// subset) into UTF-8 text plus formatting runs. Uses only std types so text_edit can map
// the result onto its own styles and tags.

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace neogfx::rich_text
{
    enum class block_alignment : std::uint8_t
    {
        Default,
        Left,
        Center,
        Right,
        Justify
    };

    // a run whose text is U+FFFC (OBJECT REPLACEMENT CHARACTER) is shown as this object
    struct inline_object
    {
        enum class kind_e : std::uint8_t
        {
            Math,
            Image,
            Rule // <hr> or a Markdown thematic break: a line across its paragraph
        };
        kind_e kind = kind_e::Math;
        std::string source;             // LaTeX, or the image URI
        bool display = false;           // display (block) math
        std::string alt;                // image alternative text
        std::optional<double> width;    // image size given by the source, in pixels
        std::optional<double> height;

        auto operator<=>(inline_object const&) const = default;
    };

    // images text_edit can load (embedded resources, data URIs, local files and http(s) URLs); others import as their alt text
    bool is_loadable_image(std::string_view aUri);

    // a CSS length as CSS pixels (96 per inch) plus ems of the base font; text_edit converts both to its own units
    struct css_length
    {
        double px = 0.0;
        double em = 0.0;

        bool operator==(css_length const&) const = default;
    };

    struct span_format
    {
        bool bold = false;
        bool italic = false;
        bool underline = false;
        bool strike = false;
        bool monospace = false;
        bool codeShade = false; // Markdown code span / code block background
        bool superscript = false;
        bool subscript = false;
        double fontScale = 1.0; // relative to the widget's font size
        std::optional<double> pointSize; // absolute; overrides fontScale
        std::optional<std::string> fontFamily;
        std::optional<std::string> textColor; // CSS colour text, e.g. "#ff0000", "red", "rgb(1,2,3)"
        std::optional<std::string> backgroundColor;
        std::optional<std::string> link;
        std::optional<inline_object> object;
        block_alignment alignment = block_alignment::Default;
        std::uint32_t indent = 0; // block indent level (lists, block quotes); the same for every run in a paragraph
        std::uint32_t rightIndent = 0; // block quotes are indented on the right too
        // a paragraph's top and bottom margins and padding: those of the blocks it starts or ends (explicit CSS, otherwise
        // browser defaults); text_edit collapses the margins where paragraphs meet
        css_length marginTop;
        css_length marginBottom;
        css_length paddingTop;
        css_length paddingBottom;
        // a paragraph's explicit CSS margin-left/right and padding-left/right in CSS pixels, summed over its enclosing blocks
        double marginLeft = 0.0;
        double marginRight = 0.0;
        double paddingLeft = 0.0;
        double paddingRight = 0.0;
        bool listMarker = false; // a list item's marker (bullet, number or task box), which hangs into the indent

        bool operator==(span_format const&) const = default;
    };

    struct run
    {
        std::size_t end; // UTF-8 byte offset one past the end of the run
        span_format format;
    };

    struct css_box
    {
        css_length left;
        css_length top;
        css_length right;
        css_length bottom;
    };

    struct document
    {
        std::string text; // UTF-8; '\n' separates paragraphs
        std::vector<run> runs; // contiguous, non-empty, covering all of text
        std::optional<css_box> body; // HTML with a <body style>: its margin plus padding (a browser's default 8px margin where
                                     // the style doesn't say), which inset all of the content
    };

    document parse_markdown(std::string_view aMarkdown);
    document parse_html(std::string_view aHtml);
}
