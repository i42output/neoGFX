// slider_box.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gui/widget/slider.hpp>
#include <neogfx/gui/widget/spin_box.hpp>

namespace neogfx
{
    template <typename T>
    class basic_slider_box : public widget<>
    {
        meta_object(widget<>)
    public:
        typedef T value_type;
    public:
        define_event(ValueChanged, value_changed)
        define_event(ConstraintsChanged, constraints_changed)
    public:
        basic_slider_box() : 
            widget{},
            iLayout{ *this },
            iSlider{ iLayout },
            iSpinBox{ iLayout },
            iSettingParameter{ false }
        {
            init();
        }
        basic_slider_box(i_widget& aParent) :
            widget{ aParent },
            iLayout{ *this },
            iSlider{ iLayout },
            iSpinBox{ iLayout },
            iSettingParameter{ false }
        {
            init();
        }
        basic_slider_box(i_layout& aLayout) :
            widget{ aLayout },
            iLayout{ *this },
            iSlider{ iLayout },
            iSpinBox{ iLayout },
            iSettingParameter{ false }
        {
            init();
        }
    public:
        value_type minimum() const
        {
            return iSlider.minimum();
        }
        void set_minimum(value_type aMinimum)
        {
            if (iSettingParameter)
                return;
            neolib::scoped_flag sf{ iSettingParameter };
            iSlider.set_minimum(aMinimum);
            iSpinBox.set_minimum(aMinimum);
            ConstraintsChanged.trigger();
        }
        value_type maximum() const
        {
            return iSlider.maximum();
        }
        void set_maximum(value_type aMaximum)
        {
            if (iSettingParameter)
                return;
            neolib::scoped_flag sf{ iSettingParameter };
            iSlider.set_maximum(aMaximum);
            iSpinBox.set_maximum(aMaximum);
            ConstraintsChanged.trigger();
        }
        value_type step() const
        {
            return iSlider.step();
        }
        void set_step(value_type aStep)
        {
            if (iSettingParameter)
                return;
            neolib::scoped_flag sf{ iSettingParameter };
            iSlider.set_step(aStep);
            iSpinBox.set_step(aStep);
            ConstraintsChanged.trigger();
        }
        value_type value() const
        {
            return iSlider.value();
        }
        void set_value(value_type aValue)
        {
            if (iSettingParameter)
                return;
            neolib::scoped_flag sf{ iSettingParameter };
            iSlider.set_value(aValue);
            iSpinBox.set_value(aValue);
            ValueChanged.trigger();
        }
    public:
        const basic_slider<value_type>& slider() const
        {
            return iSlider;
        }
        basic_slider<value_type>& slider()
        {
            return iSlider;
        }
        const basic_spin_box<value_type>& spin_box() const
        {
            return iSpinBox;
        }
        basic_spin_box<value_type>& spin_box()
        {
            return iSpinBox;
        }
    private:
        void init()
        {
            iSlider.ValueChanged([&]()
            {
                if (iSettingParameter)
                    return;
                neolib::scoped_flag sf{ iSettingParameter };
                iSpinBox.set_value(iSlider.value());
                ValueChanged.trigger();
            });
            iSpinBox.ValueChanged([&]()
            {
                if (iSettingParameter)
                    return;
                neolib::scoped_flag sf{ iSettingParameter };
                iSlider.set_value(iSpinBox.value());
                ValueChanged.trigger();
            });
            iSlider.ConstraintsChanged([&]()
            {
                if (iSettingParameter)
                    return;
                neolib::scoped_flag sf{ iSettingParameter };
                iSpinBox.set_minimum(iSlider.minimum());
                iSpinBox.set_maximum(iSlider.maximum());
                iSpinBox.set_step(iSlider.step());
                ConstraintsChanged.trigger();
            });
            iSpinBox.ConstraintsChanged([&]()
            {
                if (iSettingParameter)
                    return;
                neolib::scoped_flag sf{ iSettingParameter };
                iSlider.set_minimum(iSpinBox.minimum());
                iSlider.set_maximum(iSpinBox.maximum());
                iSlider.set_step(iSpinBox.step());
                ConstraintsChanged.trigger();
            });
        }
    private:
        horizontal_layout iLayout;
        basic_slider<value_type> iSlider;
        basic_spin_box<value_type> iSpinBox;
        bool iSettingParameter;
    };

    typedef basic_slider_box<int32_t> slider_box;
    typedef basic_slider_box<double> double_slider_box;
}