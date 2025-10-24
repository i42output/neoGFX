// i_tab_page.hpp
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
#include "i_widget.hpp"

namespace neogfx
{
    class i_tab;

    class i_tab_page : public i_reference_counted
    {
    public:
        typedef i_tab_page abstract_type;
    public:
        declare_event(selected)
        declare_event(deselected)
    public:
        virtual ~i_tab_page() = default;
    public:
        virtual const i_tab& tab() const = 0;
        virtual i_tab& tab() = 0;
        virtual const i_widget& as_widget() const = 0;
        virtual i_widget& as_widget() = 0;
    public:
        virtual void* data() const = 0;
        virtual void set_data(void* aData) = 0;
    };
}