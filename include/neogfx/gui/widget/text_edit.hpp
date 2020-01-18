// text_edit.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <neolib/tag_array.hpp>
#include <neolib/segmented_array.hpp>
#include <neolib/indexitor.hpp>
#include <neogfx/app/i_clipboard.hpp>
#include <neogfx/gfx/text/glyph.hpp>
#include <neogfx/gui/window/context_menu.hpp>
#include "scrollable_widget.hpp"
#include "i_text_document.hpp"
#include "cursor.hpp"

namespace neogfx
{
    class text_edit : public scrollable_widget, public i_clipboard_sink, public i_text_document
    {
        typedef text_edit self_type;
    public:
        define_event(TextFilter, text_filter, const std::string&, bool&)
        define_event(TextChanged, text_changed)
        define_event(DefaultStyleChanged, default_style_changed)
    private:
        typedef self_type property_context_type;
    public:
        enum type_e
        {
            SingleLine,
            MultiLine
        };
        typedef std::optional<std::string> optional_password_mask;
        class style
        {
        public:
            style();
            style(
                const optional_font& aFont,
                const colour_or_gradient& aTextColour = colour_or_gradient{},
                const colour_or_gradient& aBackgroundColour = colour_or_gradient{},
                const optional_text_effect& aTextEffect = optional_text_effect{});
            style(
                text_edit& aParent,
                const style& aOther);
        public:
            void add_ref() const;
            void release() const;
        public:
            const optional_font& font() const;
            const colour_or_gradient& glyph_colour() const;
            const colour_or_gradient& text_colour() const;
            const colour_or_gradient& background_colour() const;
            const optional_text_effect& text_effect() const;
            void set_font(const optional_font& aFont = optional_font{});
            void set_glyph_colour(const colour_or_gradient& aColour = colour_or_gradient{});
            void set_text_colour(const colour_or_gradient& aColour = colour_or_gradient{});
            void set_background_colour(const colour_or_gradient& aColour = colour_or_gradient{});
            void set_text_effect(const optional_text_effect& aEffect = optional_text_effect{});
        public:
            style& merge(const style& aOverridingStyle);
        public:
            bool operator==(const style& aRhs) const;
            bool operator!=(const style& aRhs) const;
            bool operator<(const style& aRhs) const;
        private:
            text_edit* iParent;
            mutable uint32_t iUseCount;
            optional_font iFont;
            colour_or_gradient iGlyphColour;
            colour_or_gradient iTextColour;
            colour_or_gradient iBackgroundColour;
            optional_text_effect iTextEffect;
        };
        typedef std::set<style> style_list;
        class column_info
        {
        public:
            column_info() :
                iDelimiter{ U'\t' }
            {
            }

        public:
            char32_t delimiter() const { return iDelimiter; }
            void set_delimiter(char32_t aDelimiter) { iDelimiter = aDelimiter; }
            const optional_dimension& min_width() const { return iMinWidth; }
            void set_min_width(const optional_dimension& aMinWidth) { iMinWidth = aMinWidth; }
            const optional_dimension& max_width() const { return iMaxWidth; }
            void set_max_width(const optional_dimension& aMaxWidth) { iMaxWidth = aMaxWidth; }
            const neogfx::margins& margins() const { return iMargins; }
            void set_margins(const neogfx::margins& aMargins) { iMargins = aMargins; }
            const std::optional<text_edit::style>& style() const { return iStyle; }
            void set_style(const std::optional<text_edit::style>& aStyle) { iStyle = aStyle; }

        public:
            bool operator==(const column_info& aRhs) const
            {
                return std::tie(iDelimiter, iMinWidth, iMaxWidth, iMargins, iStyle) == std::tie(aRhs.iDelimiter, aRhs.iMinWidth, aRhs.iMaxWidth, aRhs.iMargins, aRhs.iStyle);
            }
            bool operator!=(const column_info& aRhs) const
            {
                return !(*this == aRhs);
            }

        private:
            char32_t iDelimiter;
            optional_dimension iMinWidth;
            optional_dimension iMaxWidth;
            neogfx::margins iMargins;
            std::optional<text_edit::style> iStyle;
        };
    private:
        class multiple_text_changes;
        struct unknown_node {};
        template <typename Node = unknown_node>
        class tag
        {
        private:
            static constexpr std::size_t SMALL_OPTIMIZATION_FONT_COUNT = 4;
        private:
            typedef neolib::variant<style_list::const_iterator, nullptr_t> tag_style;
        public:
            struct tag_data
            {
                tag_style style;
                bool operator==(const tag_data& rhs) const
                {
                    return style == rhs.style;
                }
            };
        public:
            template <typename Node2>
            struct rebind { typedef tag<Node2> type; };
        private:
            typedef Node node_type;
        public:
            tag(const tag_data& aContents) :
                iNode(nullptr), iContents(aContents)
            {
                if (std::holds_alternative<style_list::const_iterator>(style()))
                    static_variant_cast<style_list::const_iterator>(style())->add_ref();
            }
            template <typename Node2>
            tag(node_type& aNode, const tag<Node2>& aTag) : 
                iNode(&aNode), iContents(aTag.iContents)
            {
                if (std::holds_alternative<style_list::const_iterator>(style()))
                    static_variant_cast<style_list::const_iterator>(style())->add_ref();
            }
            tag(const tag& aOther) : 
                iNode(aOther.iNode), iContents(aOther.iContents)
            {
                if (std::holds_alternative<style_list::const_iterator>(style()))
                    static_variant_cast<style_list::const_iterator>(style())->add_ref();
            }
            ~tag()
            {
                if (std::holds_alternative<style_list::const_iterator>(style()))
                    static_variant_cast<style_list::const_iterator>(style())->release();
            }
        public:
            bool operator==(const tag& aOther) const
            {
                return contents() == aOther.contents();
            }
            bool operator!=(const tag& aOther) const
            {
                return !(*this == aOther);
            }
        public:
            const tag_style& style() const
            {
                return contents().style;
            }
        private:
            const tag_data& contents() const
            {
                return iContents;
            }
            tag_data& contents()
            {
                return iContents;
            }
        private:
            node_type* iNode;
            tag_data iContents;
        };
        typedef neolib::tag_array<tag<>, char32_t, 16, 256> document_text;
        class paragraph_positioned_glyph : public glyph
        {
        public:
            using glyph::glyph;
            paragraph_positioned_glyph(double aX) : x(aX)
            {
            }
            paragraph_positioned_glyph(const glyph& aOther) : glyph(aOther), x(0.0)
            {
            }
        public:
            paragraph_positioned_glyph& operator=(const glyph& aOther)
            {
                glyph::operator=(aOther);
                x = 0.0;
                return *this;
            }
        public:
            bool operator<(const paragraph_positioned_glyph& aOther) const
            {
                return x < aOther.x;
            }
        public:
            double x = 0.0;
        };
        typedef neolib::segmented_array<paragraph_positioned_glyph, 256> glyph_container_type;
        class document_glyphs : public glyph_font_cache, public glyph_container_type
        {
        public:
            using glyph_container_type::glyph_container_type;
        public:
            void clear()
            {
                glyph_font_cache::clear();
                glyph_container_type::clear();
            }
        };
        class glyph_paragraph;
        class glyph_paragraph_index
        {
        public:
            glyph_paragraph_index() :
                iCharacters(0), iGlyphs(0)
            {
            }
            glyph_paragraph_index(document_text::size_type aCharacters, document_glyphs::size_type aGlyphs) :
                iCharacters(aCharacters), iGlyphs(aGlyphs)
            {
            }
        public:
            document_text::size_type characters() const { return iCharacters; }
            document_glyphs::size_type glyphs() const { return iGlyphs; }
        public:
            bool operator==(const glyph_paragraph_index& aRhs) const { return iCharacters == aRhs.iCharacters; }
            bool operator!=(const glyph_paragraph_index& aRhs) const { return !(*this == aRhs);    }
            bool operator<(const glyph_paragraph_index& aRhs) const { return iCharacters < aRhs.iCharacters; }
            bool operator>(const glyph_paragraph_index& aRhs) const { return aRhs < *this; }
            bool operator<=(const glyph_paragraph_index& aRhs) const { return iCharacters <= aRhs.iCharacters; }
            bool operator>=(const glyph_paragraph_index& aRhs) const { return aRhs <= *this; }
            glyph_paragraph_index operator+(const glyph_paragraph_index& aRhs) const { glyph_paragraph_index result = *this; result += aRhs; return result; }
            glyph_paragraph_index operator-(const glyph_paragraph_index& aRhs) const { glyph_paragraph_index result = *this; result -= aRhs; return result; }
            glyph_paragraph_index& operator+=(const glyph_paragraph_index& aRhs) { iCharacters += aRhs.iCharacters; iGlyphs += aRhs.iGlyphs; return *this; };
            glyph_paragraph_index& operator-=(const glyph_paragraph_index& aRhs) { iCharacters -= aRhs.iCharacters; iGlyphs -= aRhs.iGlyphs; return *this; };
        private:
            document_text::size_type iCharacters;
            document_glyphs::size_type iGlyphs;
        };
        typedef neolib::indexitor<
            glyph_paragraph, 
            glyph_paragraph_index, 
            boost::fast_pool_allocator<std::pair<glyph_paragraph, const glyph_paragraph_index>, boost::default_user_allocator_new_delete, boost::details::pool::null_mutex>> glyph_paragraphs;
        class glyph_paragraph
        {
        public:
            typedef std::map<document_glyphs::size_type, dimension, std::less<document_glyphs::size_type>, boost::fast_pool_allocator<std::pair<const document_glyphs::size_type, dimension>>> height_list;
        public:
            glyph_paragraph(text_edit& aParent) :
                iParent{&aParent}, iSelf{}
            {
            }
            glyph_paragraph() :
                iParent{nullptr}, iSelf{}
            {
            }
            ~glyph_paragraph()
            {
            }
        public:
            void set_self(glyph_paragraphs::const_iterator aSelf)
            {
                iSelf = aSelf;
            }
            glyph_paragraph& operator=(const glyph_paragraph& aOther)
            {
                iParent = aOther.iParent;
                iSelf = aOther.iSelf;
                iHeights = aOther.iHeights;
                return *this;
            }
        public:
            text_edit& parent() const
            {
                return *iParent;
            }
            document_text::size_type text_start_index() const
            {
                return parent().iGlyphParagraphs.foreign_index(iSelf).characters();
            }
            document_text::const_iterator text_start() const
            {
                return parent().iText.begin() + text_start_index();
            }
            document_text::iterator text_start()
            {
                return parent().iText.begin() + text_start_index();
            }
            document_text::size_type text_end_index() const
            {
                return (parent().iGlyphParagraphs.foreign_index(iSelf) + iSelf->second + parent().iGlyphParagraphs.skip_after(iSelf)).characters();
            }
            document_text::const_iterator text_end() const
            {
                return parent().iText.begin() + text_end_index();
            }
            document_text::iterator text_end()
            {
                return parent().iText.begin() + text_end_index();
            }
            document_glyphs::size_type start_index() const
            {
                return parent().iGlyphParagraphs.foreign_index(iSelf).glyphs();
            }
            document_glyphs::const_iterator start() const
            {
                return parent().iGlyphs.begin() + start_index();
            }
            document_glyphs::iterator start()
            {
                return parent().iGlyphs.begin() + start_index();
            }
            document_glyphs::size_type end_index() const
            {
                return (parent().iGlyphParagraphs.foreign_index(iSelf) + iSelf->second + parent().iGlyphParagraphs.skip_after(iSelf)).glyphs();
            }
            document_glyphs::const_iterator end() const
            {
                return parent().iGlyphs.begin() + end_index();
            }
            document_glyphs::iterator end()
            {
                return parent().iGlyphs.begin() + end_index();
            }
            dimension height(document_glyphs::iterator aStart, document_glyphs::iterator aEnd) const
            {
                if (iHeights.empty())
                {
                    dimension previousHeight = 0.0;
                    auto textStartIndex = text_start_index();
                    auto glyphsStartIndex = start_index();
                    auto glyphsEndIndex = end_index();
                    auto iterGlyph = start();
                    for (auto i = glyphsStartIndex; i != glyphsEndIndex; ++i)
                    {
                        const auto& glyph = *(iterGlyph++);
                        const auto& tag = parent().iText.tag(parent().iText.begin() + textStartIndex + glyph.source().first);
                        const auto& style = std::holds_alternative<style_list::const_iterator>(tag.style()) ? *static_variant_cast<style_list::const_iterator>(tag.style()) : parent().default_style();
                        auto& glyphFont = style.font() != std::nullopt ? *style.font() : parent().font();
                        dimension cy = glyph.extents(glyphFont).cy;
                        if (i == glyphsStartIndex || cy != previousHeight)
                        {
                            iHeights[i] = cy;
                            previousHeight = cy;
                        }
                    }
                    iHeights[end_index()] = 0.0;
                }
                dimension result = 0.0;
                auto start = iHeights.lower_bound(aStart - parent().iGlyphs.begin());
                if (start != iHeights.begin() && aStart < parent().iGlyphs.begin() + start->first)
                    --start;
                auto stop = iHeights.lower_bound(aEnd - parent().iGlyphs.begin());
                if (start == stop && stop != iHeights.end())
                    ++stop;
                for (auto i = start; i != stop; ++i)
                    result = std::max(result, (*i).second);
                return result;
            }
        private:
            text_edit* iParent;
            glyph_paragraphs::const_iterator iSelf;
            mutable height_list iHeights;
        };
        struct glyph_line
        {
            std::pair<glyph_paragraphs::size_type, glyph_paragraphs::const_iterator> paragraph;
            std::pair<document_glyphs::size_type, document_glyphs::const_iterator> lineStart;
            std::pair<document_glyphs::size_type, document_glyphs::const_iterator> lineEnd;
            coordinate ypos;
            size extents;
        };
        typedef std::vector<glyph_line> glyph_lines;
        class glyph_column : public column_info
        {
        public:
            glyph_column() :
                iWidth(0.0)
            {
            }
        public:
            const glyph_lines& lines() const { return iLines; }
            glyph_lines& lines() { return iLines; }
            dimension width() const { return iWidth; }
            void set_width(dimension aWidth) { iWidth = aWidth; }
        private:
            glyph_lines iLines;
            dimension iWidth;
        };
        typedef std::vector<glyph_column> glyph_columns;
    public:
        typedef document_text::size_type position_type;
    public:
        struct bad_column_index : std::logic_error { bad_column_index() : std::logic_error("neogfx::text_edit::bad_column_index") {} }; 
        // text_edit
    public:
        text_edit(type_e aType = MultiLine, frame_style aFrameStyle = frame_style::SolidFrame);
        text_edit(i_widget& aParent, type_e aType = MultiLine, frame_style aFrameStyle = frame_style::SolidFrame);
        text_edit(i_layout& aLayout, type_e aType = MultiLine, frame_style aFrameStyle = frame_style::SolidFrame);
        ~text_edit();
        // scrollable_widget
    public:
        void moved() override;
        void resized() override;
    public:
        size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
        size maximum_size(const optional_size& aAvailableSpace = optional_size()) const override;
        neogfx::margins margins() const override;
   public:
        void paint(i_graphics_context& aGraphicsContext) const override;
    public:
        const neogfx::font& font() const override;
    public:
        void focus_gained(focus_reason aFocusReason) override;
        void focus_lost(focus_reason aFocusReason) override;
    public:
        void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_released(mouse_button aButton, const point& aPosition) override;
        void mouse_moved(const point& aPosition) override;
        void mouse_entered(const point& aPosition) override;
        void mouse_left() override;
        neogfx::mouse_cursor mouse_cursor() const override;
    public:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
        bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
        bool text_input(const std::string& aText) override;
    public:
        neogfx::scrolling_disposition scrolling_disposition() const override;
        using scrollable_widget::update_scrollbar_visibility;
        void update_scrollbar_visibility(usv_stage_e aStage) override;
    public:
        colour frame_colour() const override;
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
        void delete_selected(i_clipboard& aClipboard) override;
        void select_all(i_clipboard& aClipboard) override;
        // i_text_document
    public:
        void move_cursor(cursor::move_operation_e aMoveOperation, bool aMoveAnchor = true) override;
    public:
        std::string plain_text() const override;
        bool set_plain_text(const std::string& aPlainText) override;
        std::string rich_text(rich_text_format aFormat = rich_text_format::Html) const override;
        bool set_rich_text(const std::string& aRichText, rich_text_format aFormat = rich_text_format::Html) override;
    public:
        void paste_plain_text() override;
        void paste_rich_text(rich_text_format aFormat = rich_text_format::Html) override;
        // text_edit
    public:
        bool read_only() const;
        void set_read_only(bool aReadOnly = true);
        bool word_wrap() const;
        void set_word_wrap(bool aWordWrap = true);
        bool password() const;
        const std::string& password_mask() const;
        void set_password(bool aPassword, const std::string& aMask = "\xE2\x97\x8F");
        neogfx::alignment alignment() const;
        void set_alignment(neogfx::alignment aAlignment);
        const style& default_style() const;
        void set_default_style(const style& aDefaultStyle, bool aPersist = false);
        colour default_text_colour() const;
    public:
        void clear();
        const std::string& text() const;
        std::size_t set_text(const std::string& aText);
        std::size_t set_text(const std::string& aText, const style& aStyle);
        std::size_t insert_text(const std::string& aText, bool aMoveCursor = false);
        std::size_t insert_text(const std::string& aText, const style& aStyle, bool aMoveCursor = false);
        void delete_text(position_type aStart, position_type aEnd);
        std::size_t columns() const;
        void set_columns(std::size_t aColumnCount);
        void remove_columns();
        const column_info& column(std::size_t aColumnIndex) const;
        void set_column(std::size_t aColumnIndex, const column_info& aColumn);
        const style& column_style(std::size_t aColumnIndex) const;
        const style& column_style(const column_info& aColumn) const;
    public:
        const neogfx::size_hint& size_hint() const;
        void set_size_hint(const neogfx::size_hint& aSizeHint);
        dimension tab_stops() const;
        void set_tab_stop_hint(const std::string& aTabStopHint = "0000");
        void set_tab_stops(const optional_dimension& aTabStops);
    public:
        position_type document_hit_test(const point& aPosition, bool aAdjustForScrollPosition = true) const;
        virtual bool same_word(position_type aTextPositionLeft, position_type aTextPositionRight) const;
        virtual std::pair<position_type, position_type> word_at(position_type aTextPosition) const;
    public:
        neogfx::cursor& cursor() const;
        void set_cursor_position(const point& aPosition, bool aMoveAnchor = true, bool aEnableDragger = false);
    protected:
        std::size_t column_index(const column_info& aColumn) const;
        rect column_rect(std::size_t aColumnIndex, bool aIncludeMargins = false) const;
        std::size_t column_hit_test(const point& aPosition, bool aAdjustForScrollPosition = true) const;
    private:
        struct position_info
        {
            document_glyphs::const_iterator glyph;
            glyph_columns::const_iterator column;
            glyph_lines::const_iterator line;
            document_glyphs::const_iterator lineStart;
            document_glyphs::const_iterator lineEnd;
            point pos;
        };
        position_info glyph_position(position_type aGlyphPosition, bool aForCursor = false) const;
        position_type cursor_glyph_position() const;
        position_type cursor_glyph_anchor() const;
        void set_cursor_glyph_position(position_type aGlyphPosition, bool aMoveAnchor = true);
    private:
        void init();
        std::size_t do_insert_text(const std::string& aText, const style& aStyle, bool aMoveCursor, bool aClearFirst);
        void delete_any_selection();
        void notify_text_changed();
        std::pair<position_type, position_type> related_glyphs(position_type aGlyphPosition) const;
        bool same_paragraph(position_type aFirstGlyphPos, position_type aSecondGlyphPos) const;
        glyph_paragraphs::const_iterator character_to_paragraph(position_type aCharacterPos) const;
        glyph_paragraphs::const_iterator glyph_to_paragraph(position_type aGlyphPos) const;
        document_glyphs::const_iterator to_glyph(document_text::const_iterator aWhere) const;
        std::pair<document_text::size_type, document_text::size_type> from_glyph(document_glyphs::const_iterator aWhere) const;
        void refresh_paragraph(document_text::const_iterator aWhere, ptrdiff_t aDelta);
        void refresh_columns();
        void refresh_lines();
        void animate();
        void update_cursor();
        void make_cursor_visible(bool aForcePreviewScroll = false);
        style glyph_style(document_glyphs::const_iterator aGlyph, const glyph_column& aColumn) const;
        void draw_glyphs(const i_graphics_context& aGraphicsContext, const point& aPosition, const glyph_column& aColumn, glyph_lines::const_iterator aLine) const;
        void draw_cursor(const i_graphics_context& aGraphicsContext) const;
        rect cursor_rect() const;
        static std::pair<document_glyphs::iterator, document_glyphs::iterator> word_break(document_glyphs::iterator aBegin, document_glyphs::iterator aFrom, document_glyphs::iterator aEnd);
    private:
        sink iSink;
        type_e iType;
        style iDefaultStyle;
        bool iPersistDefaultStyle;
        font_info iDefaultFont;
        mutable neogfx::cursor iCursor;
        style_list iStyles;
        std::u32string iNormalizedTextBuffer;
        document_text iPreviousText;
        document_text iText;
        mutable std::optional<std::string> iUtf8TextCache;
        document_glyphs iGlyphs;
        glyph_paragraphs iGlyphParagraphs;
        glyph_columns iGlyphColumns;
        size iTextExtents;
        uint64_t iCursorAnimationStartTime;
        typedef std::pair<position_type, position_type> find_span;
        typedef std::map<
            find_span,
            glyph_paragraphs::const_iterator,
            std::less<find_span>,
            boost::fast_pool_allocator<std::pair<const find_span, glyph_paragraphs::const_iterator>, boost::default_user_allocator_new_delete, boost::details::pool::null_mutex>> find_in_paragraph_cache;
        mutable find_in_paragraph_cache iCharacterToParagraphCache;
        mutable std::optional<find_in_paragraph_cache::iterator> iCharacterToParagraphCacheLastAccess;
        mutable find_in_paragraph_cache iGlyphToParagraphCache;
        mutable std::optional<find_in_paragraph_cache::iterator> iGlyphToParagraphCacheLastAccess;
        neogfx::size_hint iSizeHint;
        mutable std::optional<std::pair<neogfx::font, size>> iHintedSize;
        optional_dimension iTabStops;
        std::string iTabStopHint;
        basic_point<std::optional<dimension>> iCursorHint;
        mutable std::optional<std::pair<neogfx::font, dimension>> iCalculatedTabStops;
        neolib::callback_timer iAnimator;
        std::optional<neolib::callback_timer> iDragger;
        std::unique_ptr<context_menu> iMenu;
        uint32_t iSuppressTextChangedNotification;
        uint32_t iWantedToNotfiyTextChanged;
        bool iOutOfMemory;
    public:
        define_property(property_category::other, bool, ReadOnly, read_only, false)
        define_property(property_category::other, bool, WordWrap, word_wrap, iType == MultiLine)
        define_property(property_category::other, bool, Password, password, false)
        define_property(property_category::other, std::string, PasswordMask, password_mask)
        define_property(property_category::other, neogfx::alignment, Alignment, alignment, neogfx::alignment::Left | neogfx::alignment::Top)
    };
}