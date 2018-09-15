// i_property.hpp
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
#include <type_traits>
#include <any>
#include <neolib/variant.hpp>
#include <neogfx/core/event.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/core/colour.hpp>
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/gui/widget/widget_bits.hpp>

namespace neogfx
{
	class custom_type : public std::any
	{
	public:
		custom_type() : ptr{ &custom_type::do_ptr<void> } 
		{
		}
		custom_type(const custom_type& aOther) : std::any{ aOther }, ptr{ aOther.ptr }
		{
		}
		custom_type(custom_type&& aOther) : std::any{ std::move(aOther) }, ptr{ aOther.ptr }
		{
			aOther.ptr = &custom_type::do_ptr<void>;
		}
		template <typename ValueType>
		custom_type(ValueType&& aValue) : std::any{ std::forward<ValueType>(aValue) }, ptr{ &custom_type::do_ptr<typename std::remove_reference<ValueType>::type> }
		{
		}
	public:
		custom_type& operator=(const custom_type& aRhs)
		{
			std::any::operator=(aRhs);
			ptr = aRhs.ptr;
			return *this;
		}
		custom_type& operator=(custom_type&& aRhs)
		{
			std::any::operator=(std::move(aRhs));
			ptr = aRhs.ptr;
			aRhs.ptr = &custom_type::do_ptr<void>;
			return *this;
		}
		template<typename ValueType>
		custom_type& operator=(ValueType&& aRhs)
		{
			std::any::operator=(std::forward<ValueType>(aRhs));
			ptr = &custom_type::do_ptr<typename std::remove_reference<ValueType>::type>;
			return *this;
		}
	public:
		template<class ValueType, class... Args >
		std::decay_t<ValueType>& emplace(Args&&... args)
		{
			auto& result = std::any::emplace<ValueType>(std::forward<Args...>(args...));
			ptr = &custom_type::do_ptr<typename std::remove_reference<ValueType>::type>;
			return result;
		}
		template<class ValueType, class U, class... Args >
		std::decay_t<ValueType>& emplace(std::initializer_list<U> il, Args&&... args)
		{
			auto& result = std::any::emplace<ValueType>(il, std::forward<Args...>(args...));
			ptr = &custom_type::do_ptr<typename std::remove_reference<ValueType>::type>;
			return result;
		}
		void reset()
		{
			std::any::reset();
			ptr = &custom_type::do_ptr<void>;
		}
	public:
		bool operator==(const custom_type& aOther) const
		{
			return ptr(*this) == aOther.ptr(aOther);
		}
		bool operator!=(const custom_type& aOther) const
		{
			return ptr(*this) != aOther.ptr(aOther);
		}
		bool operator<(const custom_type& aOther) const
		{
			return ptr(*this) < aOther.ptr(aOther);
		}
	private:
		template <typename T>
		static const void* do_ptr(const custom_type& aArg)
		{
			return std::any_cast<const T*>(&aArg);
		}
	private:
		const void*(*ptr)(const custom_type&);
	};

	typedef neolib::variant<
		void*,
		bool,
		char,
		int32_t,
		uint32_t,
		int64_t,
		uint64_t,
		float,
		double,
		std::string,
		size,
		point,
		rect,
		custom_type> variant_t;

	class property_variant : public variant_t
	{
	public:
		property_variant() : 
			variant_t{}
		{
		}
		property_variant(const property_variant& other) :
			variant_t{ static_cast<const variant_t&>(other) }
		{
		}
		property_variant(property_variant&& other) :
			variant_t{ static_cast<variant_t&&>(std::move(other)) }
		{
		}
		template <typename T>
		property_variant(T&& aValue) :
			variant_t{ std::forward<T>(aValue) }
		{
		}
	public:
		using variant_t::operator==;
		using variant_t::operator!=;
	};

	template <typename T> struct variant_type_for { typedef custom_type type; };
	template <typename T> struct variant_type_for<T*> { typedef void* type; };

	class i_property
	{
	public:
		event<property_variant> changed;
	public:
		virtual ~i_property() {}
	public:
		virtual const std::string& name() const = 0;
		virtual const std::type_info& type() const = 0;
		virtual const std::type_info& category() const = 0;
		virtual bool optional() const = 0;
		virtual property_variant get() const = 0;
		virtual void set(const property_variant& aValue) = 0;
	};
}
