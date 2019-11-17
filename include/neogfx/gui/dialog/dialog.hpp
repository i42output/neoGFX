// dialog.hpp
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
#include <neogfx/gui/window/window.hpp>
#include "dialog_button_box.hpp"

namespace neogfx
{
    enum class dialog_result
    {
        NoResult,
        Accepted,
        Rejected,
        Other
    };

    class dialog : public window
    {
    public:
        define_event(TryAccept, try_accept, bool&)
        define_event(TryReject, try_reject, bool&)
    public:
        dialog(window_style aStyle = window_style::Default);
        dialog(const std::string& aDialogTitle, window_style aStyle = window_style::Default);
        dialog(const size& aDimensions, window_style aStyle = window_style::Default);
        dialog(const size& aDimensions, const std::string& aDialogTitle, window_style aStyle = window_style::Default);
        dialog(const point& aPosition, const size& aDimensions, window_style aStyle = window_style::Default);
        dialog(const point& aPosition, const size& aDimensions, const std::string& aDialogTitle, window_style aStyle = window_style::Default);
        dialog(i_widget& aParent, window_style aStyle = window_style::Default);
        dialog(i_widget& aParent, const std::string& aDialogTitle, window_style aStyle = window_style::Default);
        dialog(i_widget& aParent, const size& aDimensions, window_style aStyle = window_style::Default);
        dialog(i_widget& aParent, const size& aDimensions, const std::string& aDialogTitle, window_style aStyle = window_style::Default);
        dialog(i_widget& aParent, const point& aPosition, const size& aDimensions, window_style aStyle = window_style::Default);
        dialog(i_widget& aParent, const point& aPosition, const size& aDimensions, const std::string& aDialogTitle, window_style aStyle = window_style::Default);
        ~dialog();
    public:
        virtual void accept();
        virtual void reject();
        virtual dialog_result result() const;
        virtual void set_result(dialog_result aResult);
    public:
        size set_standard_layout(const size& aControlSpacing, bool aCreateButtonBox = true, bool aDpiScaling = true);
        dialog_button_box& button_box();
    public:
        virtual dialog_result exec();
    public:
        neogfx::size_policy size_policy() const override;
    public:
        bool can_close() const override;
    public:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
    public:
        const i_layout& client_layout() const override;
        i_layout& client_layout() override;
        virtual const i_layout& button_box_layout() const;
        virtual i_layout& button_box_layout();
    private:
        void init();
    private:
        vertical_layout iClientLayout;
        vertical_layout iButtonBoxLayout;
        std::optional<dialog_button_box> iButtonBox;
        std::optional<dialog_result> iResult;
    };
}