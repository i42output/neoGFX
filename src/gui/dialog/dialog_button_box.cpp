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
		static const std::map<platform, std::vector<button_role>> sRoleOrder
		{
			{ platform::Windows, { button_role::ResetRole, button_role::YesRole, button_role::AcceptRole, button_role::DestructiveRole, button_role::NoRole, button_role::ActionRole, button_role::RejectRole, button_role::ApplyRole, button_role::HelpRole } },
			{ platform::Mac, { button_role::HelpRole, button_role::ResetRole, button_role::ApplyRole, button_role::ActionRole, button_role::DestructiveRole, button_role::RejectRole, button_role::AcceptRole, button_role::NoRole, button_role::YesRole } },
			{ platform::Kde, { button_role::HelpRole, button_role::ResetRole, button_role::YesRole, button_role::NoRole, button_role::ActionRole, button_role::AcceptRole, button_role::ApplyRole, button_role::DestructiveRole, button_role::RejectRole } },
			{ platform::Gnome, { button_role::HelpRole, button_role::ResetRole, button_role::ActionRole, button_role::ApplyRole, button_role::DestructiveRole, button_role::RejectRole, button_role::AcceptRole, button_role::NoRole, button_role::YesRole } }
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

	standard_button dialog_button_box::button_with_role(button_role aButtonRole) const
	{
		for (auto& button : iButtons)
			if (button.first.second == aButtonRole)
				return button.first.first;
		for (auto& button : iButtons)
			if (similar_role(button.first.second, aButtonRole))
				return button.first.first;
		return iButtons.begin()->first.first;
	}

	push_button& dialog_button_box::button(standard_button aStandardButton) const
	{
		for (auto& button : iButtons)
			if (button.first.first == aStandardButton)
				return *button.second;
		throw button_not_found();
	}

	void dialog_button_box::add_button(standard_button aStandardButton)
	{
		auto bd = standard_button_details(aStandardButton);
		add_button(aStandardButton, bd.first, bd.second);
	}

	void dialog_button_box::add_button(standard_button aStandardButton, button_role aButtonRole, const std::string& aButtonText)
	{
		auto newButton = iButtons.emplace(std::make_pair(aStandardButton, aButtonRole), std::make_unique<push_button>(*this, aButtonText, push_button_style::ButtonBox));
		switch (newButton->first.second)
		{
		case button_role::AcceptRole:
		case button_role::YesRole:
			newButton->second->clicked([this, newButton]() { clicked.trigger(newButton->first.first); accepted.trigger(); });
			break;
		case button_role::RejectRole:
		case button_role::NoRole:
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

	void dialog_button_box::add_buttons(standard_button aStandardButtons)
	{
		uint32_t buttons = static_cast<uint32_t>(aStandardButtons);
		uint32_t bit = 0;
		while (bit < 32)
		{
			auto button = buttons & (1 << bit++);
			if (button != 0)
				add_button(static_cast<standard_button>(button));
		}
	}

	void dialog_button_box::clear()
	{
		iLayout.remove_items();
		iButtons.clear();
	}

	dialog_button_box::button_details dialog_button_box::standard_button_details(standard_button aStandardButton)
	{
		static const std::map<standard_button, button_details> sStandardButtonDetails
		{
			{ standard_button::Ok,					{ button_role::AcceptRole, "OK"} },
			{ standard_button::Cancel,				{ button_role::RejectRole, "Cancel"} },
			{ standard_button::Close,				{ button_role::RejectRole, "Close"} },
			{ standard_button::Discard,				{ button_role::DestructiveRole, "Discard"} },
			{ standard_button::Apply,				{ button_role::ApplyRole, "Apply"} },
			{ standard_button::Reset,				{ button_role::ResetRole, "Reset"} },
			{ standard_button::RestoreDefaults,		{ button_role::ResetRole, "Restore Defaults"} },
			{ standard_button::Yes,					{ button_role::YesRole, "Yes"} },
			{ standard_button::No,					{ button_role::NoRole, "No"} },
			{ standard_button::YesToAll,			{ button_role::YesRole, "Yes To All"} },
			{ standard_button::NoToAll,				{ button_role::NoRole, "No To All"} },
			{ standard_button::Abort,				{ button_role::RejectRole, "Abort"} },
			{ standard_button::Retry,				{ button_role::AcceptRole, "Retry"} },
			{ standard_button::Ignore,				{ button_role::AcceptRole, "Ignore"} },
			{ standard_button::Open,				{ button_role::AcceptRole, "Open"} },
			{ standard_button::Save,				{ button_role::AcceptRole, "Save"} },
			{ standard_button::SaveAll,				{ button_role::AcceptRole, "Save All"} },
			{ standard_button::Help,				{ button_role::HelpRole, "Help"} },
			{ standard_button::Custom1,				{ button_role::InvalidRole, "" } },
			{ standard_button::Custom2,				{ button_role::InvalidRole, "" } },
			{ standard_button::Custom3,				{ button_role::InvalidRole, "" } },
			{ standard_button::Custom4,				{ button_role::InvalidRole, "" } }
		};
		auto bd = sStandardButtonDetails.find(aStandardButton);
		if (bd != sStandardButtonDetails.end())
			return bd->second;
		return button_details{ button_role::InvalidRole, "" };
	}

	void dialog_button_box::init()
	{
		set_margins(neogfx::margins{});
		iLayout.set_margins(neogfx::margins{});
	}

	bool dialog_button_box::similar_role(button_role aButtonRole1, button_role aButtonRole2)
	{
		switch (aButtonRole1)
		{
		case button_role::AcceptRole:
		case button_role::YesRole:
			switch (aButtonRole2)
			{
			case button_role::AcceptRole:
			case button_role::YesRole:
				return true;
			default:
				return false;
			}
		case button_role::RejectRole:
		case button_role::NoRole:
			switch (aButtonRole2)
			{
			case button_role::RejectRole:
			case button_role::NoRole:
				return true;
			default:
				return false;
			}
		case button_role::InvalidRole:
		case button_role::DestructiveRole:
		case button_role::ActionRole:
		case button_role::ApplyRole:
		case button_role::ResetRole:
		case button_role::HelpRole:
		default:
			return false;
		}
	}
}