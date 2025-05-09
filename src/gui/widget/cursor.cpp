// cursor.cpp
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

#include <neogfx/app/i_accessibility.hpp>
#include <neogfx/hid/i_keyboard.hpp>
#include <neogfx/gui/widget/cursor.hpp>
#include <neogfx/gui/widget/i_document.hpp>

namespace neogfx
{
    cursor::cursor() :
        iDocument{}, iPosition{}, iAnchor{}, iFlashInterval{ 1000 }
    {
        init();
    }

    cursor::cursor(i_document& aDocument) :
        iDocument{&aDocument}, iPosition{}, iAnchor{}, iFlashInterval{ 1000 }
    {
        init();
    }

    cursor::position_type cursor::position() const
    {
        return iPosition;
    }

    bool cursor::has_document() const
    {
        return iDocument != nullptr;
    }

    i_document& cursor::document() const
    {
        if (!has_document())
            throw no_document();
        return *iDocument;
    }

    void cursor::move(move_operation_e aMoveOperation)
    {
        document().move_cursor(aMoveOperation);
    }

    void cursor::set_position(position_type aPosition, bool aMoveAnchor)
    {
        auto oldPosition = iPosition;
        iPosition = aPosition;
        auto oldAnchor = iAnchor;
        if (aMoveAnchor)
            iAnchor = aPosition;
        if (iPosition != oldPosition)
            PositionChanged();
        if (iAnchor != oldAnchor)
            AnchorChanged();
    }

    cursor::position_type cursor::anchor() const
    {
        return iAnchor;
    }

    void cursor::set_anchor(position_type aAnchor)
    {
        if (iAnchor != aAnchor)
        {
            iAnchor = aAnchor;
            AnchorChanged();
        }
    }

    bool cursor::anchored_over_entire_document() const
    {
        return (position() == 0 && anchor() == document().document_length()) ||
            (position() == document().document_length() && anchor() == 0);
    }

    const color_or_gradient& cursor::color() const
    {
        return iColor;
    }

    void cursor::set_color(const color_or_gradient& aColor)
    {
        if (iColor != aColor)
        {
            iColor = aColor;
            AppearanceChanged();
        }
    }

    cursor_style cursor::style() const
    {
        if (iStyle.has_value())
            return iStyle.value();
        if ((service<i_keyboard>().locks() & keyboard_locks::InsertLock) != keyboard_locks::InsertLock &&
            has_document() && document().overwrite_cursor_available())
            return cursor_style::Xor;
        return cursor_style::Standard;
    }

    void cursor::set_style(cursor_style aStyle)
    {
        if (iStyle != aStyle)
        {
            iStyle = aStyle;
            AppearanceChanged();
        }
    }

    void cursor::clear_style()
    {
        if (iStyle != std::nullopt)
        {
            iStyle = std::nullopt;
            AppearanceChanged();
        }
    }

    dimension cursor::width(i_units_context const& aContext, std::optional<scalar> const& aGlyphWidth) const
    {
        if (iWidth.has_value())
            return iWidth.value();
        auto const defaultWidth = aContext.dpi_scale(static_cast<dimension>(service<i_accessibility>().text_cursor_width()));
        if ((service<i_keyboard>().locks() & keyboard_locks::InsertLock) != keyboard_locks::InsertLock &&
            has_document() && document().overwrite_cursor_available())
        {
            if (aGlyphWidth.has_value())
                return std::max(aGlyphWidth.value(), defaultWidth);
            else
                return defaultWidth * 2;
        }
        return defaultWidth;
    }

    void cursor::set_width(dimension aWidth)
    {
        if (iWidth != aWidth)
        {
            iWidth = aWidth;
            AppearanceChanged();
        }
    }

    void cursor::clear_wdith()
    {
        if (iWidth != std::nullopt)
        {
            iWidth = std::nullopt;
            AppearanceChanged();
        }
    }

    std::chrono::milliseconds cursor::flash_interval() const
    {
        return iFlashInterval;
    }

    void cursor::set_flash_interval(std::chrono::milliseconds aInterval)
    {
        iFlashInterval = aInterval;
    }

    bool cursor::visible() const
    {
        return iVisible;
    }

    bool cursor::hidden() const
    {
        return !iVisible;
    }

    void cursor::show()
    {
        iVisible = true;
    }

    void cursor::hide()
    {
        iVisible = false;
    }

    void cursor::init()
    {
        iSink = service<i_accessibility>().settings_changed([&]()
        {
            if (!iWidth.has_value())
                AppearanceChanged();
        });
    }
}