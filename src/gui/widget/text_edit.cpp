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
#include <boost/algorithm/string/find.hpp>
#include <neolib/core/scoped.hpp>
#include <neolib/task/thread.hpp>
#include <neolib/app/i_power.hpp>
#include <neogfx/gui/widget/text_edit.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/text/text_category_map.hpp>
#include <neogfx/gfx/text/glyph.ipp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/core/easing.hpp>

namespace neogfx
{
    template class basic_glyph_text_content<text_edit::glyph_container_type>;

    text_edit::character_style::character_style() :
        iIgnoreEmoji{ true }
    {
    }

    text_edit::character_style::character_style(character_style const& aOther) :
        iFont{ aOther.iFont },
        iGlyphColor{ aOther.iGlyphColor },
        iTextColor{ aOther.iTextColor },
        iPaperColor{ aOther.iPaperColor },
        iIgnoreEmoji{ true },
        iTextEffect{ aOther.iTextEffect }
    {
    }
        
    text_edit::character_style::character_style(
        optional_font const& aFont,
        const color_or_gradient& aTextColor,
        const color_or_gradient& aPaperColor,
        const optional_text_effect& aTextEffect) :
        iFont{ aFont },
        iTextColor{ aTextColor },
        iPaperColor{ aPaperColor },
        iIgnoreEmoji{ true },
        iTextEffect{ aTextEffect }
    {
    }

    optional_font const& text_edit::character_style::font() const
    {
        return iFont;
    }

    const color_or_gradient& text_edit::character_style::glyph_color() const
    {
        return iGlyphColor;
    }

    const color_or_gradient& text_edit::character_style::text_color() const
    {
        return iTextColor;
    }

    const color_or_gradient& text_edit::character_style::paper_color() const
    {
        return iPaperColor;
    }

    bool text_edit::character_style::ignore_emoji() const
    {
        return iIgnoreEmoji;
    }

    const optional_text_effect& text_edit::character_style::text_effect() const
    {
        return iTextEffect;
    }

    text_appearance text_edit::character_style::as_text_appearance() const
    {
        return text_appearance{ glyph_color() != neolib::none ? glyph_color() : text_color(), paper_color() != neolib::none ? paper_color() : optional_text_color{}, text_effect() }.with_emoji_ignored(ignore_emoji());
    }

    text_edit::character_style& text_edit::character_style::set_font(optional_font const& aFont)
    {
        iFont = aFont;
        return *this;
    }

    text_edit::character_style& text_edit::character_style::set_font_if_none(neogfx::font const& aFont)
    {
        if (iFont == std::nullopt)
            iFont = aFont;
        return *this;
    }

    text_edit::character_style& text_edit::character_style::set_glyph_color(const color_or_gradient& aColor)
    {
        iGlyphColor = aColor;
        return *this;
    }

    text_edit::character_style& text_edit::character_style::set_text_color(const color_or_gradient& aColor)
    {
        iTextColor = aColor;
        return *this;
    }

    text_edit::character_style& text_edit::character_style::set_paper_color(const color_or_gradient& aColor)
    {
        iPaperColor = aColor;
        return *this;
    }

    text_edit::character_style& text_edit::character_style::set_text_effect(const optional_text_effect& aEffect)
    {
        iTextEffect = aEffect;
        return *this;
    }

    text_edit::character_style& text_edit::character_style::set_from_text_appearance(const text_appearance& aAppearance)
    {
        iGlyphColor = aAppearance.ink();
        iTextColor = neolib::none; // todo
        iPaperColor = aAppearance.paper() ? *aAppearance.paper() : neolib::none;
        iIgnoreEmoji = aAppearance.ignore_emoji();
        iTextEffect = aAppearance.effect();
        return *this;
    }

    text_edit::character_style& text_edit::character_style::merge(const character_style& aRhs)
    {
        if (aRhs.font() != std::nullopt)
            iFont = aRhs.font();
        if (aRhs.glyph_color() != neolib::none)
            iGlyphColor = aRhs.glyph_color();
        if (aRhs.text_color() != neolib::none)
            iTextColor = aRhs.text_color();
        if (aRhs.paper_color() != neolib::none)
            iPaperColor = aRhs.paper_color();
        iIgnoreEmoji = aRhs.ignore_emoji();
        if (aRhs.text_effect() != std::nullopt)
            iTextEffect = aRhs.text_effect();
        return *this;
    }

    bool text_edit::character_style::operator==(const character_style& aRhs) const
    {
        return std::tie(iFont, iGlyphColor, iTextColor, iPaperColor, iTextEffect) == std::tie(aRhs.iFont, aRhs.iGlyphColor, aRhs.iTextColor, aRhs.iPaperColor, aRhs.iTextEffect);
    }

    bool text_edit::character_style::operator!=(const character_style& aRhs) const
    {
        return !(*this == aRhs);
    }

    bool text_edit::character_style::operator<(const character_style& aRhs) const
    {
        return std::tie(iFont, iGlyphColor, iTextColor, iPaperColor, iTextEffect) < std::tie(aRhs.iFont, aRhs.iGlyphColor, aRhs.iTextColor, aRhs.iPaperColor, aRhs.iTextEffect);
    }

    text_edit::paragraph_style::paragraph_style()
    {
    }

    text_edit::paragraph_style::paragraph_style(paragraph_style const& aOther) :
        iPadding{ aOther.iPadding },
        iLineSpacing{ aOther.iLineSpacing }
    {
    }

    text_edit::paragraph_style::paragraph_style(optional_padding const& aPadding, optional<double> const& aLineSpacing) :
        iPadding{ aPadding },
        iLineSpacing{ aLineSpacing }
    {
    }

    optional_padding const& text_edit::paragraph_style::padding() const
    {
        return iPadding;
    }

    text_edit::paragraph_style& text_edit::paragraph_style::set_padding(optional_padding const& aPadding)
    {
        iPadding = aPadding;
        return *this;
    }

    optional<double> const& text_edit::paragraph_style::line_spacing() const
    {
        return iLineSpacing;
    }

    text_edit::paragraph_style& text_edit::paragraph_style::set_line_spacing(optional<double> const& aLineSpacing)
    {
        iLineSpacing = aLineSpacing;
        return *this;
    }

    text_edit::paragraph_style& text_edit::paragraph_style::merge(const paragraph_style& aRhs)
    {
        if (aRhs.padding() != std::nullopt)
            iPadding = aRhs.padding();
        if (aRhs.line_spacing() != std::nullopt)
            iLineSpacing = aRhs.line_spacing();
        return *this;
    }

    bool text_edit::paragraph_style::operator==(const paragraph_style& aRhs) const
    {
        return std::tie(iPadding, iLineSpacing) == std::tie(aRhs.iPadding, aRhs.iLineSpacing);
    }

    bool text_edit::paragraph_style::operator!=(const paragraph_style& aRhs) const
    {
        return !(*this == aRhs);
    }

    bool text_edit::paragraph_style::operator<(const paragraph_style& aRhs) const
    {
        return std::tie(iPadding, iLineSpacing) < std::tie(aRhs.iPadding, aRhs.iLineSpacing);
    }

    text_edit::style::style() : 
        iParent{ nullptr },
        iUseCount{ 0 }
    {
    }

    text_edit::style::style(character_style const& aCharacter) :
        iParent{ nullptr },
        iUseCount{ 0 },
        iCharacter{ aCharacter }
    {
    }
        
    text_edit::style::style(character_style const& aCharacter, paragraph_style const& aParagraph) :
        iParent{ nullptr },
        iUseCount{ 0 },
        iCharacter{ aCharacter },
        iParagraph{ aParagraph }
    {
    }

    text_edit::style::style(paragraph_style const& aParagraph) :
        iParent{ nullptr },
        iUseCount{ 0 },
        iParagraph{ aParagraph }
    {
    }

    text_edit::style::style(text_edit& aParent, const style& aOther) :
        iParent{ &aParent },
        iUseCount{ 0 },
        iCharacter{ aOther.iCharacter },
        iParagraph{ aOther.iParagraph }
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

    text_edit::style& text_edit::style::merge(const style& aOverridingStyle)
    {
        iCharacter.merge(aOverridingStyle.iCharacter);
        iParagraph.merge(aOverridingStyle.iParagraph);
        return *this;
    }

    bool text_edit::style::operator==(const style& aRhs) const
    {
        return std::tie(iCharacter) == std::tie(aRhs.iCharacter);
    }

    bool text_edit::style::operator!=(const style& aRhs) const
    {
        return !(*this == aRhs);
    }

    bool text_edit::style::operator<(const style& aRhs) const
    {
        return std::tie(iCharacter) < std::tie(aRhs.iCharacter);
    }

    text_edit::character_style const& text_edit::style::character() const
    {
        return iCharacter;
    }

    text_edit::character_style& text_edit::style::character()
    {
        return iCharacter;
    }

    text_edit::paragraph_style const& text_edit::style::paragraph() const
    {
        return iParagraph;
    }

    text_edit::paragraph_style& text_edit::style::paragraph()
    {
        return iParagraph;
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

    text_edit::text_edit(text_edit_caps aCaps, frame_style aFrameStyle) :
        framed_scrollable_widget{ (aCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine ? scrollbar_style::Normal : scrollbar_style::Invisible, aFrameStyle },
        iCaps{ aCaps },
        iPersistDefaultStyle{ false },
        iCursor{ *this },
        iUpdatingDocument{ false },
        iGlyphColumns{ 1 },
        iCursorAnimationStartTime{ neolib::thread::program_elapsed_ms() },
        iTabStopHint{ "0000" },
        iAnimator{ *this, [this](widget_timer&)
        {
            iAnimator.again();
            animate();
        }, std::chrono::milliseconds{ 16 } },
        iSuppressTextChangedNotification{ 0u },
        iWantedToNotfiyTextChanged{ 0u },
        iOutOfMemory{ false }
    {
        init();
    }

    text_edit::text_edit(i_widget& aParent, text_edit_caps aCaps, frame_style aFrameStyle) :
        framed_scrollable_widget{ aParent, (aCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine ? scrollbar_style::Normal : scrollbar_style::Invisible, aFrameStyle },
        iCaps{ aCaps },
        iPersistDefaultStyle{ false },
        iCursor{ *this },
        iUpdatingDocument{ false },
        iGlyphColumns{ 1 },
        iCursorAnimationStartTime{ neolib::thread::program_elapsed_ms() },
        iTabStopHint{ "0000" },
        iAnimator{ *this, [this](widget_timer&)
        {
            iAnimator.again();
            animate();
        }, std::chrono::milliseconds{ 16 } },
        iSuppressTextChangedNotification{ 0u },
        iWantedToNotfiyTextChanged{ 0u },
        iOutOfMemory{ false }
    {
        init();
    }

    text_edit::text_edit(i_layout& aLayout, text_edit_caps aCaps, frame_style aFrameStyle) :
        framed_scrollable_widget{ aLayout, (aCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine ? scrollbar_style::Normal : scrollbar_style::Invisible, aFrameStyle },
        iCaps{ aCaps },
        iPersistDefaultStyle{ false },
        iCursor{ *this },
        iUpdatingDocument{ false },
        iGlyphColumns{ 1 },
        iCursorAnimationStartTime{ neolib::thread::program_elapsed_ms() },
        iTabStopHint{ "0000" },
        iAnimator{ *this, [this](widget_timer&)
        {
            iAnimator.again();
            animate();
        }, std::chrono::milliseconds{ 16 } },
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

    size text_edit::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (has_minimum_size())
            return framed_scrollable_widget::minimum_size(aAvailableSpace);
        scoped_units su{ *this, units::Pixels };
        auto const childLayoutSize = has_layout() ? layout().minimum_size(aAvailableSpace) : size{};
        auto const frameBits = framed_scrollable_widget::minimum_size(aAvailableSpace) - childLayoutSize;
        auto result = frameBits;
        if (!size_hint())
            result += size{ font().height() }.max(childLayoutSize);
        else
        {
            if (iHintedSize == std::nullopt || iHintedSize->first != font())
            {
                iHintedSize.emplace(font(), size{});
                graphics_context gc{ *this, graphics_context::type::Unattached };
                iHintedSize->second = gc.text_extent(size_hint().primary_hint(), font()).max(gc.text_extent(size_hint().secondary_hint(), font()));
                if (iHintedSize->second.cy == 0.0)
                    iHintedSize->second.cy = font().height();
            }
            result += iHintedSize->second.max(childLayoutSize);
        }
        if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::GrowLines)
            result.cy = std::max(result.cy, frameBits.cy + std::min(iTextExtents.get().cy, grow_lines() * font().height()));
        return to_units(*this, su.saved_units(), result);
    }

    size text_edit::maximum_size(optional_size const& aAvailableSpace) const
    {
        if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::MultiLine || has_maximum_size())
            return framed_scrollable_widget::maximum_size(aAvailableSpace);
        return size{ framed_scrollable_widget::maximum_size(aAvailableSpace).cx, minimum_size(aAvailableSpace).cy };
    }

    neogfx::padding text_edit::padding() const
    {
        auto result = framed_scrollable_widget::padding();
        return result + padding_adjust();
    }

    void text_edit::paint(i_graphics_context& aGc) const
    {
        framed_scrollable_widget::paint(aGc);
        rect clipRect = default_clip_rect().intersection(client_rect(false));
        clipRect.inflate(size{ padding_adjust() });
        if (iOutOfMemory)
        {
            draw_alpha_background(aGc, clipRect);
            return;
        }
        scoped_scissor scissor{ aGc, clipRect };
        if (iDefaultStyle.character().paper_color() != neolib::none)
            aGc.fill_rect(client_rect(), to_brush(iDefaultStyle.character().paper_color()));
        coordinate x = 0.0;
        for (auto columnIndex = 0u; columnIndex < columns(); ++columnIndex)
        {
            auto const& column = static_cast<const glyph_column&>(text_edit::column(columnIndex));
            auto columnClipRect = clipRect.intersection(column_rect(columnIndex, true));
            columnClipRect.inflate(size{ std::max(calc_padding_adjust(column_style(columnIndex)), padding_adjust()) });
            scoped_scissor scissor2{ aGc, columnClipRect };
            auto const& columnRectSansPadding = column_rect(columnIndex);
            auto const& lines = column.lines();
            auto line = std::lower_bound(lines.begin(), lines.end(), glyph_line{ {}, {}, {}, vertical_scrollbar().position(), {} },
                [](const glyph_line& left, const glyph_line& right) { return left.ypos < right.ypos; });
            if (line != lines.begin() && (line == lines.end() || line->ypos > vertical_scrollbar().position()))
                --line;
            if (line == lines.end())
                continue;
            for (auto paintLine = line; paintLine != lines.end(); paintLine++)
            {
                point linePos = columnRectSansPadding.top_left() + point{ -horizontal_scrollbar().position(), paintLine->ypos - vertical_scrollbar().position() };
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
        if (has_focus() && !read_only())
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
        return default_style().character().font() != std::nullopt ? *default_style().character().font() : framed_scrollable_widget::font();
    }

    void text_edit::set_font(optional_font const& aFont)
    {
        framed_scrollable_widget::set_font(aFont);
        if (!default_style().character().font())
            refresh_paragraph(iText.begin(), 0);
    }

    void text_edit::focus_gained(focus_reason aFocusReason)
    {
        framed_scrollable_widget::focus_gained(aFocusReason);
        neolib::service<neolib::i_power>().register_activity();
        service<i_clipboard>().activate(*this);
        iCursorAnimationStartTime = neolib::thread::program_elapsed_ms();
        if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::SingleLine && aFocusReason == focus_reason::Tab)
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
        if (service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
            service<i_clipboard>().deactivate(*this);
        if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::SingleLine)
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
            iMenu = std::make_unique<neogfx::context_menu>(*this, aPosition + non_client_rect().top_left() + root().window_position());
            ContextMenu.trigger(iMenu->menu());
            iMenu->menu().add_action(service<i_app>().action_undo());
            iMenu->menu().add_action(service<i_app>().action_redo());
            iMenu->menu().add_separator();
            iMenu->menu().add_action(service<i_app>().action_cut());
            iMenu->menu().add_action(service<i_app>().action_copy());
            iMenu->menu().add_action(service<i_app>().action_paste());
            auto& pasteAs = iMenu->menu().add_sub_menu("Paste As"_t);
            auto pastePlainText = make_ref<action>("Plain Text"_t);
            auto pasteRichText = make_ref<action>("Rich Text (HTML)"_t);
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
        if (client_rect(false).contains(mouse_position()) || iDragger != std::nullopt)
            return mouse_system_cursor::IBeam;
        else
            return framed_scrollable_widget::mouse_cursor();
    }

    bool text_edit::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        neolib::service<neolib::i_power>().register_activity();

        bool handled = true;
        switch (aScanCode)
        {
        case ScanCode_TAB:
            if (!read_only())
            {
                multiple_text_changes mtc{ *this };
                delete_any_selection();
                insert_text(string{ "\t" }, next_style());
                cursor().set_position(cursor().position() + 1);
            }
            break;
        case ScanCode_RETURN:
            if (!read_only())
            {
                if ((aKeyModifiers & KeyModifier_CTRL) == KeyModifier_NONE)
                {
                    bool canAccept = false;
                    CanAcceptText.trigger(text(), canAccept);
                    if (canAccept)
                    {
                        AcceptText.trigger(text());
                        break;
                    }
                    else if ((iCaps & text_edit_caps::OnlyAccept) == text_edit_caps::OnlyAccept)
                        break;
                }
                if ((iCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine)
                {
                    multiple_text_changes mtc{ *this };
                    delete_any_selection();
                    if ((aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE)
                        insert_text(string{ "\n" }, next_style());
                    else
                        insert_text(string{ "\r" }, next_style());
                    cursor().set_position(cursor().position() + 1);
                }
                else
                    handled = framed_scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
            }
            else
                handled = framed_scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
            break;
        case ScanCode_BACKSPACE:
            if (!read_only() && cursor().position() == cursor().anchor())
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
            if (!read_only() && cursor().position() == cursor().anchor())
            {
                if (cursor().position() < glyphs().size())
                {
                    delete_text(cursor().position(), cursor().position() + 1);
                    make_cursor_visible(true);
                }
            }
            else
                delete_any_selection();
            break;
        case ScanCode_UP:
            if ((iCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine)
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
            if ((iCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine)
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
            if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::MultiLine)
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
            else if (!read_only() && (iCaps & (text_edit_caps::SingleLine | text_edit_caps::OnlyAccept)) != text_edit_caps::None)
                set_text(string{});
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

    bool text_edit::text_input(i_string const& aText)
    {
        if (aText[0] == '\r' || aText[0] == '\n') // handled in key_pressed()
        {
            if ((focus_policy() & neogfx::focus_policy::ConsumeReturnKey) == neogfx::focus_policy::ConsumeReturnKey)
                return true;
            else
                return framed_scrollable_widget::text_input(aText);
        }
        else if (aText[0] == '\t') // handled in key_pressed()
        {
            if ((focus_policy() & neogfx::focus_policy::ConsumeTabKey) == neogfx::focus_policy::ConsumeTabKey)
                return true;
            else
                return framed_scrollable_widget::text_input(aText);
        }
        if (!read_only())
        {
            multiple_text_changes mtc{ *this };
            delete_any_selection();
            insert_text(aText, next_style(), true);
        }
        return true;
    }

    scrolling_disposition text_edit::scrolling_disposition() const
    {
        return neogfx::scrolling_disposition::DontScrollChildWidget;
    }

    bool text_edit::use_scrollbar_container_updater() const
    {
        return false;
    }

    void text_edit::update_scrollbar_visibility(usv_stage_e aStage)
    {
        std::optional<scoped_property_transition_suppression> sts1;
        std::optional<scoped_property_transition_suppression> sts2;

        bool refreshLines = (iTextExtents == std::nullopt);

        if (!refreshLines) // must be a resize event
        {
            sts1.emplace(vertical_scrollbar().Position);
            sts2.emplace(horizontal_scrollbar().Position);
        }

        switch (aStage)
        {
        case UsvStageInit:
            if (resizing())
                vertical_scrollbar().push_zone();
            if (vertical_scrollbar().visible())
            {
                vertical_scrollbar().hide();
//                refreshLines = true;  // todo: optimize this properly
            }
            if (horizontal_scrollbar().visible())
            {
                horizontal_scrollbar().hide();
//                refreshLines = true;  // todo: optimize this properly
            }
            refreshLines = true;  // todo: optimize this properly
            break;
        case UsvStageCheckVertical1:
        case UsvStageCheckVertical2:
            {
                i_scrollbar::value_type oldPosition = vertical_scrollbar().position();
                vertical_scrollbar().set_maximum(iTextExtents->cy);
                vertical_scrollbar().set_step(font().height());
                vertical_scrollbar().set_page(client_rect(false).height());
                vertical_scrollbar().set_position(oldPosition);
                if (vertical_scrollbar().maximum() - vertical_scrollbar().page() > 0.0)
                {
                    if (!vertical_scrollbar().visible())
                    {
                        vertical_scrollbar().show();
                        // refreshLines = true;  // todo: optimize this properly
                    }
                    refreshLines = true;  // todo: optimize this properly
                }
                else
                {
                    if (vertical_scrollbar().visible())
                    {
                        vertical_scrollbar().hide();
                        // refreshLines = true;  // todo: optimize this properly
                    }
                    refreshLines = true;  // todo: optimize this properly
                }
                framed_scrollable_widget::update_scrollbar_visibility(aStage);
            }
            break;
        case UsvStageCheckHorizontal:
            {
                i_scrollbar::value_type oldPosition = horizontal_scrollbar().position();
                horizontal_scrollbar().set_maximum(iTextExtents->cx <= client_rect(false).width() ? 0.0 : iTextExtents->cx);
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
            if (has_focus() && !read_only())
                make_cursor_visible();
            else if (resizing())
            {
                auto zone = vertical_scrollbar().pop_zone();
                if (zone == scrollbar_zone::Middle)
                    make_visible(glyph_position(document_hit_test(column_rect(0).top_left()), true));
                else if (zone == scrollbar_zone::Bottom)
                {
                    vertical_scrollbar().set_position(vertical_scrollbar().maximum());
                    horizontal_scrollbar().set_position(0.0);
                }
            }
            break;
        default:
            break;
        }

        if (refreshLines)
            refresh_lines();
    }

    color text_edit::frame_color() const
    {
        if (has_frame_color())
            return framed_scrollable_widget::frame_color();
        else if (service<i_app>().current_style().palette().color(color_role::Theme).similar_intensity(background_color(), 0.03125))
            return framed_scrollable_widget::frame_color();
        else
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
            for (auto sti = selectedText.begin(); sti != selectedText.end();)
            {
                auto const ch = *sti++;
                if (ch == U'\r' && (sti == selectedText.end() || *sti != U'\n'))
                    *std::prev(sti) = U'\n';
            }
            aClipboard.set_text(string{ neolib::utf32_to_utf8(selectedText) });
        }
    }

    void text_edit::paste(i_clipboard& aClipboard)
    {
        {
            multiple_text_changes mtc{ *this };
            if (cursor().position() != cursor().anchor())
                delete_selected();
            auto len = insert_text(aClipboard.text(), next_style());
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

    std::size_t text_edit::document_length() const
    {
        return iText.size();
    }

    void text_edit::move_cursor(cursor::move_operation_e aMoveOperation, bool aMoveAnchor)
    {
        if (glyphs().empty())
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
                    while (p > 0 && category(glyphs()[p - 1]) == c)
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
                    auto const cursorPos = point{ *iCursorHint.x, std::prev(currentPosition.line)->ypos } + columnRectSansPadding.top_left();
                    auto const glyph = document_hit_test(cursorPos, false);
                    cursor().set_position(from_glyph(glyphs().begin() + glyph).first, aMoveAnchor);
                }
            }
            break;
        case cursor::Down:
            {
                auto currentPosition = glyph_position(cursor_glyph_position());
                if (currentPosition.line != currentPosition.column->lines().end())
                {
                    if (std::next(currentPosition.line) != currentPosition.column->lines().end())
                    {
                        auto const columnRectSansPadding = column_rect(column_index(*currentPosition.column));
                        auto const cursorPos = point{ *iCursorHint.x, std::next(currentPosition.line)->ypos } + columnRectSansPadding.top_left();
                        auto const glyph = document_hit_test(cursorPos, false);
                        cursor().set_position(from_glyph(glyphs().begin() + glyph).first, aMoveAnchor);
                    }
                    else if (currentPosition.lineEnd == glyphs().end() || is_line_breaking_whitespace(*currentPosition.lineEnd))
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

    i_string const& text_edit::plain_text() const
    {
        return text();
    }

    bool text_edit::set_plain_text(i_string const& aPlainText)
    {
        return set_text(aPlainText) != 0 || aPlainText.empty();
    }

    i_string const& text_edit::rich_text(rich_text_format aFormat) const
    {
        // todo
        return text();
    }

    bool text_edit::set_rich_text(i_string const& aRichText, rich_text_format aFormat)
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

    void text_edit::begin_update()
    {
        iUpdatingDocument = true;
    }

    void text_edit::end_update()
    {
        iUpdatingDocument = false;
        refresh_paragraph(iText.begin(), 0);
        update();
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

    uint32_t text_edit::grow_lines() const
    {
        return GrowLines;
    }

    void text_edit::set_grow_lines(uint32_t aGrowLines)
    {
        if (GrowLines != aGrowLines)
        {
            GrowLines = aGrowLines;
            update_layout();
        }
    }

    bool text_edit::password() const
    {
        return Password;
    }

    i_string const& text_edit::password_mask() const
    {
        return PasswordMask;
    }

    void text_edit::set_password(bool aPassword, i_string const& aMask)
    {
        if (Password != aPassword || PasswordMask != aMask)
        {
            Password = aPassword;
            PasswordMask = string{ aMask };
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
        auto oldEffect = (iDefaultStyle.character().text_effect() == std::nullopt);
        iDefaultStyle = aDefaultStyle;
        if (oldFont != font() || oldEffect != (iDefaultStyle.character().text_effect() == std::nullopt))
            refresh_paragraph(iText.begin(), 0);
        iPersistDefaultStyle = aPersist;
        DefaultStyleChanged.trigger();
        update();
    }

    color text_edit::default_text_color() const
    {
        if (std::holds_alternative<color>(default_style().character().text_color()))
            return static_variant_cast<const color&>(default_style().character().text_color());
        else if (std::holds_alternative<gradient>(default_style().character().text_color()))
            return static_variant_cast<const gradient&>(default_style().character().text_color()).at(0.0);
        else
            return service<i_app>().current_style().palette().default_text_color_for_widget(*this);
    }

    text_edit::style text_edit::current_style() const
    {
        if (iText.empty())
            return style{ default_style() }.character().set_font_if_none(font());
        auto t = std::next(iText.begin(), cursor().anchor());
        if (t != iText.begin() && cursor().position() == cursor().anchor())
            t = std::prev(t);
        auto const g = to_glyph(t);
        return glyph_style(g, *glyph_column_line(g - glyphs().begin()).first).character().set_font_if_none(g != glyphs().end() ? glyphs().glyph_font(*g) : font());
    }

    void text_edit::apply_style(style const& aStyle)
    {
        if (cursor().position() == cursor().anchor())
        {
            set_default_style(aStyle);
            iNextStyle = aStyle;
        }
        else
            apply_style(std::min(cursor().anchor(), cursor().position()), std::max(cursor().anchor(), cursor().position()), aStyle);
    }

    void text_edit::apply_style(position_type aStart, position_type aEnd, style const& aStyle)
    {
        // todo: optimize this (by changing style in-place)
        std::u32string part;
        part.assign(iText.begin() + aStart, iText.begin() + aEnd);
        delete_text(aStart, aEnd);
        insert_text(aStart, string{ neolib::utf32_to_utf8(part) }, aStyle);
    }

    text_edit::style text_edit::next_style() const
    {
        if (iNextStyle != std::nullopt)
        {
            auto nextStyle = *iNextStyle;
            iNextStyle = std::nullopt;
            return nextStyle;
        }
        return current_style();
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
            iDragger.emplace(*this);
        }
    }

    std::size_t text_edit::column_index(const column_info& aColumn) const
    {
        return static_cast<const glyph_column*>(&aColumn) - static_cast<const glyph_column*>(&column(0));
    }

    rect text_edit::column_rect(std::size_t aColumnIndex, bool aExtendIntoPadding) const
    {
        auto result = client_rect(false);
        for (std::size_t ci = 0; ci < aColumnIndex; ++ci)
            result.x += static_cast<const glyph_column&>(column(ci)).width();
        if (!aExtendIntoPadding)
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

    std::pair<text_edit::glyph_columns::const_iterator, text_edit::glyph_lines::const_iterator> text_edit::glyph_column_line(position_type aGlyphPosition) const
    {
        auto column = iGlyphColumns.begin();
        glyph_lines::const_iterator line;
        for (; column != iGlyphColumns.end(); ++column)
        {
            line = std::lower_bound(column->lines().begin(), column->lines().end(), glyph_line{ {}, { aGlyphPosition, glyphs().begin() + aGlyphPosition }, {}, 0.0, {} },
                [](const glyph_line& left, const glyph_line& right) { return left.lineStart.first < right.lineStart.first; });
            if (line != column->lines().end())
                break;
        }
        if (column == iGlyphColumns.end())
            --column;
        return std::make_pair(column, line);
    }

    text_edit::position_info text_edit::glyph_position(position_type aGlyphPosition, bool aForCursor) const
    {
        auto const columnLine = glyph_column_line(aGlyphPosition);
        auto column = columnLine.first;
        auto line = columnLine.second;
        auto const& columnRectSansPadding = column_rect(column - iGlyphColumns.begin());
        auto& lines = column->lines();
        if (line != lines.begin())
        {
            if (line == lines.end())
            {
                auto const& lastLine = lines.back();
                if (aGlyphPosition <= lastLine.lineEnd.first || !is_line_breaking_whitespace(glyphs().back()))
                {
                    if (!is_line_breaking_whitespace(glyphs().back()) || lastLine.lineEnd.first - lastLine.lineStart.first != 1)
                        --line;
                }
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
                if (aGlyphPosition == glyphs().size() || aGlyphPosition == paragraph->first.end_index())
                {
                    auto iterChar = iText.begin() + from_glyph(glyphs().begin() + aGlyphPosition).first;
                    if (iterChar != iText.begin())
                    {
                        auto iterGlyph = to_glyph(iterChar - 1);
                        auto const& g = *iterGlyph;
                        if (direction(g) == text_direction::RTL || direction(g) == text_direction::Digit_RTL)
                        {
                            aGlyphPosition = iterGlyph - glyphs().begin();
                            placeCursorToRight = (direction(g) == text_direction::RTL ? false : true);
                        }
                    }
                }
                else if (direction(glyphs()[aGlyphPosition]) == text_direction::RTL)
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
                    auto iterGlyph = glyphs().begin() + aGlyphPosition;
                    auto const& glyph = aGlyphPosition < lineEnd ? *iterGlyph : *(iterGlyph - 1);
                    point linePos{ glyph.x - line->lineStart.second->x, line->ypos };
                    if (placeCursorToRight)
                        linePos.x += advance(glyph).cx;
                    return position_info{ iterGlyph, column, line, glyphs().begin() + lineStart, glyphs().begin() + lineEnd, linePos + alignmentAdjust };
                }
                else
                    return position_info{ line->lineStart.second, column, line, glyphs().begin() + lineStart, glyphs().begin() + lineEnd, point{ 0.0, line->ypos } + alignmentAdjust };
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
        return position_info{ glyphs().end(), column, lines.end(), glyphs().end(), glyphs().end(), pos };
    }

    text_edit::position_type text_edit::cursor_glyph_position() const
    {
        return to_glyph(iText.begin() + cursor().position()) - glyphs().begin();
    }

    text_edit::position_type text_edit::cursor_glyph_anchor() const
    {
        return to_glyph(iText.begin() + cursor().anchor()) - glyphs().begin();
    }

    void text_edit::set_cursor_glyph_position(position_type aGlyphPosition, bool aMoveAnchor)
    {
        cursor().set_position(from_glyph(glyphs().begin() + aGlyphPosition).first, aMoveAnchor);
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
            auto lineStart = (line != lines.end() ? line->lineStart.first : glyphs().size());
            auto lineEnd = (line != lines.end() ? line->lineEnd.first : glyphs().size());
            auto lineStartX = line->lineStart.second->x;
            for (auto gi = line->lineStart.first; gi != lineEnd; ++gi)
            {
                auto& glyph = glyphs()[gi];
                auto const glyphAdvance = advance(glyph).cx;
                if (adjustedPosition.x >= glyph.x - lineStartX && (adjustedPosition.x < glyph.x - lineStartX + glyphAdvance || glyphAdvance == 0.0))
                {
                    if (direction(glyph) != text_direction::RTL)
                        return adjustedPosition.x < glyph.x - lineStartX + glyphAdvance / 2.0 || glyphAdvance == 0.0 ? gi : gi + 1;
                    else
                        return gi + 1;
                }
            }
            if (lineEnd > lineStart && line != std::prev(lines.end()) && !is_whitespace(glyphs()[lineEnd]))
                return lineEnd - 1;
            if (lineEnd > lineStart && direction(glyphs()[lineEnd - 1]) == text_direction::RTL)
                return lineEnd - 1;
            return lineEnd;
        }
        else
        {
            return glyphs().size();
        }
    }

    bool text_edit::same_word(position_type aTextPositionLeft, position_type aTextPositionRight) const
    {
        if (iText[aTextPositionLeft] == U'\n' || iText[aTextPositionRight] == U'\n' ||
            iText[aTextPositionLeft] == U'\r' || iText[aTextPositionRight] == U'\r')
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
        iPreviousText = iText;
        iText.clear();
        glyphs().clear();
        iGlyphParagraphs.clear();
        for (std::size_t i = 0; i < iGlyphColumns.size(); ++i)
            iGlyphColumns[i].lines().clear();
        iUtf8TextCache = std::nullopt;
        refresh_columns();
        if (iPreviousText != iText)
            notify_text_changed();
    }

    std::size_t text_edit::paragraph_count() const
    {
        return std::count(text().begin(), text().end(), '\n') + 1;
    }

    void text_edit::delete_paragraph(std::size_t aParagraphIndex)
    {
        if (paragraph_count() > 1)
        {
            auto const start = aParagraphIndex > 0 ?
                boost::find_nth(text(), "\n", static_cast<int>(aParagraphIndex - 1)).end() : text().begin();
            auto const end = boost::find_nth(text(), "\n", static_cast<int>(aParagraphIndex)).end();
            delete_text(std::distance(text().begin(), start), std::distance(text().begin(), end));
        }
        else
            clear();
    }

    i_string const& text_edit::text() const
    {
        if (iUtf8TextCache == std::nullopt)
        {
            std::u32string u32text{ iText.begin(), iText.end() };
            iUtf8TextCache = neolib::utf32_to_utf8(u32text);
        }
        return *iUtf8TextCache;
    }

    std::size_t text_edit::set_text(i_string const& aText)
    {
        return set_text(aText, default_style());
    }

    std::size_t text_edit::set_text(i_string const& aText, const style& aStyle)
    {
        return do_insert_text(0, aText, aStyle, true, true);
    }

    std::size_t text_edit::append_text(i_string const& aText, bool aMoveCursor)
    {
        return do_insert_text(iText.size(), aText, default_style(), aMoveCursor, false);
    }

    std::size_t text_edit::append_text(i_string const& aText, const style& aStyle, bool aMoveCursor)
    {
        return do_insert_text(iText.size(), aText, aStyle, aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(i_string const& aText, bool aMoveCursor)
    {
        return do_insert_text(cursor().position(), aText, default_style(), aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(i_string const& aText, const style& aStyle, bool aMoveCursor)
    {
        return do_insert_text(cursor().position(), aText, aStyle, aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(position_type aPosition, i_string const& aText, bool aMoveCursor)
    {
        return do_insert_text(aPosition, aText, default_style(), aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(position_type aPosition, i_string const& aText, const style& aStyle, bool aMoveCursor)
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
        while (result.first > 0 && same_paragraph(aGlyphPosition, result.first - 1) && glyphs()[result.first-1].source == glyphs()[aGlyphPosition].source)
            --result.first;
        while (result.second < glyphs().size() && same_paragraph(aGlyphPosition, result.second) && glyphs()[result.second].source == glyphs()[aGlyphPosition].source)
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

    void text_edit::set_tab_stop_hint(i_string const& aTabStopHint)
    {
        if (iTabStopHint != aTabStopHint)
        {
            iTabStopHint = aTabStopHint;
            iCalculatedTabStops.reset();
        }
    }

    void text_edit::set_tab_stops(const optional_dimension& aTabStops)
    {
        optional_dimension newTabStops = (aTabStops != std::nullopt ? optional_dimension{ units_converter(*this).to_device_units(size{ *aTabStops, 0.0 }).cx } : optional_dimension{});
        if (iTabStops != newTabStops)
        {
            iTabStops = newTabStops;
            refresh_columns();
        }
        iCalculatedTabStops.reset();
    }

    void text_edit::init()
    {
        if ((iCaps & text_edit_caps::Password) == text_edit_caps::Password)
            set_password(true);

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
        if ((iCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine)
            focusPolicy |= neogfx::focus_policy::ConsumeReturnKey;
        set_focus_policy(focusPolicy);
        cursor().set_width(2.0);
        iSink += cursor().PositionChanged([this]()
        {
            iNextStyle = std::nullopt;
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

    text_edit::document_glyphs const& text_edit::glyphs() const
    {
        if (iGlyphs == std::nullopt)
            iGlyphs = *make_ref<basic_glyph_text_content<text_edit::glyph_container_type>>(font());
        return *iGlyphs;
    }

    text_edit::document_glyphs& text_edit::glyphs()
    {
        return const_cast<document_glyphs&>(to_const(*this).glyphs());
    }

    std::size_t text_edit::do_insert_text(position_type aPosition, i_string const& aText, const style& aStyle, bool aMoveCursor, bool aClearFirst)
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
        for (auto ti = text.begin(); ti != text.end();)
        {
            auto ch = *ti++;
            if (ch != U'\r' || (ti == text.end() || (*ti) != U'\n'))
                iNormalizedTextBuffer.push_back(ch);
        }
        auto eos = iNormalizedTextBuffer.size();
        if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::SingleLine)
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
        {
            cursor().set_position(insertionPoint - iText.begin() + eos);
            iCursorHint.x = glyph_position(cursor_glyph_position(), true).pos.x;
        }
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
            return glyphs().end();
        auto paragraphEnd = paragraph->first.end();
        for (auto i = paragraph->first.start(); i != paragraphEnd; ++i)
        {
            auto const& g = *i;
            auto si = paragraph->first.text_start_index();
            auto start = g.source.first + si;
            auto end = g.source.second + si;
            if (textIndex >= start && textIndex < end)
                return i;
        }
        return paragraphEnd;
    }

    std::pair<text_edit::document_text::size_type, text_edit::document_text::size_type> text_edit::from_glyph(document_glyphs::const_iterator aWhere) const
    {
        if (aWhere == glyphs().end())
            return std::make_pair(iText.size(), iText.size());
        auto paragraph = glyph_to_paragraph(aWhere - glyphs().begin());
        if (paragraph == iGlyphParagraphs.end() && paragraph != iGlyphParagraphs.begin() && aWhere <= (paragraph - 1)->first.end())
            --paragraph;
        if (paragraph != iGlyphParagraphs.end())
        {
            if (paragraph->first.start() > aWhere)
                --paragraph;
            auto textStart = paragraph->first.text_start_index();
            auto rg = related_glyphs(aWhere - glyphs().begin());
            return std::make_pair(textStart + glyphs()[rg.first].source.first, textStart + glyphs()[rg.second - 1].source.second);
        }
        return std::make_pair(iText.size(), iText.size());
    }

    void text_edit::refresh_paragraph(document_text::const_iterator aWhere, ptrdiff_t aDelta)
    {
        if (iUpdatingDocument)
            return;

        /* simple (naive) implementation just to get things moving (so just refresh everything) ... */
        (void)aWhere;
        graphics_context gc{ *this, graphics_context::type::Unattached };
        if (password())
            gc.set_password(true, PasswordMask.value().empty() ? "\xE2\x97\x8F"_s : PasswordMask);
        glyphs().clear();
        iGlyphParagraphs.clear();
        iCharacterToParagraphCache.clear();
        iCharacterToParagraphCacheLastAccess.reset();
        iGlyphToParagraphCache.clear();
        iGlyphToParagraphCacheLastAccess.reset();
        std::u32string paragraphBuffer;
        auto nextParagraph = iText.begin();
        auto iterColumn = iGlyphColumns.begin();
        neolib::vecarray<std::u32string::size_type, 16, -1> columnDelimiters;
        auto fs = [this, &nextParagraph, &columnDelimiters](std::u32string::size_type aSourceIndex)
        {
            auto const& tagStyle = iText.tag(nextParagraph + aSourceIndex).style();
            std::size_t indexColumn = std::lower_bound(columnDelimiters.begin(), columnDelimiters.end(), aSourceIndex) - columnDelimiters.begin();
            if (indexColumn > columns() - 1)
                indexColumn = columns() - 1;
            auto const& columnStyle = column_style(indexColumn);
            auto const& style =
                std::holds_alternative<style_list::const_iterator>(tagStyle) ? *static_variant_cast<style_list::const_iterator>(tagStyle) :
                columnStyle.character().font() != std::nullopt ? columnStyle : iDefaultStyle;
            return style.character().font() != std::nullopt ? *style.character().font() : font();
        };
        for (auto iterChar = iText.begin(); iterChar != iText.end(); ++iterChar)
        {
            auto& column = *(iterColumn);
            auto ch = *iterChar;
            if (ch == column.delimiter() && iterColumn + 1 != iGlyphColumns.end())
            {
                ++iterColumn;
                columnDelimiters.push_back(iterChar - nextParagraph);
                continue;
            }
            bool newParagraph = (ch == U'\n');
            if (newParagraph || iterChar == iText.end() - 1)
            {
                paragraphBuffer.assign(nextParagraph, iterChar + 1);
                auto gt = gc.to_glyph_text(paragraphBuffer.begin(), paragraphBuffer.end(), fs);
                if (gt.cbegin() != gt.cend())
                {
                    auto paragraphGlyphs = glyphs().container().insert(glyphs().container().end(), gt.cbegin(), gt.cend());
                    for (auto& newGlyph : gt)
                        glyphs().cache_glyph_font(newGlyph.font);
                    auto paragraph = iGlyphParagraphs.insert(iGlyphParagraphs.end(),
                        std::make_pair(
                            glyph_paragraph{ *this },
                            glyph_paragraph_index{
                                static_cast<std::size_t>((iterChar + 1) - iText.begin()) - static_cast<std::size_t>(nextParagraph - iText.begin()),
                                glyphs().size() - static_cast<std::size_t>(paragraphGlyphs - glyphs().container().begin()) }),
                                glyph_paragraphs::skip_type{ glyph_paragraph_index{}, glyph_paragraph_index{} });
                    paragraph->first.set_self(paragraph);
                    paragraph->first.set_line_breaks(gt.content().line_breaks());
                }
                nextParagraph = iterChar + 1;
                columnDelimiters.clear();
            }
        }
        for (auto p = iGlyphParagraphs.begin(); p != iGlyphParagraphs.end(); ++p)
        {
            auto& paragraph = *p;

            thread_local std::vector<std::pair<document_glyphs::iterator, document_glyphs::iterator>> paragraphLines;
            paragraphLines.clear();
            glyph_text::size_type lastBreak = 0;
            for (auto lineBreak : paragraph.first.line_breaks())
            {
                paragraphLines.emplace_back(paragraph.first.start() + lastBreak, paragraph.first.start() + lineBreak);
                lastBreak = lineBreak + 1;
            }
            paragraphLines.emplace_back(paragraph.first.start() + lastBreak, paragraph.first.end());

            for (auto const& paragraphLine : paragraphLines)
            {
                auto const paragraphLineStart = paragraphLine.first;
                auto const paragraphLineEnd = paragraphLine.second;

                if (paragraphLineStart == paragraphLineEnd)
                    continue;

                coordinate x = 0.0;
                iterColumn = iGlyphColumns.begin();
                for (auto iterGlyph = paragraphLineStart; iterGlyph != paragraphLineEnd; ++iterGlyph)
                {
                    auto& glyph = *iterGlyph;
                    if (iText[glyph.source.first] == iterColumn->delimiter() && iterColumn + 1 != iGlyphColumns.end())
                    {
                        glyph.advance = size{};
                        ++iterColumn;
                        continue;
                    }
                    else if (is_whitespace(glyph))
                    {
                        if (glyph.value == U'\t')
                        {
                            auto advance = neogfx::advance(glyph);
                            advance.cx = tab_stops() - std::fmod(x, tab_stops());
                            glyph.advance = advance;
                        }
                        else if (is_line_breaking_whitespace(glyph))
                        {
                            glyph.advance = size{};
                        }
                    }
                    glyph.x = x;
                    x += advance(glyph).cx;
                }
            }
        }
        refresh_columns();
    }

    void text_edit::refresh_columns()
    {
        iTextExtents = std::nullopt;
        update_scrollbar_visibility();
        if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::GrowLines)
            update_layout();
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

                thread_local std::vector<std::pair<document_glyphs::iterator, document_glyphs::iterator>> paragraphLines;
                paragraphLines.clear();
                glyph_text::size_type lastBreak = 0;
                for (auto lineBreak : paragraph.first.line_breaks())
                {
                    paragraphLines.emplace_back(paragraph.first.start() + lastBreak, paragraph.first.start() + lineBreak);
                    lastBreak = lineBreak + 1;
                }
                paragraphLines.emplace_back(paragraph.first.start() + lastBreak, paragraph.first.end());

                auto const& paragraphStyle = glyph_style(paragraph.first.start(), column);

                if (paragraphStyle.paragraph().padding())
                    pos.y += paragraphStyle.paragraph().padding().value().top;

                bool again = false;

                for (auto const& paragraphLine : paragraphLines)
                {
                    auto const paragraphLineStart = paragraphLine.first;
                    auto const paragraphLineEnd = paragraphLine.second;

                    if (again)
                    {
                        if (paragraphStyle.paragraph().line_spacing())
                            pos.y += paragraphStyle.paragraph().line_spacing().value();
                    }
                    else
                        again = true;

                    if (paragraphLineStart == paragraphLineEnd || is_line_breaking_whitespace(*paragraphLineStart))
                    {
                        auto lineStart = paragraphLineStart;
                        auto lineEnd = paragraphLineEnd;
                        auto height = paragraph.first.height(lineStart, lineEnd);
                        if (height == 0.0)
                        {
                            if (lines.empty())
                                height = paragraphStyle.character().font()->height();
                            else
                                height = lines.back().extents.cy;
                        }
                        lines.push_back(
                            glyph_line{
                                { p - iGlyphParagraphs.begin(), p },
                                { lineStart - glyphs().begin(), lineStart },
                                { lineEnd - glyphs().begin(), lineEnd },
                                pos.y,
                                { 0.0, height } });
                        pos.y += height;
                    }
                    else if (WordWrap && (paragraphLineEnd - 1)->x + advance(*(paragraphLineEnd - 1)).cx > availableWidth)
                    {
                        auto insertionPoint = lines.end();
                        bool first = true;
                        auto next = paragraphLineStart;
                        auto lineStart = next;
                        auto lineEnd = paragraphLineEnd;
                        coordinate offset = 0.0;
                        while (next != paragraphLineEnd)
                        {
                            auto split = std::lower_bound(next, paragraphLineEnd, paragraph_positioned_glyph{ offset + availableWidth });
                            if (split != next && (split != paragraphLineEnd || (split - 1)->x + advance(*(split - 1)).cx >= offset + availableWidth))
                                --split;
                            if (split == next)
                                ++split;
                            if (split != paragraphLineEnd)
                            {
                                std::pair<document_glyphs::iterator, document_glyphs::iterator> wordBreak = word_break(lineStart, split, paragraphLineEnd);
                                if (wordBreak.first == wordBreak.second)
                                {
                                    auto previousLineEnd = wordBreak.first;
                                    while (previousLineEnd != lineStart && (previousLineEnd - 1)->source == wordBreak.first->source)
                                        --previousLineEnd;
                                    if (previousLineEnd != lineStart)
                                    {
                                        lineEnd = wordBreak.first;
                                        next = previousLineEnd;
                                    }
                                    else
                                        next = lineEnd = split;
                                }
                                else
                                {
                                    lineEnd = wordBreak.first;
                                    next = wordBreak.second;
                                }
                            }
                            else
                                next = paragraphLineEnd;
                            dimension x = (split != glyphs().end() ? split->x : (lineStart != lineEnd ? glyphs().back().x + advance(glyphs().back()).cx : 0.0));
                            auto height = paragraph.first.height(lineStart, lineEnd);
                            if (lineEnd != lineStart && is_line_breaking_whitespace(*(lineEnd - 1)))
                                --lineEnd;
                            bool rtl = false;
                            if (!first &&
                                insertionPoint->lineStart != insertionPoint->lineEnd &&
                                lineStart != lineEnd &&
                                direction(*insertionPoint->lineStart.second) == text_direction::RTL &&
                                direction(*(lineEnd - 1)) == text_direction::RTL)
                                rtl = true; // todo: is this sufficient for multi-line RTL text?
                            if (!rtl)
                                insertionPoint = lines.end();
                            insertionPoint = lines.insert(insertionPoint,
                                glyph_line{
                                    { p - iGlyphParagraphs.begin(), p },
                                    { lineStart - glyphs().begin(), lineStart },
                                    { lineEnd - glyphs().begin(), lineEnd },
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
                            iTextExtents->cx = std::max(iTextExtents->cx, x - offset);
                            lineStart = next;
                            if (lineStart != paragraphLineEnd)
                                offset = lineStart->x;
                            lineEnd = paragraphLineEnd;
                            first = false;
                        }
                    }
                    else
                    {
                        auto lineStart = paragraphLineStart;
                        auto lineEnd = paragraphLineEnd;
                        auto height = paragraph.first.height(lineStart, lineEnd);
                        if (lineEnd != lineStart && is_line_breaking_whitespace(*(lineEnd - 1)))
                            --lineEnd;
                        lines.push_back(
                            glyph_line{
                                { p - iGlyphParagraphs.begin(), p },
                                { lineStart - glyphs().begin(), lineStart },
                                { lineEnd - glyphs().begin(), lineEnd },
                                pos.y,
                                { (lineEnd - 1)->x + advance(*(lineEnd - 1)).cx, height} });
                        pos.y += lines.back().extents.cy;
                        iTextExtents->cx = std::max(iTextExtents->cx, lines.back().extents.cx);
                    }
                }

                if (p + 1 == iGlyphParagraphs.end() && !glyphs().empty() && is_line_breaking_whitespace(glyphs().back()))
                    pos.y += font().height();

                if (paragraphStyle.paragraph().padding())
                    pos.y += paragraphStyle.paragraph().padding().value().bottom;

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
                    if (!vertical_scrollbar().visible() && !showVerticalScrollbar && pos.y > availableHeight)
                    {
                        showVerticalScrollbar = true;
                        availableWidth -= vertical_scrollbar().width();
                        next_pass();
                    }
                    else if (++p == iGlyphParagraphs.end() && pass == 1)
                        next_pass();
                    break;
                case 2:
                    if (!horizontal_scrollbar().visible() && !showHorizontalScrollbar && iTextExtents->cx > availableWidth)
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

            iTextExtents->cy = pos.y;

            if (iTextExtents->cy < client_rect(false).cy)
            {
                auto const space = client_rect(false).cy - iTextExtents->cy;
                auto const adjust =
                    ((Alignment & alignment::Vertical) == alignment::Bottom) ? space :
                    ((Alignment & alignment::Vertical) == alignment::VCenter) ? std::floor(space / 2.0) : 0.0;
                if (adjust != 0.0)
                    for (auto& column : iGlyphColumns)
                        for (auto& line : column.lines())
                            line.ypos += adjust;
            }
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
        make_visible(glyph_position(cursor_glyph_position(), true), aForcePreviewScroll ? point{ std::ceil(std::min(client_rect(false).width() / 3.0, 200.0)), 0.0 } : point{});
    }

    void text_edit::make_visible(position_info const& aGlyphPosition, point const& aPreview)
    {
        scoped_units su{ *this, units::Pixels };
        auto e = (aGlyphPosition.line != aGlyphPosition.column->lines().end() ?
            size{ aGlyphPosition.glyph != aGlyphPosition.lineEnd ? advance(*aGlyphPosition.glyph).cx : 0.0, aGlyphPosition.line->extents.cy } :
            size{ 0.0, font().height() });
        e.cy = std::min(e.cy, vertical_scrollbar().page());
        if (aGlyphPosition.pos.y < vertical_scrollbar().position())
            vertical_scrollbar().set_position(aGlyphPosition.pos.y);
        else if (aGlyphPosition.pos.y + e.cy > vertical_scrollbar().position() + vertical_scrollbar().page())
            vertical_scrollbar().set_position(aGlyphPosition.pos.y + e.cy - vertical_scrollbar().page());
        if (aGlyphPosition.pos.x < horizontal_scrollbar().position() + aPreview.x)
            horizontal_scrollbar().set_position(aGlyphPosition.pos.x - aPreview.x);
        else if (aGlyphPosition.pos.x + e.cx > horizontal_scrollbar().position() + horizontal_scrollbar().page() - aPreview.x)
            horizontal_scrollbar().set_position(aGlyphPosition.pos.x + e.cx + aPreview.x - horizontal_scrollbar().page());
    }

    text_edit::style text_edit::glyph_style(document_glyphs::const_iterator aGlyph, const glyph_column& aColumn) const
    {
        style result = iDefaultStyle;
        result.merge(column_style(aColumn));
        result.character().set_paper_color();
        auto const& tagStyle = iText.tag(iText.begin() + from_glyph(aGlyph).first).style();
        if (std::holds_alternative<style_list::const_iterator>(tagStyle))
            result.merge(*static_variant_cast<style_list::const_iterator>(tagStyle));
        return result;
    }

    void text_edit::draw_glyphs(i_graphics_context const& aGc, const point& aPosition, const glyph_column& aColumn, glyph_lines::const_iterator aLine) const
    {
        auto lineStart = aLine->lineStart.second;
        auto lineEnd = aLine->lineEnd.second;
        if (lineEnd != lineStart && is_line_breaking_whitespace(*(lineEnd - 1)))
            --lineEnd;
        {
            thread_local std::optional<glyph_text> tGlyphText;
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
                auto const& glyphFont = glyphs().glyph_font(glyph);
                auto const& glyphColor = with_bounding_box(style.character().glyph_color() == neolib::none ?
                    style.character().text_color() != neolib::none ?
                        with_bounding_box(style.character().text_color(), column_rect(column_index(aColumn))) :
                        default_text_color() : 
                    style.character().glyph_color(), client_rect(), true);
                auto const& nextTextAppearance = !selected ?
                    text_appearance{
                        glyphColor,
                        style.character().paper_color() != neolib::none ? optional_text_color{ neogfx::text_color{ style.character().paper_color() } } : optional_text_color{},
                        style.character().text_effect() }.with_emoji_ignored(style.character().ignore_emoji()) :
                    text_appearance{
                        has_focus() ? service<i_app>().current_style().palette().color(color_role::SelectedText) : glyphColor,
                        has_focus() ? service<i_app>().current_style().palette().color(color_role::Selection) : service<i_app>().current_style().palette().color(color_role::Selection).with_alpha(64) };
                if (textAppearance != std::nullopt && *textAppearance != nextTextAppearance)
                {
                    aGc.draw_glyph_text(textPos, *tGlyphText, *textAppearance);
                    tGlyphText = glyph_text{ font() };
                    textPos = glyphPos;
                }
                if (tGlyphText == std::nullopt)
                    tGlyphText = glyph_text{ font() };
                tGlyphText->content().push_back(glyph);
                tGlyphText->content().cache_glyph_font(glyphFont);
                textAppearance = nextTextAppearance;
                glyphPos.x += advance(glyph).cx;
            }
            if (tGlyphText != std::nullopt && !tGlyphText->empty())
            {
                aGc.draw_glyph_text(textPos, *tGlyphText, *textAppearance);
                tGlyphText = std::nullopt;
            }
        }
    }

    void text_edit::draw_cursor(i_graphics_context const& aGc) const
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
        scalar yOffset = 0.0;
        if (cursorPos.glyph != glyphs().end() && cursorPos.lineStart != cursorPos.lineEnd)
        {
            auto iterGlyph = (cursorPos.glyph > cursorPos.lineStart ? cursorPos.glyph - 1 : cursorPos.glyph);
            auto const& glyph = *iterGlyph;
            glyphHeight = glyph.extents.cy;
            yOffset = glyph.offset.as<scalar>().y;
        }
        else if (cursorPos.line != cursorPos.column->lines().end())
            glyphHeight = cursorPos.line->extents.cy;
        else
            glyphHeight = font().height();
        auto const columnRectSansPadding = column_rect(column_index(*cursorPos.column));
        rect cursorRect{ point{ cursorPos.pos - point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + columnRectSansPadding.top_left() + point{ 0.0, yOffset },
            size{ cursor().width(), glyphHeight } };
        if (cursorRect.right() > columnRectSansPadding.right())
            cursorRect.x += (columnRectSansPadding.right() - cursorRect.right());
        return cursorRect;
    }

    double text_edit::calc_padding_adjust(style const& aStyle) const
    {
        if (!aStyle.character().text_effect())
            return 0.0;
        else
        {
            scalar scale = 1.0;
            switch (aStyle.character().text_effect()->type())
            {
            case text_effect_type::None:
            default:
                scale = 1.0;
                break;
            case text_effect_type::Outline:
                scale = 1.0;
                break;
            case text_effect_type::Glow:
            case text_effect_type::Shadow:
                scale = 0.5;
                break;
            }
            return std::floor(aStyle.character().text_effect()->width() * scale);
        }
    }

    double text_edit::padding_adjust() const
    {
        auto paddingAdjust = calc_padding_adjust(default_style());
        for (auto const& s : iStyles)
            paddingAdjust = std::max(paddingAdjust, calc_padding_adjust(s));
        return paddingAdjust;
    }

    std::pair<text_edit::document_glyphs::iterator, text_edit::document_glyphs::iterator> text_edit::word_break(document_glyphs::iterator aBegin, document_glyphs::iterator aFrom, document_glyphs::iterator aEnd)
    {
        std::pair<document_glyphs::iterator, document_glyphs::iterator> result{ aFrom, aFrom };
        if (!is_whitespace(*aFrom))
        {
            while (result.first != aBegin && !is_whitespace(*(result.first - 1)))
                --result.first;
            result.second = result.first;
        }
        else 
        {
            while (result.first != aBegin && is_whitespace(*(result.first - 1)))
                --result.first;
            while (result.first != aBegin && !is_whitespace(*(result.first - 1)))
                --result.first;
            result.second = result.first;
        }
        if (result.second != aEnd && is_line_breaking_whitespace(*result.second))
            ++result.second;
        if (result.first == result.second && result.first == aBegin)
            result = { aFrom, aFrom };
        return result;
    }
}