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

#include "neogfx.hpp"
#include "app.hpp"
#include "widget.hpp"
#include "horizontal_layout.hpp"
#include "spacer.hpp"
#include "push_button.hpp"

namespace neogfx
{
	class dialog_button_box : public widget
	{
	public:
		event<> accepted;
		event<> rejected;
	public:
		enum standard_button_e
		{
			Custom,
			Ok,
			Cancel,
			Close,
			Discard,
			Apply,
			Reset,
			RestoreDefaults,
			Yes,
			No,
			YesToAll,
			NoToAll,
			Abort,
			Retry,
			Ignore,
			Open,
			Save,
			SaveAll,
			Help
		};
		enum button_role_e
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
	private:
		typedef std::pair<standard_button_e, button_role_e> button_key;
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
		push_button& button(standard_button_e aStandardButton) const;
		void add_button(standard_button_e aStandardButton);
	private:
		void init();
	private:
		horizontal_layout iLayout;
		horizontal_spacer iSpacer;
		button_list iButtons;
	};
}