// button.ipp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/widget/button.hpp>

namespace neogfx
{
    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(alignment aAlignment) :
        base_type{}, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(std::string const& aText, alignment aAlignment) :
        base_type{}, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, aText, label_type::MultiLine, aAlignment }
    {
        init();
    }
    
    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(const i_texture& aTexture, alignment aAlignment) :
        base_type{}, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, aTexture, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(const i_image& aImage, alignment aAlignment) :
        base_type{}, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, aImage, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(std::string const& aText, const i_texture& aTexture, alignment aAlignment) :
        base_type{}, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, aText, aTexture, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(std::string const& aText, const i_image& aImage, alignment aAlignment) :
        base_type{}, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, aText, aImage, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(i_widget& aParent, alignment aAlignment) :
        base_type{ aParent }, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(i_widget& aParent, std::string const& aText, alignment aAlignment) :
        base_type{ aParent }, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, aText, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(i_widget& aParent, const i_texture& aTexture, alignment aAlignment) :
        base_type{ aParent }, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, aTexture, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(i_widget& aParent, const i_image& aImage, alignment aAlignment) :
        base_type{ aParent }, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, aImage, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(i_widget& aParent, std::string const& aText, const i_texture& aTexture, alignment aAlignment) :
        base_type{ aParent }, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, aText, aTexture, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(i_widget& aParent, std::string const& aText, const i_image& aImage, alignment aAlignment) :
        base_type{ aParent }, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, aText, aImage, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(i_layout& aLayout, alignment aAlignment) :
        base_type{ aLayout }, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(i_layout& aLayout, std::string const& aText, alignment aAlignment) :
        base_type{ aLayout }, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, aText, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(i_layout& aLayout, const i_texture& aTexture, alignment aAlignment) :
        base_type{ aLayout }, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, aTexture, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(i_layout& aLayout, const i_image& aImage, alignment aAlignment) :
        base_type{ aLayout }, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, aImage, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(i_layout& aLayout, std::string const& aText, const i_texture& aTexture, alignment aAlignment) :
        base_type{ aLayout }, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, aText, aTexture, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::button(i_layout& aLayout, std::string const& aText, const i_image& aImage, alignment aAlignment) :
        base_type{ aLayout }, iPressed{ false }, iCheckable{ button_checkable::NotCheckable }, iCheckedState{ false }, iLayout{ *this }, iLabel{ iLayout, aText, aImage, label_type::MultiLine, aAlignment }
    {
        init();
    }

    template <typename ButtonInterface>
    inline button<ButtonInterface>::~button()
    {
        service<i_app>().remove_mnemonic(*this);
    }

    template <typename ButtonInterface>
    inline size_policy button<ButtonInterface>::size_policy() const
    {
        if (base_type::has_size_policy())
            return base_type::size_policy();
        else if (base_type::has_fixed_size())
            return size_constraint::Fixed;
        else
            return neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum };
    }

    template <typename ButtonInterface>
    inline size button<ButtonInterface>::maximum_size(optional_size const&) const
    {
        if (base_type::has_maximum_size())
            return base_type::maximum_size();
        auto result = base_type::maximum_size();
        if (size_policy().horizontal_constraint() != size_constraint::Minimum)
            result.cx = size::max_dimension();
        if (size_policy().vertical_constraint() != size_constraint::Minimum)
            result.cy = size::max_dimension();
        return result;
    }

    template <typename ButtonInterface>
    inline padding button<ButtonInterface>::padding() const
    {
        neogfx::padding result = base_type::padding();
        if (!base_type::has_padding() && (label().effective_placement() & label_placement::Text) == label_placement::Text)
        {
            if (base_type::style_sheet_value(".button", "padding", std::vector<length>{}).empty())
            {
                result.left *= 2.0;
                result.right *= 2.0;
            }
        }
        return result;
    }

    template <typename ButtonInterface>
    inline bool button<ButtonInterface>::is_pressed() const
    {
        return iPressed;
    }

    template <typename ButtonInterface>
    inline button_checkable button<ButtonInterface>::checkable() const
    {
        return iCheckable;
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::set_checkable(button_checkable aCheckable)
    {
        iCheckable = aCheckable;
    }

    template <typename ButtonInterface>
    inline bool button<ButtonInterface>::is_checked() const
    {
        return iCheckedState != std::nullopt && *iCheckedState == true;
    }

    template <typename ButtonInterface>
    inline bool button<ButtonInterface>::is_unchecked() const
    {
        return iCheckedState != std::nullopt && *iCheckedState == false;
    }

    template <typename ButtonInterface>
    inline bool button<ButtonInterface>::is_indeterminate() const
    {
        return iCheckedState == std::nullopt;
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::check()
    {
        set_checked_state(true);
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::uncheck()
    {
        set_checked_state(false);
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::set_indeterminate()
    {
        set_checked_state(std::nullopt);
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::set_checked(bool aChecked)
    {
        set_checked_state(aChecked);
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::toggle()
    {
        if (!can_toggle())
            return;
        if (is_checked() || is_indeterminate())
            set_checked(false);
        else
            set_checked(true);
    }

    template <typename ButtonInterface>
    inline i_string const& button<ButtonInterface>::text() const
    {
        return label().text();
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::set_text(i_string const& aText)
    {
        label().set_text(aText);
    }

    template <typename ButtonInterface>
    inline const texture& button<ButtonInterface>::image() const
    {
        return label().image();
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::set_image(i_string const& aImageUri)
    {
        label().set_image(neogfx::image{ aImageUri });
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::set_image(const neogfx::image& aImage)
    {
        label().set_image(aImage);
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::set_image(const texture& aImage)
    {
        label().set_image(aImage);
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::set_image_extents(const optional_size& aImageExtents)
    {
        image_widget().set_fixed_size(aImageExtents);
        image_widget().set_size_policy(aImageExtents ? size_constraint::Fixed : size_constraint::Minimum);
    }

    template <typename ButtonInterface>
    inline const label& button<ButtonInterface>::label() const
    {
        return iLabel;
    }

    template <typename ButtonInterface>
    inline label& button<ButtonInterface>::label()
    {
        return iLabel;
    }

    template <typename ButtonInterface>
    inline const text_widget& button<ButtonInterface>::text_widget() const
    {
        return label().text_widget();
    }

    template <typename ButtonInterface>
    inline text_widget& button<ButtonInterface>::text_widget()
    {
        return label().text_widget();
    }

    template <typename ButtonInterface>
    inline const image_widget& button<ButtonInterface>::image_widget() const
    {
        return label().image_widget();
    }

    template <typename ButtonInterface>
    inline image_widget& button<ButtonInterface>::image_widget()
    {
        return label().image_widget();
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        base_type::mouse_button_clicked(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left)
        {
            as_widget().update();
            iPressed = true;
            Pressed();
        }
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        base_type::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
        if (aButton == mouse_button::Left)
        {
            as_widget().update();
            DoubleClicked();
        }
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        bool wasCapturing = as_widget().capturing();
        base_type::mouse_button_released(aButton, aPosition);
        if (aButton == mouse_button::Left)
        {
            as_widget().update();
            if (wasCapturing)
            {
                destroyed_flag destroyed{ *this };
                iPressed = false;
                if (as_widget().client_rect().contains(aPosition))
                    handle_clicked();
                if (!destroyed)
                    Released();
            }
        }
        else if (aButton == mouse_button::Right)
        {
            if (wasCapturing && as_widget().client_rect().contains(aPosition))
                RightClicked();
        }
    }

    template <typename ButtonInterface>
    inline bool button<ButtonInterface>::key_pressed(scan_code_e aScanCode, key_code_e, key_modifiers_e)
    {
        if (aScanCode == ScanCode_SPACE)
        {
            handle_clicked();
            return true;
        }
        return false;
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::handle_clicked()
    {
        destroyed_flag destroyed{ *this };
        Clicked();
        if (!destroyed && iCheckable != button_checkable::NotCheckable)
            toggle();
    }

    template <typename ButtonInterface>
    inline bool button<ButtonInterface>::can_toggle() const
    {
        return true;
    }

    template <typename ButtonInterface>
    inline const std::optional<bool>& button<ButtonInterface>::checked_state() const
    {
        return iCheckedState;
    }

    template <typename ButtonInterface>
    inline bool button<ButtonInterface>::set_checked_state(const std::optional<bool>& aCheckedState)
    {
        if (iCheckedState == aCheckedState)
            return false;
        if (aCheckedState == std::nullopt && iCheckable != button_checkable::TriState)
            throw not_tri_state_checkable();
        iCheckedState = aCheckedState;
        as_widget().update();
        Toggled();
        if (is_checked())
            Checked();
        else if (is_unchecked())
            Unchecked();
        else if (is_indeterminate())
            Indeterminate();
        return true;
    }

    template <typename ButtonInterface>
    inline std::string button<ButtonInterface>::mnemonic() const
    {
        return mnemonic_from_text(label().text());
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::mnemonic_execute()
    {
        handle_clicked();
    }

    template <typename ButtonInterface>
    inline i_widget& button<ButtonInterface>::mnemonic_widget()
    {
        return label().text_widget();
    }

    template <typename ButtonInterface>
    inline void button<ButtonInterface>::init()
    {
        Pressed.set_trigger_type(trigger_type::Asynchronous);
        Clicked.set_trigger_type(trigger_type::Asynchronous);
        DoubleClicked.set_trigger_type(trigger_type::Asynchronous);
        RightClicked.set_trigger_type(trigger_type::Asynchronous);
        Released.set_trigger_type(trigger_type::Asynchronous);
        Toggled.set_trigger_type(trigger_type::Asynchronous);
        Checked.set_trigger_type(trigger_type::Asynchronous);
        Unchecked.set_trigger_type(trigger_type::Asynchronous);
        Indeterminate.set_trigger_type(trigger_type::Asynchronous);

        as_widget().layout().set_padding(neogfx::padding{});
        as_widget().layout().set_alignment(alignment::Center | alignment::VCenter);
        as_widget().set_focus_policy(focus_policy::TabFocus);

        auto label_text_updated = [this]()
        {
            auto m = mnemonic_from_text(label().text());
            if (!m.empty())
                service<i_app>().add_mnemonic(*this);
            else
                service<i_app>().remove_mnemonic(*this);
        };
        iSink = label().text_widget().TextChanged(label_text_updated);
        label_text_updated();
    }
}


