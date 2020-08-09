// cursor.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neolib/core/variant.hpp>
#include <neogfx/core/event.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/gfx/primitives.hpp>

namespace neogfx
{
    class i_document;

    enum class cursor_style
    {
        Standard,
        Xor
    };

    class cursor
    {
    public:
        define_event(PositionChanged, position_changed)
        define_event(AnchorChanged, anchor_changed)
        define_event(AppearanceChanged, appearance_changed)
    public:
        enum move_operation_e
        {
            None,
            StartOfDocument,
            StartOfParagraph,
            StartOfLine,
            StartOfWord,
            EndOfDocument,
            EndOfParagraph,
            EndOfLine,
            EndOfWord,
            PreviousParagraph, 
            PreviousLine,
            PreviousWord,
            PreviousCharacter,
            NextParagraph,
            NextLine,
            NextWord,
            NextCharacter,
            Up,
            Down,
            Left,
            Right
        };
        typedef std::size_t position_type;
    public:
        struct no_document : std::logic_error { no_document() : std::logic_error("neogfx::cursor::no_document") {} };
    public:
        cursor();
        cursor(i_document& aDocument);
    public:
        bool has_document() const;
        i_document& document() const;
        void move(move_operation_e aMoveOperation);
        position_type position() const;
        void set_position(position_type aPosition, bool aMoveAnchor = true);
        position_type anchor() const;
        void set_anchor(position_type aAnchor);
        const color_or_gradient& color() const;
        void set_color(const color_or_gradient& aColor);
        cursor_style style() const;
        void set_style(cursor_style aStyle);
        dimension width() const;
        void set_width(dimension aWidth);
        std::chrono::milliseconds flash_interval() const;
        void set_flash_interval(std::chrono::milliseconds aInterval);
    private:
        i_document* iDocument;
        position_type iPosition;
        position_type iAnchor;
        color_or_gradient iColor;
        cursor_style iStyle;
        dimension iWidth;
        std::chrono::milliseconds iFlashInterval;
    };
}