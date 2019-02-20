// i_controller.hpp
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
#include <neogfx/core/event.hpp>

namespace neogfx
{
    class i_model;
    class i_view;

    class i_view_container;

    class i_controller
    {
    public:
        event<i_view&> view_added;
        event<i_view&> view_removed;
    public:
        struct view_not_found : std::logic_error { view_not_found() : std::logic_error("neogfx::i_controller::view_not_found") {} };
    public:
        virtual const i_model& model() const = 0;
        virtual i_model& model() = 0;
    public:
        virtual void add_view(i_view& aView) = 0;
        virtual void add_view(std::shared_ptr<i_view> aView) = 0;
        virtual void remove_view(i_view& aView) = 0;
        virtual bool only_weak_views() const = 0;
    public:
        virtual const i_view_container& container() const = 0;
        virtual i_view_container& container() = 0;
    };
}