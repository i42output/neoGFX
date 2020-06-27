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
        auto const& themeSetting = register_setting<color>("environment.general.theme"_s, service<i_app>().current_style().palette().color(color_role::Theme), "Color Theme: %?%"_t);
        
        if (dirty())
            save();

        service<i_app>().current_style().palette().set_color(color_role::Theme, themeSetting.value<color>());

        SettingChanged([&](neolib::i_setting const& aSetting)
        {
            if (&aSetting == &themeSetting)
                service<i_app>().current_style().palette().set_color(color_role::Theme, aSetting.value<color>());
        });
    }
}