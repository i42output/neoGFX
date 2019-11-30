// layout.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/layout.hpp>
#include <neogfx/gui/layout/layout_item_proxy.hpp>
#include <neogfx/gui/layout/i_spacer.hpp>
#include <neogfx/app/i_app.hpp>
#include "layout.inl"

namespace neogfx
{
    i_layout* i_layout::debug;

    template size layout::do_minimum_size<layout::column_major<horizontal_layout>>(const optional_size& aAvailableSpace) const;
    template size layout::do_maximum_size<layout::column_major<horizontal_layout>>(const optional_size& aAvailableSpace) const;
    template void layout::do_layout_items<layout::column_major<horizontal_layout>>(const point& aPosition, const size& aSize);

    template size layout::do_minimum_size<layout::row_major<vertical_layout>>(const optional_size& aAvailableSpace) const;
    template size layout::do_maximum_size<layout::row_major<vertical_layout>>(const optional_size& aAvailableSpace) const;
    template void layout::do_layout_items<layout::row_major<vertical_layout>>(const point& aPosition, const size& aSize);

    layout::layout(neogfx::alignment aAlignment) :
        iParent{ nullptr },
        iOwner{ nullptr },
        iAlwaysUseSpacing{ false },
        iAlignment{ aAlignment },
        iEnabled{ false },
        iMinimumSize{},
        iMaximumSize{},
        iLayoutStarted{ false },
        iLayoutId{ 0 },
        iInvalidated{ false }
    {
        enable();
    }

    layout::layout(i_widget& aOwner, neogfx::alignment aAlignment) :
        iParent{ nullptr },
        iOwner{ &aOwner },
        iAlwaysUseSpacing{ false },
        iAlignment{ aAlignment },
        iEnabled{ false },
        iMinimumSize{},
        iMaximumSize{},
        iLayoutStarted{ false },
        iLayoutId{ 0 },
        iInvalidated{ false }
    {
        aOwner.set_layout(*this);
        enable();
    }

    layout::layout(i_layout& aParent, neogfx::alignment aAlignment) :
        iParent{ nullptr },
        iOwner{ aParent.has_layout_owner() ? &aParent.layout_owner() : nullptr },
        iMargins{ neogfx::margins{} },
        iAlwaysUseSpacing{ false },
        iAlignment{ aAlignment },
        iEnabled{ false },
        iMinimumSize{},
        iMaximumSize{},
        iLayoutStarted{ false },
        iLayoutId{ 0 },
        iInvalidated{ false }
    {
        aParent.add(*this);
        enable();
    }

    layout::~layout()
    {
        remove_all();
        if (has_parent_layout())
            parent_layout().remove(*this);
        if (has_layout_owner() && layout_owner().has_layout() && &layout_owner().layout() == this)
            layout_owner().set_layout(nullptr);
    }

    bool layout::is_layout() const
    {
        return true;
    }

    const i_layout& layout::as_layout() const
    {
        return *this;
    }

    i_layout& layout::as_layout()
    {
        return *this;
    }

    bool layout::is_widget() const
    {
        return false;
    }

    const i_widget& layout::as_widget() const
    {
        throw not_a_widget();
    }

    i_widget& layout::as_widget()
    {
        throw not_a_widget();
    }

    bool layout::has_parent_layout() const
    {
        return iParent != nullptr;
    }

    const i_layout& layout::parent_layout() const
    {
        if (has_parent_layout())
            return *iParent;
        throw no_parent_layout();
    }

    i_layout& layout::parent_layout()
    {
        return const_cast<i_layout&>(to_const(*this).parent_layout());
    }

    void layout::set_parent_layout(i_layout* aParentLayout)
    {
        iParent = aParentLayout;
    }

    bool layout::has_layout_owner() const
    {
        return iOwner != nullptr;
    }

    const i_widget& layout::layout_owner() const
    {
        if (has_layout_owner())
            return *iOwner;
        if (has_parent_layout())
            return *(iOwner = const_cast<i_widget*>(&parent_layout().layout_owner()));
        throw no_layout_owner();
    }

    i_widget& layout::layout_owner()
    {
        return const_cast<i_widget&>(to_const(*this).layout_owner());
    }

    void layout::set_layout_owner(i_widget* aOwner)
    {
        iOwner = aOwner;
        for (auto& i : iItems)
            if (!i.subject().has_layout_owner())
                i.subject().set_layout_owner(aOwner);
    }

    bool layout::is_proxy() const
    {
        return false;
    }

    const i_layout_item_proxy& layout::proxy_for_layout() const
    {
        return parent_layout().find_proxy(*this);
    }

    i_layout_item_proxy& layout::proxy_for_layout()
    {
        return parent_layout().find_proxy(*this);
    }

    i_layout_item& layout::add(i_layout_item& aItem)
    {
        return add(std::shared_ptr<i_layout_item>{std::shared_ptr<i_layout_item>{}, &aItem});
    }

    i_layout_item& layout::add_at(item_index aPosition, i_layout_item& aItem)
    {
        return add_at(aPosition, std::shared_ptr<i_layout_item>{std::shared_ptr<i_layout_item>{}, &aItem});
    }

    i_layout_item& layout::add(std::shared_ptr<i_layout_item> aItem)
    {
        return add_at(static_cast<item_index>(iItems.size()), aItem);
    }

    i_layout_item& layout::add_at(item_index aPosition, std::shared_ptr<i_layout_item> aItem)
    {
        if (aItem->has_parent_layout() && !aItem->is_proxy())
        {
            if (&aItem->parent_layout() != this) // move
                aItem->parent_layout().remove(*aItem);
            else
                throw item_already_added();
        }
        while (aPosition > iItems.size())
            add_spacer_at(0);
        auto i = iItems.insert(std::next(iItems.begin(), aPosition), item{ aItem });
        i->set_parent_layout(this);
        if (has_layout_owner())
            i->set_layout_owner(&layout_owner());
        invalidate();
        return *aItem;
    }

    void layout::remove_at(item_index aIndex)
    {
        remove(std::next(iItems.begin(), aIndex));
    }

    bool layout::remove(i_layout_item& aItem)
    {
        for (auto i = begin(); i != end(); ++i)
            if (&i->subject() == &aItem)
            {
                remove(i);
                return true;
            }
        for (auto i = rbegin(); i != rend(); ++i)
            if (i->subject().is_layout() && i->subject().as_layout().remove(aItem))
                return true;
        return false;
    }

    void layout::remove_all()
    {
        while (!iItems.empty())
            remove(std::prev(iItems.end()));
        invalidate();
    }

    void layout::move_all_to(i_layout& aDestination)
    {
        try
        {
            auto& compatibleDestination = dynamic_cast<layout&>(aDestination); // dynamic_cast? not a fan but heh.
            compatibleDestination.iItems.splice(compatibleDestination.items().end(), iItems);
            for (auto& item : compatibleDestination)
                item.set_parent_layout(&compatibleDestination);
        }
        catch (std::bad_cast)
        {
            throw incompatible_layouts();
        }
        invalidate();
        aDestination.invalidate();
    }

    layout::item_index layout::count() const
    {
        return static_cast<item_index>(iItems.size());
    }

    layout::optional_item_index layout::find(const i_layout_item& aItem) const
    {
        for (auto i = iItems.begin(); i != iItems.end(); ++i)
        {
            const auto& item = *i;
            if (&item.subject() == &aItem)
                return std::distance(iItems.begin(), i);
        }
        return optional_item_index();
    }

    bool layout::is_widget_at(item_index aIndex) const
    {
        if (aIndex >= iItems.size())
            throw bad_item_index();
        auto item = std::next(iItems.begin(), aIndex);
        return item->is_widget();
    }

    const i_layout_item& layout::item_at(item_index aIndex) const
    {
        if (aIndex >= iItems.size())
            throw bad_item_index();
        auto item = std::next(iItems.begin(), aIndex);
        return item->subject();
    }

    i_layout_item& layout::item_at(item_index aIndex)
    {
        return const_cast<i_layout_item&>(to_const(*this).item_at(aIndex));
    }

    const i_widget& layout::get_widget_at(item_index aIndex) const
    {
        if (aIndex >= iItems.size())
            throw bad_item_index();
        auto item = std::next(iItems.begin(), aIndex);
        if (item->subject().is_widget())
            return item->subject().as_widget();
        throw not_a_widget();
    }

    i_widget& layout::get_widget_at(item_index aIndex)
    {
        return const_cast<i_widget&>(to_const(*this).get_widget_at(aIndex));
    }
        
    const i_layout& layout::get_layout_at(item_index aIndex) const
    {
        if (aIndex >= iItems.size())
            throw bad_item_index();
        auto item = std::next(iItems.begin(), aIndex);
        if (item->subject().is_layout())
            return item->subject().as_layout();
        throw not_a_layout();
    }

    i_layout& layout::get_layout_at(item_index aIndex)
    {
        return const_cast<i_layout&>(to_const(*this).get_layout_at(aIndex));
    }

    const i_layout_item_proxy& layout::find_proxy(const i_layout_item& aItem) const
    {
        for (auto& item : items())
            if (&item == &aItem || &item.subject() == &aItem)
                return item;
        throw item_not_found();
    }

    i_layout_item_proxy& layout::find_proxy(i_layout_item& aItem)
    {
        return const_cast<i_layout_item_proxy&>(to_const(*this).find_proxy(aItem));
    }

    bool layout::high_dpi() const
    {
        return has_layout_owner() && layout_owner().has_surface() ? 
            layout_owner().surface().ppi() >= 150.0 : 
            service<i_surface_manager>().display().metrics().ppi() >= 150.0;
    }

    dimension layout::dpi_scale_factor() const
    {
        return default_dpi_scale_factor(
            has_layout_owner() && layout_owner().has_surface() ?
                layout_owner().surface().ppi() : 
                service<i_surface_manager>().display().metrics().ppi());
    }

    bool layout::has_margins() const
    {
        return iMargins != std::nullopt;
    }

    margins layout::margins() const
    {
        const auto& adjustedMargins = (has_margins() ? *iMargins : service<i_app>().current_style().margins() * dpi_scale_factor());
        return units_converter(*this).from_device_units(adjustedMargins);
    }

    void layout::set_margins(const optional_margins& aMargins, bool aUpdateLayout)
    {
        optional_margins newMargins = (aMargins != std::nullopt ? units_converter(*this).to_device_units(*aMargins) : optional_margins());
        if (iMargins != newMargins)
        {
            iMargins = newMargins;
            if (aUpdateLayout)
                invalidate();
        }
    }

    bool layout::has_spacing() const
    {
        return iSpacing != std::nullopt;
    }

    size layout::spacing() const
    {
        const auto& adjustedSpacing = (has_spacing() ? *iSpacing : service<i_app>().current_style().spacing() * dpi_scale_factor());
        return units_converter(*this).from_device_units(adjustedSpacing);
    }

    void layout::set_spacing(const optional_size& aSpacing, bool aUpdateLayout)
    {
        if (iSpacing != aSpacing)
        {
            iSpacing = (aSpacing != std::nullopt ? 
                optional_size{ units_converter(*this).to_device_units(*aSpacing) } :
                aSpacing);
            if (aUpdateLayout)
                invalidate();
        }
    }

    bool layout::always_use_spacing() const
    {
        return iAlwaysUseSpacing;
    }

    void layout::set_always_use_spacing(bool aAlwaysUseSpacing)
    {
        iAlwaysUseSpacing = aAlwaysUseSpacing;
    }

    neogfx::alignment layout::alignment() const
    {
        return iAlignment;
    }

    void layout::set_alignment(neogfx::alignment aAlignment, bool aUpdateLayout)
    {
        if (iAlignment != aAlignment)
        {
            iAlignment = aAlignment;
            AlignmentChanged.trigger();
            if (aUpdateLayout)
                invalidate();
        }
    }

    void layout::enable()
    {
        if (!enabled())
        {
            iEnabled = true;
            invalidate();
        }
    }

    void layout::disable()
    {
        if (enabled())
            iEnabled = false;
    }

    bool layout::enabled() const
    {
        return iEnabled;
    }

    void layout::layout_as(const point& aPosition, const size& aSize)
    {
        layout_items(aPosition, aSize);
    }

    uint32_t layout::layout_id() const
    {
        return iLayoutId;
    }

    void layout::next_layout_id()
    {
        if (++iLayoutId == static_cast<uint32_t>(-1))
            iLayoutId = 0;
        for (auto& item : *this)
            if (item.is_layout())
                item.as_layout().next_layout_id();
            else if (item.is_widget() && item.as_widget().has_layout())
                item.as_widget().layout().next_layout_id();
    }

    bool layout::visible() const
    {
        return true;
    }

    bool layout::invalidated() const
    {
        return iInvalidated;
    }

    void layout::invalidate()
    {
        if (!enabled())
            return;
        if (invalidated())
            return;
        iInvalidated = true;
        if (has_parent_layout())
            parent_layout().invalidate();
        if (has_layout_owner())
        {
            if (layout_owner().is_managing_layout())
                layout_owner().layout_items(true);
            i_widget* w = iOwner;
            while (w != nullptr && w->has_parent())
            {
                w = &w->parent();
                if (w->has_layout())
                    break;
            }
            if (w != nullptr && w != iOwner && w->has_layout())
                w->layout().invalidate();
        }
    }

    void layout::validate()
    {
        if (!invalidated())
            return;
        iInvalidated = false;
    }

    point layout::position() const
    {
        return units_converter(*this).from_device_units(iPosition);
    }
    
    void layout::set_position(const point& aPosition)
    {
        iPosition = units_converter(*this).to_device_units(aPosition);
    }

    size layout::extents() const
    {
        return units_converter(*this).from_device_units(iExtents);
    }

    void layout::set_extents(const size& aExtents)
    {
        iExtents = units_converter(*this).to_device_units(aExtents);
    }

    bool layout::has_size_policy() const
    {
        return iSizePolicy != std::nullopt;
    }

    size_policy layout::size_policy() const
    {
        if (has_size_policy())
            return *iSizePolicy;
        neogfx::size_policy result{ size_constraint::Minimum, size_constraint::Minimum };
        for (auto& i : items())
        {
            if (i.is_spacer())
                continue;
            if (i.size_policy().horizontal_size_policy() == size_constraint::Expanding)
                result.set_horizontal_size_policy(size_constraint::Expanding);
            if (i.size_policy().vertical_size_policy() == size_constraint::Expanding)
                result.set_vertical_size_policy(size_constraint::Expanding);
        }
        return result;
    }

    void layout::set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout)
    {
        if (iSizePolicy != aSizePolicy)
        {
            iSizePolicy = aSizePolicy;
            if (aUpdateLayout)
                invalidate();
        }
    }

    bool layout::has_weight() const
    {
        return iWeight != std::nullopt;
    }

    size layout::weight() const
    {
        if (has_weight())
            return *iWeight;
        else
            return size{ 1.0 };
    }

    void layout::set_weight(const optional_size& aWeight, bool aUpdateLayout)
    {
        if (iWeight != aWeight)
        {
            iWeight = aWeight;
            if (aUpdateLayout)
                invalidate();
        }
    }

    bool layout::has_minimum_size() const
    {
        return iMinimumSize != std::nullopt;
    }

    size layout::minimum_size(const optional_size&) const
    {
        if (has_minimum_size())
            return units_converter(*this).from_device_units(*iMinimumSize);
        else
            return size{};
    }

    void layout::set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout)
    {
        optional_size newMinimumSize = (aMinimumSize != std::nullopt ? units_converter(*this).to_device_units(*aMinimumSize) : optional_size());
        if (iMinimumSize != newMinimumSize)
        {
            iMinimumSize = newMinimumSize;
            if (aUpdateLayout)
                invalidate();
        }
    }

    bool layout::has_maximum_size() const
    {
        return iMaximumSize != std::nullopt;
    }

    size layout::maximum_size(const optional_size&) const
    {
        return has_maximum_size() ?
            units_converter(*this).from_device_units(*iMaximumSize) :
            size::max_size();
    }

    void layout::set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout)
    {
        optional_size newMaximumSize = (aMaximumSize != std::nullopt ? units_converter(*this).to_device_units(*aMaximumSize) : optional_size());
        if (iMaximumSize != newMaximumSize)
        {
            iMaximumSize = newMaximumSize;
            if (aUpdateLayout)
                invalidate();
        }
    }

    bool layout::device_metrics_available() const
    {
        return has_layout_owner() && layout_owner().device_metrics_available();
    }

    const i_device_metrics& layout::device_metrics() const
    {
        if (device_metrics_available())
            return layout_owner().device_metrics();
        throw no_device_metrics();
    }

    layout::item_list::const_iterator layout::cbegin() const
    {
        return iItems.cbegin();
    }

    layout::item_list::const_iterator layout::cend() const
    {
        return iItems.cend();
    }

    layout::item_list::const_iterator layout::begin() const
    {
        return iItems.begin();
    }

    layout::item_list::const_iterator layout::end() const
    {
        return iItems.end();
    }

    layout::item_list::iterator layout::begin()
    {
        return iItems.begin();
    }

    layout::item_list::iterator layout::end()
    {
        return iItems.end();
    }

    layout::item_list::const_reverse_iterator layout::rbegin() const
    {
        return iItems.rbegin();
    }

    layout::item_list::const_reverse_iterator layout::rend() const
    {
        return iItems.rend();
    }

    layout::item_list::reverse_iterator layout::rbegin()
    {
        return iItems.rbegin();
    }

    layout::item_list::reverse_iterator layout::rend()
    {
        return iItems.rend();
    }

    layout::item_list::const_iterator layout::find_item(const i_layout_item& aItem) const
    {
        for (auto i = iItems.begin(); i != iItems.end(); ++i)
            if (&*i == &aItem || &i->subject() == &aItem)
                return i;
        return iItems.end();
    }

    layout::item_list::iterator layout::find_item(i_layout_item& aItem)
    {
        for (auto i = iItems.begin(); i != iItems.end(); ++i)
            if (&*i == &aItem || &i->subject() == &aItem)
                return i;
        return iItems.end();
    }

    const layout::item_list& layout::items() const
    {
        return iItems;
    }

    uint32_t layout::spacer_count() const
    {
        uint32_t count = 0u;
        for (const auto& i : iItems)
            if (i.is_spacer())
                ++count;
        return count;
    }

    void layout::remove(item_list::iterator aItem)
    {
        {
            item_list toRemove;
            toRemove.splice(toRemove.begin(), iItems, aItem);
            if (aItem->has_parent_layout() && &aItem->parent_layout() == this)
            {
                aItem->set_parent_layout(nullptr);
                aItem->set_layout_owner(nullptr);
            }
        }
        invalidate();
    }

    uint32_t layout::items_visible(item_type_e aItemType) const
    {
        uint32_t count = 0u;
        for (const auto& i : iItems)
            if (i.visible())
            {
                if ((aItemType & ItemTypeWidget) && i.is_widget())
                    ++count;
                else if ((aItemType & ItemTypeLayout) && i.is_layout())
                    ++count;
                else if ((aItemType & ItemTypeSpacer) && i.is_spacer())
                    ++count;
            }
        return count;
    }
}