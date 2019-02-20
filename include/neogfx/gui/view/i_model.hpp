// i_model.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

namespace neogfx
{
    class i_controller;

    class i_model
    {
    public:
        event<> modified;
    public:
        virtual bool dirty() const = 0;
        virtual bool controller_registered(i_controller& aController) = 0;
        virtual void add_controller(i_controller& aContainer) = 0;
        virtual void remove_controller(i_controller& aContainer) = 0;
    };
}