// font.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

#include <neolib/core/jar.hpp>
#include <neolib/core/variant.hpp>
#include <neolib/app/i_settings.hpp>

#include <neogfx/core/geometrical.hpp>

namespace neogfx
{
    class i_native_font_face;
    struct glyph_char;
    class i_glyph;

    enum class font_style : std::uint32_t
    {
        Invalid             = 0x00000000,
        Normal              = 0x00000001,
        Italic              = 0x00000002,
        Bold                = 0x00000004,
        Underline           = 0x00000008,
        Superscript         = 0x00000010,
        Subscript           = 0x00000020,
        BelowAscenderLine   = 0x00000040,
        AboveBaseline       = 0x00000040,
        Strike              = 0x00000100,
        Emulated            = 0x80000000,
        BoldItalic          = Bold | Italic,
        BoldItalicUnderline = Bold | Italic | Underline,
        BoldUnderline       = Bold | Underline,
        ItalicUnderline     = Italic | Underline,
        EmulatedBold        = Bold | Emulated,
        EmulatedItalic      = Italic | Emulated,
        EmulatedBoldItalic  = Bold | Italic | Emulated
    };

    inline constexpr font_style operator~(font_style aLhs)
    {
        return static_cast<font_style>(~static_cast<std::uint32_t>(aLhs));
    }

    inline constexpr font_style operator|(font_style aLhs, font_style aRhs)
    {
        return static_cast<font_style>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr font_style operator&(font_style aLhs, font_style aRhs)
    {
        return static_cast<font_style>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr font_style operator^(font_style aLhs, font_style aRhs)
    {
        return static_cast<font_style>(static_cast<std::uint32_t>(aLhs) ^ static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr font_style& operator|=(font_style& aLhs, font_style aRhs)
    {
        return aLhs = static_cast<font_style>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr font_style& operator&=(font_style& aLhs, font_style aRhs)
    {
        return aLhs = static_cast<font_style>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr font_style& operator^=(font_style& aLhs, font_style aRhs)
    {
        return aLhs = static_cast<font_style>(static_cast<std::uint32_t>(aLhs) ^ static_cast<std::uint32_t>(aRhs));
    }
}

begin_declare_enum(neogfx::font_style)
declare_enum_string(neogfx::font_style, Invalid)
declare_enum_string(neogfx::font_style, Normal)
declare_enum_string(neogfx::font_style, Italic)
declare_enum_string(neogfx::font_style, Bold)
declare_enum_string(neogfx::font_style, Underline)
declare_enum_string(neogfx::font_style, Superscript)
declare_enum_string(neogfx::font_style, Subscript)
declare_enum_string(neogfx::font_style, BelowAscenderLine)
declare_enum_string(neogfx::font_style, AboveBaseline)
declare_enum_string(neogfx::font_style, Strike)
declare_enum_string(neogfx::font_style, Emulated)
declare_enum_string(neogfx::font_style, BoldItalic)
declare_enum_string(neogfx::font_style, BoldItalicUnderline)
declare_enum_string(neogfx::font_style, BoldUnderline)
declare_enum_string(neogfx::font_style, ItalicUnderline)
declare_enum_string(neogfx::font_style, EmulatedBold)
declare_enum_string(neogfx::font_style, EmulatedItalic)
declare_enum_string(neogfx::font_style, EmulatedBoldItalic)
end_declare_enum(neogfx::font_style)

namespace neogfx 
{
    enum class font_weight : std::uint32_t
    {
        Unknown     = 0,
        Thin        = 100,
        Extralight  = 200,
        Ultralight  = 200,
        Light       = 300,
        Normal      = 400,
        Regular     = 400,
        Medium      = 500,
        Semibold    = 600,
        Demibold    = 600,
        Bold        = 700,
        Extrabold   = 800,
        Ultrabold   = 800,
        Heavy       = 900,
        Black       = 900
    };

    enum class stroke_line_cap : std::uint32_t
    {
        Butt    = 0,
        Round   = 1,
        Square  = 2
    };

    enum class stroke_line_join : std::uint32_t
    {
        Round           = 0,
        Bevel           = 1,
        MiterVariable   = 2,
        Miter           = MiterVariable,
        MiterFixed      = 3
    };

    struct stroke
    {
        scalar radius = 0.0;
        stroke_line_cap lineCap = stroke_line_cap::Round;
        stroke_line_join lineJoin = stroke_line_join::Round;
        scalar miterLimit = 0.0;

        auto operator<=>(stroke const&) const = default;
    };

    class font_info
    {
        // exceptions
    public:
        struct unknown_style : std::logic_error { unknown_style() : std::logic_error("neogfx::font_info::unknown_style") {} };
        struct unknown_style_name : std::logic_error { unknown_style_name() : std::logic_error("neogfx::font_info::unknown_style_name") {} };
        // types
    public:
        typedef font_info abstract_type; // todo
        typedef double point_size;
    private:
        typedef std::optional<font_style> optional_style;
        typedef std::optional<string> optional_style_name;
    private:
        class instance;
    public:
        font_info();
        font_info(string const& aFamilyName, font_style aStyle, point_size aSize);
        font_info(string const& aFamilyName, string const& aStyleName, point_size aSize);
        font_info(string const& aFamilyName, font_style aStyle, string const& aStyleName, point_size aSize);
        font_info(const font_info& aOther);
        virtual ~font_info();
        font_info& operator=(const font_info& aOther);
    private:
        font_info(string const& aFamilyName, const optional_style& aStyle, const optional_style_name& aStyleName, point_size aSize);
    public:
        i_string const& family_name() const;
        bool style_available() const;
        font_style style() const;
        font_style style_maybe() const;
        bool style_name_available() const;
        i_string const& style_name() const;
        bool underline() const;
        void set_underline(bool aUnderline);
        font_weight weight() const;
        point_size size() const;
        stroke outline() const;
        void set_outline(stroke aOutline);
        bool kerning() const;
        void enable_kerning();
        void disable_kerning();
    public:
        font_info with_style(font_style aStyle) const;
        font_info with_style_xor(font_style aStyle) const;
        font_info with_style_name(string const& aStyleName) const;
        font_info with_underline(bool aUnderline) const;
        font_info with_size(point_size aSize) const;
        font_info with_outline(stroke aOutline) const;
    public:
        auto operator<=>(const font_info& aRhs) const = default;
    public:
        static font_weight weight_from_style(font_style aStyle);
        static font_weight weight_from_style_name(string const& aStyleName, bool aUnknownAsRegular = true);
    private:
        string iFamilyName;
        optional_style iStyle;
        optional_style_name iStyleName;
        bool iUnderline;
        font_weight iWeight;
        point_size iSize;
        stroke iOutline;
        bool iKerning;
    };

    using optional_font_info = optional<font_info>;

    using font_id = neolib::small_cookie;
    
    // todo: abstract font
    class font
    {
        friend class font_manager;
        friend class graphics_context;
        // exceptions
    public:
        struct no_fallback_font : std::logic_error { no_fallback_font() : std::logic_error("neogfx::font::no_fallback_font") {} };
        // types
    public:
        typedef font abstract_type; // todo
        using point_size = font_info::point_size;
    private:
        class instance;
        // construction
    public:
        font();
        font(string const& aFamilyName, font_style aStyle, point_size aSize);
        font(string const& aFamilyName, string const& aStyleName, point_size aSize);
        font(const font_info& aFontInfo);
        font(const font& aOther);
        font(const font& aOther, font_style aStyle, point_size aSize);
        font(const font& aOther, string const& aStyleName, point_size aSize);
        static font load_from_file(string const& aFileName);
        static font load_from_file(string const& aFileName, font_style aStyle, point_size aSize);
        static font load_from_file(string const& aFileName, string const& aStyleName, point_size aSize);
        static font load_from_memory(const void* aData, std::size_t aSizeInBytes);
        static font load_from_memory(const void* aData, std::size_t aSizeInBytes, font_style aStyle, point_size aSize);
        static font load_from_memory(const void* aData, std::size_t aSizeInBytes, string const& aStyleName, point_size aSize);
        ~font();
        font& operator=(const font& aOther);
    private:
        font(i_native_font_face& aNativeFontFace);
        font(i_native_font_face& aNativeFontFace, font_style aStyle);
    public:
        font with_style(font_style aStyle) const;
        font with_style_xor(font_style aStyle) const;
        font with_underline(bool aUnderline) const;
        font with_size(point_size aSize) const;
        font with_outline(stroke aOutline) const;
    public:
        font_id id() const;
        font_info const& info() const;
    public:
        bool has_fallback() const;
        font fallback() const;
        // operations
    public:
        i_string const& family_name() const;
        font_style style() const;
        i_string const& style_name() const;
        bool underline() const;
        point_size size() const;
        neogfx::size em_size() const;
        dimension height() const;
        dimension max_advance() const;
        dimension ascender() const;
        dimension descender() const;
        dimension line_spacing() const;
        bool kerning() const;
        dimension kerning(std::uint32_t aLeftGlyphIndex, std::uint32_t aRightGlyphIndex) const;
        bool is_bitmap_font() const;
        std::uint32_t num_fixed_sizes() const;
        point_size fixed_size(std::uint32_t aFixedSizeIndex) const;
    public:
        const i_glyph& glyph(const glyph_char& aGlyphChar) const;
    public:
        bool operator==(const font& aRhs) const;
        std::partial_ordering operator<=>(const font& aRhs) const;
    public:
        i_native_font_face& native_font_face() const;
        // attributes
    private:
        mutable std::shared_ptr<instance> iInstance;
        mutable std::optional<point_size> iSize;
        mutable std::optional<neogfx::size> iEmSize;
        mutable std::optional<dimension> iHeight;
        mutable std::optional<dimension> iMaxAdvance;
        mutable std::optional<dimension> iAscender;
        mutable std::optional<dimension> iDescender;
    };

    typedef optional<font> optional_font;

    template <typename Elem, typename Traits>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const font_info& aFontInfo)
    {
        aStream << "[";
        aStream << aFontInfo.family_name();
        aStream << ",";
        if (aFontInfo.style_name_available())
            aStream << aFontInfo.style_name();
        else
            aStream << aFontInfo.style();
        aStream << ", ";
        aStream << aFontInfo.size();
        aStream << ", ";
        aStream << aFontInfo.underline();
        aStream << ", ";
        aStream << aFontInfo.kerning();
        aStream << "]";
        return aStream;
    }

    template <typename Elem, typename Traits>
    inline std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& aStream, font_info& aFontInfo)
    {
        string familyName;
        std::variant<font_style, string> style;
        font_info::point_size size;
        bool underline;
        bool kerning;

        auto previousImbued = aStream.getloc();
        if (typeid(std::use_facet<std::ctype<char>>(previousImbued)) != typeid(neolib::comma_only_whitespace))
            aStream.imbue(std::locale{ previousImbued, new neolib::comma_only_whitespace{} });
        char ignore;
        aStream >> ignore;
        aStream >> familyName;
        string styleString;
        aStream >> styleString;
        auto tryStyle = neolib::try_string_to_enum<font_style>(styleString);
        if (tryStyle.has_value())
            style = tryStyle.value();
        else
            style = styleString;
        aStream.imbue(std::locale{ previousImbued, new neolib::comma_as_whitespace{} });
        aStream >> size;
        aStream >> underline;
        aStream >> kerning;
        aStream >> ignore;
        aStream.imbue(previousImbued);

        if (std::holds_alternative<font_style>(style))
            aFontInfo = font_info{ familyName, std::get<font_style>(style), size };
        else
            aFontInfo = font_info{ familyName, std::get<string>(style), size };
        aFontInfo.set_underline(underline);
        if (kerning)
            aFontInfo.enable_kerning();
        else
            aFontInfo.disable_kerning();

        return aStream;
    }
}

define_setting_type_as(neogfx::font_info, neogfx::font)
