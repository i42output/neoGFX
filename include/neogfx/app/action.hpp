// action.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include "i_action.hpp"

namespace neogfx
{
	class action : public i_action
	{
	public:
		action();
		action(const std::string& aText);
		action(const std::string& aText, const std::string& aImageUri, texture_sampling aSampling = texture_sampling::Normal);
		action(const std::string& aText, const i_texture& aImage);
		action(const std::string& aText, const i_image& aImage);
	public:
		virtual bool is_enabled() const;
		virtual bool is_disabled() const;
		virtual bool is_checkable() const;
		virtual bool is_checked() const;
		virtual bool is_unchecked() const;
		virtual uint32_t group() const;
		virtual bool is_separator() const;
		virtual std::string text() const;
		virtual std::string menu_text() const;
		virtual std::string button_text() const;
		virtual std::string tool_tip_text() const;
		virtual std::string statis_tip_text() const;
		virtual const i_texture& image() const;
		virtual const i_texture& checked_image() const;
		virtual const optional_key_sequence& shortcut() const;
		virtual i_action& enable();
		virtual i_action& disable();
		virtual i_action& set_checkable(bool aCheckable);
		virtual i_action& check();
		virtual i_action& uncheck();
		virtual i_action& set_group(uint32_t aGroup);
		virtual i_action& set_separator(bool aIsSeparator);
		virtual i_action& set_text(const optional_text& aText = optional_text());
		virtual i_action& set_menu_text(const optional_text& aMenuText = optional_text());
		virtual i_action& set_button_text(const optional_text& aButtonText = optional_text());
		virtual i_action& set_tool_tip_text(const optional_text& aToolTipText = optional_text());
		virtual i_action& set_statis_tip_text(const optional_text& aStatusTipText = optional_text());
		virtual i_action& set_image(const std::string& aUri, texture_sampling aSampling = texture_sampling::Normal);
		virtual i_action& set_image(const i_image& aImage);
		virtual i_action& set_image(const i_texture& aTexture);
		virtual i_action& set_checked_image(const std::string& aUri, texture_sampling aSampling = texture_sampling::Normal);
		virtual i_action& set_checked_image(const i_image& aImage);
		virtual i_action& set_checked_image(const i_texture& aTexture);
		virtual i_action& set_shortcut(const optional_key_sequence& aShortcut);
		virtual i_action& set_shortcut(const std::string& aShortcut);
	protected:
		virtual void set_enabled(bool aEnabled);
		virtual void set_checked(bool aChecked);
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
		optional_text iStatusTipText;
		texture iImage;
		texture iCheckedTexture;
		optional_key_sequence iShortcut;
	};
}