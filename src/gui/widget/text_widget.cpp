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

#include <neogfx/app/i_app.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/widget/text_widget.hpp>

namespace neogfx
{
    text_widget::text_widget(std::string const& aText, text_widget_type aType, text_widget_flags aFlags) :
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

    text_widget::text_widget(i_widget& aParent, std::string const& aText, text_widget_type aType, text_widget_flags aFlags) :
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

    text_widget::text_widget(i_layout& aLayout, std::string const& aText, text_widget_type aType, text_widget_flags aFlags) :
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

    size text_widget::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (widget::has_minimum_size())
            return widget::minimum_size(aAvailableSpace);
        else
        {
            size extent = units_converter{ *this }.to_device_units(text_extent().max(size_hint_extent()));
            size result = extent + units_converter{ *this }.to_device_units(internal_spacing().size());
            if (has_maximum_size())
            {
                result.cx = std::min(std::ceil(result.cx), maximum_size().cx);
                result.cy = std::min(std::ceil(result.cy), maximum_size().cy);
            }
            if ((flags() & text_widget_flags::CutOff) == text_widget_flags::CutOff)
                result.cx = 1.0;
            // todo: ellipsis
            if (result.cx == 0.0 && (flags() & text_widget_flags::TakesSpaceWhenEmpty) == text_widget_flags::TakesSpaceWhenEmpty)
                result.cx = 1.0;
#ifdef NEOGFX_DEBUG
            if (debug::layoutItem == this)
                service<debug::logger>() << neolib::logger::severity::Debug << "text_widget::minimum_size(" << aAvailableSpace << ") --> " << result << std::endl;
#endif // NEOGFX_DEBUG
            return units_converter{ *this }.from_device_units(result);
        }
    }

    void text_widget::paint(i_graphics_context& aGc) const
    {
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << "text_widget::paint(...)" << std::endl;
#endif // NEOGFX_DEBUG

        scoped_mnemonics sm{ aGc, service<i_keyboard>().is_key_pressed(ScanCode_LALT) && has_root() && root().is_active() };

        size textSize = text_extent();
        point textPosition;
        switch (iAlignment & neogfx::alignment::Horizontal)
        {
        case neogfx::alignment::Left:
        case neogfx::alignment::Justify:
            textPosition.x = 0.0;
            break;
        case neogfx::alignment::Center:
            textPosition.x = std::floor((client_rect().width() - textSize.cx) / 2.0);
            break;
        case neogfx::alignment::Right:
            textPosition.x = std::floor((client_rect().width() - textSize.cx));
            break;
        default:
            break;
        }
        switch (iAlignment & neogfx::alignment::Vertical)
        {
        case neogfx::alignment::Top:
            textPosition.y = 0.0;
            break;
        case neogfx::alignment::VCenter:
            textPosition.y = std::floor((client_rect().height() - textSize.cy) / 2.0);
            break;
        case neogfx::alignment::Bottom:
            textPosition.y = std::floor((client_rect().height() - textSize.cy));
            break;
        default:
            break;
        }
        auto appearance = text_format();
        if (appearance.ink() == neolib::none)
            appearance.set_ink(text_color());
        if (appearance.effect())
            textPosition += size{ appearance.effect()->width() };
        if (multi_line())
            aGc.draw_multiline_glyph_text(textPosition, std::get<multiline_glyph_text>(glyph_text()), appearance);
        else
            aGc.draw_glyph_text(textPosition, std::get<neogfx::glyph_text>(glyph_text()), appearance);
    }

    void text_widget::set_font(optional_font const& aFont)
    {
        widget::set_font(aFont);
        reset_cache();
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
            size oldSize = minimum_size();
            iText = aText;
            reset_cache();
            TextChanged();
            if (oldSize != minimum_size())
            {
                TextGeometryChanged();
                if (visible() || effective_size_policy().ignore_visibility())
                    update_layout(true, true);
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
            if (visible() || effective_size_policy().ignore_visibility())
                update_layout(true, true);
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
        iFlags = aFlags;
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
            set_text_format(neogfx::text_format{ aTextColor != std::nullopt ? *aTextColor : neogfx::text_color{}, iTextAppearance->paper(), iTextAppearance->effect() });
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
        if (text_format().effect())
            *iTextExtent += size{ text_format().effect()->width() * 2.0 };
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
        set_padding(neogfx::padding{ 0.0 });
        set_ignore_mouse_events(true);
        auto style_changed = [&]()
        {
            reset_cache();
            update_layout(true, true);
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
                iGlyphText = gc.to_glyph_text(iText, font());
        }
        return iGlyphText;
    }

    void text_widget::reset_cache()
    {
        iTextExtent = std::nullopt;
        iSizeHintExtent = std::nullopt;
        iGlyphText = std::monostate{};
    }
}