// widget.ipp
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
#include <unordered_map>
#include <typeindex>
#include <neolib/core/scoped.hpp>
#include <neolib/app/i_shared_thread_local.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/layout/i_async_layout.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/layout/i_layout_item_cache.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_surface_window.hpp>

namespace neogfx
{
    template <WidgetInterface Interface>
    inline widget<Interface>::widget() :
        iSingular{ false },
        iParent{ nullptr },
        iAddingChild{ false },
        iLinkBefore{ nullptr },
        iLinkAfter{ nullptr },
        iParentLayout{ nullptr },
        iResizing{ false },
        iLayoutPending{ false },
        iLayoutInProgress{ 0 },
        iLayer{ LayerWidget }
    {
        base_type::Position.Changed([this](const point&) { moved(); });
        base_type::Size.Changed([this](const size&) { resized(); });
        base_type::set_alive();
    }
    
    template <WidgetInterface Interface>
    inline widget<Interface>::widget(i_widget& aParent) :
        iSingular{ false },
        iParent{ nullptr },
        iAddingChild{ false },
        iLinkBefore{ nullptr },
        iLinkAfter{ nullptr },
        iParentLayout{ nullptr },
        iResizing{ false },
        iLayoutPending{ false },
        iLayoutInProgress{ 0 },
        iLayer{ LayerWidget }
    {
        base_type::Position.Changed([this](const point&) { moved(); });
        base_type::Size.Changed([this](const size&) { resized(); });
        aParent.add(*this);
        base_type::set_alive();
    }

    template <WidgetInterface Interface>
    inline widget<Interface>::widget(i_layout& aLayout) :
        iSingular{ false },
        iParent{ nullptr },
        iAddingChild{ false },
        iLinkBefore{ nullptr },
        iLinkAfter{ nullptr },
        iParentLayout{ nullptr },
        iResizing{ false },
        iLayoutPending{ false },
        iLayoutInProgress{ 0 },
        iLayer{ LayerWidget }
    {
        base_type::Position.Changed([this](const point&) { moved(); });
        base_type::Size.Changed([this](const size&) { resized(); });
        aLayout.add(*this);
        base_type::set_alive();
    }

    template <WidgetInterface Interface>
    inline widget<Interface>::~widget()
    {
        unlink();
        if (service<i_keyboard>().is_keyboard_grabbed_by(*this))
            service<i_keyboard>().ungrab_keyboard(*this);
        remove_all();
        {
            auto layout = iLayout;
            iLayout.reset();
        }
        if (has_parent())
            parent().remove(*this);
        if (has_parent_layout())
            parent_layout().remove(*this);
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::property_changed(i_property& aProperty)
    {
        static auto invalidate_layout = [](i_widget& self) 
        { 
            self.update_layout();
        };
        static auto invalidate_canvas = [](i_widget& self) 
        { 
            self.update(true); 
        };
        static auto invalidate_window_canvas = [](i_widget& self) 
        { 
            self.root().as_widget().update(true); 
        };
        static auto ignore = [](i_widget&) {};
        static const std::unordered_map<std::type_index, std::function<void(i_widget&)>> sActions =
        {
            { std::type_index{ typeid(property_category::hard_geometry) }, invalidate_layout },
            { std::type_index{ typeid(property_category::soft_geometry) }, invalidate_window_canvas },
            { std::type_index{ typeid(property_category::font) }, invalidate_layout },
            { std::type_index{ typeid(property_category::color) }, invalidate_canvas },
            { std::type_index{ typeid(property_category::other_appearance) }, invalidate_canvas },
            { std::type_index{ typeid(property_category::other) }, ignore }
        };
        auto iterAction = sActions.find(std::type_index{ aProperty.category() });
        if (iterAction != sActions.end())
            iterAction->second(*this);
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::device_metrics_available() const
    {
        if (iDeviceMetrics == std::nullopt)
        {
            if (widget::has_root() && widget::root().has_native_window())
            {
                iDeviceMetrics.emplace(*this);
                DeviceMetricsUpdated(*this);
            }
        }
        return iDeviceMetrics != std::nullopt;
    }

    template <WidgetInterface Interface>
    inline const i_device_metrics& widget<Interface>::device_metrics() const
    {
        if (widget::device_metrics_available())
            return *iDeviceMetrics;
        return service<i_surface_manager>().display().metrics();
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::is_singular() const
    {
        return iSingular;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_singular(bool aSingular)
    {
        if (iSingular != aSingular)
        {
            iSingular = aSingular;
            if (iSingular)
            {
                iParent = nullptr;
            }
        }
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::is_root() const
    {
        if constexpr (std::is_base_of_v<i_window, Interface>)
            return iRoot == this;
        else
            return false;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_root() const
    {
        if (iRoot == std::nullopt)
        {
            const i_widget* w = this;
            while (!w->is_root() && w->has_parent())
                w = &w->parent();
            if (w->is_root())
                iRoot = &w->root();
        }
        return iRoot != std::nullopt;
    }

    template <WidgetInterface Interface>
    inline const i_window& widget<Interface>::root() const
    {
        if constexpr (std::is_base_of_v<i_window, Interface>)
            return *this;
        else
        {
            if (widget::has_root())
                return **iRoot;
            throw no_root();
        }
    }

    template <WidgetInterface Interface>
    inline i_window& widget<Interface>::root()
    {
        return const_cast<i_window&>(to_const(*this).widget::root());
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_root(i_window& aRoot)
    {
        if (iRoot != &aRoot)
        {
            iRoot = &aRoot;
            if (widget::root().has_native_window())
            {
                iDeviceMetrics.emplace(*this);
                DeviceMetricsUpdated(*this);
            }
        }
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_surface() const
    {
        return widget::has_root() && widget::root().has_surface();
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::is_surface() const
    {
        return widget::is_root() && widget::root().is_surface();
    }

    template <WidgetInterface Interface>
    inline const i_surface& widget<Interface>::surface() const
    {
        return widget::root().surface();
    }

    template <WidgetInterface Interface>
    inline i_surface& widget<Interface>::surface()
    {
        return widget::root().surface();
    }

    template <WidgetInterface Interface>
    inline const i_surface& widget<Interface>::real_surface() const
    {
        return widget::root().real_surface();
    }

    template <WidgetInterface Interface>
    inline i_surface& widget<Interface>::real_surface()
    {
        return widget::root().real_surface();
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_parent() const
    {
        return iParent != nullptr;
    }

    template <WidgetInterface Interface>
    inline const i_widget& widget<Interface>::parent() const
    {
        if (!has_parent())
            throw no_parent();
        return *iParent;
    }

    template <WidgetInterface Interface>
    inline i_widget& widget<Interface>::parent()
    {
        return const_cast<i_widget&>(to_const(*this).parent());
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_parent(i_widget& aParent)
    {
        if (iParent != &aParent)
        {
            if (!widget::is_root() && aParent.has_root())
                widget::set_root(aParent.root());
            if ((widget::is_root() && !widget::root().is_nested()) || aParent.adding_child())
            {
                iParent = &aParent;
                parent_changed();
            }
            else
            {
                if (widget::use_count())
                    aParent.add(ref_ptr<i_widget>{ this });
                else
                    aParent.add(*this);
            }
        }
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::parent_changed()
    {
        auto& self = *this;

        if (!widget::is_root())
        {
            self.reset_origin();
            if (self.has_parent_layout())
                self.update_layout();
        }
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::adding_child() const
    {
        return iAddingChild;
    }

    template <WidgetInterface Interface>
    inline i_widget& widget<Interface>::add(i_widget& aChild)
    {
        return add(ref_ptr<i_widget>{ ref_ptr<i_widget>{}, &aChild });
    }

    template <WidgetInterface Interface>
    inline i_widget& widget<Interface>::add(const i_ref_ptr<i_widget>& aChild)
    {
        if (aChild->has_parent() && &aChild->parent() == this)
            return *aChild;
        neolib::scoped_flag sf{ iAddingChild };
        i_widget* oldParent = aChild->has_parent() ? &aChild->parent() : nullptr;
        ref_ptr<i_widget> child = aChild;
        if (oldParent != nullptr)
            oldParent->remove(*child, true);
        iChildren.push_back(child);
        iChildMap[&*child] = iChildren.size() - 1u;
        child->set_parent(*this);
        child->set_singular(false);
        if (widget::has_root())
            widget::root().widget_added(*child);
        ChildAdded(*child);
        return *child;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::remove(i_widget& aChild, bool aSingular, i_ref_ptr<i_widget>& aChildRef)
    {
        auto const posIter = iChildMap.find(&aChild);
        if (posIter == iChildMap.end())
            return;
        auto const pos = posIter->second;
        iChildMap.erase(posIter);
        auto const existing = std::next(iChildren.begin(), pos);
        if (existing == iChildren.end())
            return;
        destroyed_flag childDestroyed{ aChild };
        ref_ptr<i_widget> keep = **existing;
        iChildren.erase(existing);
        for (auto& cpos : iChildMap)
            if (cpos.second > pos)
                --cpos.second;
        if (childDestroyed)
            return;
        if (aSingular)
            keep->set_singular(true);
        if (has_layout())
            layout().remove(aChild);
        if (childDestroyed)
            return;
        if (widget::has_root())
            widget::root().widget_removed(aChild);
        if (childDestroyed)
            return;
        ChildRemoved(*keep);
        aChildRef = keep;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::remove_all()
    {
        while (!iChildren.empty())
            remove(*iChildren.back(), true);
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_children() const
    {
        return !iChildren.empty();
    }

    template <WidgetInterface Interface>
    inline const typename widget<Interface>::widget_list& widget<Interface>::children() const
    {
        return iChildren;
    }

    template <WidgetInterface Interface>
    inline typename widget<Interface>::widget_list::const_iterator widget<Interface>::last() const
    {
        if (!has_children())
        {
            if (has_parent())
                return parent().find(*this);
            else
                throw no_children();
        }
        else
            return iChildren.back()->last();
    }

    template <WidgetInterface Interface>
    inline typename widget<Interface>::widget_list::iterator widget<Interface>::last()
    {
        if (!has_children())
        {
            if (has_parent())
                return parent().find(*this);
            else
                throw no_children();
        }
        else
            return iChildren.back()->last();
    }

    template <WidgetInterface Interface>
    inline typename widget<Interface>::widget_list::const_iterator widget<Interface>::find(const i_widget& aChild, bool aThrowIfNotFound) const
    {
        auto const pos = iChildMap.find(&aChild);
        if (pos == iChildMap.end() || pos->second >= iChildren.size())
        {
            if (aThrowIfNotFound)
                throw not_child();
            else
                return iChildren.end();
        }
        return std::next(iChildren.begin(), pos->second);
    }

    template <WidgetInterface Interface>
    inline typename widget<Interface>::widget_list::iterator widget<Interface>::find(const i_widget& aChild, bool aThrowIfNotFound)
    {
        auto const pos = iChildMap.find(&aChild);
        if (pos == iChildMap.end() || pos->second >= iChildren.size())
        {
            if (aThrowIfNotFound)
                throw not_child();
            else
                return iChildren.end();
        }
        return std::next(iChildren.begin(), pos->second);
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::bring_child_to_front(const i_widget& aChild)
    {
        auto const pos = iChildMap.find(&aChild);
        if (pos != iChildMap.end())
        {
            auto existing = std::next(iChildren.begin(), pos->second);
            ref_ptr<i_widget> child = *existing;
            iChildren.erase(existing);
            iChildren.insert(iChildren.begin(), child);
            widget_child_pos newPos = 0u;
            for (auto& c : iChildren)
                iChildMap[&*c] = newPos++;
        }
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::send_child_to_back(const i_widget& aChild)
    {
        auto const pos = iChildMap.find(&aChild);
        if (pos != iChildMap.end())
        {
            auto existing = std::next(iChildren.begin(), pos->second);
            ref_ptr<i_widget> child = *existing;
            iChildren.erase(existing);
            iChildren.insert(iChildren.end(), child);
            widget_child_pos newPos = 0u;
            for (auto& c : iChildren)
                iChildMap[&*c] = newPos++;
        }
    }

    template <WidgetInterface Interface>
    inline layer_t widget<Interface>::layer() const
    {
        return iLayer;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_layer(layer_t aLayer)
    {
        if (iLayer != aLayer)
        {
            iLayer = aLayer;
            update(true);
        }
    }

    template <WidgetInterface Interface>
    inline const i_widget& widget<Interface>::before() const
    {
        if (iLinkBefore != nullptr)
            return *iLinkBefore;
        if (has_parent())
        {
            auto me = parent().find(*this);
            if (me != parent().children().begin())
                return **(*(me - 1))->last();
            else
                return parent();
        }
        else if (has_children())
            return **last();
        else
            return *this;
    }

    template <WidgetInterface Interface>
    inline i_widget& widget<Interface>::before()
    {
        return const_cast<i_widget&>(to_const(*this).before());
    }

    template <WidgetInterface Interface>
    inline const i_widget& widget<Interface>::after() const
    {
        if (iLinkAfter != nullptr)
            return *iLinkAfter;
        if (has_children())
            return *iChildren.front();
        if (has_parent())
        {
            auto me = parent().find(*this);
            if (me + 1 != parent().children().end())
                return *(*(me + 1));
            else if (parent().has_parent())
            {
                auto myParent = parent().parent().find(parent());
                while ((*myParent)->has_parent() && (*myParent)->parent().has_parent() &&
                    myParent + 1 == (*myParent)->parent().children().end())
                    myParent = (*myParent)->parent().parent().find((*myParent)->parent());
                if ((*myParent)->has_parent() && myParent + 1 != (*myParent)->parent().children().end())
                    return **(myParent + 1);
                else if ((*(myParent))->has_parent())
                    return (*(myParent))->parent();
                else
                    return **myParent;
            }
            else
                return parent();
        }
        else
            return *this;
    }

    template <WidgetInterface Interface>
    inline i_widget& widget<Interface>::after()
    {
        return const_cast<i_widget&>(to_const(*this).after());
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::link_before(i_widget* aPreviousWidget)
    {
        iLinkBefore = aPreviousWidget;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::link_after(i_widget* aNextWidget)
    {
        iLinkAfter = aNextWidget;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::unlink()
    {
        if (iLinkBefore != nullptr)
            iLinkBefore->link_after(iLinkAfter);
        if (iLinkAfter != nullptr)
            iLinkAfter->link_before(iLinkBefore);
        iLinkBefore = nullptr;
        iLinkAfter = nullptr;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_layout() const
    {
        return iLayout != nullptr;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_layout(i_layout& aLayout, bool aMoveExistingItems)
    {
        set_layout(ref_ptr<i_layout>{ ref_ptr<i_layout>{}, &aLayout }, aMoveExistingItems);
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_layout(const i_ref_ptr<i_layout>& aLayout, bool aMoveExistingItems)
    {
        if (iLayout == aLayout)
            throw layout_already_set();
        auto oldLayout = iLayout;
        iLayout = aLayout;
        if (iLayout != nullptr)
        {
            if (has_parent_layout())
                iLayout->set_parent_layout(&parent_layout());
            iLayout->set_parent_widget(this);
            if (aMoveExistingItems)
            {
                if (oldLayout == nullptr)
                {
                    for (auto& child : iChildren)
                        if (child->has_parent_layout() && &child->parent_layout() == nullptr)
                            iLayout->add(child);
                }
                else
                    oldLayout->move_all_to(*iLayout);
            }
        }
    }

    template <WidgetInterface Interface>
    inline const i_layout& widget<Interface>::layout() const
    {
        if (!iLayout)
            throw no_layout();
        return *iLayout;
    }
    
    template <WidgetInterface Interface>
    inline i_layout& widget<Interface>::layout()
    {
        if (!iLayout)
            throw no_layout();
        return *iLayout;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::can_defer_layout() const
    {
        return is_managing_layout();
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::is_managing_layout() const
    {
        return false;
    }

    template <WidgetInterface Interface>
    inline rect widget<Interface>::element_rect(skin_element) const
    {
        return client_rect();
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_parent_layout() const
    {
        return iParentLayout != nullptr;
    }
    
    template <WidgetInterface Interface>
    inline const i_layout& widget<Interface>::parent_layout() const
    {
        if (has_parent_layout())
            return *iParentLayout;
        throw no_parent_layout();
    }

    template <WidgetInterface Interface>
    inline i_layout& widget<Interface>::parent_layout()
    {
        return const_cast<i_layout&>(to_const(*this).parent_layout());
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_parent_layout(i_layout* aParentLayout)
    {
        if (has_layout() && layout().has_parent_layout() && &layout().parent_layout() == iParentLayout)
            layout().set_parent_layout(aParentLayout);
        iParentLayout = aParentLayout;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_parent_widget() const
    {
        return has_parent();
    }

    template <WidgetInterface Interface>
    inline const i_widget& widget<Interface>::parent_widget() const
    {
        return parent();
    }

    template <WidgetInterface Interface>
    inline i_widget& widget<Interface>::parent_widget()
    {
        return parent();
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_parent_widget(i_widget* aParentWidget)
    {
        if (aParentWidget != nullptr)
            set_parent(*aParentWidget);
        else if (has_parent())
        {
            ref_ptr<i_widget> keep;
            parent().remove(*this, true, keep);
            iParent = nullptr;
            parent_changed();
        }
    }

    template <WidgetInterface Interface>
    inline optional<neogfx::layout_reason>& widget<Interface>::layout_reason()
    {
        return iLayoutReason;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::layout_items(bool aDefer)
    {
        auto& self = *this;

        if (layout_items_in_progress())
            return;

        if (!aDefer)
        {
#ifdef NEOGFX_DEBUG
            if (debug::layoutItem == this)
            {
                if (!iLayoutPending)
                    service<debug::logger>() << neolib::logger::severity::Debug << "widget:layout_items: layout now" << std::endl;
                else
                    service<debug::logger>() << neolib::logger::severity::Debug << "widget:layout_items: layout a deferred layout now" << std::endl;
            }
#endif
            iLayoutPending = false;
            service<i_async_layout>().validate(*this);
            if (has_layout())
            {
                layout_items_started();
                if (widget::is_root() && size_policy() != size_constraint::Manual)
                {
                    size desiredSize = self.extents();
                    switch (size_policy().horizontal_constraint())
                    {
                    case size_constraint::Fixed:
                        desiredSize.cx = self.has_fixed_size() ? self.fixed_size().cx : minimum_size(self.extents()).cx;
                        break;
                    case size_constraint::Minimum:
                        desiredSize.cx = minimum_size(self.extents()).cx;
                        break;
                    case size_constraint::Maximum:
                        desiredSize.cx = maximum_size(self.extents()).cx;
                        break;
                    default:
                        break;
                    }
                    switch (size_policy().vertical_constraint())
                    {
                    case size_constraint::Fixed:
                        desiredSize.cy = self.has_fixed_size() ? self.fixed_size().cy : minimum_size(self.extents()).cy;
                        break;
                    case size_constraint::Minimum:
                        desiredSize.cy = minimum_size(self.extents()).cy;
                        break;
                    case size_constraint::Maximum:
                        desiredSize.cy = maximum_size(self.extents()).cy;
                        break;
                    default:
                        break;
                    }
                    resize(desiredSize);
                }
                auto const& cr = client_rect(false);
                layout().layout_items(cr.top_left(), cr.extents());
                layout_items_completed();
            }
        }
        else if (can_defer_layout())
        {
            if (!iLayoutPending)
            {
#ifdef NEOGFX_DEBUG
                if (debug::layoutItem == this)
                    service<debug::logger>() << neolib::logger::severity::Debug << "widget:layout_items: deferred layout" << std::endl;
#endif
                iLayoutPending = service<i_async_layout>().defer_layout(*this);
            }
        }
        else if (self.has_layout_manager())
        {
            throw widget_cannot_defer_layout();
        }
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::layout_items_started()
    {
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << "widget:layout_items_started()" << std::endl;
#endif // NEOGFX_DEBUG
        ++iLayoutInProgress;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::layout_items_in_progress() const
    {
        return iLayoutInProgress != 0;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::layout_items_completed()
    {
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << "widget:layout_items_completed()" << std::endl;
#endif // NEOGFX_DEBUG
        if (--iLayoutInProgress == 0)
        {
            LayoutCompleted();
            update();
        }
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_logical_coordinate_system() const
    {
        return LogicalCoordinateSystem != std::nullopt;
    }

    template <WidgetInterface Interface>
    inline logical_coordinate_system widget<Interface>::logical_coordinate_system() const
    {
        if (has_logical_coordinate_system())
            return *LogicalCoordinateSystem;
        return neogfx::logical_coordinate_system::AutomaticGui;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_logical_coordinate_system(const optional_logical_coordinate_system& aLogicalCoordinateSystem)
    {
        LogicalCoordinateSystem = aLogicalCoordinateSystem;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::move(const point& aPosition)
    {
        auto& self = *this;

        if (base_type::Position == units_converter{ *this }.to_device_units(aPosition))
            return;

#ifdef NEOGFX_DEBUG
        auto const currentPosition = self.position();
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << "widget<Interface>::move(" << aPosition << ")" << 
                " (from " << currentPosition << ")" << std::endl;
#endif // NEOGFX_DEBUG

        self.set_position(aPosition);
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::moved()
    {
        auto& self = *this;

        if (!widget::is_root() || widget::root().is_nested())
        {
            update(true);
            self.reset_origin();
            update(true);
            for (auto& child : iChildren)
                child->parent_moved();
            if ((widget_type() & neogfx::widget_type::Floating) == neogfx::widget_type::Floating)
            {
                parent().layout_items_started();
                parent().layout_items_completed();
                scoped_layout_items{};
            }
        }
        if (widget::is_root())
            widget::root().surface().move_surface(self.position());
        PositionChanged();
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::parent_moved()
    {
        auto& self = *this;

        self.reset_origin();
        for (auto& child : iChildren)
            child->parent_moved();
        ParentPositionChanged();
    }
    
    template <WidgetInterface Interface>
    inline bool widget<Interface>::resizing() const
    {
        return iResizing;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::resize(const size& aSize)
    {
        auto& self = *this;

        if (base_type::Size == units_converter{ *this }.to_device_units(aSize))
            return;

        neolib::scoped_flag sf{ iResizing };

#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << "widget<Interface>::resize(" << aSize << ")" << std::endl;
#endif // NEOGFX_DEBUG

        update(true);
        self.set_extents(aSize);
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::resized()
    {
        auto& self = *this;

        if (widget::is_root())
            widget::root().surface().resize_surface(self.extents());

        update(true);
        
        SizeChanged();
        
        neolib::scoped_optional_if soi{ layout_reason(), neogfx::layout_reason::Resize };

        layout_items();
        
        if ((widget_type() & neogfx::widget_type::Floating) == neogfx::widget_type::Floating)
        {
            parent().layout_items_started();
            parent().layout_items_completed();
            scoped_layout_items{};
        }
    }

    template <WidgetInterface Interface>
    inline rect widget<Interface>::non_client_rect() const
    {
        auto& self = *this;
        return rect{self.origin(), self.extents()};
    }

    template <WidgetInterface Interface>
    inline rect widget<Interface>::client_rect(bool aExtendIntoPadding) const
    {
        auto& self = *this;
        return base_type::to_client_rect(self.extents(), aExtendIntoPadding);
    }

    template <WidgetInterface Interface>
    inline const i_widget& widget<Interface>::get_widget_at(const point& aPosition) const
    {
        scoped_units su{ *this, units::Pixels };

        if (client_rect().contains(aPosition))
        {
            i_widget const* hitWidget = nullptr;
            for (auto const& child : children())
                if (child->visible() && to_client_coordinates(child->non_client_rect()).contains(aPosition))
                {
                    if (hitWidget == nullptr || child->layer() > hitWidget->layer())
                        hitWidget = &*child;
                }
            if (hitWidget)
            {
                auto const hitWidgetOrigin = to_client_coordinates(hitWidget->origin());
                auto const childWidgetPosition = aPosition - hitWidgetOrigin;
                return hitWidget->get_widget_at(childWidgetPosition);
            }
        }
        return *this;
    }

    template <WidgetInterface Interface>
    inline i_widget& widget<Interface>::get_widget_at(const point& aPosition)
    {
        return const_cast<i_widget&>(to_const(*this).get_widget_at(aPosition));
    }

    template <WidgetInterface Interface>
    inline widget_type widget<Interface>::widget_type() const
    {
        return neogfx::widget_type::Client;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::part_active(widget_part aPart) const
    {
        return true;
    }

    template <WidgetInterface Interface>
    inline widget_part widget<Interface>::part(const point& aPosition) const
    {
        if (client_rect().contains(aPosition))
            return widget_part{ *this, widget_part::Client };
        else if (to_client_coordinates(non_client_rect()).contains(aPosition))
            return widget_part{ *this, widget_part::NonClient };
        else
            return widget_part{ *this, widget_part::Nowhere };
    }

    template <WidgetInterface Interface>
    inline widget_part widget<Interface>::hit_test(const point& aPosition) const
    {
        return part(aPosition);
    }

    template <WidgetInterface Interface>
    inline size_policy widget<Interface>::size_policy() const
    {
        auto& self = *this;

#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::size_policy()" << std::endl;
#endif // NEOGFX_DEBUG
        if (self.has_size_policy())
            return base_type::size_policy();
        else if (self.has_fixed_size())
            return size_constraint::Fixed;
        else
            return size_constraint::Expanding;
    }

    template <WidgetInterface Interface>
    inline size widget<Interface>::minimum_size(optional_size const& aAvailableSpace) const
    {
        auto& self = *this;

#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::minimum_size(" << aAvailableSpace << ")" << std::endl;
#endif // NEOGFX_DEBUG
        size result;
        if (self.has_ideal_size() && querying_ideal_size())
            result = base_type::ideal_size(aAvailableSpace);
        else if (self.has_minimum_size() || (base_type::Anchor_MinimumSize.active() && !base_type::Anchor_MinimumSize.calculating()))
            result = base_type::minimum_size(aAvailableSpace);
        else if (has_layout())
        {
            auto const is = self.internal_spacing();
            result = layout().minimum_size(aAvailableSpace != std::nullopt ? *aAvailableSpace - is.size() : aAvailableSpace);
            if (result.cx != 0.0)
                result.cx += is.size().cx;
            if (result.cy != 0.0)
                result.cy += is.size().cy;
        }
        else
            result = self.internal_spacing().size();
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::minimum_size(" << aAvailableSpace << ") --> " << result << std::endl;
#endif // NEOGFX_DEBUG
        return result;
    }

    template <WidgetInterface Interface>
    inline size widget<Interface>::maximum_size(optional_size const& aAvailableSpace) const
    {
        auto& self = *this;

#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::maximum_size(" << aAvailableSpace << ")" << std::endl;
#endif // NEOGFX_DEBUG
        size result;
        if (self.has_maximum_size() || (base_type::Anchor_MaximumSize.active() && !base_type::Anchor_MaximumSize.calculating()))
            result = base_type::maximum_size(aAvailableSpace);
        else if (size_policy() == size_constraint::Minimum)
            result = minimum_size(aAvailableSpace);
        else if (has_layout())
        {
            auto const is = self.internal_spacing();
            result = layout().maximum_size(aAvailableSpace != std::nullopt ? *aAvailableSpace - is.size() : aAvailableSpace);
            if (result.cx != 0.0)
                result.cx += is.size().cx;
            if (result.cy != 0.0)
                result.cy += is.size().cy;
        }
        else
            result = size::max_size();
        if (size_policy().horizontal_constraint() == size_constraint::Maximum)
            result.cx = size::max_size().cx;
        if (size_policy().vertical_constraint() == size_constraint::Maximum)
            result.cy = size::max_size().cy;
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::maximum_size(" << aAvailableSpace << ") --> " << result << std::endl;
#endif // NEOGFX_DEBUG
        return result;
    }

    template <WidgetInterface Interface>
    inline padding widget<Interface>::padding() const
    {
        auto& self = *this;
        auto const& adjustedPadding = (self.has_padding() ? 
            base_type::padding() : service<i_app>().current_style().padding(widget::is_root() ? padding_role::Window : padding_role::Widget));
        return self.transformation() * units_converter{ *this }.from_device_units(adjustedPadding);
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::layout_as(const point& aPosition, const size& aSize)
    {
        auto& self = *this;

#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::layout_as(" << aPosition << ", " << aSize << ")" << std::endl;
#endif // NEOGFX_DEBUG
        move(aPosition);
        if (self.extents() != aSize)
            resize(aSize);
        else if (has_layout() && layout().invalidated())
        {
            neolib::scoped_optional_if soi{ layout_reason(), neogfx::layout_reason::Explicit };
            layout_items();
        }
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_view() const
    {
        return iView != std::nullopt;
    }

    template <WidgetInterface Interface>
    inline view widget<Interface>::view(bool aExtendIntoPadding) const
    {
        if (has_view())
            return iView.value();
        neogfx::view const defaultView = client_rect(aExtendIntoPadding);
        return defaultView;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_view(optional_view const& aView)
    {
        iView = aView;
    }

    template <WidgetInterface Interface>
    inline layer_t widget<Interface>::render_layer() const
    {
        if (iRenderLayer != std::nullopt)
            return *iRenderLayer;
        return layer();
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_render_layer(const std::optional<layer_t>& aLayer)
    {
        if (iRenderLayer != aLayer)
        {
            iRenderLayer = aLayer;
            update(true);
        }
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::can_update() const
    {
        return widget::has_root() && widget::root().has_native_surface() && !effectively_hidden() && !layout_items_in_progress();
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::update(bool aIncludeNonClient)
    {
        if (!can_update())
            return false;
        return update(aIncludeNonClient ? to_client_coordinates(non_client_rect()) : client_rect());
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::update(const rect& aUpdateRect)
    {
#ifdef NEOGFX_DEBUG
        if (debug::renderItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::update(" << aUpdateRect << ")" << std::endl;
#endif // NEOGFX_DEBUG
        if (!can_update())
            return false;
        if (aUpdateRect.empty())
            return false;
        surface().invalidate_surface(to_window_coordinates(aUpdateRect));
        return true;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::requires_update() const
    {
        return surface().has_invalidated_area() && !surface().invalidated_area().intersection(non_client_rect()).empty();
    }

    template <WidgetInterface Interface>
    inline rect widget<Interface>::update_rect() const
    {
        if (!requires_update())
            throw no_update_rect();
        return to_client_coordinates(surface().invalidated_area().intersection(non_client_rect()));
    }

    template <WidgetInterface Interface>
    inline rect widget<Interface>::default_clip_rect(bool aIncludeNonClient) const
    {
        auto& cachedRect = (aIncludeNonClient ? iDefaultNonClientClipRect : iDefaultClientClipRect);
        if (cachedRect != invalid)
            return *cachedRect;
        rect clipRect = to_client_coordinates(non_client_rect());
        if (!aIncludeNonClient)
            clipRect = clipRect.intersection(view().viewport());
        if (!widget::is_root())
            clipRect = clipRect.intersection(to_client_coordinates(parent().to_window_coordinates(parent().default_clip_rect((widget_type() & neogfx::widget_type::NonClient) == neogfx::widget_type::NonClient))));
        return *(cachedRect = clipRect);
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::ready_to_render() const
    {
        return !iLayoutPending;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::render(i_graphics_context& aGc) const
    {
        auto& self = *this;

        if (effectively_hidden())
            return;
        if (!requires_update())
            return;

        scoped_units su{ *this, units::Pixels };

        iDefaultNonClientClipRect = invalid;
        iDefaultClientClipRect = invalid;

        const rect updateRect = update_rect();
        const rect nonClientClipRect = default_clip_rect(true).intersection(updateRect);

#ifdef NEOGFX_DEBUG
        if (debug::renderItem == this)
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::render(...), updateRect: " << updateRect << ", nonClientClipRect: " << nonClientClipRect << std::endl;
#endif // NEOGFX_DEBUG

        aGc.set_extents(self.extents());
        aGc.set_origin(self.origin());

        aGc.set_default_font(font());

        scoped_snap_to_pixel snap{ aGc };
        scoped_opacity sc{ aGc, effectively_enabled() ? opacity() : opacity() * 0.75 };

        {
            scoped_scissor scissor(aGc, nonClientClipRect);

            PaintingNonClient(aGc);

            paint_non_client(aGc);

            for (auto iterChild = iChildren.rbegin(); iterChild != iChildren.rend(); ++iterChild)
            {
                auto const& childWidget = **iterChild;
                if ((childWidget.widget_type() & neogfx::widget_type::Client) == neogfx::widget_type::Client)
                    continue;
                rect intersection = nonClientClipRect.intersection(childWidget.non_client_rect() - self.origin());
                if (intersection.empty() && !childWidget.is_root())
                    continue;
                childWidget.render(aGc);
            }

            PaintedNonClient(aGc);
        }

        {
            const rect clipRect = default_clip_rect().intersection(updateRect);

            aGc.set_extents(client_rect().extents());
            aGc.set_origin(self.origin());

#ifdef NEOGFX_DEBUG
            if (debug::renderItem == this)
                service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::render(...): client_rect: " << client_rect() << ", origin: " << self.origin() << std::endl;
#endif // NEOGFX_DEBUG

            scoped_scissor scissor(aGc, clipRect);

            scoped_coordinate_system scs1(aGc, self.origin(), self.extents(), logical_coordinate_system());

            Painting(aGc);

            paint(aGc);

            scoped_coordinate_system scs2(aGc, self.origin(), self.extents(), logical_coordinate_system());

            PaintingChildren(aGc);

            typedef std::map<std::int32_t, std::vector<i_widget const*>> widget_layers_t;
            shared_thread_local(std::vector<std::unique_ptr<widget_layers_t>>, neogfx::widget::render, widgetLayersStack);

            shared_thread_local(std::size_t, neogfx::widget::render, stack);
            neolib::scoped_counter<std::size_t> stackCounter{ stack };
            if (widgetLayersStack.size() < stack)
                widgetLayersStack.push_back(std::make_unique<widget_layers_t>());

            widget_layers_t& widgetLayers = *widgetLayersStack[stack - 1];

            for (auto& layer : widgetLayers)
                layer.second.clear();

            for (auto iterChild = iChildren.rbegin(); iterChild != iChildren.rend(); ++iterChild)
            {
                auto const& childWidget = **iterChild;
                if ((childWidget.widget_type() & neogfx::widget_type::NonClient) == neogfx::widget_type::NonClient)
                    continue;
                rect intersection = clipRect.intersection(to_client_coordinates(childWidget.non_client_rect()));
                if (intersection.empty() && !childWidget.is_root())
                    continue;
                widgetLayers[childWidget.render_layer()].push_back(&childWidget);
            }
                
            for (auto const& layer : widgetLayers)
            {
                for (auto const& childWidgetPtr : layer.second)
                {
                    auto const& childWidget = *childWidgetPtr;
                    childWidget.render(aGc);
                }
            }

            aGc.set_extents(client_rect().extents());
            aGc.set_origin(self.origin());

            scoped_coordinate_system scs3(aGc, self.origin(), self.extents(), logical_coordinate_system());

            Painted(aGc);
        }

        aGc.set_extents(self.extents());
        aGc.set_origin(self.origin());
        {
            scoped_scissor scissor(aGc, nonClientClipRect);
            paint_non_client_after(aGc);
        }
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::paint_non_client(i_graphics_context& aGc) const
    {
        auto& self = *this;

        auto const& updateRect = update_rect();

#ifdef NEOGFX_DEBUG
        if (debug::renderItem == this)
        {
            aGc.flush();
            service<debug::logger>() << neolib::logger::severity::Debug << typeid(*this).name() << "::paint_non_client(...), updateRect: " << updateRect << std::endl;
        }
#endif // NEOGFX_DEBUG

        if (self.has_background_color() || !self.background_is_transparent())
        {
            auto const backgroundColor = self.background_color().with_combined_alpha(has_background_opacity() ? background_opacity() : 1.0);
            aGc.fill_rect(updateRect, backgroundColor);
        }
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::paint_non_client_after(i_graphics_context& aGc) const
    {
        auto& self = *this;

#ifdef NEOGFX_DEBUG
        // todo: move to debug::layoutItem function/service
        if (debug::renderItem == this || debug::layoutItem == this || (debug::layoutItem != nullptr && has_layout() && debug::layoutItem->is_layout() &&
            (debug::layoutItem == &layout() || static_cast<i_layout const*>(debug::layoutItem)->is_descendent_of(layout()))))
        {
            neogfx::font debugFont1 = service<i_app>().current_style().font().with_size(16);
            neogfx::font debugFont2 = service<i_app>().current_style().font().with_size(8);
            if (debug::renderGeometryText)
            {
                if (debug::layoutItem == this)
                {
                    aGc.draw_text(self.position(), typeid(*this).name(), debugFont1, text_format{ color::Yellow.with_alpha(0.75), text_effect{ text_effect_type::Outline, color::Black.with_alpha(0.75), 2.0 } });
                    std::ostringstream oss;
                    oss << "sizepol: " << size_policy();
                    oss << " minsize: " << minimum_size() << " maxsize: " << maximum_size();
                    oss << " fixsize: " << (self.has_fixed_size() ? self.fixed_size() : optional_size{}) << " weight: " << self.weight() << " extents: " << self.extents();
                    aGc.draw_text(self.position() + size{ 0.0, debugFont1.height() }, oss.str(), debugFont2, text_format{ color::Orange.with_alpha(0.75), text_effect{ text_effect_type::Outline, color::Black.with_alpha(0.75), 2.0 } });
                }
            }
            rect const nonClientRect = to_client_coordinates(non_client_rect());
            aGc.draw_rect(nonClientRect, pen{ color::White, 3.0 });
            aGc.draw_rect(nonClientRect, pen{ color::Green, 3.0, line_dash{ 0x5555u } });
            if (nonClientRect != client_rect(false))
            {
                aGc.draw_rect(client_rect(false), pen{ color::White, 1.0 });
                aGc.draw_rect(client_rect(false), pen{ color::Red, 1.0, line_dash{ 0x5555u } });
            }
            if (debug::layoutItem != nullptr && (debug::layoutItem != this || has_layout()))
            {
                i_layout const& debugLayout = (debug::layoutItem == this ? layout() : *static_cast<i_layout const*>(debug::layoutItem));
                if (debug::renderGeometryText)
                {
                    if (debug::layoutItem != this)
                    {
                        aGc.draw_text(debugLayout.position(), typeid(debugLayout).name(), debugFont1, text_format{ color::Yellow.with_alpha(0.75), text_effect{ text_effect_type::Outline, color::Black.with_alpha(0.75), 2.0 } });
                        std::ostringstream oss;
                        oss << "sizepol: " << debugLayout.size_policy();
                        oss << " minsize: " << debugLayout.minimum_size() << " maxsize: " << debugLayout.maximum_size();
                        oss << " fixsize: " << (debugLayout.has_fixed_size() ? debugLayout.fixed_size() : optional_size{}) << " weight: " << debugLayout.weight() << " extents: " << debugLayout.extents();
                        aGc.draw_text(debugLayout.position() + size{ 0.0, debugFont1.height() }, oss.str(), debugFont2, text_format{ color::Orange.with_alpha(0.75), text_effect{ text_effect_type::Outline, color::Black.with_alpha(0.75), 2.0 } });
                    }
                }
                for (layout_item_index itemIndex = 0; itemIndex < debugLayout.count(); ++itemIndex)
                {
                    auto const& item = debugLayout.item_at(itemIndex);
                    if (debug::renderGeometryText)
                    {
                        std::string text = typeid(item).name();
                        auto* l = &item;
                        while (l->has_parent_layout())
                        {
                            l = &l->parent_layout();
                            text = typeid(*l).name() + " > "_s + text;
                        }
                        aGc.draw_text(item.position(), text, debugFont2, text_format{ color::White.with_alpha(0.5), text_effect{ text_effect_type::Outline, color::Black.with_alpha(0.5), 2.0 } });
                    }
                    rect const itemRect{ item.position(), item.extents() };
                    aGc.draw_rect(itemRect, color::White.with_alpha(0.5));
                    aGc.draw_rect(itemRect, pen{ color::Black.with_alpha(0.5), line_dash{ 0x5555u } });
                }
                rect const layoutRect{ debugLayout.position(), debugLayout.extents() };
                aGc.draw_rect(layoutRect, color::White);
                aGc.draw_rect(layoutRect, pen{ debug::layoutItem == &layout() ? color::Blue : color::Purple, line_dash{ 0x5555u } });
            }
        }
#endif // NEOGFX_DEBUG
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::paint(i_graphics_context& aGc) const
    {
        // do nothing
    }

    template <WidgetInterface Interface>
    inline double widget<Interface>::opacity() const
    {
        return Opacity;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_opacity(double aOpacity)
    {
        if (Opacity != aOpacity)
        {
            Opacity = aOpacity;
            update(true);
        }
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_background_opacity() const
    {
        return BackgroundOpacity != std::nullopt;
    }

    template <WidgetInterface Interface>
    inline double widget<Interface>::background_opacity() const
    {
        if (has_background_opacity())
            return *BackgroundOpacity.value();
        return 0.0;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_background_opacity(double aOpacity)
    {
        if (BackgroundOpacity != aOpacity)
        {
            BackgroundOpacity = aOpacity;
            update(true);
        }
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_palette() const
    {
        return Palette != std::nullopt;
    }

    template <WidgetInterface Interface>
    inline const i_palette& widget<Interface>::palette() const
    {
        if (has_palette())
            return *Palette.value();
        return service<i_app>().current_style().palette();
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_palette(const i_palette& aPalette)
    {
        if (Palette != aPalette)
        {
            Palette = aPalette;
            update(true);
        }
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_palette_color(color_role aColorRole) const
    {
        return has_palette() && palette().has_color(aColorRole);
    }

    template <WidgetInterface Interface>
    inline color widget<Interface>::palette_color(color_role aColorRole) const
    {
        return palette().color(aColorRole);
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_palette_color(color_role aColorRole, const optional_color& aColor)
    {
        if (Palette == std::nullopt)
            Palette = neogfx::palette{ current_style_palette_proxy() };
        if (palette_color(aColorRole) != aColor)
        {
            auto existing = neogfx::palette{ palette() }; // todo: support indirectly changing and notifying a property so we don't have to make a copy?
            existing.set_color(aColorRole, aColor);
            Palette = existing;
            update(true);
        }
    }

    template <WidgetInterface Interface>
    inline color widget<Interface>::container_background_color() const
    {
        const i_widget* w = this;
        while (w->background_is_transparent() && w->has_parent())
            w = &w->parent();
        if (!w->background_is_transparent() && w->has_background_color())
            return w->background_color();
        else
            return service<i_app>().current_style().palette().color(color_role::Theme);
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_font_role() const
    {
        return FontRole != std::nullopt;
    }

    template <WidgetInterface Interface>
    inline font_role widget<Interface>::font_role() const
    {
        if (has_font_role())
            return *FontRole.value();
        return neogfx::font_role::Widget;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_font_role(const optional_font_role& aFontRole)
    {
        if (FontRole != aFontRole)
        {
            FontRole = aFontRole;
            update(true);
        }
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_font() const
    {
        return Font != std::nullopt;
    }

    template <WidgetInterface Interface>
    inline const font& widget<Interface>::font() const
    {
        if (has_font())
            return *Font;
        else
            return service<i_app>().current_style().font(font_role());
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_font(optional_font const& aFont)
    {
        auto& self = *this;

        if (Font != aFont)
        {
            Font = aFont;
            self.update_layout();
            update(true);
        }
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::visible() const
    {
        return Visible && (base_type::MaximumSize == std::nullopt || (base_type::MaximumSize->cx != 0.0 && base_type::MaximumSize->cy != 0.0));
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::effectively_visible() const
    {
        return visible() && (widget::is_root() || !has_parent() || parent().effectively_visible());
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::hidden() const
    {
        return !visible();
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::effectively_hidden() const
    {
        return !effectively_visible();
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::show(bool aVisible)
    {
        auto& self = *this;

        if (Visible != aVisible)
        {
            bool isEntered = entered();
            Visible = aVisible;
            if (!visible() && isEntered)
            {
                if (!widget::is_root())
                    widget::root().as_widget().mouse_entered(widget::root().mouse_position());
                else
                    mouse_left();
            }
            VisibilityChanged();
            if (effectively_hidden())
                release_focus();
            self.update_layout(true, true);
            return true;
        }
        return false;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::enabled() const
    {
        return Enabled;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::effectively_enabled() const
    {
        return enabled() && (widget::is_root() || !has_parent() || parent().effectively_enabled());
    }
    
    template <WidgetInterface Interface>
    inline bool widget<Interface>::disabled() const
    {
        return !enabled();
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::effectively_disabled() const
    {
        return !effectively_enabled();
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::enable(bool aEnable)
    {
        if (Enabled != aEnable)
        {
            bool isEntered = entered();
            Enabled = aEnable;
            if (!enabled() && isEntered)
            {
                if (!widget::is_root())
                    widget::root().as_widget().mouse_entered(widget::root().mouse_position());
                else
                    mouse_left();
            }
            update(true);
            return true;
        }
        return false;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::entered(bool aChildEntered) const
    {
        return widget::has_root() && widget::root().has_entered_widget() && (&widget::root().entered_widget() == this || (aChildEntered && widget::root().entered_widget().is_descendent_of(*this)));
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::can_capture() const
    {
        return true;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::capture_locks_cursor() const
    {
        return true;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::capturing() const
    {
        return surface().as_surface_window().has_capturing_widget() && &surface().as_surface_window().capturing_widget() == this;
    }

    template <WidgetInterface Interface>
    inline const optional_point& widget<Interface>::capture_position() const
    {
        return iCapturePosition;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_capture(capture_reason aReason, const optional_point& aPosition)
    {
        auto& self = *this;

        if (can_capture())
        {
            switch (aReason)
            {
            case capture_reason::MouseEvent:
                if (!self.mouse_event_is_non_client())
                    surface().as_surface_window().set_capture(*this);
                else
                    surface().as_surface_window().non_client_set_capture(*this);
                break;
            default:
                surface().as_surface_window().set_capture(*this);
                break;
            }
            iCapturePosition = aPosition;
        }
        else
            throw widget_cannot_capture();
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::release_capture(capture_reason aReason)
    {
        auto& self = *this;

        switch (aReason)
        {
        case capture_reason::MouseEvent:
            if (!self.mouse_event_is_non_client())
                surface().as_surface_window().release_capture(*this);
            else
                surface().as_surface_window().non_client_release_capture(*this);
            break;
        default:
            surface().as_surface_window().release_capture(*this);
            break;
        }
        iCapturePosition = std::nullopt;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::non_client_set_capture()
    {
        if (can_capture())
            surface().as_surface_window().non_client_set_capture(*this);
        else
            throw widget_cannot_capture();
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::non_client_release_capture()
    {
        surface().as_surface_window().non_client_release_capture(*this);
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::captured()
    {
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::capture_released()
    {
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_focus_policy() const
    {
        return FocusPolicy != std::nullopt;
    }

    template <WidgetInterface Interface>
    inline focus_policy widget<Interface>::focus_policy() const
    {
        if (has_focus_policy())
            return *FocusPolicy;
        return neogfx::focus_policy::NoFocus;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_focus_policy(optional_focus_policy const& aFocusPolicy)
    {
        FocusPolicy = aFocusPolicy;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::can_set_focus(focus_reason aFocusReason) const
    {
        if (effectively_hidden() || effectively_disabled())
            return false;
        switch (aFocusReason)
        {
        case focus_reason::ClickNonClient:
            return (focus_policy() & (neogfx::focus_policy::ClickFocus | neogfx::focus_policy::IgnoreNonClient)) == neogfx::focus_policy::ClickFocus;
        case focus_reason::ClickClient:
            return (focus_policy() & neogfx::focus_policy::ClickFocus) == neogfx::focus_policy::ClickFocus;
        case focus_reason::Tab:
            return (focus_policy() & neogfx::focus_policy::TabFocus) == neogfx::focus_policy::TabFocus;
        case focus_reason::Wheel:
            return (focus_policy() & neogfx::focus_policy::WheelFocus) == neogfx::focus_policy::WheelFocus;
        case focus_reason::Pointer:
            return (focus_policy() & neogfx::focus_policy::PointerFocus) == neogfx::focus_policy::PointerFocus;
        case focus_reason::WindowActivation:
        case focus_reason::Other:
        default:
            return focus_policy() != neogfx::focus_policy::NoFocus;
        }
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::has_focus() const
    {
        return widget::has_root() && widget::root().is_active() && widget::root().has_focused_widget() && &widget::root().focused_widget() == this;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::child_has_focus() const
    {
        return widget::has_root() && widget::root().is_active() && widget::root().has_focused_widget() && widget::root().focused_widget().is_descendent_of(*this);
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::set_focus(focus_reason aFocusReason)
    {
        if (!has_focus() && effectively_enabled() && effectively_visible())
        {
            widget::root().set_focused_widget(*this, aFocusReason);
            return true;
        }
        return false;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::release_focus()
    {
        if (has_focus() || child_has_focus())
        {
            widget::root().release_focused_widget(widget::root().focused_widget());
            return true;
        }
        return false;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::focus_gained(focus_reason aReason)
    {
        update(true);
        Focus(focus_event::FocusGained, aReason);
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::focus_lost(focus_reason aReason)
    {
        update(true);
        Focus(focus_event::FocusLost, aReason);
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::consider_ancestors_for_mouse_events() const
    {
        return ConsiderAncestorsForMouseEvents;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_consider_ancestors_for_mouse_events(bool aConsiderAncestors)
    {
        ConsiderAncestorsForMouseEvents = aConsiderAncestors;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::ignore_mouse_events(bool aConsiderAncestors) const
    {
        return IgnoreMouseEvents || (aConsiderAncestors && consider_ancestors_for_mouse_events() && 
            has_parent() && parent().ignore_mouse_events());
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_ignore_mouse_events(bool aIgnoreMouseEvents)
    {
        IgnoreMouseEvents = aIgnoreMouseEvents;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::ignore_non_client_mouse_events(bool aConsiderAncestors) const
    {
        return IgnoreNonClientMouseEvents || (aConsiderAncestors && consider_ancestors_for_mouse_events() &&
            has_parent() && parent().ignore_non_client_mouse_events());
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::set_ignore_non_client_mouse_events(bool aIgnoreNonClientMouseEvents)
    {
        IgnoreNonClientMouseEvents = aIgnoreNonClientMouseEvents;
    }

    template <WidgetInterface Interface>
    inline mouse_event_location widget<Interface>::mouse_event_location() const
    {
        if (widget::has_root() && widget::root().has_native_surface())
            return surface().as_surface_window().current_mouse_event_location();
        else
            return neogfx::mouse_event_location::None;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifiers_e aKeyModifiers)
    {
        auto& self = *this;

        if (has_parent() && same_surface(parent()))
            return parent().mouse_wheel_scrolled(aWheel, aPosition + self.position(), aDelta, aKeyModifiers);

        return false;
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        auto& self = *this;

        if (aButton == mouse_button::Middle && has_parent())
            parent().mouse_button_clicked(aButton, aPosition + self.position(), aKeyModifiers);
        else if (aButton == mouse_button::Left && capture_ok(hit_test(aPosition)) && can_capture())
            set_capture(capture_reason::MouseEvent, aPosition);
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        auto& self = *this;

        if (aButton == mouse_button::Middle && has_parent())
            parent().mouse_button_double_clicked(aButton, aPosition + self.position(), aKeyModifiers);
        else if (aButton == mouse_button::Left && capture_ok(hit_test(aPosition)) && can_capture())
            set_capture(capture_reason::MouseEvent, aPosition);
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::mouse_button_click(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        auto& self = *this;

        if (aButton == mouse_button::Middle && has_parent())
            parent().mouse_button_click(aButton, aPosition + self.position(), aKeyModifiers);
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::mouse_button_double_click(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        auto& self = *this;

        if (aButton == mouse_button::Middle && has_parent())
            parent().mouse_button_double_click(aButton, aPosition + self.position(), aKeyModifiers);
        else if (aButton == mouse_button::Left && capture_ok(hit_test(aPosition)) && can_capture())
            set_capture(capture_reason::MouseEvent, aPosition);
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        auto& self = *this;

        if (aButton == mouse_button::Middle && has_parent())
            parent().mouse_button_released(aButton, aPosition + self.position());
        else if (capturing())
            release_capture(capture_reason::MouseEvent);
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::mouse_moved(const point&, key_modifiers_e)
    {
        // do nothing
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::mouse_entered(const point&)
    {
        // do nothing
    }

    template <WidgetInterface Interface>
    inline void widget<Interface>::mouse_left()
    {
        // do nothing
    }

    template <WidgetInterface Interface>
    inline point widget<Interface>::mouse_position() const
    {
        auto& self = *this;

        auto const rootMousePosition = widget::root().mouse_position();
        if (widget::is_root())
            return rootMousePosition;
        else
            return rootMousePosition + widget::root().origin() - self.origin();
    }

    template <WidgetInterface Interface>
    inline mouse_cursor widget<Interface>::mouse_cursor() const
    {
        std::optional<neogfx::mouse_cursor> mouseCursor;
        auto const mousePosition = mouse_position();
        auto const partUnderMouse = part(mousePosition);
        if (part_active(partUnderMouse))
        {
            switch (partUnderMouse.part)
            {
            case widget_part::BorderLeft:
                mouseCursor = mouse_system_cursor::SizeWE;
                break;
            case widget_part::BorderTopLeft:
                mouseCursor = mouse_system_cursor::SizeNWSE;
                break;
            case widget_part::BorderTop:
                mouseCursor = mouse_system_cursor::SizeNS;
                break;
            case widget_part::BorderTopRight:
                mouseCursor = mouse_system_cursor::SizeNESW;
                break;
            case widget_part::BorderRight:
                mouseCursor = mouse_system_cursor::SizeWE;
                break;
            case widget_part::BorderBottomRight:
                mouseCursor = mouse_system_cursor::SizeNWSE;
                break;
            case widget_part::BorderBottom:
                mouseCursor = mouse_system_cursor::SizeNS;
                break;
            case widget_part::BorderBottomLeft:
                mouseCursor = mouse_system_cursor::SizeNESW;
                break;
            case widget_part::GrowBox:
                mouseCursor = mouse_system_cursor::SizeNWSE;
                break;
            case widget_part::VerticalScrollbar:
                mouseCursor = mouse_system_cursor::Arrow;
                break;
            case widget_part::HorizontalScrollbar:
                mouseCursor = mouse_system_cursor::Arrow;
                break;
            }
        }
        if (!mouseCursor && has_parent())
            mouseCursor = parent().mouse_cursor();
        if (!mouseCursor)
            mouseCursor = mouse_system_cursor::Arrow;
        QueryMouseCursor(*mouseCursor);
        return mouseCursor.value();
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::key_pressed(scan_code_e, key_code_e, key_modifiers_e)
    {
        return false;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::key_released(scan_code_e, key_code_e, key_modifiers_e)
    {
        return false;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::text_input(i_string const&)
    {
        return false;
    }

    template <WidgetInterface Interface>
    inline bool widget<Interface>::sys_text_input(i_string const&)
    {
        return false;
    }

    template <WidgetInterface Interface>
    inline const i_widget& widget<Interface>::widget_for_mouse_event(const point& aPosition, bool aForHitTest) const
    {
        auto& self = *this;

        scoped_units su{ *this, units::Pixels };

        auto const clientPosition = aPosition - self.origin();
        const i_widget* result = nullptr;
        if (widget::is_root() && (widget::root().style() & window_style::Resize) == window_style::Resize)
        {
            auto const outerRect = to_client_coordinates(non_client_rect());
            auto const innerRect = outerRect.deflated(widget::root().window_border());
            if (outerRect.contains(clientPosition) && !innerRect.contains(clientPosition))
                result = this;
        }
        if (!result && non_client_rect().contains(aPosition))
        {
            auto const location = mouse_event_location();
            const i_widget* w = &get_widget_at(clientPosition);
            for (; w != this ; w = &w->parent()) 
            {
                auto const widgetClientPosition = aPosition - w->origin();
                if (w->effectively_hidden() || (w->effectively_disabled() && !aForHitTest))
                    continue;
                switch (w->part(widgetClientPosition).part)
                {
                case widget_part::Nowhere:
                    continue;
                case widget_part::TitleBar:
                case widget_part::BorderBottomRight:
                    if (w->root().is_nested())
                        return w->root();
                    break;
                default:
                    break;
                }
                if (!w->ignore_mouse_events() && location != neogfx::mouse_event_location::NonClient)
                    break;
                if (!w->ignore_non_client_mouse_events() && location == neogfx::mouse_event_location::NonClient)
                    break;
                if (location != neogfx::mouse_event_location::NonClient &&
                    w->ignore_mouse_events() && !w->ignore_non_client_mouse_events(false) &&
                    w->non_client_rect().deflated(w->internal_spacing()).contains(aPosition) &&
                    !w->client_rect(false).contains(widgetClientPosition))
                    break;
            }
            result = w;
        }
        else if (!result)
            result = this;
        return *result;
    }

    template <WidgetInterface Interface>
    inline i_widget& widget<Interface>::widget_for_mouse_event(const point& aPosition, bool aForHitTest)
    {
        return const_cast<i_widget&>(to_const(*this).widget_for_mouse_event(aPosition, aForHitTest));
    }
}

