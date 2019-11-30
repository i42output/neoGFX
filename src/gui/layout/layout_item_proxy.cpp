// layout_item_proxy.cpp
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
#include <neogfx/gui/layout/layout_item_proxy.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/layout/i_spacer.hpp>
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
    layout_item_proxy::layout_item_proxy(i_layout_item& aItem) :
        layout_item_proxy{ std::shared_ptr<i_layout_item>{ std::shared_ptr<i_layout_item>{}, &aItem } } 
    {
    }

    layout_item_proxy::layout_item_proxy(std::shared_ptr<i_layout_item> aItem) :
        iSubject{ aItem }, iLayoutId{ -1, -1 }
    {
    }

    layout_item_proxy::layout_item_proxy(const layout_item_proxy& aOther) :
        iSubject{ aOther.iSubject }, iLayoutId{ -1, -1 }
    {
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

    void layout_item_proxy::anchor_to(i_anchorable_object& aRhs, const i_string& aLhsAnchor, anchor_constraint_function aLhsFunction, const i_string& aRhsAnchor, anchor_constraint_function aRhsFunction)
    {
        return subject().anchor_to(aRhs, aLhsAnchor, aLhsFunction, aRhsAnchor, aRhsFunction);
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
        if (!subject().is_proxy())
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
        if (!subject().is_proxy())
            subject().set_layout_owner(aOwner);
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

    void layout_item_proxy::layout_as(const point& aPosition, const size& aSize)
    {
        point adjustedPosition = aPosition;
        size adjustedSize = aSize.min(maximum_size());
        if (adjustedSize != aSize)
        {
            adjustedPosition += point{
            (parent_layout().alignment() & alignment::Centre) == alignment::Centre ?
                (aSize - adjustedSize).cx / 2.0 :
                (parent_layout().alignment() & alignment::Right) == alignment::Right ?
                    (aSize - adjustedSize).cx :
                    0.0,
            (parent_layout().alignment() & alignment::VCentre) == alignment::VCentre ?
                (aSize - adjustedSize).cy / 2.0 :
                (parent_layout().alignment() & alignment::Bottom) == alignment::Bottom ?
                    (aSize - adjustedSize).cy :
                    0.0 }.floor();
        }

        subject().layout_as(adjustedPosition, adjustedSize);
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
        return subject().size_policy();
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
        return subject().weight();
    }

    void layout_item_proxy::set_weight(const optional_size& aWeight, bool aUpdateLayout)
    {
        subject().set_weight(aWeight, aUpdateLayout);
    }

    bool layout_item_proxy::has_minimum_size() const
    {
        return subject().has_minimum_size();
    }

    size layout_item_proxy::minimum_size(const optional_size& aAvailableSpace) const
    {
        if (!visible())
            return size{};
        if (iLayoutId.first == parent_layout().layout_id() && iLayoutId.first != -1)
            return iMinimumSize;
        else
        {
            if (iMinimumSizeAnchor == std::nullopt)
            {   
                auto anchorIter = anchors().find(string{ "MinimumSize" });
                if (anchorIter != anchors().end())
                    iMinimumSizeAnchor = static_cast<const neolib::optional_t<decltype(iMinimumSizeAnchor)>>(anchorIter->second());
                else
                    iMinimumSizeAnchor = nullptr;
            }
            if (*iMinimumSizeAnchor == nullptr)
                iMinimumSize = subject().minimum_size(aAvailableSpace);
            else
                iMinimumSize = (**iMinimumSizeAnchor).evaluate_constraints(aAvailableSpace);
            if (size_policy().maintain_aspect_ratio())
            {
                const auto& aspectRatio = size_policy().aspect_ratio();
                if (aspectRatio.cx < aspectRatio.cy)
                {
                    if (iMinimumSize.cx < iMinimumSize.cy)
                        iMinimumSize = size{ iMinimumSize.cx, iMinimumSize.cx * (aspectRatio.cy / aspectRatio.cx) };
                    else
                        iMinimumSize = size{ iMinimumSize.cy * (aspectRatio.cx / aspectRatio.cy), iMinimumSize.cy };
                }
                else
                {
                    if (iMinimumSize.cx < iMinimumSize.cy)
                        iMinimumSize = size{ iMinimumSize.cy * (aspectRatio.cx / aspectRatio.cy), iMinimumSize.cy };
                    else
                        iMinimumSize = size{ iMinimumSize.cx, iMinimumSize.cx * (aspectRatio.cy / aspectRatio.cx) };
                }
            }
            iLayoutId.first = parent_layout().layout_id();
            return iMinimumSize;
        }
    }

    void layout_item_proxy::set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout)
    {
        subject().set_minimum_size(aMinimumSize, aUpdateLayout);
        if (aMinimumSize != std::nullopt)
            iMinimumSize = *aMinimumSize;
    }

    bool layout_item_proxy::has_maximum_size() const
    {
        return subject().has_maximum_size();
    }

    size layout_item_proxy::maximum_size(const optional_size& aAvailableSpace) const
    {
        if (!visible())
            return size::max_size();
        if (iLayoutId.second == parent_layout().layout_id() && iLayoutId.second != -1)
            return iMaximumSize;
        else
        {
            iMaximumSize = subject().maximum_size(aAvailableSpace);
            iLayoutId.second = parent_layout().layout_id();
            return iMaximumSize;
        }
    }

    void layout_item_proxy::set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout)
    {
        subject().set_maximum_size(aMaximumSize, aUpdateLayout);
        if (aMaximumSize != std::nullopt)
            iMaximumSize = *aMaximumSize;
    }

    bool layout_item_proxy::has_margins() const
    {
        return subject().has_margins();
    }

    margins layout_item_proxy::margins() const
    {
        return subject().margins();
    }

    void layout_item_proxy::set_margins(const optional_margins& aMargins, bool aUpdateLayout)
    {
        subject().set_margins(aMargins, aUpdateLayout);
    }

    bool layout_item_proxy::visible() const
    {
        return subject().visible();
    }

    bool layout_item_proxy::operator==(const layout_item_proxy& aOther) const
    {
        return iSubject == aOther.iSubject;
    }
}
