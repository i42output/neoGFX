// i_skin_manager.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/event.hpp>
#include <neogfx/gui/widget/i_skin.hpp>
#include <neogfx/gui/widget/i_skinnable_item.hpp>

namespace neogfx
{
    class i_skin_manager
    {
    public:
        event<i_skin&> skin_registered;
        event<i_skin&> skin_unregistered;
        event<i_skinnable_item&> item_registered;
        event<i_skinnable_item&> item_unregistered;
    public:
        virtual uint32_t skin_count() const = 0;
        virtual const i_skin& skin(uint32_t aIndex) const = 0;
        virtual i_skin& skin(uint32_t aIndex) = 0;
        virtual void register_skin(i_skin& aSkin) = 0;
        virtual void unregister_skin(i_skin& aSkin) = 0;
    public:
        virtual const i_skin& active_skin() const = 0;
        virtual i_skin& active_skin() = 0;
        virtual void activate_skin(i_skin& aSkin) = 0;
    public:
        virtual bool is_item_registered(const i_skinnable_item& aItem) const = 0;
        virtual void register_item(i_skinnable_item& aItem) = 0;
        virtual void unregister_item(i_skinnable_item& aItem) = 0;
    };
}