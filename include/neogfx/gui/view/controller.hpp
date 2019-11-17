// controller.hpp
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
#include <neogfx/gui/view/i_controller.hpp>

namespace neogfx
{
    class controller : public i_controller
    {
    public:
        define_declared_event(ViewAdded, view_added, i_view&)
        define_declared_event(ViewRemoved, view_removed, i_view&)
    public:
        controller(i_model& aModel, i_view_container& aContainer);
    public:
        virtual const i_model& model() const;
        virtual i_model& model();
    public:
        virtual void add_view(i_view& aView);
        virtual void add_view(std::shared_ptr<i_view> aView);
        virtual void remove_view(i_view& aView);
        virtual bool only_weak_views() const;
    public:
        virtual const i_view_container& container() const;
        virtual i_view_container& container();
    private:
        i_model& iModel;
        i_view_container& iContainer;
        std::vector<std::shared_ptr<i_view>> iViews;
    };
}