// i_model.hpp
/*
neogfx C++ App/Game Engine
Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.

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
#include <neogfx/core/i_event.hpp>

namespace neogfx
{
    class i_controller;

    class i_model
    {
    public:
        declare_event(modified)
        declare_event(controller_added, i_controller&)
        declare_event(controller_removed, i_controller&)
    public:
        virtual ~i_model() = default;
    public:
        virtual bool dirty() const = 0;
        virtual void set_dirty() = 0;
        virtual void set_clean() = 0;
        virtual void add_controller(i_controller& aController) = 0;
        virtual void remove_controller(i_controller& aController) = 0;
    };
}