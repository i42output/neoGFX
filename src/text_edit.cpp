// text_edit.cpp
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

#include "neogfx.hpp"
#include "text_edit.hpp"
#include "app.hpp"

namespace neogfx
{
	text_edit::style::style() :
		iParent(nullptr),
		iUseCount(0)
	{
	}
		
	text_edit::style::style(
		const optional_font& aFont,
		const colour_type& aTextColour,
		const colour_type& aBackgroundColour) :
		iParent(nullptr),
		iUseCount(0),
		iFont(aFont),
		iTextColour(aTextColour),
		iBackgroundColour(aBackgroundColour)
	{
	}

	text_edit::style::style(
		text_edit& aParent,
		const style& aOther) : 
		iParent(&aParent), 
		iUseCount(0),
		iFont(aOther.iFont),
		iTextColour(aOther.iTextColour),
		iBackgroundColour(aOther.iBackgroundColour)
	{
	}

	void text_edit::style::add_ref() const
	{
		++iUseCount;
	}

	void text_edit::style::release() const
	{
		if (--iUseCount == 0 && iParent)
			iParent->iStyles.erase(iParent->iStyles.find(*this));
	}

	const optional_font& text_edit::style::font() const
	{
		return iFont;
	}

	const text_edit::style::colour_type& text_edit::style::text_colour() const
	{
		return iTextColour;
	}

	const text_edit::style::colour_type& text_edit::style::background_colour() const
	{
		return iBackgroundColour;
	}

	bool text_edit::style::operator<(const style& aRhs) const
	{
		return std::tie(iFont, iTextColour, iBackgroundColour) < std::tie(aRhs.iFont, aRhs.iTextColour, aRhs.iBackgroundColour);
	}

	text_edit::text_edit() : 
		scrollable_widget(), 
		iAlignment(neogfx::alignment::Left|neogfx::alignment::Top), 
		iAnimator(app::instance(), [this](neolib::callback_timer&)
		{
			iAnimator.again();
			animate();
		}, 100)
	{
		init();
	}

	text_edit::text_edit(i_widget& aParent) :
		scrollable_widget(aParent), 
		iAlignment(neogfx::alignment::Left | neogfx::alignment::Top), 
		iAnimator(app::instance(), [this](neolib::callback_timer&)
		{
			iAnimator.again();
			animate();
		}, 100)
	{
		init();
	}

	text_edit::text_edit(i_layout& aLayout) :
		scrollable_widget(aLayout), 
		iAlignment(neogfx::alignment::Left | neogfx::alignment::Top), 
		iAnimator(app::instance(), [this](neolib::callback_timer&)
		{
			iAnimator.again();
			animate();
		}, 100)
	{
		init();
	}

	text_edit::~text_edit()
	{
	}

	size text_edit::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return scrollable_widget::minimum_size(aAvailableSpace);
		scoped_units su(*this, UnitsPixels);
		auto result = scrollable_widget::minimum_size() + size{ font().height() } + margins().size();
		return convert_units(*this, su.saved_units(), result);
	}

	void text_edit::paint(graphics_context& aGraphicsContext) const
	{
		/* simple (naive) implementation just to get things moving... */
		typedef std::pair<document_glyphs::const_iterator, document_glyphs::const_iterator> line_t;
		typedef std::vector<line_t> lines_t;
		lines_t lines;
		std::array<glyph, 2> delimeters = { glyph(text_direction::Whitespace, '\r'), glyph(text_direction::Whitespace, '\n') };
		neolib::tokens(iGlyphs.begin(), iGlyphs.end(), delimeters.begin(), delimeters.end(), lines, 0, false);
		point pos{ margins().left, margins().top };
		for (lines_t::const_iterator i = lines.begin(); i != lines.end(); ++i)
		{
			const auto& line = *i;
			document_glyphs::const_iterator next = line.first;
			document_glyphs::const_iterator lineStart = next;
			document_glyphs::const_iterator lineEnd = line.second;
			dimension lineWidth = 0;
			while (next != line.second)
			{
				bool gotLine = false;
				if (lineWidth + next->extents().cx > client_rect(false).width())
				{
					std::pair<document_glyphs::const_iterator, document_glyphs::const_iterator> wordBreak = word_break(lineStart, next);
					lineWidth -= extents(font(), wordBreak.first, next).cx;
					lineEnd = wordBreak.first;
					next = wordBreak.second;
					if (lineEnd == next)
					{
						while (lineEnd != line.second && (lineEnd + 1)->source() == wordBreak.first->source())
							++lineEnd;
						next = lineEnd;
					}
					gotLine = true;
				}
				else
				{
					lineWidth += next->extents().cx;
					++next;
				}
				if (gotLine || next == line.second)
				{
					point linePos = pos;
					if (iAlignment == alignment::Left && glyph_text_direction(lineStart, next) == text_direction::RTL ||
						iAlignment == alignment::Right && glyph_text_direction(lineStart, next) == text_direction::LTR)
						linePos.x += client_rect(false).width() - aGraphicsContext.from_device_units(size{lineWidth, 0}).cx;
					else if (iAlignment == alignment::Centre)
						linePos.x += std::ceil((client_rect().width() - aGraphicsContext.from_device_units(size{lineWidth, 0}).cx) / 2);
					draw_glyph_text(aGraphicsContext, linePos, lineStart, lineEnd);
					pos.y += extents(font(), lineStart, lineEnd).cy;
					lineStart = next;
					lineEnd = line.second;
					lineWidth = 0;
				}
			}
			if (line.first == line.second)
				pos.y += font().height();
		}
	}

	bool text_edit::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
		bool handled = true;
		switch (aScanCode)
		{
		case ScanCode_RETURN:
			insert_text("\n");
			cursor().set_position(cursor().position() + 1);
			break;
		case ScanCode_UP:
			if ((aKeyModifiers & KeyModifier_CTRL) != KeyModifier_NONE)
				scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
			else
				move_cursor(cursor::Up);
			break;
		case ScanCode_DOWN:
			if ((aKeyModifiers & KeyModifier_CTRL) != KeyModifier_NONE)
				scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
			else
				move_cursor(cursor::Down);
			break;
		case ScanCode_LEFT:
			move_cursor((aKeyModifiers & KeyModifier_CTRL) != KeyModifier_NONE ? cursor::PreviousWord : cursor::Left);
			break;
		case ScanCode_RIGHT:
			move_cursor((aKeyModifiers & KeyModifier_CTRL) != KeyModifier_NONE ? cursor::NextWord : cursor::Right);
			break;
		case ScanCode_HOME:
			move_cursor((aKeyModifiers & KeyModifier_CTRL) != KeyModifier_NONE ? cursor::StartOfDocument : cursor::StartOfLine);
			break;
		case ScanCode_END:
			move_cursor((aKeyModifiers & KeyModifier_CTRL) != KeyModifier_NONE ? cursor::EndOfDocument : cursor::EndOfLine);
			break;
		case ScanCode_PAGEUP:
		case ScanCode_PAGEDOWN:
			{
				auto pos = position(cursor().position()) - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() };
				scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
				cursor().set_position(hit_test(pos));
			}
			break;
		default:
			handled = scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
			break;
		}
		return handled;
	}

	bool text_edit::key_released(scan_code_e, key_code_e, key_modifiers_e)
	{
		return false;
	}

	bool text_edit::text_input(const std::string& aText)
	{
		insert_text(aText);
		cursor().set_position(cursor().position() + aText.size());
		return true;
	}

	void text_edit::move_cursor(cursor::move_operation_e aMoveOperation) const
	{
		switch (aMoveOperation)
		{
		case cursor::StartOfDocument:
			break;
		case cursor::StartOfParagraph:
			break;
		case cursor::StartOfLine:
			break;
		case cursor::StartOfWord:
			break;
		case cursor::EndOfDocument:
			break;
		case cursor::EndOfParagraph:
			break;
		case cursor::EndOfLine:
			break;
		case cursor::EndOfWord:
			break;
		case cursor::PreviousParagraph:
			break;
		case cursor::PreviousLine:
			break;
		case cursor::PreviousWord:
			break;
		case cursor::PreviousCharacter:
			break;
		case cursor::NextParagraph:
			break;
		case cursor::NextLine:
			break;
		case cursor::NextWord:
			break;
		case cursor::NextCharacter:
			break;
		case cursor::Up:
			break;
		case cursor::Down:
			break;
		case cursor::Left:
			break;
		case cursor::Right:
			break;
		default:
			break;
		}
	}

	neogfx::alignment text_edit::alignment() const
	{
		return iAlignment;
	}

	void text_edit::set_alignment(neogfx::alignment aAlignment)
	{
		if (iAlignment != aAlignment)
		{
			iAlignment = aAlignment;
			update();
		}
	}

	const text_edit::style& text_edit::default_style() const
	{
		return iDefaultStyle;
	}

	void text_edit::set_default_style(const style& aDefaultStyle)
	{
		iDefaultStyle = aDefaultStyle;
		update();
	}

	colour text_edit::default_text_colour() const
	{
		if (default_style().text_colour().is<colour>())
			return static_variant_cast<const colour&>(default_style().text_colour());
		else if(default_style().text_colour().is<gradient>())
			return static_variant_cast<const gradient&>(default_style().text_colour()).at(0.0);
		optional_colour textColour;
		const i_widget* w = 0;
		do
		{
			if (w == 0)
				w = this;
			else
				w = &w->parent();
			if (w->has_background_colour())
			{
				textColour = w->background_colour().to_hsl().lightness() >= 0.5f ? colour::Black : colour::White;
				break;
			}
			else if (w->has_foreground_colour())
			{
				textColour = w->foreground_colour().to_hsl().lightness() >= 0.5f ? colour::Black : colour::White;
				break;
			}
		} while (w->has_parent());
		colour defaultTextColour = app::instance().current_style().text_colour();
		if (textColour == boost::none || textColour->similar_intensity(defaultTextColour))
			return defaultTextColour;
		else
			return *textColour;
	}

	neogfx::cursor& text_edit::cursor() const
	{
		return iCursor;
	}

	point text_edit::position(position_type aPosition) const
	{
		/* todo */
		(void)aPosition;
		return point{};
	}

	text_edit::position_type text_edit::hit_test(const point& aPoint) const
	{
		/* todo */
		(void)aPoint;
		return 0;
	}

	std::string text_edit::text() const
	{
		return std::string(iText.begin(), iText.end());
	}

	void text_edit::set_text(const std::string& aText)
	{
		set_text(aText, default_style());
	}

	void text_edit::set_text(const std::string& aText, const style& aStyle)
	{
		iCursor.set_position(0);
		iText.clear();
		iGlyphs.clear();
		insert_text(aText, aStyle);
	}

	void text_edit::insert_text(const std::string& aText)
	{
		insert_text(aText, default_style());
	}

	void text_edit::insert_text(const std::string& aText, const style& aStyle)
	{
		auto s = iStyles.insert(style(*this, aStyle)).first;
		refresh_paragraph(iText.insert(document_text::tag_type(static_cast<style_list::const_iterator>(s)), iText.begin() + cursor().position(), aText.begin(), aText.end()));
		update();
	}

	void text_edit::init()
	{
		set_focus_policy(focus_policy::ClickTabFocus);
		iCursor.position_changed([this]()
		{
			update();
		}, this);
		iCursor.anchor_changed([this]()
		{
			update();
		}, this);
	}

	void text_edit::refresh_paragraph(document_text::const_iterator aWhere)
	{
		/* simple (naive) implementation just to get things moving (so just refresh everything) ... */
		graphics_context gc(*this);
		auto gt = gc.to_glyph_text(text(), font());
		iGlyphs.clear();
		iGlyphs.insert(iGlyphs.begin(), gt.cbegin(), gt.cend());
	}

	void text_edit::animate()
	{
		update();
	}

	void text_edit::draw_glyph_text(const graphics_context& aGraphicsContext, const point& aPoint, document_glyphs::const_iterator aTextBegin, document_glyphs::const_iterator aTextEnd) const
	{
		{
			graphics_context::glyph_drawing gd(aGraphicsContext);
			point pos = aPoint;
			for (document_glyphs::const_iterator i = aTextBegin; i != aTextEnd; ++i)
			{
				const auto& glyph = *i;
				const auto& tagContents = iText.tag(iText.begin() + glyph.source().first).contents();
				const auto& style = *static_variant_cast<style_list::const_iterator>(tagContents);
				aGraphicsContext.draw_glyph(pos + glyph.offset(), glyph, 
					style.font() != boost::none ? *style.font() : font(),
					style.text_colour().is<colour>() ?
						static_variant_cast<const colour&>(style.text_colour()) : style.text_colour().is<gradient>() ? 
							static_variant_cast<const gradient&>(style.text_colour()).at((pos.x - margins().left) / client_rect(false).width()) : 
							default_text_colour());
				pos.x += glyph.extents().cx;
			}
		}
		point pos = aPoint;
		for (document_glyphs::const_iterator i = aTextBegin; i != aTextEnd; ++i)
		{
			const auto& glyph = *i;
			const auto& tagContents = iText.tag(iText.begin() + glyph.source().first).contents();
			const auto& style = *static_variant_cast<style_list::const_iterator>(tagContents);
			if (glyph.underline())
				aGraphicsContext.draw_glyph_underline(pos, glyph,
					style.font() != boost::none ? *style.font() : font(),
					style.text_colour().is<colour>() ?
						static_variant_cast<const colour&>(style.text_colour()) : style.text_colour().is<gradient>() ? 
							static_variant_cast<const gradient&>(style.text_colour()).at((pos.x - margins().left) / client_rect(false).width()) : 
							default_text_colour());
			pos.x += glyph.extents().cx;
		}
	}

	size text_edit::extents(const neogfx::font& aFont, document_glyphs::const_iterator aBegin, document_glyphs::const_iterator aEnd) const
	{
		neogfx::size result;
		bool usingNormal = false;
		bool usingFallback = false;
		for (document_glyphs::const_iterator i = aBegin; i != aEnd; ++i)
		{
			result.cx += i->extents().cx;
			if (!i->use_fallback())
				usingNormal = true;
			else
				usingFallback = true;
		}
		if (usingNormal || !usingFallback)
			result.cy = aFont.height();
		if (usingFallback)
			result.cy = std::max(result.cy, aFont.fallback().height());
		return neogfx::size(std::ceil(result.cx), std::ceil(result.cy));
	}

	std::pair<text_edit::document_glyphs::const_iterator, text_edit::document_glyphs::const_iterator> text_edit::word_break(document_glyphs::const_iterator aBegin, document_glyphs::const_iterator aFrom) const
	{
		std::pair<document_glyphs::const_iterator, document_glyphs::const_iterator> result(aFrom, aFrom);
		if (!aFrom->is_whitespace())
		{
			while (result.first != aBegin && !result.first->is_whitespace())
				--result.first;
			if (!result.first->is_whitespace())
			{
				result.first = aFrom;
				while (result.first != aBegin && (result.first - 1)->source() == aFrom->source())
					--result.first;
				result.second = result.first;
				return result;
			}
			result.second = result.first;
		}
		while (result.first != aBegin && (result.first - 1)->is_whitespace())
			--result.first;
		while (result.second->is_whitespace() && result.second != iGlyphs.end())
			++result.second;
		return result;
	}
}