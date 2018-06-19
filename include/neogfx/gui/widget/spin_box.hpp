// spin_box.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/widget/line_edit.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/widget/push_button.hpp>

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
		neogfx::size_policy size_policy() const override;
	protected:
		virtual colour frame_colour() const;
	public:
		virtual void mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta);
	public:
		virtual bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
	public:
		const std::string& text();
		line_edit& text_box();
	public:
		virtual const std::string& valid_text_characters() const = 0;
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
		sink iSink;
		std::string iText;
		cursor::position_type iTextCursorPos;
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
		const std::string& valid_text_characters() const override;
		double normalized_step_value() const override;
		double normalized_value() const override;
		void set_normalized_value(double aValue, bool aUpdateTextBox = false) override;
		boost::optional<double> string_to_normalized_value(const std::string& aText) const override;
		std::string normalized_value_to_string(double aNormalizedValue) const override;
		std::string value_to_string() const override;
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
	typedef basic_spin_box<int32_t> int32_spin_box;
	typedef basic_spin_box<uint32_t> uint32_spin_box;
	typedef basic_spin_box<int64_t> int64_spin_box;
	typedef basic_spin_box<uint64_t> uint64_spin_box;
	typedef basic_spin_box<float> float_spin_box;
	typedef basic_spin_box<double> double_spin_box;

	typedef int32_spin_box spin_box;
}