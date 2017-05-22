// dialog_button_box.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/dialog/dialog_button_box.hpp>

namespace neogfx
{
	bool dialog_button_box::button_sorter::operator()(const button_key& aLhs, const button_key& aRhs) const
	{
		static const std::map<platform, std::vector<button_role_e>> sRoleOrder
		{
			{ platform::Windows, { ResetRole, YesRole, AcceptRole, DestructiveRole, NoRole, ActionRole, RejectRole, ApplyRole, HelpRole } },
			{ platform::Mac, { HelpRole, ResetRole, ApplyRole, ActionRole, DestructiveRole, RejectRole, AcceptRole, NoRole, YesRole } },
			{ platform::Kde, { HelpRole, ResetRole, YesRole, NoRole, ActionRole, AcceptRole, ApplyRole, DestructiveRole, RejectRole } },
			{ platform::Gnome, { HelpRole, ResetRole, ActionRole, ApplyRole, DestructiveRole, RejectRole, AcceptRole, NoRole, YesRole } }
		};
		static const auto& sPlatformSpecific = sRoleOrder.find(app::instance().basic_services().platform())->second;
		auto left = std::find(sPlatformSpecific.begin(), sPlatformSpecific.end(), aLhs.second);
		auto right = std::find(sPlatformSpecific.begin(), sPlatformSpecific.end(), aRhs.second);
		return left < right;
	}

	dialog_button_box::dialog_button_box(i_widget& aParent) :
		widget(aParent), iLayout(*this), iSpacer(iLayout)
	{
		init();
	}

	dialog_button_box::dialog_button_box(i_layout& aLayout) :
		widget(aLayout), iLayout(*this), iSpacer(iLayout)
	{
		init();
	}

	dialog_button_box::~dialog_button_box()
	{
	}

	push_button& dialog_button_box::button(standard_button_e aStandardButton) const
	{
		for (auto& button : iButtons)
			if (button.first.first == aStandardButton)
				return *button.second;
		throw button_not_found();
	}

	void dialog_button_box::add_button(standard_button_e aStandardButton)
	{
		static const std::map<standard_button_e, std::pair<button_role_e, std::string>> sButtonDetails
		{
			{ Custom,				{ InvalidRole, ""} },
			{ Ok,					{ AcceptRole, "OK"} },
			{ Cancel,				{ RejectRole, "Cancel"} },
			{ Close,				{ RejectRole, "Close"} },
			{ Discard,				{ DestructiveRole, "Discard"} },
			{ Apply,				{ ApplyRole, "Apply"} },
			{ Reset,				{ ResetRole, "Reset"} },
			{ RestoreDefaults,		{ ResetRole, "Restore Defaults"} },
			{ Yes,					{ YesRole, "Yes"} },
			{ No,					{ NoRole, "No"} },
			{ YesToAll,				{ YesRole, "Yes To All"} },
			{ NoToAll,				{ NoRole, "No To All"} },
			{ Abort,				{ RejectRole, "Abort"} },
			{ Retry,				{ AcceptRole, "Retry"} },
			{ Ignore,				{ AcceptRole, "Ignore"} },
			{ Open,					{ AcceptRole, "Open"} },
			{ Save,					{ AcceptRole, "Save"} },
			{ SaveAll,				{ AcceptRole, "Save All"} },
			{ Help,					{ HelpRole, "Help"} }
		};
		auto bi = sButtonDetails.find(aStandardButton);
		auto newButton = iButtons.emplace(std::make_pair(bi->first, bi->second.first), std::make_unique<push_button>(*this, bi->second.second, push_button_style::ButtonBox));
		switch (newButton->first.second)
		{
		case AcceptRole:
		case YesRole:
			newButton->second->clicked([this, newButton]() { clicked.trigger(newButton->first.first); accepted.trigger(); });
			break;
		case RejectRole:
		case NoRole:
			newButton->second->clicked([this, newButton]() { clicked.trigger(newButton->first.first); rejected.trigger(); });
			break;
		default:
			newButton->second->clicked([this, newButton]() { clicked.trigger(newButton->first.first); });
			break;
		}
		iLayout.remove_items();
		iLayout.add_item(iSpacer);
		for (auto& button : iButtons)
			iLayout.add_item(*button.second);
	}

	void dialog_button_box::clear()
	{
		iLayout.remove_items();
		iButtons.clear();
	}

	void dialog_button_box::init()
	{
		set_margins(neogfx::margins{});
		iLayout.set_margins(neogfx::margins{});
	}
}