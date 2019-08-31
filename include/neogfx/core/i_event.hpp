// i_event.hpp
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

namespace neogfx
{
    template <typename... Arguments>
    class i_event_handle
    {
    public:
        virtual ~i_event_handle() {}
    };

    template <typename... Arguments>
    class i_event_callback
    {
    public:
        virtual ~i_event_callback() {}
    public:
        std::unique_ptr<i_event_callback> clone() const
        {
            return std::unique_ptr<i_event_callback>{ do_clone() };
        }
    public:
        virtual void operator()(Arguments... aArguments) const = 0;
    private:
        virtual i_event_callback* do_clone() const = 0;
    };

    template <typename... Arguments>
    class event_callback : public i_event_callback<Arguments...>, public std::function<void(Arguments...)>
    {
    public:
        typedef std::function<void(Arguments...)> base;
    public:
        using base::base;
    public:
        void operator()(Arguments... aArguments) const override
        {
            base::operator()(aArguments...);
        }
    private:
        i_event_callback* do_clone() const override
        {
            return new event_callback{ *this };
        }
    };

    template <typename... Arguments>
    class i_event
    {
    public:
        typedef i_event_handle<Arguments...> handle;
        typedef std::unique_ptr<handle> handle_ptr;
        typedef i_event_callback<Arguments...> callback;
        typedef event_callback<Arguments...> concrete_callback;
    public:
        virtual ~i_event() {}
    public:
        virtual bool trigger(Arguments... aArguments) const = 0;
        virtual bool sync_trigger(Arguments... aArguments) const = 0;
        virtual void async_trigger(Arguments... aArguments) const = 0;
        virtual void accept() const = 0;
        virtual void ignore() const = 0;
    public:
        handle_ptr subscribe(const concrete_callback& aCallback, const void* aUniqueId = nullptr) const
        {
            return handle_ptr{ do_subscribe(aCallback, aUniqueId) };
        }
        handle_ptr operator()(const concrete_callback& aCallback, const void* aUniqueId = nullptr) const
        {
            return handle_ptr{ do_subscribe(aCallback, aUniqueId) };
        }
        template <typename T>
        handle_ptr subscribe(const concrete_callback& aCallback, const T* aUniqueIdObject) const
        {
            return handle_ptr{ do_subscribe(aCallback, static_cast<const void*>(aUniqueIdObject)) };
        }
        template <typename T>
        handle_ptr operator()(const concrete_callback& aCallback, const T* aUniqueIdObject) const
        {
            return handle_ptr{ do_subscribe(aCallback, static_cast<const void*>(aUniqueIdObject)) };
        }
        template <typename T>
        handle_ptr subscribe(const concrete_callback& aCallback, const T& aUniqueIdObject) const
        {
            return handle_ptr{ do_subscribe(aCallback, static_cast<const void*>(&aUniqueIdObject)) };
        }
        template <typename T>
        handle_ptr operator()(const concrete_callback& aCallback, const T& aUniqueIdObject) const
        {
            return handle_ptr{ do_subscribe(aCallback, static_cast<const void*>(&aUniqueIdObject)) };
        }
        void unsubscribe(handle aHandle) const
        {
            return do_unsubscribe(aHandle);
        }
        void unsubscribe(const void* aUniqueId) const
        {
            return do_unsubscribe(aUniqueId);
        }
        template <typename T>
        void unsubscribe(const T* aUniqueIdObject) const
        {
            return do_unsubscribe(static_cast<const void*>(aUniqueIdObject));
        }
        template <typename T>
        void unsubscribe(const T& aUniqueIdObject) const
        {
            return do_unsubscribe(static_cast<const void*>(&aUniqueIdObject));
        }
    private:
        virtual handle* do_subscribe(const callback& aCallback, const void* aUniqueId = nullptr) const = 0;
        virtual void do_unsubscribe(handle& aHandle) const = 0;
        virtual void do_unsubscribe(const void* aUniqueId) const = 0;
    };

    #define declare_event( declName, ... ) \
        virtual const i_event<__VA_ARGS__>& declName() const = 0;\
        virtual i_event<__VA_ARGS__>& declName() = 0;\
        std::unique_ptr<i_event_handle<__VA_ARGS__>> declName(const event_callback<__VA_ARGS__>& aCallback) const { return declName()(aCallback); }\
        std::unique_ptr<i_event_handle<__VA_ARGS__>> declName(const event_callback<__VA_ARGS__>& aCallback) { return declName()(aCallback); }
}