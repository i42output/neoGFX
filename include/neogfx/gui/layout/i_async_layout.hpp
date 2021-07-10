// i_async_layout.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2021 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
    class i_async_layout : public i_service
    {
    public:
        virtual ~i_async_layout() = default;
    public:
        virtual bool exists(i_widget& aWidget) const noexcept = 0;
        virtual bool defer_layout(i_widget& aWidget) = 0;
        virtual void validate(i_widget& aWidget) = 0;
        virtual void invalidate(i_widget& aWidget) = 0;
    public:
        static uuid const& iid() { static uuid const sIid{ 0x3e50d155, 0xa8a, 0x4867, 0xacf8, { 0x42, 0xab, 0xd3, 0x34, 0xc0, 0xab } }; return sIid; }
    };
}