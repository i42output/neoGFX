// palette.cpp
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

#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/palette.hpp>

namespace neogfx
{
    namespace
    {
        // WCAG 2.x relative luminance (linear light, Rec. 709 primaries)
        scalar relative_luminance(color const& aColor)
        {
            auto const linear = aColor.to_linear();
            return 0.2126 * linear.x + 0.7152 * linear.y + 0.0722 * linear.z;
        }

        // WCAG 2.x contrast ratio (1:1 to 21:1)
        scalar contrast_ratio(color const& aLhs, color const& aRhs)
        {
            auto const lhs = relative_luminance(aLhs);
            auto const rhs = relative_luminance(aRhs);
            return (std::max(lhs, rhs) + 0.05) / (std::min(lhs, rhs) + 0.05);
        }

        // light text is used on any backdrop where it reaches this contrast (the WCAG minimum for large text); WCAG's
        // luminance contrast overstates the legibility of dark text on mid-tones so a symmetric "pick the higher
        // contrast" test chooses black where white reads better (e.g. black on the default selection colour)
        constexpr scalar LightTextMinimumContrast = 3.0;
        // the palette's own text colour is kept if it has the chosen polarity and reaches this contrast (the WCAG
        // AA minimum for normal text) or at least the contrast of black/white text if that is lower
        constexpr scalar DefaultTextMinimumContrast = 4.5;
    }

    palette::palette()
    {
    }

    palette::palette(current_style_palette_proxy_t) :
        iProxy{ nullptr }
    {
    }

    palette::palette(const i_palette& aOther) :
        iThemeColor{ aOther.maybe_color(color_role::Theme) },
        iBackgroundColor{ aOther.maybe_color(color_role::Background) },
        iForegroundColor{ aOther.maybe_color(color_role::Foreground) },
        iBaseColor{ aOther.maybe_color(color_role::Base) },
        iAlternateBaseColor{ aOther.maybe_color(color_role::AlternateBase) },
        iTextColor{ aOther.maybe_color(color_role::Text) },
        iSelectionColor{ aOther.maybe_color(color_role::Selection) },
        iAlternateSelectionColor{ aOther.maybe_color(color_role::AlternateSelection) },
        iSelectedTextColor{ aOther.maybe_color(color_role::SelectedText) },
        iFocusColor{ aOther.maybe_color(color_role::Focus) },
        iHoverColor{ aOther.maybe_color(color_role::Hover) },
        iPrimaryAccentColor{ aOther.maybe_color(color_role::PrimaryAccent) },
        iSecondaryAccentColor{ aOther.maybe_color(color_role::SecondaryAccent) },
        iVoidColor{ aOther.maybe_color(color_role::Void) }
    {
        if (aOther.has_proxy())
            iProxy.emplace(aOther.proxy_ptr());
    }

    palette::palette(const palette& aOther) :
        palette{ static_cast<const i_palette&>(aOther) }
    {
    }

    palette::palette(const i_palette& aOther, palette_proxy_t) :
        iProxy{ &aOther }
    {
    }

    palette::palette(const palette& aOther, palette_proxy_t) :
        palette{ static_cast<const i_palette&>(aOther), palette_proxy_t{} }
    {
    }

    palette& palette::operator=(const i_palette& aOther)
    {
        if (*this != aOther)
        {
            if (aOther.has_proxy())
                iProxy = aOther.proxy_ptr();
            else
                iProxy = std::nullopt;
            iThemeColor = aOther.maybe_color(color_role::Theme);
            iBackgroundColor = aOther.maybe_color(color_role::Background);
            iForegroundColor = aOther.maybe_color(color_role::Foreground);
            iBaseColor = aOther.maybe_color(color_role::Base);
            iAlternateBaseColor = aOther.maybe_color(color_role::AlternateBase);
            iTextColor = aOther.maybe_color(color_role::Text);
            iSelectionColor = aOther.maybe_color(color_role::Selection);
            iAlternateSelectionColor = aOther.maybe_color(color_role::AlternateSelection);
            iSelectedTextColor = aOther.maybe_color(color_role::SelectedText);
            iFocusColor = aOther.maybe_color(color_role::Focus);
            iHoverColor = aOther.maybe_color(color_role::Hover);
            iPrimaryAccentColor = aOther.maybe_color(color_role::PrimaryAccent);
            iSecondaryAccentColor = aOther.maybe_color(color_role::SecondaryAccent);
            iVoidColor = aOther.maybe_color(color_role::Void);
            Changed();
        }
        return *this;
    }

    palette& palette::operator=(const palette& aOther)
    {
        return operator=(static_cast<const i_palette&>(aOther));
    }

    bool palette::operator==(const palette& aOther) const
    {
        return
            iThemeColor == aOther.maybe_color(color_role::Theme) &&
            iBackgroundColor == aOther.maybe_color(color_role::Background) &&
            iForegroundColor == aOther.maybe_color(color_role::Foreground) &&
            iBaseColor == aOther.maybe_color(color_role::Base) &&
            iAlternateBaseColor == aOther.maybe_color(color_role::AlternateBase) &&
            iTextColor == aOther.maybe_color(color_role::Text) &&
            iSelectionColor == aOther.maybe_color(color_role::Selection) &&
            iAlternateSelectionColor == aOther.maybe_color(color_role::AlternateSelection) &&
            iSelectedTextColor == aOther.maybe_color(color_role::SelectedText) &&
            iFocusColor == aOther.maybe_color(color_role::Focus) &&
            iHoverColor == aOther.maybe_color(color_role::Hover) &&
            iPrimaryAccentColor == aOther.maybe_color(color_role::PrimaryAccent) &&
            iSecondaryAccentColor == aOther.maybe_color(color_role::SecondaryAccent) &&
            iVoidColor == aOther.maybe_color(color_role::Void);
    }

    bool palette::operator!=(const palette& aOther) const
    {
        return !(*this == aOther);
    }

    bool palette::operator==(const i_palette& aOther) const
    {
        return
            iThemeColor == aOther.maybe_color(color_role::Theme) &&
            iBackgroundColor == aOther.maybe_color(color_role::Background) &&
            iForegroundColor == aOther.maybe_color(color_role::Foreground) &&
            iBaseColor == aOther.maybe_color(color_role::Base) &&
            iAlternateBaseColor == aOther.maybe_color(color_role::AlternateBase) &&
            iTextColor == aOther.maybe_color(color_role::Text) &&
            iSelectionColor == aOther.maybe_color(color_role::Selection) &&
            iAlternateSelectionColor == aOther.maybe_color(color_role::AlternateSelection) &&
            iSelectedTextColor == aOther.maybe_color(color_role::SelectedText) &&
            iFocusColor == aOther.maybe_color(color_role::Focus) &&
            iHoverColor == aOther.maybe_color(color_role::Hover) &&
            iPrimaryAccentColor == aOther.maybe_color(color_role::PrimaryAccent) &&
            iSecondaryAccentColor == aOther.maybe_color(color_role::SecondaryAccent) &&
            iVoidColor == aOther.maybe_color(color_role::Void);
    }

    bool palette::operator!=(const i_palette& aOther) const
    {
        return !(*this == aOther);
    }

    bool palette::has_color(color_role aRole) const
    {
        switch (aRole)
        {
        case color_role::Theme:
            return iThemeColor != std::nullopt;
        case color_role::Background:
            return iBackgroundColor != std::nullopt;
        case color_role::Foreground:
            return iForegroundColor != std::nullopt;
        case color_role::Base:
            return iBaseColor != std::nullopt;
        case color_role::AlternateBase:
            return iAlternateBaseColor != std::nullopt;
        case color_role::Text:
            return iTextColor != std::nullopt;
        case color_role::Selection:
            return iSelectionColor != std::nullopt;
        case color_role::AlternateSelection:
            return iAlternateSelectionColor != std::nullopt;
        case color_role::SelectedText:
            return iSelectedTextColor != std::nullopt;
        case color_role::Focus:
            return iFocusColor != std::nullopt;
        case color_role::Hover:
            return iHoverColor != std::nullopt;
        case color_role::PrimaryAccent:
            return iPrimaryAccentColor != std::nullopt;
        case color_role::SecondaryAccent:
            return iSecondaryAccentColor != std::nullopt;
        case color_role::Void:
            return iVoidColor != std::nullopt;
        default:
            return false;
        }
    }

    color palette::color(color_role aRole) const
    {
        switch (aRole)
        {
        case color_role::Theme:
            if (has_color(color_role::Theme))
                return iThemeColor->with_alpha(1.0);
            else if (has_proxy())
                return proxy().color(color_role::Theme);
            else
                return neogfx::color{ 0xEF, 0xEB, 0xE7 };
        case color_role::Background:
            if (has_color(color_role::Background))
                return *iBackgroundColor;
            else
                return color(color_role::Theme);
        case color_role::Foreground:
            if (has_color(color_role::Foreground))
                return *iForegroundColor;
            else
                return color(color_role::Theme);
        case color_role::Base:
            if (has_color(color_role::Base))
                return *iBaseColor;
            else
                return color(color_role::Theme).unshaded(0x1C);
        case color_role::AlternateBase:
            if (has_color(color_role::AlternateBase))
                return *iAlternateBaseColor;
            else
                return color(color_role::Base).mid(color(color_role::Theme));
        case color_role::Text:
            if (has_color(color_role::Text))
                return *iTextColor;
            else
                return color(color_role::Theme).to_hsl().lightness() < 0.6 ? color::White : color::Black;
        case color_role::Selection:
            if (has_color(color_role::Selection))
                return *iSelectionColor;
            else if (has_proxy())
                return proxy().color(color_role::Selection);
            else
                return neogfx::color{ 0x2A, 0x82, 0xDA };
        case color_role::AlternateSelection:
            if (has_color(color_role::AlternateSelection))
                return *iAlternateSelectionColor;
            else if (has_proxy())
                return proxy().color(color_role::AlternateSelection);
            else
                return neogfx::color{ 0xFE, 0xD8, 0xB1 };
        case color_role::SelectedText:
            if (has_color(color_role::SelectedText))
                return *iSelectedTextColor;
            else if (has_proxy())
                return proxy().color(color_role::SelectedText);
            else
                return color(color_role::Selection).to_hsl().lightness() < 0.6 ? color::White : color::Black;
        case color_role::Focus:
            if (has_color(color_role::Focus))
                return *iFocusColor;
            else
                return neogfx::color{ 0x0A, 0x84, 0xFF };;
        case color_role::Hover:
            if (has_color(color_role::Hover))
                return *iHoverColor;
            else
                return color(color_role::Selection).lighter(0x40);
        case color_role::PrimaryAccent:
            if (has_color(color_role::PrimaryAccent))
                return *iPrimaryAccentColor;
            else
                return color(color_role::Theme).same_lightness_as(color(color_role::Theme).light() ? neogfx::color{ 32, 32, 32 } : neogfx::color{ 224, 224, 224 });
        case color_role::SecondaryAccent:
            if (has_color(color_role::SecondaryAccent))
                return *iSecondaryAccentColor;
            else
                return color(color_role::Theme).same_lightness_as(color(color_role::Theme).light() ? neogfx::color{ 64, 64, 64 } : neogfx::color{ 192, 192, 192 });
        case color_role::Void:
            if (has_color(color_role::Void))
                return *iVoidColor;
            else
                return color(color_role::Theme).to_hsl().lightness() < 0.6 ? color::Black : color::White;
        default:
            if (has_proxy())
                return proxy().color(aRole);
            else
                return color(color_role::Theme);
        }
    }

    const optional_color& palette::maybe_color(color_role aRole) const
    {
        switch (aRole)
        {
        case color_role::Theme:
            return iThemeColor;
        case color_role::Background:
            return iBackgroundColor;
        case color_role::Foreground:
            return iForegroundColor;
        case color_role::Base:
            return iBaseColor;
        case color_role::AlternateBase:
            return iAlternateBaseColor;
        case color_role::Text:
            return iTextColor;
        case color_role::Selection:
            return iSelectionColor;
        case color_role::AlternateSelection:
            return iAlternateSelectionColor;
        case color_role::SelectedText:
            return iSelectedTextColor;
        case color_role::Focus:
            return iFocusColor;
        case color_role::Hover:
            return iHoverColor;
        case color_role::PrimaryAccent:
            return iPrimaryAccentColor;
        case color_role::SecondaryAccent:
            return iSecondaryAccentColor;
        case color_role::Void:
            return iVoidColor;
        default:
            return iThemeColor;
        }
    }

    void palette::set_color(color_role aRole, const optional_color& aColor)
    {
        auto const oldColor = maybe_color(aRole);
        switch (aRole)
        {
        case color_role::Theme:
            iThemeColor = aColor;
            break;
        case color_role::Background:
            iBackgroundColor = aColor;
            break;
        case color_role::Foreground:
            iForegroundColor = aColor;
            break;
        case color_role::Base:
            iBaseColor = aColor;
            break;
        case color_role::AlternateBase:
            iAlternateBaseColor = aColor;
            break;
        case color_role::Text:
            iTextColor = aColor;
            break;
        case color_role::Selection:
            iSelectionColor = aColor;
            break;
        case color_role::AlternateSelection:
            iAlternateSelectionColor = aColor;
            break;
        case color_role::SelectedText:
            iSelectedTextColor = aColor;
            break;
        case color_role::Focus:
            iFocusColor = aColor;
            break;
        case color_role::Hover:
            iHoverColor = aColor;
            break;
        case color_role::PrimaryAccent:
            iPrimaryAccentColor = aColor;
            break;
        case color_role::SecondaryAccent:
            iSecondaryAccentColor = aColor;
            break;
        case color_role::Void:
            iVoidColor = aColor;
            break;
        }
        if (maybe_color(aRole) != oldColor)
            Changed();
    }

    color palette::default_text_color_for_widget(const i_widget& aWidget) const
    {
        // work out the colour the text is actually drawn on: translucent widget colours are composited (front to back)
        // over those of their ancestors until an opaque colour is reached
        bool foundColor = false;
        scalar backdropRed = 0.0, backdropGreen = 0.0, backdropBlue = 0.0, coverage = 0.0;
        auto const add_layer = [&](neogfx::color const& aColor, scalar aAlpha)
        {
            auto const weight = (1.0 - coverage) * aAlpha;
            backdropRed += weight * aColor.red<scalar>();
            backdropGreen += weight * aColor.green<scalar>();
            backdropBlue += weight * aColor.blue<scalar>();
            coverage += weight;
        };
        for (i_widget const* w = &aWidget; w != nullptr && coverage < 1.0; w = w->has_parent() ? &w->parent() : nullptr)
        {
            // (a widget's base colour (e.g. a push button's face, a text edit's text area) is what its text is drawn on so it takes 
            // precedence over its background colour)
            if (w->has_base_color())
            {
                auto const baseColor = w->base_color();
                add_layer(baseColor, baseColor.alpha<scalar>());
                foundColor = true;
            }
            else if (w->has_background_color())
            {
                // (as painted by widget::paint_non_client)
                auto const backgroundColor = w->background_color();
                add_layer(backgroundColor, backgroundColor.alpha<scalar>() * (w->has_background_opacity() ? w->background_opacity() : 1.0));
                foundColor = true;
            }
        }
        auto const defaultTextColor = color(color_role::Text);
        if (!foundColor)
            return defaultTextColor;
        add_layer(color(color_role::Background), 1.0);
        neogfx::color const backdrop{ backdropRed, backdropGreen, backdropBlue, 1.0 };

        bool const lightText = contrast_ratio(neogfx::color::White, backdrop) >= LightTextMinimumContrast;
        neogfx::color const textColor = lightText ? neogfx::color::White : neogfx::color::Black;
        bool const defaultTextIsLight = relative_luminance(defaultTextColor) > relative_luminance(backdrop);
        if (defaultTextIsLight == lightText &&
            contrast_ratio(defaultTextColor, backdrop) >= std::min(DefaultTextMinimumContrast, contrast_ratio(textColor, backdrop)))
            return defaultTextColor;
        return textColor;
    }

    bool palette::has_proxy() const
    {
        return iProxy != std::nullopt;
    }

    const i_palette& palette::proxy() const
    {
        if (!has_proxy())
            throw no_proxy();
        else if (!proxy_ptr())
            return service<i_app>().current_style().palette();
        else
            return (*proxy_ptr());
    }

    const i_palette* palette::proxy_ptr() const
    {
        if (!has_proxy())
            throw no_proxy();
        else
            return *iProxy;
    }
}
