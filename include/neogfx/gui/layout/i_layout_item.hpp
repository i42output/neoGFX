// i_layout_item.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/i_property.hpp>
#include <neogfx/gui/layout/anchor.hpp>
#include <neogfx/gui/layout/anchorable.hpp>
#include <neogfx/gui/layout/i_geometry.hpp>
#include <neogfx/gui/layout/i_anchorable.hpp>

namespace neogfx
{
    class i_layout;
    class i_spacer;
    class i_widget;
    class i_layout_item_cache;

    struct not_a_layout : std::logic_error { not_a_layout() : std::logic_error("neogfx::not_a_layout") {} };
    struct not_a_widget : std::logic_error { not_a_widget() : std::logic_error("neogfx::not_a_widget") {} };
    struct not_a_spacer : std::logic_error { not_a_spacer() : std::logic_error("neogfx::not_a_spacer") {} };
    struct no_parent_layout : std::logic_error { no_parent_layout() : std::logic_error("neogfx::no_parent_layout") {} };
    struct no_parent_layout_item : std::logic_error { no_parent_layout_item() : std::logic_error("neogfx::no_parent_layout_item") {} };
    struct no_parent_widget : std::logic_error { no_parent_widget() : std::logic_error("neogfx::no_parent_widget") {} };
    struct no_layout_manager : std::logic_error { no_layout_manager() : std::logic_error("neogfx::no_layout_manager") {} };
    struct layout_item_not_found : std::logic_error { layout_item_not_found() : std::logic_error{ "neogfx::layout_item_not_found" } {} };
    struct ancestor_layout_type_not_found : std::logic_error { ancestor_layout_type_not_found() : std::logic_error{ "neogfx::ancestor_layout_type_not_found" } {} };
    struct cannot_fix_weightings : std::logic_error { cannot_fix_weightings() : std::logic_error{ "neogfx::cannot_fix_weightings" } {} };

    enum class layout_item_category
    {
        Unspecified,
        Layout,
        Spacer,
        Widget
    };

    class i_layout_item : public i_reference_counted, public i_property_owner, public i_geometry, public i_anchorable
    {
        friend class layout_item_cache;
    public:
        typedef i_layout_item abstract_type;
    public:
        virtual ~i_layout_item() = default;
    public:
        virtual const i_string& id() const = 0;
        virtual void set_id(const i_string& aId) = 0;
    public:
        virtual bool is_cache() const = 0;
    public:
        virtual bool is_layout() const = 0;
        virtual const i_layout& as_layout() const = 0;
        virtual i_layout& as_layout() = 0;
        virtual bool is_spacer() const = 0;
        virtual const i_spacer& as_spacer() const = 0;
        virtual i_spacer& as_spacer() = 0;
        virtual bool is_widget() const = 0;
        virtual const i_widget& as_widget() const = 0;
        virtual i_widget& as_widget() = 0;
    public:
        virtual bool has_parent_layout_item() const = 0;
        virtual const i_layout_item& parent_layout_item() const = 0;
        virtual i_layout_item& parent_layout_item() = 0;
    public:
        virtual bool has_parent_layout() const = 0;
        virtual const i_layout& parent_layout() const = 0;
        virtual i_layout& parent_layout() = 0;
        virtual void set_parent_layout(i_layout* aParentLayout) = 0;
        virtual bool has_parent_widget() const = 0;
        virtual const i_widget& parent_widget() const = 0;
        virtual i_widget& parent_widget() = 0;
        virtual void set_parent_widget(i_widget* aParentWidget) = 0;
        virtual bool has_layout_manager() const = 0;
        virtual const i_widget& layout_manager() const = 0;
        virtual i_widget& layout_manager() = 0;
    public:
        virtual void update_layout(bool aDeferLayout = true, bool aAncestors = false) = 0;
        virtual void layout_as(const point& aPosition, const size& aSize) = 0;
    public:
        virtual void invalidate_combined_transformation() = 0;
        virtual void fix_weightings(bool aRecalculate = true) = 0;
    public:
        virtual void layout_item_enabled(i_layout_item& aItem) = 0;
        virtual void layout_item_disabled(i_layout_item& aItem) = 0;
    public:
        virtual bool visible() const = 0;
    public:
        template <typename LayoutType>
        bool has_ancestor_layout() const
        {
            return find_ancestor_layout<LayoutType>() != nullptr;
        }
        template <typename LayoutType>
        LayoutType* find_ancestor_layout()
        {
            if (has_parent_layout())
            {
                auto existing = dynamic_cast<LayoutType*>(&parent_layout());
                if (existing != nullptr)
                    return existing;
                return parent_layout().template find_ancestor_layout<LayoutType>();
            }
            return nullptr;
        }
        template <typename LayoutType>
        LayoutType& ancestor_layout()
        {
            auto existing = find_ancestor_layout<LayoutType>();
            if (existing != nullptr)
                return *existing;
            throw ancestor_layout_type_not_found();
        }
        bool same_parent_widget_as(i_layout_item const& aOther) const
        {
            return has_parent_widget() && aOther.has_parent_widget() && &parent_widget() == &aOther.parent_widget();
        }
    public:
    };

    template <typename LayoutItemType, layout_item_category ParentItemCategory = layout_item_category::Unspecified>
    class size_policy_of_parent : public LayoutItemType
    {
    private:
        typedef LayoutItemType base_type;
    public:
        using base_type::base_type;
    public:
        neogfx::size_policy size_policy() const override
        {
            if (base_type::has_size_policy())
            {
                return base_type::size_policy();
            }
            else if (base_type::has_parent_layout_item())
            {
                if constexpr (ParentItemCategory == layout_item_category::Layout)
                {
                    if (base_type::has_parent_layout())
                        return base_type::parent_layout().size_policy();
                    return base_type::size_policy();
                }
                else if constexpr (ParentItemCategory == layout_item_category::Widget)
                {
                    if (base_type::has_parent_widget())
                        return base_type::parent_widget().size_policy();
                    return base_type::size_policy();
                }
                else
                    return base_type::parent_layout_item().size_policy();
            }
            return base_type::size_policy();
        }
    };

    class i_item_layout : public i_service
    {
    public:
        virtual std::uint32_t id() const = 0;
        virtual void increment_id() = 0;
        virtual bool& in_progress() = 0;
        virtual bool& querying_ideal_size() = 0;
    public:
        static uuid const& iid() { static uuid const sIid{ 0xd7e05b0f, 0xc4eb, 0x440a, 0x844e, { 0x35, 0x18, 0xc0, 0x48, 0xee, 0x53 } }; return sIid; }
    };

    inline uint32_t global_layout_id()
    {
        return service<i_item_layout>().id();
    }

    inline bool querying_ideal_size()
    {
        return service<i_item_layout>().querying_ideal_size();
    }
}
