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

#include <neogfx/neogfx.hpp>

#include <neolib/core/scoped.hpp>
#include <neogfx/core/transition_animator.hpp>

template<> neogfx::i_animator& services::start_service<neogfx::i_animator>()
{
    static neogfx::animator sAnimator{};
    return sAnimator;
}

template<> void services::teardown_service<neogfx::i_animator>()
{
    if (services::service_registered<neogfx::i_animator>())
        services::service<neogfx::i_animator>().stop();
}

namespace neogfx
{
    transition::transition(i_animator& aAnimator, easing aEasingFunction, double aDuration, bool aEnabled) :
        iAnimator{ aAnimator }, iId{ aAnimator.add_transition(*this) }, iEnabled{ aEnabled }, iDisableWhenFinished{ false }, iEasingFunction{ aEasingFunction }, iDuration{ aDuration }, iPaused{ false }
    {
    }

    transition::~transition()
    {
        animator().remove_transition(id());
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
        auto const elapsed = animator().animation_time() - start_time();
        auto const elapsedNormalized = elapsed / duration();
        auto const result = ease(easing_function(), std::min(1.0, std::max(0.0, elapsedNormalized)));
        return result;
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

    void transition::reset(bool aEnable, bool aDisableWhenFinished, bool aResetStartTime)
    {
        if (aResetStartTime)
            iStartTime = std::nullopt;
        if (aEnable)
            enable(aDisableWhenFinished);
    }

    void transition::reset(easing aNewEasingFunction, bool aEnable, bool aDisableWhenFinished, bool aResetStartTime)
    {
        iEasingFunction = aNewEasingFunction;
        reset(aEnable, aDisableWhenFinished, aResetStartTime);
        if (can_apply())
            apply();
    }

    animator::animator() :
        iTimer { service<i_async_task>(), [this](neolib::callback_timer& aTimer)
        {
            aTimer.again();
            next_frame();
        }, std::chrono::milliseconds{ 10 } },
        iZeroHour{ std::chrono::high_resolution_clock::now() },
        iAnimationTime{ 0.0 }
    {
    }

    i_transition& animator::transition(transition_id aTransitionId)
    {
        return *iTransitions[aTransitionId];
    }

    transition_id animator::add_transition(i_transition& aTransition)
    {
        return iTransitions.emplace(aTransition);
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