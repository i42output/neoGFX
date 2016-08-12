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

#include <neogfx/neogfx.hpp>
#include <list>
#include <deque>
#include <boost/pool/pool_alloc.hpp>
#include <neolib/destroyable.hpp>

namespace neogfx
{
	class sink;

	template <typename... Arguments>
	class event;
		
	template <typename... Arguments>
	class event_handle
	{
	public:
		typedef event<Arguments...>* event_ptr;
		typedef std::shared_ptr<event_ptr> event_instance_ptr;
		typedef std::weak_ptr<event_ptr> event_instance_weak_ptr;
		typedef std::function<void(Arguments...)> sink_callback;
		typedef uint32_t sink_reference_count;
		struct sink_list_item { sink_callback iSinkCallback; sink_reference_count iSinkReferenceCount; };
		typedef std::list<sink_list_item, boost::fast_pool_allocator<sink_list_item>> sink_list;
	public:
		event_instance_weak_ptr iEvent;
		typename sink_list::iterator iSink;
	};

	template <typename... Arguments>
	class event : private neolib::destroyable
	{
		friend class sink;
	private:
		typedef event_handle<Arguments...> handle;
		typedef typename handle::event_ptr ptr;
		typedef typename handle::event_instance_ptr instance_ptr;
		typedef typename handle::event_instance_weak_ptr instance_weak_ptr;
		typedef typename handle::sink_callback sink_callback;
		typedef typename handle::sink_reference_count sink_reference_count;
		typedef typename handle::sink_list_item sink_list_item;
		typedef typename handle::sink_list sink_list;
		typedef std::deque<typename sink_list::const_iterator> notification_list;
	public:
		event() :
			iInstancePtr{new ptr{this}},
			iAccepted{false}
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
				i->iSinkCallback(aArguments...);
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
			return handle{ iInstancePtr, iSinks.insert(iSinks.end(), sink_list_item{ aSinkCallback, 0 }) };
		}
		handle operator()(const sink_callback& aSinkCallback)
		{
			return subscribe(aSinkCallback);
		}
	private:
		void unsubscribe(handle aHandle)
		{
			iNotifications.erase(std::remove(iNotifications.begin(), iNotifications.end(), aHandle.iSink), iNotifications.end());
			iSinks.erase(aHandle.iSink);
		}
	private:
		instance_ptr iInstancePtr;
		sink_list iSinks;
		mutable bool iAccepted;
		mutable notification_list iNotifications;
	};

	class sink
	{
	private:
		enum controller_op_e
		{
			AddRef,
			Release
		};
	public:
		sink()
		{
		}
		template <typename... Arguments>
		sink(event_handle<Arguments...> aHandle) :
			iControllers{[aHandle](controller_op_e aOperation)
				{ 
					if (!aHandle.iEvent.expired())
					{
						switch (aOperation)
						{
						case AddRef:
							++aHandle.iSink->iSinkReferenceCount;
							break;
						case Release:
							if (--aHandle.iSink->iSinkReferenceCount == 0)
								(**aHandle.iEvent.lock()).unsubscribe(aHandle);
							break;
						}
					}
				}}
		{
			add_ref();
		}
		sink(const sink& aSink) : 
			iControllers{aSink.iControllers}
		{
			add_ref();
		}
		sink& operator=(const sink& aSink)
		{
			if (this == &aSink)
				return *this;
			release();
			iControllers = aSink.iControllers;
			add_ref();
			return *this;
		}
		template <typename... Arguments>
		sink& operator=(event_handle<Arguments...> aHandle)
		{
			return *this = sink{aHandle};
		}
		template <typename... Arguments>
		sink& operator+=(event_handle<Arguments...> aHandle)
		{
			sink s{aHandle};
			s.add_ref();
			iControllers.insert(iControllers.end(), s.iControllers.begin(), s.iControllers.end());
			return *this;
		}
		~sink()
		{
			release();
		}
	private:
		void add_ref() const
		{
			for (auto c : iControllers)
				c(AddRef);
		}
		void release() const
		{
			for (auto c : iControllers)
				c(Release);
		}
	private:
		std::vector<std::function<void(controller_op_e)>> iControllers;
	};
}