// action.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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
		iEnabled(false), iCheckable(false), iChecked(false), iGroup(0), iSeparator(true)
	{
	}

	action::action(const std::string& aText) :
		iEnabled(true), iCheckable(false), iChecked(false), iGroup(0), iSeparator(false), iText(aText)
	{
	}

	action::action(const std::string& aText, const std::string& aImageUri, texture_sampling aSampling) :
		iEnabled(true), iCheckable(false), iChecked(false), iGroup(0), iSeparator(false), iText(aText), iImage(neogfx::image(aImageUri, aSampling))
	{
	}

	action::action(const std::string& aText, const i_texture& aImage) :
		iEnabled(true), iCheckable(false), iChecked(false), iGroup(0), iSeparator(false), iText(aText), iImage(aImage)
	{
	}

	action::action(const std::string& aText, const i_image& aImage) :
		iEnabled(true), iCheckable(false), iChecked(false), iGroup(0), iSeparator(false), iText(aText), iImage(aImage)
	{
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

	uint32_t action::group() const
	{
		return iGroup;
	}

	bool action::is_separator() const
	{
		return iSeparator;
	}

	std::string action::text() const
	{
		if (iText != boost::none)
			return *iText;
		else if (iMenuText != boost::none)
			return *iMenuText;
		else if (iButtonText != boost::none)
			return *iButtonText;
		else
			return std::string();
	}

	std::string action::menu_text() const
	{
		if (iMenuText != boost::none)
			return *iMenuText;
		else if (iText != boost::none)
			return *iText;
		else if (iButtonText != boost::none)
			return *iButtonText;
		else
			return std::string();
	}

	std::string action::button_text() const
	{
		if (iButtonText != boost::none)
			return *iButtonText;
		else if (iText != boost::none)
			return *iText;
		else if (iMenuText != boost::none)
			return *iMenuText;
		else
			return std::string();
	}

	std::string action::tool_tip_text() const
	{
		if (iToolTipText != boost::none)
			return *iToolTipText;
		else if (iText != boost::none)
			return *iText;
		else if (iMenuText != boost::none)
			return *iMenuText;
		else if (iButtonText != boost::none)
			return *iButtonText;
		else
			return std::string();
	}

	std::string action::statis_tip_text() const
	{
		if (iStatusTipText != boost::none)
			return *iToolTipText;
		else if (iText != boost::none)
			return *iText;
		else if (iToolTipText != boost::none)
			return *iToolTipText;
		else if (iMenuText != boost::none)
			return *iMenuText;
		else if (iButtonText != boost::none)
			return *iButtonText;
		else
			return std::string();
	}

	const i_texture& action::image() const
	{
		return iImage;
	}

	const i_texture& action::checked_image() const
	{
		if (!iCheckedTexture.is_empty())
			return iCheckedTexture;
		return image();
	}

	const optional_key_sequence& action::shortcut() const
	{
		return iShortcut;
	}

	i_action& action::enable()
	{
		set_enabled(true);
		return *this;
	}

	i_action& action::disable()
	{
		set_enabled(false);
		return *this;
	}

	i_action& action::set_checkable(bool aCheckable)
	{
		if (iCheckable != aCheckable)
		{
			iCheckable = aCheckable;
			changed.trigger();
		}
		return *this;
	}

	i_action& action::check()
	{
		set_checked(true);
		return *this;
	}

	i_action& action::uncheck()
	{
		set_checked(false);
		return *this;
	}

	i_action& action::set_group(uint32_t aGroup)
	{
		if (iGroup != aGroup)
		{
			iGroup = aGroup;
			changed.trigger();
		}
		return *this;
	}

	i_action& action::set_separator(bool aIsSeparator)
	{
		if (iSeparator != aIsSeparator)
		{
			iSeparator = aIsSeparator;
			changed.trigger();
		}
		return *this;
	}

	i_action& action::set_text(const optional_text& aText)
	{
		if (iText != aText)
		{
			iText = aText;
			if (iText != boost::none && iSeparator)
				iSeparator = false;
			changed.trigger();
		}
		return *this;
	}

	i_action& action::set_menu_text(const optional_text& aMenuText)
	{
		if (iMenuText != aMenuText)
		{
			iMenuText = aMenuText;
			if (iMenuText != boost::none && iSeparator)
				iSeparator = false;
			changed.trigger();
		}
		return *this;
	}

	i_action& action::set_button_text(const optional_text& aButtonText)
	{
		if (iButtonText != aButtonText)
		{
			iButtonText = aButtonText;
			if (iButtonText != boost::none && iSeparator)
				iSeparator = false;
			changed.trigger();
		}
		return *this;
	}

	i_action& action::set_tool_tip_text(const optional_text& aToolTipText)
	{
		if (iToolTipText != aToolTipText)
		{
			iToolTipText = aToolTipText;
			if (iToolTipText != boost::none && iSeparator)
				iSeparator = false;
			changed.trigger();
		}
		return *this;
	}

	i_action& action::set_statis_tip_text(const optional_text& aStatusTipText)
	{
		if (iStatusTipText != aStatusTipText)
		{
			iStatusTipText = aStatusTipText;
			if (iStatusTipText != boost::none && iSeparator)
				iSeparator = false;
			changed.trigger();
		}
		return *this;
	}

	i_action& action::set_image(const std::string& aUri, texture_sampling aSampling)
	{
		set_image(neogfx::image{ aUri, aSampling });
		return *this;
	}

	i_action& action::set_image(const i_image& aImage)
	{
		set_image(texture(aImage));
		return *this;
	}

	i_action& action::set_image(const i_texture& aTexture)
	{
		iImage = aTexture;
		if (!iImage.is_empty() && iSeparator)
			iSeparator = false;
		changed.trigger();
		return *this;
	}

	i_action& action::set_checked_image(const std::string& aUri, texture_sampling aSampling)
	{
		set_checked_image(neogfx::image{ aUri, aSampling });
		return *this;
	}

	i_action& action::set_checked_image(const i_image& aImage)
	{
		set_checked_image(texture(aImage));
		return *this;
	}

	i_action& action::set_checked_image(const i_texture& aTexture)
	{
		iCheckedTexture = aTexture;
		if (!iCheckedTexture.is_empty() && iSeparator)
			iSeparator = false;
		changed.trigger();
		return *this;
	}

	i_action& action::set_shortcut(const optional_key_sequence& aShortcut)
	{
		if (iShortcut != aShortcut)
		{
			iShortcut = aShortcut;
			if (iShortcut != boost::none && iSeparator)
				iSeparator = false;
			changed.trigger();
		}
		return *this;
	}

	i_action& action::set_shortcut(const std::string& aShortcut)
	{
		return set_shortcut(key_sequence(aShortcut));
	}

	void action::set_enabled(bool aEnabled)
	{
		if (iEnabled != aEnabled)
		{
			iEnabled = aEnabled;
			if (is_enabled())
				enabled.trigger();
			else
				disabled.trigger();
		}
	}

	void action::set_checked(bool aChecked)
	{
		if (iChecked != aChecked)
		{
			iChecked = aChecked;
			if (is_checked())
				checked.trigger();
			else
				unchecked.trigger();
		}
	}
}