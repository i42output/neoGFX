// layout.cpp
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

#include <neogfx/neogfx.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/layout.hpp>
#include <neogfx/gui/layout/layout_item_cache.hpp>
#include <neogfx/gui/layout/i_spacer.hpp>
#include <neogfx/gui/widget/i_scrollbar.hpp>
#include "layout.ipp"

namespace neogfx
{
    class item_layout : public i_item_layout
    {
    public:
        item_layout() :
            iLayoutId{ 0u },
            iLayoutInProgress{ false },
            iQueryingIdealSize{ false }
        {
        }
    public:
        std::uint32_t id() const final
        {
            return iLayoutId;
        }
        void increment_id() final
        {
            if (++iLayoutId == static_cast<std::uint32_t>(-1))
                iLayoutId = 0u;
        }
        bool& in_progress() final
        {
            return iLayoutInProgress;
        }
        bool& querying_ideal_size() final
        {
            return iQueryingIdealSize;
        }
    private:
        std::uint32_t iLayoutId;
        bool iLayoutInProgress;
        bool iQueryingIdealSize;
    };
}

template<> neogfx::i_item_layout& services::start_service<neogfx::i_item_layout>()
{
    static neogfx::item_layout sLayoutState;
    return sLayoutState;
}

namespace neogfx
{
    scoped_layout_items::scoped_layout_items(bool aForceRefresh) :
        neolib::scoped_flag{ service<i_item_layout>().in_progress() },
        iStartLayout{ !saved() || aForceRefresh }
    {
        if (iStartLayout)
            service<i_item_layout>().increment_id();
    }
    
    scoped_layout_items::~scoped_layout_items()
    {
        if (iStartLayout)
            service<i_scrollbar_container_updater>().process();
    }

    scoped_query_ideal_size::scoped_query_ideal_size() :
        neolib::scoped_flag{ service<i_item_layout>().querying_ideal_size() }
    {
        if (!saved())
            service<i_item_layout>().increment_id();
    }

    scoped_query_ideal_size::~scoped_query_ideal_size()
    {
        if (!saved())
            service<i_item_layout>().increment_id();
    }

    template size layout::do_minimum_size<layout::row_major<horizontal_layout>>(optional_size const& aAvailableSpace) const;
    template size layout::do_maximum_size<layout::row_major<horizontal_layout>>(optional_size const& aAvailableSpace) const;
    template void layout::do_layout_items<layout::row_major<horizontal_layout>>(const point& aPosition, const size& aSize);

    template size layout::do_minimum_size<layout::column_major<vertical_layout>>(optional_size const& aAvailableSpace) const;
    template size layout::do_maximum_size<layout::column_major<vertical_layout>>(optional_size const& aAvailableSpace) const;
    template void layout::do_layout_items<layout::column_major<vertical_layout>>(const point& aPosition, const size& aSize);

    layout::layout(optional_alignment const& aAlignment) :
        iParent{ nullptr },
        iOwner{ nullptr },
        iAlwaysUseSpacing{ false },
        iAlignment{ aAlignment },
        iAutoscale{ neogfx::autoscale::Default },
        iChildVisibility{ visibility_constraint::Consider },
        iEnabled{ false },
        iLayoutStarted{ false },
        iInvalidated{ false }
    {
        enable();
    }

    layout::layout(i_widget& aOwner, optional_alignment const& aAlignment) :
        iParent{ nullptr },
        iOwner{ &aOwner },
        iAlwaysUseSpacing{ false },
        iAlignment{ aAlignment },
        iAutoscale{ neogfx::autoscale::Default },
        iChildVisibility{ visibility_constraint::Consider },
        iEnabled{ false },
        iLayoutStarted{ false },
        iInvalidated{ false }
    {
        aOwner.set_layout(*this);
        enable();
    }

    layout::layout(i_layout& aParent, optional_alignment const& aAlignment) :
        iParent{ nullptr },
        iOwner{ aParent.has_parent_widget() ? &aParent.parent_widget() : nullptr },
        iAlwaysUseSpacing{ false },
        iAlignment{ aAlignment },
        iAutoscale{ neogfx::autoscale::Default },
        iChildVisibility{ visibility_constraint::Consider },
        iEnabled{ false },
        iLayoutStarted{ false },
        iInvalidated{ false }
    {
        Padding = neogfx::padding{};
        aParent.add(*this);
        enable();
    }

    layout::~layout()
    {
        remove_all();
        if (has_parent_layout())
            parent_layout().remove(*this);
        if (has_parent_widget() && parent_widget().has_layout() && &parent_widget().layout() == this)
            parent_widget().set_layout(ref_ptr<i_layout>{});
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

    bool layout::has_parent_widget() const
    {
        return iOwner != nullptr;
    }

    const i_widget& layout::parent_widget() const
    {
        if (has_parent_widget())
            return *iOwner;
        if (has_parent_layout())
            return *(iOwner = const_cast<i_widget*>(&parent_layout().parent_widget()));
        throw no_parent_widget();
    }

    i_widget& layout::parent_widget()
    {
        return const_cast<i_widget&>(to_const(*this).parent_widget());
    }

    void layout::set_parent_widget(i_widget* aParentWidget)
    {
        if (iOwner != aParentWidget)
        {
            iOwner = aParentWidget;
            for (auto& itemRef : items())
            {
                auto& item = (*itemRef).identity();
                if (!item.has_parent_widget())
                    item.set_parent_widget(aParentWidget);
            }
        }
    }

    i_layout_item& layout::add(i_layout_item& aItem)
    {
        return add(ref_ptr<i_layout_item>{ref_ptr<i_layout_item>{}, & aItem});
    }

    i_layout_item& layout::add_at(layout_item_index aPosition, i_layout_item& aItem)
    {
        return add_at(aPosition, ref_ptr<i_layout_item>{ref_ptr<i_layout_item>{}, &aItem});
    }

    i_layout_item& layout::add(i_ref_ptr<i_layout_item> const& aItem)
    {
        return add_at(static_cast<layout_item_index>(items().size()), aItem);
    }

    i_layout_item& layout::add_at(layout_item_index aPosition, i_ref_ptr<i_layout_item> const& aItem)
    {
        if (aItem->is_widget() && (aItem->as_widget().widget_type() & widget_type::Floating) == widget_type::Floating)
            throw widget_is_floating();
        if (aItem->has_parent_layout())
        {
            if (&aItem->parent_layout() != this) // move
                aItem->parent_layout().remove(*aItem);
            else
                throw item_already_added();
        }
        while (aPosition > items().size())
            add_spacer_at(0);
        auto i = items().insert(std::next(items().begin(), aPosition), aItem->is_cache() ?
            dynamic_pointer_cast<i_layout_item_cache>(aItem) : make_abstract_ref<layout_item_cache>(aItem));
        (**i).set_parent_layout(this);
        if (has_parent_widget())
            (**i).set_parent_widget(&parent_widget());
        invalidate();
        return *aItem;
    }

    void layout::remove_at(layout_item_index aIndex)
    {
        remove(std::next(items().begin(), aIndex));
    }

    bool layout::remove(i_layout_item& aItem)
    {
        auto existing = std::find_if(begin(), end(), [&](auto const& cachedItem) 
            { return !cachedItem->subject_destroyed() && &cachedItem->identity() == &aItem.identity(); });
        if (existing == end())
            return false;
        remove(existing);
        return true;
    }

    void layout::remove_all()
    {
        while (!items().empty())
            remove(std::prev(items().end()));
        invalidate();
    }

    void layout::move_all_to(i_layout& aDestination)
    {
        try
        {
            auto& compatibleDestination = dynamic_cast<layout&>(aDestination); // dynamic_cast? not a fan but heh.
            compatibleDestination.items().splice(compatibleDestination.items().end(), iItems);
            for (auto& item : compatibleDestination)
            {
                item->set_parent_layout(&compatibleDestination);
                if (aDestination.has_parent_widget())
                    item->set_parent_widget(&aDestination.parent_widget());
            }
        }
        catch (std::bad_cast)
        {
            throw incompatible_layouts();
        }
        invalidate();
        aDestination.invalidate();
    }

    layout_item_index layout::count() const
    {
        return static_cast<layout_item_index>(items().size());
    }

    layout_item_index layout::index_of(const i_layout_item& aItem) const
    {
        auto result = find(aItem);
        if (result)
            return *result;
        throw layout_item_not_found();
    }

    optional_layout_item_index layout::find(const i_layout_item& aItem) const
    {
        for (auto i = items().begin(); i != items().end(); ++i)
        {
            auto const& item = **i;
            if (&item.identity() == &aItem.identity())
                return static_cast<layout_item_index>(std::distance(items().begin(), i));
        }
        return optional_layout_item_index{};
    }

    bool layout::is_widget_at(layout_item_index aIndex) const
    {
        if (aIndex >= items().size())
            throw bad_item_index();
        auto item = std::next(items().begin(), aIndex);
        return (**item).is_widget();
    }

    const i_layout_item& layout::item_at(layout_item_index aIndex) const
    {
        if (aIndex >= items().size())
            throw bad_item_index();
        auto item = std::next(items().begin(), aIndex);
        return (**item).identity();
    }

    i_layout_item& layout::item_at(layout_item_index aIndex)
    {
        return const_cast<i_layout_item&>(to_const(*this).item_at(aIndex));
    }

    const i_widget& layout::get_widget_at(layout_item_index aIndex) const
    {
        if (aIndex >= items().size())
            throw bad_item_index();
        auto item = *std::next(items().begin(), aIndex);
        if (item->identity().is_widget())
            return item->identity().as_widget();
        throw not_a_widget();
    }

    i_widget& layout::get_widget_at(layout_item_index aIndex)
    {
        return const_cast<i_widget&>(to_const(*this).get_widget_at(aIndex));
    }

    const i_layout& layout::get_layout_at(layout_item_index aIndex) const
    {
        if (aIndex >= items().size())
            throw bad_item_index();
        auto item = *std::next(items().begin(), aIndex);
        if (item->identity().is_layout())
            return item->identity().as_layout();
        throw not_a_layout();
    }

    i_layout& layout::get_layout_at(layout_item_index aIndex)
    {
        return const_cast<i_layout&>(to_const(*this).get_layout_at(aIndex));
    }

    margin layout::margin() const
    {
        auto const& adjustedMargin = (has_margin() ? base_type::margin() : service<i_app>().current_style().margin(margin_role::Layout) * dpi_scale_factor());
        return transformation(true) * units_converter{ *this }.from_device_units(adjustedMargin);
    }

    padding layout::padding() const
    {
        auto const& adjustedPadding = (has_padding() ? base_type::padding() : service<i_app>().current_style().padding(padding_role::Layout) * dpi_scale_factor());
        return transformation(true) * units_converter{ *this }.from_device_units(adjustedPadding);
    }

    layout_direction layout::direction() const
    {
        return layout_direction::Unknown;
    }

    bool layout::has_spacing() const
    {
        return iSpacing != std::nullopt;
    }

    size layout::spacing() const
    {
        auto const& adjustedSpacing = (has_spacing() ? *iSpacing : service<i_app>().current_style().spacing() * dpi_scale_factor());
        return units_converter{ *this }.from_device_units(adjustedSpacing);
    }

    void layout::set_spacing(optional_size const& aSpacing, bool aUpdateLayout)
    {
        if (iSpacing != aSpacing)
        {
            iSpacing = (aSpacing != std::nullopt ?
                optional_size{ units_converter{ *this }.to_device_units(*aSpacing) } :
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

    bool layout::has_alignment() const
    {
        return iAlignment != std::nullopt;
    }

    alignment layout::alignment() const
    {
        if (has_alignment())
            return *iAlignment;
        return neogfx::alignment::Center | neogfx::alignment::VCenter;
    }

    void layout::set_alignment(optional_alignment const& aAlignment, bool aUpdateLayout)
    {
        if (iAlignment != aAlignment)
        {
            iAlignment = aAlignment;
            AlignmentChanged();
            if (aUpdateLayout)
                invalidate();
        }
    }

    autoscale layout::autoscale() const
    {
        return iAutoscale;
    }

    void layout::set_autoscale(neogfx::autoscale aAutoscale, bool aUpdateLayout)
    {
        if (iAutoscale != aAutoscale)
        {
            iAutoscale = aAutoscale;
            if (aUpdateLayout)
                invalidate();
        }
    }

    visibility_constraint layout::child_visibility() const
    {
        return iChildVisibility;
    }

    bool layout::ignore_child_visibility() const
    {
        return iChildVisibility == visibility_constraint::Ignore;
    }

    void layout::set_ignore_child_visibility(bool aIgnoreChildVisibility)
    {
        iChildVisibility = (aIgnoreChildVisibility ? visibility_constraint::Ignore : visibility_constraint::Consider);
    }

    void layout::enable(bool aEnable)
    {
        if (iEnabled != aEnable)
        {
            iEnabled = aEnable;
            if (has_parent_layout())
                enabled() ? 
                    parent_layout().layout_item_enabled(*this) : 
                    parent_layout().layout_item_disabled(*this);
        }
    }

    bool layout::enabled() const
    {
        return iEnabled;
    }

    void layout::layout_as(const point& aPosition, const size& aSize)
    {
        layout_items(aPosition, aSize);
    }

    void layout::fix_weightings(bool aRecalculate)
    {
        if ((autoscale() & neogfx::autoscale::Active) == neogfx::autoscale::Active)
        {
            neolib::scoped_object<neogfx::autoscale> so{ iAutoscale, iAutoscale & ~neogfx::autoscale::Active };
            update_layout(false, false);
        }
        base_type::fix_weightings(aRecalculate);
    }

    void layout::layout_item_enabled(i_layout_item& aItem)
    {
        base_type::layout_item_enabled(aItem);
        invalidate();
    }
    
    void layout::layout_item_disabled(i_layout_item& aItem)
    {
        base_type::layout_item_disabled(aItem);
        invalidate();
    }

    bool layout::visible() const
    {
        return iEnabled;
    }

    bool layout::invalidated() const
    {
        return iInvalidated;
    }

    void layout::invalidate(bool aDeferLayout)
    {
#ifdef NEOGFX_DEBUG
        if (service<i_debug>().layout_item() == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::invalidate(" << aDeferLayout << ")" << std::endl;
#endif
        if (!iEnabled)
            return;
        if (iInvalidated)
            return;
        iInvalidated = true;
    }

    void layout::validate()
    {
#ifdef NEOGFX_DEBUG
        if (service<i_debug>().layout_item() == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::validate()" << std::endl;
#endif
        if (!iInvalidated)
            return;
        iInvalidated = false;
    }

    void layout::set_extents(const size& aExtents)
    {
        base_type::set_extents(aExtents);
        if (iEnabled && (autoscale() & neogfx::autoscale::LockFixedSize) == neogfx::autoscale::LockFixedSize)
            FixedSize = extents();
    }

    size_policy layout::size_policy() const
    {
#ifdef NEOGFX_DEBUG
        if (service<i_debug>().layout_item() == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::size_policy()" << std::endl;
#endif
        if (has_size_policy())
            return base_type::size_policy();
        neogfx::size_policy result{ size_constraint::Minimum, size_constraint::Minimum };
        for (auto& i : items())
        {
            auto& item = *i;
            if (item.is_spacer())
                continue;
            auto const itemSizePolicy = item.effective_size_policy();
            if (!item.visible())
                continue;
            if (itemSizePolicy.horizontal_constraint() == size_constraint::Expanding)
                result.set_horizontal_constraint(size_constraint::Expanding);
            else if (itemSizePolicy.horizontal_constraint() == size_constraint::Maximum)
                result.set_horizontal_constraint(size_constraint::Maximum);
            if (itemSizePolicy.vertical_constraint() == size_constraint::Expanding)
                result.set_vertical_constraint(size_constraint::Expanding);
            else if (itemSizePolicy.vertical_constraint() == size_constraint::Maximum)
                result.set_vertical_constraint(size_constraint::Maximum);
        }
        return result;
    }

    bool layout::device_metrics_available() const
    {
        if (iDeviceMetrics == std::nullopt)
        {
            if (has_parent_widget() && parent_widget().device_metrics_available())
                iDeviceMetrics = &parent_widget().device_metrics();
        }
        return iDeviceMetrics != std::nullopt;
    }

    const i_device_metrics& layout::device_metrics() const
    {
        if (layout::device_metrics_available())
            return **iDeviceMetrics;
        return service<i_surface_manager>().display().metrics();
    }

    layout::item_list::const_iterator layout::cbegin() const
    {
        return items().cbegin();
    }

    layout::item_list::const_iterator layout::cend() const
    {
        return items().cend();
    }

    layout::item_list::const_iterator layout::begin() const
    {
        return items().begin();
    }

    layout::item_list::const_iterator layout::end() const
    {
        return items().end();
    }

    layout::item_list::iterator layout::begin()
    {
        return items().begin();
    }

    layout::item_list::iterator layout::end()
    {
        return items().end();
    }

    layout::item_list::const_reverse_iterator layout::rbegin() const
    {
        return items().rbegin();
    }

    layout::item_list::const_reverse_iterator layout::rend() const
    {
        return items().rend();
    }

    layout::item_list::reverse_iterator layout::rbegin()
    {
        return items().rbegin();
    }

    layout::item_list::reverse_iterator layout::rend()
    {
        return items().rend();
    }

    layout::item_list::const_iterator layout::find_item(const i_layout_item& aItem) const
    {
        for (auto i = items().begin(); i != items().end(); ++i)
            if (&**i == &aItem || &(**i).identity() == &aItem.identity())
                return i;
        return items().end();
    }

    layout::item_list::iterator layout::find_item(i_layout_item& aItem)
    {
        for (auto i = items().begin(); i != items().end(); ++i)
            if (&**i == &aItem || &(**i).identity() == &aItem.identity())
                return i;
        return items().end();
    }

    const layout::item_list& layout::items() const
    {
        return iItems;
    }

    layout::item_list& layout::items()
    {
        return iItems;
    }

    std::uint32_t layout::spacer_count() const
    {
        std::uint32_t count = 0u;
        for (auto const& i : items())
            if (i->is_spacer())
                ++count;
        return count;
    }

    void layout::remove(item_list::iterator aItem)
    {
#ifdef NEOGFX_DEBUG
        if (service<i_debug>().layout_item() == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::remove(" << std::distance(items().begin(), aItem) << ")" << std::endl;
#endif // NEOGFX_DEBUG
        {
            auto& item = **aItem;
            item_list toRemove;
            toRemove.splice(toRemove.begin(), iItems, aItem);
            if (!item.subject_destroyed() && item.has_parent_layout() && &item.parent_layout() == this)
            {
                item.set_parent_layout(nullptr);
                if (!item.is_widget())
                    item.set_parent_widget(nullptr);
            }
            update_layout(true, true);
        }
    }

    std::uint32_t layout::items_visible(item_type_e aItemType) const
    {
        std::uint32_t count = 0u;
        for (auto const& itemRef : items())
        {
            auto const& item = *itemRef;
            if (item.visible())
            {
                if ((aItemType & ItemTypeWidget) && item.is_widget())
                    ++count;
                else if ((aItemType & ItemTypeLayout) && item.is_layout())
                    ++count;
                else if ((aItemType & ItemTypeSpacer) && item.is_spacer())
                    ++count;
            }
        }
        return count;
    }
}