// console_client.hpp
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

#include <neogfx/tools/DesignStudio/i_console_client.hpp>

namespace neogfx::DesignStudio
{
    class console_client : public reference_counted<i_console_client>, public neolib::lifetime<>
    {
    public:
        define_declared_event(Output, output, std::string const&)
    };

    class console : public console_client
    {
    public:
        define_event(Command, command, std::string const&)
    public:
        console(std::string const& aName = {}) :
            iName{ aName }
        {
        }
    public:
        void start() final
        {
            output()(iName + ">");
        }
        void resize_window(std::uint16_t aWidth, std::uint16_t aHeight) final
        {
        }
        void input(std::string const& aText) final
        {
            destroyed_flag destroyed{ *this };

            if (aText == "\x7F")
            {
                output()("\b\x1B[K");
                if (!iBuffer.empty())
                    iBuffer.pop_back();
                return;
            }

            output()(aText);
            iBuffer += aText;
            std::size_t next;
            while (!destroyed && (next = iBuffer.find("\r\n")) != std::string::npos)
            {
                auto const nextCommand = iBuffer.substr(0, next);
                iBuffer = iBuffer.substr(next + 2);
                if (!nextCommand.empty())
                    command()(nextCommand);
                if (!destroyed)
                    output()(iName + ">");
            }
        }
    private:
        std::string iName;
        std::string iBuffer;
    };
}
