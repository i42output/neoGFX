// i_emoticon_translator.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2025 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/geometrical.hpp>
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
    class i_emoticon_translator : public i_service
    {
    public:
        virtual bool active() const = 0;
        virtual bool active(i_widget const& aWidget) const = 0;
        virtual void activate(i_widget& aWidget) = 0;
        virtual void deactivate() = 0;
    public:
        virtual void update_cursor_position(point const& aPosition) = 0;
    public:
        static uuid const& iid() { static uuid const sIid{ 0x989cf6ec, 0xbb5c, 0x4592, 0xb7a0, { 0x96, 0x6e, 0xa1, 0xa2, 0x41, 0xbe } }; return sIid; }
    };
}