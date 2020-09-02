// i_game_controllers.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neolib/core/i_vector.hpp>
#include <neolib/core/i_map.hpp>
#include <neogfx/hid/i_game_controller.hpp>

namespace neogfx
{
    class i_game_controllers : public i_service
    {
    public:
        declare_event(controller_connected, i_game_controller&)
        declare_event(controller_disconnected, i_game_controller&)
        declare_event(controller_calibration_required, i_game_controller&)
    public:
        struct unassigned_player : std::logic_error { unassigned_player() : std::logic_error{ "neogfx::i_game_controllers::unassigned_player" } {} };
    public:
        typedef neolib::i_vector<i_ref_ptr<i_game_controller>> controller_list;
        typedef neolib::i_map<game_controller_button_ordinal, game_controller_button> button_map_type;
    public:
        virtual ~i_game_controllers() = default;
    public:
        virtual void enumerate_controllers() = 0;
        virtual const controller_list& controllers() const = 0;
        virtual bool have_controller_for(game_player aPlayer) const = 0;
        virtual i_game_controller& controller_for(game_player aPlayer) const = 0;
    public:
        virtual const i_string& product_name(const hid_device_uuid& aProductId) const = 0;
        virtual const button_map_type& button_map(const hid_device_uuid& aProductId) const = 0;
    public:
        virtual controller_list::iterator add_device(i_game_controller& aController) = 0;
        virtual controller_list::iterator remove_device(i_game_controller& aController) = 0;
    public:
        template <typename Controller, typename... Args>
        ref_ptr<Controller> add_controller(Args&&... aArgs)
        {
            auto newController = make_ref<Controller>(std::forward<Args>(aArgs)...);
            add_device(*newController);
            return newController;
        }
    public:
        static uuid const& iid() { static uuid const sIid{ 0xda529547, 0xbddd, 0x4f52, 0x9f0, { 0xa5, 0xec, 0xee, 0x8e, 0x6e, 0x7f } }; return sIid; }
    };
}