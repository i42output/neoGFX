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
#include <neolib/task/i_event.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/widget/i_menu.hpp>
#include <neogfx/tools/DesignStudio/i_element_component.hpp>

namespace neogfx::DesignStudio
{
    class i_project;

    class i_element_caddy : public i_widget
    {
    public:
        virtual void start_drag(cardinal aPart, point const& aPosition) = 0;
        virtual void drag(point const& aPosition, bool aIgnoreConstraints) = 0;
        virtual void end_drag() = 0;
    };

    enum class element_group : uint32_t
    {
        Unknown,
        Perspective,
        Project,
        UserInterface,
        Code,
        Script,
        Node,
        App,
        Menu,
        Action,
        Widget,
        Layout,
        Workflow
    };

    class i_element_library;

    enum class element_mode
    {
        None,
        Drag,
        Edit
    };

    class i_element : public i_reference_counted
    {
    public:
        declare_event(mode_changed)
        declare_event(selection_changed)
        declare_event(context_menu, i_menu&)
    public:
        typedef i_element abstract_type;
    public:
        typedef neolib::i_vector<i_ref_ptr<i_element>> children_t;
    public:
        struct no_parent : std::logic_error { no_parent() : std::logic_error{ "neogfx::DesignStudio::i_element::no_parent" } {} };
        struct no_layout_item : std::logic_error { no_layout_item() : std::logic_error{ "neogfx::DesignStudio::i_element::no_layout_item" } {} };
        struct no_caddy : std::logic_error { no_caddy() : std::logic_error{ "neogfx::DesignStudio::i_element::no_caddy" } {} };
    public:
        virtual i_element_library const& library() const = 0;
        virtual i_project& project() const = 0;
        virtual element_group group() const = 0;
        virtual neolib::i_string const& type() const = 0;
        virtual neolib::i_string const& id() const = 0;
    public:
        virtual i_element const& root() const = 0;
        virtual i_element& root() = 0;
        virtual bool has_parent() const = 0;
        virtual i_element const& parent() const = 0;
        virtual i_element& parent() = 0;
        virtual void set_parent(i_element& aParent) = 0;
        virtual children_t const& children() const = 0;
        virtual children_t& children() = 0;
        virtual void add_child(i_element& aChild) = 0;
        virtual void remove_child(i_element& aChild) = 0;
    public:
        virtual void create_default_children() = 0;
    public:
        virtual bool needs_caddy() const = 0;
        virtual bool has_caddy() const = 0;
        virtual i_element_caddy& caddy() const = 0;
        virtual void set_caddy(i_element_caddy& aCaddy) = 0;
        virtual bool has_layout_item() const = 0;
        virtual void create_layout_item(i_widget& aParent) = 0;
        virtual i_layout_item& layout_item() const = 0;
    public:
        virtual element_mode mode() const = 0;
        virtual void set_mode(element_mode aMode) = 0;
        virtual bool is_selected() const = 0;
        virtual void select(bool aSelected = true, bool aDeselectRest = true) = 0;
    public:
        children_t::const_iterator begin() const { return children().begin(); }
        children_t::iterator begin() { return children().begin(); }
        children_t::const_iterator end() const { return children().end(); }
        children_t::iterator end() { return children().end(); }
    public:
        bool is_root() const
        {
            return !has_parent();
        }
        std::size_t selected_child_count() const
        {
            std::size_t result = 0;
            for (auto& child : children())
                child->visit([&](i_element& aChild) { if (aChild.is_selected()) ++result; });
            return result;
        }
        void visit(std::function<void(i_element&)> aVisitor)
        {
            aVisitor(*this);
            for (auto& child : children())
                child->visit(aVisitor);
        }
        void reverse_visit(std::function<void(i_element&)> aVisitor)
        {
            for (auto iterChild = children().rbegin(); iterChild != children().rend(); ++iterChild)
                (*iterChild)->visit(aVisitor);
            aVisitor(*this);
        }
    public:
        bool has_widget() const
        {
            return has_layout_item() && layout_item().is_widget();
        }
        i_widget& widget() const
        {
            return layout_item().as_widget();
        }
    };

    template <typename T>
    struct element_traits
    {
        typedef i_element base;
        static constexpr bool needsCaddy = true;
    };

    template <typename Type>
    inline void create_default_children(i_element& aParent)
    {
        // specialize
    }
}
