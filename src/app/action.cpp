// action.cpp
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

#include <neogfx/neogfx.hpp>

#include <neogfx/gfx/image.hpp>
#include <neogfx/app/action.hpp>

namespace neogfx
{
    action::action() :
        iEnabled{ false }, iCheckable{ false }, iChecked{ false }, iGroup{}, iSeparator{ true }
    {
        Triggered.set_trigger_type(trigger_type::Asynchronous);
    }

    action::action(string const& aText) :
        iEnabled{ true }, iCheckable{ false }, iChecked{ false }, iGroup{}, iSeparator{ false }, iText{ aText }
    {
        Triggered.set_trigger_type(trigger_type::Asynchronous);
    }

    action::action(string const& aText, string const& aImageUri, dimension aDpiScaleFactor, texture_sampling aSampling) :
        iEnabled{ true }, iCheckable{ false }, iChecked{ false }, iGroup{}, iSeparator{ false }, iText{ aText }, iImage{ neogfx::image{ aImageUri, aDpiScaleFactor, aSampling } }
    {
        Triggered.set_trigger_type(trigger_type::Asynchronous);
    }

    action::action(string const& aText, const i_texture& aImage) :
        iEnabled{ true }, iCheckable{ false }, iChecked{ false }, iGroup{}, iSeparator{ false }, iText{ aText }, iImage{ aImage }
    {
        Triggered.set_trigger_type(trigger_type::Asynchronous);
    }

    action::action(string const& aText, const i_image& aImage) :
        iEnabled{ true }, iCheckable{ false }, iChecked{ false }, iGroup{}, iSeparator{ false }, iText{ aText }, iImage{ aImage }
    {
        Triggered.set_trigger_type(trigger_type::Asynchronous);
    }

    action::action(i_action_container& aContainer) :
        action{}
    {
        aContainer.add_action(*this);
    }

    action::action(i_action_container& aContainer, string const& aText) :
        action{ aText }
    {
        aContainer.add_action(*this);
    }

    action::action(i_action_container& aContainer, string const& aText, string const& aImageUri, dimension aDpiScaleFactor, texture_sampling aSampling) :
        action{ aText, aImageUri, aDpiScaleFactor, aSampling }
    {
        aContainer.add_action(*this);
    }

    action::action(i_action_container& aContainer, string const& aText, const i_texture& aImage) :
        action{ aText, aImage }
    {
        aContainer.add_action(*this);
    }

    action::action(i_action_container& aContainer, string const& aText, const i_image& aImage) :
        action{ aText, aImage }
    {
        aContainer.add_action(*this);
    }

    bool action::is_enabled() const
    {
        return iEnabled;
    }

    bool action::is_disabled() const
    {
        return !iEnabled;
    }

    bool action::is_checkable() const
    {
        return iCheckable;
    }

    bool action::is_checked() const
    {
        return iChecked;
    }

    bool action::is_unchecked() const
    {
        return !iChecked;
    }

    uuid const& action::group() const
    {
        return iGroup;
    }

    bool action::is_separator() const
    {
        return iSeparator;
    }

    i_string const& action::text() const
    {
        if (iText != std::nullopt)
            return *iText;
        else if (iMenuText != std::nullopt)
            return *iMenuText;
        else if (iButtonText != std::nullopt)
            return *iButtonText;
        else
            return empty_string;;
    }

    i_string const& action::menu_text() const
    {
        if (iMenuText != std::nullopt)
            return *iMenuText;
        else if (iText != std::nullopt)
            return *iText;
        else if (iButtonText != std::nullopt)
            return *iButtonText;
        else
            return empty_string;;
    }

    i_string const& action::button_text() const
    {
        if (iButtonText != std::nullopt)
            return *iButtonText;
        else if (iText != std::nullopt)
            return *iText;
        else if (iMenuText != std::nullopt)
            return *iMenuText;
        else
            return empty_string;;
    }

    i_string const& action::tool_tip_text() const
    {
        if (iToolTipText != std::nullopt)
            return *iToolTipText;
        else if (iText != std::nullopt)
            return *iText;
        else if (iMenuText != std::nullopt)
            return *iMenuText;
        else if (iButtonText != std::nullopt)
            return *iButtonText;
        else
            return empty_string;;
    }

    i_string const& action::help_text() const
    {
        if (iHelpText != std::nullopt)
            return *iToolTipText;
        else if (iText != std::nullopt)
            return *iText;
        else if (iToolTipText != std::nullopt)
            return *iToolTipText;
        else if (iMenuText != std::nullopt)
            return *iMenuText;
        else if (iButtonText != std::nullopt)
            return *iButtonText;
        else
            return empty_string;;
    }

    const i_texture& action::image() const
    {
        return iImage;
    }

    const i_texture& action::checked_image() const
    {
        if (!iCheckedImage.is_empty())
            return iCheckedImage;
        return image();
    }

    const optional_key_sequence& action::shortcut() const
    {
        return iShortcut;
    }

    action& action::set_enabled(bool aEnabled)
    {
        if (iEnabled != aEnabled)
        {
            iEnabled = aEnabled;
            if (is_enabled())
                Enabled();
            else
                Disabled();
        }
        return *this;
    }

    action& action::set_checkable(bool aCheckable)
    {
        if (iCheckable != aCheckable)
        {
            iCheckable = aCheckable;
            Changed();
        }
        return *this;
    }

    action& action::set_checked(bool aChecked)
    {
        if (iChecked != aChecked)
        {
            iChecked = aChecked;
            if (is_checked())
                Checked();
            else
                Unchecked();
        }
        return *this;
    }

    action& action::set_group(uuid const& aGroup)
    {
        if (iGroup != aGroup)
        {
            iGroup = aGroup;
            Changed();
        }
        return *this;
    }

    action& action::set_separator(bool aIsSeparator)
    {
        if (iSeparator != aIsSeparator)
        {
            iSeparator = aIsSeparator;
            Changed();
        }
        return *this;
    }

    action& action::set_text(i_optional<i_string> const& aText)
    {
        if (iText != aText)
        {
            iText = aText;
            if (iText != std::nullopt && iSeparator)
                iSeparator = false;
            Changed();
        }
        return *this;
    }

    action& action::set_menu_text(i_optional<i_string> const& aMenuText)
    {
        if (iMenuText != aMenuText)
        {
            iMenuText = aMenuText;
            if (iMenuText != std::nullopt && iSeparator)
                iSeparator = false;
            Changed();
        }
        return *this;
    }

    action& action::set_button_text(i_optional<i_string> const& aButtonText)
    {
        if (iButtonText != aButtonText)
        {
            iButtonText = aButtonText;
            if (iButtonText != std::nullopt && iSeparator)
                iSeparator = false;
            Changed();
        }
        return *this;
    }

    action& action::set_tool_tip_text(i_optional<i_string> const& aToolTipText)
    {
        if (iToolTipText != aToolTipText)
        {
            iToolTipText = aToolTipText;
            if (iToolTipText != std::nullopt && iSeparator)
                iSeparator = false;
            Changed();
        }
        return *this;
    }

    action& action::set_help_text(i_optional<i_string> const& aHelpText)
    {
        if (iHelpText != aHelpText)
        {
            iHelpText = aHelpText;
            if (iHelpText != std::nullopt && iSeparator)
                iSeparator = false;
            Changed();
        }
        return *this;
    }

    action& action::set_image(const i_texture& aTexture)
    {
        iImage = aTexture;
        if (!iImage.is_empty() && iSeparator)
            iSeparator = false;
        Changed();
        return *this;
    }

    action& action::set_checked_image(const i_texture& aTexture)
    {
        iCheckedImage = aTexture;
        if (!iCheckedImage.is_empty() && iSeparator)
            iSeparator = false;
        Changed();
        return *this;
    }

    action& action::set_shortcut(const optional_key_sequence& aShortcut)
    {
        if (iShortcut != aShortcut)
        {
            iShortcut = aShortcut;
            if (iShortcut != std::nullopt && iSeparator)
                iSeparator = false;
            Changed();
        }
        return *this;
    }

    action& action::set_shortcut(i_string const& aShortcut)
    {
        return set_shortcut(key_sequence{ aShortcut });
    }
}
 