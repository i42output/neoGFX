// i_drag_drop.hpp
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
#include <neogfx/hid/i_surface.hpp>
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx::drag_drop
{
    typedef uuid object_type;

    const object_type FileList = { 0xfaa77f8e, 0xfabc, 0x413c, 0xacdb, { 0x5b, 0xc1, 0x74, 0xc5, 0xd2, 0xbf } };

    class i_object
    {
    public:
        virtual ~i_object() = default;
    public:
        virtual object_type ddo_type() const = 0;
    };

    class i_source
    {
    public:
        declare_event(dragging_object, i_object const&)
        declare_event(dragging_cancelled, i_object const&)
        declare_event(object_dropped, i_object const&)
    public:
        virtual ~i_source() = default;
    public:
        virtual bool drag_drop_active() const = 0;
        virtual void start_drag_drop() = 0;
        virtual void end_drag_drop() = 0;
    };

    class i_target
    {
    public:
        declare_event(object_dropped, i_object const&)
    public:
        virtual ~i_target() = default;
    public:
        virtual bool can_accept(i_object const& aOobject) const = 0;
        virtual void accept(i_object const& aOobject) = 0;
    public:
        virtual bool is_widget() const = 0;
        virtual i_widget const& as_widget() const = 0;
        virtual i_widget& as_widget() = 0;
    };

    class i_drag_drop
    {
    public:
        declare_event(source_registered, i_source&)
        declare_event(source_unregistered, i_source&)
        declare_event(target_registered, i_target&)
        declare_event(target_unregistered, i_target&)
    public:
        virtual ~i_drag_drop() = default;
    public:
        virtual void register_source(i_source& aSource) = 0;
        virtual void unregister_source(i_source& aSource) = 0;
        virtual void register_target(i_target& aTarget) = 0;
        virtual void unregister_target(i_target& aTarget) = 0;
    public:
        virtual bool is_target_at(i_object const& aObject, i_surface const& aSurface, point const& aPosition) const = 0;
        virtual i_target& target_at(i_object const& aObject, i_surface const& aSurface, point const& aPosition) = 0;
    };
}