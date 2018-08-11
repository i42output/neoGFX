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
#include <boost/container/stable_vector.hpp>
#include <neogfx/game/ecs_ids.hpp>
#include <neogfx/game/i_component.hpp>

namespace neogfx::game
{
	namespace detail
	{
		template <typename Data>
		struct shared;
			
		template <typename Data>
		struct crack_component_data
		{
			typedef Data data_type;
			typedef Data value_type;
			typedef std::vector<value_type> container_type;
			static constexpr bool optional = false;
		};

		template <typename Data>
		struct crack_component_data<std::optional<Data>>
		{
			typedef Data data_type;
			typedef std::optional<Data> value_type;
			typedef std::vector<value_type> container_type;
			static constexpr bool optional = true;
		};

		template <typename Data>
		struct crack_component_data<shared<Data>>
		{
			typedef Data data_type;
			typedef Data value_type;
			typedef boost::container::stable_vector<value_type> container_type;
			static constexpr bool optional = false;
		};

		template <typename Data>
		struct crack_component_data<shared<std::optional<Data>>>
		{
			typedef Data data_type;
			typedef std::optional<Data> value_type;
			typedef boost::container::stable_vector<value_type> container_type;
			static constexpr bool optional = true;
		};
	}

	template <typename Data, typename Base>
	class static_component_base : public Base
	{
	public:
		struct entity_record_not_found : std::logic_error { entity_record_not_found() : std::logic_error("neogfx::static_component::entity_record_not_found") {} };
		struct invalid_data : std::logic_error { invalid_data() : std::logic_error("neogfx::static_component::invalid_data") {} };
	public:
		typedef typename detail::crack_component_data<Data>::data_type data_type;
		typedef typename data_type::meta meta_data_type;
		typedef typename detail::crack_component_data<Data>::value_type value_type;
		typedef typename detail::crack_component_data<Data>::container_type contents_t;
	private:
		typedef static_component_base<Data, Base> self_type;
	public:
		static_component_base()
		{
		}
	public:
		const component_id& id() const override
		{
			return meta_data_type::id();
		}
	public:
		bool is_data_optional() const override
		{
			return detail::crack_component_data<Data>::optional;
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
		neolib::uuid field_type_id(uint32_t aFieldIndex) const override
		{
			return meta_data_type::field_type_id(aFieldIndex);
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
		const value_type& operator[](typename contents_t::size_type aIndex) const
		{
			return contents()[aIndex];
		}
		value_type& operator[](typename contents_t::size_type aIndex)
		{
			return contents()[aIndex];
		}
	private:
		contents_t iContents;
	};

	template <typename Data>
	class static_component : public static_component_base<Data, i_component>
	{
	private:
		typedef static_component_base<Data, i_component> base_type;
	public:
		typedef typename base_type::data_type data_type;
		typedef typename base_type::meta_data_type meta_data_type;
		typedef typename base_type::value_type value_type;
		typedef typename base_type::contents_t contents_t;
		typedef std::vector<entity_id> contents_index_t;
		struct reverse_index_entry
		{
			entity_id entityId;
			typename contents_t::size_type contentsIndex;
			bool operator<(const reverse_index_entry& aOther) const
			{
				return entityId < aOther.entityId;
			}
		};
		typedef std::vector<reverse_index_entry> reverse_index_t;
	private:
		typedef static_component<Data> self_type;
	private:
		static constexpr typename contents_t::size_type INVALID_INDEX = ~contents_t::size_type{};
	public:
		static_component()
		{
		}
		const contents_index_t& index() const
		{
			return iIndex;
		}
		contents_index_t& index()
		{
			return iIndex;
		}
		const reverse_index_t& reverse_index() const
		{
			return iReverseIndex;
		}
		reverse_index_t& reverse_index()
		{
			return iReverseIndex;
		}
		const data_type& entity_record(entity_id aEntity) const
		{
			auto record = std::lower_bound(reverse_index().begin(), reverse_index().end(), reverse_index_entry{ aEntity });
			if (record != reverse_index().end() && record->entityId == aEntity && record->contentsIndex != INVALID_INDEX)
				return contents()[record->contentsIndex];
			throw entity_record_not_found();
		}
		value_type& entity_record(entity_id aEntity)
		{
			return const_cast<value_type&>(const_cast<const self_type*>(this)->entity_record(aEntity));
		}
		value_type& populate(entity_id aEntity, const value_type& aData)
		{
			contents().push_back(aData);
			index().push_back(aEntity);
			reverse_index().push_back(reverse_index_entry{ aEntity, iContents.size() - 1 });
			return contents().back();
		}
		value_type& populate(entity_id aEntity, value_type&& aData)
		{
			contents().push_back(std::move(aData));
			index().push_back(aEntity);
			reverse_index().push_back(reverse_index_entry{ aEntity, iContents.size() - 1 });
			return contents().back();
		}
		void* populate(entity_id aEntity, const void* aComponentData, std::size_t aComponentDataSize) override
		{
			if ((aComponentData == nullptr && !is_data_optional()) || aComponentDataSize != sizeof(data_type))
				throw invalid_data();
			if (aComponentData != nullptr)
				return &populate(aEntity, *static_cast<const data_type*>(aComponentData));
			else
				return &populate(aEntity, value_type{}); // empty optional
		}
	private:
		contents_index_t iIndex;
		reverse_index_t iReverseIndex;
	};

	template <typename Data>
	class static_shared_component : public static_component_base<Data, i_shared_component>
	{
	private:
		typedef static_component_base<Data, i_shared_component> base_type;
	public:
		typedef typename base_type::data_type data_type;
		typedef typename base_type::meta_data_type meta_data_type;
		typedef typename base_type::value_type value_type;
	private:
		typedef static_shared_component<Data> self_type;
	public:
		static_shared_component()
		{
		}
		value_type& populate(const value_type& aData)
		{
			contents().push_back(aData);
			return contents().back();
		}
		value_type& populate(value_type&& aData)
		{
			contents().push_back(std::move(aData));
			return contents().back();
		}
		void* populate(const void* aComponentData, std::size_t aComponentDataSize) override
		{
			if ((aComponentData == nullptr && !is_data_optional()) || aComponentDataSize != sizeof(data_type))
				throw invalid_data();
			if (aComponentData != nullptr)
				return &populate(aEntity, *static_cast<const data_type*>(aComponentData));
			else
				return &populate(aEntity, value_type{}); // empty optional
		}
	};

	template <typename Data>
	struct shared
	{
		typedef typename detail::crack_component_data<shared<Data>>::value_type value_type;
		const value_type* ptr;
	};
}