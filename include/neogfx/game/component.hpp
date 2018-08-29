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
#include <neolib/intrusive_sort.hpp>
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
		typedef typename detail::crack_component_data<Data>::container_type component_data_t;
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
		const component_data_t& component_data() const
		{
			return iComponentData;
		}
		component_data_t& component_data()
		{
			return iComponentData;
		}
		const value_type& operator[](typename component_data_t::size_type aIndex) const
		{
			return component_data()[aIndex];
		}
		value_type& operator[](typename component_data_t::size_type aIndex)
		{
			return component_data()[aIndex];
		}
	private:
		component_data_t iComponentData;
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
		typedef typename base_type::component_data_t component_data_t;
		typedef std::vector<entity_id> component_data_index_t;
		typedef std::vector<component_data_index_t::size_type> reverse_index_t;
	private:
		typedef static_component<Data> self_type;
	private:
		static constexpr component_data_index_t::size_type invalid = ~component_data_index_t::size_type{};
	public:
		static_component()
		{
		}
		const component_data_index_t& index() const
		{
			return iIndex;
		}
		component_data_index_t& index()
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
			auto reverseIndex = reverse_index()[aEntity];
			if (reverseIndex != invalid)
				return component_data()[reverseIndex];
			throw entity_record_not_found();
		}
		value_type& entity_record(entity_id aEntity)
		{
			return const_cast<value_type&>(const_cast<const self_type*>(this)->entity_record(aEntity));
		}
		value_type& populate(entity_id aEntity, const value_type& aData)
		{
			return do_populate(aEntity, aData);
		}
		value_type& populate(entity_id aEntity, value_type&& aData)
		{
			return do_populate(aEntity, aData);
		}
		void* populate(entity_id aEntity, const void* aComponentData, std::size_t aComponentDataSize) override
		{
			if ((aComponentData == nullptr && !is_data_optional()) || aComponentDataSize != sizeof(data_type))
				throw invalid_data();
			if (aComponentData != nullptr)
				return &do_populate(aEntity, *static_cast<const data_type*>(aComponentData));
			else
				return &do_populate(aEntity, value_type{}); // empty optional
		}
	public:
		template <typename Compare>
		void sort(Compare aComparator)
		{
			neolib::intrusive_sort(component_data().begin(), component_data().end(), 
				[this](auto lhs, auto rhs) 
				{ 
					std::swap(*lhs, *rhs);
					auto lhsIndex = lhs - component_data().begin();
					auto rhsIndex = rhs - component_data().begin();
					std::swap(index()[lhsIndex], index()[rhsIndex]);
					std::swap(reverse_index()[index()[lhsIndex]], reverse_index()[index()[rhsIndex]]);
				}, aComparator);
		}
	private:
		template <typename T>
		value_type& do_populate(entity_id aEntity, T&& aComponentData)
		{
			component_data().push_back(std::forward<T>(aComponentData));
			index().push_back(aEntity);
			if (reverse_index().size() < aEntity)
				reverse_index().resize(aEntity, invalid);
			reverse_index()[aEntity] = index().size() - 1;
			return component_data().back();
		}
	private:
		component_data_index_t iIndex;
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
			component_data().push_back(aData);
			return component_data().back();
		}
		value_type& populate(value_type&& aData)
		{
			component_data().push_back(std::move(aData));
			return component_data().back();
		}
		void* populate(const void* aComponentData, std::size_t aComponentDataSize) override
		{
			if ((aComponentData == nullptr && !is_data_optional()) || aComponentDataSize != sizeof(data_type))
				throw invalid_data();
			if (aComponentData != nullptr)
				return &populate(*static_cast<const data_type*>(aComponentData));
			else
				return &populate(value_type{}); // empty optional
		}
	};

	template <typename Data>
	struct shared
	{
		typedef typename detail::crack_component_data<shared<Data>>::value_type value_type;
		const value_type* ptr;
	};
}