// spin_box.hpp
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
#include "horizontal_layout.hpp"
#include "line_edit.hpp"
#include "vertical_layout.hpp"
#include "push_button.hpp"

namespace neogfx
{
	class spin_box_impl : public framed_widget
	{
	public:
		event<> value_changed;
		event<> constraints_changed;
	public:
		spin_box_impl();
		spin_box_impl(i_widget& aParent);
		spin_box_impl(i_layout& aLayout);
		~spin_box_impl();
	protected:
		virtual colour frame_colour() const;
	public:
		line_edit& text_box();
	public:
		virtual double normalized_step_value() const = 0;
		virtual double normalized_value() const = 0;
		virtual void set_normalized_value(double aValue, bool aUpdateTextBox = false);
		virtual boost::optional<double> string_to_normalized_value(const std::string& aText) const = 0;
		virtual std::string normalized_value_to_string(double aNormalizedValue) const = 0;
		virtual std::string value_to_string() const = 0;
	private:
		void init();
		void update_arrows();
	private:
		double iNormalizedValue;
		horizontal_layout iPrimaryLayout;
		line_edit iTextBox;
		vertical_layout iSecondaryLayout;
		push_button iStepUpButton;
		push_button iStepDownButton;
		boost::optional<neolib::callback_timer> iStepper;
		mutable boost::optional<std::pair<colour, texture>> iUpArrow;
		mutable boost::optional<std::pair<colour, texture>> iDownArrow;
	};

	template <typename T>
	class basic_spin_box : public spin_box_impl
	{
	public:
		typedef T value_type;
	public:
		basic_spin_box();
		basic_spin_box(i_widget& aParent);
		basic_spin_box(i_layout& aLayout);
	public:
		value_type minimum() const;
		void set_minimum(value_type aMinimum);
		value_type maximum() const;
		void set_maximum(value_type aMaximum);
		value_type step() const;
		void set_step(value_type aStep);
		value_type value() const;
		void set_value(value_type aValue);
		const std::string& format() const;
		void set_format(const std::string& aFormat);
	public:
		virtual double normalized_step_value() const;
		virtual double normalized_value() const;
		virtual void set_normalized_value(double aValue, bool aUpdateTextBox = false);
		virtual boost::optional<double> string_to_normalized_value(const std::string& aText) const;
		virtual std::string normalized_value_to_string(double aNormalizedValue) const;
		virtual std::string value_to_string() const;
	private:
		value_type iMinimum;
		value_type iMaximum;
		value_type iStep;
		value_type iValue;
		std::string iFormat;
		bool iSettingNormalizedValue;
	};
}

#include "spin_box.inl"

namespace neogfx
{
	typedef basic_spin_box<uint32_t> integer_spin_box;
	typedef basic_spin_box<double> decimal_spin_box;
	typedef integer_spin_box spin_box;
}