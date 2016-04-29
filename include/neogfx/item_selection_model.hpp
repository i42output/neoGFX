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

#include "neogfx.hpp"
#include <neolib/observable.hpp>
#include "i_item_model.hpp"
#include "i_item_selection_model.hpp"

namespace neogfx
{
	class item_selection_model : public i_item_selection_model, private neolib::observable<i_item_selection_model_subscriber>, private i_item_model_subscriber
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
		item_selection_model(i_item_model& aModel, item_selection_mode aMode = item_selection_mode::SingleSelection) :
			iModel(0),
			iMode(aMode)
		{
			set_item_model(aModel);
		}
		~item_selection_model()
		{
			if (has_item_model())
				item_model().unsubscribe(*this);
			notify_observers(i_item_selection_model_subscriber::NotifySelectionModelDestroyed);
		}
	public:
		virtual bool has_item_model() const
		{
			return iModel != 0;
		}
		virtual i_item_model& item_model() const
		{
			if (iModel == 0)
				throw no_item_model();
			return *iModel;
		}
		virtual void set_item_model(i_item_model& aItemModel)
		{
			if (iModel == &aItemModel)
				return;
			if (has_item_model())
				item_model().unsubscribe(*this);
			unset_current_index();
			i_item_model* oldModel = iModel;
			iModel = &aItemModel;
			item_model().subscribe(*this);
			if (oldModel == 0)
				notify_observers(i_item_selection_model_subscriber::NotifyItemModelAdded, item_model());
			else
				notify_observers(i_item_selection_model_subscriber::NotifyItemModelChanged, item_model(), *oldModel);
		}
	public:
		virtual item_selection_mode mode() const
		{
			return iMode;
		}
		virtual void set_mode(item_selection_mode aMode)
		{
			if (iMode == aMode)
				return;
			iMode = aMode;
			notify_observers(i_item_selection_model_subscriber::NotifySelectionModeChanged, mode());
		}
	public:
		virtual bool has_current_index() const
		{
			return iCurrentIndex != boost::none;
		}
		virtual const item_model_index& current_index() const
		{
			if (iCurrentIndex == boost::none)
				throw no_current_index();
			return *iCurrentIndex;
		}
		virtual void set_current_index(const item_model_index& aIndex)
		{
			if (iCurrentIndex == aIndex)
				return;
			optional_item_model_index previousIndex = iCurrentIndex;
			iCurrentIndex = aIndex;
			notify_observers(i_item_selection_model_subscriber::NotifyCurrentIndexChanged, iCurrentIndex, previousIndex);
			current_index_changed.trigger(iCurrentIndex, previousIndex);
		}
		virtual void unset_current_index()
		{
			if (iCurrentIndex == boost::none)
				return;
			optional_item_model_index previousIndex = iCurrentIndex;
			iCurrentIndex = boost::none;
			notify_observers(i_item_selection_model_subscriber::NotifyCurrentIndexChanged, iCurrentIndex, previousIndex);
			current_index_changed.trigger(iCurrentIndex, previousIndex);
		}
	public:
		virtual const item_selection& selection() const
		{
			return iSelection;
		}
		virtual bool is_selected(const item_model_index& aIndex) const
		{
			return (item_model().cell_meta(aIndex).selection & i_item_presentation_model::cell_meta_type::selection_flags::Selected) == i_item_presentation_model::cell_meta_type::selection_flags::Selected;
		}	
		virtual void select(const item_model_index& aIndex, item_selection_operation aOperation = item_selection_operation::ClearAndSelect)
		{
			/* todo */
			(void)aIndex;
			(void)aOperation;
		}
		virtual void select(const item_selection::range& aRange, item_selection_operation aOperation = item_selection_operation::ClearAndSelect)
		{
			/* todo */
			(void)aRange;
			(void)aOperation;
		}
	public:
		virtual void subscribe(i_item_selection_model_subscriber& aSubscriber)
		{
			add_observer(aSubscriber);
		}
		virtual void unsubscribe(i_item_selection_model_subscriber& aSubscriber)
		{
			remove_observer(aSubscriber);
		}
	private:
		virtual void notify_observer(i_item_selection_model_subscriber& aObserver, i_item_selection_model_subscriber::notify_type aType, const void* aParameter, const void* aParameter2)
		{
			switch (aType)
			{
			case i_item_selection_model_subscriber::NotifyItemModelAdded:
				aObserver.item_model_added(*this, const_cast<i_item_model&>(*static_cast<const i_item_model*>(aParameter)));
				break;
			case i_item_selection_model_subscriber::NotifyItemModelChanged:
				aObserver.item_model_changed(*this, const_cast<i_item_model&>(*static_cast<const i_item_model*>(aParameter)), const_cast<i_item_model&>(*static_cast<const i_item_model*>(aParameter2)));
				break;
			case i_item_selection_model_subscriber::NotifyItemModelRemoved:
				aObserver.item_model_removed(*this, const_cast<i_item_model&>(*static_cast<const i_item_model*>(aParameter)));
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
		virtual void column_info_changed(const i_item_model&, item_model_index::value_type)
		{
		}
		virtual void item_added(const i_item_model&, const item_model_index&)
		{
		}
		virtual void item_changed(const i_item_model&, const item_model_index&)
		{
		}
		virtual void item_removed(const i_item_model&, const item_model_index&)
		{
		}
		virtual void items_sorted(const i_item_model&)
		{
		}
		virtual void model_destroyed(const i_item_model& aModel)
		{
			unset_current_index();
			iModel = 0;
			notify_observers(i_item_selection_model_subscriber::NotifyItemModelRemoved, aModel);
		}
	private:
		i_item_model* iModel;
		item_selection_mode iMode;
		optional_item_model_index iCurrentIndex;
		item_selection iSelection;
	};
}