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
        iSelectedTextColor{ aOther.maybe_color(color_role::SelectedText) },
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
            iSelectedTextColor = aOther.maybe_color(color_role::SelectedText);
            iHoverColor = aOther.maybe_color(color_role::Hover);
            iPrimaryAccentColor = aOther.maybe_color(color_role::PrimaryAccent);
            iSecondaryAccentColor = aOther.maybe_color(color_role::SecondaryAccent);
            iVoidColor = aOther.maybe_color(color_role::Void);
            Changed.trigger();
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
            iSelectedTextColor == aOther.maybe_color(color_role::SelectedText) &&
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
            iSelectedTextColor == aOther.maybe_color(color_role::SelectedText) &&
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
        case color_role::SelectedText:
            return iSelectedTextColor != std::nullopt;
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
        case color_role::SelectedText:
            if (has_color(color_role::SelectedText))
                return *iSelectedTextColor;
            else if (has_proxy())
                return proxy().color(color_role::SelectedText);
            else
                return color(color_role::Selection).to_hsl().lightness() < 0.6 ? color::White : color::Black;
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
        case color_role::SelectedText:
            return iSelectedTextColor;
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
        case color_role::SelectedText:
            iSelectedTextColor = aColor;
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
            Changed.trigger();
    }

    color palette::default_text_color_for_widget(const i_widget& aWidget) const
    {
        optional_color textColor;
        const i_widget* w = nullptr;
        do
        {
            if (w == nullptr)
                w = &aWidget;
            else
                w = &w->parent();
            if (w->has_background_color())
            {
                textColor = w->background_color().brightness() >= 0.509 ? color::Black : color::White;
                break;
            }
            else if (w->has_base_color())
            {
                textColor = w->base_color().brightness() >= 0.509 ? color::Black : color::White;
                break;
            }
        } while (w->has_parent());
        auto defaultTextColor = color(color_role::Text);
        if (textColor == std::nullopt || textColor->similar_intensity(defaultTextColor))
            textColor = defaultTextColor;
        return *textColor;
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