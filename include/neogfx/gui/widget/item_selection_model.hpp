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
		const item_model_index& current_index() const override
		{
			if (iCurrentIndex == boost::none)
				throw no_current_index();
			return *iCurrentIndex;
		}
		void set_current_index(const item_model_index& aIndex) override
		{
			if (iCurrentIndex == aIndex)
				return;
			optional_item_model_index previousIndex = iCurrentIndex;
			iCurrentIndex = aIndex;
			notify_observers(i_item_selection_model_subscriber::NotifyCurrentIndexChanged, iCurrentIndex, previousIndex);
			current_index_changed.trigger(iCurrentIndex, previousIndex);
		}
		void unset_current_index() override
		{
			if (iCurrentIndex == boost::none)
				return;
			optional_item_model_index previousIndex = iCurrentIndex;
			iCurrentIndex = boost::none;
			notify_observers(i_item_selection_model_subscriber::NotifyCurrentIndexChanged, iCurrentIndex, previousIndex);
			current_index_changed.trigger(iCurrentIndex, previousIndex);
		}
	public:
		const item_selection& selection() const override
		{
			return iSelection;
		}
		bool is_selected(const item_model_index& aIndex) const override
		{
			return (presentation_model().cell_meta(aIndex).selection & item_selection_flags::Selected) == item_selection_flags::Selected;
		}	
		void select(const item_model_index& aIndex, item_selection_operation aOperation = item_selection_operation::ClearAndSelect) override
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
		void subscribe(i_item_selection_model_subscriber& aSubscriber) override
		{
			add_observer(aSubscriber);
		}
		void unsubscribe(i_item_selection_model_subscriber& aSubscriber) override
		{
			remove_observer(aSubscriber);
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
				aObserver.current_index_changed(*this, *static_cast<const optional_item_model_index*>(aParameter), *static_cast<const optional_item_model_index*>(aParameter2));
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
		void items_sorted(const i_item_presentation_model&) override
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
		optional_item_model_index iCurrentIndex;
		item_selection iSelection;
	};
}