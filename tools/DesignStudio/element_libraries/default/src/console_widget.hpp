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
    class telnet
    {
    public:
        define_event(Text, text, std::string const&)
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
        telnet(std::string const& aHost) :
            iConnection{ neolib::service<neolib::i_async_task>(), aHost, 23 }
        {
            iConnection.PacketArrived([&](neolib::binary_packet const& aData)
            {
                iBuffer.insert(iBuffer.end(), aData.data(), aData.data() + aData.length());
                process_buffer();
            });
        }
    public:
        void resize_window(std::uint16_t aWidth, std::uint16_t aHeight)
        {
            iWindowWidth = aWidth;
            iWindowHeight = aHeight;
            iConnection.send_packet(neolib::binary_packet::contents_type{
                static_cast<char>(code::IAC),
                static_cast<char>(command::Suboption),
                static_cast<char>(sub_command::NegotiateAboutWindowSize),
                static_cast<char>(iWindowWidth >> 8u), static_cast<char>(iWindowWidth),
                static_cast<char>(iWindowHeight >> 8u), static_cast<char>(iWindowHeight),
                static_cast<char>(code::IAC),
                static_cast<char>(command::SuboptionEnd) });
        }
        void send(std::string const& aText)
        {
            iConnection.send_packet(neolib::binary_packet{ aText.data(), aText.length() });
        }
    private:
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
                                    iConnection.send_packet(neolib::binary_packet::contents_type{
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(code::WILL),
                                        static_cast<char>(sub_command::Echo) });
                                    break;
                                case sub_command::TerminalType:
                                    iConnection.send_packet(neolib::binary_packet::contents_type{ 
                                        static_cast<char>(code::IAC), static_cast<char>(code::WILL), 
                                        static_cast<char>(sub_command::TerminalType) });
                                    iConnection.send_packet(neolib::binary_packet::contents_type{
                                        static_cast<char>(code::IAC), 
                                        static_cast<char>(command::Suboption), 
                                        static_cast<char>(sub_command::TerminalType),
                                        '\0', 'X', 'T', 'E', 'R', 'M',
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(command::SuboptionEnd) });
                                    break;
                                case sub_command::NegotiateAboutWindowSize:
                                    iConnection.send_packet(neolib::binary_packet::contents_type{
                                        static_cast<char>(code::IAC), static_cast<char>(code::WILL),
                                        static_cast<char>(sub_command::NegotiateAboutWindowSize) });
                                    iConnection.send_packet(neolib::binary_packet::contents_type{
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(command::Suboption),
                                        static_cast<char>(sub_command::NegotiateAboutWindowSize),
                                        static_cast<char>(iWindowWidth >> 8u), static_cast<char>(iWindowWidth),
                                        static_cast<char>(iWindowHeight >> 8u), static_cast<char>(iWindowHeight),
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(command::SuboptionEnd) });
                                    break;
                                default:
                                    iConnection.send_packet(neolib::binary_packet::contents_type{
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
                                    iConnection.send_packet(neolib::binary_packet::contents_type{
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(code::WONT),
                                        static_cast<char>(sub_command::Echo) });
                                    break;
                                default:
                                    iConnection.send_packet(neolib::binary_packet::contents_type{
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
                Text.trigger(someText);
        }
    private:
        neolib::tcp_binary_packet_stream iConnection;
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
            iTerminal{ client_layout() },
            iTestConnection{ "172.16.0.2" }
        {
            ref_ptr<i_settings> settings{ aElement.library().application() };

            iTerminal.TerminalResized([&](terminal::size_type aTerminalSize)
            {
                basic_size<std::uint16_t> newSize(aTerminalSize);
                iTestConnection.resize_window(newSize.cx, newSize.cy);
            });
            iTerminal.Input([&](i_string const& aText)
            {
                iTestConnection.send(aText.to_std_string());
            });
            iTestConnection.Text([&](std::string const& aText)
            {
                iTerminal.output(string{ aText });
            });

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
        terminal iTerminal;
        telnet iTestConnection;
        sink iSink;
    };

    template <>
    struct element_traits<console_widget> : element_traits<i_element>
    {
        static constexpr bool needsCaddy = false;
    };
}
