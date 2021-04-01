// i_ide.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2021 Leigh Johnston
  
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

#include <neogfx/app/i_action.hpp>
#include <neogfx/gui/widget/i_menu.hpp>
#include <neogfx/tools/DesignStudio/DesignStudio.hpp>

namespace neogfx::DesignStudio
{
    uuid constexpr ToolsMenu{ 0x486a96f9, 0x397e, 0x4005, 0x80c9, { 0xa7, 0xb8, 0xeb, 0x29, 0xf5, 0x9a } };

    class i_ide : public ng::i_reference_counted
    {
    public:
        typedef i_ide abstract_type;
    public:
        virtual void add_action(uuid const& aMenuId, i_action& aAction) = 0;
        virtual void add_sub_menu(uuid const& aMenuId, i_menu& aSubMenu) = 0;
    public:
        static uuid const& iid() { static uuid const sIid{ 0x7ca9ec87, 0xda34, 0x4c28, 0xb843, { 0xc1, 0x3e, 0x93, 0x81, 0xe0, 0xcf } }; return sIid; }
    };
}
