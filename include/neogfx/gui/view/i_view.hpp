// i_view.hpp
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
#include <neogfx/gui/widget/i_widget.hpp>
#include "i_model.hpp"

namespace neogfx
{
    class i_view
    {
    public:
        declare_event(activated)
        declare_event(deactivated)
    public:
        virtual const i_widget& as_widget() const = 0;
        virtual i_widget& as_widget() = 0;
    public:
        virtual const i_model& model() const = 0;
        virtual i_model& model() = 0;
        virtual void update() = 0;
    public:
        virtual bool is_strong() const = 0;
        virtual bool is_weak() const = 0;
        virtual bool is_active() const = 0;
        virtual void activate() = 0;
        virtual void deactivate() = 0;
    };
}