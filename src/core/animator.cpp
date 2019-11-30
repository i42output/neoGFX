// animator.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/animator.hpp>

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
        iAnimator{ aAnimator }, iId{ aAnimator.allocate_id() }, iEnabled{ aEnabled }, iDisableWhenFinished{ false }, iEasingFunction{ aEasingFunction }, iDuration{ aDuration }
    {
    }

    i_animator& transition::animator() const
    {
        return iAnimator;
    }

    bool transition::enabled() const
    {
        return iEnabled;
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

    bool transition::finished() const
    {
        return animator().animation_time() - start_time() > duration();
    }

    void transition::reset()
    {
        iStartTime = std::nullopt;
    }

    transition_id transition::cookie() const
    {
        return iId;
    }

    property_transition::property_transition(i_animator& aAnimator, i_property& aProperty, easing aEasingFunction, double aDuration, bool aEnabled) :
        transition{ aAnimator, aEasingFunction, aDuration, aEnabled }, iProperty{ aProperty }, iPropertyDestroyed{ aProperty.as_lifetime() }, iFrom{ aProperty.get_as_variant() }, iTo{ aProperty.get_as_variant() }, iUpdatingProperty{ false }
    {
        iSink += aProperty.changed_from_to([this](const property_variant& aFrom, const property_variant& aTo)
        {
            if (!iUpdatingProperty)
            {
                iFrom = aFrom;
                iTo = aTo;
                reset();
            }
        });
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

    void property_transition::apply()
    {
        if (!finished() && enabled())
        {
            std::visit([this](auto&& aFrom)
            {
                std::visit([this, &aFrom](auto&& aTo)
                {
                    neolib::scoped_flag sf{ iUpdatingProperty };
                    property().set_from_variant(mix(mix_value(), aFrom, aTo));
                }, to().for_visitor());
            }, from().for_visitor());
        }
        if (finished() && disable_when_finished())
            disable();
    }

    bool property_transition::finished() const
    {
        return property_destroyed() || (transition::finished() && property().get_as_variant() == to());
    }

    bool property_transition::property_destroyed() const
    {
        return iPropertyDestroyed;
    }

    animator::animator() :
        iTimer { service<neolib::async_task>(), [this](neolib::callback_timer& aTimer)
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
        return (**iTransitions.add(std::make_unique<property_transition>(*this, aProperty, aEasingFunction, aDuration, aEnabled))).cookie();
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
            if (!t->finished())
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