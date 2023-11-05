// slider.ipp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neolib/core/scoped.hpp>
#include <neogfx/gui/widget/slider.hpp>

namespace neogfx
{
    template <typename T>
    inline basic_slider<T>::basic_slider(slider_orientation aOrientation) :
        slider_impl(aOrientation), iMinimum{}, iMaximum{}, iStep{}, iValue{}, iSettingNormalizedValue{ false }
    {
    }

    template <typename T>
    inline basic_slider<T>::basic_slider(i_widget& aParent, slider_orientation aOrientation) :
        slider_impl(aParent, aOrientation), iMinimum{}, iMaximum{}, iStep{}, iValue{}, iSettingNormalizedValue{ false }
    {
    }

    template <typename T>
    inline basic_slider<T>::basic_slider(i_layout& aLayout, slider_orientation aOrientation) :
        slider_impl(aLayout, aOrientation), iMinimum{}, iMaximum{}, iStep{}, iValue{}, iSettingNormalizedValue{ false }
    {
    }

    template <typename T>
    inline typename basic_slider<T>::value_type basic_slider<T>::minimum() const
    {
        return iMinimum;
    }

    template <typename T>
    inline void basic_slider<T>::set_minimum(value_type aMinimum)
    {
        iMinimum = aMinimum;
        ConstraintsChanged.trigger();
        if (iValue < minimum())
            set_value(minimum());
    }

    template <typename T>
    typename basic_slider<T>::value_type basic_slider<T>::maximum() const
    {
        return iMaximum;
    }

    template <typename T>
    inline void basic_slider<T>::set_maximum(value_type aMaximum)
    {
        iMaximum = aMaximum;
        ConstraintsChanged.trigger();
        if (iValue > maximum())
            set_value(maximum());
    }

    template <typename T>
    inline typename basic_slider<T>::value_type basic_slider<T>::step() const
    {
        return iStep;
    }

    template <typename T>
    inline void basic_slider<T>::set_step(value_type aStep)
    {
        iStep = aStep;
        ConstraintsChanged.trigger();
    }

    template <typename T>
    inline typename basic_slider<T>::value_type basic_slider<T>::value() const
    {
        return iValue;
    }

    template <typename T>
    inline void basic_slider<T>::set_value(value_type aValue)
    {
        if (iValue != aValue)
        {
            iValue = aValue;
            if (!iSettingNormalizedValue)
                slider_impl::set_normalized_value(normalized_value());
            if (!handling_event())
                ValueChanged.sync_trigger();
            else
                ValueChanged.trigger();
        }
    }

    template <typename T>
    inline double basic_slider<T>::normalized_step_value() const
    {
        auto range = maximum() - minimum();
        if (range == 0)
            return 1.0;
        return static_cast<double>(step()) / range;
    }

    template <typename T>
    inline double basic_slider<T>::normalized_value() const
    {
        auto range = maximum() - minimum();
        if (range == 0)
            return 1.0;
        return (static_cast<double>(value()) - minimum()) / range;
    }

    template <typename T>
    inline void basic_slider<T>::set_normalized_value(double aValue)
    {
        double const stepValue = normalized_step_value();
        double steps = 0.0;
        auto r = std::modf(aValue / stepValue, &steps);
        if (r > stepValue / 2.0)
            steps += 1.0;
        aValue = std::max(0.0, std::min(1.0, steps * stepValue));
        neolib::scoped_flag sf{ iSettingNormalizedValue };
        auto const range = maximum() - minimum();
        auto denormalized = range * aValue + minimum();
        if (std::is_integral<value_type>())
        {
            if (denormalized < 0.0)
                denormalized = std::floor(denormalized + 0.5);
            else if (denormalized > 0.0)
                denormalized = std::ceil(denormalized - 0.5);
        }
        set_value(static_cast<value_type>(denormalized));
        slider_impl::set_normalized_value(aValue);
    }
}
