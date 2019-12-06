// native_window.hpp
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
#include <neolib/variant.hpp>
#include <neolib/timer.hpp>
#include <neogfx/core/object.hpp>
#include "i_native_window.hpp"

namespace neogfx
{
    class i_rendering_engine;
    class i_surface_manager;

    class native_window : public object<i_native_window>
    {
    public:
        define_declared_event(TargetActivating, target_activating)
        define_declared_event(TargetActivated, target_activated)
        define_declared_event(TargetDeactivating, target_deactivating)
        define_declared_event(TargetDeactivated, target_deactivated)
        define_declared_event(Filter, filter, native_event&)
    private:
        typedef std::deque<native_event> event_queue;
    public:
        native_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager);
        virtual ~native_window();
    public:
        dimension horizontal_dpi() const override;
        dimension vertical_dpi() const override;
        dimension ppi() const override;
        dimension em_size() const override;
    public:
        void display_error_message(const std::string& aTitle, const std::string& aMessage) const override;
        bool events_queued() const override;
        void push_event(const native_event& aEvent) override;
        bool pump_event() override;
        void handle_event(const native_event& aEvent) override;
        bool has_current_event() const override;
        const native_event& current_event() const override;
        void handle_event() override;
        bool processing_event() const override;
        double rendering_priority() const override;
        const std::string& title_text() const override;
        void set_title_text(const std::string& aTitleText) override;
    public:
        i_rendering_engine& rendering_engine() const;
        i_surface_manager& surface_manager() const;
    public:
        bool non_client_entered() const;
    protected:
        size& pixel_density() const;
        void handle_dpi_changed() override;
    private:
        template <typename EventCategory, typename EventType>
        event_queue::const_iterator find_event(EventType aEventType) const
        {
            for (auto e = iEventQueue.end(); e != iEventQueue.begin();)
            {
                --e;
                if (std::holds_alternative<EventCategory>(*e) && static_variant_cast<const EventCategory&>(*e).type() == aEventType)
                    return e;
            }
            return iEventQueue.end();
        }
    private:
        i_rendering_engine& iRenderingEngine;
        i_surface_manager& iSurfaceManager;
        mutable optional_size iPixelDensityDpi;
        event_queue iEventQueue;
        native_event iCurrentEvent;
        uint32_t iProcessingEvent;
        std::string iTitleText;
        bool iNonClientEntered;
        neolib::callback_timer iUpdater;
    };
}