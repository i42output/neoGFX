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
#include <neogfx/app/i_clipboard.hpp>
#include <neogfx/gfx/text/glyph.hpp>
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
		public:
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
	private:
		struct unknown_node {};
		template <typename Node = unknown_node>
		class tag
		{
		public:
			typedef neolib::variant<style_list::const_iterator> contents_type;
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
		typedef neolib::tag_array<tag<>, char, 32, 256> document_text;
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
		class glyph_paragraph
		{
		public:
			typedef std::map<document_glyphs::size_type, dimension, std::less<document_glyphs::size_type>, boost::fast_pool_allocator<std::pair<const document_glyphs::size_type, dimension>>> height_list;
		public:
			glyph_paragraph(text_edit& aParent, document_text::size_type aTextStart, document_text::size_type aTextEnd, document_glyphs::size_type aStart, document_glyphs::size_type aEnd) :
				iParent(&aParent), iTextStart(aTextStart), iTextEnd(aTextEnd), iStart(aStart), iEnd(aEnd)
			{
			}
			glyph_paragraph(document_text::size_type aTextStart, document_text::size_type aTextEnd, document_glyphs::size_type aStart, document_glyphs::size_type aEnd) :
				iParent(nullptr), iTextStart(aTextStart), iTextEnd(aTextEnd), iStart(aStart), iEnd(aEnd)
			{
			}
		public:
			glyph_paragraph& operator=(const glyph_paragraph& aOther)
			{
				iParent = aOther.iParent;
				iTextStart = aOther.iTextStart;
				iTextEnd = aOther.iTextEnd;
				iStart = aOther.iStart;
				iEnd = aOther.iEnd;
				iHeights = aOther.iHeights;
				return *this;
			}
		public:
			document_text::size_type text_start_index() const
			{
				return iTextStart;
			}
			document_text::const_iterator text_start() const
			{
				return iParent->iText.begin() + iTextStart;
			}
			document_text::iterator text_start()
			{
				return iParent->iText.begin() + iTextStart;
			}
			document_text::size_type text_end_index() const
			{
				return iTextEnd;
			}
			document_text::const_iterator text_end() const
			{
				return iParent->iText.begin() + iTextEnd;
			}
			document_text::iterator text_end()
			{
				return iParent->iText.begin() + iTextEnd;
			}
			document_glyphs::size_type start_index() const
			{
				return iStart;
			}
			document_glyphs::const_iterator start() const
			{
				return iParent->iGlyphs.begin() + iStart;
			}
			document_glyphs::iterator start()
			{
				return iParent->iGlyphs.begin() + iStart;
			}
			document_glyphs::size_type end_index() const
			{
				return iEnd;
			}
			document_glyphs::const_iterator end() const
			{
				return iParent->iGlyphs.begin() + iEnd;
			}
			document_glyphs::iterator end()
			{
				return iParent->iGlyphs.begin() + iEnd;
			}
			dimension height(document_glyphs::iterator aStart, document_glyphs::iterator aEnd) const
			{
				if (iHeights.empty())
				{
					dimension previousHeight = 0.0;
					auto iterGlyph = start();
					for (auto i = iStart; i != iEnd; ++i)
					{
						const auto& glyph = *(iterGlyph++);
						const auto& tagContents = iParent->iText.tag(iParent->iText.begin() + iTextStart + glyph.source().first).contents();
						const auto& style = *static_variant_cast<style_list::const_iterator>(tagContents);
						auto& glyphFont = style.font() != boost::none ? *style.font() : iParent->font();
						dimension cy = !glyph.use_fallback() ? glyphFont.height() : glyphFont.fallback().height();
						if (!style.text_outline_colour().empty())
							cy += 2.0;
						if (i == iStart || cy != previousHeight)
						{
							iHeights[i] = cy;
							previousHeight = cy;
						}
					}
					iHeights[iEnd] = 0.0;
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
			document_text::size_type iTextStart;
			document_text::size_type iTextEnd;
			document_glyphs::size_type iStart;
			document_glyphs::size_type iEnd;
			mutable height_list iHeights;
		};
		typedef neolib::segmented_array<glyph_paragraph> glyph_paragraphs;
		struct glyph_line
		{
			document_glyphs::const_iterator start;
			document_glyphs::const_iterator end;
			coordinate y;
			size extents;
		};
		typedef neolib::segmented_array<glyph_line> glyph_lines;
	public:
		typedef document_text::size_type position_type;
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
			glyph_lines::const_iterator line;
			point pos;
		};
		position_info position(position_type aPosition) const;
		position_type hit_test(const point& aPoint, bool aAdjustForScrollPosition = true) const;
		std::string text() const;
		std::size_t set_text(const std::string& aText);
		std::size_t set_text(const std::string& aText, const style& aStyle);
		std::size_t insert_text(const std::string& aText, bool aMoveCursor = false);
		std::size_t insert_text(const std::string& aText, const style& aStyle, bool aMoveCursor = false);
		void delete_text(position_type aStart, position_type aEnd);
	public:
		void set_hint(const std::string& aHint);
	private:
		void init();
		void delete_any_selection();
		document_glyphs::const_iterator to_glyph(document_text::const_iterator aWhere) const;
		std::pair<document_text::size_type, document_text::size_type> from_glyph(document_glyphs::const_iterator aWhere) const;
		void refresh_paragraph(document_text::const_iterator aWhere, ptrdiff_t aDelta);
		void refresh_lines();
		void animate();
		void update_cursor();
		void make_cursor_visible(bool aForcePreviewScroll = false);
		void draw_glyphs(const graphics_context& aGraphicsContext, const point& aPoint, glyph_lines::const_iterator aLine) const;
		void draw_cursor(const graphics_context& aGraphicsContext) const;
		std::pair<document_glyphs::iterator, document_glyphs::iterator> word_break(document_glyphs::iterator aBegin, document_glyphs::iterator aFrom, document_glyphs::iterator aEnd);
	private:
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
		std::string iNormalizedTextBuffer;
		document_text iText;
		document_glyphs iGlyphs;
		glyph_paragraphs iGlyphParagraphs;
		glyph_lines iGlyphLines;
		size iTextExtents;
		uint64_t iCursorAnimationStartTime;
		mutable const glyph_paragraph* iGlyphParagraphCache;
		std::string iHint;
		mutable boost::optional<std::pair<neogfx::font, size>> iHintedSize;
		neolib::callback_timer iAnimator;
		boost::optional<neolib::callback_timer> iDragger;
	};
}