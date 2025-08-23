// item_presentation_model.hpp
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

#include <vector>
#include <deque>
#include <boost/algorithm/string.hpp>

#include <neolib/core/vecarray.hpp>
#include <neolib/core/segmented_array.hpp>
#include <neolib/core/scoped.hpp>

#include <neogfx/core/object.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/i_drag_drop.hpp>
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/widget/spin_box.hpp>
#include <neogfx/gui/widget/item_model.hpp>
#include <neogfx/gui/widget/i_item_presentation_model.hpp>
#include <neogfx/gui/widget/i_skin_manager.hpp>

namespace neogfx
{
    template <typename ItemModel>
    class basic_item_presentation_model : public object<reference_counted<i_item_presentation_model>>
    {
        typedef object<reference_counted<i_item_presentation_model>> base_type;
    public:
        define_declared_event(VisualAppearanceChanged, visual_appearance_changed)
        define_declared_event(ColumnInfoChanged, column_info_changed, item_presentation_model_index::column_type)
        define_declared_event(ItemModelChanged, item_model_changed, const i_item_model&)
        define_declared_event(ItemAdded, item_added, item_presentation_model_index const&)
        define_declared_event(ItemChanged, item_changed, item_presentation_model_index const&)
        define_declared_event(ItemRemoving, item_removing, item_presentation_model_index const&)
        define_declared_event(ItemRemoved, item_removed, item_presentation_model_index const&)
        define_declared_event(ItemExpanding, item_expanding, item_presentation_model_index const&)
        define_declared_event(ItemCollapsing, item_collapsing, item_presentation_model_index const&)
        define_declared_event(ItemExpanded, item_expanded, item_presentation_model_index const&)
        define_declared_event(ItemCollapsed, item_collapsed, item_presentation_model_index const&)
        define_declared_event(ItemToggled, item_toggled, item_presentation_model_index const&)
        define_declared_event(ItemChecked, item_checked, item_presentation_model_index const&)
        define_declared_event(ItemUnchecked, item_unchecked, item_presentation_model_index const&)
        define_declared_event(ItemIndeterminate, item_indeterminate, item_presentation_model_index const&)
        define_declared_event(ItemsUpdating, items_updating)
        define_declared_event(ItemsUpdated, items_updated)
        define_declared_event(ItemsSorting, items_sorting)
        define_declared_event(ItemsSorted, items_sorted)
        define_declared_event(ItemsFiltering, items_filtering)
        define_declared_event(ItemsFiltered, items_filtered)
        define_declared_event(DraggingItem, dragging_item, i_drag_drop_item const&)
        define_declared_event(DraggingItemRenderInfo, dragging_item_render_info, i_drag_drop_item const&, bool&, size&)
        define_declared_event(DraggingItemRender, dragging_item_render, i_drag_drop_item const&, i_graphics_context&, point const&)
        define_declared_event(DraggingItemCancelled, dragging_item_cancelled, i_drag_drop_item const&)
        define_declared_event(ItemDropped, item_dropped, i_drag_drop_item const&, i_drag_drop_target&)
    public:
        using typename base_type::sort_direction;
        using typename base_type::optional_sort_direction;
        using typename base_type::sort_by_param;
        using typename base_type::optional_sort_by_param;
        using typename base_type::filter_search_key;
        using typename base_type::filter_search_type;
        using typename base_type::case_sensitivity;
    private:
        typedef ItemModel item_model_type;
        typedef typename item_model_type::container_traits::template rebind<item_model_index::row_type, cell_meta_type, true>::other container_traits;
        typedef typename container_traits::row_cell_array row_cell_array;
        typedef typename container_traits::container_type container_type;
        typedef typename container_traits::const_skip_iterator const_iterator;
        typedef typename container_traits::skip_iterator iterator;
        typedef typename container_traits::const_sibling_iterator const_sibling_iterator;
        typedef typename container_traits::sibling_iterator sibling_iterator;
        typedef typename container_traits::allocator_type allocator_type;
        typedef typename container_type::value_type row_type;
        typedef std::optional<i_scrollbar::value_type> optional_position;
    private:
        typedef std::vector<item_presentation_model_index::optional_row_type, typename std::allocator_traits<allocator_type>:: template rebind_alloc<item_presentation_model_index::optional_row_type>> row_map_type;
        typedef std::vector<item_presentation_model_index::optional_column_type, typename std::allocator_traits<allocator_type>:: template rebind_alloc<item_presentation_model_index::optional_column_type>> column_map_type;
        struct column_info
        {
            column_info(const item_model_index::optional_column_type modelColumn = {}) : modelColumn{ modelColumn } {}
            ~column_info() {}
            mutable item_model_index::optional_column_type modelColumn;
            item_cell_flags flags = item_cell_flags::Default;
            mutable std::map<dimension, std::uint32_t> cellWidths;
            mutable std::optional<std::string> headingText;
            mutable font headingFont;
            mutable optional_size headingExtents;
            optional_size imageSize;

            void add_cell_width(dimension aWidth) const
            {
                ++cellWidths[aWidth];
            }
            void remove_cell_width(dimension aWidth) const
            {
                auto existing = cellWidths.find(aWidth);
                if (existing != cellWidths.end())
                {
                    if (--existing->second == 0)
                        cellWidths.erase(existing);
                }
            }
        };
        typedef typename container_traits::template rebind<item_presentation_model_index::row_type, column_info>::other::row_cell_array column_info_array;
    public:
        using typename base_type::no_item_model;
        using typename base_type::bad_index;
        using typename base_type::no_mapped_row;
    public:
        basic_item_presentation_model(bool aSortable = false) : iItemModel{ nullptr }, iSortable{ aSortable }, iAlternatingRowColor{ false }
        {
            init();
        }
        basic_item_presentation_model(i_item_model& aItemModel, bool aSortable = false) : iItemModel{ nullptr }, iSortable{ aSortable }, iAlternatingRowColor{ false }
        {
            init();
            set_item_model(aItemModel);
        }
        ~basic_item_presentation_model()
        {
            set_destroying();
            iSink.clear();
            iItemModelSink.clear();
        }
    public:
        bool metrics_available() const
        {
            return attached() && attachment().has_root();
        }
        bool attached() const final
        {
            return iAttachment != nullptr;
        }
        bool attached_to(i_widget& aWidget) const final
        {
            return attached() && &attachment() == &aWidget;
        }
        i_widget& attachment() const final
        {
            if (iAttachment == nullptr)
                throw not_attached();
            return *iAttachment;
        }
        void attach(i_ref_ptr<i_widget> const& aWidget) final
        {
            if (iAttachment && aWidget != nullptr)
                throw already_attached();
            iAttachment = aWidget;
        }
        void detach() final
        {
            if (iAttachment == nullptr)
                throw not_attached();
            iAttachment = nullptr;
        }
    public:
        bool updating() const final
        {
            return iUpdating != 0u;
        }
        void begin_update() final
        {
            if (++iUpdating == 1)
                ItemsUpdating();
        }
        void end_update() final
        {
            if (--iUpdating == 0)
            {
                reset_maps();
                reset_meta();
                reset_sort();
                
                ItemsUpdated();
            }
        }
        bool has_item_model() const final
        {
            return iItemModel != nullptr;
        }
        item_model_type& item_model() const final
        {
            if (iItemModel == nullptr)
                throw no_item_model();
            return static_cast<item_model_type&>(*iItemModel);
        }
        void set_item_model(i_item_model& aItemModel) final
        {
            if (iItemModel != &aItemModel)
            {
                auto reset_model = [this]()
                {
                    {
                        scoped_item_update siu{ *this };
                        iColumns.clear();
                        for (item_model_index::column_type col = 0; col < item_model().columns(); ++col)
                            iColumns.emplace_back(col);
                        iRows.clear();
                        for (item_model_index::row_type row = 0; row < item_model().rows(); ++row)
                            item_added(item_model_index{ row });
                    }

                    ItemModelChanged(item_model());
                };
                iItemModelSink.clear();
                iItemModel = &aItemModel;
                iItemModelSink += item_model().column_info_changed([this](item_model_index::column_type aColumnIndex) { item_model_column_info_changed(aColumnIndex); });
                iItemModelSink += item_model().item_added([this](const item_model_index& aItemIndex) { item_added(aItemIndex); });
                iItemModelSink += item_model().item_changed([this](const item_model_index& aItemIndex) { item_changed(aItemIndex); });
                iItemModelSink += item_model().item_removing([this](const item_model_index& aItemIndex) { item_removing(aItemIndex); });
                iItemModelSink += item_model().item_removed([this](const item_model_index& aItemIndex) { item_removed(aItemIndex); });
                iItemModelSink += item_model().cleared([this]()
                {  
                    iRows.clear();
                    reset_maps();
                    reset_meta();
                    reset_sort();
                    ItemModelChanged(item_model());
                });
                iItemModelSink += item_model().destroying([this]() 
                { 
                    iItemModel = nullptr;
                    iColumns.clear(); 
                    iRows.clear(); 
                    reset_maps();
                    reset_meta();
                    reset_sort();
                });
                reset_model();
            }
        }
        item_model_index to_item_model_index(item_presentation_model_index const& aIndex) const final
        {
            return item_model_index{ row(aIndex).value, model_column(aIndex.column()) };
        }
        bool has_item_model_index(item_model_index const& aIndex) const final
        {
            return aIndex.row() < row_map().size() && row_map()[aIndex.row()];
        }
        item_presentation_model_index from_item_model_index(item_model_index const& aIndex, bool aIgnoreColumn = false) const final
        {
            if (has_item_model_index(aIndex))
                return item_presentation_model_index{ mapped_row(aIndex.row()), !aIgnoreColumn ? mapped_column(aIndex.column()) : 0 };
            throw bad_index();
        }
    public:
        std::uint32_t rows() const final
        {
            if constexpr (container_traits::is_flat)
                return static_cast<std::uint32_t>(iRows.size());
            else
                return static_cast<std::uint32_t>(iRows.ksize());
        }
        std::uint32_t columns() const final
        {
            return static_cast<std::uint32_t>(iColumns.size());
        }
        std::uint32_t columns(item_presentation_model_index const& aIndex) const final
        {
            return static_cast<std::uint32_t>(row(aIndex).cells.size());
        }
    public:
        void accept(i_meta_visitor& aVisitor, bool aIgnoreCollapsedState = false) final
        {
            if constexpr (container_traits::is_flat)
            {
                for (auto row = iRows.begin(); row != iRows.end(); ++row)
                    for (auto& cell : row->cells)
                        aVisitor.visit(cell);
            }
            else if (!aIgnoreCollapsedState)
            {
                for (auto row = iRows.kbegin(); row != iRows.kend(); ++row)
                    for (auto& cell : row->cells)
                        aVisitor.visit(cell);
            }
            else
            {
                for (auto row = iRows.begin(); row != iRows.end(); ++row)
                    for (auto& cell : row->cells)
                        aVisitor.visit(cell);
            }
        }
    public:
        dimension column_width(item_presentation_model_index::column_type aColumnIndex, i_units_context const& aUnitsContext, bool aExtendIntoPadding = true) const override
        {
            if (rows() == 0 || columns() < aColumnIndex + 1u)
                return 0.0;
            auto& cellWidths = column(aColumnIndex).cellWidths;
            if (!cellWidths.empty())
                return units_converter(aUnitsContext).from_device_units(cellWidths.rbegin()->first) + (aExtendIntoPadding ? cell_padding(aUnitsContext).size().cx : 0.0);
            for (item_presentation_model_index::row_type row = 0u; row < rows(); ++row)
                cell_extents(item_presentation_model_index{ row, aColumnIndex }, aUnitsContext);
            return units_converter(aUnitsContext).from_device_units(cellWidths.rbegin()->first) + (aExtendIntoPadding ? cell_padding(aUnitsContext).size().cx : 0.0);
        }
        std::string const& column_heading_text(item_presentation_model_index::column_type aColumnIndex) const override
        {
            if (column(aColumnIndex).headingText != std::nullopt)
                return *column(aColumnIndex).headingText;
            else if (has_model_column(aColumnIndex))
                return item_model().column_name(model_column(aColumnIndex));
            else
            {
                static std::string const none;
                return none;
            }
        }
        size column_heading_extents(item_presentation_model_index::column_type aColumnIndex, i_units_context const& aUnitsContext) const override
        {
            if (column(aColumnIndex).headingFont != font{})
            {
                column(aColumnIndex).headingFont = font{};
                column(aColumnIndex).headingExtents = std::nullopt;
            }
            if (column(aColumnIndex).headingExtents != std::nullopt)
                return units_converter(aUnitsContext).from_device_units(*column(aColumnIndex).headingExtents);
            if (!metrics_available())
                return size{};
            size columnHeadingExtents = graphics_context{ attachment(), graphics_context::type::Unattached }.
                multiline_text_extent(column_heading_text(aColumnIndex), column(aColumnIndex).headingFont);
            column(aColumnIndex).headingExtents = units_converter(aUnitsContext).to_device_units(columnHeadingExtents).ceil();
            return units_converter(aUnitsContext).from_device_units(*column(aColumnIndex).headingExtents);
        }
        void set_column_heading_text(item_presentation_model_index::column_type aColumnIndex, std::string const& aHeadingText) final
        {
            column(aColumnIndex).headingText = aHeadingText;
            column(aColumnIndex).headingExtents = std::nullopt;
            ColumnInfoChanged(aColumnIndex);
        }
        item_cell_flags column_flags(item_presentation_model_index::column_type aColumnIndex) const override
        {
            if (aColumnIndex < columns())
                return column(aColumnIndex).flags;
            return item_cell_flags::Default;
        }
        void set_column_flags(item_presentation_model_index::column_type aColumnIndex, item_cell_flags aFlags) final
        {
            if (column(aColumnIndex).flags != aFlags)
            {
                column(aColumnIndex).flags = aFlags;
                ColumnInfoChanged(aColumnIndex);
            }
        }
        optional_size column_image_size(item_presentation_model_index::column_type aColumnIndex) const override
        {
            return column(aColumnIndex).imageSize;
        }
        void set_column_image_size(item_presentation_model_index::column_type aColumnIndex, optional_size const& aImageSize) final
        {
            if (column(aColumnIndex).imageSize != aImageSize)
            {
                column(aColumnIndex).imageSize = aImageSize;
                reset_meta();
                ColumnInfoChanged(aColumnIndex);
            }
        }
        bool expand(item_presentation_model_index const& aIndex) final
        {
            if constexpr (container_traits::is_tree)
            {
                if (!cell_meta(aIndex.with_column(0)).expanded)
                {
                    toggle_expanded(aIndex.with_column(0));
                    return true;
                }
            }
            return false;
        }
        bool collapse(item_presentation_model_index const& aIndex) final
        {
            if constexpr (container_traits::is_tree)
            {
                if (cell_meta(aIndex.with_column(0)).expanded)
                {
                    toggle_expanded(aIndex.with_column(0));
                    return true;
                }
            }
            return false;
        }
        bool toggle_expanded(item_presentation_model_index const& aIndex) final
        {
            if constexpr (container_traits::is_tree)
            {
                if (!item_model().has_children(to_item_model_index(aIndex)))
                    return false;
                item_presentation_model_index const indexFirstColumn{ aIndex.row() };
                if (!cell_meta(indexFirstColumn).expanded)
                    ItemExpanding(aIndex);
                else
                    ItemCollapsing(aIndex);
                cell_meta(indexFirstColumn).expanded = !cell_meta(indexFirstColumn).expanded;
                if (cell_meta(indexFirstColumn).expanded)
                    std::next(begin(), aIndex.row()).unskip_children();
                else
                    std::next(begin(), aIndex.row()).skip_children();
                reset_row_map();
                reset_meta();
                if (cell_meta(indexFirstColumn).expanded)
                    ItemExpanded(aIndex);
                else
                    ItemCollapsed(aIndex);
                return true;
            }
            else
                return false;
        }
        bool expand_to(item_model_index const& aIndex) final
        {
            if constexpr (container_traits::is_tree)
            {
                if (!has_item_model_index(aIndex) && item_model().has_parent(aIndex))
                    expand_to(item_model().parent(aIndex));
                expand(from_item_model_index(aIndex));
                return true;
            }
            else
                return false;
        }
        const button_checked_state& checked_state(item_presentation_model_index const& aIndex) final
        {
            return cell_meta(aIndex).checked;
        }
        bool is_checked(item_presentation_model_index const& aIndex) const final
        {
            return cell_meta(aIndex).checked == true;
        }
        bool is_unchecked(item_presentation_model_index const& aIndex) const final
        {
            return cell_meta(aIndex).checked == false;
        }
        bool is_indeterminate(item_presentation_model_index const& aIndex) const final
        {
            return cell_meta(aIndex).checked == std::nullopt;
        }
        void set_checked_state(item_presentation_model_index const& aIndex, button_checked_state const& aState) final
        {
            if (cell_meta(aIndex).checked != aState)
            {
                cell_meta(aIndex).checked = aState;
                if (aState == std::nullopt && !cell_tri_state_checkable(aIndex))
                    throw not_tri_state_checkable();
                cell_meta(aIndex).checked = aState;
                ItemToggled(aIndex);
                if (is_checked(aIndex))
                    ItemChecked(aIndex);
                else if (is_unchecked(aIndex))
                    ItemUnchecked(aIndex);
                else if (is_indeterminate(aIndex))
                    ItemIndeterminate(aIndex);
            }
        }
        void check(item_presentation_model_index const& aIndex) final
        {
            set_checked(aIndex, true);
        }
        void uncheck(item_presentation_model_index const& aIndex) final
        {
            set_checked(aIndex, false);
        }
        void set_indeterminate(item_presentation_model_index const& aIndex) final
        {
            set_checked_state(aIndex, button_checked_state{});
        }
        void set_checked(item_presentation_model_index const& aIndex, bool aChecked) final
        {
            set_checked_state(aIndex, aChecked);
        }
        void toggle_check(item_presentation_model_index const& aIndex) final
        {
            if (is_checked(aIndex))
            {
                if (cell_tri_state_checkable(aIndex))
                    set_indeterminate(aIndex);
                else
                    set_checked(aIndex, false);
            }
            else if (is_indeterminate(aIndex))
                set_checked(aIndex, false);
            else
                set_checked(aIndex, true);
        }
        const font& default_font() const final
        {
            if (iDefaultFont != std::nullopt)
                return *iDefaultFont;
            return service<i_app>().current_style().font();
        }
        void set_default_font(optional_font const& aDefaultFont) final
        {
            if (iDefaultFont != aDefaultFont)
            {
                iDefaultFont = aDefaultFont;
                reset_meta();
            }
        }
        size cell_spacing(i_units_context const& aUnitsContext) const final
        {
            if (iCellSpacing == std::nullopt)
            {
                std::optional<scoped_units> su;
                if (attached())
                    su.emplace(attachment(), scoped_units::current_units());
                size result{ 1.0_mm, 1.0_mm };
                if (to_px<std::uint32_t>(result.cx) % 2u == 1u)
                    result.cx = from_px<dimension>(to_px<std::uint32_t>(result.cx) + 1u);
                if (to_px<std::uint32_t>(result.cy) % 2u == 1u)
                    result.cy = from_px<dimension>(to_px<std::uint32_t>(result.cy) + 1u);
                return result;
            }
            return units_converter(aUnitsContext).from_device_units(*iCellSpacing);
        }
        void set_cell_spacing(optional_size const& aSpacing, i_units_context const& aUnitsContext) final
        {
            if (aSpacing == std::nullopt)
                iCellSpacing = aSpacing;
            else
                iCellSpacing = units_converter(aUnitsContext).to_device_units(*aSpacing);
        }
        neogfx::padding cell_padding(i_units_context const& aUnitsContext) const final
        {
            if (iCellPadding == std::nullopt)
            {
                return units_converter(aUnitsContext).from_device_units(neogfx::padding{ 1.0, 1.0, 1.0, 1.0 });
            }
            return units_converter(aUnitsContext).from_device_units(*iCellPadding);
        }
        void set_cell_padding(optional_padding const& aPadding, i_units_context const& aUnitsContext) final
        {
            if (aPadding == std::nullopt)
                iCellPadding = aPadding;
            else
                iCellPadding = units_converter(aUnitsContext).to_device_units(*aPadding);
        }
        bool alternating_row_color() const final
        {
            return iAlternatingRowColor;
        }
        void set_alternating_row_color(bool aAlternatingColor) final
        {
            iAlternatingRowColor = aAlternatingColor;
        }
    public:
        dimension item_height(item_presentation_model_index const& aIndex, i_units_context const& aUnitsContext) const final
        {
            dimension height = 0.0;
            for (std::uint32_t col = 0; col < row(aIndex).cells.size(); ++col)
            {
                auto const index = item_presentation_model_index{ aIndex.row(), col };
                auto const modelIndex = to_item_model_index(index);
                if (cell_meta(index).extents != std::nullopt)
                    height = std::max(height, units_converter(aUnitsContext).from_device_units(*cell_meta(index).extents).cy);
                else
                {
                    std::string cellString = cell_to_string(index);
                    auto const& cellFont = cell_font(index);
                    auto const& effectiveFont = (cellFont == std::nullopt ? default_font() : *cellFont);
                    height = std::max(height, units_converter(aUnitsContext).from_device_units(size{ 0.0, std::ceil(effectiveFont.height()) }).cy *
                        (1 + std::count(cellString.begin(), cellString.end(), '\n')));
                    auto const& maybeCellImageSize = cell_image_size(index);
                    if (maybeCellImageSize != std::nullopt)
                        height = std::max(height, units_converter(aUnitsContext).from_device_units(*maybeCellImageSize).cy);
                    auto const& cellInfo = item_model().cell_info(modelIndex);
                    if (cell_editable(index) && cellInfo.dataStep != neolib::none)
                        height = std::max<dimension>(height, dip(basic_spin_box<double>::SPIN_BUTTON_MINIMUM_SIZE.cy * 2.0));
                }
            }
            return height + cell_padding(aUnitsContext).size().cy + cell_spacing(aUnitsContext).cy;
        }
        double total_height(i_units_context const& aUnitsContext) const final
        {
            if (iTotalHeight != std::nullopt)
                return *iTotalHeight;
            i_scrollbar::value_type height = 0.0;
            for (item_presentation_model_index::row_type row = 0; row < rows(); ++row)
                height += item_height(item_presentation_model_index(row, 0), aUnitsContext);
            iTotalHeight = height;
            return *iTotalHeight;
        }
        double item_position(item_presentation_model_index const& aIndex, i_units_context const& aUnitsContext) const final
        {
            if (iPositions[aIndex.row()] == std::nullopt)
            {
                auto pred = [](const optional_position& lhs, const optional_position& rhs) -> bool
                {
                    if (lhs == std::nullopt && rhs == std::nullopt)
                        return false;
                    else if (lhs != std::nullopt && rhs == std::nullopt)
                        return true;
                    else if (lhs == std::nullopt && rhs != std::nullopt)
                        return false;
                    else
                        return *lhs < *rhs;
                };
                auto i = std::lower_bound(iPositions.begin(), iPositions.end(), optional_position(), pred);
                auto row = static_cast<item_presentation_model_index::row_type>(std::distance(iPositions.begin(), i));
                double position = (row == 0 ? 0.0 : **(std::prev(i)) + item_height(item_presentation_model_index(row - 1), aUnitsContext));
                while (row <= aIndex.row())
                {
                    iPositions[row] = position;
                    position += item_height(item_presentation_model_index(row), aUnitsContext);
                    ++row;
                }
            }
            return *iPositions[aIndex.row()];
        }
        std::pair<item_presentation_model_index::row_type, coordinate> item_at(double aPosition, i_units_context const& aUnitsContext) const final
        {
            if (rows() == 0)
                return std::pair<item_presentation_model_index::row_type, coordinate>{ 0u, 0.0 };
            auto pred = [](const optional_position& lhs, const optional_position& rhs) -> bool
            {
                if (lhs == std::nullopt && rhs == std::nullopt)
                    return false;
                else if (lhs != std::nullopt && rhs == std::nullopt)
                    return true;
                else if (lhs == std::nullopt && rhs != std::nullopt)
                    return false;
                else
                    return *lhs < *rhs;
            };
            auto i = std::lower_bound(iPositions.begin(), iPositions.end(), aPosition, pred);
            if ((i == iPositions.end() || (*i != std::nullopt && **i > aPosition)) && i != iPositions.begin())
                --i;
            while (*i == std::nullopt)
            {
                auto j = std::lower_bound(iPositions.begin(), iPositions.end(), optional_position(), pred);
                auto row = static_cast<item_presentation_model_index::row_type>(std::distance(iPositions.begin(), j));
                double position = (row == 0 ? 0.0 : **(std::prev(j)) + item_height(item_presentation_model_index(row - 1), aUnitsContext));
                while (row < iPositions.size())
                {
                    iPositions[row] = position;
                    position += item_height(item_presentation_model_index(row), aUnitsContext);
                    ++row;
                }
                i = std::lower_bound(iPositions.begin(), iPositions.end(), aPosition, pred);
                if ((i == iPositions.end() || (*i != std::nullopt && **i > aPosition)) && i != iPositions.begin())
                    --i;
            }
            auto const result = std::pair<item_presentation_model_index::row_type, coordinate>{ static_cast<item_presentation_model_index::row_type>(std::distance(iPositions.begin(), i)), static_cast<coordinate>(**i - aPosition) };
            return result;
        }
    public:
        item_cell_flags cell_flags(item_presentation_model_index const& aIndex) const override
        {
            if (cell_meta(aIndex).flags != std::nullopt)
                return *cell_meta(aIndex).flags;
            return column_flags(aIndex.column());
        }
        void set_cell_flags(item_presentation_model_index const& aIndex, item_cell_flags aFlags) final
        {
            if (cell_meta(aIndex).flags != aFlags)
            {
                cell_meta(aIndex).flags = aFlags;
                ItemChanged(aIndex);
            }
        }
        cell_meta_type& cell_meta(item_presentation_model_index const& aIndex) const final
        {
            if (aIndex.row() < rows())
            {
                if (aIndex.column() >= row(aIndex).cells.size())
                {
                    row(aIndex).cells.resize(aIndex.column() + 1);
                    if constexpr (container_traits::is_tree)
                        if (aIndex.column() == 0)
                            row(aIndex).cells[aIndex.column()].expanded = !std::next(begin(), aIndex.row()).children_skipped();
                }
                return row(aIndex).cells[aIndex.column()];
            }
            throw bad_index();
        }
    public:
        std::string cell_to_string(item_presentation_model_index const& aIndex) const final
        {
            auto const modelIndex = to_item_model_index(aIndex);
            return std::visit([&, this](auto&& arg) -> std::string
            {
                typedef std::decay_t<decltype(arg)> type;
                if constexpr(!std::is_same_v<type, std::monostate> && classify_item_call_data<type>::category == item_cell_data_category::Value)
                    return (cell_format(aIndex) % std::get<type>(item_model().cell_data(modelIndex))).str();
                else
                    return "";
            }, item_model().cell_data(modelIndex));
        }
        item_cell_data string_to_cell_data(item_presentation_model_index const& aIndex, std::string const& aString) const final
        {
            bool error = false;
            return string_to_cell_data(aIndex, aString, error);
        }
        item_cell_data string_to_cell_data(item_presentation_model_index const& aIndex, std::string const& aString, bool& aError) const final
        {
            aError = false;
            auto const& cellInfo = item_model().cell_info(to_item_model_index(aIndex));
            std::istringstream input{ aString };
            std::string guff;
            switch (cellInfo.dataType)
            {
            case item_data_type::Bool:
                {
                    bool value;
                    if (!(input >> value) || (input >> guff))
                    {
                        aError = true;
                        return has_item_model() ? item_model().cell_data(to_item_model_index(aIndex)) : item_cell_data{};
                    }
                    return value;
                }
            case item_data_type::Int32:
                {
                    std::int32_t value;
                    if (!(input >> value) || (input >> guff))
                    {
                        aError = true;
                        return has_item_model() ? item_model().cell_data(to_item_model_index(aIndex)) : item_cell_data{};
                    }
                    if (cellInfo.dataMin != neolib::none)
                        value = std::max(value, static_variant_cast<std::int32_t>(cellInfo.dataMin));
                    if (cellInfo.dataMax != neolib::none)
                        value = std::min(value, static_variant_cast<std::int32_t>(cellInfo.dataMax));
                    return value;
                }
            case item_data_type::UInt32:
                {
                    std::uint32_t value;
                    if (!(input >> value) || (input >> guff))
                    {
                        aError = true;
                        return has_item_model() ? item_model().cell_data(to_item_model_index(aIndex)) : item_cell_data{};
                    }
                    if (cellInfo.dataMin != neolib::none)
                        value = std::max(value, static_variant_cast<std::uint32_t>(cellInfo.dataMin));
                    if (cellInfo.dataMax != neolib::none)
                        value = std::min(value, static_variant_cast<std::uint32_t>(cellInfo.dataMax));
                    return value;
                }
            case item_data_type::Int64:
                {
                    std::int64_t value;
                    if (!(input >> value) || (input >> guff))
                    {
                        aError = true;
                        return has_item_model() ? item_model().cell_data(to_item_model_index(aIndex)) : item_cell_data{};
                    }
                    if (cellInfo.dataMin != neolib::none)
                        value = std::max(value, static_variant_cast<std::int64_t>(cellInfo.dataMin));
                    if (cellInfo.dataMax != neolib::none)
                        value = std::min(value, static_variant_cast<std::int64_t>(cellInfo.dataMax));
                    return value;
                }
            case item_data_type::UInt64:
                {
                    std::uint64_t value;
                    if (!(input >> value) || (input >> guff))
                    {
                        aError = true;
                        return has_item_model() ? item_model().cell_data(to_item_model_index(aIndex)) : item_cell_data{};
                    }
                    if (cellInfo.dataMin != neolib::none)
                        value = std::max(value, static_variant_cast<std::uint64_t>(cellInfo.dataMin));
                    if (cellInfo.dataMax != neolib::none)
                        value = std::min(value, static_variant_cast<std::uint64_t>(cellInfo.dataMax));
                    return value;
                }
            case item_data_type::Float:
                {
                    float value;
                    if (!(input >> value) || (input >> guff))
                    {
                        aError = true;
                        return has_item_model() ? item_model().cell_data(to_item_model_index(aIndex)) : item_cell_data{};
                    }
                    if (cellInfo.dataMin != neolib::none)
                        value = std::max(value, static_variant_cast<float>(cellInfo.dataMin));
                    if (cellInfo.dataMax != neolib::none)
                        value = std::min(value, static_variant_cast<float>(cellInfo.dataMax));
                    return value;
                }
            case item_data_type::Double:
                {
                    double value;
                    if (!(input >> value) || (input >> guff))
                    {
                        aError = true;
                        return has_item_model() ? item_model().cell_data(to_item_model_index(aIndex)) : item_cell_data{};
                    }
                    if (cellInfo.dataMin != neolib::none)
                        value = std::max(value, static_variant_cast<double>(cellInfo.dataMin));
                    if (cellInfo.dataMax != neolib::none)
                        value = std::min(value, static_variant_cast<double>(cellInfo.dataMax));
                    return value;
                }
            case item_data_type::String:
            default:
                if (!aString.empty())
                    return string{ aString };
                else
                    return {};
            }
        }
        boost::basic_format<char> cell_format(item_presentation_model_index const&) const override
        {
            static const boost::basic_format<char> sDefaultFormat("%1%");
            return sDefaultFormat;
        }
        optional_color cell_color(item_presentation_model_index const&, color_role) const override
        {
            return optional_color{};
        }
        optional_font cell_font(item_presentation_model_index const&) const override
        {
            return optional_font{};
        }
        optional_size cell_image_size(item_presentation_model_index const& aIndex) const override
        {
            if (column(aIndex.column()).imageSize)
                return column(aIndex.column()).imageSize;
            else if (cell_image(aIndex))
                return cell_image(aIndex)->extents();
            return {};
        }
        optional_size cell_check_box_size(item_presentation_model_index const& aIndex, i_units_context const& aUnitsContext) const override
        {
            if (cell_checkable(aIndex))
            {
                auto const& cellFont = cell_font(aIndex);
                auto const& effectiveFont = (cellFont == std::nullopt ? default_font() : *cellFont);
                dimension const length = units_converter(aUnitsContext).from_device_units(effectiveFont.height() * (2.0 / 3.0));
                auto const checkBoxSize = service<i_skin_manager>().active_skin().preferred_size(skin_element::CheckBox, size{ length });
                return checkBoxSize;
            }
            return {};
        }
        optional_size cell_tree_expander_size(item_presentation_model_index const& aIndex, i_units_context const& aUnitsContext) const override
        {
            if constexpr (container_traits::is_flat)
                return {};
            else
                return size{ 16.0_dip };
        }
        optional_texture cell_image(item_presentation_model_index const&) const override
        {
            return optional_texture{};
        }
        neogfx::glyph_text& cell_glyph_text(item_presentation_model_index const& aIndex) const override
        {
            if (cell_meta(aIndex).text != std::nullopt)
                return *cell_meta(aIndex).text;
            auto const& cellFont = cell_font(aIndex);
            auto const& effectiveFont = (cellFont == std::nullopt ? default_font() : *cellFont);
            cell_meta(aIndex).text = graphics_context{ attachment(), graphics_context::type::Unattached }.
                to_glyph_text(cell_to_string(aIndex), effectiveFont);
            return *cell_meta(aIndex).text;
        }
        size cell_extents(item_presentation_model_index const& aIndex, i_units_context const& aUnitsContext) const override
        {
            auto oldItemHeight = item_height(aIndex, aUnitsContext);
            auto const& cellFont = cell_font(aIndex);
            auto const& effectiveFont = (cellFont == std::nullopt ? default_font() : *cellFont);
            auto& cellMeta = cell_meta(aIndex);
            if (cellMeta.extents != std::nullopt)
                return units_converter(aUnitsContext).from_device_units(*cellMeta.extents);
            size cellExtents = cell_glyph_text(aIndex).extents();
            auto const& cellInfo = item_model().cell_info(to_item_model_index(aIndex));
            if (cell_editable(aIndex) && cellInfo.dataStep != neolib::none)
            {
                cellExtents.cx = std::max(cellExtents.cx, graphics_context{ attachment(), graphics_context::type::Unattached }.
                    text_extent(cellInfo.dataMax.to_string(), effectiveFont).cx);
                cellExtents.cx += dip(basic_spin_box<double>::INTERNAL_SPACING.cx + basic_spin_box<double>::SPIN_BUTTON_MINIMUM_SIZE.cx); // todo: get this from widget metrics (skin API)
                cellExtents.cy = std::max<dimension>(cellExtents.cy, dip(basic_spin_box<double>::SPIN_BUTTON_MINIMUM_SIZE.cy * 2.0));
            }
            cellExtents.cx += indent(aIndex, aUnitsContext);
            auto const& maybeCheckBoxSize = cell_check_box_size(aIndex, aUnitsContext);
            if (maybeCheckBoxSize != std::nullopt)
            {
                cellExtents.cx += (maybeCheckBoxSize->cx + units_converter(aUnitsContext).to_device_units(cell_spacing(aUnitsContext)).cx);
                cellExtents.cy = std::max(cellExtents.cy, maybeCheckBoxSize->cy);
            }
            auto const& maybeCellImageSize = cell_image_size(aIndex);
            if (maybeCellImageSize != std::nullopt)
            {
                cellExtents.cx += (maybeCellImageSize->cx + units_converter(aUnitsContext).to_device_units(cell_spacing(aUnitsContext)).cx);
                cellExtents.cy = std::max(cellExtents.cy, maybeCellImageSize->cy);
            }
            cellExtents.cy = std::max(cellExtents.cy, effectiveFont.height());
            cache_cell_meta_extents(aIndex, cellExtents.ceil());
            if (iTotalHeight != std::nullopt)
                *iTotalHeight += (item_height(aIndex, aUnitsContext) - oldItemHeight);
            return units_converter(aUnitsContext).from_device_units(*cell_meta(aIndex).extents);
        }
        dimension indent(item_presentation_model_index const& aIndex, i_units_context const& aUnitsContext) const override
        {
            if constexpr (container_traits::is_flat)
                return 0.0;
            else if (aIndex.column() != 0)
                return 0.0;
            else
            {
                auto baseIter = typename item_model_type::const_base_iterator{ item_model().index_to_iterator(to_item_model_index(aIndex)) };
                auto treeIter = baseIter.get<typename item_model_type::const_iterator, typename item_model_type::const_iterator,
                    typename item_model_type::iterator, typename item_model_type::const_sibling_iterator, typename item_model_type::sibling_iterator>();
                return (treeIter.depth() + 1) * cell_tree_expander_size(aIndex, aUnitsContext)->cx;
            }
        }
    public:
        void sort(i_item_sort_predicate const& aPredicate) final
        {
            iSortOrder.clear();
            ItemsSorting();
            if constexpr (container_traits::is_flat)
                std::sort(iRows.begin(), iRows.end(), [&](auto const& lhs, auto const& rhs) { return aPredicate.compare(lhs.value, rhs.value); });
            else
                iRows.sort([&](auto const& lhs, auto const& rhs) { return aPredicate.compare(lhs.value, rhs.value); });
            reset_row_map();
            reset_position_meta(0);
            ItemsSorted();
        }
        bool sortable() const final
        {
            return iSortable;
        }
        void set_sortable(bool aSortable) final
        {
            iSortable = aSortable;
        }
        optional_sort_by_param sorting_by() const final
        {
            if (!iSortOrder.empty())
                return iSortOrder.front();
            else
                return optional_sort_by_param{};
        }
        void sort_by(item_presentation_model_index::column_type aColumnIndex, const optional_sort_direction& aSortDirection = optional_sort_direction{}) final
        {
            iSortOrder.push_front(sort_by_param{ aColumnIndex, aSortDirection == std::nullopt ? sort_direction::Ascending : *aSortDirection });
            for (auto i = std::next(iSortOrder.begin()); i != iSortOrder.end(); ++i)
            {
                if (i->first == aColumnIndex)
                {
                    if (aSortDirection == std::nullopt)
                    {
                        if (i == std::next(iSortOrder.begin()))
                            iSortOrder.front().second = (i->second == sort_direction::Ascending ? sort_direction::Descending : sort_direction::Ascending);
                        else
                            iSortOrder.front().second = i->second;
                    }
                    iSortOrder.erase(i);
                    break;
                }
            }
            execute_sort(true);
        }
        void reset_sort() final
        {
            iSortOrder.clear();
            if (sortable())
                execute_sort();
        }
    public:
        optional_item_presentation_model_index find_item(filter_search_key const& aFilterSearchKey, item_presentation_model_index::column_type aColumnIndex = 0, 
            filter_search_type aFilterSearchType = filter_search_type::Prefix, case_sensitivity aCaseSensitivity = case_sensitivity::CaseInsensitive) const final
        {
            for (item_presentation_model_index::row_type row = 0; row < rows(); ++row)
            {
                auto modelIndex = to_item_model_index(item_presentation_model_index{ row, aColumnIndex });
                auto const& origValue = item_model().cell_data(modelIndex).to_string();
                auto const& value = aCaseSensitivity == case_sensitivity::CaseSensitive ? origValue : boost::to_upper_copy<std::string>(origValue);
                auto const& origKey = aFilterSearchKey;
                auto const& key = aCaseSensitivity == case_sensitivity::CaseSensitive ? origKey : boost::to_upper_copy<std::string>(origKey);
                if (key.empty())
                    continue;
                switch (aFilterSearchType)
                {
                case filter_search_type::Prefix:
                    if (value.size() >= key.size() && value.substr(0, key.size()) == key)
                        return from_item_model_index(modelIndex);
                }
            }
            return optional_item_presentation_model_index{};
        }
    public:
        bool filtering() const final
        {
            return iFiltering;
        }
        optional_filter filtering_by() const final
        {
            if (!iFilters.empty())
                return iFilters.front();
            else
                return optional_filter{};
        }
        void filter_by(item_presentation_model_index::column_type aColumnIndex, filter_search_key const& aFilterSearchKey, 
            filter_search_type aFilterSearchType = filter_search_type::Value, case_sensitivity aCaseSensitivity = case_sensitivity::CaseInsensitive) final
        {
            iFilters.push_back(filter{ aColumnIndex, aFilterSearchKey, aFilterSearchType, aCaseSensitivity });
            for (auto i = iFilters.begin(); i != std::prev(iFilters.end()); ++i)
            {
                if (std::get<0>(*i) == aColumnIndex)
                {
                    iFilters.erase(i);
                    break;
                }
            }            
            execute_filter();
        }
        void reset_filter() final
        {
            if (!iFilters.empty())
            {
                iFilters.clear();
                execute_filter();
            }
        }
    private:
        void init()
        {
            iSink = service<i_rendering_engine>().subpixel_rendering_changed([this]()
            {
                reset_meta();
            });
            iSink += service<i_app>().current_style_changed([this](style_aspect aAspect)
            {
                if ((aAspect & (style_aspect::Geometry | style_aspect::Font)) != style_aspect::None)
                    reset_meta();
            });
            reset_sort();
            set_alive();
        }
        void execute_sort(bool aForce = false)
        {
            if (!sortable() && !aForce)
                return;
            if (rows() <= 1)
                return;
            if (iSortOrder.empty())
            {
                sort_by(0, sort_direction::Ascending);
                return;
            }
            ItemsSorting();
            auto sortPredicate = [&](const typename container_type::value_type& aLhs, const typename container_type::value_type& aRhs) -> bool
            {
                for (std::size_t i = 0; i < iSortOrder.size(); ++i)
                {
                    auto col = iSortOrder[i].first;
                    auto const& v1 = item_model().cell_data(item_model_index{ aLhs.value, model_column(col) });
                    auto const& v2 = item_model().cell_data(item_model_index{ aRhs.value, model_column(col) });
                    if (std::holds_alternative<string>(v1) && std::holds_alternative<string>(v2))
                    {
                        std::string s1 = boost::to_upper_copy<std::string>(std::get<string>(v1));
                        std::string s2 = boost::to_upper_copy<std::string>(std::get<string>(v2));
                        if (s1 < s2)
                            return iSortOrder[i].second == sort_direction::Ascending;
                        else if (s2 < s1)
                            return iSortOrder[i].second == sort_direction::Descending;
                    }
                    if (v1 < v2)
                        return iSortOrder[i].second == sort_direction::Ascending;
                    else if (v2 < v1)
                        return iSortOrder[i].second == sort_direction::Descending;
                }
                return false;
            };
            if constexpr (container_traits::is_flat)
                std::sort(iRows.begin(), iRows.end(), sortPredicate);
            else
                iRows.sort(sortPredicate);
            reset_row_map();
            reset_position_meta(0);
            ItemsSorted();
        }
        void execute_filter()
        {
            {
                scoped_item_update siu{ *this };
                neolib::scoped_flag sf2{ iFiltering };
                ItemsFiltering();
                iRows.clear();
                for (item_model_index::row_type row = 0; row < item_model().rows(); ++row)
                {
                    bool matches = true;
                    for (auto const& filter : iFilters)
                    {
                        auto const& origValue = item_model().cell_data(item_model_index{ row, model_column(std::get<0>(filter)) }).to_string();
                        auto const& value = (std::get<3>(filter) == case_sensitivity::CaseSensitive ? origValue : boost::to_upper_copy<std::string>(origValue));
                        auto const& origKey = std::get<1>(filter);
                        auto const& key = (std::get<3>(filter) == case_sensitivity::CaseSensitive ? origKey : boost::to_upper_copy<std::string>(origKey));
                        if (key.empty())
                            continue;
                        switch (std::get<2>(filter))
                        {
                        case filter_search_type::Prefix:
                            if (value.size() < key.size() || value.substr(0, key.size()) != key)
                                matches = false;
                            break;
                        case filter_search_type::Glob:
                            // todo
                            break;
                        case filter_search_type::Regex:
                            // todo
                            break;
                        }
                    }
                    if (matches)
                        item_added(item_model_index{ row });
                }
            }
            ItemsFiltered();
            execute_sort();
        }
    private:
        void item_model_column_info_changed(item_model_index::column_type aColumnIndex)
        {
            reset_column_map(false);
            if (has_item_model_index(item_model_index{ 0, aColumnIndex }))
            {
                auto const index = from_item_model_index(item_model_index{ 0, aColumnIndex });
                reset_cell_meta(index.column());
                reset_column_meta(index.column());
                ColumnInfoChanged(index.column());
            }
        }
        void item_added(const item_model_index& aItemIndex)
        {
            if constexpr (container_traits::is_tree)
                if (item_model().has_parent(aItemIndex) && !has_item_model_index(item_model().parent(aItemIndex)))
                    return;
            for (auto& row : iRows)
                if (row.value >= aItemIndex.row())
                    ++row.value;
            if constexpr (container_traits::is_flat)
                iRows.push_back(row_type{ aItemIndex.row() });
            else
            {
                if (!item_model().has_parent(aItemIndex))
                {
                    auto const pos = iRows.csend();
                    iRows.insert(pos, row_type{ aItemIndex.row() });
                }
                else
                {
                    auto const parentIndex = from_item_model_index(item_model().parent(aItemIndex));
                    auto const pos = const_sibling_iterator{ std::next(iRows.cbegin(), parentIndex.row()) };
                    iRows.insert(pos.end(), row_type{ aItemIndex.row() });
                }
            }

            if (!updating() || container_traits::is_tree)
                reset_row_map(aItemIndex);

            if (!updating())
            {
                reset_position_meta(aItemIndex.row());
                execute_sort();
                ItemAdded(from_item_model_index(aItemIndex, true));
            }
        }
        void item_changed(const item_model_index& aItemIndex)
        {
            if (!has_item_model_index(aItemIndex))
                return;
            if (!updating())
            {
                reset_row_map();
                reset_position_meta(aItemIndex.row());
                execute_sort();
                auto const index = from_item_model_index(aItemIndex);
                auto& cellMeta = cell_meta(index);
                cellMeta.text = std::nullopt;
                cache_cell_meta_extents(index, std::nullopt);
                if (attached())
                    cell_extents(index, attachment());
                ItemChanged(from_item_model_index(aItemIndex));
            }
        }
        void item_removing(const item_model_index& aItemIndex)
        {
            if (!has_item_model_index(aItemIndex))
                return;
            auto const index = from_item_model_index(aItemIndex);
            for (item_presentation_model_index::column_type col = 0; col < columns(); ++col)
                cache_cell_meta_extents(index.with_column(col), std::nullopt);
            if (!updating())
                ItemRemoving(index);
            iRows.erase(std::next(begin(), index.row()));
            for (auto& row : iRows)
                if (row.value >= aItemIndex.row())
                    --row.value;
            if (iRowMap[aItemIndex.row()])
                for (auto& row : iRowMap)
                    if (row && *row >= index.row())
                        --* row;
            iRowMap.erase(std::next(iRowMap.begin(), aItemIndex.row()));
            if (!updating())
            {
                reset_position_meta(index.row());
                ItemRemoved(index);
            }
        }
        void item_removed(const item_model_index& aItemIndex)
        {
        }
    private:
        void cache_cell_meta_extents(item_presentation_model_index const& aIndex, const optional_size& aExtents = {}) const
        {
            auto& cellMeta = cell_meta(aIndex);
            if (aExtents != cellMeta.extents)
            {   
                auto previousExtents = cellMeta.extents;
                cellMeta.extents = aExtents;
                if (previousExtents)
                    column(aIndex.column()).remove_cell_width(previousExtents->cx);
                if (cellMeta.extents)
                    column(aIndex.column()).add_cell_width(cellMeta.extents->cx);
            }
        }
        void reset_maps(const item_model_index& aFrom = {}) const
        {
            reset_row_map(aFrom);
            reset_column_map();
        }
        void reset_row_map(const item_model_index& aFrom = {}) const
        {
            if (aFrom.row() < iRowMap.size() && (iRowMapDirtyFrom == std::nullopt || *iRowMapDirtyFrom > aFrom.row()))
                iRowMapDirtyFrom = aFrom.row();
        }
        void reset_column_map(bool aClear = true) const
        {
            if (aClear)
                iColumnMap.clear();
            if (has_item_model())
                for (item_model_index::column_type col = 0; col < item_model().columns(); ++col)
                    mapped_column(col);
        }
        item_presentation_model_index::row_type mapped_row(item_model_index::row_type aRowIndex) const
        {
            if (aRowIndex < row_map().size() && row_map()[aRowIndex])
                return *row_map()[aRowIndex];
            throw no_mapped_row();
        }
        const row_map_type& row_map() const
        {
            if (iRowMapDirtyFrom)
                iRowMap.erase(std::next(iRowMap.begin(), *iRowMapDirtyFrom), iRowMap.end());
            iRowMapDirtyFrom = std::nullopt;
            if (iRowMap.size() < item_model().rows())
            {
                iRowMap.resize(item_model().rows());
                for (item_presentation_model_index::row_type row = 0; row < rows(); ++row)
                    iRowMap[basic_item_presentation_model::row(row).value] = row;
            }
            return iRowMap;
        }
        row_map_type& row_map()
        {
            return const_cast<row_map_type&>(to_const(*this).row_map());
        }
        item_presentation_model_index::column_type mapped_column(item_model_index::column_type aColumnIndex) const
        {
            if (aColumnIndex >= iColumnMap.size())
                iColumnMap.resize(aColumnIndex + 1);
            auto& mapCol = iColumnMap[aColumnIndex];
            if (!mapCol)
            {
                for (item_presentation_model_index::column_type col = 0; col < columns(); ++col)
                    if (iColumns[col].modelColumn == aColumnIndex)
                        mapCol = col;
                if (!mapCol)
                {
                    auto newColumn = columns();
                    for (item_presentation_model_index::column_type col = 0; col < columns(); ++col)
                        if (iColumns[col].modelColumn == std::nullopt)
                        {
                            newColumn = col;
                            break;
                        }
                    column(newColumn).modelColumn = aColumnIndex;
                    mapCol = newColumn;
                }
            }
            return *mapCol;
        }
        bool has_model_column(item_presentation_model_index::column_type aColumnIndex) const
        {
            auto const& col = column(aColumnIndex);
            if (col.modelColumn)
                reset_column_map(false);
            return col.modelColumn.has_value();
        }
        item_model_index::column_type model_column(item_presentation_model_index::column_type aColumnIndex) const
        {
            auto const& col = column(aColumnIndex);
            if (col.modelColumn)
                return *col.modelColumn;
            reset_column_map(false);
            if (col.modelColumn)
                return *col.modelColumn;
            throw bad_index();
        }
        const column_map_type& column_map() const
        {
            return iColumnMap;
        }
        column_map_type& column_map()
        {
            return iColumnMap;
        }
        void reset_meta() const
        {
            reset_cell_meta();
            reset_column_meta();
            reset_position_meta(0);

            if (attached())
                for (item_presentation_model_index::row_type row = 0; row < rows(); ++row)
                    for (item_presentation_model_index::column_type col = 0; col < iColumns.size(); ++col)
                        cell_extents(item_presentation_model_index{row, col}, attachment());
        }
        void reset_cell_meta(const std::optional<item_presentation_model_index::column_type>& aColumn = {}) const
        {
            for (item_presentation_model_index::row_type row = 0; row < rows(); ++row)
            {
                for (item_presentation_model_index::column_type col = 0; col < iColumns.size(); ++col)
                {
                    if (aColumn != std::nullopt && col != *aColumn)
                        continue;
                    item_presentation_model_index const index{ row, col };
                    cell_meta(index).text = std::nullopt;
                    cache_cell_meta_extents(index, std::nullopt);
                }
            }
        }
        void reset_column_meta(const std::optional<item_presentation_model_index::column_type>& aColumn = {}) const
        {
            for (item_presentation_model_index::column_type col = 0; col < iColumns.size(); ++col)
            {
                if (aColumn != std::nullopt && col != *aColumn)
                    continue;
                column(col).headingExtents = std::nullopt;
            }
        }
        void reset_position_meta(item_presentation_model_index::row_type aFromRow) const
        {
            iTotalHeight = std::nullopt;
            iPositions.resize(rows());
            for (std::size_t i = aFromRow; i < iPositions.size(); ++i)
                iPositions[i] = std::nullopt;
        }
    private:
        const_iterator cbegin() const
        {
            if constexpr (container_traits::is_flat)
                return iRows.begin();
            else
                return iRows.kbegin();
        }
        const_iterator begin() const
        {
            return cbegin();
        }
        iterator begin()
        {
            if constexpr (container_traits::is_flat)
                return iRows.begin();
            else
                return iRows.kbegin();
        }
        const_iterator cend() const
        {
            if constexpr (container_traits::is_flat)
                return iRows.end();
            else
                return iRows.kend();
        }
        const_iterator end() const
        {
            return cend();
        }
        iterator end()
        {
            if constexpr (container_traits::is_flat)
                return iRows.end();
            else
                return iRows.kend();
        }
    private:
        const row_type& row(item_presentation_model_index::row_type aRow) const
        {
            return *std::next(begin(), aRow);
        }
        const row_type& row(item_presentation_model_index aIndex) const
        {
            return row(aIndex.row());
        }
        row_type& row(item_presentation_model_index::row_type aRow)
        {
            return *std::next(begin(), aRow);
        }
        row_type& row(item_presentation_model_index aIndex)
        {
            return row(aIndex.row());
        }
        const column_info& column(item_presentation_model_index::column_type aColumnIndex) const
        {
            while(iColumns.size() <= aColumnIndex)
                iColumns.emplace_back();
            return iColumns[aColumnIndex];
        }
        column_info& column(item_presentation_model_index::column_type aColumnIndex)
        {
            return const_cast<column_info&>(to_const(*this).column(aColumnIndex));
        }
    private:
        weak_ref_ptr<i_widget> iAttachment;
        i_item_model* iItemModel;
        bool iSortable;
        sink iItemModelSink;
        optional_size iCellSpacing;
        optional_padding iCellPadding;
        container_type iRows;
        mutable row_map_type iRowMap;
        mutable item_model_index::optional_row_type iRowMapDirtyFrom;
        mutable column_info_array iColumns;
        mutable column_map_type iColumnMap;
        mutable optional_font iDefaultFont;
        mutable std::optional<i_scrollbar::value_type> iTotalHeight;
        mutable neolib::segmented_array<optional_position, 256> iPositions;
        bool iAlternatingRowColor;
        std::deque<sort_by_param> iSortOrder;
        std::vector<filter> iFilters;
        sink iSink;
        std::uint32_t iUpdating = 0u;
        bool iFiltering = false;
    };

    typedef basic_item_presentation_model<item_model> item_presentation_model;
    typedef basic_item_presentation_model<item_tree_model> item_tree_presentation_model;
}