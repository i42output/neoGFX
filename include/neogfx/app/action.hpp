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
    class action : public i_action
    {
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
        action(std::string const& aText);
        action(std::string const& aText, std::string const& aImageUri, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::Scaled);
        action(std::string const& aText, const i_texture& aImage);
        action(std::string const& aText, const i_image& aImage);
        action(i_action_container& aContainer);
        action(i_action_container& aContainer, std::string const& aText);
        action(i_action_container& aContainer, std::string const& aText, std::string const& aImageUri, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::Scaled);
        action(i_action_container& aContainer, std::string const& aText, const i_texture& aImage);
        action(i_action_container& aContainer, std::string const& aText, const i_image& aImage);
    public:
        bool is_enabled() const override;
        bool is_disabled() const override;
        bool is_checkable() const override;
        bool is_checked() const override;
        bool is_unchecked() const override;
        uint32_t group() const override;
        bool is_separator() const override;
        std::string text() const override;
        std::string menu_text() const override;
        std::string button_text() const override;
        std::string tool_tip_text() const override;
        std::string help_text() const override;
        const i_texture& image() const override;
        const i_texture& checked_image() const override;
        const optional_key_sequence& shortcut() const override;
        action& set_enabled(bool aEnabled) override;
        action& set_checkable(bool aCheckable) override;
        action& set_checked(bool aChecked) override;
        action& set_group(uint32_t aGroup) override;
        action& set_separator(bool aIsSeparator) override;
        action& set_text(const optional_text& aText = optional_text()) override;
        action& set_menu_text(const optional_text& aMenuText = optional_text()) override;
        action& set_button_text(const optional_text& aButtonText = optional_text()) override;
        action& set_tool_tip_text(const optional_text& aToolTipText = optional_text()) override;
        action& set_help_text(const optional_text& aHelpText = optional_text()) override;
        action& set_image(std::string const& aUri, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::Scaled) override;
        action& set_image(const i_image& aImage) override;
        action& set_image(const i_texture& aTexture) override;
        action& set_checked_image(std::string const& aUri, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::Scaled) override;
        action& set_checked_image(const i_image& aImage) override;
        action& set_checked_image(const i_texture& aTexture) override;
        action& set_shortcut(const optional_key_sequence& aShortcut) override;
        action& set_shortcut(std::string const& aShortcut) override;
    private:
        bool iEnabled;
        bool iCheckable;
        bool iChecked;
        uint32_t iGroup;
        bool iSeparator;
        optional_text iText;
        optional_text iMenuText;
        optional_text iButtonText;
        optional_text iToolTipText;
        optional_text iHelpText;
        texture iImage;
        texture iCheckedTexture;
        optional_key_sequence iShortcut;
    };

    namespace nrc
    {
        class action_ref
        {
        public:
            action_ref(i_action_container& aContainer)
            {
                aContainer.add_action(std::make_shared<action>());
            }
            action_ref(i_action_container& aContainer, i_action& aAction)
            {
                aContainer.add_action(aAction);
            }
        };
    }
}