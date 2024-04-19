// native_widget.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/widget/i_native_widget.hpp>

namespace neogfx
{
    template <typename Interface = i_widget>
    class native_widget : public widget<Interface>
    {
        meta_object(widget<Interface>)
        // events
    public:
        // types
    public:
        // native_widget
    public:
        native_widget(i_widget& aParent);
        ~native_widget();
        // widget
    public:
        using base_type::as_widget;
        // widget
    private:
        void init();
    private:
        sink iSink;
    };
}
