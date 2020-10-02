// i_element.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2020 Leigh Johnston
  
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
#include <neolib/neolib.hpp>
#include <neolib/core/i_reference_counted.hpp>
#include <neolib/core/i_optional.hpp>
#include <neolib/core/i_string.hpp>
#include <neolib/core/i_vector.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/tools/DesignStudio/i_element_component.hpp>

namespace neogfx::DesignStudio
{
    enum class element_group : uint32_t
    {
        Unknown,
        Project,
        UserInterface,
        Code,
        Script,
        Node,
        App,
        Menu,
        Action,
        Widget,
        Layout
    };

    class i_element_library;

    class i_element : public i_reference_counted
    {
    public:
        typedef i_element abstract_type;
    public:
        typedef neolib::i_vector<i_ref_ptr<i_element>> children_t;
    public:
        struct no_parent : std::logic_error { no_parent() : std::logic_error{ "neogfx::DesignStudio::i_element::no_parent" } {} };
    public:
        virtual const i_element_library& library() const = 0;
        virtual element_group group() const = 0;
        virtual const neolib::i_string& type() const = 0;
        virtual const neolib::i_string& id() const = 0;
    public:
        virtual bool has_parent() const = 0;
        virtual const i_element& parent() const = 0;
        virtual i_element& parent() = 0;
        virtual void set_parent(i_element& aParent) = 0;
        virtual const children_t& children() const = 0;
        virtual children_t& children() = 0;
        virtual void add_child(i_element& aChild) = 0;
    public:
        children_t::const_iterator begin() const { return children().begin(); }
        children_t::iterator begin() { return children().begin(); }
        children_t::const_iterator end() const { return children().end(); }
        children_t::iterator end() { return children().end(); }
    public:

    };
}
