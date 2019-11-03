// item_selection_model.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neolib/raii.hpp>

#include <neogfx/core/object.hpp>
#include <neogfx/gui/widget/i_item_presentation_model.hpp>
#include <neogfx/gui/widget/i_item_selection_model.hpp>

namespace neogfx
{
    class item_selection_model : public object<i_item_selection_model>
    {
    public:
        define_declared_event(CurrentIndexChanged, current_index_changed, const optional_item_presentation_model_index&, const optional_item_presentation_model_index&)
        define_declared_event(SelectionChanged, selection_changed, const item_selection&, const item_selection&)
        define_declared_event(PresentationModelAdded, presentation_model_added, i_item_presentation_model&)
        define_declared_event(PresentationModelChanged, presentation_model_changed, i_item_presentation_model&, i_item_presentation_model&)
        define_declared_event(PresentationModelRemoved, presentation_model_removed, i_item_presentation_model&)
        define_declared_event(ModeChanged, mode_changed, item_selection_mode)
    public:
        item_selection_model(item_selection_mode aMode = item_selection_mode::SingleSelection) :
            iModel{ nullptr },
            iMode{ aMode },
            iSorting{ false },
            iFiltering{ false }
        {
        }
        item_selection_model(i_item_presentation_model& aModel, item_selection_mode aMode = item_selection_mode::SingleSelection) :
            iModel{ nullptr },
            iMode{ aMode },
            iSorting{ false },
            iFiltering{ false }
        {
            set_presentation_model(aModel);
        }
        ~item_selection_model()
        {
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

            iSink += presentation_model().item_removed([this](const item_presentation_model_index&)
            {
                if (has_current_index())
                {
                    if (presentation_model().rows() <= 1)
                        iCurrentIndex = std::nullopt;
                    else if (iCurrentIndex->row() >= presentation_model().rows() - 1)
                        iCurrentIndex->set_row(iCurrentIndex->row() - 1);
                }
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
                if (iSavedModelIndex != std::nullopt && presentation_model().have_item_model_index(*iSavedModelIndex))
                    set_current_index(presentation_model().from_item_model_index(*iSavedModelIndex));
                else if (presentation_model().rows() >= 1)
                    set_current_index(item_presentation_model_index{ 0, 0 });
                iSavedModelIndex = std::nullopt;
            });
            iSink += presentation_model().destroying([this]()
            {
                auto oldModel = iModel;
                iModel = nullptr;
                iCurrentIndex = std::nullopt;
                iSavedModelIndex = std::nullopt;
                iSelection = item_selection{};
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
        }
    public:
        bool has_current_index() const override
        {
            return iCurrentIndex != std::nullopt;
        }
        const item_presentation_model_index& current_index() const override
        {
            if (iCurrentIndex == std::nullopt)
                throw no_current_index();
            return *iCurrentIndex;
        }
        void set_current_index(const item_presentation_model_index& aIndex) override
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
        item_presentation_model_index relative_to_index(const item_presentation_model_index& aIndex, index_location aRelativeLocation, bool aSelectable = true, bool aEditable = false) const override
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
                result = item_presentation_model_index{ 0, 0 };
                while (!acceptable(result) && (result = next_cell(result)) != item_presentation_model_index{ 0, 0 })
                    ;
                break;
            case index_location::LastCell:
                result = item_presentation_model_index{ presentation_model().rows() - 1 , presentation_model().columns() - 1 };
                while (!acceptable(result) && (result = previous_cell(result)) != item_presentation_model_index{ presentation_model().rows() - 1 , presentation_model().columns() - 1 })
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
                result.set_column(0);
                while (!acceptable(result) && (result += item_presentation_model_index{ 0, 1 }).column() != presentation_model().columns() - 1)
                    ;
                break;
            case index_location::EndOfCurrentRow:
                result.set_column(presentation_model().columns() - 1);
                while (!acceptable(result) && (result -= item_presentation_model_index{ 0, 1 }).column() != 0)
                    ;
                break;
            case index_location::StartOfCurrentColumn:
                result.set_row(0);
                while (!acceptable(result) && (result += item_presentation_model_index{ 1, 0 }).column() != presentation_model().rows() - 1)
                    ;
                break;
            case index_location::EndOfCurrentColumn:
                result.set_row(presentation_model().rows() - 1);
                while (!acceptable(result) && (result -= item_presentation_model_index{ 1, 0 }).row() != 0)
                    ;
                break;
            case index_location::CellToLeft:
                if (result.column() > 0)
                {
                    result -= item_presentation_model_index{ 0, 1 };
                    while (!acceptable(result) && (result -= item_presentation_model_index{ 0, 1 }).column() != 0)
                        ;
                }
                break;
            case index_location::CellToRight:
                if (result.column() < presentation_model().columns() - 1)
                {
                    result += item_presentation_model_index{ 0, 1 };
                    while (!acceptable(result) && (result += item_presentation_model_index{ 0, 1 }).column() != presentation_model().columns() - 1)
                        ;
                }
                break;
            case index_location::RowAbove:
                if (result.row() > 0)
                {
                    result -= item_presentation_model_index{ 1, 0 };
                    while (!acceptable(result) && (result -= item_presentation_model_index{ 1, 0 }).row() != 0)
                        ;
                }
                break;
            case index_location::RowBelow:
                if (result.row() < presentation_model().rows() - 1)
                {
                    result += item_presentation_model_index{ 1, 0 };
                    while (!acceptable(result) && (result += item_presentation_model_index{ 1, 0 }).row() != presentation_model().rows() - 1)
                        ;
                }
                break;
            }
            return result;
        }
        item_presentation_model_index next_cell() const override
        {
            if (!has_current_index())
                return item_presentation_model_index{ 0, 0 };
            else
                return next_cell(current_index());
        }
        item_presentation_model_index next_cell(const item_presentation_model_index& aIndex) const override
        {
            if (aIndex.column() + 1 < presentation_model().columns())
                return item_presentation_model_index{ aIndex.row(), aIndex.column() + 1 };
            else if (aIndex.row() + 1 < presentation_model().rows())
                return item_presentation_model_index{ aIndex.row() + 1, 0 };
            else
                return item_presentation_model_index{ 0, 0 };
        }
        item_presentation_model_index previous_cell() const override
        {
            if (!has_current_index())
                return item_presentation_model_index{ 0, 0 };
            else
                return previous_cell(current_index());
        }
        item_presentation_model_index previous_cell(const item_presentation_model_index& aIndex) const override
        {
            if (aIndex.column() > 0)
                return item_presentation_model_index{ aIndex.row(), aIndex.column() - 1 };
            else if (aIndex.row() > 0)
                return item_presentation_model_index{ aIndex.row() - 1, presentation_model().columns() - 1 };
            else
                return item_presentation_model_index{ presentation_model().rows() - 1 , presentation_model().columns() - 1 };
        }
    public:
        const item_selection& selection() const override
        {
            return iSelection;
        }
        bool is_selected(const item_presentation_model_index& aIndex) const override
        {
            return (presentation_model().cell_meta(aIndex).selection & item_cell_selection_flags::Selected) == item_cell_selection_flags::Selected;
        }    
        bool is_selectable(const item_presentation_model_index& aIndex) const override
        {
            return !(has_presentation_model() && presentation_model().item_model().cell_data_info(presentation_model().to_item_model_index(aIndex)).unselectable);
        }
        void select(const item_presentation_model_index& aIndex, item_selection_operation aOperation = item_selection_operation::ClearAndSelect) override
        {
            /* todo */
            (void)aIndex;
            (void)aOperation;
        }
        void select(const item_selection::range& aRange, item_selection_operation aOperation = item_selection_operation::ClearAndSelect) override
        {
            /* todo */
            (void)aRange;
            (void)aOperation;
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
        bool is_editable(const item_presentation_model_index& aIndex) const override
        {
            return is_selectable(aIndex) && !(has_presentation_model() && presentation_model().cell_editable(aIndex) == item_cell_editable::No);
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
    private:
        i_item_presentation_model* iModel;
        item_selection_mode iMode;
        optional_item_presentation_model_index iCurrentIndex;
        optional_item_model_index iSavedModelIndex;
        item_selection iSelection;
        bool iSorting;
        bool iFiltering;
        sink iSink;
    };
}
