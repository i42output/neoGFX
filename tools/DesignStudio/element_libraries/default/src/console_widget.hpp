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
#include <boost/format.hpp>
#include <neolib/io/packet_stream.hpp>
#include <neogfx/app/settings.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/widget/terminal.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>
#include <neogfx/tools/DesignStudio/i_element_library.hpp>

namespace neogfx::DesignStudio
{
    class i_console_client
    {
    public:
        declare_event(output, std::string const&)
    public:
        virtual ~i_console_client() = default;
    public:
        virtual void start() = 0;
        virtual void resize_window(std::uint16_t aWidth, std::uint16_t aHeight) = 0;
        virtual void input(std::string const& aText) = 0;
    };

    class console_client : public i_console_client, public neolib::lifetime<>
    {
    public:
        define_declared_event(Output, output, std::string const&)
    };

    class console : public console_client
    {
    public:
        define_event(Command, command, std::string const&)
    public:
        console()
        {
        }
    public:
        void start() final
        {
            output().trigger(">");
        }
        void resize_window(std::uint16_t aWidth, std::uint16_t aHeight) final
        {
        }
        void input(std::string const& aText) final
        {
            destroyed_flag destroyed{ *this };

            if (aText == "\x7F")
            {
                output().trigger("\b\x1B[K");
                if (!iBuffer.empty())
                    iBuffer.pop_back();
                return;
            }

            output().trigger(aText);
            iBuffer += aText;
            std::size_t next;
            while (!destroyed && (next = iBuffer.find("\r\n")) != std::string::npos)
            {
                auto const nextCommand = iBuffer.substr(0, next);
                iBuffer = iBuffer.substr(next + 2);
                if (!nextCommand.empty())
                    command().trigger(nextCommand);
                if (!destroyed)
                    output().trigger(">");
            }
        }
    private:
        std::string iBuffer;
    };

    class telnet : public console_client
    {
    private:
        enum class code : std::uint8_t
        {
            SE                  = 240,
            NOP                 = 241,
            DataMark            = 242,
            Break               = 243,
            InterruptProcess    = 244,
            AbortOutput         = 245,
            AreYouThere         = 246,
            EraseCharacter      = 247,
            EraseLine           = 248,
            GoAhead             = 249,
            SB                  = 250,
            WILL                = 251,
            WONT                = 252,
            DO                  = 253,
            DONT                = 254,
            IAC                 = 255
        };
        enum class sub_command : std::uint8_t
        {   
            Echo                        = 0x01,
            TerminalType                = 0x18,
            NegotiateAboutWindowSize    = 0x1F,
        };
        enum class command : std::uint8_t 
        {
            SuboptionEnd    = 240,
            Suboption       = 250
        };
    public:
        telnet()
        {
        }
    public:
        void start() final
        {
        }
        void resize_window(std::uint16_t aWidth, std::uint16_t aHeight) final
        {
            iWindowWidth = aWidth;
            iWindowHeight = aHeight;
            connection().send_packet(neolib::binary_packet::contents_type{
                static_cast<char>(code::IAC),
                static_cast<char>(command::Suboption),
                static_cast<char>(sub_command::NegotiateAboutWindowSize),
                static_cast<char>(iWindowWidth >> 8u), static_cast<char>(iWindowWidth),
                static_cast<char>(iWindowHeight >> 8u), static_cast<char>(iWindowHeight),
                static_cast<char>(code::IAC),
                static_cast<char>(command::SuboptionEnd) });
        }
        void input(std::string const& aText) final
        {
            connection().send_packet(neolib::binary_packet{ aText.data(), aText.length() });
        }
        void connect(std::string const& aHost)
        {
            iConnection.emplace(neolib::service<neolib::i_async_task>(), aHost, 23);
            connection().PacketArrived([&](neolib::binary_packet const& aData)
            {
                iBuffer.insert(iBuffer.end(), aData.data(), aData.data() + aData.length());
                process_buffer();
            });
        }
    private:
        neolib::tcp_binary_packet_stream& connection()
        {
            return iConnection.value();
        }
        void process_buffer()
        {
            thread_local std::vector<std::uint8_t> temp;
            temp = iBuffer;
            iBuffer.clear();
            thread_local std::string someText;
            someText.clear();
            for (auto i = temp.begin(); i != temp.end();)
            {
                bool needMore = false;
                auto next = *i++;

                if (static_cast<code>(next) == code::IAC)
                {
                    if (i == temp.end())
                        needMore = true;
                    else
                    {
                        next = *i++;
                        switch (static_cast<code>(next))
                        {
                        case code::SE:
                            break;
                        case code::NOP:
                            break;
                        case code::DataMark:
                            break;
                        case code::Break:
                            break;
                        case code::InterruptProcess:
                            break;
                        case code::AbortOutput:
                            break;
                        case code::AreYouThere:
                            break;
                        case code::EraseCharacter:
                            break;
                        case code::EraseLine:
                            break;
                        case code::GoAhead:
                            break;
                        case code::SB:
                            break;
                        case code::WILL:
                            if (i == temp.end())
                                needMore = true;
                            else
                                next = *i++;
                            break;
                        case code::WONT:
                            if (i == temp.end())
                                needMore = true;
                            else
                                ++i;
                            break;
                        case code::DO:
                            if (i == temp.end())
                                needMore = true;
                            else
                            {
                                next = *i++;
                                switch (static_cast<sub_command>(next))
                                {
                                case sub_command::Echo:
                                    connection().send_packet(neolib::binary_packet::contents_type{
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(code::WILL),
                                        static_cast<char>(sub_command::Echo) });
                                    break;
                                case sub_command::TerminalType:
                                    connection().send_packet(neolib::binary_packet::contents_type{ 
                                        static_cast<char>(code::IAC), static_cast<char>(code::WILL), 
                                        static_cast<char>(sub_command::TerminalType) });
                                    connection().send_packet(neolib::binary_packet::contents_type{
                                        static_cast<char>(code::IAC), 
                                        static_cast<char>(command::Suboption), 
                                        static_cast<char>(sub_command::TerminalType),
                                        '\0', 'X', 'T', 'E', 'R', 'M',
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(command::SuboptionEnd) });
                                    break;
                                case sub_command::NegotiateAboutWindowSize:
                                    connection().send_packet(neolib::binary_packet::contents_type{
                                        static_cast<char>(code::IAC), static_cast<char>(code::WILL),
                                        static_cast<char>(sub_command::NegotiateAboutWindowSize) });
                                    connection().send_packet(neolib::binary_packet::contents_type{
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(command::Suboption),
                                        static_cast<char>(sub_command::NegotiateAboutWindowSize),
                                        static_cast<char>(iWindowWidth >> 8u), static_cast<char>(iWindowWidth),
                                        static_cast<char>(iWindowHeight >> 8u), static_cast<char>(iWindowHeight),
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(command::SuboptionEnd) });
                                    break;
                                default:
                                    connection().send_packet(neolib::binary_packet::contents_type{
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(code::WONT),
                                        static_cast<char>(next) });
                                    break;
                                }
                            }
                            break;
                        case code::DONT:
                            if (i == temp.end())
                                needMore = true;
                            else
                            {
                                next = *i++;
                                switch (static_cast<sub_command>(next))
                                {
                                case sub_command::Echo:
                                    connection().send_packet(neolib::binary_packet::contents_type{
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(code::WONT),
                                        static_cast<char>(sub_command::Echo) });
                                    break;
                                default:
                                    connection().send_packet(neolib::binary_packet::contents_type{
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(code::WONT),
                                        static_cast<char>(next) });
                                    break;
                                }
                            }
                            break;
                        case code::IAC:
                            someText.push_back(next);
                            i = temp.erase(temp.begin(), i);
                            break;
                        }
                    }
                }
                else
                    someText.push_back(next);

                if (needMore)
                    break;

                i = temp.erase(temp.begin(), i);
            }
            if (!temp.empty())
                iBuffer = temp;
            if (!someText.empty())
                Output.trigger(someText);
        }
    private:
        std::optional<neolib::tcp_binary_packet_stream> iConnection;
        std::uint16_t iWindowWidth = 80;
        std::uint16_t iWindowHeight = 25;
        std::vector<std::uint8_t> iBuffer;
    };

    class console_widget : public window
    {
        typedef window base_type;
    public:
        console_widget(i_element& aElement, i_widget& aParent, window_style aStyle) :
            window{ aParent, window_placement{ rect{ point{}, size{ 480.0_dip, 480.0_dip } } }, aStyle | window_style::SizeToContents },
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
                iTerminal.set_text_attributes(consoleExtendedFont.second());
            };
            consoleExtendedFontSetting.changed(font_changed);
            font_changed();

            create_status_bar<neogfx::status_bar>(neogfx::status_bar::style::DisplayMessage | neogfx::status_bar::style::DisplaySizeGrip);
            status_bar().set_font(iTerminal.font());

            create_console<>();
        }
        ~console_widget()
        {
        }
    public:
        void resized() override
        {
            window::resized();
            resize(ideal_size());
        }
    private:
        template <typename ConsoleT = console>
        ConsoleT& create_console()
        {
            iSink.clear();

            iConsole = std::make_unique<ConsoleT>();
            auto& newConsole = static_cast<ConsoleT&>(*iConsole);

            iSink += iTerminal.TerminalResized([&](terminal::size_type aTerminalSize)
                {
                    basic_size<std::uint16_t> newSize(aTerminalSize);
                    newConsole.resize_window(newSize.cx, newSize.cy);
                });
            iSink += iTerminal.Input([&](i_string const& aText)
                {
                    newConsole.input(aText.to_std_string());
                });
            iSink += newConsole.output([&](std::string const& aText)
                {
                    iTerminal.output(string{ aText });
                });

            if constexpr (std::is_same_v<ConsoleT, console>)
            {
                iSink += newConsole.command([&](std::string const& aCommand)
                {
                    auto bits = neolib::tokens(aCommand, " "s);
                    if (bits.size() >= 2)
                    {
                        if (bits[0] == "telnet")
                        {
                            create_console<telnet>().connect(bits[1]);
                        }
                    }
                });
            }

            newConsole.start();

            return newConsole;
        }
    private:
        terminal iTerminal;
        std::unique_ptr<i_console_client> iConsole;
        sink iSink;
    };

    template <>
    struct element_traits<console_widget> : element_traits<i_element>
    {
        static constexpr bool needsCaddy = false;
    };
}
