// text_widget.cpp
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

#include <algorithm>
#include <neolib/core/string_utf.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gfx/gradient.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/widget/text_widget.hpp>

namespace neogfx
{
    text_widget::text_widget(string const& aText, text_widget_type aType, text_widget_flags aFlags) :
        widget{}, iText{ aText }, iType{ aType }, iFlags{ aFlags }, iAlignment { neogfx::alignment::Center | neogfx::alignment::VCenter },
        iAnimator{ *this, [this](widget_timer&)
        {
            iAnimator.again();
            if (has_text_format() && text_format().animation())
                update();
        }, std::chrono::milliseconds{ 16 } }
    {
        init();
    }

    text_widget::text_widget(i_widget& aParent, string const& aText, text_widget_type aType, text_widget_flags aFlags) :
        widget{ aParent }, iText{ aText }, iType{ aType }, iFlags{ aFlags }, iAlignment{ neogfx::alignment::Center | neogfx::alignment::VCenter },
        iAnimator{ *this, [this](widget_timer&)
        {
            iAnimator.again();
            if (has_text_format() && text_format().animation())
                update();
        }, std::chrono::milliseconds{ 16 } }
    {
        init();
    }

    text_widget::text_widget(i_layout& aLayout, string const& aText, text_widget_type aType, text_widget_flags aFlags) :
        widget{ aLayout }, iText{ aText }, iType{ aType }, iFlags{ aFlags }, iAlignment{ neogfx::alignment::Center | neogfx::alignment::VCenter },
        iAnimator{ *this, [this](widget_timer&)
        {
            iAnimator.again();
            if (has_text_format() && text_format().animation())
                update();
        }, std::chrono::milliseconds{ 16 } }
    {
        init();
    }

    text_widget::~text_widget()
    {
    }

    neogfx::size_policy text_widget::size_policy() const
    {
        if (widget::has_size_policy())
            return widget::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return size_constraint::Minimum;
    }

    padding text_widget::padding() const
    {
        auto result = widget::padding();
        if (is_aligning_to())
        {
            scalar const otherDescender = aligning_to().font().descender();
            result.bottom = aligning_to().padding().bottom + -(otherDescender - font().descender());
        }
        return result;
    }

    size text_widget::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (widget::has_minimum_size())
            return widget::minimum_size(aAvailableSpace);
        else
        {
            size extent = text_extent().max(size_hint_extent());
            if (iRotation != 0.0)
                // rotated, the measured height becomes our width; that height is measured from the
                // glyph run, so it varies with the string, and quantising it to whole lines keeps
                // widgets holding different text the same width and therefore aligned with one another
                extent.cy = quantized_text_height(extent.cy);
            extent = units_converter{ *this }.to_device_units(extent);
            if (iRotation != 0.0)
                extent = scoped_transform::rotated_extents(extent, iRotation);
            size result = extent + units_converter{ *this }.to_device_units(internal_spacing().size());
            if (has_maximum_size())
            {
                result.cx = std::min(std::ceil(result.cx), maximum_size().cx);
                result.cy = std::min(std::ceil(result.cy), maximum_size().cy);
            }
            if ((flags() & text_widget_flags::CutOff) == text_widget_flags::CutOff ||
                (flags() & text_widget_flags::UseEllipsis) == text_widget_flags::UseEllipsis ||
                (flags() & text_widget_flags::UseFade) == text_widget_flags::UseFade)
                result.cx = 1.0;
            if (result.cx == 0.0 && (flags() & text_widget_flags::TakesSpaceWhenEmpty) == text_widget_flags::TakesSpaceWhenEmpty)
                result.cx = 1.0;
#ifdef NEOGFX_DEBUG
            if (service<i_debug>().layout_item() == this)
                service<debug::logger>() << neolib::logger::severity::Debug << "text_widget::minimum_size(" << aAvailableSpace << ") --> " << result << std::endl;
#endif // NEOGFX_DEBUG
            return units_converter{ *this }.from_device_units(result);
        }
    }

    void text_widget::resized()
    {
        widget::resized();
        // the text is elided to fit the width we have just been given
        if (!multi_line() && (flags() & (text_widget_flags::UseEllipsis | text_widget_flags::UseFade)) != text_widget_flags::None)
        {
            reset_cache();
            update();
        }
    }

    void text_widget::paint(i_graphics_context& aGc) const
    {
#ifdef NEOGFX_DEBUG
        if (service<i_debug>().layout_item() == this)
            service<debug::logger>() << neolib::logger::severity::Debug << "text_widget::paint(...)" << std::endl;
#endif // NEOGFX_DEBUG

        // popup menus never become the active window but accept their items' mnemonics without Alt, so always show them there
        bool const inPopupMenu = has_root() &&
            (root().style() & (window_style::Popup | window_style::Menu)) == (window_style::Popup | window_style::Menu);
        scoped_mnemonics sm{ aGc, inPopupMenu || (service<i_keyboard>().is_key_pressed(ScanCode_LALT) && has_root() && root().is_active()) };

        size textSize = text_extent();
        auto const clientRect = client_rect(false);
        // when rotated the text is laid out in the box the client rect came from, i.e. the one that
        // maps onto the client rect under the rotation, and is then rotated about its centre
        auto const textRect = [&]() -> rect
            {
                if (iRotation == 0.0)
                    return clientRect;
                auto const unrotated = scoped_transform::rotated_extents(clientRect.extents(), -iRotation);
                return rect{ point{
                    clientRect.center().x - unrotated.cx / 2.0,
                    clientRect.center().y - unrotated.cy / 2.0 }, unrotated };
            }();
        // rotated, the vertical axis is the one across the widget, and aligning on this string's
        // measured extent would put text of differing heights in differing places; quantising to
        // whole lines removes the dependence on the glyphs while still counting the lines
        auto const alignExtent = (iRotation != 0.0 ? quantized_text_height(textSize.cy) : textSize.cy);

        point textPosition;
        switch (iAlignment & neogfx::alignment::Horizontal)
        {
        case neogfx::alignment::Left:
        case neogfx::alignment::Justify:
            textPosition.x = textRect.left();
            break;
        case neogfx::alignment::Center:
            textPosition.x = std::floor(textRect.left() + (textRect.width() - textSize.cx) / 2.0);
            break;
        case neogfx::alignment::Right:
            textPosition.x = std::floor((textRect.right() - textSize.cx));
            break;
        default:
            break;
        }
        switch (iAlignment & neogfx::alignment::Vertical)
        {
        case neogfx::alignment::Top:
            textPosition.y = textRect.top();
            break;
        case neogfx::alignment::VCenter:
            textPosition.y = std::floor(textRect.top() + (textRect.height() - alignExtent) / 2.0);
            break;
        case neogfx::alignment::Bottom:
            textPosition.y = std::floor((textRect.bottom() - alignExtent));
            break;
        default:
            break;
        }
        auto appearance = text_format();
        if (appearance.ink() == neolib::none)
            appearance.set_ink(text_color());
        textPosition += size{ font().info().outline().radius };

        scalar textEffectOutset = 0.0;
        if (appearance.effect())
            textEffectOutset = appearance.effect()->outset();
        if (appearance.effect2())
            textEffectOutset = std::max(textEffectOutset, appearance.effect2()->outset());

        textPosition += size{ textEffectOutset };

        std::optional<scoped_gradient_filter> fade;
        if (!multi_line() && (flags() & text_widget_flags::UseFade) == text_widget_flags::UseFade &&
            textSize.cx > textRect.width())
            fade.emplace(aGc, fade_gradient(textPosition, textSize),
                rect{ textPosition, textSize } + aGc.origin());

        std::optional<scoped_transform> rotate;
        if (iRotation != 0.0)
        {
            // a quarter turn maps whole pixels to whole pixels only if the pivot's components share
            // a fractional part, so pin both the pivot and the text to the pixel grid; otherwise the
            // text lands half a pixel out, by differing amounts depending on the widget's extents
            textPosition = point{ std::round(textPosition.x), std::round(textPosition.y) };
            rotate.emplace(aGc, iRotation,
                point{ std::round(clientRect.center().x), std::round(clientRect.center().y) });
        }

        if (iCacheTexture)
        {
            aGc.draw_texture(point{}, iCacheTexture.value());
            return;
        }

        if ((flags() & text_widget_flags::CacheToTexture) == text_widget_flags::CacheToTexture)
            iCacheTexture.emplace(textSize, 1.0, texture_sampling::Multisample);

        if (!iCacheTexture)
        {
            if (multi_line())
                aGc.draw_multiline_glyph_text(textPosition, std::get<multiline_glyph_text>(glyph_text()), appearance);
            else
                aGc.draw_glyph_text(textPosition, std::get<neogfx::glyph_text>(glyph_text()), appearance);
        }
        else
        {
            {
                graphics_context gcCacheTexture{ iCacheTexture.value() };
                gcCacheTexture.set_logical_coordinate_system(aGc.logical_coordinate_system());
                if (multi_line())
                    gcCacheTexture.draw_multiline_glyph_text(textPosition, std::get<multiline_glyph_text>(glyph_text()), appearance);
                else
                    gcCacheTexture.draw_glyph_text(textPosition, std::get<neogfx::glyph_text>(glyph_text()), appearance);
            }
            aGc.draw_texture(point{}, iCacheTexture.value());
        }
    }

    gradient text_widget::fade_gradient(point const& aTextPosition, size const& aTextSize) const
    {
        auto const clientRect = client_rect(false);

        // fade the alpha out at whichever of our edges the text runs past, which depends on how the
        // text is aligned; the positions are fractions of the text, which is what the bounding box is
        auto const fadeWidth = std::min(font().height(), aTextSize.cx) / aTextSize.cx;
        auto const leadingEdge = std::clamp((clientRect.left() - aTextPosition.x) / aTextSize.cx, 0.0, 1.0);
        auto const trailingEdge = std::clamp((clientRect.right() - aTextPosition.x) / aTextSize.cx, 0.0, 1.0);
        bool const fadeLeading = (leadingEdge > 0.0);
        bool const fadeTrailing = (trailingEdge < 1.0);

        gradient::alpha_stop_list alphaStops;
        if (fadeLeading)
        {
            alphaStops.push_back(gradient::alpha_stop{ leadingEdge, 0_u8 });
            alphaStops.push_back(gradient::alpha_stop{ std::min(leadingEdge + fadeWidth, 1.0), 255_u8 });
        }
        else
            alphaStops.push_back(gradient::alpha_stop{ 0.0, 255_u8 });
        if (fadeTrailing)
        {
            alphaStops.push_back(gradient::alpha_stop{ std::max(trailingEdge - fadeWidth, 0.0), 255_u8 });
            alphaStops.push_back(gradient::alpha_stop{ trailingEdge, 0_u8 });
        }
        else
            alphaStops.push_back(gradient::alpha_stop{ 1.0, 255_u8 });

        // too narrow for both fades to fit between the edges, so fade the lot
        if (!std::is_sorted(alphaStops.begin(), alphaStops.end(),
            [](auto const& aLhs, auto const& aRhs) { return aLhs.first() < aRhs.first(); }))
        {
            alphaStops.clear();
            alphaStops.push_back(gradient::alpha_stop{ 0.0, 255_u8 });
            alphaStops.push_back(gradient::alpha_stop{ 1.0, 0_u8 });
        }

        // the colors are immaterial: a gradient composed onto the context filters the alpha of what
        // is drawn and leaves its color, gradient or not, as it is
        return gradient{
            gradient::color_stop_list{ { 0.0, color::White }, { 1.0, color::White } },
            alphaStops,
            gradient_direction::Horizontal };
    }

    void text_widget::set_font(optional_font const& aFont)
    {
        size oldSize = minimum_size();
        widget::set_font(aFont);
        reset_cache();
        if (oldSize != minimum_size())
        {
            TextGeometryChanged();
            if (visible() || effective_size_policy().ignore_visibility())
                update_layout();
        }
        update();
    }

    bool text_widget::visible() const
    {
        if (iText.empty() && (iFlags & text_widget_flags::HideOnEmpty) == text_widget_flags::HideOnEmpty)
            return false;
        return widget::visible();
    }

    i_string const& text_widget::text() const
    {
        return iText;
    }

    void text_widget::set_text(i_string const& aText)
    {
        if (iText != aText)
        {
            size const oldSize = minimum_size();
            iText = aText;
            reset_cache();
            TextChanged();
            if (oldSize != minimum_size())
            {
                TextGeometryChanged();
                if (visible() || effective_size_policy().ignore_visibility())
                    update_layout();
            }
            update();
        }
    }

    void text_widget::set_size_hint(const size_hint& aSizeHint)
    {
        if (iSizeHint != aSizeHint)
        {
            size oldSize = minimum_size();
            iSizeHint = aSizeHint;
            reset_cache();
            if (oldSize != minimum_size())
            {
                TextGeometryChanged();
                if (visible() || effective_size_policy().ignore_visibility())
                    update_layout();
            }
        }
    }

    bool text_widget::multi_line() const
    {
        return iType == text_widget_type::MultiLine;
    }

    text_widget_flags text_widget::flags() const
    {
        return iFlags;
    }
    
    void text_widget::set_flags(text_widget_flags aFlags)
    {
        if (iFlags != aFlags)
        {
            iFlags = aFlags;
            reset_cache();
            update_layout();
        }
    }

    neogfx::alignment text_widget::alignment() const
    {
        return iAlignment;
    }

    void text_widget::set_alignment(neogfx::alignment aAlignment, bool aUpdateLayout)
    {
        if (iAlignment != aAlignment)
        {
            iAlignment = aAlignment;
            if (aUpdateLayout)
                update_layout();
        }
    }

    bool text_widget::is_aligning_to() const
    {
        return iAlignmentTo != nullptr;
    }

    i_text_widget const& text_widget::aligning_to() const
    {
        if (iAlignmentTo == nullptr)
            throw text_widget_not_aligning_to();
        return *iAlignmentTo;
    }

    void text_widget::align_to(i_ref_ptr<i_text_widget const> const& aOtherWidget)
    {
        if (iAlignmentTo != aOtherWidget)
        {
            if (aOtherWidget != nullptr)
            {
                i_text_widget const* other = aOtherWidget.ptr();
                if (other->is_aligning_to())
                    for (auto next = &other->aligning_to(); next->is_aligning_to(); next = &next->aligning_to())
                        if (next == other || next == this)
                            throw text_widget_alignment_cycle();
            }
            iAlignmentTo = aOtherWidget;
            update_layout();
        }
    }

    void text_widget::stop_alignment_to()
    {
        if (iAlignmentTo != nullptr)
        {
            iAlignmentTo = nullptr;
            update_layout();
        }
    }

    angle text_widget::rotation() const
    {
        return iRotation;
    }

    void text_widget::set_rotation(angle aRotation)
    {
        if (iRotation != aRotation)
        {
            iRotation = aRotation;
            reset_cache();
            update_layout();
            update();
        }
    }

    bool text_widget::has_text_color() const
    {
        return has_text_format() && text_format().ink() != neolib::none && std::holds_alternative<color>(text_format().ink());
    }

    color text_widget::text_color() const
    {
        if (has_text_color())
            return static_variant_cast<color>(iTextAppearance->ink());
        return service<i_app>().current_style().palette().default_text_color_for_widget(*this);
    }

    void text_widget::set_text_color(const optional_color& aTextColor)
    {
        if (has_text_format())
            set_text_format(text_format().with_ink(aTextColor != std::nullopt ? *aTextColor : neogfx::text_color{}));
        else
            set_text_format(neogfx::text_format{ aTextColor != std::nullopt ? *aTextColor : neogfx::text_color{} });
    }

    bool text_widget::has_text_format() const
    {
        return iTextAppearance != std::nullopt;
    }

    text_format text_widget::text_format() const
    {
        if (has_text_format())
            return *iTextAppearance;
        return neogfx::text_format{ text_color() };
    }

    void text_widget::set_text_format(const optional_text_format& aTextAppearance)
    {
        if (iTextAppearance != aTextAppearance)
        {
            iTextAppearance = aTextAppearance;
            reset_cache();
            update();
        }
    }

    dimension text_widget::quantized_text_height(dimension aHeight) const
    {
        // the glyph run's measured height depends on which glyphs are in it; rounding to whole
        // lines keeps it dependent only on the font and the number of lines. the outsets that
        // text_extent adds are not part of that, so they come off first and go back on after,
        // otherwise rounding either eats them or claims an extra line
        scalar outset = font().info().outline().radius * 2.0;
        scalar effectOutset = 0.0;
        if (text_format().effect())
            effectOutset = std::max(effectOutset, text_format().effect()->outset());
        if (text_format().effect2())
            effectOutset = std::max(effectOutset, text_format().effect2()->outset());
        outset += effectOutset * 2.0;

        auto const lineHeight = font().height();
        auto const lines = std::max(1.0, std::round((aHeight - outset) / lineHeight));
        return std::ceil(lines * lineHeight + outset);
    }

    size text_widget::text_extent() const
    {
        if (iTextExtent != std::nullopt)
            return *iTextExtent;
        else if (!has_surface())
            return size{};
        graphics_context gc{ *this, graphics_context::type::Unattached };
        scoped_mnemonics sm{ gc, service<i_keyboard>().is_key_pressed(ScanCode_LALT) };
        if (multi_line())
            iTextExtent = size{ std::get<multiline_glyph_text>(glyph_text()).bbox[2] - std::get<multiline_glyph_text>(glyph_text()).bbox[0] };
        else
            iTextExtent = gc.glyph_text_extent(std::get<neogfx::glyph_text>(glyph_text()));
        if (iTextExtent->cy == 0.0)
            iTextExtent->cy = font().height();
        *iTextExtent += size{ font().info().outline().radius * 2.0 };
        scalar textEffectOutset = 0.0;
        if (text_format().effect())
            textEffectOutset = std::max(textEffectOutset, text_format().effect()->outset());
        if (text_format().effect2())
            textEffectOutset = std::max(textEffectOutset, text_format().effect2()->outset());
        *iTextExtent += size{ textEffectOutset * 2.0 };
        iTextExtent = iTextExtent->ceil();
        return *iTextExtent;
    }

    size text_widget::size_hint_extent() const
    {
        if (iSizeHintExtent != std::nullopt)
            return *iSizeHintExtent;
        else if (!has_surface())
            return size{};
        else
        {
            graphics_context gc{ *this, graphics_context::type::Unattached };
            scoped_mnemonics sm{ gc, service<i_keyboard>().is_key_pressed(ScanCode_LALT) };
            if (multi_line())
            {
                if (widget::has_minimum_size() && widget::minimum_size().cx != 0 && widget::minimum_size().cy == 0)
                    iSizeHintExtent = gc.multiline_text_extent(iSizeHint.primary_hint(), font(), widget::minimum_size().cx - internal_spacing().size().cx).max(
                        gc.multiline_text_extent(iSizeHint.secondary_hint(), font(), widget::minimum_size().cx - internal_spacing().size().cx));
                else if (widget::has_maximum_size() && widget::maximum_size().cx != size::max_dimension())
                    iSizeHintExtent = gc.multiline_text_extent(iSizeHint.primary_hint(), font(), widget::maximum_size().cx - internal_spacing().size().cx).max(
                        gc.multiline_text_extent(iSizeHint.secondary_hint(), font(), widget::maximum_size().cx - internal_spacing().size().cx));
                else
                    iSizeHintExtent = gc.multiline_text_extent(iSizeHint.primary_hint(), font()).max(
                        gc.multiline_text_extent(iSizeHint.secondary_hint(), font()));
            }
            else
                iSizeHintExtent = gc.text_extent(iSizeHint.primary_hint(), font()).max(
                    gc.text_extent(iSizeHint.secondary_hint(), font()));
        }
        if (iSizeHintExtent->cy == 0.0)
            iSizeHintExtent->cy = font().height();
        iSizeHintExtent = iSizeHintExtent->ceil();
        return *iSizeHintExtent;
    }

    void text_widget::init()
    {
        set_padding(neogfx::padding{});
        set_ignore_mouse_events(true);
        auto style_changed = [&]()
        {
            reset_cache();
            update_layout();
            update();
        };
        iSink += service<i_app>().current_style_changed([this, style_changed](style_aspect aAspect)
        {
            if (!has_font() && (aAspect & style_aspect::Font) == style_aspect::Font)
                style_changed();
        });
        iSink += service<i_rendering_engine>().subpixel_rendering_changed(style_changed);
    }

    const text_widget::glyph_text_t& text_widget::glyph_text() const
    {
        if (std::holds_alternative<std::monostate>(iGlyphText))
        {
            graphics_context gc{ *this, graphics_context::type::Unattached };
            scoped_mnemonics sm(gc, service<i_keyboard>().is_key_pressed(ScanCode_LALT));
            if (multi_line())
            {
                if (widget::has_minimum_size() && widget::minimum_size().cx != 0 && widget::minimum_size().cy == 0)
                    iGlyphText = gc.to_multiline_glyph_text(iText, font(), widget::minimum_size().cx - internal_spacing().size().cx, iAlignment & neogfx::alignment::Horizontal);
                else if (widget::has_maximum_size() && widget::maximum_size().cx != size::max_dimension())
                    iGlyphText = gc.to_multiline_glyph_text(iText, font(), widget::maximum_size().cx - internal_spacing().size().cx, iAlignment & neogfx::alignment::Horizontal);
                else
                    iGlyphText = gc.to_multiline_glyph_text(iText, font(), 0.0, iAlignment & neogfx::alignment::Horizontal);
            }
            else
            {
                iGlyphText = gc.to_glyph_text(iText, font());
                // the fade takes precedence: it truncates by fading the text out, not by ending it
                if ((flags() & text_widget_flags::UseEllipsis) == text_widget_flags::UseEllipsis &&
                    (flags() & text_widget_flags::UseFade) != text_widget_flags::UseFade)
                    elide(gc);
            }
        }
        return iGlyphText;
    }

    void text_widget::elide(i_graphics_context& aGc) const
    {
        // the extent of the text includes what the outline and the text effects add to it
        scalar decorations = font().info().outline().radius * 2.0;
        scalar textEffectOutset = 0.0;
        if (text_format().effect())
            textEffectOutset = std::max(textEffectOutset, text_format().effect()->outset());
        if (text_format().effect2())
            textEffectOutset = std::max(textEffectOutset, text_format().effect2()->outset());
        decorations += textEffectOutset * 2.0;

        auto const availableWidth = client_rect(false).width() - decorations;

        if (availableWidth <= 0.0 || aGc.glyph_text_extent(std::get<neogfx::glyph_text>(iGlyphText)).cx <= availableWidth)
            return;

        char32_t constexpr ELLIPSIS = U'\x2026';

        auto const text = neolib::utf8_to_utf32(iText.to_std_string());

        // the most characters that fit with the ellipsis in tow
        std::size_t characters = 0u;
        std::size_t lo = 0u;
        std::size_t hi = text.size();
        while (lo <= hi)
        {
            auto const candidateLength = lo + (hi - lo) / 2u;
            auto const candidate = aGc.to_glyph_text(text.substr(0u, candidateLength) + ELLIPSIS, font());
            if (aGc.glyph_text_extent(candidate).cx <= availableWidth)
            {
                characters = candidateLength;
                lo = candidateLength + 1u;
            }
            else if (candidateLength == 0u)
                break;
            else
                hi = candidateLength - 1u;
        }

        iGlyphText = aGc.to_glyph_text(text.substr(0u, characters) + ELLIPSIS, font());
        iTextExtent = std::nullopt;
    }

    void text_widget::reset_cache()
    {
        iTextExtent = std::nullopt;
        iSizeHintExtent = std::nullopt;
        iGlyphText = std::monostate{};
        iCacheTexture = std::nullopt;
    }
}