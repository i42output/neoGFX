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
        register_group("environment.toolbars"_s, "Toolbars"_t);
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

        auto& toolbarIconSize = register_setting<toolbar_icon_size>("environment.toolbars.icon_size"_s, toolbar_icon_size::Size24x24, "Icon size: %?%"_t);
        register_friendly_text(toolbarIconSize, "Size16x16"_s, "16dp x 16dp"_s);
        register_friendly_text(toolbarIconSize, "Size24x24"_s, "24dp x 24dp"_s);
        register_friendly_text(toolbarIconSize, "Size32x32"_s, "32dp x 32dp"_s);
        register_friendly_text(toolbarIconSize, "Size48x48"_s, "48dp x 48dp"_s);
        register_friendly_text(toolbarIconSize, "Size64x64"_s, "64dp x 64dp"_s);
        register_setting<bool>("environment.tabs_and_windows.autoscale_docks"_s, false, "Autoscale docks: %?%"_t);
        register_setting<ng::size>("environment.tabs_and_windows.workspace_size"_s, {});
        register_setting<ng::point>("environment.tabs_and_windows.workspace_position"_s, {});
        register_setting<double>("environment.tabs_and_windows.left_dock_width"_s, 192.0);
        register_setting<double>("environment.tabs_and_windows.right_dock_width"_s, 256.0);
        register_setting<double>("environment.tabs_and_windows.left_dock_weight"_s, 0.3);
        register_setting<double>("environment.tabs_and_windows.right_dock_weight"_s, 0.3);
        register_setting<color>("environment.fonts_and_colors.theme"_s, service<i_app>().current_style().palette().color(color_role::Theme), "Theme color: %?%"_t);
        register_setting<font_info>("environment.fonts_and_colors.font"_s, service<i_app>().current_style().font_info(), "Font: %?%"_t);
        register_setting<bool>("environment.fonts_and_colors.subpixel"_s, false, "Subpixel text rendering: %?%"_t);
        register_setting<workspace_grid>("environment.workspace.grid_type"_s, workspace_grid::Lines, "Grid type : %?% Grid size: %environment.workspace.grid_size:?%"_t);
        register_setting<uint32_t>("environment.workspace.grid_size"_s, 20, ng::setting_constraints<uint32_t>{ false, false, 2, 64, 2 });
        register_setting<gradient>("environment.workspace.grid_color"_s, gradient{ service<i_app>().current_style().palette().color(color_role::Theme).with_alpha(0.25) }, { true }, "Grid color: %?%"_t);

        if (modified())
            apply_changes();
    }

    settings::~settings()
    {
        apply_changes();
    }
}