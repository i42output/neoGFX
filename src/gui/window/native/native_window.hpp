// native_window.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>

#include <neolib/core/variant.hpp>

#include <neogfx/core/object.hpp>
#include <neogfx/gui/widget/timer.hpp>
#include <neogfx/gui/window/i_native_window.hpp>

namespace neogfx
{
    class i_rendering_engine;
    class i_surface_manager;

    class native_window : public reference_counted<object<i_native_window>>
    {
    public:
        define_declared_event(Filter, filter, native_event&)
        define_declared_event(AddedToNest, added_to_nest)
    private:
        typedef std::deque<native_event> event_queue;
    public:
        native_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aSurfaceWindow);
        virtual ~native_window();
    public:
        bool attached() const final;
        i_native_surface& attachment() const final;
        void attach(i_native_surface& aSurface) override;
        void detach() override;
    public:
        i_surface_window& surface_window() const final;
    public:
        void display_error_message(std::string const& aTitle, std::string const& aMessage) const final;
        bool events_queued() const final;
        void push_event(const native_event& aEvent) final;
        bool pump_event() final;
        void handle_event(const native_event& aEvent) final;
        bool has_current_event() const final;
        const native_event& current_event() const final;
        native_event& current_event() final;
        void handle_event() final;
        bool processing_event() const final;
        i_string const& title_text() const final;
        void set_title_text(i_string const& aTitleText) override;
    public:
        i_rendering_engine& rendering_engine() const;
        i_surface_manager& surface_manager() const;
    public:
        bool non_client_entered() const;
    protected:
        size& pixel_density() const final;
        void handle_dpi_changed() final;
    protected:
        bool internal_window_activation() const;
        void mouse_entered(i_surface_window& aWindow);
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
        i_surface_window& iSurfaceWindow;
        mutable optional_size iPixelDensityDpi;
        event_queue iEventQueue;
        native_event iCurrentEvent;
        std::uint32_t iProcessingEvent;
        string iTitleText;
        bool iNonClientEntered;
        neolib::callback_timer iUpdater;
        bool iInternalWindowActivation;
        i_surface_window* iEnteredWindow;
        sink iEnteredWindowEventSink;
        ref_ptr<i_native_surface> iSurface;
    };
}