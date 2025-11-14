// script_widget.hpp
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

#include <neogfx/neogfx.hpp>

#include <neogfx/app/settings.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/widget/terminal.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>
#include <neogfx/tools/DesignStudio/i_element_library.hpp>
#include <neogfx/tools/DesignStudio/console_client.hpp>
#include "telnet.hpp"

namespace neogfx::DesignStudio
{
    class console_widget : public window
    {
        typedef window base_type;
    public:
        console_widget(i_element& aElement, i_widget& aParent, window_style aStyle) :
            window{ aParent, window_placement{ rect{ point{}, size{ 480.0_dip, 480.0_dip } } }, aStyle | window_style::SizeToContents },
            iClientManager{ aElement.library().application() },
            iTerminal{ client_layout() }
        {
            ref_ptr<i_settings> settings{ aElement.library().application() };

            title_bar().set_icon(aElement.library().element_icon(aElement.type()));
            title_bar().set_title(""_s);

            auto& consoleExtendedFontSetting = settings->setting("environment.fonts_and_colors.console_font"_s);
            auto font_changed = [&]
            {
                auto const& consoleExtendedFont = consoleExtendedFontSetting.value<neogfx::extended_font>(true);
                iTerminal.set_font(consoleExtendedFont.first());
                iTerminal.set_text_format(consoleExtendedFont.second());
            };
            consoleExtendedFontSetting.changed(font_changed);
            font_changed();

            create_status_bar<neogfx::status_bar>(neogfx::status_bar::style::DisplayMessage | neogfx::status_bar::style::DisplaySizeGrip);
            status_bar().set_font(iTerminal.font());

            create_console<>();

            iClientManager->start_console_client_session(
                [&](i_terminal& aTerminal, i_string const& aCommand, i_ref_ptr<i_console_client>& aConsole)
                {
                    if (&aTerminal == &iTerminal)
                    {
                        auto bits = neolib::tokens(aCommand.to_std_string(), " "s);
                        if (bits.size() >= 2)
                        {
                            if (bits[0] == "telnet")
                            {
                                auto& telnetSession = create_console<telnet>();
                                aConsole = iConsole;
                                telnetSession.disconnected([&]()
                                    {
                                        create_console<>();
                                    });
                                telnetSession.connection_failure([&](const boost::system::error_code& aError)
                                    {
                                        iTerminal.output(string{ aError.message() + "\r\n" });
                                        create_console<>();
                                    });
                                telnetSession.connect(bits[1]);
                            }
                        }
                    }
                });
        }
        ~console_widget()
        {
        }
    public:
        void resized() override
        {
            window::resized();
            resize(transformed_ideal_size());
        }
    private:
        template <typename ConsoleT = console>
        ConsoleT& create_console()
        {
            attach_console(make_ref<ConsoleT>());
            auto& newConsole = static_cast<ConsoleT&>(*iConsole);

            if constexpr (std::is_same_v<ConsoleT, console>)
                iSink += newConsole.command([&](std::string const& aCommand)
                    {
                        ref_ptr<i_console_client> newConsole;
                        iClientManager->start_console_client_session()(iTerminal, string{ aCommand }, newConsole);
                        if (newConsole)
                            attach_console(newConsole);
                    });

            return newConsole;
        }
        void attach_console(ref_ptr<i_console_client> aConsole)
        {
            if (iConsole && iConsole == aConsole)
                return;

            iSink.clear();

            iConsole = aConsole;

            iSink += iTerminal.TerminalResized([&](terminal::size_type aTerminalSize)
                {
                    basic_size<std::uint16_t> newSize(aTerminalSize);
                    iConsole->resize_window(newSize.cx, newSize.cy);
                });
            iSink += iTerminal.Input([&](i_string const& aText)
                {
                    iConsole->input(aText.to_std_string());
                });
            iSink += iConsole->output([&](std::string const& aText)
                {
                    iTerminal.output(string{ aText });
                });

            iConsole->start();
        }
    private:
        ref_ptr<i_console_client_manager> iClientManager;
        terminal iTerminal;
        ref_ptr<i_console_client> iConsole;
        sink iSink;
    };

    template <>
    struct element_traits<console_widget> : element_traits<i_element>
    {
        static constexpr bool needsCaddy = false;
    };
}
