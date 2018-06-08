// property.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <string>
#include <neogfx/core/i_object.hpp>
#include <neogfx/core/i_property.hpp>

namespace neogfx
{
	using namespace std::string_literals;

	namespace detail
	{
		template <typename T>
		struct property_optional_type_cracker 
		{ 
			typedef T type;
			static constexpr bool optional = false;
		};
		template <typename T>
		struct property_optional_type_cracker<boost::optional<T>>
		{
			typedef T type;
			static constexpr bool optional = true;
		};
	}

	template <typename T>
	class property : public i_property
	{
	public:
		typedef T value_type;
	public:
		event<const value_type&> changed;
	private:
		typedef detail::property_optional_type_cracker<T> cracker;
	public:
		property(i_object& aOwner, const std::string& aName) : iName{ aName }, iValue{}
		{
			aOwner.properties().register_property(*this);
		}
		property(i_object& aOwner, const std::string& aName, const T& aValue) : iName{ aName }, iValue { aValue }
		{
			aOwner.properties().register_property(*this);
		}
	public:
		const std::string& name() const override
		{
			return iName;
		}
		const std::type_info& type() const override
		{
			return typeid(value_type);
		}
		bool optional() const override
		{
			return cracker::optional;
		}
		variant get() const override
		{
			return iValue;
		}
		void set(const variant& aValue) override
		{
			if (!aValue.empty())
			{
				if (iValue != static_variant_cast<const typename cracker::type&>(aValue))
				{
					iValue = static_variant_cast<const typename cracker::type&>(aValue);
					i_property::changed.trigger(get());
					changed.trigger(iValue);
				}
			}
			else
			{
				if (iValue != value_type{})
				{
					iValue = value_type{};
					i_property::changed.trigger(get());
					changed.trigger(iValue);
				}
			}
		}
	public:
		template <typename T2>
		value_type& operator=(const T2& aValue)
		{
			return iValue = aValue;
		}
		operator const value_type&() const
		{
			return iValue;
		}
		template <typename SFINAE = typename cracker::type>
		const typename std::enable_if<cracker::optional, SFINAE>::type& operator*() const
		{
			return *iValue;
		}
		template <typename SFINAE = typename cracker::type>
		typename std::enable_if<cracker::optional, SFINAE>::type& operator*()
		{
			return *iValue;
		}
		template <typename SFINAE = typename cracker::type>
		const typename std::enable_if<cracker::optional, SFINAE>::type* operator->() const
		{
			return &*iValue;
		}
		template <typename SFINAE = typename cracker::type>
		typename std::enable_if<cracker::optional, SFINAE>::type* operator->()
		{
			return &*iValue;
		}
		template <typename T>
		bool operator==(const T& aRhs) const
		{
			return iValue == aRhs;
		}
		template <typename T>
		bool operator!=(const T& aRhs) const
		{
			return iValue != aRhs;
		}
	private:
		std::string iName;
		mutable value_type iValue;
	};

	#define define_property( type, name, ... ) neogfx::property<type> name = { *this, #name ##s, __VA_ARGS__ };
}