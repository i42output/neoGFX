// event_processing_context.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/async_task.hpp>

#include <neogfx/app/i_event_processing_context.hpp>

namespace neogfx
{
    class event_processing_context : public i_event_processing_context
    {
    public:
        event_processing_context(neolib::async_task& aParent, const std::string& aName = std::string{});
    public:
        virtual const std::string& name() const;
    private:
        neolib::message_queue::scoped_context iContext;
        std::string iName;
    };
}