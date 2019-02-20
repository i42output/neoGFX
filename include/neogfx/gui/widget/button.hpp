// button.hpp
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
#include <neogfx/core/event.hpp>
#include <neogfx/app/i_mnemonic.hpp>
#include <neogfx/gui/widget/i_button.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>

namespace neogfx
{
    class button : public widget, public virtual i_button, protected i_mnemonic
    {
    public:
        event<> pressed;
        event<> clicked;
        event<> async_clicked;
        event<> double_clicked;
        event<> right_clicked;
        event<> released;
    public:
        event<> toggled;
        event<> checked;
        event<> unchecked;
        event<> indeterminate;
    public:
        struct not_tri_state_checkable : public std::logic_error { not_tri_state_checkable() : std::logic_error("neogfx::button::not_tri_state_checkable") {} };
        // button
    public:
        button(alignment aAlignment = alignment::Left | alignment::VCentre);
        button(const std::string& aText, alignment aAlignment = alignment::Left | alignment::VCentre);
        button(const i_texture& aTexture, alignment aAlignment = alignment::Left | alignment::VCentre);
        button(const i_image& aImaget, alignment aAlignment = alignment::Left | alignment::VCentre);
        button(i_widget& aParent, alignment aAlignment = alignment::Left | alignment::VCentre);
        button(i_widget& aParent, const std::string& aText, alignment aAlignment = alignment::Left | alignment::VCentre);
        button(i_widget& aParent, const i_texture& aTexture, alignment aAlignment = alignment::Left | alignment::VCentre);
        button(i_widget& aParent, const i_image& aImage, alignment aAlignment = alignment::Left | alignment::VCentre);
        button(i_layout& aLayout, alignment aAlignment = alignment::Left | alignment::VCentre);
        button(i_layout& aLayout, const std::string& aText, alignment aAlignment = alignment::Left | alignment::VCentre);
        button(i_layout& aLayout, const i_texture& aTexture, alignment aAlignment = alignment::Left | alignment::VCentre);
        button(i_layout& aLayout, const i_image& aImage, alignment aAlignment = alignment::Left | alignment::VCentre);
        ~button();
        // widget
    public:
        neogfx::size_policy size_policy() const override;
        neogfx::margins margins() const override;
        // i_button
    public:
        button_checkable checkable() const override;
        void set_checkable(button_checkable aCheckable = button_checkable::BiState) override;
        bool is_checked() const override;
        bool is_unchecked() const override;
        bool is_indeterminate() const override;
        void check() override;
        void uncheck() override;
        void set_indeterminate() override;
        void set_checked(bool aChecked) override;
        void toggle() override;
        // button
    public:
        const neogfx::label& label() const;
        neogfx::label& label();
        const image_widget& image() const;
        image_widget& image();
        const text_widget& text() const;
        text_widget& text();
        // widget
    protected:
        void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_released(mouse_button aButton, const point& aPosition) override;
    protected:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
        // button
    protected:
        virtual void handle_clicked();
    protected:
        virtual bool can_toggle() const;
        virtual const std::optional<bool>& checked_state() const;
        virtual bool set_checked_state(const std::optional<bool>& aCheckedState);
    protected:
        std::string mnemonic() const override;
        void mnemonic_execute() override;
        i_widget& mnemonic_widget() override;
    private:
        void init();
    private:
        sink iSink;
        button_checkable iCheckable;
        std::optional<bool> iCheckedState;
        horizontal_layout iLayout;
        neogfx::label iLabel;
    };
}