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
#include <neogfx/gui/widget/i_cursor.hpp>

namespace neogfx
{
    class cursor : public i_cursor
    {
    public:
        define_declared_event(PositionChanged, position_changed)
        define_declared_event(AnchorChanged, anchor_changed)
        define_declared_event(AppearanceChanged, appearance_changed)
    public:
        cursor();
        cursor(i_document& aDocument);
    public:
        bool has_document() const final;
        i_document& document() const final;
        void move(move_operation_e aMoveOperation) final;
        position_type position() const final;
        void set_position(position_type aPosition, bool aMoveAnchor = true) final;
        position_type anchor() const final;
        void set_anchor(position_type aAnchor) final;
        bool anchored_over_entire_document() const final;
        const color_or_gradient& color() const final;
        void set_color(const color_or_gradient& aColor) final;
        cursor_style style() const final;
        void set_style(cursor_style aStyle) final;
        void clear_style() final;
        dimension width(i_units_context const& aContext, std::optional<scalar> const& aGlyphWidth = {}) const final;
        void set_width(dimension aWidth) final;
        void clear_wdith() final;
        std::chrono::milliseconds flash_interval() const final;
        void set_flash_interval(std::chrono::milliseconds aInterval) final;
        bool visible() const final;
        bool hidden() const final;
        void show() final;
        void hide() final;
    private:
        void init();
    private:
        i_document* iDocument;
        position_type iPosition;
        position_type iAnchor;
        color_or_gradient iColor;
        std::optional<cursor_style> iStyle;
        optional_dimension iWidth;
        std::chrono::milliseconds iFlashInterval;
        bool iVisible = true;
        sink iSink;
    };
}