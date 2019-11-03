// cursor.cpp
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
#include <neogfx/gui/widget/cursor.hpp>
#include <neogfx/gui/widget/i_document.hpp>

namespace neogfx
{
    cursor::cursor() :
        iDocument{}, iPosition{}, iAnchor{}, iStyle{ cursor_style::Standard }, iWidth{ 1.0 }, iFlashInterval{ 1000 }
    {
    }

    cursor::cursor(i_document& aDocument) :
        iDocument{&aDocument}, iPosition{}, iAnchor{}, iStyle{ cursor_style::Standard }, iWidth{1.0}, iFlashInterval{ 1000 }
    {
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
            PositionChanged.trigger();
        if (iAnchor != oldAnchor)
            AnchorChanged.trigger();
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
            AnchorChanged.trigger();
        }
    }

    const colour_or_gradient& cursor::colour() const
    {
        return iColour;
    }

    void cursor::set_colour(const colour_or_gradient& aColour)
    {
        if (iColour != aColour)
        {
            iColour = aColour;
            AppearanceChanged.trigger();
        }
    }

    cursor_style cursor::style() const
    {
        return iStyle;
    }

    void cursor::set_style(cursor_style aStyle)
    {
        iStyle = aStyle;
    }

    dimension cursor::width() const
    {
        return iWidth;
    }

    void cursor::set_width(dimension aWidth)
    {
        if (iWidth != aWidth)
        {
            iWidth = aWidth;
            AppearanceChanged.trigger();
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
}