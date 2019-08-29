// event.hpp
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
#include <neolib/event.hpp>
#include <neogfx/core/i_event.hpp>

namespace neogfx
{
    using neolib::async_event_queue;
    using neolib::event_trigger_type;

    template <typename... Arguments>
    class event_handle : public i_event_handle<Arguments...>, public neolib::event_handle<Arguments...>
    {
    public:
        typedef neolib::event_handle<Arguments...> base;
    public:
        using base::base;
        event_handle(const base& aOther) :
            base{ aOther }
        {
        }
    };

    class sink : public neolib::sink
    {
    public:
        typedef neolib::sink base;
    public:
        using base::base;
    public:
        using base::operator=;
        using base::operator+=;
        template <typename... Arguments>
        sink& operator=(const i_event_handle<Arguments...>& aHandle)
        {
            base::operator=(static_cast<const event_handle<Arguments...>&>(aHandle));
            return *this;
        }
        template <typename... Arguments>
        sink& operator+=(const i_event_handle<Arguments...>& aHandle)
        {
            base::operator+=(static_cast<const event_handle<Arguments...>&>(aHandle));
            return *this;
        }
        template <typename... Arguments>
        sink& operator=(const std::unique_ptr<i_event_handle<Arguments...>>& aHandle)
        {
            base::operator=(static_cast<const event_handle<Arguments...>&>(*aHandle));
            return *this;
        }
        template <typename... Arguments>
        sink& operator+=(const std::unique_ptr<i_event_handle<Arguments...>>& aHandle)
        {
            base::operator+=(static_cast<const event_handle<Arguments...>&>(*aHandle));
            return *this;
        }
    };

    template <typename... Arguments>
    class event : public i_event<Arguments...>, public neolib::event<Arguments...>
    {
    public:
        typedef neolib::event<Arguments...> base;
        typedef i_event_handle<Arguments...> handle;
        typedef event_handle<Arguments...> concrete_handle;
    public:
        using base::base;
    public:
        using base::subscribe;
        using base::operator();
        using base::unsubscribe;
    public:
        bool trigger(Arguments... aArguments) const override
        {
            return base::trigger(aArguments...);
        }
        bool sync_trigger(Arguments... aArguments) const override
        {
            return base::sync_trigger(aArguments...);
        }
        void async_trigger(Arguments... aArguments) const override
        {
            base::async_trigger(aArguments...);
        }
        void accept() const override
        {
            base::accept();
        }
        void ignore() const override
        {
            base::ignore();
        }
    private:
        handle* do_subscribe(const callback& aCallback, const void* aUniqueId = nullptr) const override
        {
            std::shared_ptr<callback> cb = std::move(aCallback.clone());
            return new concrete_handle(
                base::subscribe(
                    [cb](Arguments&& ... aArguments)
                    {
                        (*cb)(std::forward<Arguments>(aArguments)...);
                    },
                    aUniqueId));
        }
        void do_unsubscribe(handle& aHandle) const override
        {
            return base::unsubscribe(static_cast<concrete_handle&>(aHandle));
        }
        void do_unsubscribe(const void* aUniqueId) const override
        {
            return base::unsubscribe(aUniqueId);
        }
    };

    #define define_event( name, ... ) \
        event<__VA_ARGS__> ev##name;

    #define define_declared_event( name, declName, ... ) \
        event<__VA_ARGS__> ev##name; \
        const i_event<__VA_ARGS__>& declName() const override { return ev##name; };\
        i_event<__VA_ARGS__>& declName() override { return ev##name; };
}