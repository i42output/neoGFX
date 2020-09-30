// text_edit.cpp
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

#include <neogfx/neogfx.hpp>
#include <neolib/core/scoped.hpp>
#include <neolib/task/thread.hpp>
#include <neolib/app/i_power.hpp>
#include <neogfx/gui/widget/text_edit.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/text/text_category_map.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/core/easing.hpp>

namespace neogfx
{
    text_edit::style::style() :
        iParent{ nullptr },
        iUseCount{ 0 }
    {
    }
        
    text_edit::style::style(
        const optional_font& aFont,
        const color_or_gradient& aTextColor,
        const color_or_gradient& aPaperColor,
        const optional_text_effect& aTextEffect) :
        iParent{ nullptr },
        iUseCount{ 0 },
        iFont{ aFont },
        iTextColor{ aTextColor },
        iPaperColor{ aPaperColor },
        iTextEffect{ aTextEffect }
    {
    }

    text_edit::style::style(
        text_edit& aParent,
        const style& aOther) : 
        iParent{ &aParent },
        iUseCount{ 0 },
        iFont{ aOther.iFont },
        iTextColor{ aOther.iTextColor },
        iPaperColor{ aOther.iPaperColor },
        iTextEffect{ aOther.iTextEffect }
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

    const color_or_gradient& text_edit::style::glyph_color() const
    {
        return iGlyphColor;
    }

    const color_or_gradient& text_edit::style::text_color() const
    {
        return iTextColor;
    }

    const color_or_gradient& text_edit::style::paper_color() const
    {
        return iPaperColor;
    }

    const optional_text_effect& text_edit::style::text_effect() const
    {
        return iTextEffect;
    }

    void text_edit::style::set_font(const optional_font& aFont)
    {
        iFont = aFont;
        if (iParent)
            iParent->update();
    }

    void text_edit::style::set_glyph_color(const color_or_gradient& aColor)
    {
        iGlyphColor = aColor;
        if (iParent)
            iParent->update();
    }

    void text_edit::style::set_text_color(const color_or_gradient& aColor)
    {
        iTextColor = aColor;
        if (iParent)
            iParent->update();
    }

    void text_edit::style::set_paper_color(const color_or_gradient& aColor)
    {
        iPaperColor = aColor;
        if (iParent)
            iParent->update();
    }

    void text_edit::style::set_text_effect(const optional_text_effect& aEffect)
    {
        iTextEffect = aEffect;
        if (iParent)
            iParent->update();
    }

    text_edit::style& text_edit::style::merge(const style& aOverridingStyle)
    {
        if (aOverridingStyle.font() != std::nullopt)
            iFont = aOverridingStyle.font();
        if (aOverridingStyle.text_color() != neolib::none)
            iTextColor = aOverridingStyle.text_color();
        if (aOverridingStyle.paper_color() != neolib::none)
            iPaperColor = aOverridingStyle.paper_color();
        if (aOverridingStyle.text_effect() != std::nullopt)
            iTextEffect = aOverridingStyle.text_effect();
        if (iParent)
            iParent->update();
        return *this;
    }

    bool text_edit::style::operator==(const style& aRhs) const
    {
        return std::tie(iFont, iTextColor, iPaperColor, iTextEffect) == std::tie(aRhs.iFont, aRhs.iTextColor, aRhs.iPaperColor, aRhs.iTextEffect);
    }

    bool text_edit::style::operator!=(const style& aRhs) const
    {
        return !(*this == aRhs);
    }

    bool text_edit::style::operator<(const style& aRhs) const
    {
        return std::tie(iFont, iTextColor, iPaperColor, iTextEffect) < std::tie(aRhs.iFont, aRhs.iTextColor, aRhs.iPaperColor, aRhs.iTextEffect);
    }

    class text_edit::multiple_text_changes
    {
    public:
        multiple_text_changes(text_edit& aOwner) : 
            iOwner(aOwner)
        {
            ++iOwner.iSuppressTextChangedNotification;
        }
        ~multiple_text_changes()
        {
            if (--iOwner.iSuppressTextChangedNotification == 0u)
            {
                bool notify = (iOwner.iWantedToNotfiyTextChanged > 0u);
                iOwner.iWantedToNotfiyTextChanged = 0u;
                if (notify)
                    iOwner.TextChanged.trigger();
            }
        }
    private:
        text_edit & iOwner;
    };

    text_edit::text_edit(type_e aType, frame_style aFrameStyle) :
        framed_scrollable_widget{ aType == MultiLine ? scrollbar_style::Normal : scrollbar_style::Invisible, aFrameStyle },
        iType{ aType },
        iPersistDefaultStyle{ false },
        iGlyphColumns{ 1 },
        iCursorAnimationStartTime{ neolib::thread::program_elapsed_ms() },
        iTabStopHint{ "0000" },
        iAnimator{ service<i_async_task>(), [this](neolib::callback_timer&)
        {
            iAnimator.again();
            animate();
        }, 16 },
        iSuppressTextChangedNotification{ 0u },
        iWantedToNotfiyTextChanged{ 0u },
        iOutOfMemory{ false }
    {
        init();
    }

    text_edit::text_edit(i_widget& aParent, type_e aType, frame_style aFrameStyle) :
        framed_scrollable_widget{ aParent, aType == MultiLine ? scrollbar_style::Normal : scrollbar_style::Invisible, aFrameStyle },
        iType{ aType },
        iPersistDefaultStyle{ false },
        iGlyphColumns{ 1 },
        iCursorAnimationStartTime{ neolib::thread::program_elapsed_ms() },
        iTabStopHint{ "0000" },
        iAnimator{ service<i_async_task>(), [this](neolib::callback_timer&)
        {
            iAnimator.again();
            animate();
        }, 16 },
        iSuppressTextChangedNotification{ 0u },
        iWantedToNotfiyTextChanged{ 0u },
        iOutOfMemory{ false }
    {
        init();
    }

    text_edit::text_edit(i_layout& aLayout, type_e aType, frame_style aFrameStyle) :
        framed_scrollable_widget{ aLayout, aType == MultiLine ? scrollbar_style::Normal : scrollbar_style::Invisible, aFrameStyle },
        iType{ aType },
        iPersistDefaultStyle{ false },
        iGlyphColumns{ 1 },
        iCursorAnimationStartTime{ neolib::thread::program_elapsed_ms() },
        iTabStopHint{ "0000" },
        iAnimator{ service<i_async_task>(), [this](neolib::callback_timer&)
        {
            iAnimator.again();
            animate();
        }, 16 },
        iSuppressTextChangedNotification{ 0u },
        iWantedToNotfiyTextChanged{ 0u },
        iOutOfMemory{ false }
    {
        init();
    }

    text_edit::~text_edit()
    {
        if (service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
            service<i_clipboard>().deactivate(*this);
    }

    void text_edit::moved()
    {
        framed_scrollable_widget::moved();
    }

    void text_edit::resized()
    {
        framed_scrollable_widget::resized();
    }

    size text_edit::minimum_size(const optional_size& aAvailableSpace) const
    {
        if (has_minimum_size())
            return framed_scrollable_widget::minimum_size(aAvailableSpace);
        scoped_units su{ *this, units::Pixels };
        auto childLayoutSize = has_layout() ? layout().minimum_size(aAvailableSpace) : size{};
        auto result = framed_scrollable_widget::minimum_size(aAvailableSpace) - childLayoutSize;
        if (!size_hint())
            result += size{ font().height() }.max(childLayoutSize);
        else
        {
            if (iHintedSize == std::nullopt || iHintedSize->first != font())
            {
                iHintedSize.emplace(font(), size{});
                graphics_context gc{ *this, graphics_context::type::Unattached };
                iHintedSize->second = gc.text_extent(size_hint().primaryHint, font()).max(gc.text_extent(size_hint().secondaryHint, font()));
                if (iHintedSize->second.cy == 0.0)
                    iHintedSize->second.cy = font().height();
            }
            result += iHintedSize->second.max(childLayoutSize);
        }
        return to_units(*this, su.saved_units(), result);
    }

    size text_edit::maximum_size(const optional_size& aAvailableSpace) const
    {
        if (iType == MultiLine || has_maximum_size())
            return framed_scrollable_widget::maximum_size(aAvailableSpace);
        return size{ framed_scrollable_widget::maximum_size(aAvailableSpace).cx, minimum_size(aAvailableSpace).cy };
    }

    neogfx::padding text_edit::padding() const
    {
        auto result = framed_scrollable_widget::padding();
        if (default_style().text_effect())
        {
            scalar scale = 1.0;
            switch (default_style().text_effect()->type())
            {
            case text_effect_type::None:
            default:
                scale = 1.0;
            case text_effect_type::Outline:
                scale = 1.0;
            case text_effect_type::Glow:
            case text_effect_type::Shadow:
                scale = 0.5;
            }
            result += default_style().text_effect()->width() * scale;
        }
        return result.floor();
    }

    void draw_alpha_background(i_graphics_context& aGc, const rect& aRect, dimension aAlphaPatternSize = 4.0);

    void text_edit::paint(i_graphics_context& aGc) const
    {
        framed_scrollable_widget::paint(aGc);
        rect clipRect = default_clip_rect().intersection(client_rect(false));
        if (default_style().text_effect())
            clipRect.inflate(size{ default_style().text_effect()->width() });
        if (iOutOfMemory)
        {
            draw_alpha_background(aGc, clipRect);
            return;
        }
        scoped_scissor scissor{ aGc, clipRect };
        if (iDefaultStyle.paper_color() != neolib::none)
            aGc.fill_rect(client_rect(), to_brush(iDefaultStyle.paper_color()));
        coordinate x = 0.0;
        for (auto columnIndex = 0u; columnIndex < columns(); ++columnIndex)
        {
            auto const& column = static_cast<const glyph_column&>(text_edit::column(columnIndex));
            auto columnClipRect = clipRect.intersection(column_rect(columnIndex, true));
            if (column_style(columnIndex).text_effect())
                columnClipRect.inflate(size{ column_style(columnIndex).text_effect()->width() });
            scoped_scissor scissor2{ aGc, columnClipRect };
            auto const& columnRectSansPadding = column_rect(columnIndex);
            auto const& lines = column.lines();
            auto line = std::lower_bound(lines.begin(), lines.end(), glyph_line{ {}, {}, {}, vertical_scrollbar().position(), {} },
                [](const glyph_line& left, const glyph_line& right) { return left.ypos < right.ypos; });
            if (line != lines.begin() && (line == lines.end() || line->ypos > vertical_scrollbar().position()))
                --line;
            if (line == lines.end())
                continue;
            auto y = line->ypos;
            for (auto paintLine = line; paintLine != lines.end(); y += (paintLine++)->extents.cy)
            {
                point linePos = columnRectSansPadding.top_left() + point{ -horizontal_scrollbar().position(), y - vertical_scrollbar().position() };
                if (linePos.y + paintLine->extents.cy < columnRectSansPadding.top() || linePos.y + paintLine->extents.cy < update_rect().top())
                    continue;
                if (linePos.y > columnRectSansPadding.bottom() || linePos.y > update_rect().bottom())
                    break;
                auto textDirection = glyph_text_direction(paintLine->lineStart.second, paintLine->lineEnd.second);
                if (((Alignment & alignment::Horizontal) == alignment::Left && textDirection == text_direction::RTL) ||
                    ((Alignment & alignment::Horizontal) == alignment::Right && textDirection == text_direction::LTR))
                    linePos.x += aGc.from_device_units(size{ columnRectSansPadding.width() - paintLine->extents.cx, 0.0 }).cx;
                else if ((Alignment & alignment::Horizontal) == alignment::Center)
                    linePos.x += std::ceil((aGc.from_device_units(size{ columnRectSansPadding.width() - paintLine->extents.cx, 0.0 }).cx) / 2.0);
                draw_glyphs(aGc, linePos, column, paintLine);
            }
            x += column.width();
        }
        if (has_focus())
            draw_cursor(aGc);
    }

    color text_edit::palette_color(color_role aColorRole) const
    {
        if (has_palette_color(aColorRole))
            return framed_scrollable_widget::palette_color(aColorRole);
        if (aColorRole == color_role::Background)
            return palette_color(color_role::Base);
        return framed_scrollable_widget::palette_color(aColorRole);
    }

    const font& text_edit::font() const
    {
        return default_style().font() != std::nullopt ? *default_style().font() : framed_scrollable_widget::font();
    }

    void text_edit::set_font(const optional_font& aFont)
    {
        framed_scrollable_widget::set_font(aFont);
        if (!default_style().font())
            refresh_paragraph(iText.begin(), 0);
    }

    void text_edit::focus_gained(focus_reason aFocusReason)
    {
        framed_scrollable_widget::focus_gained(aFocusReason);
        neolib::service<neolib::i_power>().register_activity();
        service<i_clipboard>().activate(*this);
        iCursorAnimationStartTime = neolib::thread::program_elapsed_ms();
        if (iType == SingleLine && aFocusReason == focus_reason::Tab)
        {
            cursor().set_anchor(0);
            cursor().set_position(iText.size(), false);
        }
        update();
    }

    void text_edit::focus_lost(focus_reason aFocusReason)
    {
        destroyed_flag destroyed{ *this };
        framed_scrollable_widget::focus_lost(aFocusReason);
        if (destroyed)
            return;
        service<i_clipboard>().deactivate(*this);
        if (iType == SingleLine)
            cursor().set_position(iText.size());
        update();
    }

    void text_edit::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        framed_scrollable_widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left && client_rect().contains(aPosition))
            set_cursor_position(aPosition, (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE, capturing());
    }

    void text_edit::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        framed_scrollable_widget::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
        if (!password() && aButton == mouse_button::Left && client_rect().contains(aPosition))
        {
            auto word = word_at(document_hit_test(aPosition));
            cursor().set_anchor(word.first);
            cursor().set_position(word.second, false);
        }
    }
    
    void text_edit::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        framed_scrollable_widget::mouse_button_released(aButton, aPosition);
        iDragger = std::nullopt;
        if (aButton == mouse_button::Right)
        {
            iMenu = std::make_unique<context_menu>(*this, aPosition + non_client_rect().top_left() + root().window_position());
            iMenu->menu().add_action(service<i_app>().action_undo());
            iMenu->menu().add_action(service<i_app>().action_redo());
            iMenu->menu().add_separator();
            iMenu->menu().add_action(service<i_app>().action_cut());
            iMenu->menu().add_action(service<i_app>().action_copy());
            iMenu->menu().add_action(service<i_app>().action_paste());
            auto& pasteAs = iMenu->menu().add_sub_menu("Paste As"_t);
            auto pastePlainText = std::make_shared<action>("Plain Text"_t);
            auto pasteRichText = std::make_shared<action>("Rich Text (HTML)"_t);
            pastePlainText->Triggered([this]() { paste_plain_text(); });
            pasteRichText->Triggered([this]() { paste_rich_text(); });
            sink pasteAsSink;
            pasteAsSink += service<i_app>().action_paste().enabled([&pastePlainText, &pasteRichText]() { pastePlainText->enable(); pasteRichText->enable(); });
            pasteAsSink += service<i_app>().action_paste().disabled([&pastePlainText, &pasteRichText]() { pastePlainText->disable(); pasteRichText->disable(); });
            pasteAs.add_action(pastePlainText);
            pasteAs.add_action(pasteRichText);
            iMenu->menu().add_action(service<i_app>().action_delete());
            iMenu->menu().add_separator();
            iMenu->menu().add_action(service<i_app>().action_select_all());
            iMenu->exec();
            iMenu.reset();
        }
    }

    void text_edit::mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        framed_scrollable_widget::mouse_moved(aPosition, aKeyModifiers);
        neolib::service<neolib::i_power>().register_activity();
        if (iDragger != std::nullopt)
            set_cursor_position(aPosition, false);
    }

    void text_edit::mouse_entered(const point& aPosition)
    {
        framed_scrollable_widget::mouse_entered(aPosition);
    }

    void text_edit::mouse_left()
    {
        framed_scrollable_widget::mouse_left();
    }

    neogfx::mouse_cursor text_edit::mouse_cursor() const
    {
        return client_rect(false).contains(root().mouse_position() - origin()) || iDragger != std::nullopt ? mouse_system_cursor::IBeam : framed_scrollable_widget::mouse_cursor();
    }

    bool text_edit::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        bool handled = true;
        switch (aScanCode)
        {
        case ScanCode_TAB:
            {
                multiple_text_changes mtc{ *this };
                delete_any_selection();
                insert_text("\t");
                cursor().set_position(cursor().position() + 1);
            }
            break;
        case ScanCode_RETURN:
            if (iType == MultiLine)
            {
                multiple_text_changes mtc{ *this };
                delete_any_selection();
                insert_text("\n");
                cursor().set_position(cursor().position() + 1);
            }
            else
                handled = framed_scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
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
                    framed_scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
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
                    framed_scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
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
            if (iType == MultiLine)
            {
                if (aScanCode == ScanCode_PAGEUP && vertical_scrollbar().position() == vertical_scrollbar().minimum())
                    move_cursor(cursor::StartOfDocument, (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
                else if (aScanCode == ScanCode_PAGEDOWN && vertical_scrollbar().position() == vertical_scrollbar().maximum() - vertical_scrollbar().page())
                    move_cursor(cursor::EndOfDocument, (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
                else
                {
                    auto pos = point{ glyph_position(cursor_glyph_position()).pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } };
                    framed_scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
                    set_cursor_position(pos + client_rect(false).top_left(), (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE);
                }
            }
            else
                handled = false;
            break;
        case ScanCode_ESCAPE:
            if (cursor().anchor() != cursor().position())
                cursor().set_anchor(cursor().position());
            else if (iType == SingleLine)
                set_text(std::string{});
            break;
        default:
            handled = framed_scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
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
        if (aText[0] == '\r' || aText[0] == '\n' || aText[0] == '\t') // handled in key_pressed()
        {
            if ((focus_policy() & neogfx::focus_policy::ConsumeReturnKey) == neogfx::focus_policy::ConsumeReturnKey)
                return true;
            else
                return framed_scrollable_widget::text_input(aText);
        }
        multiple_text_changes mtc{ *this };
        delete_any_selection();
        insert_text(aText, true);
        return true;
    }

    scrolling_disposition text_edit::scrolling_disposition() const
    {
        return neogfx::scrolling_disposition::DontScrollChildWidget;
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
                framed_scrollable_widget::update_scrollbar_visibility(aStage);
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
                framed_scrollable_widget::update_scrollbar_visibility(aStage);
            }
            break;
        case UsvStageDone:
            make_cursor_visible();
            break;
        default:
            break;
        }
    }

    color text_edit::frame_color() const
    {
        if (service<i_app>().current_style().palette().color(color_role::Theme).similar_intensity(background_color(), 0.03125))
            return framed_scrollable_widget::frame_color();
        return service<i_app>().current_style().palette().color(color_role::Theme).mid(background_color());
    }

    bool text_edit::can_undo() const
    {
        return iPreviousText != iText;
    }

    bool text_edit::can_redo() const
    {
        return iPreviousText != iText;
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

    void text_edit::undo(i_clipboard&)
    {
        // todo: more intelligent undo
        iText.swap(iPreviousText);
        iUtf8TextCache = std::nullopt;

        refresh_paragraph(iText.begin(), 0);
        update();
        cursor().set_position(iText.size());
        if (iPreviousText != iText)
            notify_text_changed();
    }

    void text_edit::redo(i_clipboard& aClipboard)
    {
        // todo: more intelligent redo
        undo(aClipboard);
    }

    void text_edit::cut(i_clipboard& aClipboard)
    {
        if (cursor().position() != cursor().anchor())
        {
            copy(aClipboard);
            delete_selected();
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
        {
            multiple_text_changes mtc{ *this };
            if (cursor().position() != cursor().anchor())
                delete_selected();
            auto len = insert_text(aClipboard.text());
            cursor().set_position(cursor().position() + len);
        }
    }

    void text_edit::delete_selected()
    {
        multiple_text_changes mtc{ *this };
        if (cursor().position() != cursor().anchor())
            delete_any_selection();
        else if(cursor().position() < iText.size())
            delete_text(cursor().position(), cursor().position() + 1);
    }

    void text_edit::select_all()
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
        case cursor::Up:
        case cursor::Down:
            if (iCursorHint.x == std::nullopt)
                iCursorHint.x = glyph_position(cursor_glyph_position(), true).pos.x;
            break;
        default:
            iCursorHint.x = std::nullopt;
            break;
        }
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
                auto const& emojiAtlas = service<i_font_manager>().emoji_atlas();
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
                auto const& emojiAtlas = service<i_font_manager>().emoji_atlas();
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
                {
                    auto const columnRectSansPadding = column_rect(column_index(*currentPosition.column));
                    cursor().set_position(from_glyph(iGlyphs.begin() + document_hit_test(point{ *iCursorHint.x, std::prev(currentPosition.line)->ypos } +columnRectSansPadding.top_left(), false)).first, aMoveAnchor);
                }
            }
            break;
        case cursor::Down:
            {
                auto currentPosition = glyph_position(cursor_glyph_position());
                if (currentPosition.line != currentPosition.column->lines().end())
                {
                    auto const columnRectSansPadding = column_rect(column_index(*currentPosition.column));
                    if (std::next(currentPosition.line) != currentPosition.column->lines().end())
                        cursor().set_position(from_glyph(iGlyphs.begin() + document_hit_test(point{ *iCursorHint.x, std::next(currentPosition.line)->ypos } + columnRectSansPadding.top_left(), false)).first, aMoveAnchor);
                    else if (currentPosition.lineEnd != iGlyphs.end() && currentPosition.lineEnd->is_line_breaking_whitespace())
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

    std::string text_edit::plain_text() const
    {
        return text();
    }

    bool text_edit::set_plain_text(const std::string& aPlainText)
    {
        return set_text(aPlainText) != 0 || aPlainText.empty();
    }

    std::string text_edit::rich_text(rich_text_format aFormat) const
    {
        // todo
        return text();
    }

    bool text_edit::set_rich_text(const std::string& aRichText, rich_text_format aFormat)
    {
        // todo
        return set_text(aRichText) != 0 || aRichText.empty();
    }

    void text_edit::paste_plain_text()
    {
        paste(service<i_clipboard>());
    }

    void text_edit::paste_rich_text(rich_text_format aFormat)
    {
        // todo
        paste(service<i_clipboard>());
    }

    bool text_edit::read_only() const
    {
        return ReadOnly;
    }

    void text_edit::set_read_only(bool aReadOnly)
    {
        ReadOnly = aReadOnly;
        update();
    }

    bool text_edit::word_wrap() const
    {
        return WordWrap;
    }
    
    void text_edit::set_word_wrap(bool aWordWrap)
    {
        if (WordWrap != aWordWrap)
        {
            WordWrap = aWordWrap;
            refresh_columns();
        }
    }

    bool text_edit::password() const
    {
        return Password;
    }

    const std::string& text_edit::password_mask() const
    {
        return PasswordMask;
    }

    void text_edit::set_password(bool aPassword, const std::string& aMask)
    {
        if (Password != aPassword || PasswordMask != aMask)
        {
            Password = aPassword;
            PasswordMask = aMask;
            refresh_paragraph(iText.begin(), 0);
        }
    }

    neogfx::alignment text_edit::alignment() const
    {
        return Alignment;
    }

    void text_edit::set_alignment(neogfx::alignment aAlignment)
    {
        if (Alignment != aAlignment)
        {
            Alignment = aAlignment;
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
        auto oldEffect = (iDefaultStyle.text_effect() == std::nullopt);
        iDefaultStyle = aDefaultStyle;
        if (oldFont != font() || oldEffect != (iDefaultStyle.text_effect() == std::nullopt))
            refresh_paragraph(iText.begin(), 0);
        iPersistDefaultStyle = aPersist;
        DefaultStyleChanged.trigger();
        update();
    }

    color text_edit::default_text_color() const
    {
        if (std::holds_alternative<color>(default_style().text_color()))
            return static_variant_cast<const color&>(default_style().text_color());
        else if (std::holds_alternative<gradient>(default_style().text_color()))
            return static_variant_cast<const gradient&>(default_style().text_color()).at(0.0);
        else
            return service<i_app>().current_style().palette().default_text_color_for_widget(*this);
    }

    neogfx::cursor& text_edit::cursor() const
    {
        return iCursor;
    }

    void text_edit::set_cursor_position(const point& aPosition, bool aMoveAnchor, bool aEnableDragger)
    {
        set_cursor_glyph_position(document_hit_test(aPosition), aMoveAnchor);
        neolib::service<neolib::i_power>().register_activity();
        if (aEnableDragger)
        {
            if (!capturing())
                set_capture();
            iDragger.emplace(service<i_async_task>(), [this](neolib::callback_timer& aTimer)
            {
                aTimer.again();
                set_cursor_position(root().mouse_position() - origin(), false);
            }, 250);
        }
    }

    std::size_t text_edit::column_index(const column_info& aColumn) const
    {
        return static_cast<const glyph_column*>(&aColumn) - static_cast<const glyph_column*>(&column(0));
    }

    rect text_edit::column_rect(std::size_t aColumnIndex, bool aIncludePadding) const
    {
        auto result = client_rect(false);
        for (std::size_t ci = 0; ci < aColumnIndex; ++ci)
            result.x += static_cast<const glyph_column&>(column(ci)).width();
        if (!aIncludePadding)
            result.deflate(column(aColumnIndex).padding());
        return result;
    }

    std::size_t text_edit::column_hit_test(const point& aPosition, bool aAdjustForScrollPosition) const
    {
        auto ajustedPosition = (aAdjustForScrollPosition ? aPosition + point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } : aPosition);
        for (std::size_t ci = 0; ci < columns(); ++ci)
            if (column_rect(ci).contains(ajustedPosition))
                return ci;
        if (ajustedPosition.x < column_rect(0).left())
            return 0;
        else if (ajustedPosition.x >= column_rect(columns() - 1).right())
            return columns() - 1;
        return 0;
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
        auto const& columnRectSansPadding = column_rect(column - iGlyphColumns.begin());
        auto& lines = column->lines();
        if (line != lines.begin())
        {
            if (line == lines.end())
            {
                if (aGlyphPosition <= lines.back().lineEnd.first || iGlyphs.back().is_non_line_breaking_whitespace())
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
                auto paragraph = glyph_to_paragraph(aGlyphPosition);
                if (aGlyphPosition == iGlyphs.size() || aGlyphPosition == paragraph->first.end_index())
                {
                    auto iterChar = iText.begin() + from_glyph(iGlyphs.begin() + aGlyphPosition).first;
                    if (iterChar != iText.begin())
                    {
                        auto iterGlyph = to_glyph(iterChar - 1);
                        auto const& g = *iterGlyph;
                        if (g.direction() == text_direction::RTL || g.direction() == text_direction::Digit_RTL)
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
                delta alignmentAdjust;
                auto textDirection = glyph_text_direction(lines.back().lineStart.second, lines.back().lineEnd.second);
                if (((Alignment & alignment::Horizontal) == alignment::Left && textDirection == text_direction::RTL) ||
                    ((Alignment & alignment::Horizontal) == alignment::Right && textDirection == text_direction::LTR))
                    alignmentAdjust.dx = columnRectSansPadding.cx - line->extents.cx;
                else if ((Alignment & alignment::Horizontal) == alignment::Center)
                    alignmentAdjust.dx = (columnRectSansPadding.cx - line->extents.cx) / 2.0;
                if (lineStart != lineEnd)
                {
                    auto iterGlyph = iGlyphs.begin() + aGlyphPosition;
                    auto const& glyph = aGlyphPosition < lineEnd ? *iterGlyph : *(iterGlyph - 1);
                    point linePos{ glyph.x - line->lineStart.second->x, line->ypos };
                    if (placeCursorToRight)
                        linePos.x += glyph.advance().cx;
                    return position_info{ iterGlyph, column, line, iGlyphs.begin() + lineStart, iGlyphs.begin() + lineEnd, linePos + alignmentAdjust };
                }
                else
                    return position_info{ line->lineStart.second, column, line, iGlyphs.begin() + lineStart, iGlyphs.begin() + lineEnd, point{ 0.0, line->ypos } + alignmentAdjust };
            }
        }
        point pos;
        if (!lines.empty())
        {
            pos.x = 0.0;
            auto textDirection = glyph_text_direction(lines.back().lineStart.second, lines.back().lineEnd.second);
            if (((Alignment & alignment::Horizontal) == alignment::Left && textDirection == text_direction::RTL) ||
                ((Alignment & alignment::Horizontal) == alignment::Right && textDirection == text_direction::LTR))
                pos.x = columnRectSansPadding.cx;
            else if ((Alignment & alignment::Horizontal) == alignment::Center)
                pos.x = columnRectSansPadding.cx / 2.0;
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

    text_edit::position_type text_edit::document_hit_test(const point& aPosition, bool aAdjustForScrollPosition) const
    {
        auto columnIndex = column_hit_test(aPosition, aAdjustForScrollPosition);
        auto const& column = static_cast<const glyph_column&>(text_edit::column(columnIndex));
        auto const& columnRectSansPadding = column_rect(columnIndex);
        point adjustedPosition = (aAdjustForScrollPosition ? aPosition + point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } : aPosition) - columnRectSansPadding.top_left();
        adjustedPosition = adjustedPosition.max(point{});
        auto const& lines = column.lines();
        auto line = std::lower_bound(lines.begin(), lines.end(), glyph_line{ {}, {}, {}, adjustedPosition.y, {} },
            [](const glyph_line& left, const glyph_line& right) { return left.ypos < right.ypos; });
        if (line == lines.end() && !lines.empty() && adjustedPosition.y < lines.back().ypos + lines.back().extents.cy)
            --line;
        if (line != lines.end())
        {
            if (line != lines.begin() && adjustedPosition.y < line->ypos)
                --line;
            delta alignmentAdjust;
            auto textDirection = glyph_text_direction(lines.back().lineStart.second, lines.back().lineEnd.second);
            if (((Alignment & alignment::Horizontal) == alignment::Left && textDirection == text_direction::RTL) ||
                ((Alignment & alignment::Horizontal) == alignment::Right && textDirection == text_direction::LTR))
                alignmentAdjust.dx = columnRectSansPadding.cx - line->extents.cx;
            else if ((Alignment & alignment::Horizontal) == alignment::Center)
                alignmentAdjust.dx = (columnRectSansPadding.cx - line->extents.cx) / 2.0;
            adjustedPosition.x -= alignmentAdjust.dx;
            adjustedPosition = adjustedPosition.max(point{});
            auto lineStart = (line != lines.end() ? line->lineStart.first : iGlyphs.size());
            auto lineEnd = (line != lines.end() ? line->lineEnd.first : iGlyphs.size());
            auto lineStartX = line->lineStart.second->x;
            for (auto gi = line->lineStart.first; gi != lineEnd; ++gi)
            {
                auto& g = iGlyphs[gi];
                if (adjustedPosition.x >= g.x - lineStartX && adjustedPosition.x < g.x - lineStartX + g.advance().cx)
                {
                    if (g.direction() != text_direction::RTL)
                        return adjustedPosition.x < g.x - lineStartX + g.advance().cx / 2.0 ? gi : gi + 1;
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
        auto const& emojiAtlas = service<i_font_manager>().emoji_atlas();
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

    void text_edit::clear()
    {
        cursor().set_position(0);
        iText.clear();
        iGlyphs.clear();
        iGlyphParagraphs.clear();
        for (std::size_t i = 0; i < iGlyphColumns.size(); ++i)
            iGlyphColumns[i].lines().clear();
    }

    const std::string& text_edit::text() const
    {
        if (iUtf8TextCache == std::nullopt)
        {
            std::u32string u32text{ iText.begin(), iText.end() };
            iUtf8TextCache = neolib::utf32_to_utf8(u32text);
        }
        return *iUtf8TextCache;
    }

    std::size_t text_edit::set_text(const std::string& aText)
    {
        return set_text(aText, default_style());
    }

    std::size_t text_edit::set_text(const std::string& aText, const style& aStyle)
    {
        return do_insert_text(0, aText, aStyle, true, true);
    }

    std::size_t text_edit::append_text(const std::string& aText, bool aMoveCursor)
    {
        return do_insert_text(cursor().position(), aText, default_style(), aMoveCursor, false);
    }

    std::size_t text_edit::append_text(const std::string& aText, const style& aStyle, bool aMoveCursor)
    {
        return do_insert_text(cursor().position(), aText, aStyle, aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(const std::string& aText, bool aMoveCursor)
    {
        return do_insert_text(cursor().position(), aText, default_style(), aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(const std::string& aText, const style& aStyle, bool aMoveCursor)
    {
        return do_insert_text(cursor().position(), aText, aStyle, aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(position_type aPosition, const std::string& aText, bool aMoveCursor)
    {
        return do_insert_text(aPosition, aText, default_style(), aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(position_type aPosition, const std::string& aText, const style& aStyle, bool aMoveCursor)
    {
        return do_insert_text(aPosition, aText, aStyle, aMoveCursor, false);
    }

    void text_edit::delete_text(position_type aStart, position_type aEnd)
    {
        if (aStart == aEnd)
            return;

        auto eraseBegin = iText.begin() + aStart;
        auto eraseEnd = iText.begin() + aEnd;
        auto eraseAmount = eraseEnd - eraseBegin;

        iPreviousText = iText;
        iUtf8TextCache = std::nullopt;

        refresh_paragraph(iText.erase(eraseBegin, eraseEnd), -eraseAmount);
        update();
        if (iPreviousText != iText)
            notify_text_changed();
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

    bool text_edit::same_paragraph(position_type aFirstGlyphPos, position_type aSecondGlyphPos) const
    {
        return glyph_to_paragraph(aFirstGlyphPos) == glyph_to_paragraph(aSecondGlyphPos);
    }

    text_edit::glyph_paragraphs::const_iterator text_edit::character_to_paragraph(position_type aCharacterPos) const
    {
        if (iCharacterToParagraphCacheLastAccess != std::nullopt &&
            aCharacterPos >= (*iCharacterToParagraphCacheLastAccess)->first.first && aCharacterPos < (*iCharacterToParagraphCacheLastAccess)->first.second)
            return (*iCharacterToParagraphCacheLastAccess)->second;
        auto existing = iCharacterToParagraphCache.lower_bound(std::make_pair(aCharacterPos, aCharacterPos));
        if (existing != iCharacterToParagraphCache.end() && aCharacterPos >= existing->first.first && aCharacterPos < existing->first.second)
            return (*(iCharacterToParagraphCacheLastAccess = existing))->second;
        auto gp = iGlyphParagraphs.find_by_foreign_index(glyph_paragraph_index{ aCharacterPos, 0 }, [](const glyph_paragraph_index& aLhs, const glyph_paragraph_index& aRhs) { return aLhs.characters() < aRhs.characters(); }).first;
        if (gp != iGlyphParagraphs.end())
            (*(iCharacterToParagraphCacheLastAccess = iCharacterToParagraphCache.insert(std::make_pair(std::make_pair(gp->first.text_start_index(), gp->first.text_end_index()), gp)).first));
        return gp;
    }

    text_edit::glyph_paragraphs::const_iterator text_edit::glyph_to_paragraph(position_type aGlyphPos) const
    {
        if (iGlyphToParagraphCacheLastAccess != std::nullopt &&
            aGlyphPos >= (*iGlyphToParagraphCacheLastAccess)->first.first && aGlyphPos < (*iGlyphToParagraphCacheLastAccess)->first.second)
            return (*iGlyphToParagraphCacheLastAccess)->second;
        auto existing = iGlyphToParagraphCache.lower_bound(std::make_pair(aGlyphPos, aGlyphPos));
        if (existing != iGlyphToParagraphCache.end() && aGlyphPos >= existing->first.first && aGlyphPos < existing->first.second)
            return (*(iGlyphToParagraphCacheLastAccess = existing))->second;
        auto gp = iGlyphParagraphs.find_by_foreign_index(glyph_paragraph_index{ 0, aGlyphPos }, [](const glyph_paragraph_index& aLhs, const glyph_paragraph_index& aRhs) { return aLhs.glyphs() < aRhs.glyphs(); }).first;
        if (gp != iGlyphParagraphs.end())
            (*(iGlyphToParagraphCacheLastAccess = iGlyphToParagraphCache.insert(std::make_pair(std::make_pair(gp->first.start_index(), gp->first.end_index()), gp)).first));
        return gp;
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

    const text_edit::column_info& text_edit::column(std::size_t aColumnIndex) const
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

    const text_edit::style& text_edit::column_style(std::size_t aColumnIndex) const
    {
        return column_style(column(aColumnIndex));
    }

    const text_edit::style& text_edit::column_style(const column_info& aColumn) const
    {
        if (aColumn.style())
            return *aColumn.style();
        return default_style();
    }

    const neogfx::size_hint& text_edit::size_hint() const
    {       
        return iSizeHint;
    }

    void text_edit::set_size_hint(const neogfx::size_hint& aHint)
    {
        if (iSizeHint != aHint)
        {
            iSizeHint = aHint;
            iHintedSize = std::nullopt;
            update_layout();
            update();
        }
    }

    dimension text_edit::tab_stops() const
    {
        if (iCalculatedTabStops == std::nullopt || iCalculatedTabStops->first != font())
            iCalculatedTabStops = std::make_pair(font(), (iTabStops != std::nullopt ?
                *iTabStops : 
                graphics_context{ *this, graphics_context::type::Unattached }.text_extent(iTabStopHint, font()).cx));
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
        optional_dimension newTabStops = (aTabStops != std::nullopt ? units_converter(*this).to_device_units(size{ *aTabStops, 0.0 }).cx : optional_dimension{});
        if (iTabStops != newTabStops)
        {
            iTabStops = newTabStops;
            refresh_columns();
        }
        iCalculatedTabStops.reset();
    }

    void text_edit::init()
    {
        iSink += neolib::service<neolib::i_power>().green_mode_entered([this]()
        {
            if (has_focus())
                update_cursor();
        });
        iDefaultFont = service<i_app>().current_style().font_info();
        iSink += service<i_app>().current_style_changed([this](style_aspect)
        {
            if (iDefaultFont != service<i_app>().current_style().font_info())
            {
                iDefaultFont = service<i_app>().current_style().font_info();
                refresh_paragraph(iText.begin(), 0);
            }
        });
        iSink += service<i_rendering_engine>().subpixel_rendering_changed([this]()
        {
            refresh_paragraph(iText.begin(), 0);
        });
        auto focusPolicy = neogfx::focus_policy::ClickTabFocus;
        if (iType == MultiLine)
            focusPolicy |= neogfx::focus_policy::ConsumeReturnKey;
        set_focus_policy(focusPolicy);
        cursor().set_width(2.0);
        iSink += cursor().PositionChanged([this]()
        {
            iCursorAnimationStartTime = neolib::thread::program_elapsed_ms();
            make_cursor_visible();
            update();
        });
        iSink += cursor().AnchorChanged([this]()
        {
            update();
        });
        iSink += cursor().AppearanceChanged([this]()
        {
            update();
        });
    }

    std::size_t text_edit::do_insert_text(position_type aPosition, const std::string& aText, const style& aStyle, bool aMoveCursor, bool aClearFirst)
    {
        bool accept = true;
        TextFilter.trigger(aText, accept);
        if (!accept)
            return 0;

        iPreviousText = iText;
        iUtf8TextCache = std::nullopt;

        if (aClearFirst)
            iText.clear();

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
        auto insertionPoint = iText.begin() + aPosition;
        insertionPoint = iText.insert(s != iStyles.end() ? document_text::tag_type::tag_data{ static_cast<style_list::const_iterator>(s) } : document_text::tag_type::tag_data{ nullptr },
            insertionPoint, iNormalizedTextBuffer.begin(), iNormalizedTextBuffer.begin() + eos);
        refresh_paragraph(insertionPoint, eos);
        update();
        if (aMoveCursor)
            cursor().set_position(insertionPoint - iText.begin() + eos);
        if (iPreviousText != iText)
            notify_text_changed();
        return eos;
    }

    void text_edit::delete_any_selection()
    {
        if (cursor().position() != cursor().anchor())
        {
            delete_text(std::min(cursor().position(), cursor().anchor()), std::max(cursor().position(), cursor().anchor()));
            cursor().set_position(std::min(cursor().position(), cursor().anchor()));
        }
    }

    void text_edit::notify_text_changed()
    {
        if (!iSuppressTextChangedNotification)
            TextChanged.trigger();
        else
            ++iWantedToNotfiyTextChanged;
    }

    text_edit::document_glyphs::const_iterator text_edit::to_glyph(document_text::const_iterator aWhere) const
    {
        std::size_t textIndex = static_cast<std::size_t>(aWhere - iText.begin());
        auto paragraph = character_to_paragraph(textIndex);
        if (paragraph == iGlyphParagraphs.end())
            return iGlyphs.end();
        auto paragraphEnd = paragraph->first.end();
        for (auto i = paragraph->first.start(); i != paragraphEnd; ++i)
        {
            auto const& g = *i;
            auto si = paragraph->first.text_start_index();
            auto start = g.source().first + si;
            auto end = g.source().second + si;
            if (textIndex >= start && textIndex < end)
                return i;
        }
        return paragraphEnd;
    }

    std::pair<text_edit::document_text::size_type, text_edit::document_text::size_type> text_edit::from_glyph(document_glyphs::const_iterator aWhere) const
    {
        if (aWhere == iGlyphs.end())
            return std::make_pair(iText.size(), iText.size());
        auto paragraph = glyph_to_paragraph(aWhere - iGlyphs.begin());
        if (paragraph == iGlyphParagraphs.end() && paragraph != iGlyphParagraphs.begin() && aWhere <= (paragraph - 1)->first.end())
            --paragraph;
        if (paragraph != iGlyphParagraphs.end())
        {
            if (paragraph->first.start() > aWhere)
                --paragraph;
            auto textStart = paragraph->first.text_start_index();
            auto rg = related_glyphs(aWhere - iGlyphs.begin());
            return std::make_pair(textStart + iGlyphs[rg.first].source().first, textStart + iGlyphs[rg.second - 1].source().second);
        }
        return std::make_pair(iText.size(), iText.size());
    }

    void text_edit::refresh_paragraph(document_text::const_iterator aWhere, ptrdiff_t aDelta)
    {
        /* simple (naive) implementation just to get things moving (so just refresh everything) ... */
        (void)aWhere;
        graphics_context gc{ *this, graphics_context::type::Unattached };
        if (password())
            gc.set_password(true, PasswordMask.value().empty() ? "\xE2\x97\x8F"s : PasswordMask);
        iGlyphs.clear();
        iGlyphParagraphs.clear();
        iCharacterToParagraphCache.clear();
        iCharacterToParagraphCacheLastAccess.reset();
        iGlyphToParagraphCache.clear();
        iGlyphToParagraphCacheLastAccess.reset();
        std::u32string paragraphBuffer;
        auto paragraphStart = iText.begin();
        auto iterColumn = iGlyphColumns.begin();
        neolib::vecarray<std::u32string::size_type, 16, -1> columnDelimiters;
        auto fs = [this, paragraphStart, &columnDelimiters](std::u32string::size_type aSourceIndex)
        {
            auto const& tagStyle = iText.tag(paragraphStart + aSourceIndex).style();
            std::size_t indexColumn = std::lower_bound(columnDelimiters.begin(), columnDelimiters.end(), aSourceIndex) - columnDelimiters.begin();
            if (indexColumn > columns() - 1)
                indexColumn = columns() - 1;
            auto const& columnStyle = column_style(indexColumn);
            auto const& style =
                std::holds_alternative<style_list::const_iterator>(tagStyle) ? *static_variant_cast<style_list::const_iterator>(tagStyle) :
                columnStyle.font() != std::nullopt ? columnStyle : iDefaultStyle;
            return style.font() != std::nullopt ? *style.font() : font();
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
                if (gt.cbegin() != gt.cend())
                {
                    auto paragraphGlyphs = iGlyphs.insert(iGlyphs.end(), gt.cbegin(), gt.cend());
                    for (auto& newGlyph : gt)
                        iGlyphs.cache_glyph_font(newGlyph.font_id());
                    auto newParagraph = iGlyphParagraphs.insert(iGlyphParagraphs.end(),
                        std::make_pair(
                            glyph_paragraph{ *this },
                            glyph_paragraph_index{
                                static_cast<std::size_t>((iterChar + 1) - iText.begin()) - static_cast<std::size_t>(paragraphStart - iText.begin()),
                                iGlyphs.size() - static_cast<std::size_t>(paragraphGlyphs - iGlyphs.begin()) }),
                                glyph_paragraphs::skip_type{ glyph_paragraph_index{}, glyph_paragraph_index{} });
                    newParagraph->first.set_self(newParagraph);
                }
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
                auto& glyph = *iterGlyph;
                if (iText[glyph.source().first] == iterColumn->delimiter() && iterColumn + 1 != iGlyphColumns.end())
                {
                    glyph.set_advance(size{});
                    ++iterColumn;
                    continue;
                }
                else if (glyph.is_whitespace())
                {
                    if (glyph.value() == U'\t')
                    {
                        auto advance = glyph.advance();
                        advance.cx = tab_stops() - std::fmod(x, tab_stops());
                        glyph.set_advance(advance);
                    }
                    else if (glyph.is_line_breaking_whitespace())
                    {
                        glyph.set_advance(size{});
                    }
                }
                glyph.x = x;
                x += glyph.advance().cx;
            }
        }
        refresh_columns();
    }

    void text_edit::refresh_columns()
    {
        update_scrollbar_visibility();
        update();
    }

    void text_edit::refresh_lines()
    {
        try
        {
            /* simple (naive) implementation just to get things moving... */
            iOutOfMemory = false;
            for (auto& column : iGlyphColumns)
                column.lines().clear();
            point pos{};
            dimension availableWidth = column_rect(0).width(); // todo: columns
            dimension availableHeight = column_rect(0).height();
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
                if (paragraphStart == paragraphEnd || paragraphStart->is_line_breaking_whitespace())
                {
                    auto lineStart = paragraphStart;
                    auto lineEnd = lineStart;
                    auto const& glyph = *lineStart;
                    auto const& glyphFont = glyph.font(iGlyphs);
                    auto height = paragraph.first.height(lineStart, lineEnd);
                    lines.push_back(
                        glyph_line{
                            { p - iGlyphParagraphs.begin(), p },
                            { lineStart - iGlyphs.begin(), lineStart },
                            { lineEnd - iGlyphs.begin(), lineEnd },
                            pos.y,
                            { 0.0, height } });
                    pos.y += glyphFont.height();
                }
                else if (WordWrap && (paragraphEnd - 1)->x + (paragraphEnd - 1)->advance().cx > availableWidth)
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
                        if (lineEnd != lineStart && (lineEnd - 1)->is_line_breaking_whitespace())
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
                    if (lineEnd != lineStart && (lineEnd - 1)->is_line_breaking_whitespace())
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
                if (p + 1 == iGlyphParagraphs.end() && !iGlyphs.empty() && iGlyphs.back().is_line_breaking_whitespace())
                    pos.y += font().height();
                auto next_pass = [&]()
                {
                    if (pass <= 3)
                    {
                        lines.clear();
                        pos = point{};
                        iTextExtents = size{};
                        p = iGlyphParagraphs.begin();
                        ++pass;
                    }
                };
                switch (pass)
                {
                case 1:
                case 3:
                    if (!showVerticalScrollbar && pos.y >= availableHeight)
                    {
                        showVerticalScrollbar = true;
                        availableWidth -= vertical_scrollbar().width();
                        next_pass();
                    }
                    else if (++p == iGlyphParagraphs.end() && pass == 1)
                        next_pass();
                    break;
                case 2:
                    if (!showHorizontalScrollbar && iTextExtents.cx > availableWidth)
                    {
                        showHorizontalScrollbar = true;
                        availableHeight -= horizontal_scrollbar().width();
                        next_pass();
                    }
                    else if (++p == iGlyphParagraphs.end())
                        next_pass();
                    break;
                case 4:
                    ++p;
                    break;
                }
            }
            iTextExtents.cy = pos.y;
        }
        catch (std::bad_alloc)
        {
            for (auto& column : iGlyphColumns)
                column.lines().clear();
            iOutOfMemory = true;
        }
    }

    void text_edit::animate()
    {
        if (neolib::service<neolib::i_power>().green_mode_active())
            return;
        if (has_focus())
            update_cursor();
    }

    void text_edit::update_cursor()
    {
        update(cursor_rect());
    }

    void text_edit::make_cursor_visible(bool aForcePreviewScroll)
    {
        scoped_units su{ *this, units::Pixels };
        auto p = glyph_position(cursor_glyph_position(), true);
        auto e = (p.line != p.column->lines().end() ? 
            size{ p.glyph != p.lineEnd ? p.glyph->advance().cx : 0.0, p.line->extents.cy } : 
            size{ 0.0, font().height() });
        e.cy = std::min(e.cy, vertical_scrollbar().page());
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
        result.merge(column_style(aColumn));
        result.set_paper_color();
        auto const& tagStyle = iText.tag(iText.begin() + from_glyph(aGlyph).first).style();
        if (std::holds_alternative<style_list::const_iterator>(tagStyle))
            result.merge(*static_variant_cast<style_list::const_iterator>(tagStyle));
        return result;
    }

    void text_edit::draw_glyphs(const i_graphics_context& aGc, const point& aPosition, const glyph_column& aColumn, glyph_lines::const_iterator aLine) const
    {
        auto lineStart = aLine->lineStart.second;
        auto lineEnd = aLine->lineEnd.second;
        if (lineEnd != lineStart && (lineEnd - 1)->is_line_breaking_whitespace())
            --lineEnd;
        {
            thread_local glyph_text tGlyphText;
            optional_text_appearance textAppearance;
            point textPos = aPosition;
            point glyphPos = aPosition;
            for (document_glyphs::const_iterator i = lineStart; i != lineEnd; ++i)
            {
                bool selected = false;
                if (cursor().position() != cursor().anchor())
                {
                    auto gp = static_cast<cursor::position_type>(from_glyph(i).first);
                    selected = (gp >= std::min(cursor().position(), cursor().anchor()) && gp < std::max(cursor().position(), cursor().anchor()));
                }
                auto const& glyph = *i;
                auto const& style = glyph_style(i, aColumn);
                auto const& glyphFont = glyph.font(iGlyphs);
                auto nextTextAppearance = selected ?
                    text_appearance{
                        service<i_app>().current_style().palette().color(color_role::Selection).light() ? color::Black : color::White,
                        has_focus() ? service<i_app>().current_style().palette().color(color_role::Selection) : service<i_app>().current_style().palette().color(color_role::Selection).with_alpha(64) } :
                    text_appearance{
                        with_bounding_box(style.glyph_color() == neolib::none ? style.text_color() != neolib::none ? with_bounding_box(style.text_color(), column_rect(column_index(aColumn))) : default_text_color() : style.glyph_color(), client_rect(), true),
                        style.paper_color() != neolib::none ? optional_text_color{ neogfx::text_color{ style.paper_color() } } :  optional_text_color{}, 
                        style.text_effect() };
                if (textAppearance != std::nullopt && *textAppearance != nextTextAppearance)
                {
                    aGc.draw_glyph_text(textPos, tGlyphText, *textAppearance);
                    tGlyphText = glyph_text{};
                    textPos = glyphPos;
                }
                tGlyphText.push_back(glyph);
                tGlyphText.cache_glyph_font(glyphFont);
                textAppearance = nextTextAppearance;
                glyphPos.x += glyph.advance().cx;
            }
            if (!tGlyphText.empty())
            {
                aGc.draw_glyph_text(textPos, tGlyphText, *textAppearance);
                tGlyphText = glyph_text{};
            }
        }
    }

    void text_edit::draw_cursor(const i_graphics_context& aGc) const
    {
        auto elapsedTime_ms = (neolib::thread::program_elapsed_ms() - iCursorAnimationStartTime);
        auto const flashInterval_ms = cursor().flash_interval().count();
        auto const normalizedFrameTime = (elapsedTime_ms % flashInterval_ms) / ((flashInterval_ms - 1) * 1.0);
        auto const cursorAlpha = neolib::service<neolib::i_power>().green_mode_active() ? 1.0 : partitioned_ease(easing::InvertedInOutQuint, easing::InOutQuint, normalizedFrameTime);
        auto cursorColor = cursor().color();
        if (cursorColor == neolib::none && cursor().style() == cursor_style::Standard)
            cursorColor = service<i_app>().current_style().palette().default_text_color_for_widget(*this);
        if (cursorColor == neolib::none)
        {
            aGc.push_logical_operation(logical_operation::Xor);
            aGc.fill_rect(cursor_rect(), color::White.with_combined_alpha(cursorAlpha));
            aGc.pop_logical_operation();
        }
        else if (std::holds_alternative<color>(cursorColor))
        {
            aGc.fill_rect(cursor_rect(), static_variant_cast<const color&>(cursorColor).with_combined_alpha(cursorAlpha));
        }
        else if (std::holds_alternative<gradient>(cursorColor))
        {
            aGc.fill_rect(cursor_rect(), static_variant_cast<const gradient&>(cursorColor).with_combined_alpha(cursorAlpha));
        }
    }

    rect text_edit::cursor_rect() const
    {
        auto cursorGlyphIndex = cursor_glyph_position();
        auto cursorPos = glyph_position(cursorGlyphIndex, true);
        dimension glyphHeight = 0.0;
        dimension lineHeight = 0.0;
        if (cursorPos.glyph != iGlyphs.end() && cursorPos.lineStart != cursorPos.lineEnd)
        {
            auto iterGlyph = cursorPos.glyph < cursorPos.lineEnd ? cursorPos.glyph : cursorPos.glyph - 1;
            auto const& glyph = *iterGlyph;
            auto const& glyphFont = glyph.font(iGlyphs);
            glyphHeight = glyphFont.height();
            lineHeight = cursorPos.line->extents.cy;
        }
        else if (cursorPos.line != cursorPos.column->lines().end())
            glyphHeight = lineHeight = cursorPos.line->extents.cy;
        else
            glyphHeight = lineHeight = font().height();
        auto const columnRectSansPadding = column_rect(column_index(*cursorPos.column));
        rect cursorRect{ point{ cursorPos.pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + columnRectSansPadding.top_left() + point{ 0.0, lineHeight - glyphHeight },
            size{ cursor().width(), glyphHeight } };
        if (cursorRect.right() > columnRectSansPadding.right())
            cursorRect.x += (columnRectSansPadding.right() - cursorRect.right());
        return cursorRect;
    }

    std::pair<text_edit::document_glyphs::iterator, text_edit::document_glyphs::iterator> text_edit::word_break(document_glyphs::iterator aBegin, document_glyphs::iterator aFrom, document_glyphs::iterator aEnd)
    {
        std::pair<document_glyphs::iterator, document_glyphs::iterator> result{ aFrom, aFrom };
        if (!aFrom->is_whitespace())
        {
            while (result.first != aBegin && !(result.first - 1)->is_whitespace())
                --result.first;
            result.second = result.first;
        }
        else 
        {
            while (result.first != aBegin && (result.first - 1)->is_whitespace())
                --result.first;
            while (result.first != aBegin && !(result.first - 1)->is_whitespace())
                --result.first;
            result.second = result.first;
        }
        if (result.second != aEnd && result.second->is_line_breaking_whitespace())
            ++result.second;
        if (result.first == result.second && result.first == aBegin)
            result = { aFrom, aFrom };
        return result;
    }
}