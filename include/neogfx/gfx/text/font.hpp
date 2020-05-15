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
#include <neolib/jar.hpp>
#include <neolib/variant.hpp>
#include <neogfx/core/geometrical.hpp>

namespace neogfx
{
    class i_native_font_face;
    class glyph;
    class i_glyph_texture;

    enum class font_style : uint32_t
    {
        Invalid = 0x00,
        Normal = 0x01,
        Italic = 0x02,
        Bold = 0x04,
        Underline = 0x08,
        BoldItalic = Bold | Italic,
        BoldItalicUnderline = Bold | Italic | Underline,
        BoldUnderline = Bold | Underline,
        ItalicUnderline = Italic | Underline
    };

    inline constexpr font_style operator|(font_style aLhs, font_style aRhs)
    {
        return static_cast<font_style>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr font_style operator&(font_style aLhs, font_style aRhs)
    {
        return static_cast<font_style>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    inline constexpr font_style& operator|=(font_style& aLhs, font_style aRhs)
    {
        return aLhs = static_cast<font_style>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr font_style& operator&=(font_style& aLhs, font_style aRhs)
    {
        return aLhs = static_cast<font_style>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    enum class font_weight : uint32_t
    {
        Thin = 100,
        Extralight = 200,
        Ultralight = 200,
        Light = 300,
        Normal = 400,
        Regular = 400,
        Medium = 500,
        Semibold = 600,
        Demibold = 600,
        Bold = 700,
        Extrabold = 800,
        Ultrabold = 800,
        Heavy = 900,
        Black = 900
    };

    class font_info
    {
        // exceptions
    public:
        struct unknown_style : std::logic_error { unknown_style() : std::logic_error("neogfx::font_info::unknown_style") {} };
        struct unknown_style_name : std::logic_error { unknown_style_name() : std::logic_error("neogfx::font_info::unknown_style_name") {} };
        // types
    public:
        typedef double point_size;
    private:
        typedef std::optional<font_style> optional_style;
        typedef std::optional<std::string> optional_style_name;
    private:
        class instance;
    public:
        font_info();
        font_info(const std::string& aFamilyName, font_style aStyle, point_size aSize);
        font_info(const std::string& aFamilyName, const std::string& aStyleName, point_size aSize);
        font_info(const std::string& aFamilyName, font_style aStyle, const std::string& aStyleName, point_size aSize);
        font_info(const font_info& aOther);
        virtual ~font_info();
        font_info& operator=(const font_info& aOther);
    private:
        font_info(const std::string& aFamilyName, const optional_style& aStyle, const optional_style_name& aStyleName, point_size aSize);
    public:
        virtual const std::string& family_name() const;
        virtual bool style_available() const;
        virtual font_style style() const;
        virtual bool style_name_available() const;
        virtual const std::string& style_name() const;
        virtual bool underline() const;
        virtual void set_underline(bool aUnderline);
        virtual font_weight weight() const;
        virtual point_size size() const;
        virtual bool kerning() const;
        virtual void enable_kerning();
        virtual void disable_kerning();
    public:
        font_info with_style(font_style aStyle) const;
        font_info with_size(point_size aSize) const;
    public:
        bool operator==(const font_info& aRhs) const;
        bool operator!=(const font_info& aRhs) const;
        bool operator<(const font_info& aRhs) const;
    public:
        static font_weight weight_from_style(font_style aStyle);
        static font_weight weight_from_style_name(std::string aStyleName);
    private:
        mutable std::shared_ptr<instance> iInstance;
    };

    typedef neolib::small_cookie font_id;

    class font : public font_info
    {
        friend class font_manager;
        friend class graphics_context;
        // exceptions
    public:
        struct no_fallback_font : std::logic_error { no_fallback_font() : std::logic_error("neogfx::font::no_fallback_font") {} };
        // types
    public:
    private:
        class instance;
        // construction
    public:
        font();
        font(const std::string& aFamilyName, font_style aStyle, point_size aSize);
        font(const std::string& aFamilyName, const std::string& aStyleName, point_size aSize);
        font(const font_info& aFontInfo);
        font(const font& aOther);
        font(const font& aOther, font_style aStyle, point_size aSize);
        font(const font& aOther, const std::string& aStyleName, point_size aSize);
        static font load_from_file(const std::string& aFileName);
        static font load_from_file(const std::string& aFileName, font_style aStyle, point_size aSize);
        static font load_from_file(const std::string& aFileName, const std::string& aStyleName, point_size aSize);
        static font load_from_memory(const void* aData, std::size_t aSizeInBytes);
        static font load_from_memory(const void* aData, std::size_t aSizeInBytes, font_style aStyle, point_size aSize);
        static font load_from_memory(const void* aData, std::size_t aSizeInBytes, const std::string& aStyleName, point_size aSize);
        ~font();
        font& operator=(const font& aOther);
    private:
        font(std::shared_ptr<i_native_font_face> aNativeFontFace);
        font(std::shared_ptr<i_native_font_face> aNativeFontFace, font_style aStyle);
    public:
        font_id id() const;
    private:
        long use_count() const;
    public:
        bool has_fallback() const;
        font fallback() const;
        // operations
    public:
        const std::string& family_name() const override;
        font_style style() const override;
        const std::string& style_name() const override;
        point_size size() const override;
        dimension height() const;
        dimension descender() const;
        dimension line_spacing() const;
        using font_info::kerning;
        dimension kerning(uint32_t aLeftGlyphIndex, uint32_t aRightGlyphIndex) const;
        bool is_bitmap_font() const;
        uint32_t num_fixed_sizes() const;
        point_size fixed_size(uint32_t aFixedSizeIndex) const;
    public:
        const i_glyph_texture& glyph_texture(const glyph& aGlyph) const;
    public:
        bool operator==(const font& aRhs) const;
        bool operator!=(const font& aRhs) const;
        bool operator<(const font& aRhs) const;
    public:
        i_native_font_face& native_font_face() const;
        std::shared_ptr<i_native_font_face> native_font_face_ptr() const;
        // attributes
    private:
        mutable std::shared_ptr<instance> iInstance;
    };

    typedef std::optional<font> optional_font;
}