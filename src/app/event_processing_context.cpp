// event_processing_conteixt.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/event_processing_context.hpp>
#include <neogfx/app/i_app.hpp>

namespace neogfx
{
	event_processing_context::event_processing_context(neolib::async_task& aParent, const std::string& aName) :
		iContext{ aParent.have_message_queue() ?
			aParent.message_queue() :
			aParent.create_message_queue([]()
			{
				return service<i_app>().process_events(service<i_app>().app_message_queue_context());
			}) },
		iName{ aName }
	{
	}

	const std::string& event_processing_context::name() const
	{
		return iName;
	}
}