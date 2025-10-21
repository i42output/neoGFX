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

#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/gui/widget/text_edit.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gfx/text/text_category_map.hpp>
#include <neogfx/gfx/text/glyph_text.ipp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/core/easing.hpp>

namespace neogfx
{
    template class basic_glyph_text_content<text_edit::glyph_container_type>;

    text_edit::password_bits::password_bits(text_edit& aParent) :
        parent{ aParent },
        previousPadding{ aParent.padding() },
        showPassword{ aParent, image{ ":/neogfx/resources/icons/eye-outline.png" } }
    {
        showPassword.set_size_policy(size_constraint::Minimum);
        showPassword.set_image_extents(size{ 16_dip, 16_dip });
        showPassword.image_widget().set_image_color(service<i_app>().current_style().palette().color(color_role::Text));
        showPassword.show(!aParent.iText.empty());

        parent.set_padding(previousPadding.with_right(previousPadding.right + showPassword.minimum_size().cx));

        showPassword.pressed([&]() { aParent.refresh_paragraph(aParent.iText.begin(), 0); });
        showPassword.released([&]() { aParent.refresh_paragraph(aParent.iText.begin(), 0); });
    }

    text_edit::password_bits::~password_bits()
    {
        parent.set_padding(previousPadding);
    }

    class text_edit::dragger : public widget_timer
    {
    public:
        dragger(text_edit& aOwner) :
            widget_timer{ aOwner, [&](widget_timer& aTimer)
            {
                aTimer.again();
                aOwner.set_cursor_position(aOwner.mouse_position(), false);
            }, std::chrono::milliseconds{ 250 } },
            iSts1{ aOwner.vertical_scrollbar().Position },
            iSts2{ aOwner.horizontal_scrollbar().Position }
            {
            }
            ~dragger()
            {
            }
    private:
        scoped_property_transition_suppression iSts1;
        scoped_property_transition_suppression iSts2;
    };

    text_edit::character_style::character_style() :
        iSmartUnderline{ false },
        iIgnoreEmoji{ true }
    {
    }

    text_edit::character_style::character_style(character_style const& aOther) :
        iFont{ aOther.iFont },
        iGlyphColor{ aOther.iGlyphColor },
        iTextColor{ aOther.iTextColor },
        iPaperColor{ aOther.iPaperColor },
        iSmartUnderline{ aOther.iSmartUnderline },
        iIgnoreEmoji{ aOther.iIgnoreEmoji },
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
        iSmartUnderline{ false },
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

    bool text_edit::character_style::smart_underline() const
    {
        return iSmartUnderline;
    }

    bool text_edit::character_style::ignore_emoji() const
    {
        return iIgnoreEmoji;
    }

    const optional_text_effect& text_edit::character_style::text_effect() const
    {
        return iTextEffect;
    }

    text_format text_edit::character_style::as_text_format() const
    {
        return text_format{ 
            glyph_color() != neolib::none ? glyph_color() : text_color(), 
            paper_color() != neolib::none ? optional_text_color{ paper_color() } : optional_text_color{}, 
            text_effect() }.with_smart_underline(smart_underline()).with_emoji_ignored(ignore_emoji());
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

    text_edit::character_style& text_edit::character_style::set_from_text_format(const text_format& aTextFormat)
    {
        iGlyphColor = aTextFormat.ink();
        iTextColor = neolib::none; // todo
        iPaperColor = aTextFormat.paper() ? *aTextFormat.paper() : neolib::none;
        iSmartUnderline = aTextFormat.smart_underline();
        iIgnoreEmoji = aTextFormat.ignore_emoji();
        iTextEffect = aTextFormat.effect();
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
        iSmartUnderline = aRhs.smart_underline();
        iIgnoreEmoji = aRhs.ignore_emoji();
        if (aRhs.text_effect() != std::nullopt)
            iTextEffect = aRhs.text_effect();
        return *this;
    }

    bool text_edit::character_style::operator==(const character_style& aRhs) const
    {
        return std::forward_as_tuple(iFont, iGlyphColor, iTextColor, iPaperColor, iSmartUnderline, iIgnoreEmoji, iTextEffect) == 
            std::forward_as_tuple(aRhs.iFont, aRhs.iGlyphColor, aRhs.iTextColor, aRhs.iPaperColor, aRhs.iSmartUnderline, aRhs.iIgnoreEmoji, aRhs.iTextEffect);
    }

    bool text_edit::character_style::operator!=(const character_style& aRhs) const
    {
        return !(*this == aRhs);
    }

    bool text_edit::character_style::operator<(const character_style& aRhs) const
    {
        return std::forward_as_tuple(iFont, iGlyphColor, iTextColor, iPaperColor, iSmartUnderline, iIgnoreEmoji, iTextEffect) < 
            std::forward_as_tuple(aRhs.iFont, aRhs.iGlyphColor, aRhs.iTextColor, aRhs.iPaperColor, aRhs.iSmartUnderline, aRhs.iIgnoreEmoji, aRhs.iTextEffect);
    }

    text_edit::paragraph_style::paragraph_style()
    {
    }

    text_edit::paragraph_style::paragraph_style(paragraph_style const& aOther) :
        iAlignment{ aOther.iAlignment },
        iPadding{ aOther.iPadding },
        iLineSpacing{ aOther.iLineSpacing }
    {
    }

    text_edit::paragraph_style::paragraph_style(optional_alignment const& aAlignment, optional_padding const& aPadding, optional<double> const& aLineSpacing) :
        iAlignment{ aAlignment },
        iPadding{ aPadding },
        iLineSpacing{ aLineSpacing }
    {
    }

    optional_alignment const& text_edit::paragraph_style::alignment() const
    {
        return iAlignment;
    }

    text_edit::paragraph_style& text_edit::paragraph_style::set_alignment(optional_alignment const& aAlignment)
    {
        iAlignment = aAlignment;
        return *this;
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
        if (aRhs.alignment() != std::nullopt)
            iAlignment = aRhs.alignment();
        if (aRhs.padding() != std::nullopt)
            iPadding = aRhs.padding();
        if (aRhs.line_spacing() != std::nullopt)
            iLineSpacing = aRhs.line_spacing();
        return *this;
    }

    bool text_edit::paragraph_style::operator==(const paragraph_style& aRhs) const
    {
        return std::forward_as_tuple(iAlignment, iPadding, iLineSpacing) == std::forward_as_tuple(aRhs.iAlignment, aRhs.iPadding, aRhs.iLineSpacing);
    }

    bool text_edit::paragraph_style::operator!=(const paragraph_style& aRhs) const
    {
        return !(*this == aRhs);
    }

    bool text_edit::paragraph_style::operator<(const paragraph_style& aRhs) const
    {
        return std::forward_as_tuple(iAlignment, iPadding, iLineSpacing) < std::forward_as_tuple(aRhs.iAlignment, aRhs.iPadding, aRhs.iLineSpacing);
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

    bool text_edit::style::has_cookie() const
    {
        return iCookie != neolib::invalid_cookie<style_cookie>;
    }

    text_edit::style_cookie text_edit::style::cookie() const
    {
        return iCookie;
    }

    void text_edit::style::set_cookie(style_cookie aCookie)
    {
        iCookie = aCookie;
    }

    void text_edit::style::add_ref() const
    {
        ++iUseCount;
    }

    void text_edit::style::release() const
    {
        if (iParent && &iParent->iDefaultStyle != this && --iUseCount == 0u)
        {
            iParent->iStyles.erase(iParent->iStyleMap[cookie()]);
            iParent->iStyleMap.remove(cookie());
        }
    }

    text_edit::style& text_edit::style::merge(const style& aOverridingStyle)
    {
        iCharacter.merge(aOverridingStyle.iCharacter);
        iParagraph.merge(aOverridingStyle.iParagraph);
        return *this;
    }

    bool text_edit::style::operator==(const style& aRhs) const
    {
        return std::forward_as_tuple(iCharacter) == std::forward_as_tuple(aRhs.iCharacter);
    }

    bool text_edit::style::operator!=(const style& aRhs) const
    {
        return !(*this == aRhs);
    }

    bool text_edit::style::operator<(const style& aRhs) const
    {
        return std::forward_as_tuple(iCharacter) < std::forward_as_tuple(aRhs.iCharacter);
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

    text_edit::style text_edit::style::with_font(optional_font const& aFont) const
    {
        style newStyle = *this;
        newStyle.character().set_font(aFont);
        return newStyle;
    }

    text_edit::style text_edit::style::with_glyph_color(const color_or_gradient& aColor) const
    {
        style newStyle = *this;
        newStyle.character().set_glyph_color(aColor);
        return newStyle;
    }

    text_edit::style text_edit::style::with_text_color(const color_or_gradient& aColor) const
    {
        style newStyle = *this;
        newStyle.character().set_text_color(aColor);
        return newStyle;
    }

    text_edit::style text_edit::style::with_paper_color(const color_or_gradient& aColor) const
    {
        style newStyle = *this;
        newStyle.character().set_paper_color(aColor);
        return newStyle;
    }

    text_edit::style text_edit::style::with_text_effect(const optional_text_effect& aEffect) const
    {
        style newStyle = *this;
        newStyle.character().set_text_effect(aEffect);
        return newStyle;
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
                bool notify = (iOwner.iWantedToNotifyTextChanged > 0u);
                iOwner.iWantedToNotifyTextChanged = 0u;
                if (notify)
                    iOwner.TextChanged();
            }
        }
    private:
        text_edit & iOwner;
    };

    struct text_edit::position_info
    {
        glyph_paragraphs::const_iterator paragraph;
        std::optional<glyph_columns::const_iterator> column;
        std::optional<glyph_lines::const_iterator> line;
        std::optional<document_glyphs::const_iterator> glyph;
        std::optional<document_glyphs::const_iterator> lineStart;
        std::optional<document_glyphs::const_iterator> lineEnd;
        point pos;

        glyph_columns::size_type column_index() const
        {
            return column.has_value() ? column.value()->index() : 0;
        }
    };

    dimension text_edit::glyph_paragraph::height(document_glyphs::iterator aStart, document_glyphs::iterator aEnd) const
    {
        if (heightMap.empty())
        {
            dimension previousHeight = 0.0;
            auto iterGlyph = std::next(owner->glyphs().begin(), span.glyphsFirst);
            for (auto i = span.glyphsFirst; i != span.glyphsLast; ++i)
            {
                auto const& glyph = *(iterGlyph++);
                dimension cy = owner->glyphs().extents(glyph).cy;
                if (i == span.glyphsFirst || cy != previousHeight)
                {
                    heightMap.emplace_back(i, cy);
                    previousHeight = cy;
                }
            }
            heightMap.emplace_back(span.glyphsLast, 0.0);
        }
        dimension result = 0.0;
        auto start = std::lower_bound(heightMap.begin(), heightMap.end(), height_map_entry{ aStart - owner->glyphs().begin() },
            [](auto const& lhs, auto const& rhs) { return lhs.glyphIndex < rhs.glyphIndex; });
        if (start != heightMap.begin() && aStart < owner->glyphs().begin() + start->glyphIndex)
            --start;
        auto stop = std::lower_bound(heightMap.begin(), heightMap.end(), height_map_entry{ aEnd - owner->glyphs().begin() },
            [](auto const& lhs, auto const& rhs) { return lhs.glyphIndex < rhs.glyphIndex; });
        if (start == stop && stop != heightMap.end())
            ++stop;
        for (auto i = start; i != stop; ++i)
            result = std::max(result, (*i).height);
        return result;
    }

    text_edit::text_edit(text_edit_caps aCaps, frame_style aFrameStyle) :
        framed_scrollable_widget{ (aCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine ? scrollbar_style::Normal : scrollbar_style::None, aFrameStyle, 2.0 },
        iCaps{ aCaps },
        iLineEnding{ text_edit_line_ending::AutomaticLf },
        iPersistDefaultStyle{ false },
        iCursor{ *this },
        iUpdatingDocument{ false },
        iHandlingKeyPress{ false },
        iColumns{ 1 },
        iCursorAnimationStartTime{ neolib::this_process::elapsed_ms() },
        iTabStopHint{ "0000" },
        iAnimator{ *this, [this](widget_timer&)
        {
            iAnimator.again();
            animate();
        }, std::chrono::milliseconds{ 16 } },
        iSuppressTextChangedNotification{ 0u },
        iWantedToNotifyTextChanged{ 0u },
        iOutOfMemory{ false }
    {
        init();
    }

    text_edit::text_edit(i_widget& aParent, text_edit_caps aCaps, frame_style aFrameStyle) :
        framed_scrollable_widget{ aParent, (aCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine ? scrollbar_style::Normal : scrollbar_style::None, aFrameStyle, 2.0 },
        iCaps{ aCaps },
        iLineEnding{ text_edit_line_ending::AutomaticLf },
        iPersistDefaultStyle{ false },
        iCursor{ *this },
        iUpdatingDocument{ false },
        iHandlingKeyPress{ false },
        iColumns{ 1 },
        iCursorAnimationStartTime{ neolib::this_process::elapsed_ms() },
        iTabStopHint{ "0000" },
        iAnimator{ *this, [this](widget_timer&)
        {
            iAnimator.again();
            animate();
        }, std::chrono::milliseconds{ 16 } },
        iSuppressTextChangedNotification{ 0u },
        iWantedToNotifyTextChanged{ 0u },
        iOutOfMemory{ false }
    {
        init();
    }

    text_edit::text_edit(i_layout& aLayout, text_edit_caps aCaps, frame_style aFrameStyle) :
        framed_scrollable_widget{ aLayout, (aCaps & text_edit_caps::MultiLine) == text_edit_caps::MultiLine ? scrollbar_style::Normal : scrollbar_style::None, aFrameStyle, 2.0 },
        iCaps{ aCaps },
        iLineEnding{ text_edit_line_ending::AutomaticLf },
        iPersistDefaultStyle{ false },
        iCursor{ *this },
        iUpdatingDocument{ false },
        iHandlingKeyPress{ false },
        iColumns{ 1 },
        iCursorAnimationStartTime{ neolib::this_process::elapsed_ms() },
        iTabStopHint{ "0000" },
        iAnimator{ *this, [this](widget_timer&)
        {
            iAnimator.again();
            animate();
        }, std::chrono::milliseconds{ 16 } },
        iSuppressTextChangedNotification{ 0u },
        iWantedToNotifyTextChanged{ 0u },
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

    void text_edit::layout_items(bool aDefer)
    {
        framed_scrollable_widget::layout_items(aDefer);
        if (!aDefer && iPasswordBits)
        {
            auto r = client_rect();
            r.x = r.right() - padding().right;
            auto& spb = iPasswordBits.value().showPassword;
            spb.resize(spb.minimum_size());
            spb.move(point{ r.x, std::ceil(r.cy - spb.extents().cy) / 2.0 });
        }
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
            result.cy = std::max(result.cy, frameBits.cy + std::min(iTextExtents.value().cy, grow_lines() * font().height()));
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
        scoped_units su{ *this, units::Pixels };
        auto result = framed_scrollable_widget::padding();
        return to_units(*this, su.saved_units(), result + padding_adjust());
    }

    void text_edit::paint(i_graphics_context& aGc) const
    {
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << "text_edit::paint(...)" << std::endl;
#endif // NEOGFX_DEBUG

        framed_scrollable_widget::paint(aGc);
        rect clipRect = default_clip_rect().intersection(client_rect(false));
        clipRect.inflate(size{ padding_adjust() });

        if (iOutOfMemory)
        {
            draw_alpha_background(aGc, clipRect);
            return;
        }

        scoped_scissor scissor{ aGc, clipRect };
        if (default_style().character().paper_color() != neolib::none)
            aGc.fill_rect(client_rect(), to_brush(default_style().character().paper_color()));

        auto const top = vertical_scrollbar().position();

        coordinate x = 0.0;
        for (auto columnIndex = 0u; columnIndex < iGlyphColumns.size(); ++columnIndex)
        {
            auto const& columnRectSansPadding = column_rect(columnIndex);

            auto const& glyphColumn = iGlyphColumns[columnIndex];

            auto columnClipRect = clipRect.intersection(column_rect(columnIndex, true));
            columnClipRect.inflate(size{ std::max(calc_padding_adjust(column_style(columnIndex)), padding_adjust()) });
            scoped_scissor scissor2{ aGc, columnClipRect };

            auto const& lines = glyphColumn.lines;
            auto line = std::lower_bound(lines.begin(), lines.end(), top,
                [](const glyph_line& left, coordinate const& right) { return left.ypos() < right; });
            if (line != lines.begin() && (line == lines.end() || top < line->ypos()))
                --line;
            if (line == lines.end())
                continue;
            for (auto paintLine = line; paintLine != lines.end(); paintLine++)
            {
                point linePos = columnRectSansPadding.top_left() + point{ -horizontal_scrollbar().position(), paintLine->ypos() - top};
                if (linePos.y + paintLine->extents.cy < columnRectSansPadding.top() || linePos.y + paintLine->extents.cy < update_rect().top())
                    continue;
                if (linePos.y > columnRectSansPadding.bottom() || linePos.y > update_rect().bottom())
                    break;
                linePos.x += paintLine->xpos();
                draw_glyphs(aGc, linePos, glyphColumn, paintLine);
            }
            x += glyphColumn.width;
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
        {
            glyphs().set_major_font(font());
            refresh_paragraph(iText.begin(), 0);
        }
    }

    bool text_edit::capture_locks_cursor() const
    {
        return false;
    }

    void text_edit::focus_gained(focus_reason aFocusReason)
    {
        framed_scrollable_widget::focus_gained(aFocusReason);

        if (neolib::service<i_keyboard>().layout().ime_open())
            neolib::service<i_keyboard>().layout().activate_ime(*this, cursor_rect().bottom_left());

        neolib::service<neolib::i_power>().register_activity();

        service<i_clipboard>().activate(*this);

        iCursorAnimationStartTime = neolib::this_process::elapsed_ms();

        if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::SingleLine && aFocusReason == focus_reason::Tab)
        {
            cursor().set_anchor(0);
            cursor().set_position(iText.size(), false);
        }

        update(true);
    }

    void text_edit::focus_lost(focus_reason aFocusReason)
    {
        destroyed_flag destroyed{ *this };

        framed_scrollable_widget::focus_lost(aFocusReason);

        if (destroyed)
            return;

        if (neolib::service<i_keyboard>().layout().ime_open())
            neolib::service<i_keyboard>().layout().deactivate_ime(*this);

        if (service<i_clipboard>().sink_active() && &service<i_clipboard>().active_sink() == this)
            service<i_clipboard>().deactivate(*this);

        if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::SingleLine)
            cursor().set_position(iText.size());

        update(true);
    }

    void text_edit::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        framed_scrollable_widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left && client_rect().contains(aPosition))
        {
            auto const docPos = document_hit_test_ex(aPosition);
            if ((iCaps & text_edit_caps::ParseURIs) == text_edit_caps::ParseURIs && read_only() && docPos.second)
            {
                auto wordSpan = word_at(docPos.first, true);
                thread_local std::u32string utf32word;
                utf32word.clear();
                std::copy(std::next(iText.begin(), wordSpan.first), std::next(iText.begin(), wordSpan.second), std::back_inserter(utf32word));
                if (utf32word.find(U"://") != std::u32string::npos)
                    iSelectedUri = wordSpan;
            }
            set_cursor_position(aPosition, (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE, capturing());
        }
        else if (aButton == mouse_button::Right && focus_policy() == neogfx::focus_policy::NoFocus && capture_ok(hit_test(aPosition)) && can_capture())
        {
            set_capture(capture_reason::MouseEvent, aPosition);
            if (cursor().position() == cursor().anchor())
                set_cursor_position(aPosition, (aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE, capturing());
        }
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
        bool wasCapturing = capturing();
        framed_scrollable_widget::mouse_button_released(aButton, aPosition);
        iDragger = nullptr;
        if (aButton == mouse_button::Left && client_rect().contains(aPosition))
        {
            auto const docPos = document_hit_test_ex(aPosition);
            if ((iCaps & text_edit_caps::ParseURIs) == text_edit_caps::ParseURIs && read_only() && docPos.second &&
                cursor().position() == cursor().anchor() && iSelectedUri)
            {
                auto wordSpan = word_at(docPos.first, true);
                if (iSelectedUri == wordSpan)
                {
                    thread_local std::u32string utf32word;
                    utf32word.clear();
                    std::copy(std::next(iText.begin(), wordSpan.first), std::next(iText.begin(), wordSpan.second), std::back_inserter(utf32word));
                    if (utf32word.find(U"://") != std::u32string::npos)
                    {
                        if (!service<i_basic_services>().open_uri(neolib::utf32_to_utf8(utf32word)))
                            service<i_basic_services>().system_beep();
                    }
                }
            }
        }
        else if (aButton == mouse_button::Right)
        {
            iMenu = std::make_unique<neogfx::context_menu>(*this, aPosition + non_client_rect().top_left() + root().window_position());
            ContextMenu(iMenu->menu());
            if (!read_only())
            {
                iMenu->menu().add_action(service<i_app>().action_undo());
                iMenu->menu().add_action(service<i_app>().action_redo());
                iMenu->menu().add_separator();
                iMenu->menu().add_action(service<i_app>().action_cut());
            }
            iMenu->menu().add_action(service<i_app>().action_copy());
            if (!read_only())
            {
                iMenu->menu().add_action(service<i_app>().action_paste());
            }
            auto pastePlainText = make_ref<action>("Plain Text"_t);
            auto pasteRichText = make_ref<action>("Rich Text (HTML)"_t);
            pastePlainText->Triggered([this]() { paste_plain_text(); });
            pasteRichText->Triggered([this]() { paste_rich_text(); });
            sink tempSink;
            tempSink += service<i_app>().action_paste().enabled([&pastePlainText, &pasteRichText]() { pastePlainText->enable(); pasteRichText->enable(); });
            tempSink += service<i_app>().action_paste().disabled([&pastePlainText, &pasteRichText]() { pastePlainText->disable(); pasteRichText->disable(); });
            if (!read_only())
            {
                auto& pasteAs = iMenu->menu().add_sub_menu("Paste As"_t);
                pasteAs.add_action(pastePlainText);
                pasteAs.add_action(pasteRichText);
                iMenu->menu().add_action(service<i_app>().action_delete());
            }
            iMenu->menu().add_separator();
            iMenu->menu().add_action(service<i_app>().action_select_all());
            if (!read_only())
            {
                iMenu->menu().add_separator();
                if (!neolib::service<i_keyboard>().layout().ime_open())
                {
                    iMenu->menu().add_action(make_ref<action>("Open IME"_t)).triggered([&]()
                    {
                        neolib::service<i_keyboard>().layout().open_ime();
                        neolib::service<i_keyboard>().layout().activate_ime(*this, cursor_rect().bottom_left());
                    });
                }
                else
                {
                    iMenu->menu().add_action(make_ref<action>("Close IME"_t)).triggered([&]()
                    {
                        neolib::service<i_keyboard>().layout().close_ime();
                    });
                }
            }
            bool selectAll = false;
            tempSink += service<i_app>().action_select_all().triggered([&]() { selectAll = true; });
            scoped_clipboard_sink scs{ *this };
            iMenu->exec();
            iMenu.reset();
            if (!selectAll && focus_policy() == neogfx::focus_policy::NoFocus && wasCapturing)
                cursor().set_anchor(cursor().position());
        }
        iSelectedUri = std::nullopt;
    }

    void text_edit::mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        framed_scrollable_widget::mouse_moved(aPosition, aKeyModifiers);
        neolib::service<neolib::i_power>().register_activity();
        if (iDragger != nullptr)
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
        auto const mousePosition = mouse_position();
        auto const clientRect = client_rect(false);
        if (clientRect.contains(mousePosition) || iDragger != nullptr)
        {
            auto const docPos = document_hit_test_ex(mousePosition);
            if (docPos.first >= static_cast<position_type>(iText.size()))
                return mouse_system_cursor::IBeam;
            if (docPos.second && iText[docPos.first].tag != neolib::invalid_cookie<tag_cookie> &&
                cursor().position() == cursor().anchor())
            {
                auto const& tag = *iTagMap[iText[docPos.first].tag];
                if (tag.query_mouse_cursor().has_slots())
                {
                    neogfx::mouse_cursor cursor = mouse_system_cursor::IBeam;
                    tag.ev_query_mouse_cursor().trigger(cursor);
                    return cursor;
                }
                else if (tag.mouse_event().has_slots())
                    return mouse_system_cursor::Hand;
            }
            if (docPos.second && (iCaps & text_edit_caps::ParseURIs) == text_edit_caps::ParseURIs && read_only() &&
                cursor().position() == cursor().anchor())
            {
                auto wordSpan = word_at(docPos.first, true);
                thread_local std::u32string utf32word;
                utf32word.clear();
                std::copy(std::next(iText.begin(), wordSpan.first), std::next(iText.begin(), wordSpan.second), std::back_inserter(utf32word));
                if (utf32word.find(U"://") != std::u32string::npos)
                    return mouse_system_cursor::Hand;
            }
            return mouse_system_cursor::IBeam;
        }
        else
            return framed_scrollable_widget::mouse_cursor();
    }

    bool text_edit::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        neolib::service<neolib::i_power>().register_activity();

        neolib::scoped_flag sf{ iHandlingKeyPress };

        bool handled = true;
        switch (aScanCode)
        {
        case ScanCode_TAB:
            if (!read_only())
            {
                multiple_text_changes mtc{ *this };
                delete_any_selection();
                insert_text(string{ "\t" }, { next_style() });
                cursor().set_position(cursor().position() + 1);
            }
            break;
        case ScanCode_RETURN:
        case ScanCode_KEYPAD_ENTER:
            if (!read_only())
            {
                if ((aKeyModifiers & KeyModifier_CTRL) == KeyModifier_NONE)
                {
                    bool canAccept = false;
                    CanAcceptText(text(), canAccept);
                    if (canAccept)
                    {
                        AcceptText(text());
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
                        insert_text(string{ "\n" }, { next_style() });
                    else
                        insert_text(string{ "\r" }, { next_style() });
                    cursor().set_position(cursor().position() + 1);
                    iCursorHint.x = std::nullopt;
                }
                else
                    handled = framed_scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
            }
            else
                handled = framed_scrollable_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
            break;
        case ScanCode_BACKSPACE:
            if (!read_only())
            {
                if (cursor().position() == cursor().anchor())
                {
                    if (cursor().position() > 0)
                    {
                        auto const from = cursor().position() - 1;
                        auto const to = cursor().position();
                        cursor().set_position(cursor().position() - 1);
                        delete_text(from, to);
                        make_cursor_visible(true);
                    }
                }
                else
                    delete_any_selection();
            }
            break;
        case ScanCode_DELETE:
            if (!read_only())
            {
                if (cursor().position() == cursor().anchor())
                {
                    if (cursor().position() < glyphs().size())
                    {
                        auto const from = cursor().position();
                        auto const to = cursor().position() + 1;
                        delete_text(from, to);
                        make_cursor_visible(true);
                    }
                }
                else
                    delete_any_selection();
            }
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
                    auto pos = point{ glyph_position(cursor_glyph_position()).pos - point{ horizontal_scrollbar().Position.effective_value(), vertical_scrollbar().Position.effective_value()}};
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
            if (cursor().position() == cursor().anchor() &&
                overwrite_cursor_available() &&
                (service<i_keyboard>().locks() & keyboard_locks::InsertLock) != keyboard_locks::InsertLock)
            {
                if (cursor().position() < glyphs().size())
                {
                    auto const from = cursor().position();
                    auto const to = cursor().position() + 1;
                    delete_text(from, to);
                    make_cursor_visible(true);
                }
            }
            else
                delete_any_selection();
            insert_text(aText, { next_style() }, true);
        }
        return true;
    }

    scrolling_disposition text_edit::scrolling_disposition() const
    {
        return neogfx::scrolling_disposition::DontScrollChildWidget;
    }

    scrolling_disposition text_edit::scrolling_disposition(const i_widget& aWidget) const
    {
        if (iPasswordBits && &iPasswordBits.value().showPassword == &aWidget)
            return neogfx::scrolling_disposition::DontScrollChildWidget;
        return framed_scrollable_widget::scrolling_disposition(aWidget);
    }

    rect text_edit::scroll_area() const
    {
        auto const& internalPadding = padding();
        auto const& columnPadding = column(0).padding; ///< todo: other columns?
        auto const& totalPadding = internalPadding + columnPadding;
        return rect{ point{}, iTextExtents.value_or(size{}) + totalPadding.size() };
    }

    rect text_edit::scroll_page() const
    {
        return framed_scrollable_widget::scroll_page();
    }

    bool text_edit::use_scrollbar_container_updater() const
    {
        return false;
    }

    bool text_edit::update_scrollbar_visibility(usv_stage_e aStage)
    {
        std::optional<scoped_property_transition_suppression> sts1;
        std::optional<scoped_property_transition_suppression> sts2;

        if (iTextExtents != std::nullopt) // must be a resize event
        {
            sts1.emplace(vertical_scrollbar().Position);
            sts2.emplace(horizontal_scrollbar().Position);
        }

        switch (aStage)
        {
        case UsvStageInit:
            if (resizing())
                vertical_scrollbar().push_zone();
            vertical_scrollbar().set_step(font().height());
            horizontal_scrollbar().set_step(font().height());
            if (!framed_scrollable_widget::update_scrollbar_visibility(aStage))
                refresh_lines();
            break;
        case UsvStageDone:
            framed_scrollable_widget::update_scrollbar_visibility(aStage);
            if (has_focus() && !read_only())
                make_cursor_visible();
            else if (resizing())
            {
                auto zone = vertical_scrollbar().pop_zone();
                switch(zone)
                {
                case scrollbar_zone::Top:
                    make_cursor_visible();
                    break;
                case scrollbar_zone::Middle:
                    make_visible(glyph_position(glyph_hit_test(column_rect(0).top_left()).first, true));
                    break;
                case scrollbar_zone::Bottom:
                    vertical_scrollbar().set_position(vertical_scrollbar().maximum());
                    horizontal_scrollbar().set_position(0.0);
                    break;
                }
            }
            break;
        default:
            return framed_scrollable_widget::update_scrollbar_visibility(aStage);
        }

        return true;
    }

    void text_edit::scroll_page_updated()
    {
        refresh_lines();
    }

    color text_edit::frame_color() const
    {
        if (has_frame_color())
            return framed_scrollable_widget::frame_color();
        else if (has_focus())
            return service<i_app>().current_style().palette().color(color_role::Focus);
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
            auto len = insert_text(aClipboard.text(), { next_style() });
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
                auto previousLine = previous_line(currentPosition.line);
                if (previousLine)
                {
                    auto const columnRectSansPadding = column_rect(currentPosition.column_index());
                    auto const cursorPos = point{ *iCursorHint.x, previousLine.value()->ypos()} + columnRectSansPadding.top_left();
                    auto const documentPos = document_hit_test(cursorPos, false);
                    cursor().set_position(documentPos, aMoveAnchor);
                }
            }
            break;
        case cursor::Down:
            {
                auto currentPosition = glyph_position(cursor_glyph_position());
                auto nextLine = next_line(currentPosition.line);
                if (nextLine)
                {
                    auto const columnRectSansPadding = column_rect(currentPosition.column_index());
                    auto const cursorPos = point{ *iCursorHint.x, nextLine.value()->ypos() } + columnRectSansPadding.top_left();
                    auto const documentPos = document_hit_test(cursorPos, false);
                    cursor().set_position(documentPos, aMoveAnchor);
                }
                else if (currentPosition.lineEnd.value() == glyphs().end() || is_line_breaking_whitespace(*currentPosition.lineEnd.value()))
                    cursor().set_position(iText.size(), aMoveAnchor);
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

    bool text_edit::overwrite_cursor_available() const
    {
        return (iCaps & text_edit_caps::OverwriteMode) == text_edit_caps::OverwriteMode;
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

    text_edit_line_ending text_edit::line_ending() const
    {
        return iLineEnding;
    }

    void text_edit::set_line_ending(text_edit_line_ending aLineEnding)
    {
        iLineEnding = aLineEnding;
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

    std::uint32_t text_edit::grow_lines() const
    {
        return GrowLines;
    }

    void text_edit::set_grow_lines(std::uint32_t aGrowLines)
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

    const text_edit::style& text_edit::default_style() const
    {
        return iDefaultStyle;
    }

    void text_edit::set_default_style(const style& aDefaultStyle, bool aPersist)
    {
        neogfx::font oldFont = font();
        auto oldEffect = (default_style().character().text_effect() == std::nullopt);
        iDefaultStyle = aDefaultStyle;
        if (oldFont != font() || oldEffect != (default_style().character().text_effect() == std::nullopt))
        {
            glyphs().set_major_font(font());
            refresh_paragraph(iText.begin(), 0);
        }
        iPersistDefaultStyle = aPersist;
        DefaultStyleChanged();
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

    alignment text_edit::alignment() const
    {
        return default_style().paragraph().alignment().as_std_optional().value_or(neogfx::alignment::Left | neogfx::alignment::Top);
    }

    void text_edit::set_alignment(neogfx::alignment aAlignment)
    {
        if (alignment() != aAlignment)
        {
            iDefaultStyle.paragraph().set_alignment(aAlignment);
            DefaultStyleChanged();
            update();
        }
    }

    text_edit::style text_edit::current_style() const
    {
        if (iText.empty())
            return style{ default_style() }.character().set_font_if_none(font());
        auto t = std::next(iText.begin(), cursor().anchor());
        if (t != iText.begin() && cursor().position() == cursor().anchor())
            t = std::prev(t);
        auto const g = to_glyph(t);
        auto style = glyph_style(g, iColumns.at(glyph_position(g - glyphs().begin(), true).column_index()));
        style.character().set_font_if_none(g != glyphs().end() ? glyphs().glyph_font(*g) : font());
        return style;
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
        insert_text(aStart, string{ neolib::utf32_to_utf8(part) }, { aStyle });
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
        auto const docPos = document_hit_test(aPosition);
        cursor().set_position(docPos, aMoveAnchor);
        neolib::service<neolib::i_power>().register_activity();
        if (aEnableDragger)
        {
            if (!capturing())
                set_capture();
            iDragger = std::make_unique<dragger>(*this);
        }
    }

    void text_edit::cancel_object_selection()
    {
        iSelectedUri = std::nullopt;
    }

    rect text_edit::column_rect(std::size_t aColumnIndex, bool aExtendIntoPadding) const
    {
        auto result = page_rect();
        for (std::size_t ci = 0; ci < aColumnIndex; ++ci)
            result.x += iColumns.at(ci).width;
        if (!aExtendIntoPadding)
            result.deflate(iColumns.at(aColumnIndex).info.padding);
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
        auto paragraph = std::lower_bound(iGlyphParagraphs.begin(), iGlyphParagraphs.end(), document_span{ 0, 0, aGlyphPosition, aGlyphPosition },
            [](auto const& p, auto const& s) { return p.glyph_begin_index() < s.glyphsFirst; });
        if (paragraph != iGlyphParagraphs.begin())
        {
            if (paragraph == iGlyphParagraphs.end() || aGlyphPosition < paragraph->glyph_begin_index())
                paragraph = std::prev(paragraph);
        }
        if (paragraph == iGlyphParagraphs.end())
            return { paragraph };
        std::optional<glyph_lines::const_iterator> match;
        for(auto column = iGlyphColumns.begin(); column != iGlyphColumns.end(); ++column)
        {
            auto lineTry = std::lower_bound(column->lines.begin(), column->lines.end(), document_span{ 0, 0, aGlyphPosition, aGlyphPosition },
                [](auto const& l, auto const& s) { return l.glyph_begin_index() < s.glyphsFirst; });
            if (lineTry != column->lines.begin())
            {
                if (lineTry == column->lines.end() || aGlyphPosition < lineTry->glyph_begin_index())
                    lineTry = std::prev(lineTry);
            }
            if (match == std::nullopt || match.value()->glyph_begin_index() < lineTry->glyph_begin_index())
                match = lineTry;
        }
        auto const line = match.value();
        auto const columnIndex = line->columnIndex;
        auto const column = std::next(iGlyphColumns.begin(), columnIndex);
        auto const& columnRectSansPadding = column_rect(columnIndex);
        auto const& lines = column->lines;
        if (line != lines.end())
        {
            position_type lineStart = line->glyph_begin_index();
            position_type lineEnd = line->glyph_end_index();
            auto textDirection = glyph_text_direction(line->glyph_begin(), line->glyph_end());
            bool placeCursorToRight = (aGlyphPosition == lineEnd);
            if (aForCursor)
            {
                if (aGlyphPosition == static_cast<position_type>(glyphs().size()) || aGlyphPosition == paragraph->glyph_end_index())
                {
                    auto iterChar = iText.begin() + from_glyph(glyphs().begin() + aGlyphPosition).first;
                    if (iterChar != iText.begin())
                    {
                        auto iterGlyph = to_glyph(iterChar - 1);
                        auto const& g = *iterGlyph;
                        if (direction(g) == text_direction::RTL)
                        {
                            aGlyphPosition = iterGlyph - glyphs().begin();
                            placeCursorToRight = (direction(g) == text_direction::RTL ? false : true);
                        }
                    }
                }
                else if (direction(glyphs()[aGlyphPosition]) == text_direction::RTL)
                    placeCursorToRight = true;
                else if (aGlyphPosition == lineEnd)
                    placeCursorToRight = (textDirection != text_direction::RTL);
            }
            if (aGlyphPosition >= lineStart && aGlyphPosition <= lineEnd)
            {
                delta alignmentAdjust;
                auto const& paragraphStyle = glyph_style(line->glyph_begin(), iColumns.at(columnIndex));
                auto const paragraphAlignment = paragraphStyle.paragraph().alignment().as_std_optional().value_or(neogfx::alignment::Left | neogfx::alignment::Top);
                if (((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Left && textDirection == text_direction::RTL) ||
                    ((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Right && textDirection == text_direction::LTR))
                    alignmentAdjust.dx = columnRectSansPadding.cx - line->extents.cx;
                else if ((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Center)
                    alignmentAdjust.dx = (columnRectSansPadding.cx - line->extents.cx) / 2.0;
                if (lineStart != lineEnd)
                {
                    auto iterGlyph = glyphs().begin() + aGlyphPosition;
                    auto const& glyphForPos = aGlyphPosition < lineEnd ? *iterGlyph : 
                        (textDirection == text_direction::LTR ? 
                            *(iterGlyph - 1) : 
                            *(glyphs().begin() + lineStart));
                    point linePos{ glyphForPos.cell[0].x - line->glyph_begin()->cell[0].x, line->ypos()};
                    if (placeCursorToRight)
                        linePos.x += glyphForPos.cell_extents().x;
                    return position_info{ paragraph, column, line, iterGlyph, line->glyph_begin(), line->glyph_end(), linePos + alignmentAdjust};
                }
                else
                    return position_info{ paragraph, column, line, line->glyph_begin(), line->glyph_begin(), line->glyph_end(), point{ 0.0, line->ypos()} + alignmentAdjust};
            }
        }
        point pos;
        if (!lines.empty())
        {
            pos.x = 0.0;
            auto textDirection = glyph_text_direction(lines.back().glyph_begin(), lines.back().glyph_end());
            auto const& paragraphStyle = glyph_style(lines.back().glyph_begin(), iColumns.at(columnIndex));
            auto const paragraphAlignment = paragraphStyle.paragraph().alignment().as_std_optional().value_or(neogfx::alignment::Left | neogfx::alignment::Top);
            if (((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Left && textDirection == text_direction::RTL) ||
                ((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Right && textDirection == text_direction::LTR))
                pos.x = columnRectSansPadding.cx;
            else if ((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Center)
                pos.x = columnRectSansPadding.cx / 2.0;
            pos.y = lines.back().ypos() + lines.back().extents.cy;
        }
        return position_info{ iGlyphParagraphs.end(), {}, {}, glyphs().end(), glyphs().end(), glyphs().end(), pos };
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

    std::pair<text_edit::document_glyphs::difference_type, bool> text_edit::glyph_hit_test(const point& aPosition, bool aAdjustForScrollPosition) const
    {
        if (iGlyphParagraphs.empty())
            return std::make_pair(0, false);
        auto const columnIndex = column_hit_test(aPosition, aAdjustForScrollPosition);
        auto const& columnRectSansPadding = column_rect(columnIndex);
        point adjustedPosition = (aAdjustForScrollPosition ? aPosition + point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } : aPosition) - columnRectSansPadding.top_left();
        adjustedPosition = adjustedPosition.max(point{});
        auto paragraph = std::lower_bound(iGlyphParagraphs.begin(), iGlyphParagraphs.end(), adjustedPosition.y,
            [](glyph_paragraph const& p, coordinate y)
            {
                return p.ypos < y;
            });
        if (paragraph == iGlyphParagraphs.end())
            paragraph = std::prev(paragraph);
        if (paragraph != iGlyphParagraphs.begin() && adjustedPosition.y < paragraph->ypos)
            paragraph = std::prev(paragraph);
        if (paragraph == iGlyphParagraphs.begin() && adjustedPosition.y < paragraph->ypos)
            return std::make_pair(0, false);
        auto const& column = iGlyphColumns.at(columnIndex);
        auto const& lines = column.lines;
        auto line = std::lower_bound(lines.begin(), lines.end(), adjustedPosition.y,
            [](const glyph_line& left, coordinate y) { return left.ypos() < y; });
        if (line == lines.end() && !lines.empty() && adjustedPosition.y < lines.back().ypos() + lines.back().extents.cy)
            --line;
        if (line != lines.end())
        {
            if (line != lines.begin() && adjustedPosition.y < line->ypos())
                --line;
            adjustedPosition.x -= line->xpos();
            auto const lineStart = line->glyph_begin_index();
            auto const lineEnd = line->glyph_end_index();
            auto const lineStartX = line->glyph_begin()->cell[0].x;
            auto const lineEndX = (lineEnd != lineStart ? std::prev(line->glyph_end())->cell[1].x : lineStartX);
            if (adjustedPosition.x >= lineEndX)
            {
                if (lineEnd > lineStart && is_line_breaking_whitespace(glyphs()[lineEnd - 1]))
                    return std::make_pair(lineEnd - 1, true);
                return std::make_pair(lineEnd, true);
            }
            for (auto gi = lineStart; gi != lineEnd; ++gi)
            {
                auto const& glyph = glyphs()[gi];
                auto const glyphAdvance = glyph.cell_extents().x;
                if (adjustedPosition.x >= glyph.cell[0].x - lineStartX && adjustedPosition.x < glyph.cell[0].x - lineStartX + glyphAdvance)
                {
                    bool const inLeftHalf = adjustedPosition.x < glyph.cell[0].x - lineStartX + glyphAdvance / 2.0 || glyphAdvance == 0.0;
                    if (direction(glyph) == text_direction::LTR)
                        return std::make_pair(inLeftHalf ? gi : gi + 1, true);
                    else
                        return std::make_pair(inLeftHalf && gi != lineStart ? gi - 1 : (inLeftHalf ? lineEnd : gi), true);
                }
            }
            if (lineEnd > lineStart && line != std::prev(lines.end()) && !is_whitespace(glyphs()[lineEnd]))
                return std::make_pair(lineEnd - 1, true);
            if (lineEnd > lineStart && direction(glyphs()[lineEnd - 1]) == text_direction::RTL)
                return std::make_pair(lineEnd - 1, true);
            return std::make_pair(lineEnd, true);
        }
        else
        {
            return std::make_pair(glyphs().size(), false);
        }
    }

    text_edit::position_type text_edit::document_hit_test(const point& aPosition, bool aAdjustForScrollPosition) const
    {
        return document_hit_test_ex(aPosition, aAdjustForScrollPosition).first;
    }

    std::pair<text_edit::position_type, bool> text_edit::document_hit_test_ex(const point& aPosition, bool aAdjustForScrollPosition) const
    {
        auto const glyphPosition = glyph_hit_test(aPosition, aAdjustForScrollPosition);
        if (glyphPosition.first < static_cast<document_glyphs::difference_type>(glyphs().size()))
        {
            auto const glyphParagraph = glyph_to_paragraph(glyphPosition.first);
            if (glyphParagraph != iGlyphParagraphs.end())
                return std::make_pair(glyphParagraph->text_begin_index() + glyphs()[glyphPosition.first].clusters.first, glyphPosition.second);
        }
        return std::make_pair(iText.size(), false);
    }

    bool text_edit::same_word(position_type aTextPositionLeft, position_type aTextPositionRight) const
    {
        if (aTextPositionRight == static_cast<position_type>(iText.size()))
            return false;
        if (iText[aTextPositionLeft] == U'\n' || iText[aTextPositionRight] == U'\n' ||
            iText[aTextPositionLeft] == U'\r' || iText[aTextPositionRight] == U'\r')
            return false;
        auto const& emojiAtlas = service<i_font_manager>().emoji_atlas();
        return get_text_category(emojiAtlas, iText[aTextPositionLeft]) == get_text_category(emojiAtlas, iText[aTextPositionRight]);
    }

    std::pair<text_edit::position_type, text_edit::position_type> text_edit::word_at(position_type aTextPosition, bool aWordBreakIsWhitespace) const
    {
        auto is_space =
            [&](char32_t ch)
            {
                switch (ch)
                {
                case U'\r':
                case U'\n':
                case U'\t':
                case U'\f':
                case U'\v':
                case U' ':
                    return true;
                default:
                    if ((iCaps & text_edit_caps::NonPrintableWhitespace) == text_edit_caps::NonPrintableWhitespace)
                        return ch < U' ' || ch == U'\x7F';
                    return false;
                }
            };
        auto start = aTextPosition;
        auto end = aTextPosition;
        if (aWordBreakIsWhitespace && (start == static_cast<position_type>(iText.size()) || is_space(iText[start])))
            return std::make_pair(start, end);
        while (start > 0 && (aWordBreakIsWhitespace ? !is_space(iText[start - 1]) : same_word(start - 1, aTextPosition)))
            --start;
        while (end < static_cast<position_type>(iText.size()) && (aWordBreakIsWhitespace ? !is_space(iText[end]) : same_word(aTextPosition, end)))
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
        iUtf8TextCache = std::nullopt;
        iStyles.clear();
        iStyleMap.clear();
        iTags.clear();
        iTagMap.clear();
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
        return set_text(aText, {});
    }

    std::size_t text_edit::set_text(i_string const& aText, format const& aFormat)
    {
        return do_insert_text(0, aText, aFormat, true, true);
    }

    std::size_t text_edit::append_text(i_string const& aText, bool aMoveCursor)
    {
        return do_insert_text(iText.size(), aText, {}, aMoveCursor, false);
    }

    std::size_t text_edit::append_text(i_string const& aText, format const& aFormat, bool aMoveCursor)
    {
        return do_insert_text(iText.size(), aText, aFormat, aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(i_string const& aText, bool aMoveCursor)
    {
        return do_insert_text(cursor().position(), aText, {}, aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(i_string const& aText, format const& aFormat, bool aMoveCursor)
    {
        return do_insert_text(cursor().position(), aText, aFormat, aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(position_type aPosition, i_string const& aText, bool aMoveCursor)
    {
        return do_insert_text(aPosition, aText, {}, aMoveCursor, false);
    }

    std::size_t text_edit::insert_text(position_type aPosition, i_string const& aText, format const& aFormat, bool aMoveCursor)
    {
        return do_insert_text(aPosition, aText, aFormat, aMoveCursor, false);
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

        for (auto& ch : std::ranges::subrange(eraseBegin, eraseEnd))
        {
            auto existingStyle = iStyleMap.find(ch.style);
            if (existingStyle != iStyleMap.end())
                (**existingStyle).release();
            auto existingTag = iTagMap.find(ch.tag);
            if (existingTag != iTagMap.end())
                (**existingTag).release();
        }
        refresh_paragraph(iText.erase(eraseBegin, eraseEnd), -eraseAmount);

        update();

        if (iPreviousText != iText)
            notify_text_changed();
    }

    bool text_edit::same_paragraph(position_type aFirstGlyphPos, position_type aSecondGlyphPos) const
    {
        return glyph_to_paragraph(aFirstGlyphPos) == glyph_to_paragraph(aSecondGlyphPos);
    }

    text_edit::paragraph_span text_edit::character_to_paragraph(position_type aCharacterPos) const
    {
        auto paragraph = std::lower_bound(iGlyphParagraphs.begin(), iGlyphParagraphs.end(), aCharacterPos,
            [](glyph_paragraph const& p, position_type cp)
            {
                return p.text_begin_index() < cp;
            });
        if (paragraph != iGlyphParagraphs.begin() && (paragraph == iGlyphParagraphs.end() || aCharacterPos < paragraph->text_begin_index()))
            paragraph = std::prev(paragraph);
        return { static_cast<std::size_t>(std::distance(iGlyphParagraphs.begin(), paragraph)), paragraph->span };
    }

    text_edit::paragraph_line_span text_edit::character_to_line(position_type aCharacterPos) const
    {
        for (auto const& column : iGlyphColumns)
        {
            auto line = std::lower_bound(column.lines.begin(), column.lines.end(), aCharacterPos,
                [](glyph_line const& l, position_type cp)
                {
                    return l.text_begin_index() < cp;
                });
            if (line != column.lines.begin() && (line == column.lines.end() || aCharacterPos < line->text_begin_index()))
                line = std::prev(line);
            if (line == column.lines.end())
                continue;
            if (aCharacterPos < line->text_begin_index())
                continue;
            if (aCharacterPos < line->text_end_index())
                return line->paragraph_span();
            if (&column == &*std::prev(iGlyphColumns.end()) && std::next(line) == column.lines.end())
                return line->paragraph_span();
        }
        return paragraph_line_span{};
    }

    text_edit::glyph_paragraphs::const_iterator text_edit::glyph_to_paragraph(position_type aGlyphPos) const
    {
        auto paragraph = std::lower_bound(iGlyphParagraphs.begin(), iGlyphParagraphs.end(), aGlyphPos,
            [](glyph_paragraph const& p, position_type gp) { return p.glyph_begin_index() < gp; });
        if (paragraph != iGlyphParagraphs.begin() && 
            (paragraph == iGlyphParagraphs.end() || aGlyphPos < paragraph->glyph_begin_index()))
            paragraph = std::prev(paragraph);
        return paragraph;
    }

    std::size_t text_edit::columns() const
    {
        return iColumns.size();
    }

    void text_edit::set_columns(std::size_t aColumnCount)
    {
        iColumns.resize(aColumnCount);
        refresh_paragraph(iText.begin(), 0);
    }

    void text_edit::remove_columns()
    {
        iColumns.resize(1);
        iColumns[0] = {};
        refresh_paragraph(iText.begin(), 0);
    }

    const text_edit::column_info& text_edit::column(std::size_t aColumnIndex) const
    {
        if (aColumnIndex >= iColumns.size())
            throw bad_column_index();
        return iColumns[aColumnIndex].info;
    }

    void text_edit::set_column(std::size_t aColumnIndex, const column_info& aColumn)
    {
        if (aColumnIndex >= iColumns.size())
            throw bad_column_index();
        if (iColumns[aColumnIndex].info != aColumn)
        {
            iColumns[aColumnIndex].info = aColumn;
            refresh_paragraph(iText.begin(), 0);
        }
    }

    const text_edit::style& text_edit::column_style(std::size_t aColumnIndex) const
    {
        return column_style(column(aColumnIndex));
    }

    const text_edit::style& text_edit::column_style(const column_info& aColumn) const
    {
        if (aColumn.style)
            return *aColumn.style;
        return default_style();
    }

    bool text_edit::has_page_rect() const
    {
        return iPageRect != std::nullopt;
    }

    rect text_edit::page_rect() const
    {
        if (has_page_rect())
            return iPageRect.value();
        return client_rect(false);
    }

    void text_edit::set_page_rect(optional_rect const& aPageRect)
    {
        iPageRect = aPageRect;
        refresh_columns();
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

    neogfx::tab_stops const& text_edit::tab_stops() const
    {
        if (iTabStops != std::nullopt)
            return iTabStops.value();
        if (iCalculatedTabStops == std::nullopt || iCalculatedTabStops->first != font())
            iCalculatedTabStops = std::make_pair(font(), tab_stop{ graphics_context{ *this, graphics_context::type::Unattached }.text_extent(iTabStopHint, font()).cx });
        return iCalculatedTabStops->second;
    }

    void text_edit::set_tab_stop_hint(i_string const& aTabStopHint)
    {
        if (iTabStopHint != aTabStopHint)
        {
            iTabStopHint = aTabStopHint;
            iCalculatedTabStops.reset();
            refresh_columns();
        }
    }

    void text_edit::set_tab_stops(std::optional<neogfx::tab_stops> const& aTabStops)
    {
        if (iTabStops != aTabStops)
        {
            iTabStops = aTabStops;
            iCalculatedTabStops.reset();
            refresh_columns();
        }
    }

    void text_edit::init()
    {
        set_frame_radius(vec4{ 4.0, 4.0, 4.0, 4.0 });

        iSink += neolib::service<neolib::i_power>().green_mode_entered([this]()
        {
            if (has_focus())
                update_cursor();
        });

        iDefaultFont = service<i_app>().current_style().font_info();
        iSink += service<i_app>().current_style_changed([this](style_aspect aAspect)
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

        iSink += Mouse([this](const neogfx::mouse_event& aEvent)
            {
                auto const docPos = document_hit_test_ex(aEvent.position() - origin());
                if (docPos.first < static_cast<position_type>(iText.size()) && docPos.second)
                {
                    if (iText[docPos.first].tag != neolib::invalid_cookie<tag_cookie>)
                    {
                        auto tagPtr = iTagMap[iText[docPos.first].tag];
                        auto const& tag = *tagPtr;
                        if (!tag.mouse_event().has_slots())
                            return;
                        bool const wasCapturingTag = (tagPtr == iTagCapturing);
                        if (aEvent.type() != mouse_event_type::Moved)
                            iTagCapturing = nullptr;
                        if (aEvent.type() == mouse_event_type::ButtonReleased)
                        {
                            if (!wasCapturingTag || cursor().position() != cursor().anchor())
                                return;
                            Mouse.accept();
                            if (capturing())
                                release_capture();
                            iDragger = nullptr;
                        }
                        auto const result = tag.mouse_event().trigger(aEvent);
                        if (result == trigger_result::Accepted)
                            Mouse.accept();
                        else if (aEvent.type() == mouse_event_type::ButtonClicked)
                            iTagCapturing = tagPtr;
                    }
                }
            });
        
        iSink += cursor().PositionChanged([this]()
        {
            if (neolib::service<i_keyboard>().layout().ime_active(*this))
                neolib::service<i_keyboard>().layout().update_ime_position(cursor_rect().bottom_left());
            iNextStyle = std::nullopt;
            iCursorAnimationStartTime = neolib::this_process::elapsed_ms();
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

        iSink += Password.Changed([&](bool const& aPassword)
        {
            if (aPassword && (iCaps & text_edit_caps::ShowPassword) == text_edit_caps::ShowPassword)
                iPasswordBits.emplace(*this);
            else
                iPasswordBits = std::nullopt;
            layout_items(false);
        });

        if ((iCaps & text_edit_caps::Password) == text_edit_caps::Password)
            set_password(true);
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

    std::size_t text_edit::do_insert_text(position_type aPosition, i_string const& aText, format const& aFormat, bool aMoveCursor, bool aClearFirst)
    {
        bool accept = true;
        TextFilter(aText, accept);
        if (!accept)
            return 0;

        iPreviousText = iText;
        iUtf8TextCache = std::nullopt;

        if (aClearFirst)
            iText.clear();

        auto insertionPoint = iText.begin() + aPosition;
        std::size_t insertionSize = 0;

        thread_local std::vector<std::ptrdiff_t> indexMap;
        indexMap.clear();
        indexMap.resize(aText.size(), -1);
        auto readPos = 0;

        auto insert = [&](document_text::const_iterator aWhere, const style& aStyle, std::u32string::const_iterator begin, std::u32string::const_iterator end)
        {
            auto existingStyle = iStyles.end();
            if (&aStyle != &iDefaultStyle || iPersistDefaultStyle)
            {
                style key = aStyle;
                existingStyle = iStyles.find(style_ptr{ style_ptr{}, &key });
                if (existingStyle == iStyles.end())
                    existingStyle = iStyles.insert(std::make_shared<style>(*this, aStyle)).first;
            }
            auto& style = (existingStyle != iStyles.end() ? **existingStyle : iDefaultStyle);
            if (existingStyle != iStyles.end())
            {
                if (!style.has_cookie())
                    style.set_cookie(iStyleMap.insert(*existingStyle));
                for (auto i = std::distance(begin, end); i > 0; --i)
                    style.add_ref();
            }

            auto const pos = std::distance(iText.cbegin(), aWhere);
            auto next = std::next(iText.begin(), pos);
            char32_t previousChar = (aWhere != iText.begin() ? std::prev(aWhere)->character : 0);
            for (auto const ch : std::ranges::subrange(begin, end))
            {
                bool discard = false;
                if (!iHandlingKeyPress)
                {
                    switch (ch)
                    {
                    case '\n':
                        if (previousChar == '\r')
                        {
                            std::prev(next)->character = '\n';
                            if (is_automatic(iLineEnding))
                                iLineEnding = text_edit_line_ending::AutomaticLfCr;
                            discard = true;
                        }
                        else
                        {
                            if (is_automatic(iLineEnding))
                                iLineEnding = text_edit_line_ending::AutomaticLf;
                        }
                        break;
                    case '\r':
                        if (previousChar == '\n')
                        {
                            if (is_automatic(iLineEnding))
                                iLineEnding = text_edit_line_ending::AutomaticCrLf;
                            discard = true;
                        }
                        break;
                    default:
                        /* do nothing */
                        break;
                    }
                }
                if (!discard)
                {
                    indexMap[readPos] = insertionSize;
                    next = std::next(iText.insert(next, document_char{ ch, style.cookie() }));
                    ++insertionSize;
                }
                ++readPos;
                previousChar = ch;
            }
            return std::next(iText.begin(), pos);
        };

        if (std::holds_alternative<std::monostate>(aFormat.style) || std::holds_alternative<style>(aFormat.style))
        {
            thread_local std::u32string text;
            text = neolib::utf8_to_utf32(aText);
            if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::SingleLine)
            {
                auto eol = text.find_first_of(U"\r\n");
                if (eol != std::u32string::npos)
                    text.erase(eol);
            }
            auto const& formatStyle = std::holds_alternative<std::monostate>(aFormat.style) ? default_style() : std::get<style>(aFormat.style);
            insert(insertionPoint, formatStyle, text.begin(), text.end());
        }
        else if (std::holds_alternative<style_callback>(aFormat.style))
        {
            std::ptrdiff_t next = 0;
            bool gotSingleLine = false;
            while (next != static_cast<std::ptrdiff_t>(aText.size()) && !gotSingleLine)
            {
                auto const& [s, nextEnd] = std::get<style_callback>(aFormat.style)(next);
                thread_local std::u32string text;
                text = neolib::utf8_to_utf32(std::string_view{ &aText[next], std::next(&aText[next], nextEnd - next) });
                if ((iCaps & text_edit_caps::LINES_MASK) == text_edit_caps::SingleLine)
                {
                    auto eol = text.find_first_of(U"\r\n");
                    if (eol != std::u32string::npos)
                    {
                        text.erase(eol);
                        gotSingleLine = true;
                    }
                }
                insertionPoint = std::next(insert(insertionPoint, s, text.begin(), text.end()), text.size());
                next = nextEnd;
            }
        }
        else if (std::holds_alternative<ansi>(aFormat.style))
        {
            throw not_implemented();
        }

        insertionPoint = iText.begin() + aPosition;

        if (std::holds_alternative<tag_ptr>(aFormat.tag))
        {
            auto const& tag = *std::get<tag_ptr>(aFormat.tag);
            for (auto& ch : std::ranges::subrange(insertionPoint, insertionPoint + insertionSize))
                ch.tag = tag.cookie();
        }
        else if (std::holds_alternative<tag_ptr_callback>(aFormat.tag))
        {
            std::ptrdiff_t next = 0;
            while (next != static_cast<std::ptrdiff_t>(aText.size()))
            {
                auto const& [t, nextEnd] = std::get<tag_ptr_callback>(aFormat.tag)(next);
                if (t)
                {
                    auto const& tag = *t;
                    for (auto index = next; index != nextEnd; ++index)
                        if (indexMap[index] != -1)
                            std::next(insertionPoint, indexMap[index])->tag = tag.cookie();
                }
                next = nextEnd;
            }
        }

        if (!aClearFirst)
            refresh_paragraph(insertionPoint, insertionSize);
        else
            refresh_paragraph(iText.begin(), 0);

        update();
        
        if (aMoveCursor)
        {
            cursor().set_position(insertionPoint - iText.begin() + insertionSize);
            iCursorHint.x = glyph_position(cursor_glyph_position(), true).pos.x;
        }
        
        if (iPreviousText != iText)
            notify_text_changed();
        
        return insertionSize;
    }

    void text_edit::delete_any_selection()
    {
        if (cursor().position() != cursor().anchor())
        {
            auto const from = std::min(cursor().position(), cursor().anchor());
            auto const to = std::max(cursor().position(), cursor().anchor());
            cursor().set_position(std::min(cursor().position(), cursor().anchor()));
            delete_text(from, to);
        }
    }

    void text_edit::notify_text_changed()
    {
        if (!iSuppressTextChangedNotification)
            TextChanged();
        else
            ++iWantedToNotifyTextChanged;
    }

    text_edit::document_glyphs::const_iterator text_edit::to_glyph(document_text::const_iterator aWhere) const
    {
        auto const textIndex = aWhere - iText.begin();
        auto const paragraphLineSpan = character_to_line(textIndex);
        auto const documentSpan = paragraphLineSpan.lineSpan + paragraphLineSpan.paragraphSpan;
        auto const lineStart = std::next(glyphs().begin(), documentSpan.glyphsFirst);
        auto const lineEnd = std::next(glyphs().begin(), documentSpan.glyphsLast);
        auto const paragraphTextIndex = textIndex - paragraphLineSpan.paragraphSpan.textFirst;
        for (auto g = lineStart; g != lineEnd; ++g)
            if (g->clusters.first <= paragraphTextIndex && g->clusters.second > paragraphTextIndex)
                return g;
        return lineEnd;
    }

    std::pair<text_edit::document_text::size_type, text_edit::document_text::size_type> text_edit::from_glyph(document_glyphs::const_iterator aWhere) const
    {
        if (aWhere == glyphs().end())
            return std::make_pair(iText.size(), iText.size());
        auto paragraph = glyph_to_paragraph(aWhere - glyphs().begin());
        if (paragraph == iGlyphParagraphs.end() && paragraph != iGlyphParagraphs.begin() && aWhere <= (paragraph - 1)->glyph_end())
            --paragraph;
        if (paragraph != iGlyphParagraphs.end())
        {
            if (paragraph->glyph_begin() > aWhere)
                --paragraph;
            auto const textStart = paragraph->span.textFirst;
            auto const& clusters = aWhere->clusters;
            return std::make_pair(textStart + clusters.first, textStart + clusters.second);
        }
        return std::make_pair(iText.size(), iText.size());
    }

    std::optional<text_edit::glyph_lines::const_iterator> text_edit::next_line(std::optional<glyph_lines::const_iterator> const& aFrom) const
    {
        if (!aFrom)
            return {};
        if (aFrom.value() < std::prev(aFrom.value()->column().lines.end()))
            return std::next(aFrom.value());
        return {};
    }

    std::optional<text_edit::glyph_lines::const_iterator> text_edit::previous_line(std::optional<glyph_lines::const_iterator> const& aFrom) const
    {
        if (!aFrom)
            return {};
        if (aFrom.value() > aFrom.value()->column().lines.begin())
            return std::prev(aFrom.value());
        return {};
    }

    void text_edit::refresh_paragraph(document_text::const_iterator aWhere, ptrdiff_t aDelta)
    {
        if (iUpdatingDocument)
            return;

        document_text::const_iterator first;
        document_text::const_iterator last;
        document_glyphs::const_iterator glyphsInsertPos;
        glyph_paragraphs::const_iterator glyphParagraphsInsertPos;

        std::ptrdiff_t charsInserted = 0;
        std::ptrdiff_t glyphsInserted = 0;

        if (aDelta == 0 || iGlyphParagraphs.empty())
        {
            (void)aWhere;
            glyphs().clear();
            iGlyphParagraphs.clear();
            first = iText.begin();
            last = iText.end();
            glyphsInsertPos = glyphs().end();
            glyphParagraphsInsertPos = iGlyphParagraphs.end();
        }
        else if (aDelta > 0)
        {   
            auto const fromParagraph = character_to_paragraph(std::distance(iText.cbegin(), aWhere));
            first = std::next(iText.begin(), fromParagraph.paragraphSpan.textFirst);
            last = std::next(iText.begin(), fromParagraph.paragraphSpan.textLast + aDelta);
            glyphsInsertPos = glyphs().erase(
                std::next(glyphs().begin(), fromParagraph.paragraphSpan.glyphsFirst), 
                std::next(glyphs().begin(), fromParagraph.paragraphSpan.glyphsLast));
            glyphParagraphsInsertPos = iGlyphParagraphs.erase(std::next(iGlyphParagraphs.begin(), fromParagraph.paragraphIndex));
            charsInserted -= (fromParagraph.paragraphSpan.textLast - fromParagraph.paragraphSpan.textFirst);
            glyphsInserted -= (fromParagraph.paragraphSpan.glyphsLast - fromParagraph.paragraphSpan.glyphsFirst);
        }
        else // aDelta < 0
        {
            auto const fromParagraph = character_to_paragraph(std::distance(iText.cbegin(), aWhere));
            auto const toParagraph = character_to_paragraph(std::distance(iText.cbegin(), aWhere) + -aDelta);
            first = std::next(iText.begin(), fromParagraph.paragraphSpan.textFirst);
            last = std::next(iText.begin(), toParagraph.paragraphSpan.textLast + aDelta);
            glyphsInsertPos = glyphs().erase(
                std::next(glyphs().begin(), fromParagraph.paragraphSpan.glyphsFirst),
                std::next(glyphs().begin(), toParagraph.paragraphSpan.glyphsLast));
            glyphParagraphsInsertPos = iGlyphParagraphs.erase(
                std::next(iGlyphParagraphs.begin(), fromParagraph.paragraphIndex),
                std::next(iGlyphParagraphs.begin(), toParagraph.paragraphIndex + 1));
            charsInserted -= (toParagraph.paragraphSpan.textLast - fromParagraph.paragraphSpan.textFirst);
            glyphsInserted -= (toParagraph.paragraphSpan.glyphsLast - fromParagraph.paragraphSpan.glyphsFirst);
        }

        graphics_context gc{ *this, graphics_context::type::Unattached };

        if (password() && (!iPasswordBits || !iPasswordBits.value().showPassword.is_pressed()))
            gc.set_password(true, PasswordMask.value().empty() ? "\xE2\x97\x8F"_s : PasswordMask);

        auto nextParagraph = first;
        thread_local std::vector<std::u32string::difference_type> cachedColumnDelimiters;
        auto& columnDelimiters = cachedColumnDelimiters;
 
        auto fs = [this, &nextParagraph, &columnDelimiters](std::u32string::size_type aSourceIndex)
        {
            auto characterStyle = iStyleMap.find(std::next(nextParagraph, aSourceIndex)->style);
            std::size_t indexColumn = std::lower_bound(columnDelimiters.begin(), columnDelimiters.end(), 
                static_cast<std::u32string::difference_type>(aSourceIndex)) - columnDelimiters.begin();
            if (indexColumn > columns() - 1)
                indexColumn = columns() - 1;
            auto const& columnStyle = column_style(indexColumn);
            auto const& style =
                characterStyle != iStyleMap.end() ? **characterStyle :
                columnStyle.character().font() != std::nullopt ? columnStyle : iDefaultStyle;
            return style.character().font() != std::nullopt ? style.character().font().value() : font();
        };
        
        columnDelimiters.clear();
        std::size_t columnCount = 0;

        for (auto iterChar = first; iterChar != last; ++iterChar)
        {
            auto ch = iterChar->character;

            auto& column = iColumns[std::min(columnDelimiters.size(), iColumns.size() - 1)];

            if (ch == column.info.delimiter && columnDelimiters.size() + 1 < iColumns.size())
                columnDelimiters.push_back(std::distance(nextParagraph, iterChar));
            
            bool newParagraph = (ch == U'\n');
            
            if (newParagraph || iterChar == std::prev(last))
            {
                thread_local std::u32string paragraphBuffer;
                paragraphBuffer.assign(nextParagraph, std::next(iterChar));
                scoped_tab_stops sts{ gc, tab_stops() };
                auto gt = service<i_font_manager>().glyph_text_factory().to_glyph_text(gc, std::u32string_view{ paragraphBuffer.begin(), paragraphBuffer.end() }, fs, false);
                if (gt.cbegin() != gt.cend())
                {
                    auto const paragraphGlyphs = glyphs().insert(glyphsInsertPos, gt.cbegin(), gt.cend());
                    glyphsInsertPos = std::next(paragraphGlyphs, gt.size());
                    for (auto& newGlyph : gt)
                        glyphs().glyph_font(newGlyph);
                    document_span const span{
                        std::distance(iText.cbegin(), nextParagraph),
                        std::distance(iText.cbegin(), std::next(iterChar)),
                        std::distance(glyphs().begin(), paragraphGlyphs),
                        std::distance(glyphs().begin(), std::next(paragraphGlyphs, gt.size())) };
                    auto const paragraph = iGlyphParagraphs.emplace(glyphParagraphsInsertPos, this, span);
                    paragraph->columnBreaks.assign(columnDelimiters.begin(), columnDelimiters.end());
                    paragraph->lineBreaks.assign(gt.content().line_breaks().begin(), gt.content().line_breaks().end());
                    glyphParagraphsInsertPos = std::next(paragraph);
                    charsInserted += (paragraph->span.textLast - paragraph->span.textFirst);
                    glyphsInserted += (paragraph->span.glyphsLast - paragraph->span.glyphsFirst);
                }
                nextParagraph = std::next(iterChar);
                columnCount = std::max(columnCount, columnDelimiters.size() + 1);
                columnDelimiters.clear();
            }
        }

        for (auto paragraphToAdjust = std::next(iGlyphParagraphs.begin(), std::distance(iGlyphParagraphs.cbegin(), glyphParagraphsInsertPos)); 
            paragraphToAdjust != iGlyphParagraphs.end(); ++paragraphToAdjust)
        {
            auto& p = *paragraphToAdjust;
            p.span.textFirst += charsInserted;
            p.span.textLast += charsInserted;
            p.span.glyphsFirst += glyphsInserted;
            p.span.glyphsLast += glyphsInserted;
            p.heightMap.clear();
            p.ypos = 0.0;
        }

        iGlyphColumns.resize(std::max(columnCount, iGlyphColumns.size()), { this });
        for (auto& column : iGlyphColumns)
            column.lines.clear();

        if (iPasswordBits)
            iPasswordBits.value().showPassword.show(!iText.empty());

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
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << "text_edit::refresh_lines()" << std::endl;
#endif // NEOGFX_DEBUG

        try
        {
            iOutOfMemory = false;

            /* simple (naive) implementation just to get things moving... */
            for (auto& column : iGlyphColumns)
                column.lines.clear();
            
            dimension availableWidth = column_rect(0).width(); // todo: columns
            dimension availableHeight = column_rect(0).height();
            bool showVerticalScrollbar = false;
            bool showHorizontalScrollbar = false;

            iTextExtents = size{};
            
            std::uint32_t pass = 1;
            dimension yposParagraph = 0.0;

            for (auto iterParagraph = iGlyphParagraphs.begin(); iterParagraph != iGlyphParagraphs.end();)
            {
                auto& paragraph = *iterParagraph;

                paragraph.ypos = yposParagraph;
                dimension yColumn = 0.0;

                for (auto& column : iGlyphColumns)
                {
                    dimension yLine = 0.0;

                    auto const columnIndex = std::distance(&iGlyphColumns[0], &column);
                    auto& lines = column.lines;

                    thread_local std::vector<std::pair<document_glyphs::difference_type, document_glyphs::difference_type>> paragraphLines;
                    paragraphLines.clear();

                    // todo: line segments to correct column

                    glyph_text::size_type lastBreak = 0;
                    for (auto lineBreak : paragraph.lineBreaks)
                    {
                        paragraphLines.emplace_back(lastBreak + paragraph.span.glyphsFirst, lineBreak + paragraph.span.glyphsFirst);
                        lastBreak = lineBreak + 1;
                    }
                    paragraphLines.emplace_back(lastBreak + paragraph.span.glyphsFirst, paragraph.span.glyphsLast);
                    if (paragraphLines.back().first != paragraphLines.back().second &&
                        is_line_breaking_whitespace(glyphs().back()) && std::next(iterParagraph) == iGlyphParagraphs.end())
                        paragraphLines.emplace_back(paragraph.span.glyphsLast, paragraph.span.glyphsLast);

                    auto const& paragraphStyle = glyph_style(paragraph.glyph_begin(), iColumns[columnIndex]);

                    if (paragraphStyle.paragraph().padding())
                        yLine += paragraphStyle.paragraph().padding().value().top;

                    bool first = true;

                    for (auto const& paragraphLine : paragraphLines)
                    {
                        auto const paragraphLineStart = std::next(glyphs().begin(), paragraphLine.first);
                        auto const paragraphLineEnd = std::next(glyphs().begin(), paragraphLine.second);

                        if (!first)
                        {
                            if (paragraphStyle.paragraph().line_spacing())
                                yLine += paragraphStyle.paragraph().line_spacing().value();
                        }
                        else
                            first = false;

                        if (paragraphLineStart == paragraphLineEnd || is_line_breaking_whitespace(*paragraphLineStart))
                        {
                            auto lineStart = paragraphLineStart;
                            auto lineEnd = (paragraphLineStart == paragraphLineEnd || !is_line_breaking_whitespace(*paragraphLineStart)) ? 
                                paragraphLineEnd : paragraphLineStart;

                            auto const alignBaselinesResult = glyphs().align_baselines(lineStart, lineEnd, true);

                            glyph_char::cluster_range clusters{
                                static_cast<glyph_char::cluster_index>(from_glyph(lineStart).first),
                                static_cast<glyph_char::cluster_index>(from_glyph(lineEnd).second) };
                            if (alignBaselinesResult.clusters)
                            {
                                clusters.first = std::min(clusters.first, alignBaselinesResult.clusters.value().first + static_cast<glyph_char::cluster_index>(paragraph.span.textFirst));
                                clusters.second = std::max(clusters.second, alignBaselinesResult.clusters.value().second + static_cast<glyph_char::cluster_index>(paragraph.span.textFirst));
                            }
                            auto textLineStart = static_cast<position_type>(clusters.first) - paragraph.span.textFirst;
                            auto textLineEnd = static_cast<position_type>(clusters.second) - paragraph.span.textFirst;
                            if (lineStart != lineEnd && std::prev(lineEnd)->clusters.first < lineStart->clusters.first) // RTL
                                textLineStart = std::prev(lineEnd)->clusters.first;

                            document_span const span{
                                textLineStart,
                                textLineEnd,
                                lineStart - glyphs().begin() - paragraph.span.glyphsFirst,
                                lineEnd - glyphs().begin() - paragraph.span.glyphsFirst };

                            size const lineExtents{
                                    lineEnd != lineStart ? (lineEnd - 1)->cell[1].x - (lineStart)->cell[0].x : 0.0f,
                                    alignBaselinesResult.yExtent };

                            dimension xLine = 0.0;

                            auto textDirection = glyph_text_direction(lineStart, lineEnd);
                            auto const paragraphAlignment = paragraphStyle.paragraph().alignment().as_std_optional().value_or(neogfx::alignment::Left | neogfx::alignment::Top);

                            if (((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Left && textDirection == text_direction::RTL) ||
                                ((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Right && textDirection == text_direction::LTR))
                                xLine += (availableWidth - lineExtents.cx);
                            else if ((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Center)
                                xLine += std::ceil((availableWidth - lineExtents.cx, 0.0) / 2.0);

                            lines.emplace_back(
                                this,
                                std::distance(iGlyphParagraphs.begin(), iterParagraph),
                                column.index(),
                                span,
                                point{ xLine, yLine },
                                lineExtents,
                                alignBaselinesResult.majorFont,
                                alignBaselinesResult.baseline);

                            yLine += lines.back().extents.cy;
                            iTextExtents->cx = std::max(iTextExtents->cx, lines.back().extents.cx);
                        }
                        else if (WordWrap && static_cast<coordinate>((paragraphLineEnd - 1)->cell[0].x) + static_cast<coordinate>((paragraphLineEnd - 1)->cell_extents().x) > availableWidth)
                        {
                            auto add_line = [&](auto first, auto last)
                            {
                                if (last != first && is_line_breaking_whitespace(*(last - 1)))
                                    --last;

                                auto const alignBaselinesResult = glyphs().align_baselines(first, last, true);

                                glyph_char::cluster_range clusters{
                                    static_cast<glyph_char::cluster_index>(from_glyph(first).first),
                                    static_cast<glyph_char::cluster_index>(from_glyph(last).second) };
                                if (alignBaselinesResult.clusters)
                                {
                                    clusters.first = std::min(clusters.first, alignBaselinesResult.clusters.value().first + static_cast<glyph_char::cluster_index>(paragraph.span.textFirst));
                                    clusters.second = std::max(clusters.second, alignBaselinesResult.clusters.value().second + static_cast<glyph_char::cluster_index>(paragraph.span.textFirst));
                                }
                                auto textLineStart = static_cast<position_type>(clusters.first) - paragraph.span.textFirst;
                                auto textLineEnd = static_cast<position_type>(clusters.second) - paragraph.span.textFirst;
                                if (first != last && std::prev(last)->clusters.first < first->clusters.first) // RTL
                                    textLineStart = std::prev(last)->clusters.first;

                                document_span const span{
                                    textLineStart,
                                    textLineEnd,
                                    first - glyphs().begin() - paragraph.span.glyphsFirst,
                                    last - glyphs().begin() - paragraph.span.glyphsFirst };

                                size const lineExtents{
                                    last != first ? (last - 1)->cell[1].x - (first)->cell[0].x : 0.0f,
                                    alignBaselinesResult.yExtent };

                                dimension xLine = 0.0;

                                auto textDirection = glyph_text_direction(first, last);
                                auto const paragraphAlignment = paragraphStyle.paragraph().alignment().as_std_optional().value_or(neogfx::alignment::Left | neogfx::alignment::Top);

                                if (((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Left && textDirection == text_direction::RTL) ||
                                    ((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Right && textDirection == text_direction::LTR))
                                    xLine += (availableWidth - lineExtents.cx);
                                else if ((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Center)
                                    xLine += std::ceil((availableWidth - lineExtents.cx, 0.0) / 2.0);

                                lines.emplace_back(
                                    this,
                                    std::distance(iGlyphParagraphs.begin(), iterParagraph),
                                    column.index(),
                                    span,
                                    point{ xLine, yLine },
                                    lineExtents,
                                    alignBaselinesResult.majorFont,
                                    alignBaselinesResult.baseline);

                                yLine += lines.back().extents.cy;
                                iTextExtents->cx = std::max(iTextExtents->cx, lines.back().extents.cx);
                            };

                            if (glyph_text_direction(paragraphLineStart, paragraphLineEnd) == text_direction::LTR)
                            {
                                auto next = paragraphLineStart;
                                auto lineStart = next;
                                auto lineEnd = paragraphLineEnd;
                                coordinate offset = (lineEnd != lineStart ? lineStart->cell[0].x : 0.0);
                                while (next != paragraphLineEnd)
                                {
                                    glyph_char const key{ {}, {}, {}, {}, {}, quadf_2d{ vec2{ offset + availableWidth, 0.0f }, vec2{ offset + availableWidth, 0.0f } }, {} };
                                    auto split = std::lower_bound(next, paragraphLineEnd, key, [](auto const& lhs, auto const& rhs) { return lhs.cell[0].x < rhs.cell[0].x; });
                                    if (split != next)
                                    {
                                        if (split != paragraphLineEnd)
                                            --split;
                                        else
                                        {
                                            auto const& previousChar = *(split - 1);
                                            auto const xPrevious = static_cast<coordinate>(previousChar.cell[0].x);
                                            auto const cxPrevious = static_cast<coordinate>(previousChar.cell_extents().x);
                                            if (xPrevious + cxPrevious >= offset + availableWidth)
                                                --split;
                                        }
                                    }
                                    if (split == next)
                                        ++split;
                                    if (split != paragraphLineEnd)
                                    {
                                        auto wordBreak = word_break(lineStart, split, paragraphLineEnd);
                                        if (wordBreak.first != lineStart)
                                        {
                                            lineEnd = wordBreak.first;
                                            next = wordBreak.second;
                                        }
                                        else
                                            next = lineEnd = split;
                                    }
                                    else
                                        next = paragraphLineEnd;
                                    add_line(lineStart, lineEnd);
                                    lineStart = next;
                                    if (lineStart != paragraphLineEnd)
                                        offset = lineStart->cell[0].x;
                                    lineEnd = paragraphLineEnd;
                                }
                            }
                            else // RTL
                            {
                                auto next = std::reverse_iterator{ paragraphLineEnd };
                                auto lineStart = next;
                                auto lineEnd = std::reverse_iterator{ paragraphLineStart };
                                coordinate const rightmost = (lineEnd != lineStart ? lineStart->cell[1].x : 0.0);
                                coordinate offset = rightmost;
                                while (next != std::reverse_iterator{ paragraphLineStart })
                                {
                                    glyph_char const key{ {}, {}, {}, {}, {}, quadf_2d{ vec2{ offset - availableWidth, 0.0f } }, {} };
                                    auto split = std::lower_bound(next, std::reverse_iterator{ paragraphLineStart }, key, [=](auto const& lhs, auto const& rhs) { return offset - lhs.cell[0].x < offset - rhs.cell[0].x; });
                                    if (split != next && (split != std::reverse_iterator{ paragraphLineStart } || static_cast<coordinate>((split - 1)->cell[0].x) + static_cast<coordinate>((split - 1)->cell_extents().x) >= rightmost - offset + availableWidth))
                                        --split;
                                    if (split == next)
                                        ++split;
                                    if (split != std::reverse_iterator{ paragraphLineStart })
                                    {
                                        auto wordBreak = word_break(lineStart, split, std::reverse_iterator{ paragraphLineStart });
                                        if (wordBreak.first != lineStart)
                                        {
                                            lineEnd = wordBreak.first;
                                            next = wordBreak.second;
                                        }
                                        else
                                            next = lineEnd = split;
                                    }
                                    else
                                        next = std::reverse_iterator{ paragraphLineStart };
                                    add_line(lineEnd.base(), lineStart.base());
                                    lineStart = next;
                                    if (lineStart != std::reverse_iterator{ paragraphLineStart })
                                        offset = lineStart->cell[1].x;
                                    lineEnd = std::reverse_iterator{ paragraphLineStart };
                                }
                            }
                        }
                        else
                        {
                            auto lineStart = paragraphLineStart;
                            auto lineEnd = paragraphLineEnd;
                            if (lineEnd != lineStart && is_line_breaking_whitespace(*(lineEnd - 1)))
                                --lineEnd;

                            auto const alignBaselinesResult = glyphs().align_baselines(lineStart, lineEnd, true);

                            glyph_char::cluster_range clusters{
                                static_cast<glyph_char::cluster_index>(from_glyph(lineStart).first),
                                static_cast<glyph_char::cluster_index>(from_glyph(lineEnd).second) };
                            if (alignBaselinesResult.clusters)
                            {
                                clusters.first = std::min(clusters.first, alignBaselinesResult.clusters.value().first + static_cast<glyph_char::cluster_index>(paragraph.span.textFirst));
                                clusters.second = std::max(clusters.second, alignBaselinesResult.clusters.value().second + static_cast<glyph_char::cluster_index>(paragraph.span.textFirst));
                            }
                            auto textLineStart = static_cast<position_type>(clusters.first) - paragraph.span.textFirst;
                            auto textLineEnd = static_cast<position_type>(clusters.second) - paragraph.span.textFirst;
                            if (lineEnd != lineStart && std::prev(lineEnd)->clusters.first < lineStart->clusters.first) // RTL
                                textLineStart = std::prev(lineEnd)->clusters.first;
                            
                            document_span const span{
                                textLineStart,
                                textLineEnd,
                                lineStart - glyphs().begin() - paragraph.span.glyphsFirst,
                                lineEnd - glyphs().begin() - paragraph.span.glyphsFirst };

                            size const lineExtents{
                                    lineEnd != lineStart ? (lineEnd - 1)->cell[1].x - (lineStart)->cell[0].x : 0.0f,
                                    alignBaselinesResult.yExtent };

                            dimension xLine = 0.0;

                            auto textDirection = glyph_text_direction(lineStart, lineEnd);
                            auto const paragraphAlignment = paragraphStyle.paragraph().alignment().as_std_optional().value_or(neogfx::alignment::Left | neogfx::alignment::Top);

                            if (((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Left && textDirection == text_direction::RTL) ||
                                ((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Right && textDirection == text_direction::LTR))
                                xLine += (availableWidth - lineExtents.cx);
                            else if ((paragraphAlignment & neogfx::alignment::Horizontal) == neogfx::alignment::Center)
                                xLine += std::ceil((availableWidth - lineExtents.cx, 0.0) / 2.0);

                            lines.emplace_back(
                                this,
                                std::distance(iGlyphParagraphs.begin(), iterParagraph),
                                column.index(),
                                span,
                                point{ xLine, yLine },
                                lineExtents,
                                alignBaselinesResult.majorFont,
                                alignBaselinesResult.baseline);
                            
                            yLine += lines.back().extents.cy;
                            iTextExtents->cx = std::max(iTextExtents->cx, lines.back().extents.cx);
                        }
                    }

                    if (paragraphStyle.paragraph().padding())
                        yLine += paragraphStyle.paragraph().padding().value().bottom;

                    yColumn = std::max(yColumn, yLine);
                }

                yposParagraph += yColumn;
                iTextExtents->cy = std::max(iTextExtents->cy, yposParagraph);

                auto next_pass = [&]()
                    {
                        if (pass <= 3)
                        {
                            for (auto& c : iGlyphColumns)
                                c.lines.clear();
                            yposParagraph = 0.0;
                            iTextExtents = size{};
                            iterParagraph = iGlyphParagraphs.begin();
                            ++pass;
                        }
                    };
                switch (pass)
                {
                case 1:
                case 3:
                    if (!vertical_scrollbar().visible() && !showVerticalScrollbar && iTextExtents->cy > availableHeight)
                    {
                        showVerticalScrollbar = true;
                        availableWidth -= vertical_scrollbar().width();
                        next_pass();
                    }
                    else if (++iterParagraph == iGlyphParagraphs.end() && pass == 1)
                        next_pass();
                    break;
                case 2:
                    if (!horizontal_scrollbar().visible() && !showHorizontalScrollbar && iTextExtents->cx > availableWidth)
                    {
                        showHorizontalScrollbar = true;
                        availableHeight -= horizontal_scrollbar().width();
                        next_pass();
                    }
                    else if (++iterParagraph == iGlyphParagraphs.end())
                        next_pass();
                    break;
                case 4:
                    ++iterParagraph;
                    break;
                }
            }

            if (iTextExtents->cy < client_rect(false).cy)
            {
                auto const space = client_rect(false).cy - iTextExtents->cy;
                auto const defaultAlignment = default_style().paragraph().alignment().as_std_optional().value_or(neogfx::alignment::Left | neogfx::alignment::Top);
                auto const adjust =
                    ((defaultAlignment & neogfx::alignment::Vertical) == neogfx::alignment::Bottom) ? space :
                    ((defaultAlignment & neogfx::alignment::Vertical) == neogfx::alignment::VCenter) ? std::floor(space / 2.0) : 0.0;
                if (adjust != 0.0)
                    for (auto& paragraph : iGlyphParagraphs)
                        paragraph.ypos += adjust;
            }
        }
        catch (std::bad_alloc)
        {
            for (auto& paragraph : iGlyphParagraphs)
            {
                paragraph.ypos = 0.0;
                for (auto& column : iGlyphColumns)
                    column.lines.clear();
            }
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
        auto const cgp = cursor_glyph_position();
        auto const gp = glyph_position(cgp, true);
        make_visible(gp, aForcePreviewScroll ? 
            point{ std::ceil(std::min(client_rect(false).width() / 3.0, 200.0)), 0.0 } : 
            point{});
    }

    void text_edit::make_visible(position_info const& aGlyphPosition, point const& aPreview)
    {
        scoped_units su{ *this, units::Pixels };
        auto extents = (aGlyphPosition.paragraph != iGlyphParagraphs.end() && 
            aGlyphPosition.column != iGlyphColumns.end() && 
            aGlyphPosition.line != aGlyphPosition.column.value()->lines.end() ?
                size{ aGlyphPosition.glyph != aGlyphPosition.lineEnd.value() ? aGlyphPosition.glyph.value()->cell_extents().x : 0.0, aGlyphPosition.line.value()->extents.cy } :
                size{ 0.0, font().height() });
        auto position = aGlyphPosition.pos;
        auto const& internalPadding = padding();
        auto const& colulmnPadding = column(0).padding; ///< todo: other columns?
        auto const& totalPadding = internalPadding + colulmnPadding;
        extents.cy = std::min(extents.cy, vertical_scrollbar().page());
        if (position.y < vertical_scrollbar().position())
            vertical_scrollbar().set_position(position.y);
        else if (position.y + extents.cy + totalPadding.size().cy > vertical_scrollbar().position() + vertical_scrollbar().page())
            vertical_scrollbar().set_position(position.y + extents.cy - vertical_scrollbar().page() + totalPadding.size().cy );
        if (position.x < horizontal_scrollbar().position() + aPreview.x)
            horizontal_scrollbar().set_position(position.x - aPreview.x);
        else if (position.x + extents.cx + totalPadding.size().cx > horizontal_scrollbar().position() + horizontal_scrollbar().page() - aPreview.x)
            horizontal_scrollbar().set_position(position.x + extents.cx + aPreview.x - horizontal_scrollbar().page() + totalPadding.size().cx);
    }

    text_edit::style text_edit::glyph_style(document_glyphs::const_iterator aGlyph, const document_column& aColumn) const
    {
        style result = iDefaultStyle;
        result.merge(column_style(aColumn.info));
        result.character().set_paper_color();
        if (iText.empty())
            return result;
        auto characterIndex = from_glyph(aGlyph).first;
        if (characterIndex == iText.size())
            --characterIndex;
        auto const existingCharacterStyleCookie = iText[characterIndex].style;
        auto const existingCharacterStyle = iStyleMap.find(existingCharacterStyleCookie);
        if (existingCharacterStyle != iStyleMap.end())
            result.merge(**existingCharacterStyle);
        return result;
    }

    void text_edit::draw_glyphs(i_graphics_context const& aGc, const point& aPosition, const glyph_column& aColumn, glyph_lines::const_iterator aLine) const
    {
        auto lineStart = aLine->glyph_begin();
        auto lineEnd = aLine->glyph_end();

        if (lineStart == lineEnd)
            return;

        if (is_line_breaking_whitespace(*(lineEnd - 1)))
            --lineEnd;

        auto documentGlyph = lineStart;

        glyph_text lineGlyphs{ font(), lineStart, lineEnd };
        lineGlyphs.content().align_baselines();

        auto const columnRect = column_rect(aColumn.index());
        auto const clientRect = client_rect();
        optional_text_format textAppearance;
        point const lineOrigin = lineStart->cell[0];
        point const textPos = aPosition - lineOrigin;
        auto segmentStart = lineGlyphs.cbegin();
        for (auto segmentGlyph = segmentStart; segmentGlyph != lineGlyphs.cend(); ++segmentGlyph, ++documentGlyph)
        {
            bool selected = false;
            if (cursor().position() != cursor().anchor())
            {
                auto gp = static_cast<cursor::position_type>(from_glyph(documentGlyph).first);
                selected = (gp >= std::min(cursor().position(), cursor().anchor()) && gp < std::max(cursor().position(), cursor().anchor()));
            }
            auto const& style = glyph_style(documentGlyph, iColumns[aColumn.index()]);
            auto const& glyphColor = with_bounding_box(style.character().glyph_color() == neolib::none ?
                style.character().text_color() != neolib::none ?
                    with_bounding_box(style.character().text_color(), columnRect) :
                    default_text_color() : 
                style.character().glyph_color(), clientRect, true);
            auto const& nextTextAppearance = !selected ?
                text_format{
                    glyphColor,
                    style.character().paper_color() != neolib::none ? optional_text_color{ neogfx::text_color{ style.character().paper_color() } } : optional_text_color{},
                    style.character().text_effect() }.with_smart_underline(style.character().smart_underline()).with_emoji_ignored(style.character().ignore_emoji()) :
                text_format{
                    has_focus() ? service<i_app>().current_style().palette().color(color_role::SelectedText) : glyphColor,
                    has_focus() ? service<i_app>().current_style().palette().color(color_role::Selection) : service<i_app>().current_style().palette().color(color_role::Selection).with_alpha(64) };
            if (textAppearance && textAppearance.value() != nextTextAppearance)
            {
                aGc.draw_glyph_text(textPos, lineGlyphs, segmentStart, segmentGlyph, textAppearance.value());
                segmentStart = segmentGlyph;
            }
            textAppearance = nextTextAppearance;
        }
        if (segmentStart != lineGlyphs.cend())
            aGc.draw_glyph_text(textPos, lineGlyphs, segmentStart, lineGlyphs.cend(), textAppearance.value());
    }

    void text_edit::draw_cursor(i_graphics_context const& aGc) const
    {
        auto elapsedTime_ms = (neolib::this_process::elapsed_ms() - iCursorAnimationStartTime);
        auto const flashInterval_ms = cursor().flash_interval().count();
        auto const normalizedFrameTime = (elapsedTime_ms % flashInterval_ms) / ((flashInterval_ms - 1) * 1.0);
        auto const cursorAlpha = neolib::service<neolib::i_power>().green_mode_active() ? 1.0 : partitioned_ease(easing::InvertedInOutQuint, easing::InOutQuint, normalizedFrameTime);
        auto cursorColor = cursor().color();
        if (cursorColor == neolib::none)
            cursorColor = service<i_app>().current_style().palette().default_text_color_for_widget(*this);
        if (cursor().style() == cursor_style::Xor)
        {
            aGc.push_logical_operation(logical_operation::Xor);
            aGc.fill_rect(cursor_rect(), (cursorAlpha >= 0.5 ? color::White : color::Black));
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
        dimension yHeight = 0.0;
        std::optional<scalar> xWidth;
        scalar yOffset = 0.0;
        if (cursorPos.glyph && cursorPos.glyph.value() != glyphs().end())
        {
            auto iterGlyph = (cursorPos.glyph > cursorPos.lineStart.value() ? cursorPos.glyph.value() - 1 : cursorPos.glyph.value());
            auto const& glyph = *iterGlyph;
            yHeight = cursorPos.line.value()->extents.cy;
            yOffset = glyph.cell[0].as<scalar>().y;
            xWidth = cursorPos.glyph.value()->cell_extents().as<scalar>().x;
        }
        else if (cursorPos.lineStart && cursorPos.lineStart.value() != cursorPos.lineEnd.value())
            yHeight = cursorPos.line.value()->extents.cy;
        else
            yHeight = current_style().character().font().value().height();
        auto const columnRectSansPadding = column_rect(cursorPos.column_index());
        rect cursorRect{ 
            point{ cursorPos.pos - 
                point{ horizontal_scrollbar().position(), vertical_scrollbar().position() } } + columnRectSansPadding.top_left() + point{ 0.0, yOffset },
            size{ static_cast<scalar>(cursor().width(*this, xWidth)), yHeight } };
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
            paddingAdjust = std::max(paddingAdjust, calc_padding_adjust(*s));
        return paddingAdjust;
    }
}