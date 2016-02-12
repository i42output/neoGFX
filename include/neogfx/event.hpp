// event.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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

#include "neogfx.hpp"
#include <list>
#include <deque>
#include <neolib/destroyable.hpp>

namespace neogfx
{
	template <typename... Arguments>
	class event : private neolib::destroyable
	{
	public:
		typedef std::function<void(Arguments...)> sink_callback;
		typedef std::list<std::pair<sink_callback, const void*>> sink_list;
		typedef typename sink_list::const_iterator handle;
	private:
		typedef std::deque<typename sink_list::const_iterator> notification_list;
	public:
		event() :
			iAccepted(false)
		{
		}
		bool trigger(Arguments... aArguments) const
		{
			destroyed_flag destroyed(*this);
			for (auto i = iSinks.begin(); i != iSinks.end(); ++i)
				iNotifications.push_back(i);
			while (!iNotifications.empty())
			{
				auto i = iNotifications.front();
				iNotifications.pop_front();
				i->first(aArguments...);
				if (destroyed)
					return false;
				if (iAccepted)
				{
					iNotifications.clear();
					iAccepted = false;
					return false;
				}
			}
			return true;
		}
		void accept() const
		{
			iAccepted = true;
		}
		void ignore() const
		{
			iAccepted = false;
		}
	public:
		handle subscribe(const sink_callback& aSinkCallback)
		{
			return iSinks.insert(iSinks.end(), std::make_pair(aSinkCallback, nullptr));
		}
		handle subscribe(const sink_callback& aSinkCallback, const void* aSinkObject)
		{
			unsubscribe(aSinkObject);
			return iSinks.insert(iSinks.end(), std::make_pair(aSinkCallback, aSinkObject));
		}
		handle operator()(const sink_callback& aSinkCallback)
		{
			return subscribe(aSinkCallback);
		}
		handle operator()(const sink_callback& aSinkCallback, const void* aSinkObject)
		{
			return subscribe(aSinkCallback);
		}
		void unsubscribe(handle aHandle)
		{
			iNotifications.erase(std::remove(iNotifications.begin(), iNotifications.end(), aHandle), iNotifications.end())
			iSinks.erase(aHandle);
		}
		void unsubscribe(const void* aSinkObject)
		{
			for (auto i = iSinks.begin(); i != iSinks.end();)
				if (i->second == aSinkObject)
				{
					iNotifications.erase(std::remove(iNotifications.begin(), iNotifications.end(), i), iNotifications.end());
					i = iSinks.erase(i);
				}
				else
					++i;
		}
	private:
		sink_list iSinks;
		mutable bool iAccepted;
		mutable notification_list iNotifications;
	};
}