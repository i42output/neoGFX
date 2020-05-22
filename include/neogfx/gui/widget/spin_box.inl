// spin_box.inl
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

#include "spin_box.hpp"
#include <cmath>
#include <boost/format.hpp>
#include <neolib/core/scoped.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/hid/i_surface_manager.hpp>

namespace neogfx
{

    template <typename T>
    basic_spin_box<T>::basic_spin_box() :
        iPrimaryLayout{ *this },
        iTextBox{ iPrimaryLayout },
        iSecondaryLayout{ iPrimaryLayout },
        iStepUpButton{ iSecondaryLayout, std::string{}, push_button_style::SpinBox },
        iStepDownButton{ iSecondaryLayout, std::string{}, push_button_style::SpinBox },
        iMinimum{}, 
        iMaximum{}, 
        iStep{}, 
        iValue{}, 
        iFormat{ "%1%" }
    {
        init();
    }

    template <typename T>
    basic_spin_box<T>::basic_spin_box(i_widget& aParent) :
        framed_widget{ aParent },
        iPrimaryLayout{ *this },
        iTextBox{ iPrimaryLayout },
        iSecondaryLayout{ iPrimaryLayout },
        iStepUpButton{ iSecondaryLayout, std::string{}, push_button_style::SpinBox },
        iStepDownButton{ iSecondaryLayout, std::string{}, push_button_style::SpinBox },
        iMinimum{},
        iMaximum{},
        iStep{},
        iValue{},
        iFormat{ "%1%" }
    {
        init();
    }

    template <typename T>
    basic_spin_box<T>::basic_spin_box(i_layout& aLayout) :
        framed_widget{ aLayout },
        iPrimaryLayout{ *this },
        iTextBox{ iPrimaryLayout },
        iSecondaryLayout{ iPrimaryLayout },
        iStepUpButton{ iSecondaryLayout, std::string{}, push_button_style::SpinBox },
        iStepDownButton{ iSecondaryLayout, std::string{}, push_button_style::SpinBox },
        iMinimum{},
        iMaximum{},
        iStep{},
        iValue{},
        iFormat{ "%1%" }
    {
        init();
    }

    template <typename T>
    neogfx::size_policy basic_spin_box<T>::size_policy() const
    {
        if (framed_widget::has_size_policy())
            return framed_widget::size_policy();
        return neogfx::size_policy{ text_box().size_hint() ? size_constraint::Minimum : size_constraint::Expanding, size_constraint::Minimum };
    }

    template <typename T>
    color basic_spin_box<T>::frame_color() const
    {
        if (service<i_app>().current_style().palette().color(color_role::Theme).similar_intensity(background_color(), 0.03125))
            return framed_widget::frame_color();
        return service<i_app>().current_style().palette().color(color_role::Theme).mid(background_color());
    }

    template <typename T>
    void basic_spin_box<T>::mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifiers_e aKeyModifiers)
    {
        if (aWheel == mouse_wheel::Vertical)
            do_step(aDelta.dy > 0.0 ? step_direction::Up : step_direction::Down, static_cast<uint32_t>(std::abs(aDelta.dy)));
        else
            framed_widget::mouse_wheel_scrolled(aWheel, aPosition, aDelta, aKeyModifiers);
    }

    template <typename T>
    bool basic_spin_box<T>::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        if (aScanCode == ScanCode_UP)
        {
            do_step(step_direction::Up);
            return true;
        }
        else if (aScanCode == ScanCode_DOWN)
        {
            do_step(step_direction::Down);
            return true;
        }
        else
            return framed_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
    }

    template <typename T>
    const std::string& basic_spin_box<T>::text()
    {
        return iText;
    }

    template <typename T>
    const line_edit& basic_spin_box<T>::text_box() const
    {
        return iTextBox;
    }

    template <typename T>
    line_edit& basic_spin_box<T>::text_box()
    {
        return iTextBox;
    }

    template <typename T>
    void basic_spin_box<T>::do_step(step_direction aDirection, uint32_t aAmount)
    {
        auto result = std::max(minimum(), std::min(maximum(), aDirection == step_direction::Up ? value() + static_cast<value_type>(aAmount * step()) : value() - static_cast<value_type>(aAmount * step())));
        if ((aDirection == step_direction::Up && result > value()) || (aDirection == step_direction::Down && result < value()))
            set_value(result, true);
    };

    template <typename T>
    inline std::optional<T> basic_spin_box<T>::string_to_value(const std::string& aText) const
    {
        if (aText.empty())
            return std::optional<value_type>{};
        value_type result = minimum();
        if (maximum() == minimum())
            return result;
        if (aText == "-")
            return result;
        std::istringstream iss(aText);
        if (!(iss >> result))
            return std::optional<value_type>{};
        std::string guff;
        if (iss >> guff)
            return std::optional<value_type>{};
        return result;
    }

    template <typename T>
    void basic_spin_box<T>::init()
    {
        set_margins(neogfx::margins{});
        iPrimaryLayout.set_margins(neogfx::margins{});
        iSecondaryLayout.set_margins(neogfx::margins{});
        iSecondaryLayout.set_spacing(size{});
        iStepUpButton.set_margins(neogfx::margins{});
        iStepUpButton.set_minimum_size(dip(SPIN_BUTTON_MINIMUM_SIZE));
        iStepUpButton.label().image_widget().set_minimum_size(size{ 3.0, 3.0 });
        iStepUpButton.label().image_widget().set_snap(2.0); // up and down buttons want to draw arrow texture at same size so use a snap of 2 pixels
        iStepUpButton.set_size_policy(neogfx::size_policy{ size_constraint::Minimum, size_constraint::Expanding });
        iStepUpButton.Clicked.set_trigger_type(event_trigger_type::Synchronous);
        iStepUpButton.DoubleClicked.set_trigger_type(event_trigger_type::Synchronous);
        iStepDownButton.set_margins(neogfx::margins{});
        iStepDownButton.set_minimum_size(dip(SPIN_BUTTON_MINIMUM_SIZE));
        iStepDownButton.label().image_widget().set_minimum_size(size{ 3.0, 3.0 });
        iStepDownButton.label().image_widget().set_snap(2.0);
        iStepDownButton.set_size_policy(neogfx::size_policy{ size_constraint::Minimum, size_constraint::Expanding });
        iStepDownButton.Clicked.set_trigger_type(event_trigger_type::Synchronous);
        iStepDownButton.DoubleClicked.set_trigger_type(event_trigger_type::Synchronous);
        iTextBox.set_frame_style(frame_style::NoFrame);

        iSink += iTextBox.TextFilter([this](const std::string& aText, bool& aAccept)
        {
            aAccept = aText.find_first_not_of(valid_text_characters()) == std::string::npos;
            if (!aAccept)
                service<i_basic_services>().system_beep();
        });

        iSink += iTextBox.TextChanged([this]()
        {
            auto text = iTextBox.text();
            auto result = string_to_value(text);
            if (result != std::nullopt)
            {
                neolib::scoped_flag sf{ iDontSetText };
                iText = text;
                iTextCursorPos = iTextBox.cursor().position();
                set_value(std::min(maximum(), std::max(minimum(), *result)));
            }
            else if (!text.empty())
            {
                iTextBox.set_text(iText);
                iTextBox.cursor().set_position(iTextCursorPos);
                service<i_basic_services>().system_beep();
            }
            else
            {
                neolib::scoped_flag sf{ iDontSetText };
                iText = text;
                iTextCursorPos = iTextBox.cursor().position();
                set_value(minimum());
            }
        });

        auto step_up = [this]()
        {
            do_step(step_direction::Up);
            iStepper.emplace(service<async_task>(), [this](neolib::callback_timer& aTimer)
            {
                aTimer.set_duration(125, true);
                aTimer.again();
                do_step(step_direction::Up);
            }, 500);
        };
        iSink += iStepUpButton.Pressed(step_up);
        iSink += iStepUpButton.clicked([this]()
        {
            if (iStepper == std::nullopt) // key press?
                do_step(step_direction::Up);
        });
        iSink += iStepUpButton.DoubleClicked(step_up);
        iSink += iStepUpButton.Released([this]()
        {
            iStepper = std::nullopt;
        });

        auto step_down = [this]()
        {
            do_step(step_direction::Down);
            iStepper.emplace(service<async_task>(), [this](neolib::callback_timer& aTimer)
            {
                aTimer.set_duration(125, true);
                aTimer.again();
                do_step(step_direction::Down);
            }, 500);
        };
        iSink += iStepDownButton.Pressed(step_down);
        iSink += iStepDownButton.clicked([this]()
        {
            if (iStepper == std::nullopt) // key press?
                do_step(step_direction::Down);
        });
        iSink += iStepDownButton.DoubleClicked(step_down);
        iSink += iStepDownButton.Released([this]()
        {
            iStepper = std::nullopt;
        });

        update_arrows();
        iSink += service<i_app>().current_style_changed([this](style_aspect aAspect)
        {
            if ((aAspect & style_aspect::Color) == style_aspect::Color)
                update_arrows();
        });

        iSink += service<i_surface_manager>().dpi_changed([this](i_surface&) { update_arrows(); });
    }

    template <typename T>
    void basic_spin_box<T>::update_arrows()
    {
        auto ink = service<i_app>().current_style().palette().color(color_role::Text);
        const char* sUpArrowImagePattern
        {
            "[9,5]"
            "{0,paper}"
            "{1,ink}"

            "000010000"
            "000111000"
            "001111100"
            "011111110"
            "111111111"
        };
        const char* sUpArrowHighDpiImagePattern
        {
            "[18,9]"
            "{0,paper}"
            "{1,ink}"

            "000000001100000000"
            "000000011110000000"
            "000000111111000000"
            "000001111111100000"
            "000011111111110000"
            "000111111111111000"
            "001111111111111100"
            "011111111111111110"
            "111111111111111111"
        };
        iUpArrow.emplace(ink, !high_dpi() ?
            image{ "neogfx::basic_spin_box<T>::iUpArrow::" + ink.to_string(), sUpArrowImagePattern,{ { "paper", color{} },{ "ink", ink } } } :
            image{ "neogfx::basic_spin_box<T>::iUpArrowHighDpi::" + ink.to_string(), sUpArrowHighDpiImagePattern,{ { "paper", color{} },{ "ink", ink } }, 2.0 });
        const char* sDownArrowImagePattern
        {
            "[9,5]"
            "{0,paper}"
            "{1,ink}"

            "111111111"
            "011111110"
            "001111100"
            "000111000"
            "000010000"
        };
        const char* sDownArrowHighDpiImagePattern
        {
            "[18,9]"
            "{0,paper}"
            "{1,ink}"

            "111111111111111111"
            "011111111111111110"
            "001111111111111100"
            "000111111111111000"
            "000011111111110000"
            "000001111111100000"
            "000000111111000000"
            "000000011110000000"
            "000000001100000000"
        };
        iDownArrow.emplace(ink, !high_dpi() ?
            image{ "neogfx::basic_spin_box<T>::iDownArrow::" + ink.to_string(), sDownArrowImagePattern,{ { "paper", color{} },{ "ink", ink } } } :
            image{ "neogfx::basic_spin_box<T>::iDownArrowHighDpi::" + ink.to_string(), sDownArrowHighDpiImagePattern,{ { "paper", color{} },{ "ink", ink } }, 2.0 });
        iStepUpButton.label().set_placement(label_placement::ImageVertical);
        iStepDownButton.label().set_placement(label_placement::ImageVertical);
        iStepUpButton.set_image(iUpArrow->second);
        iStepDownButton.set_image(iDownArrow->second);
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
        ConstraintsChanged.trigger();
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
        ConstraintsChanged.trigger();
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
        ConstraintsChanged.trigger();
    }

    template <typename T>
    inline typename basic_spin_box<T>::value_type basic_spin_box<T>::value() const
    {
        return iValue;
    }

    template <typename T>
    inline void basic_spin_box<T>::set_value(value_type aValue, bool aNotify)
    {
        aValue = std::max(minimum(), std::min(maximum(), aValue));
        if (iValue != aValue)
        {
            iValue = aValue;
            if (!iDontSetText)
                iTextBox.set_text(value_to_string());
            if (aNotify && (!text().empty() || value() != minimum()))
                ValueChanged.trigger();
        }
    }

    template <typename T>
    inline const std::string& basic_spin_box<T>::format() const
    {
        return iFormat;
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
    inline std::string basic_spin_box<T>::value_to_string() const
    {
        std::string text;
        try { text = boost::str(boost::format(iFormat) % value()); } catch (...) {}
        return text;
    }
}
