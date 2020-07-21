// transition_animator.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/core/scoped.hpp>
#include <neogfx/core/transition_animator.hpp>

namespace neogfx
{
    template<> i_animator& service<i_animator>() 
    { 
        static animator sAnimator{}; 
        return sAnimator; 
    }

    template<> void teardown_service<i_animator>()
    {
        service<i_animator>().stop();
    }

    transition::transition(i_animator& aAnimator, easing aEasingFunction, double aDuration, bool aEnabled) :
        iAnimator{ aAnimator }, iId{ aAnimator.allocate_id() }, iEnabled{ aEnabled }, iDisableWhenFinished{ false }, iEasingFunction{ aEasingFunction }, iDuration{ aDuration }, iPaused{ false }
    {
    }

    transition_id transition::id() const
    {
        return iId;
    }

    i_animator& transition::animator() const
    {
        return iAnimator;
    }

    bool transition::enabled() const
    {
        return iEnabled;
    }

    bool transition::disabled() const
    {
        return !iEnabled;
    }

    bool transition::disable_when_finished() const
    {
        return iDisableWhenFinished;
    }

    void transition::enable(bool aDisableWhenFinished)
    {
        iEnabled = true;
        iDisableWhenFinished = aDisableWhenFinished;
    }

    void transition::disable()
    {
        iEnabled = false;
    }

    easing transition::easing_function() const
    {
        return iEasingFunction;
    }

    double transition::duration() const
    {
        return iDuration;
    }

    double transition::start_time() const
    {
        if (iStartTime == std::nullopt)
            iStartTime = animator().animation_time();
        return *iStartTime;
    }

    double transition::mix_value() const
    {
        return ease(easing_function(), std::min(1.0, std::max(0.0, (animator().animation_time() - start_time()) / duration())));
    }

    bool transition::animation_finished() const
    {
        return animator().animation_time() - start_time() > duration();
    }

    bool transition::active() const
    {
        return enabled() && !finished() && !paused();
    }

    bool transition::paused() const
    {
        return enabled() && !finished() && iPaused;
    }

    void transition::pause()
    {
        iPaused = true;
    }

    void transition::resume()
    {
        iPaused = false;
    }

    void transition::reset(bool aEnable, bool aDisableWhenFinished)
    {
        iStartTime = std::nullopt;
        if (aEnable)
            enable(aDisableWhenFinished);
    }

    void transition::reset(easing aNewEasingFunction, bool aEnable, bool aDisableWhenFinished)
    {
        iEasingFunction = aNewEasingFunction;
        reset(aEnable, aDisableWhenFinished);
        if (can_apply())
            apply();
    }

    property_transition::property_transition(i_animator& aAnimator, i_property& aProperty, easing aEasingFunction, double aDuration, bool aEnabled) :
        transition{ aAnimator, aEasingFunction, aDuration, aEnabled }, 
        iProperty{ aProperty }, 
        iPropertyDestroyed{ aProperty }, 
        iFrom{ aProperty.get_as_variant() }, 
        iTo{ aProperty.get_as_variant() }, 
        iUpdatingProperty{ false },
        iEventQueueDestroyed{ neolib::async_event_queue::instance() }
    {
        neolib::async_event_queue::instance().filter_registry().install_event_filter(*this, property().property_changed().raw_event());
    }

    property_transition::~property_transition()
    {
        if (!iEventQueueDestroyed)
            neolib::async_event_queue::instance().filter_registry().install_event_filter(*this, property().property_changed().raw_event());
    }

    i_property& property_transition::property() const
    {
        return iProperty;
    }

    const property_variant& property_transition::from() const
    {
        return iFrom;
    }

    const property_variant& property_transition::to() const
    {
        return iTo;
    }

    bool property_transition::can_apply() const
    {
        return !finished() && enabled() && !paused();
    }

    void property_transition::apply()
    {
        if (!can_apply())
            throw cannot_apply();
        if (!animation_finished())
        {
            std::visit([this](auto&& aFrom)
            {
                std::visit([this, &aFrom](auto&& aTo)
                {
                    neolib::scoped_flag sf{ iUpdatingProperty };
                    auto const value = mix(mix_value(), aFrom, aTo);
                    property().set_from_variant(value);
                }, to().for_visitor());
            }, from().for_visitor());
        }
        else
        {
            neolib::scoped_flag sf{ iUpdatingProperty };
            property().set_from_variant(easing_function() != easing::Zero ? iTo : iFrom);
            clear();
            if (disable_when_finished())
                disable();
        }
    }

    bool property_transition::finished() const
    {
        return property_destroyed() || iTo == neolib::none;
    }

    void property_transition::clear()
    {
        iFrom = neolib::none;
        iTo = neolib::none;
    }

    bool property_transition::property_destroyed() const
    {
        return iPropertyDestroyed;
    }

    void property_transition::pre_filter_event(const neolib::i_event& aEvent) 
    {
        if (enabled() && !paused() && !iUpdatingProperty)
        {
            if (iFrom == neolib::none)
            {
                iFrom = property().get_as_variant();
                iTo = property().get_new_as_variant();
            }
            else
            {
                iFrom = iTo;
                iTo = property().get_new_as_variant();
            }
            property().discard_change_events();
            reset(true, disable_when_finished());
        }
    }

    void property_transition::filter_event(const neolib::i_event&)
    {
    }

    animator::animator() :
        iTimer { service<async_task>(), [this](neolib::callback_timer& aTimer)
        {
            aTimer.again();
            next_frame();
        }, 10 }, 
        iZeroHour{ std::chrono::high_resolution_clock::now() },
        iAnimationTime{ 0.0 }
    {
    }

    i_transition& animator::transition(transition_id aTransitionId)
    {
        return *iTransitions[aTransitionId];
    }

    transition_id animator::add_transition(i_property& aProperty, easing aEasingFunction, double aDuration, bool aEnabled)
    {
        auto result = iTransitions.emplace(std::make_unique<property_transition>(*this, aProperty, aEasingFunction, aDuration, aEnabled));
        if (iTransitions[result]->can_apply())
            iTransitions[result]->apply();
        return result;
    }

    void animator::remove_transition(transition_id aTransitionId)
    {
        iTransitions.remove(aTransitionId);
    }

    void animator::stop()
    {
        iTimer.disable();
    }

    void animator::next_frame()
    {
        iAnimationTime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - iZeroHour).count();
        for (auto& t : iTransitions)
            if (t->can_apply())
                t->apply();
    }

    double animator::animation_time() const
    {
        return iAnimationTime;
    }

    transition_id animator::allocate_id()
    {
        return iTransitions.next_cookie();
    }
}