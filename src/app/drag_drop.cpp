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
    drag_drop::drag_drop()
    {
    }

    void drag_drop::register_source(i_drag_drop_source& aSource)
    {
        auto existing = std::find_if(iSources.begin(), iSources.end(), [&](auto& rhs) { return &aSource == &*rhs; });
        if (existing == iSources.end())
        {
            iSources.push_back(sources_t::value_type{ sources_t::value_type{}, &aSource });
            SourceRegistered.trigger(aSource);
        }
    }

    void drag_drop::register_source(std::shared_ptr<i_drag_drop_source> const& aSource)
    {
        iSources.push_back(aSource);
    }

    void drag_drop::unregister_source(i_drag_drop_source& aSource)
    {
        auto existing = std::find_if(iSources.begin(), iSources.end(), [&](auto& rhs) { return &aSource == &*rhs; });
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
        auto window = aObject.source().drag_drop_widget() ?
            service<i_window_manager>().window_from_position(aPosition, *aObject.source().drag_drop_widget()) :
            service<i_window_manager>().window_from_position(aPosition);
        if (window)
        {
            auto const nonClientPos = window->as_widget().to_client_coordinates(aPosition - window->window_position());
            auto const& hitWidget = window->as_widget().get_widget_at(nonClientPos);
            for (auto const& target : iTargets)
                if (target->can_accept(aObject, hitWidget.to_client_coordinates(nonClientPos)) &&
                    target->is_widget() &&
                    target->as_widget().same_physical_surface(hitWidget) &&
                    (&target->as_widget() == &hitWidget || target->as_widget().is_ancestor_of(hitWidget)))
                    return target;
        }
        return nullptr;
    }
}