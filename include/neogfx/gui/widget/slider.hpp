// slider.hpp
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
#include <neogfx/gui/widget/widget.hpp>

namespace neogfx
{
    enum class slider_orientation : uint32_t
    {
        Horizontal,
        Vertical
    };
}

template <>
const neolib::enum_enumerators_t<neogfx::slider_orientation> neolib::enum_enumerators_v<neogfx::slider_orientation>
{
    declare_enum_string(neogfx::slider_orientation, Horizontal)
    declare_enum_string(neogfx::slider_orientation, Vertical)
};

namespace neogfx
{
    class slider_impl : public widget
    {
    public:
        define_event(ValueChanged, value_changed)
        define_event(ConstraintsChanged, constraints_changed)
    public:
        slider_impl(slider_orientation aOrientation = slider_orientation::Horizontal);
        slider_impl(i_widget& aParent, slider_orientation aOrientation = slider_orientation::Horizontal);
        slider_impl(i_layout& aLayout, slider_orientation aOrientation = slider_orientation::Horizontal);
        ~slider_impl();
    public:
        void set_bar_color(const optional_color_or_gradient& aBarColor);
    public:
        virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
    public:
        virtual void paint(i_graphics_context& aGc) const;
    public:
        virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
        virtual void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
        virtual void mouse_button_released(mouse_button aButton, const point& aPosition);
        virtual void mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifiers_e aKeyModifiers);
        virtual void mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers);
    public:
        virtual double normalized_step_value() const = 0;
        virtual double normalized_value() const = 0;
        virtual void set_normalized_value(double aValue);
    private:
        void init();
        rect bar_box() const;
        rect indicator_box() const;
        double normalized_value_from_position(const point& aPosition) const;
        point normalized_value_to_position(double aValue) const;
    private:
        slider_orientation iOrientation;
        optional_color_or_gradient iBarColor;
        double iNormalizedValue;
        optional_point iDragOffset;
    };

    template <typename T>
    class basic_slider : public slider_impl
    {
    public:
        typedef T value_type;
    public:
        basic_slider(slider_orientation aOrientation = slider_orientation::Horizontal);
        basic_slider(i_widget& aParent, slider_orientation aOrientation = slider_orientation::Horizontal);
        basic_slider(i_layout& aLayout, slider_orientation aOrientation = slider_orientation::Horizontal);
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