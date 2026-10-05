// telnet.hpp
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

#include <neolib/io/packet_stream.hpp>

#include <neogfx/tools/DesignStudio/console_client.hpp>

#include <neolib/secure/secure_string.hpp>

namespace neogfx::DesignStudio
{
    // a binary packet whose contents are held in memory that is zeroed before being freed, so that
    // anything typed into the session (e.g. passwords) is erased from every copy of every packet
    class secure_binary_packet : public neolib::i_packet
    {
    public:
        typedef std::vector<char, neolib::secure_allocator<char>> contents_type;
    public:
        secure_binary_packet(contents_type const& aContents = contents_type{}) :
            iContents(aContents)
        {
        }
        secure_binary_packet(void const* aPointer, size_type aLength) :
            iContents(static_cast<const_pointer>(aPointer), static_cast<const_pointer>(aPointer) + aLength)
        {
        }
        secure_binary_packet(secure_binary_packet const& aOther) :
            iContents(aOther.iContents)
        {
        }
        ~secure_binary_packet()
        {
            clear();
        }
        secure_binary_packet& operator=(secure_binary_packet const& aOther)
        {
            if (this != &aOther)
            {
                clear();
                iContents = aOther.iContents;
            }
            return *this;
        }
    public:
        const_pointer data() const final
        {
            if (empty())
                throw packet_empty();
            return &iContents[0];
        }
        pointer data() final
        {
            if (empty())
                throw packet_empty();
            return &iContents[0];
        }
        size_type length() const final
        {
            return iContents.size();
        }
        bool has_max_length() const final
        {
            return false;
        }
        size_type max_length() const final
        {
            return iContents.max_size();
        }
        void clear() final
        {
            // zero the whole capacity as clear() keeps the allocation
            iContents.resize(iContents.capacity());
            neolib::secure_erase(iContents.data(), iContents.size());
            iContents.clear();
        }
        bool take_some(const_pointer& aFirst, const_pointer aLast) final
        {
            if (aFirst == aLast)
                return false;
            iContents.insert(iContents.end(), aFirst, aLast);
            aFirst = aLast;
            return true;
        }
        clone_pointer clone() const final
        {
            return clone_pointer(new secure_binary_packet(*this));
        }
        void copy_from(neolib::i_packet const& aSource) final
        {
            clear();
            if (aSource.length() != 0)
                iContents.assign(aSource.data(), aSource.data() + aSource.length());
        }
    public:
        contents_type const& contents() const
        {
            return iContents;
        }
        contents_type& contents()
        {
            return iContents;
        }
    private:
        contents_type iContents;
    };

    class telnet : public console_client
    {
    private:
        typedef secure_binary_packet packet_type;
        typedef neolib::packet_stream<packet_type, neolib::tcp_protocol> stream_type;
    public:
        define_event(ConnectionFailure, connection_failure, const boost::system::error_code&)
        define_event(Disconnected, disconnected)
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
            connection().send_packet(packet_type::contents_type{
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
            connection().send_packet(packet_type{ aText.data(), aText.length() });
        }
        void connect(std::string const& aHost)
        {
            iConnection.emplace(neolib::service<neolib::i_async_task>(), aHost, 23);
            connection().PacketArrived([&](packet_type const& aData)
            {
                iBuffer.insert(iBuffer.end(), aData.data(), aData.data() + aData.length());
                process_buffer();
            });
            connection().ConnectionFailure([&](const boost::system::error_code& aError)
            {
                ConnectionFailure(aError);
            });
            connection().ConnectionClosed([&]()
            {
                Disconnected();
            });
        }
    private:
        stream_type& connection()
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
                                    connection().send_packet(packet_type::contents_type{
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(code::WILL),
                                        static_cast<char>(sub_command::Echo) });
                                    break;
                                case sub_command::TerminalType:
                                    connection().send_packet(packet_type::contents_type{ 
                                        static_cast<char>(code::IAC), static_cast<char>(code::WILL), 
                                        static_cast<char>(sub_command::TerminalType) });
                                    connection().send_packet(packet_type::contents_type{
                                        static_cast<char>(code::IAC), 
                                        static_cast<char>(command::Suboption), 
                                        static_cast<char>(sub_command::TerminalType),
                                        '\0', 'X', 'T', 'E', 'R', 'M',
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(command::SuboptionEnd) });
                                    break;
                                case sub_command::NegotiateAboutWindowSize:
                                    connection().send_packet(packet_type::contents_type{
                                        static_cast<char>(code::IAC), static_cast<char>(code::WILL),
                                        static_cast<char>(sub_command::NegotiateAboutWindowSize) });
                                    connection().send_packet(packet_type::contents_type{
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(command::Suboption),
                                        static_cast<char>(sub_command::NegotiateAboutWindowSize),
                                        static_cast<char>(iWindowWidth >> 8u), static_cast<char>(iWindowWidth),
                                        static_cast<char>(iWindowHeight >> 8u), static_cast<char>(iWindowHeight),
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(command::SuboptionEnd) });
                                    break;
                                default:
                                    connection().send_packet(packet_type::contents_type{
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
                                    connection().send_packet(packet_type::contents_type{
                                        static_cast<char>(code::IAC),
                                        static_cast<char>(code::WONT),
                                        static_cast<char>(sub_command::Echo) });
                                    break;
                                default:
                                    connection().send_packet(packet_type::contents_type{
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
                Output(someText);
        }
    private:
        std::optional<stream_type> iConnection;
        std::uint16_t iWindowWidth = 80;
        std::uint16_t iWindowHeight = 25;
        std::vector<std::uint8_t> iBuffer;
    };
}
