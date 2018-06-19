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
#include <boost/any.hpp>
#include <neolib/variant.hpp>
#include <neogfx/core/event.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/core/colour.hpp>
#include <neogfx/gfx/text/font.hpp>
#include <neogfx/gui/widget/widget_bits.hpp>

namespace neogfx
{
	class custom_type : public boost::any
	{
	public:
		using any::any;
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
		property_variant(const variant& other) :
			variant_t{ other }
		{
		}
		property_variant(variant&& other) :
			variant_t{ other }
		{
		}
		template <typename T>
		property_variant(T&& aValue, typename std::enable_if<variant_t::type_id<typename std::remove_cv<typename std::remove_reference<T>::type>::type>::valid, void>::type* = nullptr) :
			variant_t(std::forward<T>(aValue))
		{
		}
		template <typename T>
		property_variant(T&& aValue, typename std::enable_if<!variant_t::type_id<typename std::remove_cv<typename std::remove_reference<T>::type>::type>::valid, void>::type* = nullptr) :
			variant_t{ custom_type{ std::forward<T>(aValue) } }
		{
		}
	};

	typedef property_variant variant;

	template <typename T> struct variant_type_for { typedef custom_type type; };
	template <typename T> struct variant_type_for<T*> { typedef void* type; };
	template <> struct variant_type_for<variant::type_1> { typedef variant::type_1 type; };
	template <> struct variant_type_for<variant::type_2> { typedef variant::type_2 type; };
	template <> struct variant_type_for<variant::type_3> { typedef variant::type_3 type; };
	template <> struct variant_type_for<variant::type_4> { typedef variant::type_4 type; };
	template <> struct variant_type_for<variant::type_5> { typedef variant::type_5 type; };
	template <> struct variant_type_for<variant::type_6> { typedef variant::type_6 type; };
	template <> struct variant_type_for<variant::type_7> { typedef variant::type_7 type; };
	template <> struct variant_type_for<variant::type_8> { typedef variant::type_8 type; };
	template <> struct variant_type_for<variant::type_9> { typedef variant::type_9 type; };
	template <> struct variant_type_for<variant::type_10> { typedef variant::type_10 type; };
	template <> struct variant_type_for<variant::type_11> { typedef variant::type_11 type; };
	template <> struct variant_type_for<variant::type_12> { typedef variant::type_12 type; };
	template <> struct variant_type_for<variant::type_13> { typedef variant::type_13 type; };
	template <> struct variant_type_for<variant::type_14> { typedef variant::type_14 type; };
	template <> struct variant_type_for<variant::type_15> { typedef variant::type_15 type; };

	class i_property
	{
	public:
		event<variant> changed;
	public:
		virtual ~i_property() {}
	public:
		virtual const std::string& name() const = 0;
		virtual const std::type_info& type() const = 0;
		virtual const std::type_info& category() const = 0;
		virtual bool optional() const = 0;
		virtual variant get() const = 0;
		virtual void set(const variant& aValue) = 0;
	};
}
