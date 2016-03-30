// action.hpp
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

#pragma once

#include "neogfx.hpp"
#include "texture.hpp"
#include "i_action.hpp"

namespace neogfx
{
	class action : public i_action
	{
	public:
		action();
		action(const std::string& aText);
		action(const std::string& aText, const std::string& aImageUri);
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
		virtual const optional_key_sequence& short_cut() const;
		virtual void set_enabled();
		virtual void set_disabled();
		virtual void set_checkable(bool aCheckable);
		virtual void set_checked();
		virtual void set_unchecked();
		virtual void set_group(uint32_t aGroup);
		virtual void set_separator(bool aIsSeparator);
		virtual void set_text(const optional_text& aText = optional_text());
		virtual void set_menu_text(const optional_text& aMenuText = optional_text());
		virtual void set_button_text(const optional_text& aButtonText = optional_text());
		virtual void set_tool_tip_text(const optional_text& aToolTipText = optional_text());
		virtual void set_statis_tip_text(const optional_text& aStatusTipText = optional_text());
		virtual void set_image(const std::string& aUri);
		virtual void set_image(const i_image& aImage);
		virtual void set_image(const i_texture& aTexture);
		virtual void set_checked_image(const std::string& aUri);
		virtual void set_checked_image(const i_image& aImage);
		virtual void set_checked_image(const i_texture& aTexture);
		virtual void set_short_cut(const optional_key_sequence& aShortCut);
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
		texture iTexture;
		texture iCheckedTexture;
		optional_key_sequence iShortCut;
	};
}