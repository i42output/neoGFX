// drag_drop.hpp
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
#include <neogfx/app/i_drag_drop.hpp>

namespace neogfx::drag_drop
{
    class object : public i_object
    {
    public:
        object(object_type aType);
    public:
        object_type ddo_type() const override;
    private:
        object_type iType;
    };

    class source : public i_source
    {
    public:
        define_declared_event(DraggingObject, dragging_object, i_object const&)
        define_declared_event(DraggingCancelled, dragging_cancelled, i_object const&)
        define_declared_event(ObjectDropped, object_dropped, i_object const&)
    public:
        source();
    public:
        bool drag_drop_active() const override;
        void start_drag_drop() override;
        void end_drag_drop() override;
    };

    class target : public i_target
    {
    public:
        define_declared_event(ObjectDropped, object_dropped, i_object const&)
    public:
        target();
    public:
        bool can_accept(i_object const& aOobject) const override;
        void accept(i_object const& aOobject) override;
    public:
        bool is_widget() const override;
        i_widget const& as_widget() const override;
        i_widget& as_widget() override;
    };

    class drag_drop : public i_drag_drop
    {
    public:
        declare_event(source_registered, i_source&)
        declare_event(source_unregistered, i_source&)
        declare_event(target_registered, i_target&)
        declare_event(target_unregistered, i_target&)
    public:
        drag_drop();
    public:
        void register_source(i_source& aSource) override;
        void unregister_source(i_source& aSource) override;
        void register_target(i_target& aTarget) override;
        void unregister_target(i_target& aTarget) override;
    public:
        bool is_target_at(i_object const& aObject, i_surface const& aSurface, point const& aPosition) const override;
        i_target& target_at(i_object const& aObject, i_surface const& aSurface, point const& aPosition) override;
    };
}