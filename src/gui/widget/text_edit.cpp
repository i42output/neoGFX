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
#include <neogfx/gfx/text/text_category_map.hpp>
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
		if (iParent && &iParent->iDefaultStyle != this && --iUseCount == 0)
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

	void text_edit::style::set_font(const optional_font& aFont)
	{
		iFont = aFont;
	}

	void text_edit::style::set_text_colour(const colour_type& aColour)
	{
		iTextColour = aColour;
	}

	void text_edit::style::set_background_colour(const colour_type& aColour)
	{
		iBackgroundColour = aColour;
	}

	void text_edit::style::set_text_outline_colour(const colour_type& aColour)
	{
		iTextOutlineColour = aColour;
	}

	text_edit::style& text_edit::style::merge(const style& aOverridingStyle)
	{
		if (aOverridingStyle.font() != boost::none)
			iFont = aOverridingStyle.font();
		if (aOverridingStyle.text_colour() != boost::none)
			iTextColour = aOverridingStyle.text_colour();
		if (aOverridingStyle.background_colour() != boost::none)
			iBackgroundColour = aOverridingStyle.background_colour();
		if (aOverridingStyle.text_outline_colour() != boost::none)
			iTextOutlineColour = aOverridingStyle.text_outline_colour();
		return *this;
	}

	bool text_edit::style::operator==(const style& aRhs) const
	{
		return std::tie(iFont, iTextColour, iBackgroundColour, iTextOutlineColour) == std::tie(aRhs.iFont, aRhs.iTextColour, aRhs.iBackgroundColour, aRhs.iTextOutlineColour);
	}

	bool text_edit::style::operator!=(const style& aRhs) const
	{
		return !(*this == aRhs);
	}

	bool text_edit::style::operator<(const style& aRhs) const
	{
		return std::tie(iFont, iTextColour, iBackgroundColour, iTextOutlineColour) < std::tie(aRhs.iFont, aRhs.iTextColour, aRhs.iBackgroundColour, aRhs.iTextOutlineColour);
	}

	text_edit::text_edit(type_e aType) :
		scrollable_widget(aType == MultiLine ? i_scrollbar::Normal : i_scrollbar::Invisible), 
		iType(aType),
		iReadOnly(false),
		iWordWrap(aType == MultiLine),
		iPassword(false),
		iAlignment(neogfx::alignment::Left|neogfx::alignment::Top),
		iPersistDefaultStyle(false),
		iGlyphColumns(1),
		iCursorAnimationStartTime(app::instance().program_elapsed_ms()),
		iGlyphParagraphCache(nullptr),
		iTabStopHint("0000"),
		iAnimator(app::instance(), [this](neolib::callback_timer&)
		{
			iAnimator.again();
			animate();
		}, 40)
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
		iPersistDefaultStyle(false),
		iGlyphColumns(1),
		iCursorAnimationStartTime(app::instance().program_elapsed_ms()),
		iGlyphParagraphCache(nullptr),
		iTabStopHint("0000"),
		iAnimator(app::instance(), [this](neolib::callback_timer&)
		{
			iAnimator.again();
			animate();
		}, 40)
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
		iPersistDefaultStyle(false),
		iGlyphColumns(1),
		iCursorAnimationStartTime(app::instance().program_elapsed_ms()),
		iGlyphParagraphCache(nullptr),
		iTabStopHint("0000"),
		iAnimator(app::instance(), [this](neolib::callback_timer&)
		{
			iAnimator.again();
			animate();
		}, 40)
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
		coordinate x = 0.0;
		for (auto iterColumn = iGlyphColumns.begin(); iterColumn != iGlyphColumns.end(); ++iterColumn)
		{
			const auto& column = *iterColumn;
			const auto& lines = column.lines();
			auto line = std::lower_bound(lines.begin(), lines.end(), glyph_line{ {}, {}, {}, vertical_scrollbar().position(), {} },
				[](const glyph_line& left, const glyph_line& right) { return left.ypos < right.ypos; });
			if (line != lines.begin() && line->ypos > vertical_scrollbar().position())
				--line;
			if (line == lines.end())
				continue;
			auto y = line->ypos;
			for (auto paintLine = line; paintLine != lines.end(); y += (paintLine++)->extents.cy)
			{
				point linePos = client_rect(false).top_left() + point{ -horizontal_scrollbar().position(), y - vertical_scrollbar().position() };
				if (linePos.y + paintLine->extents.cy < client_rect(false).top() || linePos.y + paintLine->extents.cy < update_rect().top())
					continue;
				if (linePos.y > client_rect(false).bottom() || linePos.y > update_rect().bottom())
					break;
				auto textDirection = glyph_text_direction(paintLine->lineStart.second, paintLine->lineEnd.second);
				if (iAlignment == alignment::Left && textDirection == text_direction::RTL ||
					iAlignment == alignment::Right && textDirection == text_direction::LTR)
					linePos.x += column.width() - column.margins().right - aGraphicsContext.from_device_units(size{ paintLine->extents.cx, 0 }).cx;
				else if (iAlignment == alignment::Centre)
					linePos.x += std::ceil((column.width() - aGraphicsContext.from_device_units(size{ paintLine->extents.cx, 0 }).cx) / 2);
				else
					linePos.x += column.margins().left;
				draw_glyphs(aGraphicsContext, linePos, column, paintLine);
			}
			x += column.width();
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
			cursor().set_position(iText.size(), false);
		}
		update();
	}

	void text_edit::focus_lost()
	{
		scrollable_widget::focus_lost();
		app::instance().clipboard().deactivate(*this);
		if (iType == SingleLine)
			cursor().set_position(iText.size());
		update();
	}

	void text_edit::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		scrollable_widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left && client_rect().contains(aPosition))
		{
			set_cursor_glyph_position(hit_test(aPosition), (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
			if (capturing())
			{
				iDragger.emplace(app::instance(), [this](neolib::callback_timer& aTimer)
				{
					aTimer.again();
					set_cursor_glyph_position(hit_test(surface().mouse_position() - origin()), false);
				}, 250);
			}
		}
	}

	void text_edit::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		scrollable_widget::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
		if (!password() && aButton == mouse_button::Left && client_rect().contains(aPosition))
		{
			auto word = word_at(hit_test(aPosition));
			cursor().set_anchor(word.first);
			cursor().set_position(word.second, false);
		}
	}
	
	void text_edit::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		scrollable_widget::mouse_button_released(aButton, aPosition);
		iDragger = boost::none;
		if (aButton == mouse_button::Right)
		{
			iMenu = std::make_unique<context_menu>(*this, aPosition + window_rect().top_left() + surface().surface_position());
			iMenu->menu().add_action(app::instance().action_undo());
			iMenu->menu().add_action(app::instance().action_redo());
			iMenu->menu().add_separator();
			iMenu->menu().add_action(app::instance().action_cut());
			iMenu->menu().add_action(app::instance().action_copy());
			iMenu->menu().add_action(app::instance().action_paste());
			iMenu->menu().add_action(app::instance().action_delete());
			iMenu->menu().add_separator();
			iMenu->menu().add_action(app::instance().action_select_all());
			iMenu->exec();
			iMenu.reset();
		}
	}

	void text_edit::mouse_moved(const point& aPosition)
	{
		scrollable_widget::mouse_moved(aPosition);
		if (iDragger != boost::none)
			set_cursor_glyph_position(hit_test(aPosition), false);
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
					cursor().set_position(iText.size(), (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
				else
				{
					auto pos = point{ glyph_position(cursor_glyph_position()).pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } };
					scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
					set_cursor_glyph_position(hit_test(pos + client_rect(false).top_left()), (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
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
		if (aText[0] == '\n') // newlines are handled in key_pressed()
		{
			if ((focus_policy() & neogfx::focus_policy::ConsumeReturnKey) == neogfx::focus_policy::ConsumeReturnKey)
				return true;
			else
				return scrollable_widget::text_input(aText);
		}
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
				vertical_scrollbar().set_maximum(iTextExtents.cy);
				vertical_scrollbar().set_step(font().height());
				vertical_scrollbar().set_page(client_rect(false).height());
				vertical_scrollbar().set_position(oldPosition);
				bool changed = false;
				if (vertical_scrollbar().maximum() - vertical_scrollbar().page() > 0.0)
				{
					if (!vertical_scrollbar().visible())
					{
						vertical_scrollbar().show();
						changed = true;
					}
				}
				else
				{
					if (vertical_scrollbar().visible())
					{
						vertical_scrollbar().hide();
						changed = true;
					}
				}
				if (changed)
					refresh_lines();
				scrollable_widget::update_scrollbar_visibility(aStage);
			}
			break;
		case UsvStageCheckHorizontal:
			{
				i_scrollbar::value_type oldPosition = horizontal_scrollbar().position();
				horizontal_scrollbar().set_maximum(iTextExtents.cx <= client_rect(false).width() ? 0.0 : iTextExtents.cx);
				horizontal_scrollbar().set_step(font().height());
				horizontal_scrollbar().set_page(client_rect(false).width());
				horizontal_scrollbar().set_position(oldPosition);
				if (horizontal_scrollbar().maximum() - horizontal_scrollbar().page() > 0.0)
					horizontal_scrollbar().show();
				else
					horizontal_scrollbar().hide();
				scrollable_widget::update_scrollbar_visibility(aStage);
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

	bool text_edit::can_undo() const
	{
		/* todo */
		return false;
	}

	bool text_edit::can_redo() const
	{
		/* todo */
		return false;
	}

	bool text_edit::can_cut() const
	{
		return !read_only() && !iText.empty() && cursor().position() != cursor().anchor();
	}

	bool text_edit::can_copy() const
	{
		return !iText.empty() && cursor().position() != cursor().anchor();
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

	void text_edit::undo(i_clipboard& aClipboard)
	{
		/* todo */
	}

	void text_edit::redo(i_clipboard& aClipboard)
	{
		/* todo */
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
			std::u32string selectedText;
			auto selectionStart = std::min(cursor().position(), cursor().anchor());
			auto selectionEnd = std::max(cursor().position(), cursor().anchor());
			selectedText.assign(iText.begin() + selectionStart, iText.begin() + selectionEnd);
			aClipboard.set_text(neolib::utf32_to_utf8(selectedText));
		}
	}

	void text_edit::paste(i_clipboard& aClipboard)
	{
		if (cursor().position() != cursor().anchor())
			delete_selected(aClipboard);
		auto len = insert_text(aClipboard.text());
		cursor().set_position(cursor().position() + len);
	}

	void text_edit::delete_selected(i_clipboard&)
	{
		if (cursor().position() != cursor().anchor())
			delete_any_selection();
		else if(cursor().position() < iText.size())
			delete_text(cursor().position(), cursor().position() + 1);
	}

	void text_edit::select_all(i_clipboard&)
	{
		cursor().set_anchor(0);
		cursor().set_position(iText.size(), false);
	}

	void text_edit::move_cursor(cursor::move_operation_e aMoveOperation, bool aMoveAnchor)
	{
		if (iGlyphs.empty())
			return;
		switch (aMoveOperation)
		{
		case cursor::StartOfDocument:
			cursor().set_position(0, aMoveAnchor);
			break;
		case cursor::StartOfParagraph:
			break;
		case cursor::StartOfLine:
			{
				auto p = cursor().position();
				while (p > 0)
				{
					if (iText[p - 1] == U'\n')
						break;
					--p;
				}
				cursor().set_position(p, aMoveAnchor);
			}
			break;
		case cursor::StartOfWord:
			break;
		case cursor::EndOfDocument:
			cursor().set_position(iText.size(), aMoveAnchor);
			break;
		case cursor::EndOfParagraph:
			break;
		case cursor::EndOfLine:
			{
				auto p = cursor().position();
				while (p < iText.size())
				{
					if (iText[p] == U'\n')
						break;
					++p;
				}
				cursor().set_position(p, aMoveAnchor);
			}
			break;
		case cursor::EndOfWord:
			break;
		case cursor::PreviousParagraph:
			break;
		case cursor::PreviousLine:
			break;
		case cursor::PreviousWord:
			if (!iText.empty())
			{
				auto const& emojiAtlas = app::instance().rendering_engine().font_manager().emoji_atlas();
				auto p = cursor().position();
				if (p == iText.size())
					--p;
				while (p > 0 && get_text_category(emojiAtlas, iText[p]) == text_category::Whitespace)
					--p;
				if (p > 0)
				{
					auto c = get_text_category(emojiAtlas, iText[p == cursor().position() ? p - 1 : p]);
					while (p > 0 && iGlyphs[p - 1].category() == c)
						--p;
					if (p > 0 && c == text_category::Whitespace)
					{
						c = get_text_category(emojiAtlas, iText[p - 1]);
						while (p > 0 && get_text_category(emojiAtlas, iText[p - 1]) == c)
							--p;
					}
				}
				cursor().set_position(p, aMoveAnchor);
			}
			break;
		case cursor::PreviousCharacter:
			if (cursor().position() > 0)
				cursor().set_position(cursor().position() - 1, aMoveAnchor);
			break;
		case cursor::NextParagraph:
			break;
		case cursor::NextLine:
			break;
		case cursor::NextWord:
			if (!iText.empty())
			{
				auto const& emojiAtlas = app::instance().rendering_engine().font_manager().emoji_atlas();
				auto p = cursor().position();
				while (p < iText.size() && get_text_category(emojiAtlas, iText[p]) == text_category::Whitespace)
					++p;
				if (p < iText.size() && p == cursor().position())
				{
					auto c = get_text_category(emojiAtlas, iText[p]);
					while (p < iText.size() && get_text_category(emojiAtlas, iText[p]) == c)
						++p;
					while (p < iText.size() && get_text_category(emojiAtlas, iText[p]) == text_category::Whitespace)
						++p;
				}
				cursor().set_position(p, aMoveAnchor);
			}
			break;
		case cursor::NextCharacter:
			if (cursor().position() < iText.size())
				cursor().set_position(cursor().position() + 1, aMoveAnchor);
			break;
		case cursor::Up:
			{
				auto currentPosition = glyph_position(cursor_glyph_position());
				if (currentPosition.line != currentPosition.column->lines().begin())
					cursor().set_position(from_glyph(iGlyphs.begin() + hit_test(point{ currentPosition.pos.x, (currentPosition.line - 1)->ypos }, false)).first, aMoveAnchor);
			}
			break;
		case cursor::Down:
			{
				auto currentPosition = glyph_position(cursor_glyph_position());
				if (currentPosition.line != currentPosition.column->lines().end())
				{
					if (currentPosition.line + 1 != currentPosition.column->lines().end())
						cursor().set_position(from_glyph(iGlyphs.begin() + hit_test(point{ currentPosition.pos.x, (currentPosition.line + 1)->ypos }, false)).first, aMoveAnchor);
					else if (currentPosition.lineEnd != iGlyphs.end() && currentPosition.lineEnd->is_whitespace() && currentPosition.lineEnd->value() == U'\n')
						cursor().set_position(iText.size(), aMoveAnchor);
				}
			}
			break;
		case cursor::Left:
			if (cursor().position() > 0)
				cursor().set_position(cursor().position() - 1, aMoveAnchor);
			break;
		case cursor::Right:
			if (cursor().position() < iText.size())
				cursor().set_position(cursor().position() + 1, aMoveAnchor);
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
			refresh_columns();
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

	void text_edit::set_default_style(const style& aDefaultStyle, bool aPersist)
	{
		neogfx::font oldFont = font();
		iDefaultStyle = aDefaultStyle;
		if (oldFont != font())
			refresh_paragraph(iText.begin(), 0);
		iPersistDefaultStyle = aPersist;
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

	text_edit::position_info text_edit::glyph_position(position_type aGlyphPosition, bool aForCursor) const
	{
		auto column = iGlyphColumns.begin();
		glyph_lines::const_iterator line;
		for (; column != iGlyphColumns.end(); ++column)
		{
			line = std::lower_bound(column->lines().begin(), column->lines().end(), glyph_line{ {}, { aGlyphPosition, iGlyphs.begin() + aGlyphPosition }, {}, 0.0, {} },
				[](const glyph_line& left, const glyph_line& right) { return left.lineStart.first < right.lineStart.first; });
			if (line != column->lines().end())
				break;
		}
		if (column == iGlyphColumns.end())
			--column;
		auto& lines = column->lines();
		if (line != lines.begin())
		{
			if (line == lines.end())
			{
				if (aGlyphPosition <= lines.back().lineEnd.first || (!iGlyphs.back().is_whitespace() || iGlyphs.back().value() != U'\n'))
					--line;
			}
			else if (aGlyphPosition < line->lineStart.first)
				--line;
		}
		if (line != lines.end())
		{
			position_type lineStart = line->lineStart.first;
			position_type lineEnd = line->lineEnd.first;
			bool placeCursorToRight = (aGlyphPosition == lineEnd);
			if (aForCursor)
			{
				auto paragraph = iGlyphParagraphs.find_by_foreign_index(glyph_paragraph_index{ 0, aGlyphPosition }, [](const glyph_paragraph_index& aLhs, const glyph_paragraph_index& aRhs) { return aLhs.glyphs() < aRhs.glyphs(); });
				if (aGlyphPosition == iGlyphs.size() || aGlyphPosition == paragraph.first->first.end_index())
				{
					auto iterChar = iText.begin() + from_glyph(iGlyphs.begin() + aGlyphPosition).first;
					if (iterChar != iText.begin())
					{
						auto iterGlyph = to_glyph(iterChar - 1);
						const auto& g = *iterGlyph;
						if (g.direction() == text_direction::RTL || g.direction() == text_direction::Digits_RTL)
						{
							aGlyphPosition = iterGlyph - iGlyphs.begin();
							placeCursorToRight = (g.direction() == text_direction::RTL ? false : true);
						}
					}
				}
				else if (iGlyphs[aGlyphPosition].direction() == text_direction::RTL)
					placeCursorToRight = true;
			}
			if (aGlyphPosition >= lineStart && aGlyphPosition <= lineEnd)
			{
				if (lineStart != lineEnd)
				{
					auto iterGlyph = iGlyphs.begin() + aGlyphPosition;
					const auto& glyph = aGlyphPosition < lineEnd ? *iterGlyph : *(iterGlyph - 1);
					point linePos{ glyph.x - line->lineStart.second->x, line->ypos };
					if (placeCursorToRight)
						linePos.x += glyph.advance().cx;
					return position_info{ iterGlyph, column, line, iGlyphs.begin() + lineStart, iGlyphs.begin() + lineEnd, linePos };
				}
				else
					return position_info{ line->lineStart.second, column, line, iGlyphs.begin() + lineStart, iGlyphs.begin() + lineEnd, point{ 0.0, line->ypos } };
			}
		}
		point pos;
		if (!lines.empty())
		{
			pos.x = 0.0;
			pos.y = lines.back().ypos + lines.back().extents.cy;
		}
		return position_info{ iGlyphs.end(), column, lines.end(), iGlyphs.end(), iGlyphs.end(), pos };
	}

	text_edit::position_type text_edit::cursor_glyph_position() const
	{
		return to_glyph(iText.begin() + cursor().position()) - iGlyphs.begin();
	}

	text_edit::position_type text_edit::cursor_glyph_anchor() const
	{
		return to_glyph(iText.begin() + cursor().anchor()) - iGlyphs.begin();
	}

	void text_edit::set_cursor_glyph_position(position_type aGlyphPosition, bool aMoveAnchor)
	{
		cursor().set_position(from_glyph(iGlyphs.begin() + aGlyphPosition).first, aMoveAnchor);
	}

	text_edit::position_type text_edit::hit_test(const point& aPoint, bool aAdjustForScrollPosition) const
	{
		point adjusted = aAdjustForScrollPosition ?
			point{ aPoint - client_rect(false).top_left() } + point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } :
			aPoint;
		if (adjusted.y < 0.0)
			adjusted.y = 0.0;
		if (adjusted.x < 0.0)
			adjusted.x = 0.0;
		auto iterColumn = iGlyphColumns.begin();
		for (; iterColumn != iGlyphColumns.end(); ++iterColumn)
		{
			if (adjusted.x < iterColumn->width())
				break;
			adjusted.x -= iterColumn->width();
		}
		if (iterColumn == iGlyphColumns.end())
			--iterColumn;
		const auto& column = *iterColumn;
		adjusted.x -= column.margins().left;
		if (adjusted.x < 0.0)
			adjusted.x = 0.0;
		const auto& lines = column.lines();
		auto line = std::lower_bound(lines.begin(), lines.end(), glyph_line{ {}, {}, {}, adjusted.y, {} },
			[](const glyph_line& left, const glyph_line& right) { return left.ypos < right.ypos; });
		if (line == lines.end() && !lines.empty() && adjusted.y < lines.back().ypos + lines.back().extents.cy)
			--line;
		if (line != lines.end())
		{
			if (line != lines.begin() && adjusted.y < line->ypos)
				--line;
			auto lineStart = (line != lines.end() ? line->lineStart.first : iGlyphs.size());
			auto lineEnd = (line != lines.end() ? line->lineEnd.first : iGlyphs.size());
			auto lineStartX = line->lineStart.second->x;
			for (auto gi = line->lineStart.first; gi != lineEnd; ++gi)
			{
				auto& g = iGlyphs[gi];
				if (adjusted.x >= g.x - lineStartX && adjusted.x < g.x - lineStartX + g.advance().cx)
				{
					if (g.direction() != text_direction::RTL)
						return adjusted.x < g.x - lineStartX + g.advance().cx / 2.0 ? gi : gi + 1;
					else
						return gi + 1;
				}
			}
			if (lineEnd > lineStart && iGlyphs[lineEnd - 1].direction() == text_direction::RTL)
				return lineEnd - 1;
			return lineEnd;
		}
		else
		{
			return iGlyphs.size();
		}
	}

	bool text_edit::same_word(position_type aTextPositionLeft, position_type aTextPositionRight) const
	{
		if (iText[aTextPositionLeft] == U'\n' || iText[aTextPositionRight] == U'\n')
			return false;
		auto const& emojiAtlas = app::instance().rendering_engine().font_manager().emoji_atlas();
		return get_text_category(emojiAtlas, iText[aTextPositionLeft]) == get_text_category(emojiAtlas, iText[aTextPositionRight]);
	}

	std::pair<text_edit::position_type, text_edit::position_type> text_edit::word_at(position_type aTextPosition) const
	{
		auto start = aTextPosition;
		auto end = aTextPosition;
		while (start > 0 && same_word(start - 1, aTextPosition))
			--start;
		while (end < iText.size() && same_word(aTextPosition, end))
			++end;
		return std::make_pair(start, end);
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
		cursor().set_position(0);
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
		std::u32string text = neolib::utf8_to_utf32(aText);
		if (iNormalizedTextBuffer.capacity() < text.size())
			iNormalizedTextBuffer.reserve(text.size());
		iNormalizedTextBuffer.clear();
		for (auto ch : text)
			if (ch != U'\r')
				iNormalizedTextBuffer.push_back(ch);
		auto eos = iNormalizedTextBuffer.size();
		if (iType == SingleLine)
		{
			auto eol = iNormalizedTextBuffer.find(U'\n');
			if (eol != std::u32string::npos)
				eos = eol;
		}
		auto s = (&aStyle != &iDefaultStyle || iPersistDefaultStyle ? iStyles.insert(style(*this, aStyle)).first : iStyles.end());
		auto insertionPoint = iText.begin() + cursor().position();
		insertionPoint = iText.insert(s != iStyles.end() ? document_text::tag_type{ static_cast<style_list::const_iterator>(s) } : document_text::tag_type{ nullptr },
			insertionPoint, iNormalizedTextBuffer.begin(), iNormalizedTextBuffer.begin() + eos);
		refresh_paragraph(insertionPoint, eos);
		update();
		if (aMoveCursor)
			cursor().set_position(insertionPoint - iText.begin() + eos);
		text_changed.trigger();
		return eos;
	}

	void text_edit::delete_text(position_type aStart, position_type aEnd)
	{
		if (aStart == aEnd)
			return;
		auto eraseBegin = iText.begin() + aStart;
		auto eraseEnd = iText.begin() + aEnd;
		auto eraseAmount = eraseEnd - eraseBegin;
		refresh_paragraph(iText.erase(eraseBegin, eraseEnd), -eraseAmount);
		update();
		text_changed.trigger();
	}

	std::pair<text_edit::position_type, text_edit::position_type> text_edit::related_glyphs(position_type aGlyphPosition) const
	{
		std::pair<position_type, position_type> result{ aGlyphPosition, aGlyphPosition + 1 };
		while (result.first > 0 && same_paragraph(aGlyphPosition, result.first - 1) && iGlyphs[result.first-1].source() == iGlyphs[aGlyphPosition].source())
			--result.first;
		while (result.second < iGlyphs.size() && same_paragraph(aGlyphPosition, result.second) && iGlyphs[result.second].source() == iGlyphs[aGlyphPosition].source())
			++result.second;
		return result;
	}

	bool text_edit::same_paragraph(position_type aFirst, position_type aSecond) const
	{
		auto paragraph1 = iGlyphParagraphs.find_by_foreign_index(glyph_paragraph_index{ 0, aFirst }, [](const glyph_paragraph_index& aLhs, const glyph_paragraph_index& aRhs) { return aLhs.glyphs() < aRhs.glyphs(); });
		auto paragraph2 = iGlyphParagraphs.find_by_foreign_index(glyph_paragraph_index{ 0, aSecond }, [](const glyph_paragraph_index& aLhs, const glyph_paragraph_index& aRhs) { return aLhs.glyphs() < aRhs.glyphs(); });
		return paragraph1.first == paragraph2.first;
	}

	std::size_t text_edit::columns() const
	{
		return iGlyphColumns.size();
	}

	void text_edit::set_columns(std::size_t aColumnCount)
	{
		iGlyphColumns.resize(aColumnCount);
		refresh_paragraph(iText.begin(), 0);
	}

	void text_edit::remove_columns()
	{
		iGlyphColumns.resize(1);
		iGlyphColumns[0] = glyph_column{};
		refresh_paragraph(iText.begin(), 0);
	}

	const text_edit::column_info& text_edit::column(std::size_t aColumnIndex)
	{
		if (aColumnIndex >= iGlyphColumns.size())
			throw bad_column_index();
		return iGlyphColumns[aColumnIndex];
	}

	void text_edit::set_column(std::size_t aColumnIndex, const column_info& aColumn)
	{
		if (aColumnIndex >= iGlyphColumns.size())
			throw bad_column_index();
		if (iGlyphColumns[aColumnIndex] != aColumn)
		{
			static_cast<column_info&>(iGlyphColumns[aColumnIndex]) = aColumn;
			refresh_paragraph(iText.begin(), 0);
		}
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

	dimension text_edit::tab_stops() const
	{
		if (iCalculatedTabStops == boost::none || iCalculatedTabStops->first != font())
			iCalculatedTabStops = std::make_pair(font(), (iTabStops != boost::none ?
				*iTabStops : graphics_context(*this).text_extent(iTabStopHint, font()).cx));
		return iCalculatedTabStops->second;
	}

	void text_edit::set_tab_stop_hint(const std::string& aTabStopHint)
	{
		if (iTabStopHint != aTabStopHint)
		{
			iTabStopHint = aTabStopHint;
			iCalculatedTabStops.reset();
		}
	}

	void text_edit::set_tab_stops(const optional_dimension& aTabStops)
	{
		optional_dimension newTabStops = (aTabStops != boost::none ? units_converter(*this).to_device_units(size{ *aTabStops, 0.0 }).cx : optional_dimension{});
		if (iTabStops != newTabStops)
		{
			iTabStops = newTabStops;
			refresh_columns();
		}
		iCalculatedTabStops.reset();
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
		auto focusPolicy = neogfx::focus_policy::ClickTabFocus;
		if (iType == MultiLine)
			focusPolicy |= neogfx::focus_policy::ConsumeReturnKey;
		set_focus_policy(focusPolicy);
		cursor().set_width(2.0);
		iSink += cursor().position_changed([this]()
		{
			iCursorAnimationStartTime = app::instance().program_elapsed_ms();
			make_cursor_visible();
			update();
		});
		iSink += cursor().anchor_changed([this]()
		{
			update();
		});
		iSink += cursor().appearance_changed([this]()
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
		auto paragraphEnd = iGlyphParagraphCache->end();
		for (auto i = iGlyphParagraphCache->start(); i != paragraphEnd; ++i)
		{
			auto const& g = *i;
			auto start = g.source().first + iGlyphParagraphCache->text_start_index();
			auto end = g.source().second + iGlyphParagraphCache->text_start_index();
			if (textIndex >= start && textIndex < end)
				return i;
		}
		return paragraphEnd;
	}

	std::pair<text_edit::document_text::size_type, text_edit::document_text::size_type> text_edit::from_glyph(document_glyphs::const_iterator aWhere) const
	{
		if (aWhere == iGlyphs.end())
			return std::make_pair(iText.size(), iText.size());
		if (iGlyphParagraphCache != nullptr && aWhere >= iGlyphParagraphCache->start() && aWhere < iGlyphParagraphCache->end())
		{
			auto textStart = iGlyphParagraphCache->text_start_index();
			auto rg = related_glyphs(aWhere - iGlyphs.begin());
			return std::make_pair(textStart + iGlyphs[rg.first].source().first, textStart + iGlyphs[rg.second - 1].source().second);
		}
		auto paragraph = iGlyphParagraphs.find_by_foreign_index(glyph_paragraph_index{0, static_cast<std::size_t>(aWhere - iGlyphs.begin())}, 
			[](const glyph_paragraph_index& aLhs, const glyph_paragraph_index& aRhs) 
		{ 
			return aLhs.glyphs() < aRhs.glyphs();
		});
		if (paragraph.first == iGlyphParagraphs.end() && paragraph.first != iGlyphParagraphs.begin() && aWhere <= (paragraph.first - 1)->first.end())
			--paragraph.first;
		if (paragraph.first != iGlyphParagraphs.end())
		{
			if (paragraph.first->first.start() > aWhere)
				--paragraph.first;
			iGlyphParagraphCache = &paragraph.first->first;
			auto textStart = paragraph.first->first.text_start_index();
			auto rg = related_glyphs(aWhere - iGlyphs.begin());
			return std::make_pair(textStart + iGlyphs[rg.first].source().first, textStart + iGlyphs[rg.second - 1].source().second);
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
		if (password())
			gc.set_password(true, iPasswordMask.empty() ? "\xE2\x97\x8F" : iPasswordMask);
		iGlyphs.clear();
		iGlyphParagraphs.clear();
		iGlyphParagraphCache = nullptr;
		std::u32string paragraphBuffer;
		auto paragraphStart = iText.begin();
		auto iterColumn = iGlyphColumns.begin();
		neolib::vecarray<std::u32string::size_type, 16, -1> columnDelimiters;
		auto fs = [this, paragraphStart, &columnDelimiters](std::u32string::size_type aSourceIndex)
		{
			const auto& tagContents = iText.tag(paragraphStart + aSourceIndex).contents();
			std::size_t indexColumn = std::lower_bound(columnDelimiters.begin(), columnDelimiters.end(), aSourceIndex) - columnDelimiters.begin();
			if (indexColumn > columns() - 1)
				indexColumn = columns() - 1;
			const auto& columnStyle = text_edit::column(indexColumn).style();
			const auto& style =
				tagContents.is<style_list::const_iterator>() ? *static_variant_cast<style_list::const_iterator>(tagContents) :
				columnStyle.font() != boost::none ? columnStyle : iDefaultStyle;
			return style.font() != boost::none ? *style.font() : font();
		};
		for (auto iterChar = iText.begin(); iterChar != iText.end(); ++iterChar)
		{
			auto& column = *(iterColumn);
			auto ch = *iterChar;
			if (ch == column.delimiter() && iterColumn + 1 != iGlyphColumns.end())
			{
				++iterColumn;
				columnDelimiters.push_back(iterChar - paragraphStart);
				continue;
			}
			bool newLine = (ch == U'\n');
			if (newLine || iterChar == iText.end() - 1)
			{
				paragraphBuffer.assign(paragraphStart, iterChar + 1);
				auto gt = gc.to_glyph_text(paragraphBuffer.begin(), paragraphBuffer.end(), fs);
				auto paragraphGlyphs = iGlyphs.insert(iGlyphs.end(), gt.cbegin(), gt.cend());
				auto newParagraph = iGlyphParagraphs.insert(iGlyphParagraphs.end(),
					std::make_pair(
						glyph_paragraph{*this},
						glyph_paragraph_index{
							static_cast<std::size_t>((iterChar + 1) - iText.begin()) - static_cast<std::size_t>(paragraphStart - iText.begin()),
							iGlyphs.size() - static_cast<std::size_t>(paragraphGlyphs - iGlyphs.begin())}),
					glyph_paragraphs::skip_type{glyph_paragraph_index{}, glyph_paragraph_index{}});
				newParagraph->first.set_self(newParagraph);
				paragraphStart = iterChar + 1;
				columnDelimiters.clear();
			}
		}
		for (auto p = iGlyphParagraphs.begin(); p != iGlyphParagraphs.end(); ++p)
		{
			auto& paragraph = *p;
			if (paragraph.first.start() == paragraph.first.end())
				continue;
			coordinate x = 0.0;
			iterColumn = iGlyphColumns.begin();
			for (auto iterGlyph = paragraph.first.start(); iterGlyph != paragraph.first.end(); ++iterGlyph)
			{
				if (iText[iterGlyph->source().first] == iterColumn->delimiter() && iterColumn + 1 != iGlyphColumns.end())
				{
					iterGlyph->set_advance(size{});
					++iterColumn;
					continue;
				}
				else if (iterGlyph->is_whitespace() && iterGlyph->value() == U'\t')
				{
					auto advance = iterGlyph->advance();
					advance.cx = tab_stops() - std::fmod(x, tab_stops());
					iterGlyph->set_advance(advance);
				}
				iterGlyph->x = x;
				x += iterGlyph->advance().cx;
			}
		}
		refresh_columns();
	}

	void text_edit::refresh_columns()
	{
		update_scrollbar_visibility();
	}

	void text_edit::refresh_lines()
	{
		/* simple (naive) implementation just to get things moving... */
		for (auto& column : iGlyphColumns)
			column.lines().clear();
		point pos{};
		dimension availableWidth = client_rect(false).width();
		dimension availableHeight = client_rect(false).height();
		bool showVerticalScrollbar = false;
		bool showHorizontalScrollbar = false;
		iTextExtents = size{};
		uint32_t pass = 1;
		auto iterColumn = iGlyphColumns.begin();
		for (auto p = iGlyphParagraphs.begin(); p != iGlyphParagraphs.end();)
		{
			auto& column = *iterColumn;
			auto& lines = column.lines();
			auto& paragraph = *p;
			auto paragraphStart = paragraph.first.start();
			auto paragraphEnd = paragraph.first.end();
			if (paragraphStart == paragraphEnd || (paragraphStart->is_whitespace() && paragraphStart->value() == U'\n'))
			{
				auto lineStart = paragraphStart;
				auto lineEnd = lineStart;
				const auto& glyph = *lineStart;
				const auto& tagContents = iText.tag(iText.begin() + paragraph.first.text_start_index() + glyph.source().first).contents();
				const auto& style = tagContents.is<style_list::const_iterator>() ? *static_variant_cast<style_list::const_iterator>(tagContents) : iDefaultStyle;
				auto& glyphFont = style.font() != boost::none ? *style.font() : font();
				lines.push_back(
					glyph_line{
						{ p - iGlyphParagraphs.begin(), p },
						{ lineStart - iGlyphs.begin(), lineStart },
						{ lineEnd - iGlyphs.begin(), lineEnd },
						pos.y,
						{ 0.0, glyphFont.height() } });
				pos.y += glyphFont.height();
			}
			else if (iWordWrap && (paragraphEnd - 1)->x + (paragraphEnd - 1)->advance().cx > availableWidth)
			{
				auto insertionPoint = lines.end();
				bool first = true;
				auto next = paragraph.first.start();
				auto lineStart = next;
				auto lineEnd = paragraphEnd;
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
					dimension x = (split != iGlyphs.end() ? split->x : (lineStart != lineEnd ? iGlyphs.back().x + iGlyphs.back().advance().cx : 0.0));
					auto height = paragraph.first.height(lineStart, lineEnd);
					if (lineEnd != lineStart && (lineEnd - 1)->is_whitespace() && (lineEnd - 1)->value() == U'\n')
						--lineEnd;
					bool rtl = false;
					if (!first &&
						insertionPoint->lineStart != insertionPoint->lineEnd &&
						lineStart != lineEnd &&
						insertionPoint->lineStart.second->direction() == text_direction::RTL &&
						(lineEnd - 1)->direction() == text_direction::RTL)
						rtl = true; // todo: is this sufficient for multi-line RTL text?
					if (!rtl)
						insertionPoint = lines.end();
					insertionPoint = lines.insert(insertionPoint,
						glyph_line{
							{ p - iGlyphParagraphs.begin(), p },
							{ lineStart - iGlyphs.begin(), lineStart },
							{ lineEnd - iGlyphs.begin(), lineEnd },
							pos.y,
							{ x - offset, height } });
					if (rtl)
					{
						auto ypos = (insertionPoint + 1)->ypos;
						for (auto i = insertionPoint; i != lines.end(); ++i)
						{
							i->ypos = ypos;
							ypos += i->extents.cy;
						}
					}
					pos.y += height;
					iTextExtents.cx = std::max(iTextExtents.cx, x - offset);
					lineStart = next;
					if (lineStart != paragraphEnd)
						offset = lineStart->x;
					lineEnd = paragraphEnd;
					first = false;
				}
			}
			else
			{
				auto lineStart = paragraphStart;
				auto lineEnd = paragraphEnd;
				auto height = paragraph.first.height(lineStart, lineEnd);
				if (lineEnd != lineStart && (lineEnd - 1)->is_whitespace() && (lineEnd - 1)->value() == U'\n')
					--lineEnd;
				lines.push_back(
					glyph_line{
						{ p - iGlyphParagraphs.begin(), p },
						{ lineStart - iGlyphs.begin(), lineStart },
						{ lineEnd - iGlyphs.begin(), lineEnd },
						pos.y,
						{ (lineEnd - 1)->x + (lineEnd - 1)->advance().cx, height} });
				pos.y += lines.back().extents.cy;
				iTextExtents.cx = std::max(iTextExtents.cx, lines.back().extents.cx);
			}
			switch (pass)
			{
			case 1:
			case 3:
				if (!showVerticalScrollbar && pos.y >= availableHeight)
				{
					showVerticalScrollbar = true;
					availableWidth -= vertical_scrollbar().width(*this);
					lines.clear();
					pos = point{};
					iTextExtents = size{};
					p = iGlyphParagraphs.begin();
					++pass;
				}
				else
					++p;
				break;
			case 2:
				if (!showHorizontalScrollbar && iTextExtents.cx > availableWidth)
				{
					showHorizontalScrollbar = true;
					availableHeight -= horizontal_scrollbar().width(*this);
					lines.clear();
					pos = point{};
					iTextExtents = size{};
					p = iGlyphParagraphs.begin();
					++pass;
				}
				else
					++p;
				break;
			}
		}
		if (!iGlyphs.empty() && iGlyphs.back().is_whitespace() && iGlyphs.back().value() == U'\n')
			pos.y += font().height();
		iTextExtents.cy = pos.y;
	}

	void text_edit::animate()
	{
		if (has_focus())
			update_cursor();
	}

	void text_edit::update_cursor()
	{
		auto cursorPos = glyph_position(cursor_glyph_position(), true);
		dimension glyphHeight = 0.0;
		dimension lineHeight = 0.0;
		if (cursorPos.glyph != iGlyphs.end() && cursorPos.lineStart != cursorPos.lineEnd)
		{
			auto iterGlyph = cursorPos.glyph < cursorPos.lineEnd ? cursorPos.glyph : cursorPos.glyph - 1;
			const auto& glyph = *iterGlyph;
			if (cursorPos.glyph == cursorPos.lineEnd)
				cursorPos.pos.x += glyph.advance().cx;
			const auto& tagContents = iText.tag(iText.begin() + from_glyph(iterGlyph).first).contents();
			const auto& style = tagContents.is<style_list::const_iterator>() ? *static_variant_cast<style_list::const_iterator>(tagContents) : iDefaultStyle;
			auto& glyphFont = style.font() != boost::none ? *style.font() : font();
			glyphHeight = glyphFont.height();
			lineHeight = cursorPos.line->extents.cy;
		}
		else if (cursorPos.line != cursorPos.column->lines().end())
			glyphHeight = lineHeight = cursorPos.line->extents.cy;
		else
			glyphHeight = lineHeight = font().height();
		update(rect{ point{ cursorPos.pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + client_rect(false).top_left() + point{ 0.0, lineHeight - glyphHeight }, size{1.0, glyphHeight} });
	}

	void text_edit::make_cursor_visible(bool aForcePreviewScroll)
	{
		scoped_units su(*this, UnitsPixels);
		auto p = glyph_position(cursor_glyph_position(), true);
		auto e = (p.line != p.column->lines().end() ? 
			size{ p.glyph != p.lineEnd ? p.glyph->advance().cx : 0.0, p.line->extents.cy } : 
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

	text_edit::style text_edit::glyph_style(document_glyphs::const_iterator aGlyph, const glyph_column& aColumn) const
	{
		style result = iDefaultStyle;
		result.merge(aColumn.style());
		result.set_background_colour();
		const auto& tagContents = iText.tag(iText.begin() + from_glyph(aGlyph).first).contents();
		if (tagContents.is<style_list::const_iterator>())
			result.merge(*static_variant_cast<style_list::const_iterator>(tagContents));
		return result;
	}

	void text_edit::draw_glyphs(const graphics_context& aGraphicsContext, const point& aPoint, const glyph_column& aColumn, glyph_lines::const_iterator aLine) const
	{
		auto lineStart = aLine->lineStart.second;
		auto lineEnd = aLine->lineEnd.second;
		if (lineEnd != lineStart && (lineEnd - 1)->category() == text_category::Whitespace && (lineEnd - 1)->value() == U'\n')
			--lineEnd;
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
					bool selected = static_cast<cursor::position_type>(from_glyph(i).first) >= std::min(cursor().position(), cursor().anchor()) &&
						static_cast<cursor::position_type>(from_glyph(i).first) < std::max(cursor().position(), cursor().anchor());
					const auto& glyph = *i;
					const auto& style = glyph_style(i, aColumn);
					const auto& glyphFont = style.font() != boost::none ? *style.font() : font();
					switch (pass)
					{
					case 0:
						if (selected)
							aGraphicsContext.fill_rect(rect{ pos, size{glyph.advance().cx, aLine->extents.cy} }, 
								has_focus() ? 
									app::instance().current_style().selection_colour() : 
									app::instance().current_style().selection_colour().with_alpha(64));
						break;
					case 1:
						if (style.text_outline_colour().empty() || glyph.is_emoji())
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
							aGraphicsContext.draw_glyph(sOutlinePositions[outlinePos] + pos + glyph.offset() + point{ 0.0, aLine->extents.cy - glyphFont.height() - 1.0 }, glyph,
								glyphFont,
								style.text_outline_colour().is<colour>() ?
									static_variant_cast<const colour&>(style.text_outline_colour()) : style.text_outline_colour().is<gradient>() ?
										static_variant_cast<const gradient&>(style.text_outline_colour()).at((pos.x - margins().left + horizontal_scrollbar().position()) / std::max(client_rect(false).width(), iTextExtents.cx)) :
										default_text_colour());
						}
						break;
					case 2:
						aGraphicsContext.draw_glyph(pos + glyph.offset() + point{ 0.0, aLine->extents.cy - glyphFont.height() - (outlinesPresent ? 1.0 : 0.0)}, glyph,
							glyphFont,
							selected && has_focus() ? 
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
			const auto& style = glyph_style(i, aColumn);
			const auto& glyphFont = style.font() != boost::none ? *style.font() : font();
			if (glyph.underline())
				aGraphicsContext.draw_glyph_underline(pos + point{ 0.0, aLine->extents.cy - glyphFont.height() }, glyph,
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
		auto cursorGlyphIndex = cursor_glyph_position();
		auto cursorPos = glyph_position(cursorGlyphIndex, true);
		dimension glyphHeight = 0.0;
		dimension lineHeight = 0.0;
		if (cursorPos.glyph != iGlyphs.end() && cursorPos.lineStart != cursorPos.lineEnd)
		{
			auto iterGlyph = cursorPos.glyph < cursorPos.lineEnd ? cursorPos.glyph : cursorPos.glyph - 1;
			const auto& tagContents = iText.tag(iText.begin() + from_glyph(iterGlyph).first).contents();
			const auto& style = tagContents.is<style_list::const_iterator>() ? *static_variant_cast<style_list::const_iterator>(tagContents) : iDefaultStyle;
			auto& glyphFont = style.font() != boost::none ? *style.font() : font();
			glyphHeight = glyphFont.height();
			if (!style.text_outline_colour().empty())
				glyphHeight += 2.0;
			lineHeight = cursorPos.line->extents.cy;
		}
		else if (cursorPos.line != cursorPos.column->lines().end())
			glyphHeight = lineHeight = cursorPos.line->extents.cy;
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
			if (cursor().colour().empty())
			{
				aGraphicsContext.push_logical_operation(logical_operation::Xor);
				aGraphicsContext.draw_line(
					point{ cursorPos.pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + client_rect(false).top_left() + point{ 0.0, lineHeight },
					point{ cursorPos.pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + client_rect(false).top_left() + point{ 0.0, lineHeight - glyphHeight },
					pen{ colour::White.with_alpha(alpha), cursor().width() });
				aGraphicsContext.pop_logical_operation();
			}
			else if (cursor().colour().is<colour>())
			{
				aGraphicsContext.draw_line(
					point{ cursorPos.pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + client_rect(false).top_left() + point{ 0.0, lineHeight },
					point{ cursorPos.pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + client_rect(false).top_left() + point{ 0.0, lineHeight - glyphHeight },
					pen{ static_variant_cast<const colour&>(cursor().colour()).with_combined_alpha(alpha), cursor().width() });
			}
			else if (cursor().colour().is<gradient>())
			{
				aGraphicsContext.fill_rect(
					rect{
						point{ cursorPos.pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + client_rect(false).top_left() + point{ 0.0, lineHeight - glyphHeight},
						size{ cursor().width(), glyphHeight} },
					static_variant_cast<const gradient&>(cursor().colour()).with_combined_alpha(alpha));
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