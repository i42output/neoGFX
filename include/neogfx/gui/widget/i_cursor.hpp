// i_cursor.hpp
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

    class i_cursor
    {
    public:
        declare_event(position_changed)
        declare_event(anchor_changed)
        declare_event(appearance_changed)
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
        using position_type = std::size_t;
    public:
        struct no_document : std::logic_error { no_document() : std::logic_error("neogfx::cursor::no_document") {} };
    public:
        virtual ~i_cursor() = default;
    public:
        virtual bool has_document() const = 0;
        virtual i_document& document() const = 0;
        virtual void move(move_operation_e aMoveOperation) = 0;
        virtual position_type position() const = 0;
        virtual void set_position(position_type aPosition, bool aMoveAnchor = true) = 0;
        virtual position_type anchor() const = 0;
        virtual void set_anchor(position_type aAnchor) = 0;
        virtual bool anchored_over_entire_document() const = 0;
        virtual const color_or_gradient& color() const = 0;
        virtual void set_color(const color_or_gradient& aColor) = 0;
        virtual cursor_style style() const = 0;
        virtual void set_style(cursor_style aStyle) = 0;
        virtual void clear_style() = 0;
        virtual dimension width(i_units_context const& aContext, std::optional<scalar> const& aGlyphWidth = {}) const = 0;
        virtual void set_width(dimension aWidth) = 0;
        virtual void clear_wdith() = 0;
        virtual std::chrono::milliseconds flash_interval() const = 0;
        virtual void set_flash_interval(std::chrono::milliseconds aInterval) = 0;
        virtual bool visible() const = 0;
        virtual bool hidden() const = 0;
        virtual void show() = 0;
        virtual void hide() = 0;
    };
}