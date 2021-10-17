// model.hpp
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
#include <neogfx/core/event.hpp>
#include <neogfx/gui/view/i_model.hpp>

namespace neogfx
{
    template <typename AbstractModel = i_model>
    class model : public AbstractModel
    {
        typedef AbstractModel base_type;
    public:
        typedef base_type abstract_type;
    public:
        define_declared_event(Modified, modified)
        define_declared_event(ControllerAdded, controller_added, i_controller&)
        define_declared_event(ControllerRemoved, controller_removed, i_controller&)
    public:
        model() :
            iDirty{ false }
        {
        }
    public:
        bool dirty() const override
        {
            return iDirty;
        }
        void set_dirty() override
        {
            if (!iDirty)
            {
                iDirty = true;
                Modified.trigger();
            }
        }
        void set_clean() override
        {
            if (iDirty)
                iDirty = false;
        }
        void add_controller(i_controller& aController) override
        {
            ControllerAdded.trigger(aController);
            // todo
        }
        void remove_controller(i_controller& aController) override
        {
            ControllerRemoved.trigger(aController);
            // todo
        }
    private:
        bool iDirty;
    };
}