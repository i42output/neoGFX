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
	inline basic_spin_box<T>::basic_spin_box() :
		spin_box_impl(), iMinimum{}, iMaximum{}, iStep{}, iValue{}, iFormat{"%1%"}, iSettingNormalizedValue{ false }
	{
	}

	template <typename T>
	inline basic_spin_box<T>::basic_spin_box(i_widget& aParent) :
		spin_box_impl(aParent), iMinimum{}, iMaximum{}, iStep{}, iValue{}, iFormat{ "%1%" }, iSettingNormalizedValue{ false }
	{
	}

	template <typename T>
	inline basic_spin_box<T>::basic_spin_box(i_layout& aLayout) :
		spin_box_impl(aLayout), iMinimum{}, iMaximum{}, iStep{}, iValue{}, iFormat{ "%1%" }, iSettingNormalizedValue{ false }
	{
	}

	template <typename T>
	inline typename basic_spin_box<T>::value_type basic_spin_box<T>::minimum() const
	{
		return iMinimum;
	}

	template <typename T>
	inline void basic_spin_box<T>::set_minimum(value_type aMinimum)
	{
		iMinimum = aMinimum;
		std::string text;
		try { text = boost::str(boost::format(iFormat) % minimum()); } catch (...) {}
		if (text_box().text().empty())
			text_box().set_text(text);
		constraints_changed.trigger();
		if (iValue < minimum())
			set_value(minimum());
	}

	template <typename T>
	inline typename basic_spin_box<T>::value_type basic_spin_box<T>::maximum() const
	{
		return iMaximum;
	}

	template <typename T>
	inline void basic_spin_box<T>::set_maximum(value_type aMaximum)
	{
		iMaximum = aMaximum;
		constraints_changed.trigger();
		if (iValue > maximum())
			set_value(maximum());
	}

	template <typename T>
	inline typename basic_spin_box<T>::value_type basic_spin_box<T>::step() const
	{
		return iStep;
	}

	template <typename T>
	inline void basic_spin_box<T>::set_step(value_type aStep)
	{
		iStep = aStep;
		constraints_changed.trigger();
	}

	template <typename T>
	inline typename basic_spin_box<T>::value_type basic_spin_box<T>::value() const
	{
		return iValue;
	}

	template <typename T>
	inline void basic_spin_box<T>::set_value(value_type aValue)
	{
		if (iValue != aValue)
		{
			iValue = aValue;
			if (!iSettingNormalizedValue)
				spin_box_impl::set_normalized_value(normalized_value(), true);
			value_changed.trigger();
		}
	}

	template <typename T>
	inline const std::string& basic_spin_box<T>::format() const
	{
		return iForamt;
	}

	template <typename T>
	inline void basic_spin_box<T>::set_format(const std::string& aFormat)
	{
		iFormat = aFormat;
		update();
	}

	template <typename T>
	inline const std::string& basic_spin_box<T>::valid_text_characters() const
	{
		if (std::is_integral<T>::value)
		{
			if (std::is_signed<T>::value)
			{
				static const std::string sValid{ "01234567890+-" };
				return sValid;
			}
			else
			{
				static const std::string sValid{ "01234567890" };
				return sValid;
			}
		}
		else if (std::is_floating_point<T>::value)
		{
			static const std::string sValid{ "01234567890.+-eE" };
			return sValid;
		}
		else
		{
			static const std::string sValid;
			return sValid;
		}
	}

	template <typename T>
	inline double basic_spin_box<T>::normalized_step_value() const
	{
		auto range = maximum() - minimum();
		if (range == 0)
			return 1.0;
		return static_cast<double>(step()) / range;
	}

	template <typename T>
	inline double basic_spin_box<T>::normalized_value() const
	{
		auto range = maximum() - minimum();
		if (range == 0)
			return 1.0;
		return (static_cast<double>(value()) - minimum()) / range;
	}

	template <typename T>
	inline void basic_spin_box<T>::set_normalized_value(double aValue, bool aUpdateTextBox = false)
	{
		aValue = std::max(0.0, std::min(1.0, aValue));
		iSettingNormalizedValue = true;
		auto range = maximum() - minimum();
		auto denormalized = range * aValue + minimum();
		if (std::is_integral<value_type>())
		{
			if (denormalized < 0.0)
				denormalized = std::floor(denormalized + 0.5);
			else if (denormalized > 0.0)
				denormalized = std::ceil(denormalized - 0.5);
		}
		set_value(static_cast<value_type>(denormalized));
		spin_box_impl::set_normalized_value(aValue, aUpdateTextBox);
		iSettingNormalizedValue = false;
	}

	template <typename T>
	inline boost::optional<double> basic_spin_box<T>::string_to_normalized_value(const std::string& aText) const
	{
		if (aText.empty())
			return boost::optional<double>{};
		auto range = maximum() - minimum();
		if (range == 0)
			return 0.0;
		std::istringstream iss(aText);
		value_type result{};
		if (!(iss >> result))
			return boost::optional<double>{};
		std::string guff;
		if (iss >> guff)
			return boost::optional<double>{};
		return (static_cast<double>(result) - minimum()) / range;
	}

	template <typename T>
	inline std::string basic_spin_box<T>::normalized_value_to_string(double aNormalizedValue) const
	{
		auto range = maximum() - minimum();
		std::ostringstream oss;
		oss << static_cast<value_type>(aNormalizedValue * range);
		return oss.str();
	}

	template <typename T>
	inline std::string basic_spin_box<T>::value_to_string() const
	{
		std::string text;
		try { text = boost::str(boost::format(iFormat) % value()); } catch (...) {}
		return text;
	}
}
