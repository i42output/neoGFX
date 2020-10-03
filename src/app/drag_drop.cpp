// drag_drop.cpp
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
#include <neogfx/app/drag_drop.hpp>

namespace neogfx
{
    drag_drop_object::drag_drop_object(drag_drop_object_type aType) : 
        iType{ aType }
    {
    }

    drag_drop_object_type drag_drop_object::ddo_type() const
    {
        return iType;
    }


    drag_drop_source::drag_drop_source()
    {
    }

    bool drag_drop_source::drag_drop_active() const
    {
        throw std::logic_error("not yet implemented");
    }

    void drag_drop_source::start_drag_drop()
    {
        throw std::logic_error("not yet implemented");
    }

    void drag_drop_source::end_drag_drop()
    {
        throw std::logic_error("not yet implemented");
    }


    drag_drop_target::drag_drop_target()
    {
    }

    bool drag_drop_target::can_accept(i_drag_drop_object const& aOobject) const
    {
        throw std::logic_error("not yet implemented");
    }

    void drag_drop_target::accept(i_drag_drop_object const& aOobject)
    {
        throw std::logic_error("not yet implemented");
    }

    bool drag_drop_target::is_widget() const
    {
        return false;
    }

    i_widget const& drag_drop_target::as_widget() const
    {
        throw not_a_widget();
    }

    i_widget& drag_drop_target::as_widget()
    {
        throw not_a_widget();
    }


    drag_drop::drag_drop()
    {
    }

    void drag_drop::register_source(i_drag_drop_source& aSource)
    {
        throw std::logic_error("not yet implemented");
    }

    void drag_drop::unregister_source(i_drag_drop_source& aSource)
    {
        throw std::logic_error("not yet implemented");
    }

    void drag_drop::register_target(i_drag_drop_target& aTarget)
    {
        throw std::logic_error("not yet implemented");
    }

    void drag_drop::unregister_target(i_drag_drop_target& aTarget)
    {
        throw std::logic_error("not yet implemented");
    }

    bool drag_drop::is_target_at(i_drag_drop_object const& aObject, i_surface const& aSurface, point const& aPosition) const
    {
        throw std::logic_error("not yet implemented");
    }

    i_drag_drop_target& drag_drop::target_at(i_drag_drop_object const& aObject, i_surface const& aSurface, point const& aPosition)
    {
        throw std::logic_error("not yet implemented");
    }
}