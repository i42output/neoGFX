// dialog_button_box.hpp
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
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/push_button.hpp>

namespace neogfx
{
	enum class standard_button : uint32_t
	{
		Custom			= 0x00001,
		Ok				= 0x00002,
		Cancel			= 0x00004,
		Close			= 0x00008,
		Discard			= 0x00010,
		Apply			= 0x00020,
		Reset			= 0x00040,
		RestoreDefaults	= 0x00080,
		Yes				= 0x00100,
		No				= 0x00200,
		YesToAll		= 0x00400,
		NoToAll			= 0x00800,
		Abort			= 0x01000,
		Retry			= 0x02000,
		Ignore			= 0x04000,
		Open			= 0x08000,
		Save			= 0x10000,
		SaveAll			= 0x20000,
		Help			= 0x40000
	};

	inline constexpr standard_button operator|(standard_button aLhs, standard_button aRhs)
	{
		return static_cast<standard_button>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline constexpr standard_button operator&(standard_button aLhs, standard_button aRhs)
	{
		return static_cast<standard_button>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
	}

	enum class button_role
	{
		InvalidRole,
		AcceptRole,
		RejectRole,
		DestructiveRole,
		ActionRole,
		ApplyRole,
		ResetRole,
		YesRole,
		NoRole,
		HelpRole
	};

	class dialog_button_box : public widget
	{
	public:
		event<> accepted;
		event<> rejected;
	public:
		event<standard_button> clicked;
	private:
		typedef std::pair<standard_button, button_role> button_key;
		struct button_sorter
		{
			bool operator()(const button_key& aLhs, const button_key& aRhs) const;
		};
		typedef std::multimap<button_key, std::unique_ptr<push_button>, button_sorter> button_list;
	public:
		struct button_not_found : std::logic_error { button_not_found() : std::logic_error("neogfx::dialog_button_box::button_not_found") {} };
	public:
		dialog_button_box(i_widget& aParent);
		dialog_button_box(i_layout& aLayout);
		~dialog_button_box();
	public:
		push_button& button(standard_button aStandardButton) const;
		void add_button(standard_button aStandardButton);
		void clear();
	private:
		void init();
	private:
		horizontal_layout iLayout;
		horizontal_spacer iSpacer;
		button_list iButtons;
	};
}