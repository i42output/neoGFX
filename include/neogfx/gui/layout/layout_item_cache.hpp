// layout_item_cache.hpp
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
#include <neogfx/core/object.hpp>
#include <neogfx/gui/layout/i_anchor.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/layout/i_layout_item.hpp>
#include <neogfx/gui/layout/i_layout_item_cache.hpp>

namespace neogfx
{
    class layout_item_cache : public object<reference_counted<i_layout_item_cache>>
    {
    public:
        layout_item_cache(i_layout_item& aItem);
        layout_item_cache(i_ref_ptr<i_layout_item> const& aItem);
        layout_item_cache(const layout_item_cache& aOther);
        ~layout_item_cache();
    public:
        i_anchor& anchor_to(i_anchorable& aRhs, const i_string& aLhsAnchor, anchor_constraint_function aLhsFunction, const i_string& aRhsAnchor, anchor_constraint_function aRhsFunction) final;
        const anchor_map_type& anchors() const final;
        anchor_map_type& anchors() final;
    public:
        const i_string& id() const final;
        void set_id(const i_string& aId) final;
    public:
        bool is_cache() const final;
    public:
        bool is_layout() const final;
        const i_layout& as_layout() const final;
        i_layout& as_layout() final;
        bool is_spacer() const final;
        const i_spacer& as_spacer() const final;
        i_spacer& as_spacer() final;
        bool is_widget() const final;
        const i_widget& as_widget() const final;
        i_widget& as_widget() final;
    public:
        bool has_parent_layout_item() const final;
        const i_layout_item& parent_layout_item() const final;
        i_layout_item& parent_layout_item() final;;
    public:
        bool has_parent_layout() const final;
        const i_layout& parent_layout() const final;
        i_layout& parent_layout() final;
        void set_parent_layout(i_layout* aParentLayout);
        bool has_parent_widget() const final;
        const i_widget& parent_widget() const final;
        i_widget& parent_widget() final;
        void set_parent_widget(i_widget* aParentWidget) final;
        bool has_layout_manager() const final;
        const i_widget& layout_manager() const final;
        i_widget& layout_manager() final;
    public:
        bool device_metrics_available() const final;
        const i_device_metrics& device_metrics() const final;
    public:
        point origin() const final;
        void reset_origin() const final;
        point position() const final;
        void set_position(const point& aPosition) final;
        size extents() const final;
        void set_extents(const size& aExtents) final;
        bool has_size_policy() const noexcept final;
        neogfx::size_policy size_policy() const final;
        void set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout = true) final;
        bool has_weight() const noexcept final;
        size weight() const final;
        void set_weight(optional_size const& aWeight, bool aUpdateLayout = true) final;
        bool has_ideal_size() const noexcept final;
        bool is_ideal_size_constrained() const noexcept final;
        size ideal_size(optional_size const& aAvailableSpace = optional_size{}) const final;
        void set_ideal_size(optional_size const& aMinimumSize, bool aUpdateLayout = true) final;
        bool has_minimum_size() const noexcept final;
        bool is_minimum_size_constrained() const noexcept final;
        size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const final;
        void set_minimum_size(optional_size const& aMinimumSize, bool aUpdateLayout = true) final;
        bool has_maximum_size() const noexcept final;
        bool is_maximum_size_constrained() const noexcept final;
        size maximum_size(optional_size const& aAvailableSpace = optional_size{}) const final;
        void set_maximum_size(optional_size const& aMaximumSize, bool aUpdateLayout = true) final;
        bool has_fixed_size() const noexcept final;
        size fixed_size(optional_size const& aAvailableSpace = {}) const final;
        void set_fixed_size(optional_size const& aFixedSize, bool aUpdateLayout = true) final;
        bool has_transformation() const noexcept final;
        mat33 const& transformation(bool aCombineAncestorTransformations = false) const final;
        void set_transformation(optional_mat33 const& aTransformation, bool aUpdateLayout = true) final;
    public:
        bool has_margin() const noexcept final;
        neogfx::margin margin() const final;
        void set_margin(optional_margin const& aMargin, bool aUpdateLayout = true) final;
        bool has_border() const noexcept final;
        neogfx::border border() const final;
        void set_border(optional_border const& aBorder, bool aUpdateLayout = true) final;
        bool has_padding() const noexcept final;
        neogfx::padding padding() const final;
        void set_padding(optional_padding const& aPadding, bool aUpdateLayout = true) final;
    protected:
        point unconstrained_origin() const final;
        point unconstrained_position() const final;
    protected:
        void layout_item_enabled(i_layout_item& aItem) final;
        void layout_item_disabled(i_layout_item& aItem) final;
    public:
        bool visible() const final;
    public:
        void update_layout(bool aDeferLayout = true, bool aAncestors = true) final;
        void layout_as(const point& aPosition, const size& aSize) final;
    public:
        void invalidate_combined_transformation() final;
        void fix_weightings(bool aRecalculate = true) final;
    public:
        i_layout_item& subject() const final;
        bool subject_destroyed() const final;
    public:
        layout_item_disposition& cached_disposition() const final;
    public:
        bool operator==(const layout_item_cache& aOther) const;
    private:
        ref_ptr<i_layout_item> iSubject;
        destroyed_flag iSubjectDestroyed;
        mutable layout_item_disposition iCachedDisposition = layout_item_disposition::Unknown;
        mutable std::pair<uint32_t, bool> iVisible;
        mutable std::pair<uint32_t, neogfx::size_policy> iSizePolicy;
        mutable std::pair<uint32_t, size> iWeight;
        mutable std::pair<uint32_t, std::pair<optional_size, size>> iIdealSize;
        mutable std::pair<uint32_t, std::pair<optional_size, size>> iMinimumSize;
        mutable std::pair<uint32_t, std::pair<optional_size, size>> iMaximumSize;
        mutable std::pair<uint32_t, std::pair<optional_size, size>> iFixedSize;
        mutable std::pair<uint32_t, mat33> iTransformation;
        mutable std::pair<uint32_t, mat33> iCombinedTransformation;
    };
}
