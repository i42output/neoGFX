// spin_box.inl
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

#include "spin_box.hpp"
#include <boost/format.hpp>

namespace neogfx
{
	template <typename T>
	basic_spin_box<T>::basic_spin_box() :
		spin_box_impl(), iMinimum{}, iMaximum{}, iStep{}, iValue{}, iFormat{"%1"}
	{
	}

	template <typename T>
	basic_spin_box<T>::basic_spin_box(i_widget& aParent) :
		spin_box_impl(aParent), iMinimum{}, iMaximum{}, iStep{}, iValue{}, iFormat{ "%1" }
	{
	}

	template <typename T>
	basic_spin_box<T>::basic_spin_box(i_layout& aLayout) :
		spin_box_impl(aLayout), iMinimum{}, iMaximum{}, iStep{}, iValue{}, iFormat{ "%1" }
	{
	}

	template <typename T>
	typename basic_spin_box<T>::value_type basic_spin_box<T>::minimum() const
	{
		return iMinimum;
	}

	template <typename T>
	void basic_spin_box<T>::set_minimum(value_type aMinimum)
	{
		iMinimum = aMinimum;
		constraints_changed.trigger();
	}

	template <typename T>
	typename basic_spin_box<T>::value_type basic_spin_box<T>::maximum() const
	{
		return iMaximum;
	}

	template <typename T>
	void basic_spin_box<T>::set_maximum(value_type aMaximum)
	{
		iMaximum = aMaximum;
		constraints_changed.trigger();
	}

	template <typename T>
	typename basic_spin_box<T>::value_type basic_spin_box<T>::step() const
	{
		return iStep;
	}

	template <typename T>
	void basic_spin_box<T>::set_step(value_type aStep)
	{
		iStep = aStep;
		constraints_changed.trigger();
	}

	template <typename T>
	typename basic_spin_box<T>::value_type basic_spin_box<T>::value() const
	{
		return iValue;
	}

	template <typename T>
	void basic_spin_box<T>::set_value(value_type aValue)
	{
		iValue = aValue;
		value_changed.trigger();
	}

	template <typename T>
	const std::string& basic_spin_box<T>::format() const
	{
		return iForamt;
	}

	template <typename T>
	void basic_spin_box<T>::set_format(const std::string& aFormat)
	{
		iFormat = aFormat;
		update();
	}

	template <typename T>
	double basic_spin_box<T>::normalized_step_value() const
	{
		auto range = maximum() - minimum();
		if (range == 0)
			return 1.0;
		return static_cast<double>(step()) / range;
	}

	template <typename T>
	double basic_spin_box<T>::normalized_value() const
	{
		auto range = maximum() - minimum();
		if (range == 0)
			return 1.0;
		return (static_cast<double>(value()) - minimum()) / range;
	}

	template <typename T>
	void basic_spin_box<T>::set_normalized_value(double aValue)
	{
		auto range = maximum() - minimum();
		set_value(static_cast<value_type>(range * aValue + minimum()));
	}

	template <typename T>
	std::string basic_spin_box<T>::value_to_string() const
	{
		return boost::str(boost::format(iFormat) % value());
	}
}
