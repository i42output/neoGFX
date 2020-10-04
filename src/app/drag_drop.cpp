// drag_drop.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/hid/i_window_manager.hpp>
#include <neogfx/app/drag_drop.hpp>

namespace neogfx
{
    drag_drop_source_impl::drag_drop_source_impl() : 
        iObject{ nullptr }
    {
        service<i_drag_drop>().register_source(*this);
    }

    drag_drop_source_impl::~drag_drop_source_impl()
    {
        service<i_drag_drop>().unregister_source(*this);
    }

    bool drag_drop_source_impl::drag_drop_active() const
    {
        return iObject != nullptr;
    }

    i_drag_drop_object const& drag_drop_source_impl::object_being_dragged() const
    {
        if (!drag_drop_active())
            throw drag_drop_not_active();
        return *iObject;
    }

    void drag_drop_source_impl::start_drag_drop(i_drag_drop_object const& aObject)
    {
        if (drag_drop_active())
            throw drag_drop_already_active();
        iObject = &aObject;
        DraggingObject.trigger(*iObject);
    }

    void drag_drop_source_impl::cancel_drag_drop()
    {
        if (!drag_drop_active())
            throw drag_drop_not_active();
        auto object = iObject;
        iObject = nullptr;
        DraggingCancelled.trigger(*object);
    }

    void drag_drop_source_impl::end_drag_drop()
    {
        if (!drag_drop_active())
            throw drag_drop_not_active();
        auto object = iObject;
        iObject = nullptr;
        ObjectDropped.trigger(*object);
    }

    void drag_drop_source_impl::monitor_drag_drop_events(i_widget& aWidget)
    {
        iSink = aWidget.mouse_event([this, &aWidget](const neogfx::mouse_event& aEvent)
        {
            handle_drag_drop_event(aWidget, aEvent);
        });
    }

    void drag_drop_source_impl::stop_monitoring_drag_drop_events()
    {
        iSink.clear();
    }

    scalar drag_drop_source_impl::drag_drop_trigger_distance() const
    {
        return iTriggerDistance;
    }

    void drag_drop_source_impl::set_drag_drop_trigger_distance(scalar aDistance)
    {
        iTriggerDistance = aDistance;
    }

    bool drag_drop_source_impl::is_drag_drop_object(point const& aPosition) const
    {
        return false;
    }

    i_drag_drop_object const* drag_drop_source_impl::drag_drop_object(point const& aPosition)
    {
        return nullptr;
    }

    void drag_drop_source_impl::handle_drag_drop_event(i_widget& aWidget, const neogfx::mouse_event& aEvent)
    {
        switch (aEvent.type())
        {
        case mouse_event_type::ButtonClicked:
            if (aEvent.is_left_button() && is_drag_drop_object(aEvent.position()))
            {
                aWidget.set_capture();
                iTrackStart = aEvent.position();
            }
            break;
        case mouse_event_type::Moved:
            if (iTrackStart)
            {
                if ((*iTrackStart - aEvent.position()).to_vec2().magnitude() >= drag_drop_trigger_distance())
                {
                    if (!drag_drop_active())
                        start_drag_drop(*(iObject = drag_drop_object(*iTrackStart)));
                }
                else
                {
                    if (drag_drop_active())
                        cancel_drag_drop();
                }
                if (drag_drop_active() && !service<i_drag_drop>().is_target_for(object_being_dragged()))
                    cancel_drag_drop();
                if (drag_drop_active() && object_being_dragged().can_render())
                {
                    // todo
                }
            }
            break;
        case mouse_event_type::ButtonReleased:
            if (iTrackStart)
            {
                if (aEvent.is_left_button())
                {
                    iTrackStart = std::nullopt;
                    if (drag_drop_active() && service<i_drag_drop>().is_target_at(
                        object_being_dragged(), aWidget.to_window_coordinates(aEvent.position())))
                    {
                        auto& target = service<i_drag_drop>().target_at(object_being_dragged(), aWidget.to_window_coordinates(aEvent.position()));
                        target.accept(object_being_dragged());
                        end_drag_drop();
                    }
                    else
                        cancel_drag_drop();
                }
            }
            break;
        }
    }


    drag_drop_target_impl::drag_drop_target_impl()
    {
        service<i_drag_drop>().register_target(*this);
    }

    drag_drop_target_impl::~drag_drop_target_impl()
    {
        service<i_drag_drop>().unregister_target(*this);
    }

    bool drag_drop_target_impl::can_accept(i_drag_drop_object const& aObject) const
    {
        bool canAccept = false;
        ObjectAcceptable.trigger(aObject, canAccept);
        return canAccept;
    }

    bool drag_drop_target_impl::accept(i_drag_drop_object const& aObject)
    {
        if (can_accept(aObject))
        {
            ObjectDropped.trigger(aObject);
            return true;
        }
        return false;
    }

    bool drag_drop_target_impl::is_widget() const
    {
        return false;
    }

    i_widget const& drag_drop_target_impl::as_widget() const
    {
        throw drag_drop_target_not_a_widget();
    }

    i_widget& drag_drop_target_impl::as_widget()
    {
        throw drag_drop_target_not_a_widget();
    }


    drag_drop::drag_drop()
    {
    }

    void drag_drop::register_source(i_drag_drop_source& aSource)
    {
        auto existing = std::find(iSources.begin(), iSources.end(), &aSource);
        if (existing == iSources.end())
        {
            iSources.push_back(&aSource);
            SourceRegistered.trigger(aSource);
        }
    }

    void drag_drop::unregister_source(i_drag_drop_source& aSource)
    {
        auto existing = std::find(iSources.begin(), iSources.end(), &aSource);
        if (existing != iSources.end())
        {
            iSources.erase(existing);
            SourceUnregistered.trigger(aSource);
        }
    }

    void drag_drop::register_target(i_drag_drop_target& aTarget)
    {
        auto existing = std::find(iTargets.begin(), iTargets.end(), &aTarget);
        if (existing == iTargets.end())
        {
            iTargets.push_back(&aTarget);
            TargetRegistered.trigger(aTarget);
        }
    }

    void drag_drop::unregister_target(i_drag_drop_target& aTarget)
    {
        auto existing = std::find(iTargets.begin(), iTargets.end(), &aTarget);
        if (existing != iTargets.end())
        {
            iTargets.erase(existing);
            TargetUnregistered.trigger(aTarget);
        }
    }

    bool drag_drop::is_target_for(i_drag_drop_object const& aObject) const
    {
        return find_target(aObject) != nullptr;
    }

    bool drag_drop::is_target_at(i_drag_drop_object const& aObject, point const& aPosition) const
    {
        return find_target(aObject, aPosition) != nullptr;
    }

    i_drag_drop_target& drag_drop::target_for(i_drag_drop_object const& aObject) const
    {
        auto existing = find_target(aObject);
        if (existing != nullptr)
            return *existing;
        throw drag_drop_target_not_found();
    }

    i_drag_drop_target& drag_drop::target_at(i_drag_drop_object const& aObject, point const& aPosition) const
    {
        auto existing = find_target(aObject, aPosition);
        if (existing != nullptr)
            return *existing;
        throw drag_drop_target_not_found();
    }

    i_drag_drop_target* drag_drop::find_target(i_drag_drop_object const& aObject) const
    {
        for (auto const& target : iTargets)
            if (target->can_accept(aObject))
                return target;
        return nullptr;
    }

    i_drag_drop_target* drag_drop::find_target(i_drag_drop_object const& aObject, point const& aPosition) const
    {
        auto window = service<i_window_manager>().window_from_position(aPosition);
        if (window)
        {
            auto const& hitWidget = window->as_widget().get_widget_at(window->as_widget().to_client_coordinates(aPosition));
            for (auto const& target : iTargets)
                if (target->can_accept(aObject) &&
                    target->is_widget() &&
                    target->as_widget().same_surface(hitWidget) &&
                    (&target->as_widget() == &hitWidget || target->as_widget().is_ancestor_of(hitWidget)))
                    return target;
        }
        return nullptr;
    }
}