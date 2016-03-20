// i_action.hpp
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
#include <boost/optional.hpp>
#include "event.hpp"
#include "keyboard.hpp"
#include "i_texture.hpp"
#include "i_image.hpp"

namespace neogfx
{
	class i_action
	{
	public:
		event<> triggered;
		event<> hovered;
		event<> enabled;
		event<> disabled;
		event<> checked;
		event<> unchecked;
		event<> changed;
	public:
		typedef boost::optional<std::string> optional_text;
	public:
		virtual bool is_enabled() const = 0;
		virtual bool is_disabled() const = 0;
		virtual bool is_checkable() const = 0;
		virtual bool is_checked() const = 0;
		virtual bool is_unchecked() const = 0;
		virtual uint32_t group() const = 0;
		virtual bool is_separator() const = 0;
		virtual std::string text() const = 0;
		virtual std::string menu_text() const = 0;
		virtual std::string button_text() const = 0;
		virtual std::string tool_tip_text() const = 0;
		virtual std::string statis_tip_text() const = 0;
		virtual const i_texture& image() const = 0;
		virtual const key_sequence& short_cut() const = 0;
		virtual void set_enabled() = 0;
		virtual void set_disabled() = 0;
		virtual void set_checkable(bool aCheckable) = 0;
		virtual void set_checked() = 0;
		virtual void set_unchecked() = 0;
		virtual void set_group(uint32_t aGroup) = 0;
		virtual void set_separator(bool aIsSeparator) = 0;
		virtual void set_text(const optional_text& aText = optional_text()) = 0;
		virtual void set_menu_text(const optional_text& aMenuText = optional_text()) = 0;
		virtual void set_button_text(const optional_text& aButtonText = optional_text()) = 0;
		virtual void set_tool_tip_text(const optional_text& aToolTipText = optional_text()) = 0;
		virtual void set_statis_tip_text(const optional_text& aStatusTipText = optional_text()) = 0;
		virtual void set_image(const std::string& aUri) = 0;
		virtual void set_image(const i_texture& aTexture) = 0;
		virtual void set_image(const i_image& aImage) = 0;
		virtual void set_short_cut(const key_sequence& aShortCut) = 0;
	};
}