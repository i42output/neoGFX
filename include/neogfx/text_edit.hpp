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

#include "neogfx.hpp"
#include <boost/pool/pool_alloc.hpp>
#include <neolib/tag_array.hpp>
#include <neolib/segmented_array.hpp>
#include "scrollable_widget.hpp"
#include "i_clipboard.hpp"
#include "i_document.hpp"
#include "glyph.hpp"
#include "cursor.hpp"

namespace neogfx
{
	class text_edit : public scrollable_widget, public i_clipboard_sink, public i_document
	{
	public:
		class style
		{
		public:
			typedef neolib::variant<colour, gradient> colour_type;
		public:
			style();
			style(
				const optional_font& aFont,
				const colour_type& aTextColour = colour_type(),
				const colour_type& aBackgroundColour = colour_type());
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
		public:
			bool operator<(const style& aRhs) const;
		private:
			text_edit* iParent;
			mutable uint32_t iUseCount;
			optional_font iFont;
			colour_type iTextColour;
			colour_type iBackgroundColour;
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
			typedef std::map<document_glyphs::iterator, dimension, std::less<document_glyphs::iterator>, boost::fast_pool_allocator<std::pair<const document_glyphs::iterator, dimension>>> height_list;
		public:
			glyph_paragraph(document_glyphs::iterator aStart, document_glyphs::iterator aEnd) : iStart(aStart), iEnd(aEnd)
			{
			}
		public:
			document_glyphs::iterator start() const
			{
				return iStart;
			}
			document_glyphs::iterator end() const
			{
				return iEnd;
			}
			dimension height(text_edit& aParent, document_glyphs::iterator aStart, document_glyphs::iterator aEnd) const
			{
				if (iHeights.empty())
				{
					dimension previousHeight = 0.0;
					for (auto i = iStart; i != iEnd; ++i)
					{
						const auto& glyph = *i;
						const auto& tagContents = aParent.iText.tag(aParent.iText.begin() + glyph.source().first).contents();
						const auto& style = *static_variant_cast<style_list::const_iterator>(tagContents);
						auto& glyphFont = style.font() != boost::none ? *style.font() : aParent.font();
						dimension cy = !glyph.use_fallback() ? glyphFont.height() : glyphFont.fallback().height();
						if (i == iStart || cy != previousHeight)
						{
							iHeights[i] = cy;
							previousHeight = cy;
						}
					}
					iHeights[iEnd] = 0.0;
				}
				dimension result = 0.0;
				auto start = iHeights.lower_bound(aStart);
				if (start != iHeights.begin() && aStart < start->first)
					--start;
				auto stop = iHeights.lower_bound(aEnd);
				for (auto i = start; i != stop; ++i)
					result = std::max(result, (*i).second);
				return result;
			}
		private:
			document_glyphs::iterator iStart;
			document_glyphs::iterator iEnd;
			mutable height_list iHeights;
		};
		typedef neolib::segmented_array<glyph_paragraph> glyph_paragraphs;
		struct glyph_line
		{
			document_glyphs::iterator start;
			document_glyphs::iterator end;
			coordinate y;
			size extents;
		};
		typedef neolib::segmented_array<glyph_line> glyph_lines;
	public:
		typedef document_text::size_type position_type;
	public:
		text_edit();
		text_edit(i_widget& aParent);
		text_edit(i_layout& aLayout);
		~text_edit();
	public:
		virtual void resized();
	public:
		virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
	public:
		virtual void focus_gained();
		virtual void focus_lost();
	public:
		virtual bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual bool text_input(const std::string& aText);
	public:
		virtual bool can_cut() const;
		virtual bool can_copy() const;
		virtual bool can_paste() const;
		virtual void cut(i_clipboard& aClipboard);
		virtual void copy(i_clipboard& aClipboard);
		virtual void paste(i_clipboard& aClipboard);
	public:
		virtual void move_cursor(cursor::move_operation_e aMoveOperation) const;
	public:
		bool read_only() const;
		void set_read_only(bool aReadOnly = true);
		neogfx::alignment alignment() const;
		void set_alignment(neogfx::alignment aAlignment);
		const style& default_style() const;
		void set_default_style(const style& aDefaultStyle);
		colour default_text_colour() const;
	public:
		neogfx::cursor& cursor() const;
		point position(position_type aPosition) const;
		position_type hit_test(const point& aPoint) const;
		std::string text() const;
		void set_text(const std::string& aText);
		void set_text(const std::string& aText, const style& aStyle);
		void insert_text(const std::string& aText);
		void insert_text(const std::string& aText, const style& aStyle);
	private:
		void init();
		void refresh_paragraph(document_text::const_iterator aWhere);
		void refresh_lines();
		void animate();
		void draw_glyphs(const graphics_context& aGraphicsContext, const point& aPoint, document_glyphs::const_iterator aTextBegin, document_glyphs::const_iterator aTextEnd) const;
		std::pair<document_glyphs::iterator, document_glyphs::iterator> word_break(document_glyphs::iterator aBegin, document_glyphs::iterator aFrom, document_glyphs::iterator aEnd);
	private:
		bool iReadOnly;
		neogfx::alignment iAlignment;
		style iDefaultStyle;
		mutable neogfx::cursor iCursor;
		document_text iText;
		document_glyphs iGlyphs;
		glyph_paragraphs iGlyphParagraphs;
		glyph_lines iGlyphLines;
		style_list iStyles;
		neolib::callback_timer iAnimator;
	};
}