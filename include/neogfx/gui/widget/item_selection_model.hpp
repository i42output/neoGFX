// item_selection_model.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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
#include <neolib/observable.hpp>
#include <neogfx/app/app.hpp>
#include "i_item_presentation_model.hpp"
#include "i_item_selection_model.hpp"

namespace neogfx
{
	class item_selection_model : public i_item_selection_model, private neolib::observable<i_item_selection_model_subscriber>, private i_item_presentation_model_subscriber
	{
	public:
		current_index_changed_event current_index_changed;
		selection_changed_event selection_changed;
	public:
		item_selection_model(item_selection_mode aMode = item_selection_mode::SingleSelection) :
			iModel(0),
			iMode(aMode)
		{
		}
		item_selection_model(i_item_presentation_model& aModel, item_selection_mode aMode = item_selection_mode::SingleSelection) :
			iModel(0),
			iMode(aMode)
		{
			set_presentation_model(aModel);
		}
		~item_selection_model()
		{
			if (has_presentation_model())
				presentation_model().unsubscribe(*this);
			notify_observers(i_item_selection_model_subscriber::NotifySelectionModelDestroyed);
		}
	public:
		bool has_presentation_model() const override
		{
			return iModel != 0;
		}
		i_item_presentation_model& presentation_model() const override
		{
			if (iModel == 0)
				throw no_presentation_model();
			return *iModel;
		}
		void set_presentation_model(i_item_presentation_model& aModel) override
		{
			if (iModel == &aModel)
				return;
			if (has_presentation_model())
				presentation_model().unsubscribe(*this);
			unset_current_index();
			i_item_presentation_model* oldModel = iModel;
			iModel = &aModel;
			presentation_model().subscribe(*this);
			if (oldModel == 0)
				notify_observers(i_item_selection_model_subscriber::NotifyModelAdded, presentation_model());
			else
				notify_observers(i_item_selection_model_subscriber::NotifyModelChanged, presentation_model(), *oldModel);
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
			notify_observers(i_item_selection_model_subscriber::NotifySelectionModeChanged, mode());
		}
	public:
		bool has_current_index() const override
		{
			return iCurrentIndex != boost::none;
		}
		const item_presentation_model_index& current_index() const override
		{
			if (iCurrentIndex == boost::none)
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
		bool is_editable(const item_presentation_model_index& aIndex) const override
		{
			return is_selectable(aIndex) && !(has_presentation_model() && presentation_model().cell_editable(aIndex) == item_cell_editable::No);
		}
	public:
		void subscribe(i_item_selection_model_subscriber& aSubscriber) override
		{
			add_observer(aSubscriber);
		}
		void unsubscribe(i_item_selection_model_subscriber& aSubscriber) override
		{
			remove_observer(aSubscriber);
		}
	private:
		void do_set_current_index(const optional_item_presentation_model_index& aNewIndex)
		{
			if (iCurrentIndex != aNewIndex)
			{
				if (aNewIndex != boost::none && !is_selectable(*aNewIndex))
					return;
				optional_item_presentation_model_index previousIndex = iCurrentIndex;
				iCurrentIndex = aNewIndex;
				notify_observers(i_item_selection_model_subscriber::NotifyCurrentIndexChanged, iCurrentIndex, previousIndex);
				current_index_changed.trigger(iCurrentIndex, previousIndex);
			}
		}
	private:
		void notify_observer(i_item_selection_model_subscriber& aObserver, i_item_selection_model_subscriber::notify_type aType, const void* aParameter, const void* aParameter2) override
		{
			switch (aType)
			{
			case i_item_selection_model_subscriber::NotifyModelAdded:
				aObserver.model_added(*this, const_cast<i_item_presentation_model&>(*static_cast<const i_item_presentation_model*>(aParameter)));
				break;
			case i_item_selection_model_subscriber::NotifyModelChanged:
				aObserver.model_changed(*this, const_cast<i_item_presentation_model&>(*static_cast<const i_item_presentation_model*>(aParameter)), const_cast<i_item_presentation_model&>(*static_cast<const i_item_presentation_model*>(aParameter2)));
				break;
			case i_item_selection_model_subscriber::NotifyModelRemoved:
				aObserver.model_removed(*this, const_cast<i_item_presentation_model&>(*static_cast<const i_item_presentation_model*>(aParameter)));
				break;
			case i_item_selection_model_subscriber::NotifySelectionModeChanged:
				aObserver.selection_mode_changed(*this, *static_cast<const item_selection_mode*>(aParameter));
				break;
			case i_item_selection_model_subscriber::NotifyCurrentIndexChanged:
				aObserver.current_index_changed(*this, *static_cast<const optional_item_presentation_model_index*>(aParameter), *static_cast<const optional_item_presentation_model_index*>(aParameter2));
				break;
			case i_item_selection_model_subscriber::NotifySelectionChanged:
				aObserver.selection_changed(*this, *static_cast<const item_selection*>(aParameter), *static_cast<const item_selection*>(aParameter2));
				break;
			case i_item_selection_model_subscriber::NotifySelectionModelDestroyed:
				aObserver.selection_model_destroyed(*this);
				break;
			}
		}
	private:
		void column_info_changed(const i_item_presentation_model&, item_presentation_model_index::value_type) override
		{
		}
		void item_model_changed(const i_item_presentation_model&, const i_item_model&) override
		{
		}
		void item_added(const i_item_presentation_model&, const item_presentation_model_index&) override
		{
		}
		void item_changed(const i_item_presentation_model&, const item_presentation_model_index&) override
		{
		}
		void item_removed(const i_item_presentation_model& aModel, const item_presentation_model_index&) override
		{
			if (has_current_index())
			{
				if (aModel.rows() <= 1)
					iCurrentIndex = boost::none;
				else if (iCurrentIndex->row() >= aModel.rows() - 1)
					iCurrentIndex->set_row(iCurrentIndex->row() - 1);
			}
		}
		void items_sorting(const i_item_presentation_model&) override
		{
		}
		void items_sorted(const i_item_presentation_model&) override
		{
		}
		void items_filtering(const i_item_presentation_model&) override
		{
		}
		void items_filtered(const i_item_presentation_model&) override
		{
		}
		void model_destroyed(const i_item_presentation_model& aModel) override
		{
			unset_current_index();
			iModel = 0;
			notify_observers(i_item_selection_model_subscriber::NotifyModelRemoved, aModel);
		}
	private:
		i_item_presentation_model* iModel;
		item_selection_mode iMode;
		optional_item_presentation_model_index iCurrentIndex;
		item_selection iSelection;
	};
}