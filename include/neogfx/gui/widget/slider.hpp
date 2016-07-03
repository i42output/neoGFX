// slider.hpp
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

#include <neogfx/neogfx.hpp>
#include "widget.hpp"

namespace neogfx
{
	class slider_impl : public widget
	{
	public:
		event<> value_changed;
		event<> constraints_changed;
	public:
		enum type_e
		{
			Horizontal,
			Vertical
		};
	public:
		slider_impl(type_e aType = Horizontal);
		slider_impl(i_widget& aParent, type_e aType = Horizontal);
		slider_impl(i_layout& aLayout, type_e aType = Horizontal);
		~slider_impl();
	public:
		virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
	public:
		virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
		virtual void mouse_button_released(mouse_button aButton, const point& aPosition);
		virtual void mouse_moved(const point& aPosition);
	public:
		virtual double normalized_step_value() const = 0;
		virtual double normalized_value() const = 0;
		virtual void set_normalized_value(double aValue);
	private:
		void init();
		rect bar_box() const;
		rect indicator_box() const;
	private:
		type_e iType;
		double iNormalizedValue;
		optional_point iDragOffset;
	};

	template <typename T>
	class basic_slider : public slider_impl
	{
	public:
		typedef T value_type;
	public:
		basic_slider(type_e aType = Horizontal);
		basic_slider(i_widget& aParent, type_e aType = Horizontal);
		basic_slider(i_layout& aLayout, type_e aType = Horizontal);
	public:
		value_type minimum() const;
		void set_minimum(value_type aMinimum);
		value_type maximum() const;
		void set_maximum(value_type aMaximum);
		value_type step() const;
		void set_step(value_type aStep);
		value_type value() const;
		void set_value(value_type aValue);
	public:
		virtual double normalized_step_value() const;
		virtual double normalized_value() const;
		virtual void set_normalized_value(double aValue);
	private:
		value_type iMinimum;
		value_type iMaximum;
		value_type iStep;
		value_type iValue;
		bool iSettingNormalizedValue;
	};
}

#include "slider.inl"

namespace neogfx
{
	typedef basic_slider<int32_t> slider;
	typedef basic_slider<double> double_slider;
}