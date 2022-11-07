// i_console_client.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2022 Leigh Johnston
  
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/i_terminal.hpp>

namespace neogfx::DesignStudio
{
    class i_console_client : public i_reference_counted
    {
    public:
        typedef i_console_client abstract_type;
    public:
        declare_event(output, std::string const&)
    public:
        virtual ~i_console_client() = default;
    public:
        virtual void start() = 0;
        virtual void resize_window(std::uint16_t aWidth, std::uint16_t aHeight) = 0;
        virtual void input(std::string const& aText) = 0;
    };

    class i_console_client_manager : public i_reference_counted
    {
    public:
        typedef i_console_client_manager abstract_type;
    public:
        declare_event(start_console_client_session, i_terminal&, i_string const&, i_ref_ptr<i_console_client>&)
    public:
        static uuid const& iid() { static uuid const sIid{ 0x1a38eb2c, 0x220a, 0x437c, 0x9c85, { 0xaf, 0x83, 0x23, 0x78, 0x3c, 0xd4 } }; return sIid; }
    };
}
