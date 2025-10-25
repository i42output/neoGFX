// text_edit.hpp
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

#include <functional>

#include <neolib/core/gap_vector.hpp>
#include <neolib/core/jar.hpp>

#include <neogfx/app/i_clipboard.hpp>
#include <neogfx/gfx/text/glyph_text.hpp>
#include <neogfx/gui/window/context_menu.hpp>
#include <neogfx/gui/widget/scrollable_widget.hpp>
#include <neogfx/gui/widget/i_text_document.hpp>
#include <neogfx/gui/widget/cursor.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/button.hpp>

namespace neogfx
{
    enum class text_edit_caps : std::uint32_t
    {
        None                    = 0x00000000,

        SingleLine              = 0x00000001,
        MultiLine               = 0x00000002,
        GrowLines               = SingleLine | MultiLine,

        OverwriteMode           = 0x00000010,

        Password                = 0x00000100,
        ShowPassword            = 0x00000200,

        NonPrintableWhitespace  = 0x00001000,
        ParseURIs               = 0x00002000,

        OnlyAccept              = 0x00010000,

        LINES_MASK              = SingleLine | MultiLine
    };

    enum class text_edit_line_ending : std::uint32_t
    {
        Lf              = 0,
        CrLf            = 1,
        LfCr            = 2,
        AutomaticLf     = 3,
        AutomaticCrLf   = 4,
        AutomaticLfCr   = 5
    };

    inline bool is_automatic(text_edit_line_ending aLineEnding)
    {
        switch (aLineEnding)
        {
        case text_edit_line_ending::Lf:
        case text_edit_line_ending::CrLf:
        case text_edit_line_ending::LfCr:
        default:
            return false;
        case text_edit_line_ending::AutomaticLf:
        case text_edit_line_ending::AutomaticCrLf:
        case text_edit_line_ending::AutomaticLfCr:
            return true;
        }
    }
}

begin_declare_enum(neogfx::text_edit_caps)
declare_enum_string(neogfx::text_edit_caps, SingleLine)
declare_enum_string(neogfx::text_edit_caps, MultiLine)
declare_enum_string(neogfx::text_edit_caps, GrowLines)
declare_enum_string(neogfx::text_edit_caps, OverwriteMode)
declare_enum_string(neogfx::text_edit_caps, Password)
declare_enum_string(neogfx::text_edit_caps, ShowPassword)
declare_enum_string(neogfx::text_edit_caps, ParseURIs)
declare_enum_string(neogfx::text_edit_caps, OnlyAccept)
end_declare_enum(neogfx::text_edit_caps)

namespace neogfx
{
    inline text_edit_caps operator~(text_edit_caps aLhs)
    {
        return static_cast<text_edit_caps>(~static_cast<std::uint32_t>(aLhs));
    }

    inline text_edit_caps operator&(text_edit_caps aLhs, text_edit_caps aRhs)
    {
        return static_cast<text_edit_caps>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    inline text_edit_caps operator|(text_edit_caps aLhs, text_edit_caps aRhs)
    {
        return static_cast<text_edit_caps>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    class text_edit : public framed_scrollable_widget, public i_clipboard_sink, public i_text_document
    {
        meta_object(framed_scrollable_widget)
        // events
    public:
        define_event(CanAcceptText, can_accept_text, i_string const&, bool&)
        define_event(AcceptText, accept_text, i_string const&)
        define_event(TextFilter, text_filter, i_string const&, bool&)
        define_event(TextChanged, text_changed)
        define_event(DefaultStyleChanged, default_style_changed)
        define_event(ContextMenu, context_menu, i_menu&)
        define_event(UriClicked, uri_clicked, i_string const&)

        // types
    private:
        typedef text_edit property_context_type;

        struct password_bits
        {
            text_edit& parent;
            neogfx::padding previousPadding;
            button<> showPassword;

            password_bits(text_edit& aParent);
            ~password_bits();
        };

    public:
        using cookie = neolib::cookie;
        enum class style_cookie : cookie {};
        enum class tag_cookie : cookie {};

        typedef std::optional<string> optional_password_mask;

        class character_style
        {
        public:
            character_style();
            character_style(character_style const& aOther);
            character_style(
                optional_font const& aFont,
                const color_or_gradient& aTextColor = color_or_gradient{},
                const color_or_gradient& aPaperColor = color_or_gradient{},
                const optional_text_effect& aTextEffect = optional_text_effect{});
        public:
            optional_font const& font() const;
            const color_or_gradient& glyph_color() const;
            const color_or_gradient& text_color() const;
            const color_or_gradient& paper_color() const;
            bool smart_underline() const;
            bool ignore_emoji() const;
            const optional_text_effect& text_effect() const;
            text_format as_text_format() const;
            character_style& set_font(optional_font const& aFont = optional_font{});
            character_style& set_font_if_none(neogfx::font const& aFont);
            character_style& set_glyph_color(const color_or_gradient& aColor = color_or_gradient{});
            character_style& set_text_color(const color_or_gradient& aColor = color_or_gradient{});
            character_style& set_paper_color(const color_or_gradient& aColor = color_or_gradient{});
            character_style& set_text_effect(const optional_text_effect& aEffect = optional_text_effect{});
            character_style& set_from_text_format(const text_format& aTextFormat);
        public:
            character_style& merge(const character_style& aRhs);
        public:
            bool operator==(const character_style& aRhs) const;
            bool operator!=(const character_style& aRhs) const;
            bool operator<(const character_style& aRhs) const;
        private:
            optional_font iFont;
            color_or_gradient iGlyphColor;
            color_or_gradient iTextColor;
            color_or_gradient iPaperColor;
            bool iSmartUnderline;
            bool iIgnoreEmoji;
            optional_text_effect iTextEffect;
        };

        class paragraph_style
        {
        public:
            paragraph_style();
            paragraph_style(paragraph_style const& aOther);
            paragraph_style(optional_alignment const& aAlignment, optional_padding const& aPadding = {}, optional<double> const& aLineSpacing = {});
        public:
            optional_alignment const& alignment() const;
            paragraph_style& set_alignment(optional_alignment const& aALignment = {});
            optional_padding const& padding() const;
            paragraph_style& set_padding(optional_padding const& aPadding = {});
            optional<double> const& line_spacing() const;
            paragraph_style& set_line_spacing(optional<double> const& aLineSpacing = {});
        public:
            paragraph_style& merge(const paragraph_style& aRhs);
        public:
            bool operator==(const paragraph_style& aRhs) const;
            bool operator!=(const paragraph_style& aRhs) const;
            bool operator<(const paragraph_style& aRhs) const;
        public:
            optional_alignment iAlignment;
            optional_padding iPadding;
            optional<double> iLineSpacing;
        };

        class style
        {
        public:
            style();
            style(character_style const& aCharacter);
            style(character_style const& aCharacter, paragraph_style const& aParagraph);
            style(paragraph_style const& aParagraph);
            style(text_edit& aParent, const style& aOther);
        public:
            bool has_cookie() const;
            style_cookie cookie() const;
            void set_cookie(style_cookie aCookie);
        public:
            void add_ref() const;
            void release() const;
        public:
            style& merge(const style& aOverridingStyle);
        public:
            bool operator==(const style& aRhs) const;
            bool operator!=(const style& aRhs) const;
            bool operator<(const style& aRhs) const;
        public:
            character_style const& character() const;
            character_style& character();
            paragraph_style const& paragraph() const;
            paragraph_style& paragraph();
        public:
            style with_font(optional_font const& aFont = optional_font{}) const;
            style with_glyph_color(const color_or_gradient& aColor = color_or_gradient{}) const;
            style with_text_color(const color_or_gradient& aColor = color_or_gradient{}) const;
            style with_paper_color(const color_or_gradient& aColor = color_or_gradient{}) const;
            style with_text_effect(const optional_text_effect& aEffect = optional_text_effect{}) const;
        private:
			text_edit* iParent;
            style_cookie iCookie = neolib::invalid_cookie<style_cookie>;
            mutable std::uint32_t iUseCount;
            character_style iCharacter;
            paragraph_style iParagraph;
        };

        using style_ptr = std::shared_ptr<style>;

        struct column_info
        {
            char32_t delimiter = U'\t';
            optional_dimension minWidth;
            optional_dimension maxWidth;
            neogfx::padding padding;
            std::optional<text_edit::style> style;

            auto operator<=>(column_info const&) const = default;
        };

        struct document_column
        {
            column_info info;
            dimension width = 0.0;
        };

        using document_columns = neolib::vecarray<document_column, 4, -1>;

        class i_tag
        {
        public:
            declare_event(mouse_event, const neogfx::mouse_event&)
            declare_event(mouse_entered_event, const point&)
            declare_event(mouse_left_event)
            declare_event(query_mouse_cursor, neogfx::mouse_cursor&)
            declare_event(keyboard_event, const neogfx::keyboard_event&)
        public:
            virtual ~i_tag() = default;
        public:
            virtual bool has_cookie() const = 0;
            virtual tag_cookie cookie() const = 0;
            virtual void set_cookie(tag_cookie aCookie) = 0;
        public:
            virtual void add_ref() const = 0;
            virtual void release() const = 0;
        public:
            virtual bool less(i_tag const& aRhs) const = 0;
        public:
            virtual uuid const& ttid() const = 0;
            virtual bool has_data() const = 0;
            virtual void const* data() const = 0;
            virtual void* data() = 0;
            virtual i_ref_ptr<i_texture> const& image() const = 0;
        public:
            virtual bool event_origin_is_tag() const = 0;
            virtual void set_event_origin_is_tag(bool aEventOriginIsTag) = 0;
        public:
            bool operator<(i_tag const& aRhs) const
            {
                if (ttid() != aRhs.ttid())
                    return ttid() < aRhs.ttid();
                return less(aRhs);
            }
        };

        using tag_ptr = std::shared_ptr<i_tag>;

        template<typename DataT = void*>
        class tag : public i_tag
        {
        public:
            define_declared_event(Mouse, mouse_event, const neogfx::mouse_event&)
            define_declared_event(MouseEntered, mouse_entered_event, const point&)
            define_declared_event(MouseLeft, mouse_left_event)
            define_declared_event(QueryMouseCursor, query_mouse_cursor, neogfx::mouse_cursor&)
            define_declared_event(Keyboard, keyboard_event, const neogfx::keyboard_event&)
        public:
            using data_type = DataT;
        public:
            tag(uuid const& aTtid = {}) :
                iTtid{ aTtid }
            {
            }
            tag(uuid const& aTtid, data_type const& aData) :
                iTtid{ aTtid }, iData{ aData }
            {
            }
            tag(uuid const& aTtid, data_type const& aData, i_image const& aImage) :
                iTtid{ aTtid }, iData{ aData }, iTexture{ make_ref<texture>(aImage) }
            {
            }
            tag(uuid const& aTtid, i_image const& aImage) :
                iTtid{ aTtid }, iTexture{ make_ref<texture>(aImage) }
            {
            }
            tag(text_edit& aParent, i_tag const& aOther) :
                iParent{ &aParent }, iTtid{ aOther.ttid() }, iTexture{ aOther.image() }
            {
                if (aOther.has_data())
                    iData = *static_cast<data_type const*>(aOther.data());
            }
        public:
            bool has_cookie() const final
            {
                return iCookie != neolib::invalid_cookie<tag_cookie>;
            }
            tag_cookie cookie() const final
            {
                return iCookie;
            }
            void set_cookie(tag_cookie aCookie) final
            {
                iCookie = aCookie;
            }
        public:
            void add_ref() const final
            {
                ++iUseCount;
            }
            void release() const final
            {
                if (iParent && --iUseCount == 0u)
                {
                    iParent->iTags.erase(iParent->iTagMap[cookie()]);
                    iParent->iTagMap.remove(cookie());
                }
            }
        public:
            bool less(i_tag const& aRhs) const final
            {
                if constexpr (have_abstract_base_v<data_type>)
                    return *static_cast<abstract_t<data_type> const*>(data()) < *static_cast<abstract_t<data_type> const*>(aRhs.data());
                else
                    return *static_cast<data_type const*>(data()) < *static_cast<data_type const*>(aRhs.data());
            }
        public:
            uuid const& ttid() const final
            {
                return iTtid;
            }
            bool has_data() const final
            {
                return iData.has_value();
            }
            void const* data() const final
            {
                return has_data() ? &iData.value() : nullptr;
            }
            void* data() final
            {
                return has_data() ? &iData.value() : nullptr;
            }
            i_ref_ptr<i_texture> const& image() const final
            {
                return iTexture;
            }
        public:
            bool event_origin_is_tag() const final
            {
                return iEventOriginIsTag;
            }
            void set_event_origin_is_tag(bool aEventOriginIsTag) final
            {
                iEventOriginIsTag = aEventOriginIsTag;
            }
        private:
            text_edit* iParent = nullptr;
            tag_cookie iCookie = neolib::invalid_cookie<tag_cookie>;
            mutable std::uint32_t iUseCount = 0u;
            bool iEventOriginIsTag = false;
            uuid iTtid = {};
            std::optional<data_type> iData;
                ref_ptr<i_texture> iTexture;
        };

        using style_callback = std::function<std::tuple<const style&, std::ptrdiff_t>(std::ptrdiff_t)>;
        struct ansi {};

        using tag_ptr_callback = std::function<std::tuple<tag_ptr, std::ptrdiff_t>(std::ptrdiff_t)>;

        struct format
        {
            std::variant<std::monostate, style, style_callback, ansi> style;
            std::variant<std::monostate, tag_ptr, tag_ptr_callback> tag;
        };

    private:
        struct style_list_comparator
        {
            bool operator()(const style_ptr& lhs, const style_ptr& rhs) const
            {
                return *lhs < *rhs;
            }
        };
        using style_list = std::set<style_ptr, style_list_comparator>;
        using style_map = neolib::basic_std_vector_jar<style_ptr, style_cookie>;

        class multiple_text_changes;

        struct document_char
        {
            char32_t character;
            style_cookie style;
            tag_cookie tag;

            operator char32_t() const { return character; }

            document_char(char32_t aCharacter) : 
                character{ aCharacter }, 
                style{ neolib::invalid_cookie<style_cookie> },
                tag{ neolib::invalid_cookie<tag_cookie> }
            {}
            document_char(char32_t aCharacter, style_cookie aStyle) : 
                character{ aCharacter }, 
                style{ aStyle },
                tag{ neolib::invalid_cookie<tag_cookie> }
            {}
            document_char(document_char const& aOther) :
                character{ aOther.character },
                style{ aOther.style },
                tag{ aOther.tag }
            {}
        };
        using document_text = neolib::gap_vector<document_char>;

        using glyph_container_type = neolib::gap_vector<glyph_char>;
        using document_glyphs = basic_glyph_text_content<glyph_container_type>;

        struct document_span
        {
            document_text::difference_type textFirst;
            document_text::difference_type textLast;
            document_glyphs::difference_type glyphsFirst;
            document_glyphs::difference_type glyphsLast;

            document_span operator+(document_span const& rhs) const
            {
                return { 
                    textFirst + rhs.textFirst, textFirst + rhs.textFirst + (textLast - textFirst), 
                    glyphsFirst + rhs.glyphsFirst, glyphsFirst + rhs.glyphsFirst + (glyphsLast - glyphsFirst) };
            }
        };

        struct glyph_paragraph
        {
            using column_breaks = neolib::vecarray<document_glyphs::difference_type, 4, -1>;
            using line_breaks = neolib::vecarray<document_glyphs::difference_type, 8, -1>;
            struct height_map_entry
            {
                document_glyphs::difference_type glyphIndex;
                dimension height;
            };
            using height_map = neolib::vecarray<height_map_entry, 8, -1>;

            text_edit* owner;
            document_span span;
            coordinate ypos;
            mutable height_map heightMap;
            column_breaks columnBreaks;
            line_breaks lineBreaks;

            document_text::difference_type text_begin_index() const
            {
                return span.textFirst;
            }
            document_text::difference_type text_end_index() const
            {
                return span.textLast;
            }
            document_text::const_iterator text_begin() const
            {
                return std::next(owner->iText.begin(), text_begin_index());
            }
            document_text::const_iterator text_end() const
            {
                return std::next(owner->iText.begin(), text_end_index());
            }
            document_glyphs::difference_type glyph_begin_index() const
            {
                return span.glyphsFirst;
            }
            document_glyphs::difference_type glyph_end_index() const
            {
                return span.glyphsLast;
            }
            document_glyphs::const_iterator glyph_begin() const
            {
                return std::next(owner->glyphs().begin(), glyph_begin_index());
            }
            document_glyphs::const_iterator glyph_end() const
            {
                return std::next(owner->glyphs().begin(), glyph_end_index());
            }
            dimension height(document_glyphs::iterator aStart, document_glyphs::iterator aEnd) const;
        };

        using glyph_paragraphs = neolib::gap_vector<glyph_paragraph, 16>;

        struct glyph_column;

        struct paragraph_span
        {
            std::size_t paragraphIndex;
            document_span paragraphSpan;
        };

        struct paragraph_line_span
        {
            std::size_t paragraphIndex;
            document_span paragraphSpan;
            document_span lineSpan;
        };

        struct glyph_line
        {
            text_edit* owner;
            std::size_t paragraphIndex;
            std::size_t columnIndex;
            document_span span;
            point offset;
            size extents;
            font_id majorFont;
            scalar baseline;

            glyph_paragraph& paragraph() const
            {
                return owner->iGlyphParagraphs[paragraphIndex];
            }
            glyph_column& column() const
            {
                return owner->iGlyphColumns[columnIndex];
            }
            std::size_t index() const
            {
                return std::distance(&*column().lines.cbegin(), this);
            }
            document_text::difference_type text_begin_index() const
            {
                return span.textFirst + paragraph().text_begin_index();
            }
            document_text::difference_type text_end_index() const
            {
                return span.textLast + paragraph().text_begin_index();
            }
            document_text::const_iterator text_begin() const
            {
                return std::next(owner->iText.begin(), text_begin_index());
            }
            document_text::const_iterator text_end() const
            {
                return std::next(owner->iText.begin(), text_end_index());
            }
            document_glyphs::difference_type glyph_begin_index() const
            {
                return span.glyphsFirst + paragraph().glyph_begin_index();
            }
            document_glyphs::difference_type glyph_end_index() const
            {
                return span.glyphsLast + paragraph().glyph_begin_index();
            }
            document_glyphs::const_iterator glyph_begin() const
            {
                return std::next(owner->glyphs().begin(), glyph_begin_index());
            }
            document_glyphs::const_iterator glyph_end() const
            {
                return std::next(owner->glyphs().begin(), glyph_end_index());
            }
            paragraph_line_span paragraph_span() const
            {
                paragraph_line_span result = { paragraphIndex, paragraph().span, span };
                if (glyph_end() != owner->glyphs().end() && is_line_breaking_whitespace(*glyph_end()))
                    ++result.lineSpan.glyphsLast;
                return result;
            }
            coordinate xpos() const
            {
                return offset.x;
            }
            coordinate ypos() const
            {
                return offset.y + paragraph().ypos;
            }
        };
        using glyph_lines = neolib::vecarray<glyph_line, 8, -1>;

        struct glyph_column
        {
            text_edit* owner;
            glyph_lines lines;
            dimension width = 0.0;

            std::size_t index() const
            {
                return std::distance(&*owner->iGlyphColumns.cbegin(), this);
            }
        };
        using glyph_columns = neolib::vecarray<glyph_column, 4, -1>;

        struct position_info;

        struct tag_list_comparator
        {
            bool operator()(const tag_ptr& lhs, const tag_ptr& rhs) const
            {
                return *lhs < *rhs;
            }
        };
        using tag_list = std::set<tag_ptr, tag_list_comparator>;
        using tag_map = neolib::basic_std_vector_jar<tag_ptr, tag_cookie>;

    private:
        class dragger;

    public:
        using position_type = document_text::difference_type;

        // exceptions
    public:
        struct not_implemented : std::logic_error { not_implemented() : std::logic_error("neogfx::text_edit::not_implemented") {} };
        struct bad_column_index : std::logic_error { bad_column_index() : std::logic_error("neogfx::text_edit::bad_column_index") {} };

        // text_edit
    public:
        text_edit() :
            text_edit{ text_edit_caps::MultiLine, frame_style::SolidFrame } {}
        text_edit(i_widget& aParent) :
            text_edit{ aParent, text_edit_caps::MultiLine, frame_style::SolidFrame } {}
        text_edit(i_layout& aLayout) :
            text_edit{ aLayout, text_edit_caps::MultiLine, frame_style::SolidFrame } {}
        text_edit(text_edit_caps aType) :
            text_edit{ aType, frame_style::SolidFrame } {}
        text_edit(i_widget& aParent, text_edit_caps aType) :
            text_edit{ aParent, aType, frame_style::SolidFrame } {}
        text_edit(i_layout& aLayout, text_edit_caps aType) :
            text_edit{ aLayout, aType, frame_style::SolidFrame } {}
        text_edit(frame_style aFrameStyle) :
            text_edit{ text_edit_caps::MultiLine, aFrameStyle } {}
        text_edit(i_widget& aParent, frame_style aFrameStyle) :
            text_edit{ aParent, text_edit_caps::MultiLine, aFrameStyle } {}
        text_edit(i_layout& aLayout, frame_style aFrameStyle) :
            text_edit{ aLayout, text_edit_caps::MultiLine, aFrameStyle } {}
        text_edit(text_edit_caps aType, frame_style aFrameStyle);
        text_edit(i_widget& aParent, text_edit_caps aType, frame_style aFrameStyle);
        text_edit(i_layout& aLayout, text_edit_caps aType, frame_style aFrameStyle);
        ~text_edit();
        // scrollable_widget
    public:
        void moved() override;
        void resized() override;
        void layout_items(bool aDefer) override;
    public:
        size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
        size maximum_size(optional_size const& aAvailableSpace = optional_size{}) const override;
        neogfx::padding padding() const override;
    public:
        void paint(i_graphics_context& aGc) const override;
    public:
        color palette_color(color_role aColorRole) const override;
    public:
        const neogfx::font& font() const override;
        void set_font(optional_font const& aFont) override;
    public:
        bool capture_locks_cursor() const override;
        void focus_gained(focus_reason aFocusReason) override;
        void focus_lost(focus_reason aFocusReason) override;
    public:
        void mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) override;
        void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifier aKeyModifier) override;
        void mouse_button_released(mouse_button aButton, const point& aPosition) override;
        void mouse_moved(const point& aPosition, key_modifier aKeyModifier) override;
        void mouse_entered(const point& aPosition) override;
        void mouse_left() override;
        neogfx::mouse_cursor mouse_cursor() const override;
    public:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier) override;
        bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier) override;
        bool text_input(i_string const& aText) override;
    public:
        neogfx::scrolling_disposition scrolling_disposition() const override;
        neogfx::scrolling_disposition scrolling_disposition(const i_widget&) const override;
    public:
        rect scroll_area() const override;
        rect scroll_page() const override;
    public:
        bool use_scrollbar_container_updater() const override;
        using framed_scrollable_widget::update_scrollbar_visibility;
        bool update_scrollbar_visibility(usv_stage_e aStage) override;
        void scroll_page_updated() override;
    public:
        color frame_color() const override;
        // i_clipboard
    public:
        bool can_undo() const override;
        bool can_redo() const override;
        bool can_cut() const override;
        bool can_copy() const override;
        bool can_paste() const override;
        bool can_delete_selected() const override;
        bool can_select_all() const override;
        void undo(i_clipboard& aClipboard) override;
        void redo(i_clipboard& aClipboard) override;
        void cut(i_clipboard& aClipboard) override;
        void copy(i_clipboard& aClipboard) override;
        void paste(i_clipboard& aClipboard) override;
        void delete_selected() override;
        void select_all() override;
        // i_text_document
    public:
        std::size_t document_length() const override;
        void move_cursor(cursor::move_operation_e aMoveOperation, bool aMoveAnchor = true) override;
        bool overwrite_cursor_available() const override;
    public:
        i_string const& plain_text() const override;
        bool set_plain_text(i_string const& aPlainText) override;
        i_string const& rich_text(rich_text_format aFormat = rich_text_format::Html) const override;
        bool set_rich_text(i_string const& aRichText, rich_text_format aFormat = rich_text_format::Html) override;
    public:
        void paste_plain_text() override;
        void paste_rich_text(rich_text_format aFormat = rich_text_format::Html) override;
    public:
        void begin_update() override;
        void end_update() override;
        // text_edit
    public:
        text_edit_line_ending line_ending() const;
        void set_line_ending(text_edit_line_ending aLineEnding);
        bool read_only() const;
        void set_read_only(bool aReadOnly = true);
        bool word_wrap() const;
        void set_word_wrap(bool aWordWrap = true);
        std::uint32_t grow_lines() const;
        void set_grow_lines(std::uint32_t aGrowLines = 5u);
        bool password() const;
        i_string const& password_mask() const;
        void set_password(bool aPassword, i_string const& aMask = string{ "\xE2\x97\x8F" });
        const style& default_style() const;
        void set_default_style(style const& aDefaultStyle, bool aPersist = false);
        color default_text_color() const;
        neogfx::alignment alignment() const;
        void set_alignment(neogfx::alignment aAlignment);
    public:
        style current_style() const;
        void apply_style(style const& aStyle);
        void apply_style(position_type aStart, position_type aEnd, style const& aStyle);
        style next_style() const;
    public:
        template <typename DataT>
        std::pair<tag_ptr, bool> create_tag(uuid const& aTtid, DataT const& aData)
        {
            bool created = false;
            tag<DataT> key{ aTtid, aData };
            auto existingTag = iTags.find(tag_ptr{ tag_ptr{}, &key });
            if (existingTag == iTags.end())
            {
                existingTag = iTags.insert(std::make_shared<tag<DataT>>(*this, key)).first;
                created = true;
            }
            auto const& tagPtr = *existingTag;
            auto& tag = *tagPtr;
            if (!tag.has_cookie())
                tag.set_cookie(iTagMap.insert(tagPtr));
            return std::pair<tag_ptr, bool>{ tagPtr, created };
        }
    public:
        void clear();
        std::size_t paragraph_count() const;
        void delete_paragraph(std::size_t aParagraphIndex);
        i_string const& text() const;
        std::size_t set_text(i_string const& aText);
        std::size_t set_text(i_string const& aText, format const& aFormat);
        std::size_t append_text(i_string const& aText, bool aMoveCursor = false);
        std::size_t append_text(i_string const& aText, format const& aFormat, bool aMoveCursor = false);
        std::size_t insert_text(i_string const& aText, bool aMoveCursor = false);
        std::size_t insert_text(i_string const& aText, format const& aFormat, bool aMoveCursor = false);
        std::size_t insert_text(position_type aPosition, i_string const& aText, bool aMoveCursor = false);
        std::size_t insert_text(position_type aPosition, i_string const& aText, format const& aFormat, bool aMoveCursor = false);
        std::size_t set_text(std::string const& aText) { return set_text(string{ aText }); }
        std::size_t set_text(std::string const& aText, format const& aFormat) { return set_text(string{ aText }, aFormat); }
        std::size_t append_text(std::string const& aText, bool aMoveCursor = false) { return append_text(string{ aText }, aMoveCursor); }
        std::size_t append_text(std::string const& aText, format const& aFormat, bool aMoveCursor = false) { return append_text(string{ aText }, aFormat, aMoveCursor); }
        std::size_t insert_text(std::string const& aText, bool aMoveCursor = false) { return insert_text(string{ aText }, aMoveCursor); }
        std::size_t insert_text(std::string const& aText, format const& aFormat, bool aMoveCursor = false) { return insert_text(string{ aText }, aFormat, aMoveCursor); }
        std::size_t insert_text(position_type aPosition, std::string const& aText, bool aMoveCursor = false) { return insert_text(aPosition, string{ aText }, aMoveCursor); }
        std::size_t insert_text(position_type aPosition, std::string const& aText, format const& aFormat, bool aMoveCursor = false) { return insert_text(aPosition, string{ aText }, aFormat, aMoveCursor); }
        void delete_text(position_type aStart, position_type aEnd);
        std::size_t columns() const;
        void set_columns(std::size_t aColumnCount);
        void remove_columns();
        const column_info& column(std::size_t aColumnIndex) const;
        void set_column(std::size_t aColumnIndex, const column_info& aColumn);
        const style& column_style(std::size_t aColumnIndex) const;
        const style& column_style(const column_info& aColumn) const;
    public:
        bool has_page_rect() const;
        rect page_rect() const;
        void set_page_rect(optional_rect const& aPageRect);
        const neogfx::size_hint& size_hint() const;
        void set_size_hint(const neogfx::size_hint& aSizeHint);
        void set_tab_stop_hint(i_string const& aTabStopHint = string{ "0000" });
        neogfx::tab_stops const& tab_stops() const;
        void set_tab_stops(std::optional<neogfx::tab_stops> const& aTabStops);
    public:
        position_type document_hit_test(const point& aPosition, bool aAdjustForScrollPosition = true) const;
        std::pair<position_type, bool> document_hit_test_ex(const point& aPosition, bool aAdjustForScrollPosition = true) const;
        virtual bool same_word(position_type aTextPositionLeft, position_type aTextPositionRight) const;
        virtual std::pair<position_type, position_type> word_at(position_type aTextPosition, bool aWordBreakIsWhitespace = false) const;
    public:
        neogfx::cursor& cursor() const;
        void set_cursor_position(const point& aPosition, bool aMoveAnchor = true, bool aEnableDragger = false);
    public:
        void cancel_object_selection();
    protected:
        rect column_rect(std::size_t aColumnIndex, bool aExtendIntoPadding = false) const;
        std::size_t column_hit_test(const point& aPosition, bool aAdjustForScrollPosition = true) const;
    private:
        position_info glyph_position(position_type aGlyphPosition, bool aForCursor = false) const;
        position_type cursor_glyph_position() const;
        position_type cursor_glyph_anchor() const;
        void set_cursor_glyph_position(position_type aGlyphPosition, bool aMoveAnchor = true);
    private:
        void init();
        document_glyphs const& glyphs() const;
        document_glyphs& glyphs();
        std::size_t do_insert_text(position_type aPosition, i_string const& aText, format const& aFormat, bool aMoveCursor, bool aClearFirst);
        void delete_any_selection();
        void notify_text_changed();
        bool same_paragraph(position_type aFirstGlyphPos, position_type aSecondGlyphPos) const;
        paragraph_span character_to_paragraph(position_type aCharacterPos) const;
        paragraph_line_span character_to_line(position_type aCharacterPos) const;
        glyph_paragraphs::const_iterator glyph_to_paragraph(position_type aGlyphPos) const;
        document_glyphs::const_iterator to_glyph(document_text::const_iterator aWhere) const;
        std::pair<document_text::size_type, document_text::size_type> from_glyph(document_glyphs::const_iterator aWhere) const;
        std::optional<glyph_lines::const_iterator> next_line(std::optional<glyph_lines::const_iterator> const& aFrom) const;
        std::optional<glyph_lines::const_iterator> previous_line(std::optional<glyph_lines::const_iterator> const& aFrom) const;
        void refresh_paragraph(document_text::const_iterator aWhere, ptrdiff_t aDelta);
        void refresh_columns();
        void refresh_lines();
        void animate();
        void update_cursor();
        void make_cursor_visible(bool aForcePreviewScroll = false);
        std::pair<document_glyphs::difference_type, bool> glyph_hit_test(const point& aPosition, bool aAdjustForScrollPosition = true) const;
        void make_visible(position_info const& aGlyphPosition, point const& aPreview = {});
        style glyph_style(document_glyphs::const_iterator aGlyphChar, const document_column& aColumn) const;
        void draw_glyphs(i_graphics_context const& aGc, const point& aPosition, const glyph_column& aColumn, glyph_lines::const_iterator aLine) const;
        void draw_cursor(i_graphics_context const& aGc) const;
        rect cursor_rect() const;
        double calc_padding_adjust(style const& aStyle) const;
        double padding_adjust() const;
    private:
        sink iSink;
        text_edit_caps iCaps;
        text_edit_line_ending iLineEnding;
        style iDefaultStyle;
        mutable std::optional<style> iNextStyle;
        bool iPersistDefaultStyle;
        font_info iDefaultFont;
        mutable neogfx::cursor iCursor;
        style_list iStyles;
        style_map iStyleMap;
        tag_list iTags;
        tag_map iTagMap;
        tag_ptr iTagCapturing;
        bool iUpdatingContents;
        bool iUpdatingDocument;
        bool iHandlingKeyPress;
        document_text iPreviousText;
        document_text iText;
        optional_rect iPageRect;
        document_columns iColumns;
        mutable std::optional<string> iUtf8TextCache;
        mutable std::optional<document_glyphs> iGlyphs;
        glyph_paragraphs iGlyphParagraphs;
        glyph_columns iGlyphColumns;
        optional_size iTextExtents;
        std::uint64_t iCursorAnimationStartTime;
        neogfx::size_hint iSizeHint;
        mutable std::optional<std::pair<neogfx::font, size>> iHintedSize;
        std::optional<neogfx::tab_stops> iTabStops;
        string iTabStopHint;
        mutable std::optional<std::pair<neogfx::font, neogfx::tab_stops>> iCalculatedTabStops;
        basic_point<std::optional<dimension>> iCursorHint;
        widget_timer iAnimator;
        std::unique_ptr<dragger> iDragger;
        std::unique_ptr<neogfx::context_menu> iMenu;
        std::uint32_t iSuppressTextChangedNotification;
        std::uint32_t iWantedToNotifyTextChanged;
        std::optional<std::pair<text_edit::position_type, text_edit::position_type>> iSelectedUri;
        std::optional<password_bits> iPasswordBits;
        bool iOutOfMemory;
    public:
        define_property(property_category::other, bool, ReadOnly, read_only, false)
        define_property(property_category::other, bool, WordWrap, word_wrap, (iCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine)
        define_property(property_category::other, std::uint32_t, GrowLines, grow_lines, 5u)
        define_property(property_category::other, bool, Password, password, false)
        define_property(property_category::other, string, PasswordMask, password_mask)
    };

    extern template class basic_glyph_text_content<text_edit::glyph_container_type>;
}