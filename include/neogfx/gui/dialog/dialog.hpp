// dialog.hpp
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
        meta_object(window)
    public:
        define_event(TryAccept, try_accept, bool& /* aCanAccept */, bool /* aQueryOnly */)
        define_event(TryReject, try_reject, bool& /* aCanReject */, bool /* aQueryOnly */)
        define_event(Accepted, accepted)
        define_event(Rejected, rejected)
        define_event(HaveResult, have_result, dialog_result /* aResult */)
    public:
        dialog(window_style aStyle = window_style::DefaultDialog);
        dialog(std::string const& aDialogTitle, window_style aStyle = window_style::DefaultDialog);
        dialog(const size& aDimensions, window_style aStyle = window_style::DefaultDialog);
        dialog(const size& aDimensions, std::string const& aDialogTitle, window_style aStyle = window_style::DefaultDialog);
        dialog(const point& aPosition, const size& aDimensions, window_style aStyle = window_style::DefaultDialog);
        dialog(const point& aPosition, const size& aDimensions, std::string const& aDialogTitle, window_style aStyle = window_style::DefaultDialog);
        dialog(i_widget& aParent, window_style aStyle = window_style::DefaultDialog);
        dialog(i_widget& aParent, std::string const& aDialogTitle, window_style aStyle = window_style::DefaultDialog);
        dialog(i_widget& aParent, const size& aDimensions, window_style aStyle = window_style::DefaultDialog);
        dialog(i_widget& aParent, const size& aDimensions, std::string const& aDialogTitle, window_style aStyle = window_style::DefaultDialog);
        dialog(i_widget& aParent, const point& aPosition, const size& aDimensions, window_style aStyle = window_style::DefaultDialog);
        dialog(i_widget& aParent, const point& aPosition, const size& aDimensions, std::string const& aDialogTitle, window_style aStyle = window_style::DefaultDialog);
        ~dialog();
    public:
        virtual void accept();
        virtual void reject();
        virtual dialog_result result() const;
        virtual void set_result(dialog_result aResult);
    public:
        void set_standard_layout(const size& aControlSpacing, const neogfx::padding& aPadding, bool aCreateButtonBox = true);
        dialog_button_box& button_box();
    public:
        virtual dialog_result exec();
    public:
        neogfx::size_policy size_policy() const override;
    public:
        bool can_close() const override;
        void close() override;
    public:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifier aKeyModifier) override;
    public:
        using window::has_layout;   
        using window::layout;
        bool has_layout(standard_layout aStandardLayout) const override;
        const i_layout& layout(standard_layout aStandardLayout, layout_position aPosition = layout_position::None) const override;
        i_layout& layout(standard_layout aStandardLayout, layout_position aPosition = layout_position::None)  override;
    private:
        void init();
    private:
        std::optional<widget_timer> iUpdater;
        vertical_layout iButtonBoxLayout;
        std::optional<dialog_button_box> iButtonBox;
        std::optional<dialog_result> iResult;
        sink iSink;
    };
}