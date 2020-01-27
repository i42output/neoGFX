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
        iWidgetDetailSecondaryColor{ color::Goldenrod }
    {
    }

    palette::palette(const i_palette& aOther) :
        iColor{ aOther.has_color() ? aOther.color() : optional_color{} },
        iBackgroundColor{ aOther.has_background_color() ? aOther.background_color() : optional_color{} },
        iForegroundColor{ aOther.has_foreground_color() ? aOther.foreground_color() : optional_color{} },
        iTextColor{ aOther.has_text_color() ? aOther.text_color() : optional_color{} },
        iSelectionColor{ aOther.has_selection_color() ? aOther.selection_color() : optional_color{} },
        iHoverColor{ aOther.has_hover_color() ? aOther.hover_color() : optional_color{} },
        iWidgetDetailPrimaryColor{ aOther.has_widget_detail_primary_color() ? aOther.widget_detail_primary_color() : optional_color{} },
        iWidgetDetailSecondaryColor{ aOther.has_widget_detail_secondary_color() ? aOther.widget_detail_secondary_color() : optional_color{} }
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
            iColor = aOther.has_color() ? aOther.color() : optional_color{};
            iBackgroundColor = aOther.has_background_color() ? aOther.background_color() : optional_color{};
            iForegroundColor = aOther.has_foreground_color() ? aOther.foreground_color() : optional_color{};
            iTextColor = aOther.has_text_color() ? aOther.text_color() : optional_color{};
            iSelectionColor = aOther.has_selection_color() ? aOther.selection_color() : optional_color{};
            iHoverColor = aOther.has_hover_color() ? aOther.hover_color() : optional_color{};
            iWidgetDetailPrimaryColor = aOther.has_widget_detail_primary_color() ? aOther.widget_detail_primary_color() : optional_color{};
            iWidgetDetailSecondaryColor = aOther.has_widget_detail_secondary_color() ? aOther.widget_detail_secondary_color() : optional_color{};
            Changed.trigger();
        }
        return *this;
    }

    bool palette::operator==(const i_palette& aOther) const
    {
        return has_color() == aOther.has_color() &&
            color() == aOther.color() &&
            has_background_color() == aOther.has_background_color() &&
            background_color() == aOther.background_color() &&
            has_foreground_color() == aOther.has_foreground_color() &&
            foreground_color() == aOther.foreground_color() &&
            has_text_color() == aOther.has_text_color() &&
            text_color() == aOther.text_color() &&
            has_selection_color() == aOther.has_selection_color() &&
            selection_color() == aOther.selection_color() &&
            has_hover_color() == aOther.has_hover_color() &&
            hover_color() == aOther.hover_color() &&
            has_widget_detail_primary_color() == aOther.has_widget_detail_primary_color() &&
            widget_detail_primary_color() == aOther.widget_detail_primary_color() &&
            has_widget_detail_secondary_color() == aOther.has_widget_detail_secondary_color() &&
            widget_detail_secondary_color() == aOther.widget_detail_secondary_color();
    }

    bool palette::operator!=(const i_palette& aOther) const
    {
        return !(*this == aOther);
    }

    bool palette::has_color() const
    {
        return iColor != std::nullopt;
    }

    color palette::color() const
    {
        if (has_color())
            return iColor->with_alpha(0xFF);
        return neogfx::color{ 0xEF, 0xEB, 0xE7 };
    }

    void palette::set_color(const optional_color& aColor)
    {
        if (iColor != aColor)
        {
            iColor = aColor;
            Changed.trigger();
        }
    }

    bool palette::has_background_color() const
    {
        return iBackgroundColor != std::nullopt;
    }

    color palette::background_color() const
    {
        if (has_background_color())
            return *iBackgroundColor;
        else
            return color().light() ? color().lighter(0x20) : color().darker(0x20);
    }

    void palette::set_background_color(const optional_color& aBackgroundColor)
    {
        if (iBackgroundColor != aBackgroundColor)
        {
            iBackgroundColor = aBackgroundColor;
            Changed.trigger();
        }
    }

    bool palette::has_foreground_color() const
    {
        return iForegroundColor != std::nullopt;
    }

    color palette::foreground_color() const
    {
        if (has_foreground_color())
            return *iForegroundColor;
        else
            return color().light() ? color().darker(0x20) : color().lighter(0x20);
    }

    void palette::set_foreground_color(const optional_color& aForegroundColor)
    {
        if (iForegroundColor != aForegroundColor)
        {
            iForegroundColor = aForegroundColor;
            Changed.trigger();
        }
    }

    bool palette::has_text_color() const
    {
        return iTextColor != std::nullopt;
    }

    color palette::text_color() const
    {
        if (has_text_color())
            return *iTextColor;
        else
        {
            if (color().to_hsl().lightness() < 0.6)
                return color::White;
            else
                return color::Black;
        }
    }

    void palette::set_text_color(const optional_color& aTextColor)
    {
        if (iTextColor != aTextColor)
        {
            iTextColor = aTextColor;
            Changed.trigger();
        }
    }

    bool palette::has_selection_color() const
    {
        return iTextColor != std::nullopt;
    }

    color palette::selection_color() const
    {
        if (has_selection_color())
            return *iTextColor;
        else
            return neogfx::color{ 0x2A, 0x82, 0xDA };
    }

    void palette::set_selection_color(const optional_color& aSelectionColor)
    {
        if (iSelectionColor != aSelectionColor)
        {
            iSelectionColor = aSelectionColor;
            Changed.trigger();
        }
    }

    bool palette::has_hover_color() const
    {
        return iHoverColor != std::nullopt;
    }

    color palette::hover_color() const
    {
        if (has_hover_color())
            return *iHoverColor;
        else
            return selection_color().lighter(0x40);
    }

    void palette::set_hover_color(const optional_color& aHoverColor)
    {
        if (iHoverColor != aHoverColor)
        {
            iHoverColor = aHoverColor;
            Changed.trigger();
        }
    }

    bool palette::has_widget_detail_primary_color() const
    {
        return iWidgetDetailPrimaryColor != std::nullopt;
    }

    color palette::widget_detail_primary_color() const
    {
        if (has_widget_detail_primary_color())
            return *iWidgetDetailPrimaryColor;
        else
            return color().same_lightness_as(color().light() ? neogfx::color{ 32, 32, 32 } : neogfx::color{ 224, 224, 224 });
    }

    void palette::set_widget_detail_primary_color(const optional_color& aWidgetDetailPrimaryColor)
    {
        if (iWidgetDetailPrimaryColor != aWidgetDetailPrimaryColor)
        {
            iWidgetDetailPrimaryColor = aWidgetDetailPrimaryColor;
            Changed.trigger();
        }
    }

    bool palette::has_widget_detail_secondary_color() const
    {
        return iWidgetDetailSecondaryColor != std::nullopt;
    }

    color palette::widget_detail_secondary_color() const
    {
        if (has_widget_detail_secondary_color())
            return *iWidgetDetailSecondaryColor;
        else
            return color().same_lightness_as(color().light() ? neogfx::color{ 64, 64, 64 } : neogfx::color{ 192, 192, 192 });
    }

    void palette::set_widget_detail_secondary_color(const optional_color& aWidgetDetailSecondaryColor)
    {
        if (iWidgetDetailSecondaryColor != aWidgetDetailSecondaryColor)
        {
            iWidgetDetailSecondaryColor = aWidgetDetailSecondaryColor;
            Changed.trigger();
        }
    }
}