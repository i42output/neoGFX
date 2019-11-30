// i_layout.hpp
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
#include <neogfx/gui/layout/i_layout_item.hpp>

namespace neogfx
{
    class i_widget;
    class i_spacer;

    class i_layout : public i_layout_item
    {
    public:
        declare_event(layout_completed)
        declare_event(alignment_changed)
    public:
        static i_layout* debug;
    public:
        typedef i_layout abstract_type;
        typedef uint32_t item_index;
        typedef std::optional<item_index> optional_item_index;
    protected:
        class item;
    public:
        struct bad_item_index : std::logic_error { bad_item_index() : std::logic_error("neogfx::i_layout::bad_item_index") {} };
        struct wrong_item_type : std::logic_error { wrong_item_type() : std::logic_error("neogfx::i_layout::wrong_item_type") {} };
        struct item_not_found : std::logic_error { item_not_found() : std::logic_error("neogfx::i_layout::item_not_found") {} };
        struct incompatible_layouts : std::logic_error { incompatible_layouts() : std::logic_error("neogfx::i_layout::incompatible_layouts") {} };
    public:
        virtual ~i_layout() {}
    public:    
        virtual i_layout_item& add(i_layout_item& aItem) = 0;
        virtual i_layout_item& add_at(item_index aPosition, i_layout_item& aItem) = 0;
        virtual i_layout_item& add(std::shared_ptr<i_layout_item> aItem) = 0;
        virtual i_layout_item& add_at(item_index aPosition, std::shared_ptr<i_layout_item> aItem) = 0;
        virtual i_spacer& add_spacer() = 0;
        virtual i_spacer& add_spacer_at(item_index aPosition) = 0;
        virtual void remove_at(item_index aIndex) = 0;
        virtual bool remove(i_layout_item& aItem) = 0;
        virtual void remove_all() = 0;
        virtual void move_all_to(i_layout& aDestination) = 0;
        virtual item_index count() const = 0;
        virtual optional_item_index find(const i_layout_item& aItem) const = 0;
        virtual bool is_widget_at(item_index aIndex) const = 0;
        virtual const i_layout_item& item_at(item_index aIndex) const = 0;
        virtual i_layout_item& item_at(item_index aIndex) = 0;
        virtual const i_widget& get_widget_at(item_index aIndex) const = 0;
        virtual i_widget& get_widget_at(item_index aIndex) = 0;
        virtual const i_layout& get_layout_at(item_index aIndex) const = 0;
        virtual i_layout& get_layout_at(item_index aIndex) = 0;
        virtual const i_layout_item_proxy& find_proxy(const i_layout_item& aItem) const = 0;
        virtual i_layout_item_proxy& find_proxy(i_layout_item& aItem) = 0;
    public:
        virtual bool has_spacing() const = 0;
        virtual size spacing() const = 0;
        virtual void set_spacing(const optional_size& sSpacing, bool aUpdateLayout = true) = 0;
        virtual bool always_use_spacing() const = 0;
        virtual void set_always_use_spacing(bool aAlwaysUseSpacing) = 0;
        virtual neogfx::alignment alignment() const = 0;
        virtual void set_alignment(neogfx::alignment aAlignment, bool aUpdateLayout = true) = 0;
    public:
        virtual void enable() = 0;
        virtual void disable() = 0;
        virtual bool enabled() const = 0;
        virtual void layout_items(const point& aPosition, const size& aSize) = 0;
        virtual bool invalidated() const = 0;
        virtual void invalidate() = 0;
        virtual void validate() = 0;
    public:
        virtual uint32_t layout_id() const = 0;
        virtual void next_layout_id() = 0;
        // helpers
    public:
        template <typename ItemType>
        ItemType& add(ItemType&& aItem)
        {
            return static_cast<ItemType&>(add(static_cast<i_layout_item&>(aItem)));
        }
        template <typename ItemType>
        ItemType& add_at(item_index aPosition, ItemType&& aItem)
        {
            return static_cast<ItemType&>(add_at(aPosition, static_cast<i_layout_item&>(aItem)));
        }
        template <typename ItemType>
        ItemType& add(std::shared_ptr<ItemType> aItem)
        {
            return static_cast<ItemType&>(add(std::static_pointer_cast<i_layout_item>(aItem)));
        }
        template <typename ItemType>
        ItemType& add_at(item_index aPosition, std::shared_ptr<ItemType> aItem)
        {
            return static_cast<ItemType&>(add_at(aPosition, std::static_pointer_cast<i_layout_item>(aItem)));
        }
        template <typename ItemType, typename... Args>
        ItemType& emplace(Args&&... args)
        {
            auto newItem = std::make_shared<ItemType>(std::forward<Args>(args)...);
            add(newItem);
            return *newItem;
        }
        template <typename ItemType, typename... Args>
        ItemType& emplace_at(item_index aPosition, Args&&... args)
        {
            auto newItem = std::make_shared<ItemType>(std::forward<Args>(args)...);
            add(newItem);
            if (aPosition < count())
                remove_at(aPosition);
            add_at(aPosition, newItem);
            return *newItem;
        }
        template <typename ItemType>
        ItemType& replace_item_at(item_index aPosition, ItemType&& aItem)
        {
            if (aPosition < count())
                remove_at(aPosition);
            return static_cast<ItemType&>(add_at(aPosition, aItem));
        }
        template <typename ItemType>
        ItemType& replace_item_at(item_index aPosition, std::shared_ptr<ItemType> aItem)
        {
            if (aPosition < count())
                remove_at(aPosition);
            return static_cast<ItemType&>(add_at(aPosition, std::static_pointer_cast<i_layout_item>(aItem)));
        }
        template <typename WidgetT>
        const WidgetT& get_widget_at(item_index aIndex) const
        {
            return static_cast<const WidgetT&>(get_widget_at(aIndex));
        }
        template <typename WidgetT>
        WidgetT& get_widget_at(item_index aIndex)
        {
            return static_cast<WidgetT&>(get_widget_at(aIndex));
        }
    };
}