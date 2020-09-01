// settings.hpp
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
#include <neogfx/gfx/color.hpp>
#include <neogfx/app/settings.hpp>

namespace neogfx::DesignStudio
{
    enum class workspace_grid : int32_t
    {
        Lines   = 0x00000000,
        Quads   = 0x00000001,
        Points  = 0x00000002
    };

    enum class toolbar_icon_size : int32_t
    {
        Size16x16 = 0x00000000,
        Size24x24 = 0x00000001,
        Size32x32 = 0x00000002,
        Size48x48 = 0x00000003,
        Size64x64 = 0x00000004
    };
}

template <>
const neolib::enum_enumerators_t<neogfx::DesignStudio::workspace_grid> neolib::enum_enumerators_v<neogfx::DesignStudio::workspace_grid>
{
    declare_enum_string(neogfx::DesignStudio::workspace_grid, Lines)
    declare_enum_string(neogfx::DesignStudio::workspace_grid, Quads)
    declare_enum_string(neogfx::DesignStudio::workspace_grid, Points)
};

template <>
const neolib::enum_enumerators_t<neogfx::DesignStudio::toolbar_icon_size> neolib::enum_enumerators_v<neogfx::DesignStudio::toolbar_icon_size>
{
    declare_enum_string(neogfx::DesignStudio::toolbar_icon_size, Size16x16)
    declare_enum_string(neogfx::DesignStudio::toolbar_icon_size, Size24x24)
    declare_enum_string(neogfx::DesignStudio::toolbar_icon_size, Size32x32)
    declare_enum_string(neogfx::DesignStudio::toolbar_icon_size, Size48x48)
    declare_enum_string(neogfx::DesignStudio::toolbar_icon_size, Size64x64)
};

namespace neogfx::DesignStudio
{
    class settings : public neogfx::settings
    {
        typedef neogfx::settings base_type;
    public:
        settings();
    };
}