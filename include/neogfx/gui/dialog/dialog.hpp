// dialog.hpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/window/window.hpp>
#include "dialog_button_box.hpp"

namespace neogfx
{
	class dialog : public window
	{
	public:
		event<bool&> try_accept;
		event<bool&> try_reject;
	public:
		enum result_code_e
		{
			Accepted,
			Rejected
		};
	public:
		dialog(style_e aStyle = Default);
		dialog(const std::string& aDialogTitle, style_e aStyle = Default);
		dialog(const size& aDimensions, style_e aStyle = Default);
		dialog(const size& aDimensions, const std::string& aDialogTitle, style_e aStyle = Default);
		dialog(const point& aPosition, const size& aDimensions, style_e aStyle = Default);
		dialog(const point& aPosition, const size& aDimensions, const std::string& aDialogTitle, style_e aStyle = Default);
		dialog(i_widget& aParent, style_e aStyle = Default);
		dialog(i_widget& aParent, const std::string& aDialogTitle, style_e aStyle = Default);
		dialog(i_widget& aParent, const size& aDimensions, style_e aStyle = Default);
		dialog(i_widget& aParent, const size& aDimensions, const std::string& aDialogTitle, style_e aStyle = Default);
		dialog(i_widget& aParent, const point& aPosition, const size& aDimensions, style_e aStyle = Default);
		dialog(i_widget& aParent, const point& aPosition, const size& aDimensions, const std::string& aDialogTitle, style_e aStyle = Default);
		~dialog();
	public:
		virtual void accept();
		virtual void reject();
	public:
		dialog_button_box& button_box();
		result_code_e exec();
	public:
		virtual neogfx::size_policy size_policy() const;
	public:
		virtual bool can_close() const;
	public:
		virtual bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
	private:
		void init();
	private:
		boost::optional<dialog_button_box> iButtonBox;
		boost::optional<result_code_e> iResult;
	};
}