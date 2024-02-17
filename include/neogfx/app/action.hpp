// action.hpp
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

#include <neogfx/gfx/texture.hpp>
#include <neogfx/app/i_action.hpp>
#include <neogfx/gui/widget/i_menu.hpp>

namespace neogfx
{
    class action : public reference_counted<i_action>
    {
    public:
        typedef i_action abstract_type;
    public:
        define_declared_event(Triggered, triggered)
        define_declared_event(Hovered, hovered)
        define_declared_event(Enabled, enabled)
        define_declared_event(Disabled, disabled)
        define_declared_event(Checked, checked)
        define_declared_event(Unchecked, unchecked)
        define_declared_event(Changed, changed)
    public:
        action();
        action(string const& aText);
        action(string const& aText, string const& aImageUri, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::Scaled);
        action(string const& aText, const i_texture& aImage);
        action(string const& aText, const i_image& aImage);
        action(i_action_container& aContainer);
        action(i_action_container& aContainer, string const& aText);
        action(i_action_container& aContainer, string const& aText, string const& aImageUri, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::Scaled);
        action(i_action_container& aContainer, string const& aText, const i_texture& aImage);
        action(i_action_container& aContainer, string const& aText, const i_image& aImage);
    public:
        bool is_enabled() const override;
        bool is_disabled() const override;
        bool is_checkable() const override;
        bool is_checked() const override;
        bool is_unchecked() const override;
        uuid const& group() const override;
        bool is_separator() const override;
        i_string const& text() const override;
        i_string const& menu_text() const override;
        i_string const& button_text() const override;
        i_string const& tool_tip_text() const override;
        i_string const& help_text() const override;
        const i_texture& image() const override;
        const i_texture& checked_image() const override;
        const optional_key_sequence& shortcut() const override;
        action& set_enabled(bool aEnabled) override;
        action& set_checkable(bool aCheckable = true) override;
        action& set_checked(bool aChecked) override;
        action& set_group(uuid const& aGroup) override;
        action& set_separator(bool aIsSeparator) override;
        action& set_text(i_optional<i_string> const& aText = optional_text()) override;
        action& set_menu_text(i_optional<i_string> const& aMenuText = optional_text()) override;
        action& set_button_text(i_optional<i_string> const& aButtonText = optional_text()) override;
        action& set_tool_tip_text(i_optional<i_string> const& aToolTipText = optional_text()) override;
        action& set_help_text(i_optional<i_string> const& aHelpText = optional_text()) override;
        action& set_image(const i_texture& aTexture) override;
        action& set_checked_image(const i_texture& aTexture) override;
        action& set_shortcut(const optional_key_sequence& aShortcut) override;
        action& set_shortcut(i_string const& aShortcut) override;
    public:
        using i_action::set_text;
        using i_action::set_menu_text;
        using i_action::set_button_text;
        using i_action::set_tool_tip_text;
        using i_action::set_help_text;
        using i_action::set_image;
        using i_action::set_checked_image;
        using i_action::set_shortcut;
    private:
        bool iEnabled;
        bool iCheckable;
        bool iChecked;
        uuid iGroup;
        bool iSeparator;
        optional_text iText;
        optional_text iMenuText;
        optional_text iButtonText;
        optional_text iToolTipText;
        optional_text iHelpText;
        texture iImage;
        texture iCheckedImage;
        optional_key_sequence iShortcut;
    };

    namespace nrc
    {
        class action_ref
        {
        public:
            action_ref(i_action_container& aContainer)
            {
                aContainer.add_action(make_ref<action>());
            }
            action_ref(i_action_container& aContainer, i_action& aAction)
            {
                aContainer.add_action(aAction);
            }
        };
    }
}