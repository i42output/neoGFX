// i_layout_item.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/layout/i_geometry.hpp>
#include <neogfx/gui/layout/i_anchorable_object.hpp>

namespace neogfx
{
    class i_layout;
    class i_widget;
    class i_layout_item_proxy;

    struct not_a_layout : std::logic_error { not_a_layout() : std::logic_error("neogfx::not_a_layout") {} };
    struct not_a_widget : std::logic_error { not_a_widget() : std::logic_error("neogfx::not_a_widget") {} };
    struct no_parent_layout : std::logic_error { no_parent_layout() : std::logic_error("neogfx::no_parent_layout") {} };
    struct no_layout_owner : std::logic_error { no_layout_owner() : std::logic_error("neogfx::no_layout_owner") {} };
    struct layout_item_not_found : std::logic_error { layout_item_not_found() : std::logic_error{ "neogfx::layout_item_not_found" } {} };

    class i_layout_item : public i_geometry, public i_anchorable_object
    {
    public:
        virtual ~i_layout_item() = default;
    public:
        virtual bool is_layout() const = 0;
        virtual const i_layout& as_layout() const = 0;
        virtual i_layout& as_layout() = 0;
        virtual bool is_widget() const = 0;
        virtual const i_widget& as_widget() const = 0;
        virtual i_widget& as_widget() = 0;
    public:
        virtual bool has_parent_layout() const = 0;
        virtual const i_layout& parent_layout() const = 0;
        virtual i_layout& parent_layout() = 0;
        virtual void set_parent_layout(i_layout* aParentLayout) = 0;
        virtual bool has_layout_owner() const = 0;
        virtual const i_widget& layout_owner() const = 0;
        virtual i_widget& layout_owner() = 0;
        virtual void set_layout_owner(i_widget* aOwner) = 0;
        virtual bool is_proxy() const = 0;
        virtual const i_layout_item_proxy& proxy_for_layout() const = 0;
        virtual i_layout_item_proxy& proxy_for_layout() = 0;
    public:
        virtual void layout_as(const point& aPosition, const size& aSize) = 0;
    public:
        virtual bool visible() const = 0;
    };
}
