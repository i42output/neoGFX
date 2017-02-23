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
		typedef const void* unique_id_type;
		typedef std::function<void(Arguments...)> handler_callback;
		typedef uint32_t sink_reference_count;
		struct handler_list_item { unique_id_type iUniqueId; handler_callback iHandlerCallback; sink_reference_count iSinkReferenceCount; };
		typedef std::list<handler_list_item, boost::fast_pool_allocator<handler_list_item>> handler_list;
	public:
		event_instance_weak_ptr iEvent;
		typename handler_list::iterator iHandler;
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
		typedef typename handle::unique_id_type unique_id_type;
		typedef typename handle::handler_callback handler_callback;
		typedef typename handle::sink_reference_count sink_reference_count;
		typedef typename handle::handler_list_item handler_list_item;
		typedef typename handle::handler_list handler_list;
		typedef std::map<unique_id_type, typename handler_list::iterator> unique_id_map;
		typedef std::deque<typename handler_list::const_iterator> notification_list;
	public:
		event() :
			iInstancePtr{new ptr{this}},
			iAccepted{false}
		{
		}
		event(const event&) :
			iInstancePtr{new ptr{this}},
			iAccepted{false}
		{
		}
		~event()
		{
		}
		bool trigger(Arguments... aArguments) const
		{
			destroyed_flag destroyed(*this);
			for (auto i = iHandlers.begin(); i != iHandlers.end(); ++i)
				iNotifications.push_back(i);
			while (!iNotifications.empty())
			{
				auto i = iNotifications.front();
				iNotifications.pop_front();
				i->iHandlerCallback(aArguments...);
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
		handle subscribe(const handler_callback& aHandlerCallback, const void* aUniqueId = 0)
		{
			if (aUniqueId == 0)
				return handle{ iInstancePtr, iHandlers.insert(iHandlers.end(), handler_list_item{ aUniqueId, aHandlerCallback, 0 }) };
			auto existing = iUniqueIdMap.find(aUniqueId);
			if (existing == iUniqueIdMap.end())
				existing = iUniqueIdMap.insert(std::make_pair(aUniqueId, iHandlers.insert(iHandlers.end(), handler_list_item{ aUniqueId, aHandlerCallback, 0 }))).first;
			else
				existing->second->iHandlerCallback = aHandlerCallback;
			return handle{ iInstancePtr, existing->second };
		}
		handle operator()(const handler_callback& aHandlerCallback, const void* aUniqueId = 0)
		{
			return subscribe(aHandlerCallback, aUniqueId);
		}
		template <typename T>
		handle subscribe(const handler_callback& aHandlerCallback, const T* aUniqueIdObject)
		{
			return subscribe(aHandlerCallback, static_cast<const void*>(aUniqueIdObject));
		}
		template <typename T>
		handle operator()(const handler_callback& aHandlerCallback, const T* aUniqueIdObject)
		{
			return subscribe(aHandlerCallback, static_cast<const void*>(aUniqueIdObject));
		}
		template <typename T>
		handle subscribe(const handler_callback& aHandlerCallback, const T& aUniqueIdObject)
		{
			return subscribe(aHandlerCallback, static_cast<const void*>(&aUniqueIdObject));
		}
		template <typename T>
		handle operator()(const handler_callback& aHandlerCallback, const T& aUniqueIdObject)
		{
			return subscribe(aHandlerCallback, static_cast<const void*>(&aUniqueIdObject));
		}
		void unsubscribe(const void* aUniqueId)
		{
			auto existing = iUniqueIdMap.find(aUniqueId);
			if (existing != iUniqueIdMap.end())
				unsubscribe(*existing);
		}
		template <typename T>
		void unsubscribe(const T* aUniqueIdObject)
		{
			return unsubscribe(static_cast<const void*>(aUniqueIdObject));
		}
		template <typename T>
		void unsubscribe(const T& aUniqueIdObject)
		{
			return unsubscribe(static_cast<const void*>(&aUniqueIdObject));
		}
	private:
		void unsubscribe(handle aHandle)
		{
			iNotifications.erase(std::remove(iNotifications.begin(), iNotifications.end(), aHandle.iHandler), iNotifications.end());
			if (aHandle.iHandler->iUniqueId != 0)
			{
				auto existing = iUniqueIdMap.find(aHandle.iHandler->iUniqueId);
				if (existing != iUniqueIdMap.end())
					iUniqueIdMap.erase(existing);
			}
			iHandlers.erase(aHandle.iHandler);
		}
	private:
		instance_ptr iInstancePtr;
		handler_list iHandlers;
		mutable unique_id_map iUniqueIdMap;
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
							++aHandle.iHandler->iSinkReferenceCount;
							break;
						case Release:
							if (--aHandle.iHandler->iSinkReferenceCount == 0 && !aHandle.iEvent.expired())
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