// game_controllers.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/neogfx.hpp>

#include <filesystem>

#include <neolib/file/file.hpp>
#include <neolib/core/uuid.hpp>

#include <neogfx/app/i_resource_manager.hpp>
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/hid/hid_devices.hpp>
#include <neogfx/hid/game_controllers.hpp>

namespace neogfx
{
    game_controllers::game_controllers()
    {
        load_database();
    }

    const game_controllers::controller_list& game_controllers::controllers() const
    {
        return iControllers;
    }

    void game_controllers::auto_assign_controllers(bool aAutoAssign)
    {
        iAutoAssign = aAutoAssign;
    }

    bool game_controllers::have_controller_for(game_player aPlayer) const
    {
        for (auto const& controller : controllers())
            if ((*controller).player_assigned() && (*controller).player() == aPlayer)
                return true;
        return false;
    }

    i_game_controller& game_controllers::controller_for(game_player aPlayer) const
    {
        for (auto const& controller : controllers())
            if ((*controller).player_assigned() && (*controller).player() == aPlayer)
                return *controller;
        throw unassigned_player();
    }

    const game_controllers::button_map_type& game_controllers::button_map(const hid_device_uuid& aProductId) const
    {
        // todo
        return iButtonMaps[aProductId];
    }

    abstract_t<game_controllers::controller_list>::iterator game_controllers::add_device(i_game_controller& aController)
    {
        std::optional<game_player> newPlayer;
        if (iAutoAssign)
        {
            newPlayer = game_player::One;
            for (auto const& controller : iControllers)
            {
                if (controller->player_assigned() && controller->player() == newPlayer.value())
                    newPlayer = static_cast<game_player>(static_cast<std::underlying_type_t<game_player>>(newPlayer.value()) + 1u);
            }
        }
        auto newController = iControllers.insert(iControllers.end(), ref_ptr<i_game_controller>{ aController });
        service<i_hid_devices>().add_device(aController);
        if (newPlayer)
            (**newController).assign_player(newPlayer.value());
        ControllerConnected(aController);
        return newController;
    }

    abstract_t<game_controllers::controller_list>::iterator game_controllers::remove_device(i_game_controller& aController)
    {
        neolib::ref_ptr<i_game_controller> detached{ aController };
        auto existing = std::find(iControllers.begin(), iControllers.end(), detached);
        if (existing != iControllers.end())
        {
            auto next = iControllers.erase(existing);
            ControllerDisconnected(*detached);
            service<i_hid_devices>().remove_device(*detached);
            return next;
        }
        return iControllers.end();
    }

    namespace
    {
        std::string game_controller_database_uri()
        {
            if (std::filesystem::exists(neolib::program_directory() + "/gamecontrollerdb.txt"))
                return "file:///" + neolib::program_directory() + "/gamecontrollerdb.txt";
            return ":/neogfx/resources/gamecontrollerdb.txt";
        }

        platform to_platform(std::string const& aPlatform)
        {
            if (aPlatform == "platform:Windows")
                return platform::Windows;
            else if (aPlatform == "platform:Mac OS X")
                return platform::macOS;
            else if (aPlatform == "platform:Linux")
                return platform::Linux;
            else if (aPlatform == "platform:Android")
                return platform::Android;
            else if (aPlatform == "platform:iOS")
                return platform::iOS;
            else
                return platform::Unknown;
        }

        enum class controller_element
        {
            Unknown,
            A,
            B,
            X,
            Y,
            LeftShoulder,
            RightShoulder,
            LeftTrigger,
            RightTrigger,
            LeftStick,
            RightStick,
            DPadUp,
            DPadDown,
            DPadLeft,
            DPagRight,
            LeftX,
            LeftY,
            RightX,
            RightY,
            Paddle1,
            Paddle2,
            Paddle3,
            Paddle4,
            Touchpad,
            Start,
            Back,
            Guide,
            Misc1
        };

        controller_element to_controller_element(std::string const& aPartType)
        {
            static std::unordered_map<std::string, controller_element> const partTypes
            {
                { "a", controller_element::A },
                { "b", controller_element::B },
                { "x", controller_element::X },
                { "y", controller_element::Y },
                { "leftshoulder", controller_element::LeftShoulder },
                { "rightshoulder", controller_element::RightShoulder },
                { "lefttrigger", controller_element::LeftTrigger },
                { "righttrigger", controller_element::RightTrigger },
                { "leftstick", controller_element::LeftStick },
                { "rightstick", controller_element::RightStick },
                { "dpup", controller_element::DPadUp },
                { "dpdown", controller_element::DPadDown },
                { "dpleft", controller_element::DPadLeft },
                { "dpright", controller_element::DPagRight },
                { "leftx", controller_element::LeftX },
                { "lefty", controller_element::LeftY },
                { "rightx", controller_element::RightX },
                { "righty", controller_element::RightY },
                { "paddle1", controller_element::Paddle1 },
                { "paddle2", controller_element::Paddle2 },
                { "paddle3", controller_element::Paddle3 },
                { "paddle4", controller_element::Paddle4 },
                { "touchpad", controller_element::Touchpad },
                { "start", controller_element::Start },
                { "back", controller_element::Back },
                { "guide", controller_element::Guide },
                { "misc1", controller_element::Misc1 },
            };

            auto const result = partTypes.find(aPartType);
            if (result != partTypes.end())
                return result->second;
            return controller_element::Unknown;
        }
    }

    void game_controllers::load_database()
    {
        auto resource = service<i_resource_manager>().load_resource(game_controller_database_uri());
        std::istringstream lines{ std::string{ static_cast<const char*>(resource->data()), resource->size() } };
        std::string line;
        while (std::getline(lines, line))
        {
            if (line.empty() || line[0] == '#')
                continue;
            thread_local std::vector<std::string> bits;
            bits.clear();
            neolib::tokens(line, ","s, bits);
            if (bits.size() < 4)
                continue;
            if (to_platform(bits.back()) != service<i_basic_services>().platform())
                continue;
            if (bits[0].size() != 32)
                continue;
            bits[0].insert(20, 1, '-');
            bits[0].insert(16, 1, '-');
            bits[0].insert(12, 1, '-');
            bits[0].insert(8, 1, '-');
            auto const& uuid = neolib::make_uuid(bits[0]);
            auto const& name = bits[1];
            for (std::size_t part = 2; part < bits.size() - 1; ++part)
            {
                neolib::vecarray<std::string, 2> partBits;
                neolib::tokens(bits[part], ":"s, partBits);
                if (partBits.size() != 2)
                    continue;
                auto& partType = partBits[0];
                auto& partValue = partBits[1];
                std::optional<int> partTypeSign;
                if (partType[0] == '-')
                    partTypeSign = -1;
                else if (partType[0] == '+')
                    partTypeSign = 1;
                if (partTypeSign)
                    partType.erase(0, 1);
                auto const& element = to_controller_element(partType);
                std::optional<int> partValueSign;
                if (partValue[0] == '-')
                    partValueSign = -1;
                else if (partValue[0] == '+')
                    partValueSign = 1;
                if (partValueSign)
                    partValue.erase(0, 1);
                switch (partValue[0])
                {
                case 'b':
                    // todo
                    break;
                case 'a':
                    // todo
                    break;
                case 'h':
                    // todo
                    break;
                default:
                    break;
                }
            }
        }
    }
}