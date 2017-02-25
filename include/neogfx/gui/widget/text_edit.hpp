// text_edit.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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
#include "i_document.hpp"
#include "cursor.hpp"

namespace neogfx
{
	class text_edit : public scrollable_widget, public i_clipboard_sink, public i_document
	{
	public:
		event<> text_changed;
	public:
		enum type_e
		{
			SingleLine,
			MultiLine
		};
		typedef boost::optional<std::string> optional_password_mask;
		class style
		{
		public:
			typedef neolib::variant<colour, gradient> colour_type;
		public:
			style();
			style(
				const optional_font& aFont,
				const colour_type& aTextColour = colour_type(),
				const colour_type& aBackgroundColour = colour_type(),
				const colour_type& aTextOutlineColour = colour_type());
			style(
				text_edit& aParent,
				const style& aOther);
		public:
			void add_ref() const;
			void release() const;
		public:
			const optional_font& font() const;
			const colour_type& text_colour() const;
			const colour_type& background_colour() const;
			const colour_type& text_outline_colour() const;
			void set_font(const optional_font& aFont = optional_font{});
			void set_text_colour(const colour_type& aColour = colour_type{});
			void set_background_colour(const colour_type& aColour = colour_type{});
			void set_text_outline_colour(const colour_type& aColour = colour_type{});
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
			colour_type iTextColour;
			colour_type iBackgroundColour;
			colour_type iTextOutlineColour;
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
			const text_edit::style& style() const { return iStyle; }
			void set_style(const text_edit::style& aStyle) { iStyle = aStyle; }

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
			text_edit::style iStyle;
		};
	private:
		struct unknown_node {};
		template <typename Node = unknown_node>
		class tag
		{
		public:
			typedef neolib::variant<style_list::const_iterator, nullptr_t> contents_type;
			template <typename Node2>
			struct rebind { typedef tag<Node2> type; };
		private:
			typedef Node node_type;
		public:
			tag(const contents_type& aContents) :
				iNode(nullptr), iContents(aContents)
			{
				if (iContents.is<style_list::const_iterator>())
					static_variant_cast<style_list::const_iterator>(iContents)->add_ref();
			}
			template <typename Node2>
			tag(node_type& aNode, const tag<Node2>& aTag) : 
				iNode(&aNode), iContents(aTag.iContents)
			{
				if (iContents.is<style_list::const_iterator>())
					static_variant_cast<style_list::const_iterator>(iContents)->add_ref();
			}
			tag(const tag& aOther) : 
				iNode(aOther.iNode), iContents(aOther.iContents)
			{
				if (iContents.is<style_list::const_iterator>())
					static_variant_cast<style_list::const_iterator>(iContents)->add_ref();
			}
			~tag()
			{
				if (iContents.is<style_list::const_iterator>())
					static_variant_cast<style_list::const_iterator>(iContents)->release();
			}
		public:
			bool operator==(const tag& aOther) const
			{
				return iContents == aOther.iContents;
			}
			bool operator!=(const tag& aOther) const
			{
				return !(*this == aOther);
			}
		public:
			const contents_type& contents() const
			{
				return iContents;
			}
		private:
			node_type* iNode;
			contents_type iContents;
		};
		typedef neolib::tag_array<tag<>, char32_t, 32, 256> document_text;
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
		typedef neolib::segmented_array<paragraph_positioned_glyph, 256> document_glyphs;
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
			bool operator!=(const glyph_paragraph_index& aRhs) const { return !(*this == aRhs);	}
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
			document_text::size_type text_start_index() const
			{
				return iParent->iGlyphParagraphs.foreign_index(iSelf).characters();
			}
			document_text::const_iterator text_start() const
			{
				return iParent->iText.begin() + text_start_index();
			}
			document_text::iterator text_start()
			{
				return iParent->iText.begin() + text_start_index();
			}
			document_text::size_type text_end_index() const
			{
				return (iParent->iGlyphParagraphs.foreign_index(iSelf) + iSelf->second + iParent->iGlyphParagraphs.skip_after(iSelf)).characters();
			}
			document_text::const_iterator text_end() const
			{
				return iParent->iText.begin() + text_end_index();
			}
			document_text::iterator text_end()
			{
				return iParent->iText.begin() + text_end_index();
			}
			document_glyphs::size_type start_index() const
			{
				return iParent->iGlyphParagraphs.foreign_index(iSelf).glyphs();
			}
			document_glyphs::const_iterator start() const
			{
				return iParent->iGlyphs.begin() + start_index();
			}
			document_glyphs::iterator start()
			{
				return iParent->iGlyphs.begin() + start_index();
			}
			document_glyphs::size_type end_index() const
			{
				return (iParent->iGlyphParagraphs.foreign_index(iSelf) + iSelf->second + iParent->iGlyphParagraphs.skip_after(iSelf)).glyphs();
			}
			document_glyphs::const_iterator end() const
			{
				return iParent->iGlyphs.begin() + end_index();
			}
			document_glyphs::iterator end()
			{
				return iParent->iGlyphs.begin() + end_index();
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
						const auto& tagContents = iParent->iText.tag(iParent->iText.begin() + textStartIndex + glyph.source().first).contents();
						const auto& style = tagContents.is<style_list::const_iterator>() ? *static_variant_cast<style_list::const_iterator>(tagContents) : iParent->default_style();
						auto& glyphFont = style.font() != boost::none ? *style.font() : iParent->font();
						dimension cy = !glyph.use_fallback() ? glyphFont.height() : glyph.fallback_font(glyphFont).fallback().height();
						if (!style.text_outline_colour().empty())
							cy += 2.0;
						if (i == glyphsStartIndex || cy != previousHeight)
						{
							iHeights[i] = cy;
							previousHeight = cy;
						}
					}
					iHeights[end_index()] = 0.0;
				}
				dimension result = 0.0;
				auto start = iHeights.lower_bound(aStart - iParent->iGlyphs.begin());
				if (start != iHeights.begin() && aStart < iParent->iGlyphs.begin() + start->first)
					--start;
				auto stop = iHeights.lower_bound(aEnd - iParent->iGlyphs.begin());
				for (auto i = start; i != stop; ++i)
					result = std::max(result, (*i).second);
				return result;
			}
		private:
			text_edit* iParent;
			glyph_paragraphs::const_iterator iSelf;
			mutable height_list iHeights;
		};
		class glyph_line_index
		{
		public:
			glyph_line_index() :
				iGlyphs(0), iHeight(0.0)
			{
			}
			glyph_line_index(document_glyphs::size_type aGlyphs, coordinate aHeight) :
				iGlyphs(aGlyphs), iHeight(aHeight)
			{
			}
		public:
			document_glyphs::size_type glyphs() const { return iGlyphs; }
			coordinate height() const { return iHeight; }
		public:
			bool operator==(const glyph_line_index& aRhs) const { return iGlyphs == aRhs.iGlyphs; }
			bool operator!=(const glyph_line_index& aRhs) const { return !(*this == aRhs); }
			bool operator<(const glyph_line_index& aRhs) const { return iGlyphs < aRhs.iGlyphs; }
			bool operator>(const glyph_line_index& aRhs) const { return aRhs < *this; }
			bool operator<=(const glyph_line_index& aRhs) const { return iGlyphs <= aRhs.iGlyphs; }
			bool operator>=(const glyph_line_index& aRhs) const { return aRhs <= *this; }
			glyph_line_index operator+(const glyph_line_index& aRhs) const { glyph_line_index result = *this; result += aRhs; return result; }
			glyph_line_index operator-(const glyph_line_index& aRhs) const { glyph_line_index result = *this; result -= aRhs; return result; }
			glyph_line_index& operator+=(const glyph_line_index& aRhs) { iGlyphs += aRhs.iGlyphs; iHeight += aRhs.iHeight; return *this; }
			glyph_line_index& operator-=(const glyph_line_index& aRhs) { iGlyphs -= aRhs.iGlyphs; iHeight -= aRhs.iHeight; return *this; }
		private:
			document_glyphs::size_type iGlyphs;
			coordinate iHeight;
		};
		struct glyph_line
		{
			size extents;
		};
		typedef neolib::indexitor<glyph_line, glyph_line_index, boost::fast_pool_allocator<std::pair<glyph_line, const glyph_line_index>, boost::default_user_allocator_new_delete, boost::details::pool::null_mutex>> glyph_lines;
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
	public:
		text_edit(type_e aType = MultiLine);
		text_edit(i_widget& aParent, type_e aType = MultiLine);
		text_edit(i_layout& aLayout, type_e aType = MultiLine);
		~text_edit();
	public:
		virtual void resized();
	public:
		virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
		virtual size maximum_size(const optional_size& aAvailableSpace = optional_size()) const;
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
	public:
		virtual const neogfx::font& font() const;
	public:
		virtual void focus_gained();
		virtual void focus_lost();
	public:
		virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
		virtual void mouse_button_released(mouse_button aButton, const point& aPosition);
		virtual void mouse_moved(const point& aPosition);
		virtual void mouse_entered();
		virtual void mouse_left();
		virtual neogfx::mouse_cursor mouse_cursor() const;
	public:
		virtual bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual bool text_input(const std::string& aText);
	public:
		virtual child_widget_scrolling_disposition_e scrolling_disposition() const;
		using scrollable_widget::update_scrollbar_visibility;
		virtual void update_scrollbar_visibility(usv_stage_e aStage);
	protected:
		virtual colour frame_colour() const;
	public:
		virtual bool can_cut() const;
		virtual bool can_copy() const;
		virtual bool can_paste() const;
		virtual bool can_delete_selected() const;
		virtual bool can_select_all() const;
		virtual void cut(i_clipboard& aClipboard);
		virtual void copy(i_clipboard& aClipboard);
		virtual void paste(i_clipboard& aClipboard);
		virtual void delete_selected(i_clipboard& aClipboard);
		virtual void select_all(i_clipboard& aClipboard);
	public:
		virtual void move_cursor(cursor::move_operation_e aMoveOperation, bool aMoveAnchor = true);
	public:
		bool read_only() const;
		void set_read_only(bool aReadOnly = true);
		bool word_wrap() const;
		void set_word_wrap(bool aWordWrap = true);
		bool password() const;
		void set_password(bool aPassword, const std::string& aMask = "\xE2\x97\x8F");
		neogfx::alignment alignment() const;
		void set_alignment(neogfx::alignment aAlignment);
		const style& default_style() const;
		void set_default_style(const style& aDefaultStyle);
		colour default_text_colour() const;
	public:
		neogfx::cursor& cursor() const;
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
		position_type hit_test(const point& aPoint, bool aAdjustForScrollPosition = true) const;
		std::string text() const;
		std::size_t set_text(const std::string& aText);
		std::size_t set_text(const std::string& aText, const style& aStyle);
		std::size_t insert_text(const std::string& aText, bool aMoveCursor = false);
		std::size_t insert_text(const std::string& aText, const style& aStyle, bool aMoveCursor = false);
		void delete_text(position_type aStart, position_type aEnd);
		std::size_t columns() const;
		void set_columns(std::size_t aColumnCount);
		void remove_columns();
		const column_info& column(std::size_t aColumnIndex);
		void set_column(std::size_t aColumnIndex, const column_info& aColumn);
	public:
		void set_hint(const std::string& aHint);
	private:
		void init();
		void delete_any_selection();
		std::pair<position_type, position_type> related_glyphs(position_type aGlyphPosition) const;
		bool same_paragraph(position_type aFirst, position_type aSecond) const;
		document_glyphs::const_iterator to_glyph(document_text::const_iterator aWhere) const;
		std::pair<document_text::size_type, document_text::size_type> from_glyph(document_glyphs::const_iterator aWhere) const;
		void refresh_paragraph(document_text::const_iterator aWhere, ptrdiff_t aDelta);
		void refresh_columns();
		void refresh_lines();
		void animate();
		void update_cursor();
		void make_cursor_visible(bool aForcePreviewScroll = false);
		style glyph_style(document_glyphs::const_iterator aGlyph, const glyph_column& aColumn) const;
		void draw_glyphs(const graphics_context& aGraphicsContext, const point& aPoint, const glyph_column& aColumn, glyph_lines::const_iterator aLine) const;
		void draw_cursor(const graphics_context& aGraphicsContext) const;
		static std::pair<document_glyphs::iterator, document_glyphs::iterator> word_break(document_glyphs::iterator aBegin, document_glyphs::iterator aFrom, document_glyphs::iterator aEnd);
	private:
		sink iSink;
		type_e iType;
		bool iReadOnly;
		bool iWordWrap;
		bool iPassword;
		std::string iPasswordMask;
		neogfx::alignment iAlignment;
		style iDefaultStyle;
		font_info iDefaultFont;
		mutable neogfx::cursor iCursor;
		style_list iStyles;
		std::u32string iNormalizedTextBuffer;
		document_text iText;
		document_glyphs iGlyphs;
		glyph_paragraphs iGlyphParagraphs;
		glyph_columns iGlyphColumns;
		size iTextExtents;
		uint64_t iCursorAnimationStartTime;
		mutable const glyph_paragraph* iGlyphParagraphCache;
		std::string iHint;
		mutable boost::optional<std::pair<neogfx::font, size>> iHintedSize;
		neolib::callback_timer iAnimator;
		boost::optional<neolib::callback_timer> iDragger;
		std::unique_ptr<context_menu> iMenu;
	};
}