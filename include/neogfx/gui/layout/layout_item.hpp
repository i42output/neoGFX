// layout_item.hpp
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
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include <neogfx/gui/layout/i_layout_item_cache.hpp>

namespace neogfx
{
    template <typename Base>
    class layout_item : public anchorable<reference_counted<Base>>
    {
        typedef layout_item<Base> self_type;
        typedef anchorable<reference_counted<Base>> base_type;
        // events
    public:
        define_event(AnchorUpdated, anchor_updated, i_anchor&)
        // types
    public:
        typedef i_layout_item abstract_type;
    private:
        typedef self_type property_context_type;
        // construction
    public:
        layout_item()
        {
        }
        ~layout_item()
        {
            base_type::set_destroying();
        }
        // implementation
    public:
        const i_string& id() const final
        {
            return iId;
        }
        void set_id(const i_string& aId) final
        {
            iId = aId;
        }
    public:
        bool is_cache() const final
        {
            return false;
        }
    public:
        bool is_layout() const final
        {
            return std::is_base_of_v<i_layout, self_type>;
        }
        const i_layout& as_layout() const final
        {
            if constexpr (std::is_base_of_v<i_layout, self_type>)
                return static_cast<const i_layout&>(*this);
            else
                throw not_a_layout();
        }
        i_layout& as_layout() final
        {
            if constexpr (std::is_base_of_v<i_layout, self_type>)
                return static_cast<i_layout&>(*this);
            else
                throw not_a_layout();
        }
        bool is_spacer() const final
        {
            return std::is_base_of_v<i_spacer, self_type>;
        }
        const i_spacer& as_spacer() const final
        {
            if constexpr (std::is_base_of_v<i_spacer, self_type>)
                return static_cast<const i_spacer&>(*this);
            else
                throw not_a_spacer();
        }
        i_spacer& as_spacer() final
        {
            if constexpr (std::is_base_of_v<i_spacer, self_type>)
                return static_cast<i_spacer&>(*this);
            else
                throw not_a_spacer();
        }
        bool is_widget() const override
        {
            return std::is_base_of_v<i_widget, self_type>;
        }
        const i_widget& as_widget() const override
        {
            if constexpr (std::is_base_of_v<i_widget, self_type>)
                return static_cast<const i_widget&>(*this);
            else
                throw not_a_widget();
        }
        i_widget& as_widget() override
        {
            if constexpr (std::is_base_of_v<i_widget, self_type>)
                return static_cast<i_widget&>(*this);
            else
                throw not_a_widget();
        }
    public:
        bool has_parent_layout_item() const final
        {
            auto& self = as_layout_item();
            if (self.has_parent_layout() && self.same_layout_owner_as(self.parent_layout()))
                return true;
            else if (self.is_layout() && self.has_layout_owner())
                return true;
            else if (self.is_widget() && self.as_widget().has_parent())
                return true;
            return false;
        }
        const i_layout_item& parent_layout_item() const final
        {
            auto& self = as_layout_item();
            if (self.has_parent_layout() && self.same_layout_owner_as(self.parent_layout()))
                return self.parent_layout();
            else if (self.is_layout() && self.has_layout_owner())
                return self.layout_owner();
            else if (self.is_widget() && self.as_widget().has_parent())
                return self.as_widget().parent();
            throw no_parent_layout_item();
        }
        i_layout_item& parent_layout_item() final
        {
            return const_cast<i_layout_item&>(to_const(*this).parent_layout_item());
        }
    public:
        bool has_layout_manager() const final
        {
            auto& self = as_layout_item();
            if (!self.has_layout_owner())
                return false;
            const i_widget* w = &self.layout_owner();
            if (w->is_managing_layout())
                return true;
            while (w->has_parent())
            {
                w = &w->parent();
                if (w->is_managing_layout())
                    return true;
            };
            return false;
        }
        const i_widget& layout_manager() const final
        {
            auto& self = as_layout_item();
            const i_widget* w = &self.layout_owner();
            if (w->is_managing_layout())
                return *w;
            while (w->has_parent())
            {
                w = &w->parent();
                if (w->is_managing_layout())
                    return *w;
            };
            throw no_layout_manager();
        }
        i_widget& layout_manager() final
        {
            return const_cast<i_widget&>(to_const(*this).layout_manager());
        }
    public:
        void update_layout(bool aDeferLayout = true, bool aAncestors = false) final
        {
            auto& self = as_layout_item();
            if (self.has_parent_layout_item())
            {
                if (!self.is_widget() || 
                    (self.as_widget().has_parent_layout() && !self.as_widget().is_managing_layout()) || aAncestors)
                    self.parent_layout_item().update_layout(aDeferLayout, aAncestors);
            }

#ifdef NEOGFX_DEBUG
            if (debug::layoutItem == this)
                service<debug::logger>() << "layout_item::update_layout(" << aDeferLayout << ", " << aAncestors << ")" << endl;
#endif // NEOGFX_DEBUG

            if (self.is_widget() && (!aDeferLayout || self.as_widget().can_defer_layout()))
                self.as_widget().layout_items(aDeferLayout);
            else if (self.is_layout())
                self.as_layout().invalidate(aDeferLayout);
        }
    public:
        point origin() const final
        {
            if (iOrigin == std::nullopt)
            {
                auto& self = as_layout_item();
                if (!self.is_widget())
                {
                    if (has_parent_layout_item())
                        iOrigin = parent_layout_item().origin();
                    else
                        iOrigin = point{};
                }
                else
                {
                    if (!self.as_widget().is_root() || self.as_widget().root().is_nested())
                    {
                        if (self.as_widget().has_parent())
                            iOrigin = self.as_widget().position() + self.as_widget().parent().origin();
                        else
                            iOrigin = self.as_widget().position();
                    }
                    else
                        iOrigin = point{};
                }
            }
            return *iOrigin;
        }
        void reset_origin() const final
        {
            iOrigin = std::nullopt;
        }
        point position() const final
        {
            auto& self = as_layout_item();
            return (self.has_parent_layout_item() ? self.parent_layout_item().transformation(true) : mat33::identity()) *
                units_converter{ *this }.from_device_units(!Anchor_Position.active() ? 
                    static_cast<point>(Position) : static_cast<point>(Position) + Anchor_Position.evaluate_constraints() - unconstrained_origin());
        }
        size extents() const final
        {
            auto& self = as_layout_item();
            return (self.has_parent_layout_item() ? self.parent_layout_item().transformation(true) : mat33::identity()) *
                units_converter{ *this }.from_device_units(!Anchor_Size.active() ? 
                    static_cast<size>(Size) : Anchor_Size.evaluate_constraints());
        }
    protected:
        void set_position(const point& aPosition) override
        {
            reset_origin();
            if (Position != units_converter{ *this }.to_device_units(aPosition))
                Position.assign(units_converter{ *this }.to_device_units(aPosition), false);
        }
        void set_extents(const size& aExtents) override
        {
            if (Size != units_converter{ *this }.to_device_units(aExtents))
                Size.assign(units_converter{ *this }.to_device_units(aExtents), false);
        }
    public:
        bool has_size_policy() const noexcept override
        {
            return SizePolicy != std::nullopt;
        }
        neogfx::size_policy size_policy() const override
        {
#ifdef NEOGFX_DEBUG
            if (debug::layoutItem == this)
                service<debug::logger>() << typeid(*this).name() << "::size_policy()" << endl;
#endif // NEOGFX_DEBUG
            if (SizePolicy != std::nullopt)
                return *SizePolicy;
            else if (has_fixed_size())
                return size_constraint::Fixed;
            else
                return size_constraint::Minimum;
        }
        using i_geometry::set_size_policy;
        void set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout = true) override
        {
            if (SizePolicy != aSizePolicy)
            {
#ifdef NEOGFX_DEBUG
                if (debug::layoutItem == this)
                    service<debug::logger>() << typeid(*this).name() << "::set_size_policy(" << aSizePolicy << ", " << aUpdateLayout << ")" << endl;
#endif // NEOGFX_DEBUG
                SizePolicy = aSizePolicy;
                if (aUpdateLayout)
                    update_layout();
            }
        }
        bool has_weight() const noexcept override
        {
            return Weight != std::nullopt;
        }
        size weight() const override
        {
            if (has_weight())
                return *Weight;
            return size{ 1.0 };
        }
        void set_weight(optional_size const& aWeight, bool aUpdateLayout = true) override
        {
            if (Weight != aWeight)
            {
#ifdef NEOGFX_DEBUG
                if (debug::layoutItem == this)
                    service<debug::logger>() << typeid(*this).name() << "::set_weight(" << aWeight << ", " << aUpdateLayout << ")" << endl;
#endif // NEOGFX_DEBUG
                Weight.assign(aWeight, aUpdateLayout);
                if (aUpdateLayout)
                    update_layout();
            }
        }
        bool has_ideal_size() const noexcept override
        {
            return IdealSize != std::nullopt;
        }
        bool is_ideal_size_constrained() const noexcept override
        {
            return Anchor_IdealSize.active();
        }
        size ideal_size(optional_size const& aAvailableSpace = {}) const override
        {
            size result;
            if (has_ideal_size())
                result = units_converter{ *this }.from_device_units(*IdealSize);
            else if (Anchor_IdealSize.active())
                result = units_converter{ *this }.from_device_units(Anchor_IdealSize.evaluate_constraints(aAvailableSpace));
            else
            {
                scoped_query_ideal_size sqis;
                result = minimum_size(aAvailableSpace);
            }
#ifdef NEOGFX_DEBUG
            if (debug::layoutItem == this)
                service<debug::logger>() << typeid(*this).name() << "::ideal_size(" << aAvailableSpace << ") --> " << result << endl;
#endif // NEOGFX_DEBUG
            return result;
        }
        void set_ideal_size(optional_size const& aIdealSize, bool aUpdateLayout = true) override
        {
            optional_size newIdealSize = (aIdealSize != std::nullopt ? units_converter{ *this }.to_device_units(*aIdealSize) : optional_size{});
            if (IdealSize != newIdealSize)
            {
#ifdef NEOGFX_DEBUG
                if (debug::layoutItem == this)
                    service<debug::logger>() << typeid(*this).name() << "::set_ideal_size(" << aIdealSize << ", " << aUpdateLayout << ")" << endl;
#endif // NEOGFX_DEBUG
                IdealSize.assign(newIdealSize, aUpdateLayout);
                if (aUpdateLayout)
                    update_layout();
            }
        }
        bool has_minimum_size() const noexcept override
        {
            return MinimumSize != std::nullopt;
        }
        bool is_minimum_size_constrained() const noexcept override
        {
            return Anchor_MinimumSize.active();
        }
        size minimum_size(optional_size const& aAvailableSpace = {}) const override
        {
            size result;
            if (has_minimum_size())
                result = units_converter{ *this }.from_device_units(*MinimumSize);
            else if (Anchor_MinimumSize.active())
                result = units_converter{ *this }.from_device_units(Anchor_MinimumSize.evaluate_constraints(aAvailableSpace));
            else
                result = {};
#ifdef NEOGFX_DEBUG
            if (debug::layoutItem == this)
                service<debug::logger>() << typeid(*this).name() << "::minimum_size(" << aAvailableSpace << ") --> " << result << endl;
#endif // NEOGFX_DEBUG
            return result;
        }
        void set_minimum_size(optional_size const& aMinimumSize, bool aUpdateLayout = true) override
        {
            optional_size newMinimumSize = (aMinimumSize != std::nullopt ? units_converter{ *this }.to_device_units(*aMinimumSize) : optional_size{});
            if (MinimumSize != newMinimumSize)
            {
#ifdef NEOGFX_DEBUG
                if (debug::layoutItem == this)
                    service<debug::logger>() << typeid(*this).name() << "::set_minimum_size(" << aMinimumSize << ", " << aUpdateLayout << ")" << endl;
#endif // NEOGFX_DEBUG
                MinimumSize.assign(newMinimumSize, aUpdateLayout);
                if (aUpdateLayout)
                    update_layout();
            }
        }
        bool has_maximum_size() const noexcept override
        {
            return MaximumSize != std::nullopt;
        }
        bool is_maximum_size_constrained() const noexcept override
        {
            return Anchor_MaximumSize.active();
        }
        size maximum_size(optional_size const& aAvailableSpace = {}) const override
        {
            size result;
            if (has_maximum_size())
                result = units_converter{ *this }.from_device_units(*MaximumSize);
            else if (Anchor_MaximumSize.active())
                result = units_converter{ *this }.from_device_units(Anchor_MaximumSize.evaluate_constraints(aAvailableSpace));
            else
                result = size::max_size();
            return result;
        }
        void set_maximum_size(optional_size const& aMaximumSize, bool aUpdateLayout = true) override
        {
            optional_size newMaximumSize = (aMaximumSize != std::nullopt ? units_converter{ *this }.to_device_units(*aMaximumSize) : optional_size{});
            if (MaximumSize != newMaximumSize)
            {
#ifdef NEOGFX_DEBUG
                if (debug::layoutItem == this)
                    service<debug::logger>() << typeid(*this).name() << "::set_maximum_size(" << aMaximumSize << ", " << aUpdateLayout << ")" << endl;
#endif // NEOGFX_DEBUG
                MaximumSize.assign(newMaximumSize, aUpdateLayout);
                if (aUpdateLayout)
                    update_layout();
            }
        }
        bool has_fixed_size() const noexcept override
        {
            return FixedSize != std::nullopt;
        }
        size fixed_size(optional_size const& aAvailableSpace = {}) const override
        {
            if (has_fixed_size())
                return units_converter{ *this }.from_device_units(*FixedSize);
            return minimum_size(aAvailableSpace);
        }
        void set_fixed_size(optional_size const& aFixedSize, bool aUpdateLayout = true)
        {
            optional_size newFixedSize = (aFixedSize != std::nullopt ? units_converter{ *this }.to_device_units(*aFixedSize) : optional_size{});
            if (FixedSize != newFixedSize)
            {
#ifdef NEOGFX_DEBUG
                if (debug::layoutItem == this)
                    service<debug::logger>() << typeid(*this).name() << "::set_fixed_size(" << aFixedSize << ", " << aUpdateLayout << ")" << endl;
#endif // NEOGFX_DEBUG
                FixedSize.assign(newFixedSize, aUpdateLayout);
                if (aUpdateLayout)
                    update_layout();
            }
        }
        bool has_transformation() const noexcept override
        {
            return Transformation != std::nullopt;
        }
        mat33 const& transformation(bool aCombineAncestorTransformations = false) const override
        {
            if (!aCombineAncestorTransformations)
            {
                if (has_transformation())
                    return *Transformation;
                return mat33::identity();
            }
            else
            {
                if (iCombinedTransformation == std::nullopt)
                {
                    iCombinedTransformation = has_parent_layout_item() ? parent_layout_item().transformation(true) * transformation() : transformation();
                }
                return *iCombinedTransformation;
            }
        }
        void set_transformation(optional_mat33 const& aTransformation, bool aUpdateLayout = true) override
        {
            optional_mat33 newTransformation = (aTransformation != std::nullopt ? *aTransformation : optional_mat33{});
            if (Transformation != newTransformation)
            {
#ifdef NEOGFX_DEBUG
                if (debug::layoutItem == this)
                    service<debug::logger>() << typeid(*this).name() << "::set_transformation(" << aTransformation << ", " << aUpdateLayout << ")" << endl;
#endif // NEOGFX_DEBUG
                Transformation.assign(newTransformation, aUpdateLayout);
                invalidate_combined_transformation();
                if (aUpdateLayout)
                    update_layout();
            }
        }
    public:
        bool has_margin() const noexcept override
        {
            return Margin != std::nullopt;
        }
        neogfx::margin margin() const override
        {
            return Margin != std::nullopt ? *Margin : neogfx::margin{};
        }
        void set_margin(optional_margin const& aMargin, bool aUpdateLayout = true) override
        {
            auto newMargin = (aMargin != std::nullopt ? units_converter{ *this }.to_device_units(*aMargin) : optional_margin{});
            if (Margin != newMargin)
            {
                Margin = newMargin;
                if (aUpdateLayout)
                    update_layout();
            }
        }
        bool has_border() const noexcept override
        {
            return Border != std::nullopt;
        }
        neogfx::border border() const override
        {
            return Border != std::nullopt ? *Border : neogfx::border{};
        }
        void set_border(optional_border const& aBorder, bool aUpdateLayout = true) override
        {
            auto newBorder = (aBorder != std::nullopt ? units_converter{ *this }.to_device_units(*aBorder) : optional_border{});
            if (Border != newBorder)
            {
                Border = newBorder;
                if (aUpdateLayout)
                    update_layout();
            }
        }
        bool has_padding() const noexcept override
        {
            return Padding != std::nullopt;
        }
        neogfx::padding padding() const override
        {
            return Padding != std::nullopt ? *Padding : neogfx::padding{};
        }
        void set_padding(optional_padding const& aPadding, bool aUpdateLayout = true) override
        {
            auto newPadding = (aPadding != std::nullopt ? units_converter{ *this }.to_device_units(*aPadding) : optional_padding{});
            if (Padding != newPadding)
            {
                Padding = newPadding;
                if (aUpdateLayout)
                    update_layout();
            }
        }
    protected:
        point unconstrained_origin() const override
        {
            auto& self = as_layout_item();
            if (!self.is_widget())
            {
                if (has_parent_layout_item())
                    return parent_layout_item().origin();
                else
                    return {};
            }
            else
            {
                if (!self.as_widget().is_root() || self.as_widget().root().is_nested())
                {
                    if (self.as_widget().has_parent())
                        return self.as_widget().unconstrained_position() + self.as_widget().parent().origin();
                    else
                        return self.as_widget().unconstrained_position();
                }
                else
                    return {};
            }
        }
        point unconstrained_position() const override
        {
            return Position;
        }
    public:
        void invalidate_combined_transformation() override
        {
            iCombinedTransformation = std::nullopt;

            auto& self = as_layout_item();
            if (self.is_layout())
            {
                auto& layout = self.as_layout();
                for (layout_item_index itemIndex = 0; itemIndex < layout.count(); ++itemIndex)
                {
                    auto& item = layout.item_at(itemIndex);
                    item.invalidate_combined_transformation();
                }
            }
            else if (self.is_widget())
            {
                auto& widget = self.as_widget();
                for (auto& child : widget.children())
                {
                    child->invalidate_combined_transformation();
                }
                if (widget.has_layout())
                    widget.layout().invalidate_combined_transformation();
            }
        }
        void fix_weightings(bool aRecalculate = true) override
        {
#ifdef NEOGFX_DEBUG
            if (debug::layoutItem == this)
                service<debug::logger>() << typeid(*this).name() << "::fix_weightings(): " << endl;
#endif // NEOGFX_DEBUG
            auto& self = as_layout_item();
            auto& layout = (self.is_layout() ? self.as_layout() : self.as_widget().layout());
            for (layout_item_index itemIndex = 0; itemIndex < layout.count(); ++itemIndex)
            {
                auto& item = layout.item_at(itemIndex);
                if (!item.visible())
                    continue;
                if (aRecalculate || !item.has_weight())
                    item.set_weight(calculate_relative_weight(layout, item), false);
            }
#ifdef NEOGFX_DEBUG
            if (debug::layoutItem == this)
            {
                service<debug::logger>() << typeid(*this).name() << "::fix_weightings: weightings: ";
                bool first = true;
                for (layout_item_index itemIndex = 0; itemIndex < layout.count(); ++itemIndex)
                {
                    auto& item = layout.item_at(itemIndex);
                    if (!item.visible())
                        continue;
                    if (!first)
                        service<debug::logger>() << ", ";
                    first = false;
                    service<debug::logger>() << item.weight();
                }
                service<debug::logger>() << endl;
                service<debug::logger>() << typeid(*this).name() << "::fix_weightings() done" << endl;
            }
#endif // NEOGFX_DEBUG
        }
    protected:
        void layout_item_enabled(i_layout_item& aItem) override
        {
        }
        void layout_item_disabled(i_layout_item& aItem) override
        {
        }
    public:
        i_anchor& anchor_to(i_anchorable& aRhs, i_string const& aLhsAnchor, anchor_constraint_function aLhsFunction, i_string const& aRhsAnchor, anchor_constraint_function aRhsFunction) override
        {
            auto& lhsAnchor = base_type::anchor_to(aRhs, aLhsAnchor, aLhsFunction, aRhsAnchor, aRhsFunction);
            AnchorUpdated.trigger(lhsAnchor);
            update_layout(true, true);
            return lhsAnchor;
        }
    private:
        i_layout_item const& as_layout_item() const
        {
            return *this;
        }
        i_layout_item& as_layout_item()
        {
            return *this;
        }
        // properties / anchors
    public:
        // todo: declare_property
        define_property(property_category::soft_geometry, point, Position, position)
        define_property(property_category::soft_geometry, size, Size, extents)
        define_property(property_category::hard_geometry, optional_margin, Margin, margin)
        define_property(property_category::hard_geometry, optional_border, Border, border)
        define_property(property_category::hard_geometry, optional_padding, Padding, padding)
        define_property(property_category::hard_geometry, optional_size_policy, SizePolicy, size_policy)
        define_property(property_category::hard_geometry, optional_size, Weight, weight)
        define_property(property_category::hard_geometry, optional_size, IdealSize, ideal_size)
        define_property(property_category::hard_geometry, optional_size, MinimumSize, minimum_size)
        define_property(property_category::hard_geometry, optional_size, MaximumSize, maximum_size)
        define_property(property_category::hard_geometry, optional_size, FixedSize, fixed_size)
        define_property(property_category::hard_geometry, optional_mat33, Transformation, transformation)
        // todo: declare_anchor
        define_anchor_ex(Position, unconstrained_origin)
        define_anchor(Size)
        define_anchor(Padding)
        define_anchor(IdealSize)
        define_anchor(MinimumSize)
        define_anchor(MaximumSize)
    private:
        string iId;
        mutable optional_point iOrigin;
        mutable optional_mat33 iCombinedTransformation;
    };
}
