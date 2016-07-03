// i_item_selection_model.hpp
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
#include <neogfx/core/event.hpp>
#include "item_model_index.hpp"
#include "item_selection.hpp"

namespace neogfx
{
	class i_item_model;

	enum class item_selection_mode
	{
		NoSelection,
		SingleSelection,
		MultipleSelection,
		ExtendedSelection
	};

	enum class item_selection_operation
	{
		None					= 0x00,
		Select					= 0x01,
		Deselect				= 0x02,
		Toggle					= 0x04,
		Clear					= 0x08,
		Row						= 0x10,
		Column					= 0x20,
		SelectRow				= Select | Row,
		SelectColumn			= Select | Column,
		DeselectRow				= Deselect | Row,
		DeselectColumn			= Deselect | Column,
		ToggleRow				= Toggle | Row,
		ToggleColumn			= Toggle | Column,
		ClearAndSelect			= Clear | Select,
		ClearAndSelectRow		= Clear | Select | Row,
		ClearAndSelectColumn	= Clear | Select | Column
	};

	class i_item_selection_model;

	class i_item_selection_model_subscriber
	{
	public:
		virtual void item_model_added(const i_item_selection_model& aSelectionModel, i_item_model& aNewItemModel) = 0;
		virtual void item_model_changed(const i_item_selection_model& aSelectionModel, i_item_model& aNewItemModel, i_item_model& aOldItemModel) = 0;
		virtual void item_model_removed(const i_item_selection_model& aSelectionModel, i_item_model& aOldItemModel) = 0;
		virtual void selection_mode_changed(const i_item_selection_model& aSelectionModel, item_selection_mode aNewMode) = 0;
		virtual void current_index_changed(const i_item_selection_model& aSelectionModel, const optional_item_model_index& aCurrentIndex, const optional_item_model_index& aPreviousIndex) = 0;
		virtual void selection_changed(const i_item_selection_model& aSelectionModel, const item_selection& aCurrentSelection, const item_selection& aPreviousSelection) = 0;
		virtual void selection_model_destroyed(const i_item_selection_model& aSelectionModel) = 0;
	public:
		enum notify_type { NotifyItemModelAdded, NotifyItemModelChanged, NotifyItemModelRemoved, NotifySelectionModeChanged, NotifyCurrentIndexChanged, NotifySelectionChanged, NotifySelectionModelDestroyed };
	};

	class i_item_selection_model
	{
	public:
		typedef event<const optional_item_model_index&, const optional_item_model_index&> current_index_changed_event;
		typedef event<const item_selection&, const item_selection&> selection_changed_event;
	public:
		struct no_item_model : std::logic_error { no_item_model() : std::logic_error("neogfx::i_item_selection_model::no_item_model") {} };
		struct no_current_index : std::logic_error { no_current_index() : std::logic_error("neogfx::i_item_selection_model::no_current_index") {} };
	public:
		virtual ~i_item_selection_model() {}
	public:
		virtual bool has_item_model() const = 0;
		virtual i_item_model& item_model() const = 0;
		virtual void set_item_model(i_item_model& aItemModel) = 0;
	public:
		virtual item_selection_mode mode() const = 0;
		virtual void set_mode(item_selection_mode aType) = 0;
	public:
		virtual bool has_current_index() const = 0;
		virtual const item_model_index& current_index() const = 0;
		virtual void set_current_index(const item_model_index& aIndex) = 0;
		virtual void unset_current_index() = 0;
	public:
		virtual const item_selection& selection() const = 0;
		virtual bool is_selected(const item_model_index& aIndex) const = 0;
		virtual void select(const item_model_index& aIndex, item_selection_operation = item_selection_operation::Select) = 0;
		virtual void select(const item_selection::range& aRange, item_selection_operation = item_selection_operation::Select) = 0;
	public:
		virtual void subscribe(i_item_selection_model_subscriber& aSubscriber) = 0;
		virtual void unsubscribe(i_item_selection_model_subscriber& aSubscriber) = 0;
	};

	inline item_selection_operation operator|(item_selection_operation aLhs, item_selection_operation aRhs)
	{
		return static_cast<item_selection_operation>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline item_selection_operation operator&(item_selection_operation aLhs, item_selection_operation aRhs)
	{
		return static_cast<item_selection_operation>(static_cast<uint32_t>(aLhs)& static_cast<uint32_t>(aRhs));
	}

	inline item_selection_operation operator~(item_selection_operation aLhs)
	{
		return static_cast<item_selection_operation>(~static_cast<uint32_t>(aLhs));
	}
}