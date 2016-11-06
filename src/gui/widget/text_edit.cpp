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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/text_edit.hpp>
#include <neogfx/app/app.hpp>

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
		const colour_type& aBackgroundColour,
		const colour_type& aTextOutlineColour) :
		iParent(nullptr),
		iUseCount(0),
		iFont(aFont),
		iTextColour(aTextColour),
		iBackgroundColour(aBackgroundColour),
		iTextOutlineColour(aTextOutlineColour)
	{
	}

	text_edit::style::style(
		text_edit& aParent,
		const style& aOther) : 
		iParent(&aParent), 
		iUseCount(0),
		iFont(aOther.iFont),
		iTextColour(aOther.iTextColour),
		iBackgroundColour(aOther.iBackgroundColour),
		iTextOutlineColour(aOther.iTextOutlineColour)
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

	const text_edit::style::colour_type& text_edit::style::text_outline_colour() const
	{
		return iTextOutlineColour;
	}

	bool text_edit::style::operator<(const style& aRhs) const
	{
		return std::tie(iFont, iTextColour, iBackgroundColour) < std::tie(aRhs.iFont, aRhs.iTextColour, aRhs.iBackgroundColour);
	}

	text_edit::text_edit(type_e aType) :
		scrollable_widget(aType == MultiLine ? i_scrollbar::Normal : i_scrollbar::Invisible), 
		iType(aType),
		iReadOnly(false),
		iWordWrap(aType == MultiLine),
		iPassword(false),
		iAlignment(neogfx::alignment::Left|neogfx::alignment::Top),
		iAnimator(app::instance(), [this](neolib::callback_timer&)
		{
			iAnimator.again();
			animate();
		}, 40),
		iCursorAnimationStartTime(app::instance().program_elapsed_ms()),
		iGlyphParagraphCache(nullptr)
	{
		init();
	}

	text_edit::text_edit(i_widget& aParent, type_e aType) :
		scrollable_widget(aParent, aType == MultiLine ? i_scrollbar::Normal : i_scrollbar::Invisible),
		iType(aType),
		iReadOnly(false),
		iWordWrap(aType == MultiLine),
		iPassword(false),
		iAlignment(neogfx::alignment::Left | neogfx::alignment::Top),
		iAnimator(app::instance(), [this](neolib::callback_timer&)
		{
			iAnimator.again();
			animate();
		}, 40),
		iCursorAnimationStartTime(app::instance().program_elapsed_ms()),
		iGlyphParagraphCache(nullptr)
	{
		init();
	}

	text_edit::text_edit(i_layout& aLayout, type_e aType) :
		scrollable_widget(aLayout, aType == MultiLine ? i_scrollbar::Normal : i_scrollbar::Invisible),
		iType(aType),
		iReadOnly(false),
		iWordWrap(aType == MultiLine),
		iPassword(false),
		iAlignment(neogfx::alignment::Left | neogfx::alignment::Top),
		iAnimator(app::instance(), [this](neolib::callback_timer&)
		{
			iAnimator.again();
			animate();
		}, 40),
		iCursorAnimationStartTime(app::instance().program_elapsed_ms()),
		iGlyphParagraphCache(nullptr)
	{
		init();
	}

	text_edit::~text_edit()
	{
		if (app::instance().clipboard().sink_active() && &app::instance().clipboard().active_sink() == this)
			app::instance().clipboard().deactivate(*this);
	}

	void text_edit::resized()
	{
		scrollable_widget::resized();
	}

	size text_edit::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return scrollable_widget::minimum_size(aAvailableSpace);
		scoped_units su(*this, UnitsPixels);
		auto result = scrollable_widget::minimum_size(aAvailableSpace);
		if (iHint.empty())
			result += size{ font().height() };
		else
		{
			if (iHintedSize == boost::none || iHintedSize->first != font())
			{
				iHintedSize.emplace(font(), size{});
				graphics_context gc(*this);
				iHintedSize->second = gc.text_extent(iHint, font());
			}
			result += iHintedSize->second;
		}
		return convert_units(*this, su.saved_units(), result);
	}

	size text_edit::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (iType == MultiLine || has_maximum_size())
			return scrollable_widget::maximum_size(aAvailableSpace);
		scoped_units su(*this, UnitsPixels);
		auto result = scrollable_widget::maximum_size(aAvailableSpace);
		result.cy = minimum_size(aAvailableSpace).cy;
		return convert_units(*this, su.saved_units(), result);
	}

	void text_edit::paint(graphics_context& aGraphicsContext) const
	{
		scrollable_widget::paint(aGraphicsContext);
		auto line = iGlyphLines.find_by_foreign_index(glyph_line_index{ 0, vertical_scrollbar().position() },
			[](const glyph_line_index& left, const glyph_line_index& right) { return left.height() < right.height(); });
		if (line.first != iGlyphLines.begin() && line.second.height() > vertical_scrollbar().position())
			line = std::make_pair(line.first - 1, iGlyphLines.foreign_index(line.first - 1));
		auto y = line.second.height();
		auto startLineGlyph = line.second.glyphs();
		for (auto paintLine = line.first; paintLine != iGlyphLines.end(); y += (paintLine++)->first.extents.cy)
		{
			point linePos = client_rect(false).top_left() + point{-horizontal_scrollbar().position(), y - vertical_scrollbar().position()};
			if (linePos.y + paintLine->first.extents.cy < client_rect(false).top() || linePos.y + paintLine->first.extents.cy < update_rect().top())
				continue;
			if (linePos.y > client_rect(false).bottom() || linePos.y > update_rect().bottom())
				break;
			auto textDirection = glyph_text_direction(iGlyphs.begin() + startLineGlyph, iGlyphs.begin() + startLineGlyph + paintLine->second.glyphs());
			if (iAlignment == alignment::Left && textDirection == text_direction::RTL ||
				iAlignment == alignment::Right && textDirection == text_direction::LTR)
				linePos.x += iTextExtents.cx - aGraphicsContext.from_device_units(size{paintLine->first.extents.cx, 0}).cx;
			else if (iAlignment == alignment::Centre)
				linePos.x += std::ceil((iTextExtents.cx - aGraphicsContext.from_device_units(size{paintLine->first.extents.cx, 0}).cx) / 2);
			draw_glyphs(aGraphicsContext, linePos, paintLine);
			startLineGlyph += paintLine->second.glyphs();
		}
		if (has_focus())
			draw_cursor(aGraphicsContext);
	}

	const font& text_edit::font() const
	{
		return default_style().font() != boost::none ? *default_style().font() : scrollable_widget::font();
	}

	void text_edit::focus_gained()
	{
		scrollable_widget::focus_gained();
		app::instance().clipboard().activate(*this);
		iCursorAnimationStartTime = app::instance().program_elapsed_ms();
		if (iType == SingleLine)
		{
			cursor().set_anchor(0);
			cursor().set_position(iGlyphs.size(), false);
		}
	}

	void text_edit::focus_lost()
	{
		scrollable_widget::focus_lost();
		app::instance().clipboard().deactivate(*this);
		if (iType == SingleLine)
			cursor().set_position(iGlyphs.size());
	}

	void text_edit::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		scrollable_widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left && client_rect().contains(aPosition))
		{
			cursor().set_position(hit_test(aPosition), (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
			if (capturing())
			{
				iDragger.emplace(app::instance(), [this](neolib::callback_timer& aTimer)
				{
					aTimer.again();
					cursor().set_position(hit_test(surface().mouse_position() - origin()), false);
				}, 250);
			}
		}
	}

	void text_edit::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		scrollable_widget::mouse_button_released(aButton, aPosition);
		iDragger = boost::none;
	}

	void text_edit::mouse_moved(const point& aPosition)
	{
		scrollable_widget::mouse_moved(aPosition);
		if (iDragger != boost::none)
			cursor().set_position(hit_test(aPosition), false);
	}

	void text_edit::mouse_entered()
	{
	}

	void text_edit::mouse_left()
	{
	}

	neogfx::mouse_cursor text_edit::mouse_cursor() const
	{
		return client_rect(false).contains(surface().mouse_position() - origin()) || iDragger != boost::none ? mouse_system_cursor::Ibeam : scrollable_widget::mouse_cursor();
	}

	bool text_edit::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
		bool handled = true;
		switch (aScanCode)
		{
		case ScanCode_RETURN:
			if (iType == MultiLine)
			{
				delete_any_selection();
				insert_text("\n");
				cursor().set_position(cursor().position() + 1);
			}
			else
				handled = scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
			break;
		case ScanCode_BACKSPACE:
			if (cursor().position() == cursor().anchor())
			{
				if (cursor().position() > 0)
				{
					delete_text(cursor().position() - 1, cursor().position());
					if (cursor().position() > 0)
						cursor().set_position(cursor().position() - 1);
					make_cursor_visible(true);
				}
			}
			else
				delete_any_selection();
			break;
		case ScanCode_DELETE:
			if (cursor().position() == cursor().anchor())
			{
				if (cursor().position() < iGlyphs.size())
				{
					delete_text(cursor().position(), cursor().position() + 1);
					make_cursor_visible(true);
				}
			}
			else
				delete_any_selection();
			break;
		case ScanCode_UP:
			if (iType == MultiLine)
			{
				if ((aKeyModifiers & KeyModifier_CTRL) != KeyModifier_NONE)
					scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
				else
					move_cursor(cursor::Up, (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
			}
			else
				handled = false;
			break;
		case ScanCode_DOWN:
			if (iType == MultiLine)
			{
				if ((aKeyModifiers & KeyModifier_CTRL) != KeyModifier_NONE)
					scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
				else
					move_cursor(cursor::Down, (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
			}
			else
				handled = false;
			break;
		case ScanCode_LEFT:
			move_cursor((aKeyModifiers & KeyModifier_CTRL) != KeyModifier_NONE ? cursor::PreviousWord : cursor::Left, (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
			break;
		case ScanCode_RIGHT:
			move_cursor((aKeyModifiers & KeyModifier_CTRL) != KeyModifier_NONE ? cursor::NextWord : cursor::Right, (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
			break;
		case ScanCode_HOME:
			move_cursor((aKeyModifiers & KeyModifier_CTRL) != KeyModifier_NONE ? cursor::StartOfDocument : cursor::StartOfLine, (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
			break;
		case ScanCode_END:
			move_cursor((aKeyModifiers & KeyModifier_CTRL) != KeyModifier_NONE ? cursor::EndOfDocument : cursor::EndOfLine, (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
			break;
		case ScanCode_PAGEUP:
		case ScanCode_PAGEDOWN:
			{
				if (aScanCode == ScanCode_PAGEUP && vertical_scrollbar().position() == vertical_scrollbar().minimum())
					cursor().set_position(0, (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
				else if (aScanCode == ScanCode_PAGEDOWN && vertical_scrollbar().position() == vertical_scrollbar().maximum() - vertical_scrollbar().page())
					cursor().set_position(iGlyphs.size(), (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
				else
				{
					auto pos = point{ position(cursor().position()).pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } };
					scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
					cursor().set_position(hit_test(pos + client_rect(false).top_left()), (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
				}
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
		delete_any_selection();
		insert_text(aText, true);
		return true;
	}

	text_edit::child_widget_scrolling_disposition_e text_edit::scrolling_disposition() const
	{
		return DontScrollChildWidget;
	}

	void text_edit::update_scrollbar_visibility(usv_stage_e aStage)
	{
		switch (aStage)
		{
		case UsvStageInit:
			vertical_scrollbar().hide();
			horizontal_scrollbar().hide();
			refresh_lines();
			break;
		case UsvStageCheckVertical1:
		case UsvStageCheckVertical2:
			{
				i_scrollbar::value_type oldPosition = vertical_scrollbar().position();
				vertical_scrollbar().set_maximum(iGlyphLines.empty() ? 0.0 : iTextExtents.cy);
				vertical_scrollbar().set_step(font().height());
				vertical_scrollbar().set_page(client_rect(false).height());
				vertical_scrollbar().set_position(oldPosition);
				if (vertical_scrollbar().maximum() - vertical_scrollbar().page() > 0.0)
					vertical_scrollbar().show();
				else
					vertical_scrollbar().hide();
				scrollable_widget::update_scrollbar_visibility(aStage);
				refresh_lines();
			}
			break;
		case UsvStageCheckHorizontal:
			{
				i_scrollbar::value_type oldPosition = horizontal_scrollbar().position();
				horizontal_scrollbar().set_maximum(iGlyphLines.empty() || iTextExtents.cx <= client_rect(false).width() ? 0.0 : iTextExtents.cx);
				horizontal_scrollbar().set_step(font().height());
				horizontal_scrollbar().set_page(client_rect(false).width());
				horizontal_scrollbar().set_position(oldPosition);
				if (horizontal_scrollbar().maximum() - horizontal_scrollbar().page() > 0.0)
					horizontal_scrollbar().show();
				else
					horizontal_scrollbar().hide();
				scrollable_widget::update_scrollbar_visibility(aStage);
				refresh_lines();
			}
			break;
		case UsvStageDone:
			make_cursor_visible();
			break;
		default:
			break;
		}
	}

	colour text_edit::frame_colour() const
	{
		if (app::instance().current_style().colour().similar_intensity(background_colour(), 0.03125))
			return scrollable_widget::frame_colour();
		return app::instance().current_style().colour().mid(background_colour());
	}

	bool text_edit::can_cut() const
	{
		return !read_only() && !iText.empty() && iCursor.position() != iCursor.anchor();
	}

	bool text_edit::can_copy() const
	{
		return !iText.empty() && iCursor.position() != iCursor.anchor();
	}

	bool text_edit::can_paste() const
	{
		return !read_only();
	}

	bool text_edit::can_delete_selected() const
	{
		return !read_only() && !iText.empty();
	}

	bool text_edit::can_select_all() const
	{
		return !iText.empty();
	}

	void text_edit::cut(i_clipboard& aClipboard)
	{
		if (cursor().position() != cursor().anchor())
		{
			copy(aClipboard);
			delete_selected(aClipboard);
		}
	}

	void text_edit::copy(i_clipboard& aClipboard)
	{
		if (cursor().position() != cursor().anchor())
		{
			std::string selectedText;
			auto selectionStart = std::min(cursor().position(), cursor().anchor());
			auto selectionEnd = std::max(cursor().position(), cursor().anchor());
			auto start = from_glyph(iGlyphs.begin() + selectionStart).first;
			auto end = from_glyph(iGlyphs.begin() + selectionEnd).first;
			selectedText.assign(iText.begin() + start, iText.begin() + end);
			aClipboard.set_text(selectedText);
		}
	}

	void text_edit::paste(i_clipboard& aClipboard)
	{
		if (cursor().position() != cursor().anchor())
			delete_selected(aClipboard);
		auto len = insert_text(aClipboard.text());
		cursor().set_position(to_glyph(iText.begin() + from_glyph(iGlyphs.begin() + cursor().position()).first + len) - iGlyphs.begin());
	}

	void text_edit::delete_selected(i_clipboard&)
	{
		if (cursor().position() != cursor().anchor())
			delete_any_selection();
		else if(cursor().position() < iGlyphs.size())
			delete_text(cursor().position(), cursor().position() + 1);
	}

	void text_edit::select_all(i_clipboard&)
	{
		cursor().set_anchor(0);
		cursor().set_position(iGlyphs.size(), false);
	}

	void text_edit::move_cursor(cursor::move_operation_e aMoveOperation, bool aMoveAnchor)
	{
		if (iGlyphs.empty())
			return;
		const auto& currentPosition = position(iCursor.position());
		switch (aMoveOperation)
		{
		case cursor::StartOfDocument:
			iCursor.set_position(0, aMoveAnchor);
			break;
		case cursor::StartOfParagraph:
			break;
		case cursor::StartOfLine:
			if (currentPosition.lineStart != currentPosition.lineEnd)
				iCursor.set_position(currentPosition.lineStart - iGlyphs.begin(), aMoveAnchor);
			break;
		case cursor::StartOfWord:
			break;
		case cursor::EndOfDocument:
			iCursor.set_position(iGlyphs.size(), aMoveAnchor);
			break;
		case cursor::EndOfParagraph:
			break;
		case cursor::EndOfLine:
			if (currentPosition.lineStart != currentPosition.lineEnd)
				iCursor.set_position(currentPosition.lineEnd - iGlyphs.begin(), aMoveAnchor);
			break;
		case cursor::EndOfWord:
			break;
		case cursor::PreviousParagraph:
			break;
		case cursor::PreviousLine:
			break;
		case cursor::PreviousWord:
			/* todo: reverse for RTL */
			if (!iGlyphs.empty())
			{
				auto p = iCursor.position();
				if (p == iGlyphs.size())
					--p;
				while (p > 0 && iGlyphs[p].is_whitespace())
					--p;
				if (p > 0)
				{
					auto d = iGlyphs[p == iCursor.position() ? p - 1 : p].direction();
					while (p > 0 && iGlyphs[p - 1].direction() == d)
						--p;
					if (p > 0 && d == text_direction::Whitespace)
					{
						d = iGlyphs[p - 1].direction();
						while (p > 0 && iGlyphs[p - 1].direction() == d)
							--p;
					}
				}
				iCursor.set_position(p, aMoveAnchor);
			}
			break;
		case cursor::PreviousCharacter:
			/* todo: reverse for RTL */
			if (iCursor.position() > 0)
				iCursor.set_position(iCursor.position() - 1, aMoveAnchor);
			break;
		case cursor::NextParagraph:
			break;
		case cursor::NextLine:
			break;
		case cursor::NextWord:
			/* todo: reverse for RTL */
			if (!iGlyphs.empty())
			{
				auto p = iCursor.position();
				while (p < iGlyphs.size() && iGlyphs[p].is_whitespace())
					++p;
				if (p < iGlyphs.size() && p == iCursor.position())
				{
					auto d = iGlyphs[p].direction();
					while (p < iGlyphs.size() && iGlyphs[p].direction() == d)
						++p;
					while (p < iGlyphs.size() && iGlyphs[p].is_whitespace())
						++p;
				}
				iCursor.set_position(p, aMoveAnchor);
			}
			break;
		case cursor::NextCharacter:
			/* todo: reverse for RTL */
			if (iCursor.position() < iGlyphs.size())
				iCursor.set_position(iCursor.position() + 1, aMoveAnchor);
			break;
		case cursor::Up:
			if (currentPosition.line != iGlyphLines.begin())
				iCursor.set_position(hit_test(point{ currentPosition.pos.x, iGlyphLines.foreign_index(currentPosition.line - 1).height() }, false), aMoveAnchor);
			break;
		case cursor::Down:
			if (currentPosition.line != iGlyphLines.end())
			{
				if (currentPosition.line + 1 != iGlyphLines.end())
					iCursor.set_position(hit_test(point{ currentPosition.pos.x, iGlyphLines.foreign_index(currentPosition.line + 1).height() }, false), aMoveAnchor);
				else if (currentPosition.lineEnd != iGlyphs.end() && currentPosition.lineEnd->is_whitespace() && currentPosition.lineEnd->value() == '\n')
					iCursor.set_position(iGlyphs.size(), aMoveAnchor);
			}
			break;
		case cursor::Left:
			if (iCursor.position() > 0)
				iCursor.set_position(iCursor.position() - 1, aMoveAnchor);
			break;
		case cursor::Right:
			if (iCursor.position() < iGlyphs.size())
				iCursor.set_position(iCursor.position() + 1, aMoveAnchor);
			break;
		default:
			break;
		}
	}

	bool text_edit::read_only() const
	{
		return iReadOnly;
	}

	void text_edit::set_read_only(bool aReadOnly)
	{
		iReadOnly = aReadOnly;
		update();
	}

	bool text_edit::word_wrap() const
	{
		return iWordWrap;
	}
	
	void text_edit::set_word_wrap(bool aWordWrap)
	{
		if (iWordWrap != aWordWrap)
		{
			iWordWrap = aWordWrap;
			update_scrollbar_visibility();
		}
	}

	bool text_edit::password() const
	{
		return iPassword;
	}

	void text_edit::set_password(bool aPassword, const std::string& aMask)
	{
		if (iPassword != aPassword || iPasswordMask != aMask)
		{
			iPassword = aPassword;
			iPasswordMask = aMask;
			refresh_paragraph(iText.begin(), 0);
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
		neogfx::font oldFont = font();
		iDefaultStyle = aDefaultStyle;
		if (oldFont != font())
			refresh_paragraph(iText.begin(), 0);
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

	text_edit::position_info text_edit::position(position_type aPosition) const
	{
		auto line = iGlyphLines.find_by_foreign_index(glyph_line_index{ aPosition, 0.0 },
			[](const glyph_line_index& left, const glyph_line_index& right) { return left.glyphs() < right.glyphs(); });
		if (line.first != iGlyphLines.begin() && 
			(aPosition < static_cast<position_type>(line.second.glyphs()) || 
			(line.first == iGlyphLines.end() && (!iGlyphs.back().is_whitespace() || iGlyphs.back().value() != '\n'))))
			line = std::make_pair(line.first - 1, iGlyphLines.foreign_index(line.first - 1));
		if (line.first != iGlyphLines.end())
		{
			position_type lineStart = line.second.glyphs();
			position_type lineEnd = lineStart + line.first->second.glyphs();
			if (aPosition >= lineStart && aPosition <= lineEnd)
			{
				if (lineStart != lineEnd)
				{
					auto iterGlyph = iGlyphs.begin() + iCursor.position();
					const auto& glyph = iCursor.position() < lineEnd ? *iterGlyph : *(iterGlyph - 1);
					point linePos{ glyph.x - iGlyphs[line.second.glyphs()].x, line.second.height() };
					if (iCursor.position() == lineEnd)
						linePos.x += glyph.advance().cx;
					return position_info{ iterGlyph, line.first, iGlyphs.begin() + lineStart, iGlyphs.begin() + lineEnd, linePos };
				}
				else
					return position_info{ iGlyphs.begin() + line.second.glyphs(), line.first, iGlyphs.begin() + lineStart, iGlyphs.begin() + lineEnd, point{ 0.0, line.second.height() } };
			}
		}
		point pos;
		if (!iGlyphLines.empty())
		{
			pos.x = 0.0;
			pos.y = iGlyphLines.foreign_index(iGlyphLines.end()).height();
		}
		return position_info{ iGlyphs.end(), iGlyphLines.end(), iGlyphs.end(), iGlyphs.end(), pos };
	}

	text_edit::position_type text_edit::hit_test(const point& aPoint, bool aAdjustForScrollPosition) const
	{
		point adjusted = aAdjustForScrollPosition ?
			point{ aPoint - client_rect(false).top_left() } + point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } :
			aPoint;
		if (adjusted.x < 0.0)
			adjusted.x = 0.0;
		auto line = iGlyphLines.find_by_foreign_index(glyph_line_index{0, adjusted.y },
			[](const glyph_line_index& left, const glyph_line_index& right) { return left.height() < right.height(); });
		if (line.first != iGlyphLines.begin() && adjusted.y < line.second.height())
			line = std::make_pair(line.first - 1, iGlyphLines.foreign_index(line.first - 1));
		if (line.first == iGlyphLines.end())
			return iGlyphs.size();
		auto lineStartX = iGlyphs[line.second.glyphs()].x;
		for (auto gi = line.second.glyphs(); gi != line.second.glyphs() + line.first->second.glyphs(); ++gi)
		{
			auto& g = iGlyphs[gi];
			if (adjusted.x >= g.x - lineStartX && adjusted.x < g.x - lineStartX + g.advance().cx)
				return gi;
		}
		return line.second.glyphs() + line.first->second.glyphs();
	}

	std::string text_edit::text() const
	{
		return std::string(iText.begin(), iText.end());
	}

	std::size_t text_edit::set_text(const std::string& aText)
	{
		return set_text(aText, default_style());
	}

	std::size_t text_edit::set_text(const std::string& aText, const style& aStyle)
	{
		iCursor.set_position(0);
		iText.clear();
		iGlyphs.clear();
		return insert_text(aText, aStyle, true);
	}

	std::size_t text_edit::insert_text(const std::string& aText, bool aMoveCursor)
	{
		return insert_text(aText, default_style(), aMoveCursor);
	}

	std::size_t text_edit::insert_text(const std::string& aText, const style& aStyle, bool aMoveCursor)
	{
		if (iNormalizedTextBuffer.capacity() < aText.size())
			iNormalizedTextBuffer.reserve(aText.size());
		iNormalizedTextBuffer.clear();
		for (auto ch : aText)
			if (ch != '\r')
				iNormalizedTextBuffer.push_back(ch);
		auto eos = iNormalizedTextBuffer.size();
		if (iType == SingleLine)
		{
			auto eol = iNormalizedTextBuffer.find('\n');
			if (eol != std::string::npos)
				eos = eol;
		}
		auto s = iStyles.insert(style(*this, aStyle)).first;
		auto insertionPoint = iText.end();
		if (!iGlyphs.empty())
		{
			auto p = position(iCursor.position());
			if (p.glyph != iGlyphs.end())
			{
				if (p.glyph != p.lineEnd)
					insertionPoint = iText.begin() + from_glyph(p.glyph).first;
				else if (p.lineEnd != iGlyphs.end())
					insertionPoint = iText.begin() + from_glyph(p.lineEnd).first;
			}
		}
		insertionPoint = iText.insert(document_text::tag_type(static_cast<style_list::const_iterator>(s)), insertionPoint, iNormalizedTextBuffer.begin(), iNormalizedTextBuffer.begin() + eos);
		refresh_paragraph(insertionPoint, eos);
		update();
		// todo: move cursor left if RTL text
		if (aMoveCursor)
		{
			auto textPos = from_glyph(iGlyphs.begin() + cursor().position()).first + eos;
			cursor().set_position(to_glyph(iText.begin() + textPos) - iGlyphs.begin());
		}
		text_changed.trigger();
		return eos;
	}

	void text_edit::delete_text(position_type aStart, position_type aEnd)
	{
		if (aStart == aEnd)
			return;
		auto eraseBegin = iText.begin() + from_glyph(iGlyphs.begin() + aStart).first;
		auto eraseEnd = iText.begin() + from_glyph(iGlyphs.begin() + aEnd - 1).second;
		auto eraseAmount = eraseEnd - eraseBegin;
		refresh_paragraph(iText.erase(eraseBegin, eraseEnd), -eraseAmount);
		update();
		text_changed.trigger();
	}

	void text_edit::set_hint(const std::string& aHint)
	{
		if (iHint != aHint)
		{
			iHint = aHint;
			iHintedSize = boost::none;
			if (has_managing_layout())
				managing_layout().layout_items(true);
			update();
		}
	}

	void text_edit::init()
	{
		iDefaultFont = app::instance().current_style().font_info();
		iSink += app::instance().current_style_changed([this]()
		{
			if (iDefaultFont != app::instance().current_style().font_info())
			{
				iDefaultFont = app::instance().current_style().font_info();
				refresh_paragraph(iText.begin(), 0);
			}
		});
		iSink += app::instance().rendering_engine().subpixel_rendering_changed([this]()
		{
			refresh_paragraph(iText.begin(), 0);
		});
		set_focus_policy(focus_policy::ClickTabFocus);
		iCursor.set_width(2.0);
		iSink += iCursor.position_changed([this]()
		{
			iCursorAnimationStartTime = app::instance().program_elapsed_ms();
			make_cursor_visible();
			update();
		});
		iSink += iCursor.anchor_changed([this]()
		{
			update();
		});
		iSink += iCursor.appearance_changed([this]()
		{
			update();
		});
	}

	void text_edit::delete_any_selection()
	{
		if (cursor().position() != cursor().anchor())
		{
			delete_text(std::min(cursor().position(), cursor().anchor()), std::max(cursor().position(), cursor().anchor()));
			cursor().set_position(std::min(cursor().position(), cursor().anchor()));
		}
	}

	text_edit::document_glyphs::const_iterator text_edit::to_glyph(document_text::const_iterator aWhere) const
	{
		std::size_t textIndex = static_cast<std::size_t>(aWhere - iText.begin());
		if (iGlyphParagraphCache == nullptr || aWhere < iGlyphParagraphCache->text_start() || aWhere >= iGlyphParagraphCache->text_end())
		{
			auto paragraph = iGlyphParagraphs.find_by_foreign_index(glyph_paragraph_index{ textIndex, 0 }, 
				[](const glyph_paragraph_index& aLhs, const glyph_paragraph_index& aRhs) { return aLhs.characters() < aRhs.characters(); });
			if (paragraph.first == iGlyphParagraphs.end() && paragraph.first != iGlyphParagraphs.begin() && textIndex <= iGlyphParagraphs.foreign_index(paragraph.first - 1).characters())
				paragraph = std::make_pair(paragraph.first - 1, iGlyphParagraphs.foreign_index(paragraph.first - 1));
			if (paragraph.first != iGlyphParagraphs.end())
				iGlyphParagraphCache = &paragraph.first->first;
			else
				iGlyphParagraphCache = nullptr;
		}
		if (iGlyphParagraphCache == nullptr)
			return iGlyphs.end();
		for (auto i = iGlyphParagraphCache->start(); i != iGlyphParagraphCache->end(); ++i)
		{
			auto const& g = *i;
			if (textIndex >= g.source().first + iGlyphParagraphCache->text_start_index() && textIndex < g.source().second + iGlyphParagraphCache->text_start_index())
				return i;
		}
		return iGlyphParagraphCache->end();
	}

	std::pair<text_edit::document_text::size_type, text_edit::document_text::size_type> text_edit::from_glyph(document_glyphs::const_iterator aWhere) const
	{
		if (aWhere == iGlyphs.end())
		{
			if (iGlyphs.empty())
				return std::make_pair(0, 0);
			else
				return std::make_pair(iGlyphParagraphs.back().first.text_start_index() + (aWhere - 1)->source().second, iGlyphParagraphs.back().first.text_start_index() + (aWhere - 1)->source().second);
		}
		if (iGlyphParagraphCache != nullptr && aWhere >= iGlyphParagraphCache->start() && aWhere < iGlyphParagraphCache->end())
		{
			auto textStart = iGlyphParagraphCache->text_start_index();
			auto sourceStart = aWhere->source().first;
			auto sourceEnd = aWhere->source().second;
			return std::make_pair(textStart + sourceStart, textStart + sourceEnd);
		}
		auto paragraph = iGlyphParagraphs.find_by_foreign_index(glyph_paragraph_index{0, static_cast<std::size_t>(aWhere - iGlyphs.begin())}, [](const glyph_paragraph_index& aLhs, const glyph_paragraph_index& aRhs) { return aLhs.glyphs() < aRhs.glyphs();});
		if (paragraph.first == iGlyphParagraphs.end() && paragraph.first != iGlyphParagraphs.begin() && aWhere <= (paragraph.first - 1)->first.end())
			--paragraph.first;
		if (paragraph.first != iGlyphParagraphs.end())
		{
			if (paragraph.first->first.start() > aWhere)
				--paragraph.first;
			iGlyphParagraphCache = &paragraph.first->first;
			auto textStart = paragraph.first->first.text_start_index();
			auto sourceStart = aWhere->source().first;
			auto sourceEnd = aWhere->source().second;
			return std::make_pair(textStart + sourceStart, textStart + sourceEnd);
		}
		else
			iGlyphParagraphCache = nullptr;
		return std::make_pair(iText.size(), iText.size());
	}

	void text_edit::refresh_paragraph(document_text::const_iterator aWhere, ptrdiff_t aDelta)
	{
		/* simple (naive) implementation just to get things moving (so just refresh everything) ... */
		(void)aWhere;
		graphics_context gc(*this);
		iGlyphs.clear();
		iGlyphParagraphs.clear();
		iGlyphParagraphCache = nullptr;
		std::string paragraphBuffer;
		auto paragraphStart = iText.begin();
		for (auto ch = iText.begin(); ch != iText.end(); ++ch)
		{
			if (*ch == '\n' || ch == iText.end() - 1)
			{
				paragraphBuffer.assign(paragraphStart, ch + 1);
				auto fs = [this, paragraphStart](std::string::size_type aSourceIndex)
				{
					const auto& tagContents = iText.tag(paragraphStart + aSourceIndex).contents(); // todo: cache iterator to increase throughput
					const auto& style = *static_variant_cast<style_list::const_iterator>(tagContents);
					auto f = style.font() != boost::none ? *style.font() : font();
					if (iPassword)
						f.set_password(true, iPasswordMask.empty() ? "\xE2\x97\x8F" : iPasswordMask);
					return f;
				};
				auto gt = gc.to_glyph_text(paragraphBuffer.begin(), paragraphBuffer.end(), fs);
				auto paragraphGlyphs = iGlyphs.insert(iGlyphs.end(), gt.cbegin(), gt.cend());
				auto newParagraph = iGlyphParagraphs.insert(iGlyphParagraphs.end(),
					std::make_pair(
						glyph_paragraph{*this},
						glyph_paragraph_index{
							static_cast<std::size_t>((ch + 1) - iText.begin()) - static_cast<std::size_t>(paragraphStart - iText.begin()),
							iGlyphs.size() - (*ch == '\n' ? 1 : 0) - static_cast<std::size_t>(paragraphGlyphs - iGlyphs.begin())}),
					glyph_paragraphs::skip_type{glyph_paragraph_index{}, glyph_paragraph_index{0, *ch == '\n' ? 1u : 0u}}); // todo: only valid for naive implementation
				newParagraph->first.set_self(newParagraph);
				paragraphStart = ch + 1;
			}
		}
		for (auto p = iGlyphParagraphs.begin(); p != iGlyphParagraphs.end(); ++p)
		{
			auto& paragraph = *p;
			if (paragraph.first.start() == paragraph.first.end())
				continue;
			coordinate x = 0.0;
			for (auto g = paragraph.first.start(); g != paragraph.first.end(); ++g)
			{
				g->x = x;
				x += g->advance().cx;
			}
		}
		update_scrollbar_visibility();
	}

	void text_edit::refresh_lines()
	{
		/* simple (naive) implementation just to get things moving... */
		iGlyphLines.clear();
		point pos{};
		dimension availableWidth = client_rect(false).width();
		iTextExtents = size{};
		for (auto p = iGlyphParagraphs.begin(); p != iGlyphParagraphs.end(); ++p)
		{
			auto& paragraph = *p;
			auto paragraphStart = paragraph.first.start();
			auto paragraphEnd = paragraph.first.end();
			if (paragraphStart == paragraphEnd || (paragraphStart->is_whitespace() && paragraphStart->value() == '\n'))
			{
				const auto& glyph = *paragraphStart;
				const auto& tagContents = iText.tag(iText.begin() + paragraph.first.text_start_index() + glyph.source().first).contents();
				const auto& style = *static_variant_cast<style_list::const_iterator>(tagContents);
				auto& glyphFont = style.font() != boost::none ? *style.font() : font();
				iGlyphLines.push_back(
					std::make_pair(glyph_line{ size{ 0.0, glyphFont.height() } }, glyph_line_index{0, glyphFont.height()}), 
					std::make_pair(glyph_line_index{}, glyph_line_index{paragraphStart != paragraphEnd ? 1u : 0u, 0.0}));
				pos.y += iGlyphLines.back().first.extents.cy;
			}
			else if (iWordWrap)
			{
				document_glyphs::iterator next = paragraph.first.start();
				document_glyphs::iterator lineStart = next;
				document_glyphs::iterator lineEnd = paragraphEnd;
				coordinate offset = 0.0;
				while (next != paragraphEnd)
				{
					auto split = std::lower_bound(next, paragraphEnd, paragraph_positioned_glyph{ offset + availableWidth });
					if (split != next && (split != paragraphEnd || (split - 1)->x + (split - 1)->advance().cx >= offset + availableWidth))
						--split;
					if (split == next)
						++split;
					if (split != paragraphEnd)
					{
						std::pair<document_glyphs::iterator, document_glyphs::iterator> wordBreak = word_break(lineStart, split, paragraphEnd);
						lineEnd = wordBreak.first;
						next = wordBreak.second;
						if (wordBreak.first == wordBreak.second)
						{
							while (lineEnd != lineStart && (lineEnd - 1)->source() == wordBreak.first->source())
								--lineEnd;
							next = lineEnd;
						}
					}
					else
						next = paragraphEnd;
					bool skipNewline = false;
					if (lineStart != lineEnd && (lineEnd - 1)->is_whitespace() && (lineEnd - 1)->value() == '\n')
					{
						skipNewline = true;
						--lineEnd;
					}
					dimension x = (split != iGlyphs.end() ? split->x : (lineStart != lineEnd ? iGlyphs.back().x + iGlyphs.back().advance().cx : 0.0));
					auto height = paragraph.first.height(lineStart, lineEnd);
					auto line = iGlyphLines.insert(iGlyphLines.end(), 
						std::make_pair(glyph_line{size{x - offset, height}}, glyph_line_index{static_cast<std::size_t>(lineEnd - lineStart), height}), 
						glyph_lines::skip_type{glyph_line_index{}, glyph_line_index{ skipNewline ? 1u : 0u, 0.0 }});
					pos.y += height;
					iTextExtents.cx = std::max(iTextExtents.cx, iGlyphLines.back().first.extents.cx);
					lineStart = next;
					if (lineStart != paragraphEnd)
						offset = lineStart->x;
					lineEnd = paragraphEnd;
				}
			}
			else
			{
				auto height = paragraph.first.height(paragraphStart, paragraphEnd);
				iGlyphLines.push_back(std::make_pair(
					glyph_line{size{(paragraphEnd - 1)->x + (paragraphEnd - 1)->advance().cx, height}},
					glyph_line_index{static_cast<std::size_t>(paragraphEnd - paragraphStart), height}));
				pos.y += iGlyphLines.back().first.extents.cy;
				iTextExtents.cx = std::max(iTextExtents.cx, iGlyphLines.back().first.extents.cx);
			}
		}
		if (!iGlyphs.empty() && iGlyphs.back().is_whitespace() && iGlyphs.back().value() == '\n')
			pos.y += font().height();
		iTextExtents.cy = pos.y;
	}

	void text_edit::animate()
	{
		update_cursor();
	}

	void text_edit::update_cursor()
	{
		auto cursorPos = position(iCursor.position());
		dimension glyphHeight = 0.0;
		dimension lineHeight = 0.0;
		if (cursorPos.glyph != iGlyphs.end() && cursorPos.lineStart != cursorPos.lineEnd)
		{
			auto iterGlyph = cursorPos.glyph < cursorPos.lineEnd ? cursorPos.glyph : cursorPos.glyph - 1;
			const auto& glyph = *iterGlyph;
			if (cursorPos.glyph == cursorPos.lineEnd)
				cursorPos.pos.x += glyph.advance().cx;
			const auto& tagContents = iText.tag(iText.begin() + from_glyph(iterGlyph).first).contents();
			const auto& style = *static_variant_cast<style_list::const_iterator>(tagContents);
			auto& glyphFont = style.font() != boost::none ? *style.font() : font();
			glyphHeight = glyphFont.height();
			lineHeight = cursorPos.line->first.extents.cy;
		}
		else if (cursorPos.line != iGlyphLines.end())
			glyphHeight = lineHeight = cursorPos.line->first.extents.cy;
		else
			glyphHeight = lineHeight = font().height();
		update(rect{ point{ cursorPos.pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + client_rect(false).top_left() + point{ 0.0, lineHeight - glyphHeight }, size{1.0, glyphHeight} });
	}

	void text_edit::make_cursor_visible(bool aForcePreviewScroll)
	{
		scoped_units su(*this, UnitsPixels);
		auto p = position(cursor().position());
		auto e = (p.line != iGlyphLines.end() ? 
			size{ p.glyph != p.lineEnd ? p.glyph->advance().cx : 0.0, p.line->first.extents.cy } : 
			size{ 0.0, font().height() });
		if (p.pos.y < vertical_scrollbar().position())
			vertical_scrollbar().set_position(p.pos.y);
		else if (p.pos.y + e.cy > vertical_scrollbar().position() + vertical_scrollbar().page())
			vertical_scrollbar().set_position(p.pos.y + e.cy - vertical_scrollbar().page());
		dimension previewWidth = std::ceil(std::min(client_rect(false).width() / 3.0, 200.0));
		if (p.pos.x < horizontal_scrollbar().position() || 
			(aForcePreviewScroll && p.pos.x < horizontal_scrollbar().position() + previewWidth))
			horizontal_scrollbar().set_position(p.pos.x - previewWidth);
		else if (p.pos.x + e.cx > horizontal_scrollbar().position() + horizontal_scrollbar().page() || 
			(aForcePreviewScroll && p.pos.x + e.cx > horizontal_scrollbar().position() + horizontal_scrollbar().page() - previewWidth))
			horizontal_scrollbar().set_position(p.pos.x + e.cx + previewWidth - horizontal_scrollbar().page());
	}

	void text_edit::draw_glyphs(const graphics_context& aGraphicsContext, const point& aPoint, glyph_lines::const_iterator aLine) const
	{
		auto lineStart = iGlyphs.begin() + iGlyphLines.foreign_index(aLine).glyphs();
		auto lineEnd = lineStart + aLine->second.glyphs();
		{
			std::unique_ptr<graphics_context::glyph_drawing> gd;
			bool outlinesPresent = false;
			for (uint32_t pass = 0; pass <= 2; ++pass)
			{
				if (pass == 1)
					gd = std::make_unique<graphics_context::glyph_drawing>(aGraphicsContext);
				point pos = aPoint;
				for (document_glyphs::const_iterator i = lineStart; i != lineEnd; ++i)
				{
					bool selected = static_cast<cursor::position_type>(i - iGlyphs.begin()) >= std::min(cursor().position(), cursor().anchor()) &&
						static_cast<cursor::position_type>(i - iGlyphs.begin()) < std::max(cursor().position(), cursor().anchor());
					const auto& glyph = *i;
					const auto& tagContents = iText.tag(iText.begin() + from_glyph(i).first).contents();
					const auto& style = *static_variant_cast<style_list::const_iterator>(tagContents);
					const auto& glyphFont = style.font() != boost::none ? *style.font() : font();
					switch (pass)
					{
					case 0:
						if (selected)
							aGraphicsContext.fill_rect(rect{ pos, size{glyph.advance().cx, aLine->first.extents.cy} }, app::instance().current_style().selection_colour());
						break;
					case 1:
						if (style.text_outline_colour().empty())
						{
							pos.x += glyph.advance().cx;
							continue;
						}
						outlinesPresent = true;
						for (uint32_t outlinePos = 0; outlinePos < 8; ++outlinePos)
						{
							static point sOutlinePositions[] = 
							{
								point{-1.0, -1.0}, point{0.0, -1.0}, point{1.0, -1.0},
								point{-1.0, 0.0}, point{1.0, 0.0},
								point{-1.0, 1.0}, point{0.0, 1.0}, point{1.0, 1.0},
							};
							aGraphicsContext.draw_glyph(sOutlinePositions[outlinePos] + pos + glyph.offset() + point{ 0.0, aLine->first.extents.cy - glyphFont.height() - 1.0 }, glyph,
								glyphFont,
								style.text_outline_colour().is<colour>() ?
									static_variant_cast<const colour&>(style.text_outline_colour()) : style.text_outline_colour().is<gradient>() ?
										static_variant_cast<const gradient&>(style.text_outline_colour()).at((pos.x - margins().left + horizontal_scrollbar().position()) / std::max(client_rect(false).width(), iTextExtents.cx)) :
										default_text_colour());
						}
						break;
					case 2:
						aGraphicsContext.draw_glyph(pos + glyph.offset() + point{ 0.0, aLine->first.extents.cy - glyphFont.height() - (outlinesPresent ? 1.0 : 0.0)}, glyph,
							glyphFont,
							selected ? 
								(app::instance().current_style().selection_colour().light() ? colour::Black : colour::White) :
								style.text_colour().is<colour>() ?
									static_variant_cast<const colour&>(style.text_colour()) : style.text_colour().is<gradient>() ? 
										static_variant_cast<const gradient&>(style.text_colour()).at((pos.x - margins().left + horizontal_scrollbar().position()) / std::max(client_rect(false).width(), iTextExtents.cx)) :
										default_text_colour());
						break;
					}
					pos.x += glyph.advance().cx;
				}
			}
		}
		point pos = aPoint;
		for (document_glyphs::const_iterator i = lineStart; i != lineEnd; ++i)
		{
			const auto& glyph = *i;
			const auto& tagContents = iText.tag(iText.begin() + from_glyph(i).first).contents();
			const auto& style = *static_variant_cast<style_list::const_iterator>(tagContents);
			const auto& glyphFont = style.font() != boost::none ? *style.font() : font();
			if (glyph.underline())
				aGraphicsContext.draw_glyph_underline(pos + point{ 0.0, aLine->first.extents.cy - glyphFont.height() }, glyph,
					glyphFont,
					style.text_colour().is<colour>() ?
						static_variant_cast<const colour&>(style.text_colour()) : style.text_colour().is<gradient>() ? 
							static_variant_cast<const gradient&>(style.text_colour()).at((pos.x - margins().left) / client_rect(false).width()) : 
							default_text_colour());
			pos.x += glyph.advance().cx;
		}
	}

	void text_edit::draw_cursor(const graphics_context& aGraphicsContext) const
	{
		auto cursorPos = position(iCursor.position());
		dimension glyphHeight = 0.0;
		dimension lineHeight = 0.0;
		if (cursorPos.glyph != iGlyphs.end() && cursorPos.lineStart != cursorPos.lineEnd)
		{
			auto iterGlyph = cursorPos.glyph < cursorPos.lineEnd ? cursorPos.glyph : cursorPos.glyph - 1;
			const auto& tagContents = iText.tag(iText.begin() + from_glyph(iterGlyph).first).contents();
			const auto& style = *static_variant_cast<style_list::const_iterator>(tagContents);
			auto& glyphFont = style.font() != boost::none ? *style.font() : font();
			glyphHeight = glyphFont.height();
			if (!style.text_outline_colour().empty())
				glyphHeight += 2.0;
			lineHeight = cursorPos.line->first.extents.cy;
		}
		else if (cursorPos.line != iGlyphLines.end())
			glyphHeight = lineHeight = cursorPos.line->first.extents.cy;
		else
			glyphHeight = lineHeight = font().height();
		if (((app::instance().program_elapsed_ms() - iCursorAnimationStartTime) / 500) % 2 == 0)
		{
			auto elapsed = (app::instance().program_elapsed_ms() - iCursorAnimationStartTime) % 1000;
			colour::component alpha = 
				elapsed < 500 ? 
					255 : 
					elapsed < 750 ? 
						static_cast<colour::component>((249 - (elapsed - 500) % 250) * 255 / 249) : 
						0;
			if (iCursor.colour().empty())
			{
				aGraphicsContext.push_logical_operation(LogicalXor);
				aGraphicsContext.draw_line(
					point{ cursorPos.pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + client_rect(false).top_left() + point{ 0.0, lineHeight },
					point{ cursorPos.pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + client_rect(false).top_left() + point{ 0.0, lineHeight - glyphHeight },
					pen{ colour::White.with_alpha(alpha), iCursor.width() });
				aGraphicsContext.pop_logical_operation();
			}
			else if (iCursor.colour().is<colour>())
			{
				aGraphicsContext.draw_line(
					point{ cursorPos.pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + client_rect(false).top_left() + point{ 0.0, lineHeight },
					point{ cursorPos.pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + client_rect(false).top_left() + point{ 0.0, lineHeight - glyphHeight },
					pen{ static_variant_cast<const colour&>(iCursor.colour()).with_combined_alpha(alpha), iCursor.width() });
			}
			else if (iCursor.colour().is<gradient>())
			{
				aGraphicsContext.fill_rect(
					rect{
						point{ cursorPos.pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + client_rect(false).top_left() + point{ 0.0, lineHeight - glyphHeight},
						size{ iCursor.width(), glyphHeight} },
					static_variant_cast<const gradient&>(iCursor.colour()).with_combined_alpha(alpha));
			}
		}
	}

	std::pair<text_edit::document_glyphs::iterator, text_edit::document_glyphs::iterator> text_edit::word_break(document_glyphs::iterator aBegin, document_glyphs::iterator aFrom, document_glyphs::iterator aEnd)
	{
		std::pair<document_glyphs::iterator, document_glyphs::iterator> result(aFrom, aFrom);
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
		/* skip whitespace... */
		while (result.first != aBegin && (result.first - 1)->is_whitespace())
			--result.first;
		while (result.second->is_whitespace() && result.second != aEnd)
			++result.second;
		return result;
	}
}