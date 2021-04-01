// i_action.hpp
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
#include <optional>
#include <neogfx/core/event.hpp>
#include <neogfx/hid/keyboard.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/gfx/image.hpp>

namespace neogfx
{
    class i_action
    {
    public:
        declare_event(triggered)
        declare_event(hovered)
        declare_event(enabled)
        declare_event(disabled)
        declare_event(checked)
        declare_event(unchecked)
        declare_event(changed)
    public:
        typedef optional<string> optional_text;
    public:
        virtual bool is_enabled() const = 0;
        virtual bool is_disabled() const = 0;
        virtual bool is_checkable() const = 0;
        virtual bool is_checked() const = 0;
        virtual bool is_unchecked() const = 0;
        virtual uuid const& group() const = 0;
        virtual bool is_separator() const = 0;
        virtual i_string const& text() const = 0;
        virtual i_string const& menu_text() const = 0;
        virtual i_string const& button_text() const = 0;
        virtual i_string const& tool_tip_text() const = 0;
        virtual i_string const& help_text() const = 0;
        virtual i_texture const& image() const = 0;
        virtual i_texture const& checked_image() const = 0;
        virtual optional_key_sequence const& shortcut() const = 0;
        virtual i_action& set_enabled(bool aEnabled) = 0;
        virtual i_action& set_checkable(bool aCheckable) = 0;
        virtual i_action& set_checked(bool aChecked) = 0;
        virtual i_action& set_group(uuid const& aGroup) = 0;
        virtual i_action& set_separator(bool aIsSeparator) = 0;
        virtual i_action& set_text(i_optional<i_string> const& aText) = 0;
        virtual i_action& set_menu_text(i_optional<i_string> const& aMenuText) = 0;
        virtual i_action& set_button_text(i_optional<i_string> const& aButtonText) = 0;
        virtual i_action& set_tool_tip_text(i_optional<i_string> const& aToolTipText) = 0;
        virtual i_action& set_help_text(i_optional<i_string> const& aHelpText) = 0;
        virtual i_action& set_image(i_texture const& aTexture) = 0;
        virtual i_action& set_checked_image(i_texture const& aTexture) = 0;
        virtual i_action& set_shortcut(optional_key_sequence const& aShortcut) = 0;
        virtual i_action& set_shortcut(i_string const& aShortcut) = 0;
    public:
        i_action& enable(bool aEnabled = true)
        {
            return set_enabled(aEnabled);
        }
        i_action& disable(bool aDisabled = true)
        {
            return set_enabled(!aDisabled);
        }
        i_action& check(bool aChecked = true)
        {
            return set_checked(aChecked);
        }
        i_action& uncheck(bool aUnchecked = true)
        {
            return set_checked(!aUnchecked);
        }
        i_action& toggle()
        {
            if (is_checked())
                return uncheck();
            else
                return check();
        }
        i_action& set_text(optional_text const& aText = optional_text{})
        {
            return set_text(to_abstract(aText));
        }
        i_action& set_menu_text(optional_text const& aMenuText = optional_text{})
        {
            return set_menu_text(to_abstract(aMenuText));
        }
        i_action& set_button_text(optional_text const& aButtonText = optional_text{})
        {
            return set_button_text(to_abstract(aButtonText));
        }
        i_action& set_tool_tip_text(optional_text const& aToolTipText = optional_text{})
        {
            return set_tool_tip_text(to_abstract(aToolTipText));
        }
        i_action& set_help_text(optional_text const& aHelpText = optional_text{})
        {
            return set_help_text(to_abstract(aHelpText));
        }
        i_action& set_image(string const& aUri, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::Scaled)
        {
            return set_image(texture{ neogfx::image{aUri, aDpiScaleFactor, aSampling} });
        }
        i_action& set_image(i_image const& aImage)
        {
            return set_image(texture{ aImage });
        }
        i_action& set_checked_image(string const& aUri, dimension aDpiScaleFactor = 1.0, texture_sampling aSampling = texture_sampling::Scaled)
        {
            return set_checked_image(texture{ neogfx::image{ aUri, aDpiScaleFactor, aSampling } });
        }
        i_action& set_checked_image(i_image const& aImage)
        {
            return set_checked_image(texture{ aImage });
        }
        i_action& set_shortcut(string const& aShortcut)
        {
            return set_shortcut(to_abstract(aShortcut));
        }
    };

    class i_action_container
    {
    public:
        virtual i_action& add_action(i_action& aAction) = 0;
        virtual i_action& add_action(std::shared_ptr<i_action> aAction) = 0;
    };
}