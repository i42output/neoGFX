// component.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/i_component.hpp>

namespace neogfx
{
	template <typename Data>
	class static_component : public i_component
	{
	public:
		struct entity_record_not_found : std::logic_error { entity_record_not_found() : std::logic_error("neogfx::static_component::entity_record_not_found") {} };
		struct invalid_data : std::logic_error { invalid_data() : std::logic_error("neogfx::static_component::invalid_data") {} };
	public:
		typedef Data data_type;
		typedef typename data_type::meta meta_data_type;
		typedef std::vector<data_type> contents_t;
		struct index_entry
		{
			entity_id entityId;
			typename contents_t::size_type contentsIndex;
			bool operator<(const index_entry& aOther) const
			{
				return entityId < aOther.entityId;
			}
		};
		typedef std::vector<index_entry> index_t;
	private:
		typedef static_component<Data> self_type;
	private:
		static constexpr typename contents_t::size_type INVALID_INDEX = ~contents_t::size_type{};
	public:
		static_component()
		{
		}
	public:
		const component_id& id() const override
		{
			return meta_data_type::id();
		}
	public:
		bool is_data_shared() const override
		{
			return meta_data_type::is_shared();
		}
		const neolib::i_string& name() const override
		{
			return meta_data_type::name();
		}
		uint32_t field_count() const override
		{
			return meta_data_type::field_count();
		}
		component_data_field_type field_type(uint32_t aFieldIndex) const override
		{
			return meta_data_type::field_type(aFieldIndex);
		}
		const neolib::i_string& field_name(uint32_t aFieldIndex) const override
		{
			return meta_data_type::field_name(aFieldIndex);
		}
	public:
		const contents_t& contents() const
		{
			return iContents;
		}
		contents_t& contents()
		{
			return iContents;
		}
		const index_t& index() const
		{
			return iIndex;
		}
		index_t& index()
		{
			return iIndex;
		}
		const data_type& operator[](typename contents_t::size_type aIndex) const
		{
			return contents()[aIndex];
		}
		data_type& operator[](typename contents_t::size_type aIndex)
		{
			return contents()[aIndex];
		}
		const data_type& entity_record(entity_id aEntity) const
		{
			auto record = std::lower_bound(iIndex.begin(), iIndex.end(), index_entry{ aEntity });
			if (record != iIndex.end() && record->entityId == aEntity && record->contentsIndex != INVALID_INDEX)
				return contents()[record->contentsIndex];
			throw entity_record_not_found();
		}
		data_type& entity_record(entity_id aEntity)
		{
			return const_cast<data_type&>(const_cast<const self_type*>(this)->entity_record(aEntity));
		}
		void populate(entity_id aEntity, const data_type& aData)
		{
			iContents.push_back(aData);
			iIndex.push_back(index_entry{ aEntity, iContents.size() - 1 });
		}
		void populate(entity_id aEntity, data_type&& aData)
		{
			iContents.push_back(std::move(aData));
			iIndex.push_back(index_entry{ aEntity, iContents.size() - 1 });
		}
		void populate(entity_id aEntity, const void* aComponentData, std::size_t aComponentDataSize) override
		{
			if (aComponentData == nullptr || aComponentDataSize != sizeof(data_type))
				throw invalid_data();
			populate(aEntity, *static_cast<const data_type*>(aComponentData));
		}
	private:
		contents_t iContents;
		index_t iIndex;
	};
}