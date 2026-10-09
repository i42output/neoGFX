// header_view.cpp
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

#include <neolib/core/lifetime.hpp>
#include <neolib/task/thread.hpp>

#include <neogfx/gui/widget/timer.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/action.hpp>
#include <neogfx/app/event_processing_context.hpp>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/widget/header_view.hpp>
#include <neogfx/gui/widget/push_button.hpp>
#include <neogfx/gui/window/context_menu.hpp>

namespace neogfx
{
    class header_button : public push_button
    {
    public:
        header_button(header_view& aParent) :
            push_button{ "", push_button_style::ItemViewHeader }, iParent{ aParent }
        {
        }
    public:
        size minimum_size(optional_size const& aAvailableSpace = optional_size{}) const override
        {
            if (has_minimum_size())
                return push_button::minimum_size(aAvailableSpace);
            else if (iParent.type() == header_view_type::Horizontal)
                return size{ iParent.layout().spacing().cx * 3.0, push_button::minimum_size(aAvailableSpace).cy };
            else
                return size{ push_button::minimum_size(aAvailableSpace).cx, iParent.layout().spacing().cy * 3.0 };
        }
    private:
        header_view& iParent;
    };

    header_view::header_view(i_header_view_owner& aOwner, header_view_type aType) :
        splitter{ splitter_style::ResizeSinglePane | (aType == header_view_type::Horizontal ? splitter_style::Horizontal : splitter_style::Vertical) },
        iOwner{ aOwner },
        iType{ aType },
        iExpandLastColumn{ last_column_expansion::DontExpand },
        iUpdatingSectionWidth { false }
    {
        init();
    }

    header_view::header_view(i_widget& aParent, i_header_view_owner& aOwner, header_view_type aType) :
        splitter{ aParent, splitter_style::ResizeSinglePane | (aType == header_view_type::Horizontal ? splitter_style::Horizontal : splitter_style::Vertical) },
        iOwner{ aOwner },
        iType{ aType },
        iExpandLastColumn{ last_column_expansion::DontExpand },
        iUpdatingSectionWidth{ false }
    {
        init();
    }

    header_view::header_view(i_layout& aLayout, i_header_view_owner& aOwner, header_view_type aType) :
        splitter{ aLayout, splitter_style::ResizeSinglePane | (aType == header_view_type::Horizontal ? splitter_style::Horizontal : splitter_style::Vertical) },
        iOwner{ aOwner },
        iType{ aType },
        iExpandLastColumn{ last_column_expansion::DontExpand },
        iUpdatingSectionWidth{ false }
    {
        init();
    }

    header_view::~header_view()
    {
    }

    header_view_type header_view::type() const
    {
        return iType;
    }

    bool header_view::has_model() const
    {
        if (iModel)
            return true;
        else
            return false;
    }
    
    const i_item_model& header_view::model() const
    {
        return *iModel;
    }

    i_item_model& header_view::model()
    {
        return *iModel;
    }

    void header_view::set_model(i_item_model& aModel)
    {
        set_model(ref_ptr<i_item_model>{ aModel });
    }

    void header_view::set_model(ref_ptr<i_item_model> aModel)
    {
        if (iModel == aModel)
            return;
        iModel = aModel;
        if (has_presentation_model() && !iOwner.updating_models())
        {
            if (presentation_model().is_item_model_related(model()))
                presentation_model().set_item_model(model());
            else
                set_presentation_model(ref_ptr<i_item_presentation_model>{});
        }
        full_update();
        update();
    }

    bool header_view::has_presentation_model() const
    {
        if (iPresentationModel)
            return true;
        else
            return false;
    }

    const i_item_presentation_model& header_view::presentation_model() const
    {
        return *iPresentationModel;
    }

    i_item_presentation_model& header_view::presentation_model()
    {
        return *iPresentationModel;
    }

    void header_view::set_presentation_model(i_item_presentation_model& aPresentationModel)
    {
        set_presentation_model(ref_ptr<i_item_presentation_model>{ aPresentationModel });
    }

    void header_view::set_presentation_model(ref_ptr<i_item_presentation_model> aPresentationModel)
    {
        if (iPresentationModel == aPresentationModel)
            return;
        iPresentationModelSink.clear();
        iPresentationModel = aPresentationModel;
        if (has_presentation_model())
        {
            presentation_model().column_info_changed([this](item_presentation_model_index::column_type aColumnIndex) { column_info_changed(aColumnIndex); });
            presentation_model().item_model_changed([this](const i_item_model& aItemModel) { item_model_changed(aItemModel); });
            presentation_model().item_added([this](item_presentation_model_index const& aItemIndex) { item_added(aItemIndex); });
            presentation_model().item_changed([this](item_presentation_model_index const& aItemIndex) { item_changed(aItemIndex); });
            presentation_model().item_removed([this](item_presentation_model_index const& aItemIndex) { item_removed(aItemIndex); });
            presentation_model().items_updated([this]() { items_updated(); });
            presentation_model().items_sorting([this]() { items_sorting(); });
            presentation_model().items_sorted([this]() { items_sorted(); });
            presentation_model().items_filtering([this]() { items_filtering(); });
            presentation_model().items_filtered([this]() { items_filtered(); });
            if (has_model() && !iOwner.updating_models())
                presentation_model().set_item_model(model());
        }
    }

    last_column_expansion header_view::expand_last_column() const
    {
        return iExpandLastColumn;
    }

    void header_view::set_expand_last_column(last_column_expansion aExpandLastColumn)
    {
        if (iExpandLastColumn != aExpandLastColumn)
        {
            iExpandLastColumn = aExpandLastColumn;
            full_update();
        }
    }

    void header_view::column_info_changed(item_presentation_model_index::column_type)
    {
        full_update();
    }

    void header_view::item_model_changed(const i_item_model&)
    {
        full_update();
    }

    void header_view::item_added(item_presentation_model_index const&)
    {
        iSectionWidths.resize(presentation_model().columns());
        /* todo : optimize (don't do full update) */
        full_update();
    }

    void header_view::item_changed(item_presentation_model_index const&)
    {
        iSectionWidths.resize(presentation_model().columns());
        if (layout().count() < presentation_model().columns())
        {
            full_update();
            return;
        }
        // a changed item can only make its column wider (the column widths are cached by the presentation model) so only the section 
        // widths need updating, and the owner only needs telling if one has changed (a full update reconfigures every section and makes the 
        // owner lay out all of its items again, for every changed item)
        bool sectionWidthChanged = false;
        for (std::uint32_t col = 0u; col < presentation_model().columns(); ++col)
            if (update_section_width(col, presentation_model().column_width(col, *this, true)))
                sectionWidthChanged = true;
        if (update_expanded_section())
            sectionWidthChanged = true;
        if (sectionWidthChanged)
            iOwner.header_view_updated(*this, header_view_update_reason::PanesResized);
    }

    void header_view::item_removed(item_presentation_model_index const&)
    {
        iSectionWidths.resize(presentation_model().columns());
        /* todo : optimize (don't do full update) */
        full_update();
    }

    void header_view::items_updated()
    {
        full_update();
    }

    void header_view::items_sorting()
    {
    }

    void header_view::items_sorted()
    {
        full_update();
    }

    void header_view::items_filtering()
    {
    }

    void header_view::items_filtered()
    {
        full_update();
    }

    dimension header_view::separator_width() const
    {
        if (iSeparatorWidth != std::nullopt)
            return units_converter{ *this }.from_device_units(*iSeparatorWidth);
        else if (has_presentation_model())
            return presentation_model().cell_spacing(*this).cx;
        else
            return ceil_rasterized(1.0_mm);
    }

    void header_view::set_separator_width(const optional_dimension& aWidth)
    {
        if (aWidth != std::nullopt)
            iSeparatorWidth = std::ceil(units_converter{ *this }.to_device_units(*aWidth));
        else
            iSeparatorWidth = std::nullopt;
    }

    std::uint32_t header_view::section_count() const
    {
        return static_cast<std::uint32_t>(iSectionWidths.size());
    }

    dimension header_view::section_width(std::uint32_t aSectionIndex, bool aForHeaderButton) const
    {
        if (iSectionWidths.empty())
            return 0.0;
        auto const lastColumn = presentation_model().columns() - 1u;
        // (a last column that is expanded to fit the view isn't as wide as its contents: as the rest of the view unless resized wider)
        auto result = units_converter{ *this }.from_device_units(iSectionWidths[aSectionIndex].manual != std::nullopt ?
            *iSectionWidths[aSectionIndex].manual :
            aSectionIndex == lastColumn && expand_last_column() == last_column_expansion::ExpandToFitView ? 0.0 : 
            iSectionWidths[aSectionIndex].calculated);
        thread_local bool tInSectionWidth = false;
        if (aSectionIndex == lastColumn && expand_last_column() != last_column_expansion::DontExpand && !tInSectionWidth && !iUpdatingSectionWidth)
        {
            neolib::scoped_flag sf{ tInSectionWidth };
            // (a last column expanded to fit the view fits the view (the header's parent) rather than the header itself: the header is as 
            // wide as its sections (and the separator and section that follow the last column) so would otherwise grow each time the last 
            // column did)
            auto const width = (expand_last_column() == last_column_expansion::ExpandToFitView && has_parent()) ? 
                parent().client_rect(false).cx : client_rect(false).cx;
            auto const delta = width - total_width();
            if (delta > 0.0)
                result += delta;
        }
        if (aForHeaderButton && aSectionIndex == 0)
            result += presentation_model().cell_spacing(*this).cx / 2.0;
        return result;
    }

    dimension header_view::total_width() const
    {
        if (!has_presentation_model())
            return 0.0;
        dimension result = 0.0;
        auto const separatorWidth = separator_width();
        for (std::uint32_t col = 0; col < presentation_model().columns(); ++col)
        {
            if (col != 0)
                result += separatorWidth;
            else
                result += separatorWidth / 2.0;
            result += section_width(col);
        }
        return result;
    }

    bool header_view::is_managing_layout() const
    {
        return true;
    }

    void header_view::panes_resized()
    {
        auto const lastColumn = presentation_model().columns() - 1u;
        for (std::uint32_t col = 0; col < presentation_model().columns(); ++col)
        {
            // (a last column expanded to fit the view whose button is as it was given (see update_expanded_section) hasn't been resized)
            if (col == lastColumn && expand_last_column() == last_column_expansion::ExpandToFitView && iExpandedSectionWidth &&
                layout().get_widget_at(col).fixed_size().cx == *iExpandedSectionWidth)
                continue;
            dimension oldSectionWidth = section_width(col);
            dimension newSectionWidth = layout().get_widget_at(col).fixed_size().cx;
            if (col == 0)
                newSectionWidth -= presentation_model().cell_spacing(*this).cx / 2.0;
            if (newSectionWidth != oldSectionWidth)
            {
                iSectionWidths[col].manual = newSectionWidth;
                if (col == lastColumn && expand_last_column() == last_column_expansion::ExpandToFitView)
                {
                    // (only a last column resized wider than the rest of the view keeps its width; otherwise (e.g. given a width that the 
                    // view is later too narrow for, a scrollbar shown) it fits the view as it is resized)
                    iSectionWidths[col].manual = std::nullopt;
                    if (newSectionWidth > section_width(col))
                        iSectionWidths[col].manual = newSectionWidth;
                }
            }
        }
        update_expanded_section();
        layout_items();
        iOwner.header_view_updated(*this, header_view_update_reason::PanesResized);
    }

    void header_view::reset_pane_sizes_requested(const std::optional<std::uint32_t>& aPane)
    {
        for (std::uint32_t col = 0; col < presentation_model().columns(); ++col)
        {
            if (aPane != std::nullopt && *aPane != col)
                continue;
            iSectionWidths[col].manual = std::nullopt;
            layout().get_widget_at(col).set_fixed_size({}, false);
            layout().get_widget_at(col).set_fixed_size(size(std::max(section_width(col), layout().spacing().cx * 3.0), layout().get_widget_at(col).minimum_size().cy), false);
        }
        update_expanded_section();
        iOwner.header_view_updated(*this, header_view_update_reason::PanesResized);
    }

    void header_view::init()
    {
        set_size_policy(iType == header_view_type::Horizontal ?
            neogfx::size_policy{ size_constraint::Maximum, size_constraint::Minimum } :
            neogfx::size_policy{ size_constraint::Minimum, size_constraint::Maximum });
        iSink += service<i_app>().current_style_changed([this](style_aspect aAspect)
        {
            if ((aAspect & (style_aspect::Geometry | style_aspect::Font)) != style_aspect::None)
                full_update();
        });
    }

    void header_view::full_update()
    {
        if (!has_presentation_model())
            return;
        layout().set_spacing(size{ separator_width() }, false);
        iSectionWidths.resize(presentation_model().columns());
        for (auto& sw : iSectionWidths)
            sw.calculated = 0.0;
        while (layout().count() > presentation_model().columns() + (expand_last_column() == last_column_expansion::ExpandToFitContent ? 0 : 1))
            layout().remove_at(layout().count() - 1);
        while (layout().count() < presentation_model().columns() + (expand_last_column() == last_column_expansion::ExpandToFitContent ? 0 : 1))
            layout().add(make_ref<header_button>(*this));
        if (iButtonSinks.size() < layout().count())
            iButtonSinks.resize(layout().count());
        auto const separatorWidth = separator_width();
        for (std::uint32_t i = 0u; i < layout().count(); ++i)
        {
            header_button& button = layout().get_widget_at<header_button>(i);
            button.layout().set_alignment(alignment::Left | alignment::VCenter);
            if (i == 0u)
            {
                auto m = button.padding();
                m.left = separatorWidth / 2.0 + 1.0;
                button.set_padding(m);
            }
            if (i < presentation_model().columns())
            {
                button.set_text(string{ presentation_model().column_heading_text(i) });
                button.set_minimum_size({});
                button.set_maximum_size({});
                if (expand_last_column() != last_column_expansion::ExpandToFitContent || i != presentation_model().columns() - 1)
                    button.set_size_policy(iType == header_view_type::Horizontal ?
                        neogfx::size_policy{ size_constraint::Fixed, size_constraint::Expanding } :
                        neogfx::size_policy{ size_constraint::Expanding, size_constraint::Fixed });
                else
                    button.set_size_policy(iType == header_view_type::Horizontal ?
                        neogfx::size_policy{ size_constraint::Maximum, size_constraint::Minimum } :
                        neogfx::size_policy{ size_constraint::Minimum, size_constraint::Maximum });
                button.enable(true);
                if (iButtonSinks[i][0].empty())
                    iButtonSinks[i][0] = button.Clicked([&, i]()
                    {
                        if (presentation_model().sortable())
                        {
                            root().window_manager().save_mouse_cursor();
                            root().window_manager().set_mouse_cursor(mouse_system_cursor::Wait, true);
                            presentation_model().sort_by(i);
                            root().window_manager().restore_mouse_cursor(root());
                        }
                    });
                if (iButtonSinks[i][1].empty())
                    iButtonSinks[i][1] = button.right_clicked([&, i]()
                    {
                        if (presentation_model().sortable())
                        {
                            context_menu menu{ *this, root().mouse_position() + root().window_position() };
                            action sortAscending{ "Sort Ascending"_t };
                            action sortDescending{ "Sort Descending"_t };
                            action applySort{ "Apply Sort"_t };
                            action resetSort{ "Reset Sort"_t };
                            menu.menu().add_action(sortAscending).set_checkable(true);
                            menu.menu().add_action(sortDescending).set_checkable(true);
                            menu.menu().add_action(applySort);
                            menu.menu().add_action(resetSort);
                            if (presentation_model().sorting_by() != std::nullopt)
                            {
                                auto const& sortingBy = *presentation_model().sorting_by();
                                if (sortingBy.first == i)
                                {
                                    if (sortingBy.second == i_item_presentation_model::sort_direction::Ascending)
                                        sortAscending.check();
                                    else if (sortingBy.second == i_item_presentation_model::sort_direction::Descending)
                                        sortDescending.check();
                                }
                            }
                            sortAscending.checked([this, &sortDescending, i]()
                            {
                                presentation_model().sort_by(i, i_item_presentation_model::sort_direction::Ascending);
                                sortDescending.uncheck();
                            });
                            sortDescending.checked([this, &sortAscending, i]()
                            {
                                presentation_model().sort_by(i, i_item_presentation_model::sort_direction::Descending);
                                sortAscending.uncheck();
                            });
                            resetSort.triggered([&]()
                            {
                                presentation_model().reset_sort();
                            });
                            menu.exec();
                        }
                    });
            }
            else if (expand_last_column() != last_column_expansion::ExpandToFitContent)
            {
                button.set_text(string{});
                button.set_size_policy(size_constraint::Expanding);
                button.set_minimum_size(size{});
                button.enable(false);
            }
        }
        for (std::uint32_t col = 0u; col < presentation_model().columns(); ++col)
            update_section_width(col, presentation_model().column_width(col, *this, true));
        iExpandedSectionWidth = std::nullopt;
        update_expanded_section();
        iOwner.header_view_updated(*this, header_view_update_reason::FullUpdate);
    }

    void header_view::set_section_width(std::uint32_t aSectionIndex, optional_dimension const& aWidth)
    {
        if (aSectionIndex >= iSectionWidths.size() || aSectionIndex >= layout().count())
            return;
        if (aWidth == std::nullopt)
        {
            reset_pane_sizes_requested(aSectionIndex);
            return;
        }
        // as if resized by the user (see splitter::mouse_moved): the section's button is resized (the first's includes half the cell 
        // spacing: see section_width) then the section's width is taken from it (see panes_resized)
        auto& button = layout().get_widget_at(aSectionIndex);
        dimension const buttonWidth = *aWidth + (aSectionIndex == 0u ? presentation_model().cell_spacing(*this).cx / 2.0 : 0.0);
        button.set_fixed_size(size{ std::max(buttonWidth, layout().spacing().cx * 3.0), button.extents().cy }, false);
        panes_resized();
    }

    bool header_view::update_section_width(std::uint32_t aColumn, dimension aColumnWidth)
    {
        neolib::scoped_flag sf{ iUpdatingSectionWidth };
        auto& calculatedSectionWidth = iSectionWidths[aColumn].calculated;
        dimension const oldSectionWidth = calculatedSectionWidth;
        dimension const headingWidth = presentation_model().column_heading_extents(aColumn, *this).cx + presentation_model().cell_padding(*this).size().cx * 2.0;
        calculatedSectionWidth = std::max(calculatedSectionWidth, units_converter{ *this }.to_device_units(std::max(headingWidth, aColumnWidth)));
        auto const lastColumn = presentation_model().columns() - 1u;
        if (aColumn == lastColumn && expand_last_column() == last_column_expansion::ExpandToFitView)
            return false; // (its contents don't change its width: see update_expanded_section)
        if (calculatedSectionWidth != oldSectionWidth || layout().get_widget_at(aColumn).minimum_size().cx != section_width(aColumn, true))
        {
            size const widgetSize{ std::max(section_width(aColumn, true), layout().spacing().cx * 3.0), layout().get_widget_at(aColumn).minimum_size().cy };
            if (expand_last_column() != last_column_expansion::ExpandToFitContent || aColumn != lastColumn)
                layout().get_widget_at(aColumn).set_fixed_size(widgetSize);
            else
                layout().get_widget_at(aColumn).set_minimum_size(widgetSize);
            return true;
        }
        return false;
    }

    // a last column expanded to fit the view: its button is as wide as it is (the rest of the view unless resized wider by the user); true 
    // if changed
    bool header_view::update_expanded_section()
    {
        if (expand_last_column() != last_column_expansion::ExpandToFitView || !has_presentation_model() || presentation_model().columns() == 0u ||
            layout().count() < presentation_model().columns())
            return false;
        auto const lastColumn = presentation_model().columns() - 1u;
        auto& button = layout().get_widget_at(lastColumn);
        dimension const width = std::max(section_width(lastColumn, true), layout().spacing().cx * 3.0);
        iExpandedSectionWidth = width;
        if (button.has_fixed_size() && button.fixed_size().cx == width)
            return false;
        button.set_fixed_size(size{ width, button.minimum_size().cy }, false);
        return true;
    }

    std::optional<header_view::separator_type> header_view::separator_at(const point& aPosition) const
    {
        auto const result = splitter::separator_at(aPosition);
        if (result || expand_last_column() != last_column_expansion::ExpandToFitView || !has_presentation_model() || 
            presentation_model().columns() == 0u || layout().count() <= presentation_model().columns())
            return result;
        // a last column expanded to fit the view ends at the right of the view so the separator after it (that resizes it) is beyond 
        // the view: it is grabbed at the right of the last column instead (or, if the column is wider than the visible part of the 
        // header, at the right of the view), twice as wide as a separator is at least grabbed (see splitter::separator_at) as it is 
        // at the very edge of the view
        auto const lastColumn = presentation_model().columns() - 1u;
        auto const& button = layout().get_widget_at(lastColumn);
        scalar const tolerance = 6.0_dip;
        dimension const grab = std::max(tolerance, separator_width()) * 2.0;
        dimension right = button.position().x + button.extents().cx;
        if (has_parent())
            right = std::min(right, parent().client_rect(false).right() - position().x);
        rect const grabRect{ point{ right - grab, button.position().y }, size{ grab, button.extents().cy } };
        if (grabRect.contains(aPosition))
            return separator_type{ lastColumn, lastColumn + 1u };
        return result;
    }

    void header_view::resized()
    {
        splitter::resized();
        // (the rest of the view, which a last column expanded to fit the view is as wide as, has changed)
        if (update_expanded_section())
        {
            layout_items();
            iOwner.header_view_updated(*this, header_view_update_reason::PanesResized);
        }
    }
}