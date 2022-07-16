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
#include "layout.inl"

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
        uint32_t id() const final
        {
            return iLayoutId;
        }
        void increment_id() final
        {
            if (++iLayoutId == static_cast<uint32_t>(-1))
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
        uint32_t iLayoutId;
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

    template size layout::do_minimum_size<layout::column_major<horizontal_layout>>(optional_size const& aAvailableSpace) const;
    template size layout::do_maximum_size<layout::column_major<horizontal_layout>>(optional_size const& aAvailableSpace) const;
    template void layout::do_layout_items<layout::column_major<horizontal_layout>>(const point& aPosition, const size& aSize);

    template size layout::do_minimum_size<layout::row_major<vertical_layout>>(optional_size const& aAvailableSpace) const;
    template size layout::do_maximum_size<layout::row_major<vertical_layout>>(optional_size const& aAvailableSpace) const;
    template void layout::do_layout_items<layout::row_major<vertical_layout>>(const point& aPosition, const size& aSize);

    layout::layout(neogfx::alignment aAlignment) :
        iParent{ nullptr },
        iOwner{ nullptr },
        iAlwaysUseSpacing{ false },
        iAlignment{ aAlignment },
        iAutoscale{ neogfx::autoscale::Default },
        iIgnoreVisibility{ false },
        iEnabled{ false },
        iLayoutStarted{ false },
        iInvalidated{ false }
    {
        enable();
    }

    layout::layout(i_widget& aOwner, neogfx::alignment aAlignment) :
        iParent{ nullptr },
        iOwner{ &aOwner },
        iAlwaysUseSpacing{ false },
        iAlignment{ aAlignment },
        iAutoscale{ neogfx::autoscale::Default },
        iIgnoreVisibility{ false },
        iEnabled{ false },
        iLayoutStarted{ false },
        iInvalidated{ false }
    {
        aOwner.set_layout(*this);
        enable();
    }

    layout::layout(i_layout& aParent, neogfx::alignment aAlignment) :
        iParent{ nullptr },
        iOwner{ aParent.has_layout_owner() ? &aParent.layout_owner() : nullptr },
        iAlwaysUseSpacing{ false },
        iAlignment{ aAlignment },
        iAutoscale{ neogfx::autoscale::Default },
        iIgnoreVisibility{ false },
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
        if (has_layout_owner() && layout_owner().has_layout() && &layout_owner().layout() == this)
            layout_owner().set_layout(ref_ptr<i_layout>{});
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

    bool layout::is_spacer() const
    {
        return false;
    }

    const i_spacer& layout::as_spacer() const
    {
        throw not_a_spacer();
    }

    i_spacer& layout::as_spacer()
    {
        throw not_a_spacer();
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
        if (iOwner != aOwner)
        {
            iOwner = aOwner;
            for (auto& i : items())
                if (!i.subject().has_layout_owner())
                    i.subject().set_layout_owner(aOwner);
        }
    }

    i_layout_item& layout::add(i_layout_item& aItem)
    {
        return add(ref_ptr<i_layout_item>{ref_ptr<i_layout_item>{}, & aItem});
    }

    i_layout_item& layout::add_at(layout_item_index aPosition, i_layout_item& aItem)
    {
        return add_at(aPosition, ref_ptr<i_layout_item>{ref_ptr<i_layout_item>{}, & aItem});
    }

    i_layout_item& layout::add(i_ref_ptr<i_layout_item> const& aItem)
    {
        return add_at(static_cast<layout_item_index>(items().size()), aItem);
    }

    i_layout_item& layout::add_at(layout_item_index aPosition, i_ref_ptr<i_layout_item> const& aItem)
    {
        if (aItem->is_widget() && (aItem->as_widget().widget_type() & widget_type::Floating) == widget_type::Floating)
            throw widget_is_floating();
        if (aItem->has_parent_layout() && !aItem->is_layout_item_cache())
        {
            if (&aItem->parent_layout() != this) // move
                aItem->parent_layout().remove(*aItem);
            else
                throw item_already_added();
        }
        while (aPosition > items().size())
            add_spacer_at(0);
        auto i = items().insert(std::next(items().begin(), aPosition), item{ aItem });
        i->set_parent_layout(this);
        if (has_layout_owner())
            i->set_layout_owner(&layout_owner());
        invalidate();
        return *aItem;
    }

    void layout::remove_at(layout_item_index aIndex)
    {
        remove(std::next(items().begin(), aIndex));
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
                item.set_parent_layout(&compatibleDestination);
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
            auto const& item = *i;
            if (&item.subject() == &aItem)
                return static_cast<layout_item_index>(std::distance(items().begin(), i));
        }
        return optional_layout_item_index{};
    }

    bool layout::is_widget_at(layout_item_index aIndex) const
    {
        if (aIndex >= items().size())
            throw bad_item_index();
        auto item = std::next(items().begin(), aIndex);
        return item->is_widget();
    }

    const i_layout_item& layout::item_at(layout_item_index aIndex) const
    {
        if (aIndex >= items().size())
            throw bad_item_index();
        auto item = std::next(items().begin(), aIndex);
        return item->subject();
    }

    i_layout_item& layout::item_at(layout_item_index aIndex)
    {
        return const_cast<i_layout_item&>(to_const(*this).item_at(aIndex));
    }

    const i_widget& layout::get_widget_at(layout_item_index aIndex) const
    {
        if (aIndex >= items().size())
            throw bad_item_index();
        auto item = std::next(items().begin(), aIndex);
        if (item->subject().is_widget())
            return item->subject().as_widget();
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
        auto item = std::next(items().begin(), aIndex);
        if (item->subject().is_layout())
            return item->subject().as_layout();
        throw not_a_layout();
    }

    i_layout& layout::get_layout_at(layout_item_index aIndex)
    {
        return const_cast<i_layout&>(to_const(*this).get_layout_at(aIndex));
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

    margin layout::margin() const
    {
        auto const& adjustedMargin = (has_margin() ? *Margin : service<i_app>().current_style().margin(margin_role::Layout) * dpi_scale_factor());
        return transformation(true) * units_converter(*this).from_device_units(adjustedMargin);
    }

    padding layout::padding() const
    {
        auto const& adjustedPadding = (has_padding() ? *Padding : service<i_app>().current_style().padding(padding_role::Layout) * dpi_scale_factor());
        return transformation(true) * units_converter(*this).from_device_units(adjustedPadding);
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
        return units_converter(*this).from_device_units(adjustedSpacing);
    }

    void layout::set_spacing(optional_size const& aSpacing, bool aUpdateLayout)
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

    bool layout::ignore_visibility() const
    {
        return iIgnoreVisibility || (has_parent_layout() && parent_layout().ignore_visibility());
    }

    void layout::set_ignore_visibility(bool aIgnoreVisibility, bool aUpdateLayout)
    {
        if (iIgnoreVisibility != aIgnoreVisibility)
        {
            iIgnoreVisibility = aIgnoreVisibility;
            if (aUpdateLayout)
                invalidate();
        }
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
        if (debug::layoutItem == this)
            service<debug::logger>() << typeid(*this).name() << "::invalidate(" << aDeferLayout << ")" << endl;
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
        if (debug::layoutItem == this)
            service<debug::logger>() << typeid(*this).name() << "::validate()" << endl;
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
        if (debug::layoutItem == this)
            service<debug::logger>() << typeid(*this).name() << "::size_policy()" << endl;
#endif
        if (has_size_policy())
            return base_type::size_policy();
        neogfx::size_policy result{ size_constraint::Minimum, size_constraint::Minimum };
        for (auto& i : items())
        {
            if (i.is_spacer())
                continue;
            if (!i.visible() && !ignore_visibility())
                continue;
            if (i.effective_size_policy().horizontal_size_policy() == size_constraint::Expanding)
                result.set_horizontal_size_policy(size_constraint::Expanding);
            else if (i.effective_size_policy().horizontal_size_policy() == size_constraint::Maximum)
                result.set_horizontal_size_policy(size_constraint::Maximum);
            if (i.effective_size_policy().vertical_size_policy() == size_constraint::Expanding)
                result.set_vertical_size_policy(size_constraint::Expanding);
            else if (i.effective_size_policy().vertical_size_policy() == size_constraint::Maximum)
                result.set_vertical_size_policy(size_constraint::Maximum);
        }
        return result;
    }

    bool layout::device_metrics_available() const
    {
        if (iDeviceMetrics == std::nullopt)
        {
            if (has_layout_owner() && layout_owner().device_metrics_available())
                iDeviceMetrics = &layout_owner().device_metrics();
        }
        return iDeviceMetrics != std::nullopt;
    }

    const i_device_metrics& layout::device_metrics() const
    {
        if (layout::device_metrics_available())
            return **iDeviceMetrics;
        throw no_device_metrics();
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
            if (&*i == &aItem || &i->subject() == &aItem)
                return i;
        return items().end();
    }

    layout::item_list::iterator layout::find_item(i_layout_item& aItem)
    {
        for (auto i = items().begin(); i != items().end(); ++i)
            if (&*i == &aItem || &i->subject() == &aItem)
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

    uint32_t layout::spacer_count() const
    {
        uint32_t count = 0u;
        for (auto const& i : items())
            if (i.is_spacer())
                ++count;
        return count;
    }

    void layout::remove(item_list::iterator aItem)
    {
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << typeid(*this).name() << "::remove(" << std::distance(items().begin(), aItem) << ")" << endl;
#endif // NEOGFX_DEBUG
        {
            item_list toRemove;
            toRemove.splice(toRemove.begin(), iItems, aItem);
            if (aItem->has_parent_layout() && &aItem->parent_layout() == this)
            {
                aItem->set_parent_layout(nullptr);
                aItem->set_layout_owner(nullptr);
            }
            update_layout();
        }
    }

    uint32_t layout::items_visible(item_type_e aItemType) const
    {
        uint32_t count = 0u;
        for (auto const& item : items())
            if (item.visible() || ignore_visibility())
            {
                if ((aItemType & ItemTypeWidget) && item.is_widget())
                    ++count;
                else if ((aItemType & ItemTypeLayout) && item.is_layout())
                    ++count;
                else if ((aItemType & ItemTypeSpacer) && item.is_spacer())
                    ++count;
            }
        return count;
    }
}