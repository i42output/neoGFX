// event.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.
  
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
#include <neolib/timer.hpp>
#include <neogfx/core/event.hpp>
#include <neogfx/app/app.hpp>

namespace neogfx
{ 
	async_event_queue::async_event_queue(neolib::io_task& aIoTask) : iTimer{ aIoTask,
		[this](neolib::callback_timer& aTimer)
	{
		publish_events();
		if (!iEvents.empty() && !aTimer.waiting())
			aTimer.again();
	}, 10, false }
	{
		if (sInstance != nullptr)
			throw instance_exists();
		sInstance = this;
	}

	async_event_queue::~async_event_queue()
	{
		sInstance = nullptr;
	}

	async_event_queue* async_event_queue::sInstance;

	async_event_queue& async_event_queue::instance()
	{
		if (sInstance != nullptr)
			return *sInstance;
		throw no_instance();
	}

	void async_event_queue::add(const void* aEvent, callback aCallback, neolib::destroyable::destroyed_flag aDestroyedFlag)
	{
		iEvents.emplace(aEvent, std::make_pair(aCallback, aDestroyedFlag));
		if (!iTimer.waiting())
			iTimer.again();
	}

	void async_event_queue::remove(const void* aEvent)
	{
		auto events = iEvents.equal_range(aEvent);
		if (events.first == events.second)
			throw event_not_found();
		event_list toPublish{ events.first, events.second };
		iEvents.erase(events.first, events.second);
		for (auto& e : toPublish)
			if (!e.second.second)
				e.second.first();
	}

	bool async_event_queue::has(const void* aEvent) const
	{
		return iEvents.find(aEvent) != iEvents.end();
	}

	void async_event_queue::publish_events()
	{
		event_list toPublish;
		toPublish.swap(iEvents);
		for (auto& e : toPublish)
			if (!e.second.second)
				e.second.first();
	}
}