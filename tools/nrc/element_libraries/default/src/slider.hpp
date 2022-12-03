// slider.hpp
/*
neoGFX Resource Compiler
Copyright(C) 2019 Leigh Johnston

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
#include <neogfx/tools/nrc/ui_element.hpp>

namespace neogfx::nrc
{
    template <typename T, ui_element_type SliderType>
    class basic_slider : public ui_element<>
    {
    public:
        basic_slider(const i_ui_element_parser& aParser, i_ui_element& aParent) :
            ui_element<>{ aParser, aParent, SliderType }
        {
            add_header("neogfx/gui/widget/slider.hpp");
            add_data_names({ "orientation", "minimum", "maximum", "step", "value" });
        }
    public:
        void parse(const neolib::i_string& aName, const data_t& aData) override
        {
            ui_element<>::parse(aName, aData);
            if (aName == "orientation")
                iOrientation = get_enum<slider_orientation>(aData);
            else if (aName == "minimum")
                iMinimum = get_scalar<T>(aData);
            else if (aName == "maximum")
                iMaximum = get_scalar<T>(aData);
            else if (aName == "step")
                iStep = get_scalar<T>(aData);
            else if (aName == "value")
                iValue = get_scalar<T>(aData);
        }
        void parse(const neolib::i_string& aName, const array_data_t& aData) override
        {
            ui_element<>::parse(aName, aData);
        }
    protected:
        void emit() const override
        {
        }
        void emit_preamble() const override
        {
            emit("  %1% %2%;\n", type_name(), id());
            ui_element<>::emit_preamble();
        }
        void emit_ctor() const override
        {
            if (iOrientation)
                ui_element<>::emit_generic_ctor("slider_orientation", *iOrientation);
            else
                ui_element<>::emit_generic_ctor();
            ui_element<>::emit_ctor();
        }
        void emit_body() const override
        {
            ui_element<>::emit_body();
            if (iMinimum)
                emit("   %1%.set_minimum(%2%);\n", id(), *iMinimum);
            if (iMaximum)
                emit("   %1%.set_maximum(%2%);\n", id(), *iMaximum);
            if (iStep)
                emit("   %1%.set_step(%2%);\n", id(), *iStep);
            if (iValue)
                emit("   %1%.set_value(%2%);\n", id(), *iValue);
        }
    protected:
        using ui_element<>::emit;
    private:
        neolib::optional<slider_orientation> iOrientation;
        neolib::optional<T> iMinimum;
        neolib::optional<T> iMaximum;
        neolib::optional<T> iStep;
        neolib::optional<T> iValue;
    };

    typedef basic_slider<int32_t, ui_element_type::Slider> slider;
    typedef basic_slider<double, ui_element_type::DoubleSlider> double_slider;
}
