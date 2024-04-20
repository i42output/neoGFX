// i_layout.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/i_dock.hpp>
#include <neogfx/gui/layout/i_layout_item.hpp>

namespace neogfx
{
    class i_layout;
    class i_spacer;
    class i_widget;
    class i_title_bar;
    class i_status_bar;

    typedef std::uint32_t layout_item_index;
    typedef std::optional<layout_item_index> optional_layout_item_index;

    enum class layout_position : std::uint32_t
    {
        None    = 0x00000000,
        Top     = 0x00000001,
        Left    = 0x00000002,
        Center  = 0x00000004,
        Right   = 0x00000008,
        Bottom  = 0x00000010
    };

    enum class standard_layout : std::uint32_t
    {
        Default         = 0x00000000,
        Client          = 0x00000001,
        NonClient       = 0x00000002,
        TitleBar        = 0x00000003,
        Menu            = 0x00000004,
        Toolbar         = 0x00000005,
        Dock            = 0x00000006,
        StatusBar       = 0x00000007,
        ButtonBox       = 0x00000009
    };

    struct standard_layout_not_found : std::logic_error { standard_layout_not_found() : std::logic_error{ "neogfx::standard_layout_not_found" } {} };

    class i_standard_layout_container
    {
    public:
        struct no_title_bar : std::logic_error { no_title_bar() : std::logic_error{ "neogfx::i_standard_layout_container::no_title_bar" } {} };
        struct no_status_bar : std::logic_error { no_status_bar() : std::logic_error{ "neogfx::i_standard_layout_container::no_status_bar" } {} };
        struct no_client_widget : std::logic_error { no_client_widget() : std::logic_error{ "neogfx::i_standard_layout_container::no_client_widget" } {} };
    public:
        virtual ~i_standard_layout_container() = default;
    public:
        virtual bool is_widget() const = 0;
        virtual const i_widget& as_widget() const = 0;
        virtual i_widget& as_widget() = 0;
    public:
        virtual bool has_client_widget() const = 0;
        virtual const i_widget& client_widget() const = 0;
        virtual i_widget& client_widget() = 0;
        virtual void set_client(i_widget& aClient) = 0;
        virtual void set_client(i_ref_ptr<i_widget> const& aClient) = 0;
        virtual const i_title_bar& title_bar() const = 0;
        virtual i_title_bar& title_bar() = 0;
        virtual void set_title_bar(i_title_bar& aTitleBar) = 0;
        virtual void set_title_bar(i_ref_ptr<i_title_bar> const& aTitleBar) = 0;
        virtual const i_status_bar& status_bar() const = 0;
        virtual i_status_bar& status_bar() = 0;
        virtual void set_status_bar(i_status_bar& aStatusBar) = 0;
        virtual void set_status_bar(i_ref_ptr<i_status_bar> const& aStatusBar) = 0;
    public:
        virtual bool has_layout(standard_layout aStandardLayout) const = 0;
        virtual const i_layout& layout(standard_layout aStandardLayout, layout_position aPosition = layout_position::None) const = 0;
        virtual i_layout& layout(standard_layout aStandardLayout, layout_position aPosition = layout_position::None) = 0;
    public:
        const i_layout& non_client_layout() const
        {
            return layout(standard_layout::NonClient);
        }
        i_layout& non_client_layout()
        {
            return layout(standard_layout::NonClient);
        }
        const i_layout& title_bar_layout() const
        {
            return layout(standard_layout::TitleBar);
        }
        i_layout& title_bar_layout()
        {
            return layout(standard_layout::TitleBar);
        }
        const i_layout& menu_layout() const
        {
            return layout(standard_layout::Menu);
        }
        i_layout& menu_layout()
        {
            return layout(standard_layout::Menu);
        }
        const i_layout& toolbar_layout(layout_position aPosition = layout_position::Top) const
        {
            return layout(standard_layout::Toolbar, aPosition);
        }
        i_layout& toolbar_layout(layout_position aPosition = layout_position::Top)
        {
            return layout(standard_layout::Toolbar, aPosition);
        }
        const i_layout& dock_layout(layout_position aPosition = layout_position::Left) const
        {
            return layout(standard_layout::Dock, aPosition);
        }
        i_layout& dock_layout(layout_position aPosition = layout_position::Left)
        {
            return layout(standard_layout::Dock, aPosition);
        }
        const i_layout& dock_layout(dock_area aDockArea = dock_area::Left) const
        {
            return layout(standard_layout::Dock, to_position(aDockArea));
        }
        i_layout& dock_layout(dock_area aDockArea = dock_area::Left)
        {
            return layout(standard_layout::Dock, to_position(aDockArea));
        }
        const i_layout& client_layout() const
        {
            return layout(standard_layout::Client);
        }
        i_layout& client_layout()
        {
            return layout(standard_layout::Client);
        }
        const i_layout& button_box_layout() const
        {
            return layout(standard_layout::ButtonBox);
        }
        i_layout& button_box_layout()
        {
            return layout(standard_layout::ButtonBox);
        }
        const i_layout& status_bar_layout() const
        {
            return layout(standard_layout::StatusBar);
        }
        i_layout& status_bar_layout()
        {
            return layout(standard_layout::StatusBar);
        }
    public:
        static layout_position to_position(dock_area aDockArea)
        {
            switch (aDockArea)
            {
            case dock_area::None:
                return layout_position::None;
            case dock_area::Top:
                return layout_position::Top;
            case dock_area::Bottom:
                return layout_position::Bottom;
            case dock_area::Left:
            default:
                return layout_position::Left;
            case dock_area::Right:
                return layout_position::Right;
            }
        }
    };

    enum class autoscale : std::uint32_t
    {
        Default         = 0x00000000,
        Active          = 0x00000001,
        LockFixedSize   = 0x00000002
    };

    inline constexpr autoscale operator~(autoscale aLhs)
    {
        return static_cast<autoscale>(~static_cast<std::uint32_t>(aLhs));
    }

    inline constexpr autoscale operator|(autoscale aLhs, autoscale aRhs)
    {
        return static_cast<autoscale>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr autoscale operator&(autoscale aLhs, autoscale aRhs)
    {
        return static_cast<autoscale>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    enum class layout_direction : std::uint32_t
    {
        Unknown     = 0x00000000,
        Horizontal  = 0x00000001,
        Vertical    = 0x00000002
    };

    inline constexpr layout_direction operator~(layout_direction aLhs)
    {
        return static_cast<layout_direction>(~static_cast<std::uint32_t>(aLhs));
    }

    inline constexpr layout_direction operator|(layout_direction aLhs, layout_direction aRhs)
    {
        return static_cast<layout_direction>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr layout_direction operator&(layout_direction aLhs, layout_direction aRhs)
    {
        return static_cast<layout_direction>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    class i_layout : public i_layout_item
    {
    public:
        declare_event(layout_completed)
        declare_event(alignment_changed)
    public:
        typedef i_layout abstract_type;
    protected:
        class item;
    public:
        struct bad_item_index : std::logic_error { bad_item_index() : std::logic_error("neogfx::i_layout::bad_item_index") {} };
        struct wrong_item_type : std::logic_error { wrong_item_type() : std::logic_error("neogfx::i_layout::wrong_item_type") {} };
        struct item_not_found : std::logic_error { item_not_found() : std::logic_error("neogfx::i_layout::item_not_found") {} };
        struct incompatible_layouts : std::logic_error { incompatible_layouts() : std::logic_error("neogfx::i_layout::incompatible_layouts") {} };
    public:
        virtual ~i_layout() = default;
    public:    
        virtual i_layout_item& add(i_layout_item& aItem) = 0;
        virtual i_layout_item& add_at(layout_item_index aPosition, i_layout_item& aItem) = 0;
        virtual i_layout_item& add(i_ref_ptr<i_layout_item> const& aItem) = 0;
        virtual i_layout_item& add_at(layout_item_index aPosition, i_ref_ptr<i_layout_item> const& aItem) = 0;
        virtual i_spacer& add_spacer() = 0;
        virtual i_spacer& add_spacer_at(layout_item_index aPosition) = 0;
        virtual void remove_at(layout_item_index aIndex) = 0;
        virtual bool remove(i_layout_item& aItem) = 0;
        virtual void remove_all() = 0;
        virtual void move_all_to(i_layout& aDestination) = 0;
        virtual layout_item_index count() const = 0;
        virtual layout_item_index index_of(const i_layout_item& aItem) const = 0;
        virtual optional_layout_item_index find(const i_layout_item& aItem) const = 0;
        virtual bool is_widget_at(layout_item_index aIndex) const = 0;
        virtual const i_layout_item& item_at(layout_item_index aIndex) const = 0;
        virtual i_layout_item& item_at(layout_item_index aIndex) = 0;
        virtual const i_widget& get_widget_at(layout_item_index aIndex) const = 0;
        virtual i_widget& get_widget_at(layout_item_index aIndex) = 0;
        virtual const i_layout& get_layout_at(layout_item_index aIndex) const = 0;
        virtual i_layout& get_layout_at(layout_item_index aIndex) = 0;
    public:
        virtual layout_direction direction() const = 0;
        virtual bool has_spacing() const = 0;
        virtual size spacing() const = 0;
        virtual void set_spacing(optional_size const& sSpacing, bool aUpdateLayout = true) = 0;
        virtual bool always_use_spacing() const = 0;
        virtual void set_always_use_spacing(bool aAlwaysUseSpacing) = 0;
        virtual bool has_alignment() const = 0;
        virtual neogfx::alignment alignment() const = 0;
        virtual void set_alignment(optional_alignment const& aAlignment, bool aUpdateLayout = true) = 0;
        virtual neogfx::autoscale autoscale() const = 0;
        virtual void set_autoscale(neogfx::autoscale aAutoscale, bool aUpdateLayout = true) = 0;
        virtual visibility_constraint child_visibility() const = 0;
        virtual bool ignore_child_visibility() const = 0;
        virtual void set_ignore_child_visibility(bool aIgnoreChildVisibility) = 0;
    public:
        virtual void enable(bool aEnable) = 0;
        virtual bool enabled() const = 0;
        virtual void layout_items(const point& aPosition, const size& aSize) = 0;
        virtual bool invalidated() const = 0;
        virtual void invalidate(bool aDeferLayout = true) = 0;
        virtual void validate() = 0;
        // helpers
    public:
        template <typename ItemType>
        ItemType& add()
        {
            auto item = make_ref<ItemType>();
            return static_cast<ItemType&>(add(item));
        }
        template <typename ItemType>
        ItemType& add(i_ref_ptr<ItemType> const& aItem)
        {
            return static_cast<ItemType&>(add(to_abstract(static_pointer_cast<i_layout_item>(aItem))));
        }
        template <typename ItemType>
        ItemType& add_at(layout_item_index aPosition, i_ref_ptr<ItemType> const& aItem)
        {
            return static_cast<ItemType&>(add_at(aPosition, to_abstract(static_pointer_cast<i_layout_item>(aItem))));
        }
        template <typename ItemType>
        ItemType& add(ref_ptr<ItemType> const& aItem)
        {
            return static_cast<ItemType&>(add(to_abstract(static_pointer_cast<i_layout_item>(aItem))));
        }
        template <typename ItemType>
        ItemType& add_at(layout_item_index aPosition, ref_ptr<ItemType> const& aItem)
        {
            return static_cast<ItemType&>(add_at(aPosition, to_abstract(static_pointer_cast<i_layout_item>(aItem))));
        }
        template <typename ItemType, typename... Args>
        ItemType& emplace(Args&&... args)
        {
            auto newItem = make_ref<ItemType>(std::forward<Args>(args)...);
            add(newItem);
            return *newItem;
        }
        template <typename ItemType, typename... Args>
        ItemType& emplace_at(layout_item_index aPosition, Args&&... args)
        {
            auto newItem = make_ref<ItemType>(std::forward<Args>(args)...);
            add(newItem);
            if (aPosition < count())
                remove_at(aPosition);
            add_at(aPosition, newItem);
            return *newItem;
        }
        template <typename ItemType>
        ItemType& replace_item_at(layout_item_index aPosition, ItemType&& aItem)
        {
            if (aPosition < count())
                remove_at(aPosition);
            return static_cast<ItemType&>(add_at(aPosition, aItem));
        }
        template <typename ItemType>
        ItemType& replace_item_at(layout_item_index aPosition, i_ref_ptr<ItemType> const& aItem)
        {
            if (aPosition < count())
                remove_at(aPosition);
            return static_cast<ItemType&>(add_at(aPosition, static_pointer_cast<i_layout_item>(aItem)));
        }
        template <typename WidgetT>
        const WidgetT& get_widget_at(layout_item_index aIndex) const
        {
            return static_cast<const WidgetT&>(get_widget_at(aIndex));
        }
        template <typename WidgetT>
        WidgetT& get_widget_at(layout_item_index aIndex)
        {
            return static_cast<WidgetT&>(get_widget_at(aIndex));
        }
        bool is_descendent_of(const i_layout& aAncestor) const
        {
            auto* l = this;
            while (l->has_parent_layout() && &l->parent_widget() == &aAncestor.parent_widget())
            {
                l = &l->parent_layout();
                if (l == &aAncestor)
                    return true;
            }
            return false;
        }
        void enable()
        {
            enable(true);
        }
        void disable()
        {
            enable(false);
        }
    };

    inline size total_child_weight(const i_layout& aLayout)
    {
        size totalWeight;
        for (layout_item_index itemIndex = 0; itemIndex < aLayout.count(); ++itemIndex)
        {
            auto const& item = aLayout.item_at(itemIndex);
            if (!item.visible())
                continue;
            totalWeight += item.weight();
        }
        return totalWeight;
    }

    inline size calculate_relative_weight(const i_layout& aLayout, const i_layout_item& aItem)
    {
        size totalSize;
        for (layout_item_index itemIndex = 0; itemIndex < aLayout.count(); ++itemIndex)
        {
            auto const& item = aLayout.item_at(itemIndex);
            if (!item.visible())
                continue;
            totalSize += (item.has_fixed_size() ? item.fixed_size() : item.extents());
        }
        auto result = (aItem.has_fixed_size() ? aItem.fixed_size() : aItem.extents()) / totalSize;
        if (std::isnan(result.cx))
            result.cx = 1.0;
        if (std::isnan(result.cy))
            result.cy = 1.0;
        return result;
    }

    class scoped_layout_items : private neolib::scoped_flag
    {
    public:
        scoped_layout_items(bool aForceRefresh = false);
        ~scoped_layout_items();
    private:
        bool iStartLayout;
    };

    class scoped_query_ideal_size : private neolib::scoped_flag
    {
    public:
        scoped_query_ideal_size();
        ~scoped_query_ideal_size();
    };
}