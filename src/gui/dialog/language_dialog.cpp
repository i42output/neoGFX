// language_dialog.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.
  
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

#include <boost/algorithm/string/predicate.hpp>

#include <neolib/core/scoped.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/dialog/language_dialog.hpp>

namespace neogfx
{
    language_dialog::language_dialog() :
        dialog{ "Language"_t, window_style::Dialog | window_style::Modal | window_style::TitleBar | window_style::Close },
        iPreviousLanguage{ service<i_app>().language() },
        iLayout{ client_layout() },
        iPrompt{ iLayout, "Choose the language of the user interface:"_t },
        iLanguageList{ iLayout }
    {
        init();
    }

    language_dialog::language_dialog(i_widget& aParent) :
        dialog{ aParent, "Language"_t, window_style::Dialog | window_style::Modal | window_style::TitleBar | window_style::Close },
        iPreviousLanguage{ service<i_app>().language() },
        iLayout{ client_layout() },
        iPrompt{ iLayout, "Choose the language of the user interface:"_t },
        iLanguageList{ iLayout }
    {
        init();
    }

    language_dialog::~language_dialog()
    {
    }

    void language_dialog::init()
    {
        auto& app = service<i_app>();
        iLanguages.push_back(string{});
        for (auto const& language : app.languages())
            iLanguages.push_back(string{ language });

        iLanguageList.set_minimum_size(size{ 256.0_dip, 128.0_dip });
        iLanguageList.presentation_model().set_column_read_only(0u);
        {
            neolib::scoped_flag sf{ iUpdating };
            for (std::uint32_t row = 0u; row < iLanguages.size(); ++row)
                iLanguageList.model().insert_item(item_model_index{ row }, string{});
            update_language_names();
        }

        // the language used (or, failing that, its primary language (e.g. "de" for "de-AT")) is the one chosen to begin with
        std::string const current = app.language().to_std_string();
        std::uint32_t currentRow = 0u;
        for (auto const& candidate : { current, current.substr(0, current.find('-')) })
        {
            auto const match = std::find_if(std::next(iLanguages.begin()), iLanguages.end(), [&](string const& aLanguage)
                { return boost::iequals(aLanguage.to_std_string(), candidate); });
            if (match != iLanguages.end())
            {
                currentRow = static_cast<std::uint32_t>(std::distance(iLanguages.begin(), match));
                break;
            }
        }
        {
            neolib::scoped_flag sf{ iUpdating };
            iLanguageList.selection_model().set_current_index(iLanguageList.presentation_model().from_item_model_index(item_model_index{ currentRow }));
        }

        // (a language chosen is used at once: the dialog itself (and the rest of the user interface) is shown in it)
        iSink += iLanguageList.selection_model().current_index_changed([this](optional_item_presentation_model_index const& aCurrentIndex, optional_item_presentation_model_index const&)
        {
            if (iUpdating || !aCurrentIndex)
                return;
            auto const row = iLanguageList.presentation_model().to_item_model_index(*aCurrentIndex).row();
            if (row < iLanguages.size())
                service<i_app>().set_language(iLanguages[row]);
        });
        iSink += app.language_changed([this]() { update_language_names(); });
        iSink += Rejected([this]() { service<i_app>().set_language(iPreviousLanguage); });

        button_box().add_button(standard_button::Ok);
        button_box().add_button(standard_button::Cancel);
        center_on_parent();
        set_ready_to_render(true);
    }

    // the languages' names (in the language used)
    void language_dialog::update_language_names()
    {
        neolib::scoped_flag sf{ iUpdating };
        for (std::uint32_t row = 0u; row < iLanguages.size(); ++row)
            iLanguageList.model().update_cell_data(item_model_index{ row, 0u },
                row == 0u ? string{ "English"_t } : string{ translate(language_name(iLanguages[row])) });
    }
}