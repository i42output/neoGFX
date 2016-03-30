// action.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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

#include "neogfx.hpp"
#include "image.hpp"
#include "action.hpp"

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

	action::action(const std::string& aText, const std::string& aImageUri) :
		iEnabled(true), iCheckable(false), iChecked(false), iGroup(0), iSeparator(false), iText(aText), iTexture(neogfx::image(aImageUri))
	{
	}

	action::action(const std::string& aText, const i_texture& aImage) :
		iEnabled(true), iCheckable(false), iChecked(false), iGroup(0), iSeparator(false), iText(aText), iTexture(aImage)
	{
	}

	action::action(const std::string& aText, const i_image& aImage) :
		iEnabled(true), iCheckable(false), iChecked(false), iGroup(0), iSeparator(false), iText(aText), iTexture(aImage)
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
		return iTexture;
	}

	const i_texture& action::checked_image() const
	{
		if (!iCheckedTexture.is_empty())
			return iCheckedTexture;
		return image();
	}

	const optional_key_sequence& action::short_cut() const
	{
		return iShortCut;
	}

	void action::set_enabled()
	{
		set_enabled(true);
	}

	void action::set_disabled()
	{
		set_enabled(false);
	}

	void action::set_checkable(bool aCheckable)
	{
		if (iCheckable != aCheckable)
		{
			iCheckable = aCheckable;
			changed.trigger();
		}
	}

	void action::set_checked()
	{
		set_checked(true);
	}

	void action::set_unchecked()
	{
		set_checked(false);
	}

	void action::set_group(uint32_t aGroup)
	{
		if (iGroup != aGroup)
		{
			iGroup = aGroup;
			changed.trigger();
		}
	}

	void action::set_separator(bool aIsSeparator)
	{
		if (iSeparator != aIsSeparator)
		{
			iSeparator = aIsSeparator;
			changed.trigger();
		}
	}

	void action::set_text(const optional_text& aText)
	{
		if (iText != aText)
		{
			iText = aText;
			if (iText != boost::none && iSeparator)
				iSeparator = false;
			changed.trigger();
		}
	}

	void action::set_menu_text(const optional_text& aMenuText)
	{
		if (iMenuText != aMenuText)
		{
			iMenuText = aMenuText;
			if (iMenuText != boost::none && iSeparator)
				iSeparator = false;
			changed.trigger();
		}
	}

	void action::set_button_text(const optional_text& aButtonText)
	{
		if (iButtonText != aButtonText)
		{
			iButtonText = aButtonText;
			if (iButtonText != boost::none && iSeparator)
				iSeparator = false;
			changed.trigger();
		}
	}

	void action::set_tool_tip_text(const optional_text& aToolTipText)
	{
		if (iToolTipText != aToolTipText)
		{
			iToolTipText = aToolTipText;
			if (iToolTipText != boost::none && iSeparator)
				iSeparator = false;
			changed.trigger();
		}
	}

	void action::set_statis_tip_text(const optional_text& aStatusTipText)
	{
		if (iStatusTipText != aStatusTipText)
		{
			iStatusTipText = aStatusTipText;
			if (iStatusTipText != boost::none && iSeparator)
				iSeparator = false;
			changed.trigger();
		}
	}

	void action::set_image(const std::string& aUri)
	{
		set_image(neogfx::image(aUri));
	}

	void action::set_image(const i_image& aImage)
	{
		set_image(texture(aImage));
	}

	void action::set_image(const i_texture& aTexture)
	{
		iTexture = aTexture;
		if (!iTexture.is_empty() && iSeparator)
			iSeparator = false;
		changed.trigger();
	}

	void action::set_checked_image(const std::string& aUri)
	{
		set_checked_image(neogfx::image(aUri));
	}

	void action::set_checked_image(const i_image& aImage)
	{
		set_checked_image(texture(aImage));
	}

	void action::set_checked_image(const i_texture& aTexture)
	{
		iCheckedTexture = aTexture;
		if (!iCheckedTexture.is_empty() && iSeparator)
			iSeparator = false;
		changed.trigger();
	}

	void action::set_short_cut(const optional_key_sequence& aShortCut)
	{
		if (iShortCut != aShortCut)
		{
			iShortCut = aShortCut;
			if (iShortCut != boost::none && iSeparator)
				iSeparator = false;
			changed.trigger();
		}
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