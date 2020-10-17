// basic_item_selection_model.hpp
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
#include <neolib/core/scoped.hpp>
#include <neolib/core/map.hpp>
#include <neogfx/core/object.hpp>
#include <neogfx/gui/widget/i_item_presentation_model.hpp>
#include <neogfx/gui/widget/i_item_selection_model.hpp>

namespace neogfx
{
    template <typename Alloc = std::allocator<std::pair<const item_presentation_model_index, selection_area>>>
    class basic_item_selection_model : public object<i_item_selection_model>
    {
    public:
        define_declared_event(CurrentIndexChanged, current_index_changed, const optional_item_presentation_model_index& /* aCurrentIndex */, const optional_item_presentation_model_index& /* aPreviousIndex */)
        define_declared_event(SelectionChanged, selection_changed, const item_selection&, const item_selection&)
        define_declared_event(PresentationModelAdded, presentation_model_added, i_item_presentation_model&)
        define_declared_event(PresentationModelChanged, presentation_model_changed, i_item_presentation_model&, i_item_presentation_model&)
        define_declared_event(PresentationModelRemoved, presentation_model_removed, i_item_presentation_model&)
        define_declared_event(ModeChanged, mode_changed, item_selection_mode)
    public:
        typedef Alloc allocator_type;
    private:
        using concrete_item_selection = neolib::map<item_presentation_model_index, selection_area, std::less<item_presentation_model_index>, allocator_type>;
    public:
        basic_item_selection_model(item_selection_mode aMode = item_selection_mode::SingleSelection) :
            iModel{ nullptr },
            iMode{ aMode },
            iSorting{ false },
            iFiltering{ false }
        {
            set_alive();
        }
        basic_item_selection_model(i_item_presentation_model& aModel, item_selection_mode aMode = item_selection_mode::SingleSelection) :
            iModel{ nullptr },
            iMode{ aMode },
            iSorting{ false },
            iFiltering{ false }
        {
            set_presentation_model(aModel);
            set_alive();
        }
        ~basic_item_selection_model()
        {
            iSink.clear();
            set_destroying();
        }
    public:
        bool has_presentation_model() const override
        {
            return iModel != nullptr;
        }
        i_item_presentation_model& presentation_model() const override
        {
            if (iModel == nullptr)
                throw no_presentation_model();
            return *iModel;
        }
        void set_presentation_model(i_item_presentation_model& aModel) override
        {
            if (iModel == &aModel)
                return;

            iSink.clear();
            
            unset_current_index();
            i_item_presentation_model* oldModel = iModel;

            iModel = &aModel;

            iSink += presentation_model().item_model_changed([this](const i_item_model&)
            {
                iCurrentIndex = std::nullopt;
                reindex();
            });
            iSink += presentation_model().item_removed([this](item_presentation_model_index const&)
            {
                if (has_current_index())
                {
                    if (presentation_model().rows() <= 1u)
                        iCurrentIndex = std::nullopt;
                    else if (iCurrentIndex->row() >= presentation_model().rows() - 1u)
                        iCurrentIndex->set_row(iCurrentIndex->row() - 1u);
                }
                reindex();
            });
            iSink += presentation_model().item_expanded([this](item_presentation_model_index const& aIndex)
            {
                if (has_current_index() && current_index().row() > aIndex.row())
                    iCurrentIndex = std::nullopt;
                reindex();
            });
            iSink += presentation_model().item_collapsed([this](item_presentation_model_index const& aIndex)
            {
                if (has_current_index() && current_index().row() > aIndex.row())
                    iCurrentIndex = std::nullopt;
                reindex();
            });
            iSink += presentation_model().items_sorting([this]()
            {
                neolib::scoped_flag sf{ iSorting };
                iSavedModelIndex = has_current_index() ? presentation_model().to_item_model_index(current_index()) : optional_item_model_index{};
                unset_current_index();
            });
            iSink += presentation_model().items_sorted([this]()
            {
                neolib::scoped_flag sf{ iSorting };
                if (iSavedModelIndex != std::nullopt)
                    set_current_index(presentation_model().from_item_model_index(*iSavedModelIndex));
                iSavedModelIndex = std::nullopt;
                reindex();
            });
            iSink += presentation_model().items_filtering([this]()
            {
                neolib::scoped_flag sf{ iFiltering };
                iSavedModelIndex = has_current_index() ? presentation_model().to_item_model_index(current_index()) : optional_item_model_index{};
                unset_current_index();
            });
            iSink += presentation_model().items_filtered([this]()
            {
                neolib::scoped_flag sf{ iFiltering };
                if (iSavedModelIndex != std::nullopt && presentation_model().has_item_model_index(*iSavedModelIndex))
                    set_current_index(presentation_model().from_item_model_index(*iSavedModelIndex));
                else if (presentation_model().rows() >= 1)
                    set_current_index(item_presentation_model_index{ 0u, 0u });
                iSavedModelIndex = std::nullopt;
                reindex();
            });
            iSink += neolib::destroying(presentation_model(), [this]()
            {
                auto oldModel = iModel;
                iModel = nullptr;
                iCurrentIndex = std::nullopt;
                iSavedModelIndex = std::nullopt;
                iSelection = {};
                PresentationModelRemoved.trigger(*oldModel);
            });

            if (oldModel == nullptr)
                PresentationModelAdded.trigger(presentation_model());
            else
                PresentationModelChanged.trigger(presentation_model(), *oldModel);
        }
    public:
        item_selection_mode mode() const override
        {
            return iMode;
        }
        void set_mode(item_selection_mode aMode) override
        {
            if (iMode == aMode)
                return;
            iMode = aMode;
            ModeChanged.trigger(mode());
            clear({});
        }
    public:
        bool has_current_index() const override
        {
            return iCurrentIndex != std::nullopt;
        }
        item_presentation_model_index const& current_index() const override
        {
            if (iCurrentIndex == std::nullopt)
                throw no_current_index();
            return *iCurrentIndex;
        }
        void set_current_index(item_presentation_model_index const& aIndex) override
        {
            do_set_current_index(aIndex);
        }
        void unset_current_index() override
        {
            do_set_current_index(optional_item_presentation_model_index{});
        }
        item_presentation_model_index relative_to_current_index(index_location aRelativeLocation, bool aSelectable = true, bool aEditable = false) const override
        {
            return relative_to_index(has_current_index() ? current_index() : item_presentation_model_index{ 0, 0 }, aRelativeLocation, aSelectable, aEditable);
        }
        item_presentation_model_index relative_to_index(item_presentation_model_index const& aIndex, index_location aRelativeLocation, bool aSelectable = true, bool aEditable = false) const override
        {
            item_presentation_model_index result = aIndex;
            auto acceptable = [this, aSelectable, aEditable](item_presentation_model_index aIndex)
            {
                return (!aSelectable || is_selectable(aIndex)) && (!aEditable || is_editable(aIndex));
            };
            switch (aRelativeLocation)
            {
            case index_location::None:
            case index_location::FirstCell:
                result = item_presentation_model_index{ 0u, 0u };
                while (!acceptable(result) && (result = next_cell(result)) != item_presentation_model_index{ 0u, 0u })
                    ;
                break;
            case index_location::LastCell:
                result = item_presentation_model_index{ presentation_model().rows() - 1u , presentation_model().columns() - 1u };
                while (!acceptable(result) && (result = previous_cell(result)) != item_presentation_model_index{ presentation_model().rows() - 1u , presentation_model().columns() - 1u })
                    ;
                break;
            case index_location::PreviousCell:
                result = previous_cell();
                while (!acceptable(result) && (result = previous_cell(result)) != previous_cell())
                    ;
                break;
            case index_location::NextCell:
                result = next_cell();
                while (!acceptable(result) && (result = next_cell(result)) != next_cell())
                    ;
                break;
            case index_location::StartOfCurrentRow:
                result.set_column(0u);
                while (!acceptable(result) && (result += item_presentation_model_index{ 0u, 1u }).column() != presentation_model().columns() - 1u)
                    ;
                break;
            case index_location::EndOfCurrentRow:
                result.set_column(presentation_model().columns() - 1u);
                while (!acceptable(result) && (result -= item_presentation_model_index{ 0u, 1u }).column() != 0u)
                    ;
                break;
            case index_location::StartOfCurrentColumn:
                result.set_row(0);
                while (!acceptable(result) && (result += item_presentation_model_index{ 1u, 0u }).column() != presentation_model().rows() - 1u)
                    ;
                break;
            case index_location::EndOfCurrentColumn:
                result.set_row(presentation_model().rows() - 1u);
                while (!acceptable(result) && (result -= item_presentation_model_index{ 1, 0 }).row() != 0u)
                    ;
                break;
            case index_location::CellToLeft:
                if (result.column() > 0u)
                {
                    result -= item_presentation_model_index{ 0u, 1u };
                    while (!acceptable(result) && (result -= item_presentation_model_index{ 0u, 1u }).column() != 0u)
                        ;
                }
                break;
            case index_location::CellToRight:
                if (result.column() < presentation_model().columns() - 1u)
                {
                    result += item_presentation_model_index{ 0u, 1u };
                    while (!acceptable(result) && (result += item_presentation_model_index{ 0u, 1u }).column() != presentation_model().columns() - 1u)
                        ;
                }
                break;
            case index_location::RowAbove:
                if (result.row() > 0u)
                {
                    result -= item_presentation_model_index{ 1u, 0u };
                    while (!acceptable(result) && (result -= item_presentation_model_index{ 1u, 0u }).row() != 0u)
                        ;
                }
                break;
            case index_location::RowBelow:
                if (result.row() < presentation_model().rows() - 1u)
                {
                    result += item_presentation_model_index{ 1u, 0u };
                    while (!acceptable(result) && (result += item_presentation_model_index{ 1u, 0u }).row() != presentation_model().rows() - 1u)
                        ;
                }
                break;
            }
            return result;
        }
        item_presentation_model_index next_cell() const override
        {
            if (!has_current_index())
                return item_presentation_model_index{ 0u, 0u };
            else
                return next_cell(current_index());
        }
        item_presentation_model_index next_cell(item_presentation_model_index const& aIndex) const override
        {
            if (aIndex.column() + 1u < presentation_model().columns())
                return item_presentation_model_index{ aIndex.row(), aIndex.column() + 1u };
            else if (aIndex.row() + 1u < presentation_model().rows())
                return item_presentation_model_index{ aIndex.row() + 1u, 0u };
            else
                return item_presentation_model_index{ 0u, 0u };
        }
        item_presentation_model_index previous_cell() const override
        {
            if (!has_current_index())
                return item_presentation_model_index{ 0u, 0u };
            else
                return previous_cell(current_index());
        }
        item_presentation_model_index previous_cell(item_presentation_model_index const& aIndex) const override
        {
            if (aIndex.column() > 0u)
                return item_presentation_model_index{ aIndex.row(), aIndex.column() - 1u };
            else if (aIndex.row() > 0u)
                return item_presentation_model_index{ aIndex.row() - 1u, presentation_model().columns() - 1u };
            else
                return item_presentation_model_index{ presentation_model().rows() - 1u , presentation_model().columns() - 1u };
        }
    public:
        const item_selection& selection() const override
        {
            return iSelection;
        }
        bool is_selected(item_presentation_model_index const& aIndex) const override
        {
            return (presentation_model().cell_meta(aIndex).selection & item_cell_selection_flags::Selected) == item_cell_selection_flags::Selected;
        }    
        bool is_selectable(item_presentation_model_index const& aIndex) const override
        {
            return (presentation_model().cell_flags(aIndex) & item_cell_flags::Selectable) == item_cell_flags::Selectable;
        }
        void select(item_presentation_model_index const& aIndex, item_selection_operation aOperation) override
        {
            if (aOperation == item_selection_operation::None)
                return;
            else if (mode() == item_selection_mode::NoSelection)
                aOperation = item_selection_operation::Clear;
            // todo: cell and column
            static auto find = [](item_presentation_model_index const& aIndex, concrete_item_selection& aSelection)
            {
                if (aSelection.empty())
                    return aSelection.end();
                auto existing = aSelection.lower_bound(aIndex.with_column(0u));
                if (existing == aSelection.end())
                    existing = std::prev(existing);
                if (existing->second().topLeft.row() <= aIndex.row() &&
                    existing->second().bottomRight.row() >= aIndex.row())
                    return existing;
                return aSelection.end();
            };
            static auto find_adjacent = [](item_presentation_model_index const& aIndex, concrete_item_selection& aSelection)
            {
                if (aSelection.empty())
                    return aSelection.end();
                auto existing = find(aIndex.with_row(aIndex.row() - 1u), aSelection);
                if (existing != aSelection.end())
                    return existing;
                return find(aIndex.with_row(aIndex.row() + 1u), aSelection);
            };
            auto update = [&, this](concrete_item_selection& aSelection, bool aUpdateCells)
            {
                bool const clear = (aOperation & item_selection_operation::Clear) == item_selection_operation::Clear;
                bool const rowCurrentlySelected = (presentation_model().cell_meta(aIndex.with_column(0u)).selection & item_cell_selection_flags::Selected) ==
                    item_cell_selection_flags::Selected;
                bool const select = (aOperation & item_selection_operation::Select) == item_selection_operation::Select ||
                    ((aOperation & item_selection_operation::Toggle) == item_selection_operation::Toggle && !rowCurrentlySelected);
                bool const deselect = (aOperation & item_selection_operation::Deselect) == item_selection_operation::Deselect ||
                    ((aOperation & item_selection_operation::Toggle) == item_selection_operation::Toggle && rowCurrentlySelected);
                if (clear)
                {
                    if (aUpdateCells)
                    {
                        struct selection_clearer : i_item_presentation_model::i_meta_visitor
                        {
                            void visit(i_item_presentation_model::cell_meta_type& aMeta) override
                            {
                                aMeta.selection &= ~item_cell_selection_flags::Selected;
                            }
                        } selectionClearer;
                        presentation_model().accept(selectionClearer, true);
                    }
                    aSelection.clear();
                }
                if (select)
                {
                    if (find(aIndex, aSelection) == aSelection.end())
                    {
                        auto adjacent = find_adjacent(aIndex, aSelection);
                        if (adjacent == aSelection.end())
                            aSelection.emplace(aIndex.with_column(0u), selection_area{ aIndex.with_column(0u), aIndex.with_column(presentation_model().columns() - 1u) });
                        else
                        {
                            if (adjacent->second().bottomRight.row() == aIndex.row() - 1u)
                                adjacent->second().bottomRight = aIndex.with_column(presentation_model().columns() - 1u);
                            else
                            {
                                aSelection.emplace(aIndex.with_column(0u), selection_area{ aIndex.with_column(0u), adjacent->second().bottomRight });
                                aSelection.erase(adjacent);
                            }
                        }
                    }
                    if (aUpdateCells)
                        for (auto& cellIndex : *this)
                            if (cellIndex.row() == aIndex.row())
                                presentation_model().cell_meta(cellIndex).selection |= item_cell_selection_flags::Selected;
                }
                else if (deselect)
                {
                    if (aUpdateCells)
                        for (auto& cellIndex : *this)
                            if (cellIndex.row() == aIndex.row())
                                presentation_model().cell_meta(cellIndex).selection &= ~item_cell_selection_flags::Selected;
                    auto existing = find(aIndex, aSelection);
                    if (existing != aSelection.end())
                    {
                        if (existing->second().topLeft.row() == existing->second().bottomRight.row())
                            aSelection.erase(existing);
                        else if (existing->second().topLeft.row() == aIndex.row())
                        {
                            aSelection.emplace(aIndex.with_row(aIndex.row() + 1u).with_column(0u), selection_area{ aIndex.with_row(aIndex.row() + 1u).with_column(0u), existing->second().bottomRight });
                            aSelection.erase(existing);
                        }
                        else if (existing->second().bottomRight.row() == aIndex.row())
                            existing->second().bottomRight.set_row(aIndex.row() - 1u);
                        else
                        {
                            aSelection.emplace(aIndex.with_row(aIndex.row() + 1u).with_column(0u), selection_area{ aIndex.with_row(aIndex.row() + 1u).with_column(0u), existing->second().bottomRight });
                            existing->second().bottomRight.set_row(aIndex.row() - 1u);
                        }
                    }
                }
            };
            update(iSelection, true);
            if ((aOperation & item_selection_operation::Internal) != item_selection_operation::Internal)
                SelectionChanged.trigger(iSelection, iPreviousSelection);
            update(iPreviousSelection, false);
        }
    public:
        bool sorting() const override
        {
            return iSorting;
        }
        bool filtering() const override
        {
            return iFiltering;
        }
    public:
        bool is_editable(item_presentation_model_index const& aIndex) const override
        {
            return is_selectable(aIndex) && has_presentation_model() && presentation_model().cell_editable(aIndex);
        }
    private:
        void do_set_current_index(const optional_item_presentation_model_index& aNewIndex)
        {
            if (iCurrentIndex != aNewIndex)
            {
                if (aNewIndex != std::nullopt && !is_selectable(*aNewIndex))
                    return;
                optional_item_presentation_model_index previousIndex = iCurrentIndex;
                iCurrentIndex = aNewIndex;
                if (previousIndex != std::nullopt)
                    presentation_model().cell_meta(*previousIndex).selection = 
                        presentation_model().cell_meta(*previousIndex).selection & ~item_cell_selection_flags::Current;
                if (iCurrentIndex != std::nullopt)
                    presentation_model().cell_meta(*iCurrentIndex).selection = 
                        presentation_model().cell_meta(*iCurrentIndex).selection | item_cell_selection_flags::Current;
                CurrentIndexChanged.trigger(iCurrentIndex, previousIndex);
            }
        }
        void reindex()
        {
            iPreviousSelection.clear();
            iSelection.clear();
            for (item_presentation_model_index::row_type row = 0; row < presentation_model().rows(); ++row)
                for (item_presentation_model_index::column_type column = 0; column < presentation_model().columns(); ++column)
                    if ((presentation_model().cell_meta(item_presentation_model_index{ row, column }).selection & item_cell_selection_flags::Selected) == item_cell_selection_flags::Selected)
                        select(item_presentation_model_index{ row, column }, item_selection_operation::Select | item_selection_operation::Internal);
        }
    private:
        i_item_presentation_model* iModel;
        item_selection_mode iMode;
        optional_item_presentation_model_index iCurrentIndex;
        optional_item_model_index iSavedModelIndex;
        concrete_item_selection iPreviousSelection;
        concrete_item_selection iSelection;
        bool iSorting;
        bool iFiltering;
        sink iSink;
    };

    using item_selection_model = basic_item_selection_model<>;
}
