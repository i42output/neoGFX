// dialog_button_box.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/hid/i_keyboard.hpp>
#include <neogfx/gui/dialog/dialog_button_box.hpp>

namespace neogfx
{
    bool dialog_button_box::button_sorter::operator()(const button_key& aLhs, const button_key& aRhs) const
    {
        static const std::map<std::pair<platform, environment>, std::vector<button_role>> sRoleOrder
        {
            { { platform::Windows, environment::Native }, { button_role::Reset, button_role::Yes, button_role::Accept, button_role::Destructive, button_role::No, button_role::Action, button_role::Reject, button_role::Apply, button_role::Help } },
            { { platform::macOS, environment::Native }, { button_role::Help, button_role::Reset, button_role::Apply, button_role::Action, button_role::Destructive, button_role::Reject, button_role::Accept, button_role::No, button_role::Yes } },
            { { platform::Linux, environment::Gnome }, { button_role::Help, button_role::Reset, button_role::Action, button_role::Apply, button_role::Destructive, button_role::Reject, button_role::Accept, button_role::No, button_role::Yes } },
            { { platform::Linux, environment::Kde }, { button_role::Help, button_role::Reset, button_role::Yes, button_role::No, button_role::Action, button_role::Accept, button_role::Apply, button_role::Destructive, button_role::Reject } },
        };
        static auto const& sPlatformSpecific = sRoleOrder.find(
            std::make_pair(service<i_basic_services>().platform(), service<i_basic_services>().environment()))->second;
        auto left = std::find(sPlatformSpecific.begin(), sPlatformSpecific.end(), aLhs.second);
        auto right = std::find(sPlatformSpecific.begin(), sPlatformSpecific.end(), aRhs.second);
        return left < right;
    }

    dialog_button_box::dialog_button_box(i_widget& aParent) :
        widget{ aParent }, iLayout{ *this }, iOptionLayout{ iLayout }, iSpacer{ iLayout }, iStandardButtonLayout{ iLayout }
    {
        init();
    }

    dialog_button_box::dialog_button_box(i_layout& aLayout) :
        widget{ aLayout }, iLayout{ *this }, iOptionLayout{ iLayout }, iSpacer{ iLayout }, iStandardButtonLayout{ iLayout }
    {
        init();
    }

    dialog_button_box::~dialog_button_box()
    {
    }

    standard_button dialog_button_box::button_with_role(button_role aButtonRole) const
    {
        if (iButtons.empty())
            return standard_button::Close;
        for (auto& button : iButtons)
            if (button.first.second == aButtonRole)
                return button.first.first;
        for (auto& button : iButtons)
            if (similar_role(button.first.second, aButtonRole))
                return button.first.first;
        return iButtons.begin()->first.first;
    }

    button_role dialog_button_box::role_of_button(standard_button aStandardButton)
    {
        for (auto& button : iButtons)
            if (button.first.first == aStandardButton)
                return button.first.second;
        throw button_not_found();
    }

    void dialog_button_box::enable_role(button_role aButtonRole)
    {
        for (auto& button : iButtons)
            if (similar_role(aButtonRole, button.first.second))
                button.second->enable();
    }

    void dialog_button_box::disable_role(button_role aButtonRole)
    {
        for (auto& button : iButtons)
            if (similar_role(aButtonRole, button.first.second))
                button.second->disable();
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

    void dialog_button_box::add_button(standard_button aStandardButton, button_role aButtonRole, std::string const& aButtonText)
    {
        auto newButton = iButtons.emplace(std::make_pair(aStandardButton, aButtonRole), std::make_unique<push_button>(*this, aButtonText, push_button_style::ButtonBox));
        switch (newButton->first.second)
        {
        case button_role::Accept:
        case button_role::Yes:
            newButton->second->clicked([this, newButton]() { Clicked(newButton->first.first); Accepted(); });
            break;
        case button_role::Reject:
        case button_role::No:
            newButton->second->clicked([this, newButton]() { Clicked(newButton->first.first); Rejected(); });
            break;
        default:
            newButton->second->clicked([this, newButton]() { Clicked(newButton->first.first); });
            break;
        }
        iStandardButtonLayout.remove_all();
        for (auto& button : iButtons)
            iStandardButtonLayout.add(*button.second);
    }

    void dialog_button_box::add_buttons(standard_button aStandardButtons)
    {
        std::uint32_t buttons = static_cast<std::uint32_t>(aStandardButtons);
        std::uint32_t bit = 0;
        while (bit < 32)
        {
            auto button = buttons & (1 << bit++);
            if (button != 0)
                add_button(static_cast<standard_button>(button));
        }
    }

    void dialog_button_box::set_default_button(standard_button aButton)
    {
        iDefaultButton = aButton;
    }

    void dialog_button_box::clear()
    {
        iStandardButtonLayout.remove_all();
        iButtons.clear();
        iDefaultButton = {};
    }

    i_layout& dialog_button_box::option_layout()
    {
        return iOptionLayout;
    }

    bool dialog_button_box::has_reject_role(standard_button aStandardButtons)
    {
        std::uint32_t buttons = static_cast<std::uint32_t>(aStandardButtons);
        std::uint32_t bit = 0;
        while (bit < 32)
        {
            auto button = buttons & (1 << bit++);
            if (button != 0 && standard_button_details(static_cast<standard_button>(button)).first == button_role::Reject)
                return true;
        }
        return false;
    }

    dialog_button_box::button_details dialog_button_box::standard_button_details(standard_button aStandardButton)
    {
        static const std::map<standard_button, button_details> sStandardButtonDetails
        {
            { standard_button::Ok,                    { button_role::Accept, "OK"_t } },
            { standard_button::Cancel,                { button_role::Reject, "Cancel"_t } },
            { standard_button::Close,                { button_role::Reject, "Close"_t } },
            { standard_button::Discard,                { button_role::Destructive, "Discard"_t } },
            { standard_button::Apply,                { button_role::Apply, "Apply"_t } },
            { standard_button::Reset,                { button_role::Reset, "Reset"_t } },
            { standard_button::RestoreDefaults,        { button_role::Reset, "Restore Defaults"_t } },
            { standard_button::Yes,                    { button_role::Yes, "Yes"_t } },
            { standard_button::No,                    { button_role::No, "No"_t } },
            { standard_button::YesToAll,            { button_role::Yes, "Yes To All"_t } },
            { standard_button::NoToAll,                { button_role::No, "No To All"_t } },
            { standard_button::Abort,                { button_role::Reject, "Abort"_t } },
            { standard_button::Retry,                { button_role::Accept, "Retry"_t } },
            { standard_button::Ignore,                { button_role::Accept, "Ignore"_t } },
            { standard_button::Open,                { button_role::Accept, "Open"_t } },
            { standard_button::Save,                { button_role::Accept, "Save"_t } },
            { standard_button::SaveAll,                { button_role::Accept, "Save All"_t } },
            { standard_button::Help,                { button_role::Help, "Help"_t } },
            { standard_button::Custom1,                { button_role::Invalid, "" } },
            { standard_button::Custom2,                { button_role::Invalid, "" } },
            { standard_button::Custom3,                { button_role::Invalid, "" } },
            { standard_button::Custom4,                { button_role::Invalid, "" } }
        };
        auto bd = sStandardButtonDetails.find(aStandardButton);
        if (bd != sStandardButtonDetails.end())
            return bd->second;
        return button_details{ button_role::Invalid, "" };
    }

    void dialog_button_box::init()
    {
        iSink += service<i_keyboard>().key_pressed([this](scan_code_e aScanCode, key_code_e, key_modifiers_e)
        {
            if (root().is_active())
            {
                switch (aScanCode)
                {
                case ScanCode_RETURN:
                case ScanCode_KEYPAD_ENTER:
                    if (!root().has_focused_widget() ||
                        !find_widget_with_focus_policy(root().focused_widget(), neogfx::focus_policy::ConsumeReturnKey))
                    {
                        if (!iDefaultButton || similar_role(role_of_button(*iDefaultButton), button_role::Accept))
                            Accepted();
                        else if (similar_role(role_of_button(*iDefaultButton), button_role::Reject))
                            Rejected();
                    }
                    break;
                case ScanCode_ESCAPE:
                    if (!root().has_focused_widget() ||
                        !find_widget_with_focus_policy(root().focused_widget(), neogfx::focus_policy::ConsumeEscapeKey))
                    {
                        if (can_reject())
                            Rejected();
                    }
                    break;
                default:
                    break;
                }
            }
        });
        set_padding(neogfx::padding{});
        iLayout.set_padding(neogfx::padding{});
        iOptionLayout.set_padding(neogfx::padding{});
        iOptionLayout.set_size_policy(size_constraint::Minimum);
        iStandardButtonLayout.set_padding(neogfx::padding{});
    }

    bool dialog_button_box::can_reject() const
    {
        for (auto const& b : iButtons)
            if (similar_role(b.first.second, button_role::Reject))
                return true;
        return false;
    }

    bool dialog_button_box::similar_role(button_role aButtonRole1, button_role aButtonRole2)
    {
        switch (aButtonRole1)
        {
        case button_role::Accept:
        case button_role::Yes:
            switch (aButtonRole2)
            {
            case button_role::Accept:
            case button_role::Yes:
                return true;
            default:
                return false;
            }
        case button_role::Reject:
        case button_role::No:
            switch (aButtonRole2)
            {
            case button_role::Reject:
            case button_role::No:
                return true;
            default:
                return false;
            }
        case button_role::Invalid:
            return aButtonRole2 == button_role::Invalid;
        case button_role::Destructive:
            return aButtonRole2 == button_role::Destructive;
        case button_role::Action:
            return aButtonRole2 == button_role::Action;
        case button_role::Apply:
            return aButtonRole2 == button_role::Apply;
        case button_role::Reset:
            return aButtonRole2 == button_role::Reset;
        case button_role::Help:
            return aButtonRole2 == button_role::Help;
        default:
            return false;
        }
    }
}