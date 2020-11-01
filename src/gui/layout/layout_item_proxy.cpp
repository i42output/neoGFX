// layout_item_proxy.cpp
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
#include <neogfx/gui/layout/layout_item_proxy.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/layout/i_spacer.hpp>
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
    layout_item_proxy::layout_item_proxy(i_layout_item& aItem) :
        layout_item_proxy{ std::shared_ptr<i_layout_item>{ std::shared_ptr<i_layout_item>{}, &aItem } } 
    {
        set_alive();
    }

    layout_item_proxy::layout_item_proxy(std::shared_ptr<i_layout_item> aItem) :
        iSubject{ aItem }, 
        iSubjectIsProxy{ aItem->is_proxy() }, 
        iSizePolicy{ static_cast<uint32_t>(-1), { size_constraint::Minimum } }, 
        iWeight{ static_cast<uint32_t>(-1), {} },
        iMinimumSize{ static_cast<uint32_t>(-1), {} },
        iMaximumSize{ static_cast<uint32_t>(-1), {} }
    {
        set_alive();
    }

    layout_item_proxy::layout_item_proxy(const layout_item_proxy& aOther) :
        iSubject{ aOther.iSubject }, 
        iSubjectIsProxy{ aOther.iSubject->is_proxy() }, 
        iSizePolicy{ static_cast<uint32_t>(-1), { size_constraint::Minimum } }, 
        iWeight{ static_cast<uint32_t>(-1), {} },
        iMinimumSize{ static_cast<uint32_t>(-1), {} },
        iMaximumSize{ static_cast<uint32_t>(-1), {} }
    {
        set_alive();
    }

    layout_item_proxy::~layout_item_proxy()
    {
    }

    const i_layout_item& layout_item_proxy::subject() const
    {
        if (iSubject->is_proxy())
            return iSubject->proxy_for_layout().subject();
        return *iSubject;
    }

    i_layout_item& layout_item_proxy::subject()
    {
        if (iSubject->is_proxy())
            return iSubject->proxy_for_layout().subject();
        return *iSubject;
    }

    std::shared_ptr<i_layout_item> layout_item_proxy::subject_ptr()
    {
        return iSubject;
    }

    layout_item_disposition& layout_item_proxy::cached_disposition() const
    {
        return iCachedDisposition;
    }

    void layout_item_proxy::anchor_to(i_anchorable& aRhs, const i_string& aLhsAnchor, anchor_constraint_function aLhsFunction, const i_string& aRhsAnchor, anchor_constraint_function aRhsFunction)
    {
        subject().anchor_to(aRhs, aLhsAnchor, aLhsFunction, aRhsAnchor, aRhsFunction);
    }

    const layout_item_proxy::anchor_map_type& layout_item_proxy::anchors() const
    {
        return subject().anchors();
    }

    layout_item_proxy::anchor_map_type& layout_item_proxy::anchors()
    {
        return subject().anchors();
    }

    bool layout_item_proxy::is_layout() const
    {
        return subject().is_layout();
    }

    const i_layout& layout_item_proxy::as_layout() const
    {
        return subject().as_layout();
    }

    i_layout& layout_item_proxy::as_layout()
    {
        return subject().as_layout();
    }

    bool layout_item_proxy::is_widget() const
    {
        return subject().is_widget();
    }

    const i_widget& layout_item_proxy::as_widget() const
    {
        return subject().as_widget();
    }

    i_widget& layout_item_proxy::as_widget()
    {
        return subject().as_widget();
    }

    bool layout_item_proxy::is_spacer() const
    {
        return !is_layout() && !is_widget();
    }

    bool layout_item_proxy::has_parent_layout() const
    {
        return subject().has_parent_layout();
    }

    const i_layout& layout_item_proxy::parent_layout() const
    {
        if (has_parent_layout())
            return subject().parent_layout();
        throw no_parent_layout();
    }

    i_layout& layout_item_proxy::parent_layout()
    {
        return const_cast<i_layout&>(to_const(*this).parent_layout());
    }

    void layout_item_proxy::set_parent_layout(i_layout* aParentLayout)
    {
        if (!subject_is_proxy())
            subject().set_parent_layout(aParentLayout);
    }

    bool layout_item_proxy::has_layout_owner() const
    {
        return subject().has_layout_owner();
    }

    const i_widget& layout_item_proxy::layout_owner() const
    {
        if (has_layout_owner())
            return subject().layout_owner();
        throw no_layout_owner();
    }

    i_widget& layout_item_proxy::layout_owner()
    {
        return const_cast<i_widget&>(to_const(*this).layout_owner());
    }

    void layout_item_proxy::set_layout_owner(i_widget* aOwner)
    {
        if (!subject_is_proxy())
            subject().set_layout_owner(aOwner);
    }

    bool layout_item_proxy::has_layout_manager() const
    {
        return subject().has_layout_manager();
    }

    const i_widget& layout_item_proxy::layout_manager() const
    {
        if (has_layout_manager())
            return subject().layout_manager();
        throw no_layout_manager();
    }

    i_widget& layout_item_proxy::layout_manager()
    {
        return const_cast<i_widget&>(to_const(*this).layout_manager());
    }

    bool layout_item_proxy::is_proxy() const
    {
        return true;
    }

    const i_layout_item_proxy& layout_item_proxy::proxy_for_layout() const
    {
        return *this;
    }

    i_layout_item_proxy& layout_item_proxy::proxy_for_layout()
    {
        return *this;
    }

    void layout_item_proxy::update_layout(bool aDeferLayout)
    {
        subject().update_layout(aDeferLayout);
    }

    void layout_item_proxy::layout_as(const point& aPosition, const size& aSize)
    {
        point adjustedPosition = aPosition;
        size adjustedSize = aSize.min(maximum_size());
        if (adjustedSize != aSize)
        {
            adjustedPosition += point{
            (parent_layout().alignment() & alignment::Center) == alignment::Center ?
                (aSize - adjustedSize).cx / 2.0 :
                (parent_layout().alignment() & alignment::Right) == alignment::Right ?
                    (aSize - adjustedSize).cx :
                    0.0,
            (parent_layout().alignment() & alignment::VCenter) == alignment::VCenter ?
                (aSize - adjustedSize).cy / 2.0 :
                (parent_layout().alignment() & alignment::Bottom) == alignment::Bottom ?
                    (aSize - adjustedSize).cy :
                    0.0 }.floor();
        }

        subject().layout_as(adjustedPosition, adjustedSize);
    }

    void layout_item_proxy::fix_weightings(bool aRecalculate)
    {
        subject().fix_weightings(aRecalculate);
    }

    bool layout_item_proxy::high_dpi() const
    {
        return subject().high_dpi();
    }

    dimension layout_item_proxy::dpi_scale_factor() const
    {
        return subject().dpi_scale_factor();
    }

    bool layout_item_proxy::device_metrics_available() const
    {
        return parent_layout().device_metrics_available();
    }

    const i_device_metrics& layout_item_proxy::device_metrics() const
    {
        return parent_layout().device_metrics();
    }

    point layout_item_proxy::position() const
    {
        return subject().position();
    }

    void layout_item_proxy::set_position(const point& aPosition)
    {
        subject().set_position(aPosition);
    }

    size layout_item_proxy::extents() const
    {
        return subject().extents();
    }

    void layout_item_proxy::set_extents(const size& aExtents)
    {
        subject().set_extents(aExtents);
    }

    bool layout_item_proxy::has_size_policy() const
    {
        return subject().has_size_policy();
    }

    size_policy layout_item_proxy::size_policy() const
    {
#ifdef NEOGFX_DEBUG
        if (&subject() == debug::layoutItem)
            service<debug::logger>() << "layout_item_proxy::size_policy()" << endl;
#endif // NEOGFX_DEBUG
        auto& cachedSizePolicy = iSizePolicy.second;
        if (iSizePolicy.first != global_layout_id())
        {
            cachedSizePolicy = subject().size_policy();
            iSizePolicy.first = global_layout_id();
        }
#ifdef NEOGFX_DEBUG
        if (&subject() == debug::layoutItem)
            service<debug::logger>() << "layout_item_proxy::size_policy() -> " << cachedSizePolicy << endl;
#endif // NEOGFX_DEBUG
        return cachedSizePolicy;
    }

    void layout_item_proxy::set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout)
    {
        subject().set_size_policy(aSizePolicy, aUpdateLayout);
    }

    bool layout_item_proxy::has_weight() const
    {
        return subject().has_weight();
    }

    size layout_item_proxy::weight() const
    {
#ifdef NEOGFX_DEBUG
        if (&subject() == debug::layoutItem)
            service<debug::logger>() << "layout_item_proxy::weight()" << endl;
#endif // NEOGFX_DEBUG
        auto& cachedWeight = iWeight.second;
        if (iWeight.first != global_layout_id())
        {
            cachedWeight = subject().weight();
            iWeight.first = global_layout_id();
        }
#ifdef NEOGFX_DEBUG
        if (&subject() == debug::layoutItem)
            service<debug::logger>() << "layout_item_proxy::weight() -> " << cachedWeight << endl;
#endif // NEOGFX_DEBUG
        return cachedWeight;
    }

    void layout_item_proxy::set_weight(optional_size const& aWeight, bool aUpdateLayout)
    {
        subject().set_weight(aWeight, aUpdateLayout);
    }

    bool layout_item_proxy::has_minimum_size() const
    {
        return subject().has_minimum_size();
    }

    size layout_item_proxy::minimum_size(optional_size const& aAvailableSpace) const
    {
#ifdef NEOGFX_DEBUG
        if (&subject() == debug::layoutItem)
            service<debug::logger>() << "layout_item_proxy::minimum_size(" << aAvailableSpace << ")" << endl;
#endif // NEOGFX_DEBUG
        if (!visible())
            return size{};
        auto& cachedMinSize = iMinimumSize.second;
        if (iMinimumSize.first != global_layout_id())
        {
#ifdef NEOGFX_DEBUG
            if (&subject() == debug::layoutItem)
                service<debug::logger>() << "layout_item_proxy::minimum_size(" << aAvailableSpace << ") (cache invalid)" << endl;
#endif // NEOGFX_DEBUG
            if (iMinimumSizeAnchor == std::nullopt)
            {   
                auto anchorIter = anchors().find(string{ "MinimumSize" });
                if (anchorIter != anchors().end())
                    iMinimumSizeAnchor = static_cast<const neolib::optional_t<decltype(iMinimumSizeAnchor)>>(anchorIter->second());
                else
                    iMinimumSizeAnchor = nullptr;
            }
            if (*iMinimumSizeAnchor == nullptr)
                cachedMinSize = subject().minimum_size(aAvailableSpace);
            else
                cachedMinSize = (**iMinimumSizeAnchor).evaluate_constraints(aAvailableSpace);
            if (effective_size_policy().maintain_aspect_ratio())
            {
                auto const& aspectRatio = effective_size_policy().aspect_ratio();
                if (aspectRatio.cx < aspectRatio.cy)
                {
                    if (cachedMinSize.cx < cachedMinSize.cy)
                        cachedMinSize = size{ cachedMinSize.cx, cachedMinSize.cx * (aspectRatio.cy / aspectRatio.cx) };
                    else
                        cachedMinSize = size{ cachedMinSize.cy * (aspectRatio.cx / aspectRatio.cy), cachedMinSize.cy };
                }
                else
                {
                    if (cachedMinSize.cx < cachedMinSize.cy)
                        cachedMinSize = size{ cachedMinSize.cy * (aspectRatio.cx / aspectRatio.cy), cachedMinSize.cy };
                    else
                        cachedMinSize = size{ cachedMinSize.cx, cachedMinSize.cx * (aspectRatio.cy / aspectRatio.cx) };
                }
            }
            cachedMinSize = subject().apply_fixed_size(cachedMinSize);
            iMinimumSize.first = global_layout_id();
        }
#ifdef NEOGFX_DEBUG
        if (&subject() == debug::layoutItem)
            service<debug::logger>() << "layout_item_proxy::minimum_size(" << aAvailableSpace << ") -> " << cachedMinSize << endl;
#endif // NEOGFX_DEBUG
        return cachedMinSize;
    }

    void layout_item_proxy::set_minimum_size(optional_size const& aMinimumSize, bool aUpdateLayout)
    {
        subject().set_minimum_size(aMinimumSize, aUpdateLayout);
        if (aMinimumSize != std::nullopt)
            iMinimumSize.second = *aMinimumSize;
    }

    bool layout_item_proxy::has_maximum_size() const
    {
        return subject().has_maximum_size();
    }

    size layout_item_proxy::maximum_size(optional_size const& aAvailableSpace) const
    {
#ifdef NEOGFX_DEBUG
        if (&subject() == debug::layoutItem)
            service<debug::logger>() << "layout_item_proxy::maximum_size(" << aAvailableSpace << ")" << endl;
#endif // NEOGFX_DEBUG
        if (!visible())
            return size::max_size();
        auto& cachedMaxSize = iMaximumSize.second;
        if (iMaximumSize.first != global_layout_id())
        {
#ifdef NEOGFX_DEBUG
            if (&subject() == debug::layoutItem)
                service<debug::logger>() << "layout_item_proxy::maximum_size(" << aAvailableSpace << ") (cache invalid)" << endl;
#endif // NEOGFX_DEBUG
            cachedMaxSize = subject().apply_fixed_size(subject().maximum_size(aAvailableSpace));
            iMaximumSize.first = global_layout_id();
        }
#ifdef NEOGFX_DEBUG
        if (&subject() == debug::layoutItem)
            service<debug::logger>() << "layout_item_proxy::maximum_size(" << aAvailableSpace << ") -> " << cachedMaxSize << endl;
#endif // NEOGFX_DEBUG
        return cachedMaxSize;
    }

    void layout_item_proxy::set_maximum_size(optional_size const& aMaximumSize, bool aUpdateLayout)
    {
        subject().set_maximum_size(aMaximumSize, aUpdateLayout);
        if (aMaximumSize != std::nullopt)
            iMaximumSize.second = *aMaximumSize;
    }

    bool layout_item_proxy::has_fixed_size() const
    {
        return subject().has_fixed_size();
    }

    size layout_item_proxy::fixed_size(optional_size const& aAvailableSpace) const
    {
#ifdef NEOGFX_DEBUG
        if (&subject() == debug::layoutItem)
            service<debug::logger>() << "layout_item_proxy::fixed_size(" << aAvailableSpace << ")" << endl;
#endif // NEOGFX_DEBUG
        auto& cachedFixedSize = iFixedSize.second;
        if (iFixedSize.first != global_layout_id())
        {
#ifdef NEOGFX_DEBUG
            if (&subject() == debug::layoutItem)
                service<debug::logger>() << "layout_item_proxy::fixed_size(" << aAvailableSpace << ") (cache invalid)" << endl;
#endif // NEOGFX_DEBUG
            cachedFixedSize = subject().fixed_size(aAvailableSpace);
            iFixedSize.first = global_layout_id();
        }
#ifdef NEOGFX_DEBUG
        if (&subject() == debug::layoutItem)
            service<debug::logger>() << "layout_item_proxy::fixed_size(" << aAvailableSpace << ") -> " << cachedFixedSize << endl;
#endif // NEOGFX_DEBUG
        return cachedFixedSize;
    }

    void layout_item_proxy::set_fixed_size(optional_size const& aFixedSize, bool aUpdateLayout)
    {
        subject().set_fixed_size(aFixedSize, aUpdateLayout);
        if (aFixedSize != std::nullopt)
            iFixedSize.second = *aFixedSize;
    }

    bool layout_item_proxy::has_padding() const
    {
        return subject().has_padding();
    }

    padding layout_item_proxy::padding() const
    {
        return subject().padding();
    }

    void layout_item_proxy::set_padding(optional_padding const& aPadding, bool aUpdateLayout)
    {
        subject().set_padding(aPadding, aUpdateLayout);
    }

    void layout_item_proxy::layout_item_enabled(i_layout_item& aItem)
    {
        subject().layout_item_enabled(aItem);
    }

    void layout_item_proxy::layout_item_disabled(i_layout_item& aItem)
    {
        subject().layout_item_disabled(aItem);
    }

    bool layout_item_proxy::visible() const
    {
        auto& cachedVisible = iVisible.second;
        if (iVisible.first != global_layout_id())
        {
            cachedVisible = subject().visible() || parent_layout().ignore_visibility();
            iVisible.first = global_layout_id();
        }
        return cachedVisible;
    }

    bool layout_item_proxy::operator==(const layout_item_proxy& aOther) const
    {
        return iSubject == aOther.iSubject;
    }

    bool layout_item_proxy::subject_is_proxy() const
    {
        return iSubjectIsProxy;
    }
}
