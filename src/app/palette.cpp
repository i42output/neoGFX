// palette.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/app/palette.hpp>

namespace neogfx
{
    palette::palette() : 
        iSecondaryAccentColor{ color::Goldenrod }
    {
    }

    palette::palette(const i_palette& aOther) :
        iThemeColor{ aOther.maybe_color(color_role::Theme) },
        iBackgroundColor{ aOther.maybe_color(color_role::Background) },
        iForegroundColor{ aOther.maybe_color(color_role::Foreground) },
        iTextColor{ aOther.maybe_color(color_role::Text) },
        iSelectionColor{ aOther.maybe_color(color_role::Selection) },
        iHoverColor{ aOther.maybe_color(color_role::Hover) },
        iPrimaryAccentColor{ aOther.maybe_color(color_role::PrimaryAccent) },
        iSecondaryAccentColor{ aOther.maybe_color(color_role::SecondaryAccent) }
    {
    }

    palette::palette(const palette& aOther) :
        palette(static_cast<const i_palette&>(aOther))
    {
    }

    palette& palette::operator=(const i_palette& aOther)
    {
        if (*this != aOther)
        {
            iThemeColor = aOther.maybe_color(color_role::Theme);
            iBackgroundColor = aOther.maybe_color(color_role::Background);
            iForegroundColor = aOther.maybe_color(color_role::Foreground);
            iTextColor = aOther.maybe_color(color_role::Text);
            iSelectionColor = aOther.maybe_color(color_role::Selection);
            iHoverColor = aOther.maybe_color(color_role::Hover);
            iPrimaryAccentColor = aOther.maybe_color(color_role::PrimaryAccent);
            iSecondaryAccentColor = aOther.maybe_color(color_role::SecondaryAccent);
            Changed.trigger();
        }
        return *this;
    }

    bool palette::operator==(const i_palette& aOther) const
    {
        return
            iThemeColor == aOther.maybe_color(color_role::Theme) &&
            iBackgroundColor == aOther.maybe_color(color_role::Background) &&
            iForegroundColor == aOther.maybe_color(color_role::Foreground) &&
            iTextColor == aOther.maybe_color(color_role::Text) &&
            iSelectionColor == aOther.maybe_color(color_role::Selection) &&
            iHoverColor == aOther.maybe_color(color_role::Hover) &&
            iPrimaryAccentColor == aOther.maybe_color(color_role::PrimaryAccent) &&
            iSecondaryAccentColor == aOther.maybe_color(color_role::SecondaryAccent);
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
        case color_role::Text:
            return iTextColor != std::nullopt;
        case color_role::Selection:
            return iSelectionColor != std::nullopt;
        case color_role::Hover:
            return iHoverColor != std::nullopt;
        case color_role::PrimaryAccent:
            return iPrimaryAccentColor != std::nullopt;
        case color_role::SecondaryAccent:
            return iSecondaryAccentColor != std::nullopt;
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
            else
                return neogfx::color{ 0xEF, 0xEB, 0xE7 };
        case color_role::Background:
            if (has_color(color_role::Background))
                return *iBackgroundColor;
            else
                return color(color_role::Theme).unshade(0x20);
        case color_role::Foreground:
            if (has_color(color_role::Foreground))
                return *iForegroundColor;
            else
                return color(color_role::Theme).shade(0x20);
        case color_role::Text:
            if (has_color(color_role::Text))
                return *iTextColor;
            else
            {
                if (color(color_role::Theme).to_hsl().lightness() < 0.6)
                    return color::White;
                else
                    return color::Black;
            }
        case color_role::Selection:
            if (has_color(color_role::Selection))
                return *iSelectionColor;
            else
                return neogfx::color{ 0x2A, 0x82, 0xDA };
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
        default:
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
        case color_role::Text:
            return iTextColor;
        case color_role::Selection:
            return iSelectionColor;
        case color_role::Hover:
            return iHoverColor;
        case color_role::PrimaryAccent:
            return iPrimaryAccentColor;
        case color_role::SecondaryAccent:
            return iSecondaryAccentColor;
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
        case color_role::Text:
            iTextColor = aColor;
            break;
        case color_role::Selection:
            iSelectionColor = aColor;
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
        }
        if (maybe_color(aRole) != oldColor)
            Changed.trigger();
    }
}