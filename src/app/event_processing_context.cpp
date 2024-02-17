// event_processing_conteixt.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/app/event_processing_context.hpp>
#include <neogfx/app/i_app.hpp>

namespace neogfx
{
    event_processing_context::event_processing_context(std::string const& aName) :
        event_processing_context{ service<i_async_task>(), aName }
    {
    }

    event_processing_context::event_processing_context(i_async_task& aParent, std::string const& aName) :
        iName{ aName }
    {
        if (aParent.have_message_queue() && aParent.message_queue().in_idle())
            throw currently_idle();
        if (!aParent.have_message_queue())
            aParent.create_message_queue();
    }

    std::string const& event_processing_context::name() const
    {
        return iName;
    }

    bool event_processing_context::process_events()
    {
        return service<i_app>().process_events(*this);
    }
}