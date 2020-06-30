// settings.cpp
/*
  neoGFX Design Studio
  Copyright(C) 2020 Leigh Johnston
  
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

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/tools/DesignStudio/Settings.hpp>

namespace neogfx::DesignStudio
{
    settings::settings() : 
        base_type{}
    {
        register_category("environment"_s, "Environment"_t);
        register_group("environment.general"_s, "General"_t);
        register_group("environment.accounts_and_licensing"_s, "Accounts and Licensing"_t);
        register_group("environment.documents"_s, "Documents"_t);
        register_group("environment.fonts_and_colors"_s, "Fonts and Colors"_t);
        register_group("environment.workspace"_s, "Workspace"_t);
        register_group("environment.keyboard"_s, "Keyboard"_t);
        register_group("environment.tabs_and_windows"_s, "Tabs and Windows"_t);
        register_category("text_editor"_s, "Text Editor"_t);
        register_group("text_editor.general"_s, "General"_t);
        register_group("text_editor.advanced"_s, "Advanced"_t);
        register_group("text_editor.tabs_and_indentation"_s, "Tabs and Indentation"_t);
        register_group("text_editor.syntax_highlighting"_s, "Syntax Highlighting"_t);
        register_category("node_editor"_s, "Node Editor"_t);
        register_category("scripting"_s, "Scripting"_t);
        register_category("debugging"_s, "Debugging"_t);
        register_category("projects"_s, "Projects"_t);
        register_category("source_control"_s, "Source Control"_t);
        register_category("team"_s, "Team"_t);
        register_category("plugins"_s, "Plugins"_t);
        
        auto const& themeSetting = register_setting<color>("environment.general.theme"_s, service<i_app>().current_style().palette().color(color_role::Theme), "Color Theme: %?%"_t);
        auto themeChanged = [&](neolib::i_setting const& aSetting)
        {
            if (&aSetting == &themeSetting)
                service<i_app>().current_style().palette().set_color(color_role::Theme, aSetting.modified() ? aSetting.new_value<color>() : aSetting.value<color>());
        };
        SettingChanging(themeChanged);
        SettingChanged(themeChanged);
        themeChanged(themeSetting);

        if (modified())
            save();
    }
}