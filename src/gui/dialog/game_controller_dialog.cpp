// game_controller_dialog.cpp
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
#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/dialog/game_controller_dialog.hpp>
#include <neogfx/gui/dialog/message_box.hpp>

namespace neogfx
{
    game_controller_dialog::game_controller_dialog() :
        dialog{ "Game Controller Settings"_t, window_style::Dialog | window_style::Modal | window_style::TitleBar | window_style::Close },
        iControllerSelectorGroupBox{ client_layout(), "Installed Game Controllers"_t },
        iControllerSelector{ iControllerSelectorGroupBox.item_layout() },
        iTabs{ client_layout(), false, tab_container_style::TabAlignmentTop | tab_container_style::ResizeToPages },
        iCalibrateTab{ iTabs, "Calibrate"_t },
        iTestTab{ iTabs, "Test"_t },
        iLayout1{ iCalibrateTab },
        iSchematicGroupBox{ iLayout1, "Button Mappings"_t },
        iSchematic{ iSchematicGroupBox.item_layout(), image{ ":/neogfx/resources/images/game_controller.png" } },
        iLayout2{ iTestTab },
        iTestGroupBox{ iLayout2, "" },
        iTestOutput{ iTestGroupBox.item_layout() }
    {
        init();
    }

    game_controller_dialog::game_controller_dialog(i_widget& aParent) :
        dialog{ aParent, "Game Controller Settings", window_style::Dialog | window_style::Modal | window_style::TitleBar | window_style::Close },
        iControllerSelectorGroupBox{ client_layout(), "Installed Game Controllers"_t },
        iControllerSelector{ iControllerSelectorGroupBox.item_layout() },
        iTabs{ client_layout(), false, tab_container_style::TabAlignmentTop | tab_container_style::ResizeToPages },
        iCalibrateTab{ iTabs, "Calibrate"_t },
        iTestTab{ iTabs, "Test"_t },
        iLayout1{ iCalibrateTab },
        iSchematicGroupBox{ iLayout1, "Button Mappings"_t },
        iSchematic{ iSchematicGroupBox.item_layout(), image{ ":/neogfx/resources/images/game_controller.png" } },
        iLayout2{ iTestTab },
        iTestGroupBox{ iLayout2, "" },
        iTestOutput{ iTestGroupBox.item_layout() }
    {
        init();
    }

    game_controller_dialog::~game_controller_dialog()
    {
    }

    void game_controller_dialog::init()
    {
        auto gameControllerModel = make_ref<basic_item_model<ref_ptr<i_game_controller>>>();
        iControllerSelector.set_model(gameControllerModel);
        iControllerSelector.SelectionChanged([&, gameControllerModel](optional_item_model_index aIndex)
        {
            iSink.clear();
            if (aIndex)
            {
                auto& controller = *gameControllerModel->item(*aIndex);
                iSink += controller.raw_button_pressed([&](game_controller_button_ordinal aButtonOrdinal, key_modifiers_e aKeyboardState)
                {
                    if (!controller.button_mapped(aButtonOrdinal))
                    {
                        std::ostringstream oss;
                        oss << "Button #" << aButtonOrdinal << " pressed\n";
                        iTestOutput.append_text(oss.str(), true);
                    }
                });
                iSink += controller.raw_button_released([&](game_controller_button_ordinal aButtonOrdinal, key_modifiers_e aKeyboardState)
                {
                    if (!controller.button_mapped(aButtonOrdinal))
                    {
                        std::ostringstream oss;
                        oss << "Button #" << aButtonOrdinal << " released\n";
                        iTestOutput.append_text(oss.str(), true);
                    }
                });
                iSink += controller.pov_moved([&](game_controller_pov_ordinal aPovOrdinal, vec2 const& aPosition, key_modifiers_e aKeyboardState)
                {
                    std::ostringstream oss;
                    oss << "POV #" << aPovOrdinal << " moved to " << aPosition << "\n";
                    iTestOutput.append_text(oss.str(), true);
                });
            }
        });

        for (auto const& controller : service<i_game_controllers>().controllers())
        {
            gameControllerModel->insert_item(iControllerSelector.model().rows(), controller, controller->product_name());
            if (controller->player_assigned() && controller->player() == game_player::One)
                iControllerSelector.selection_model().set_current_index(iControllerSelector.presentation_model().rows() - 1);
        }

        iControllerSelector.accept_selection();

        iTestGroupBox.set_size_policy(size_constraint::Expanding);
        iTestOutput.set_read_only(true);

        iTabs.page_layout().set_ignore_child_visibility(true);

        center_on_parent();
        set_ready_to_render(true);
    }
}