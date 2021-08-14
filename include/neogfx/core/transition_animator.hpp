// transition_animator.hpp
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
#include <neogfx/gui/widget/timer.hpp>
#include <neogfx/core/event.hpp>
#include <neogfx/core/i_transition_animator.hpp>

namespace neogfx
{
    class transition : public reference_counted<i_transition>
    {
    public:
        transition(i_animator& aAnimator, easing aEasingFunction, double aDuration, bool aEnabled = true);
    public:
        transition_id id() const override;
        i_animator& animator() const override;
        bool enabled() const override;
        bool disabled() const override;
        bool disable_when_finished() const override;
        void enable(bool aDisableWhenFinished = false) override;
        void disable() override;
        easing easing_function() const override;
        double duration() const override;
        double start_time() const override;
        double mix_value() const override;
        bool animation_finished() const override;
    public:
        bool active() const override;
        bool paused() const override;
        void pause() override;
        void resume() override;
    public:
        void reset(bool aEnable = true, bool aDisableWhenFinished = false) override;
        void reset(easing aNewEasingFunction, bool aEnable = true, bool aDisableWhenFinished = false) override;
    private:
        i_animator& iAnimator;
        transition_id iId;
        bool iEnabled;
        bool iDisableWhenFinished;
        easing iEasingFunction;
        double iDuration;
        mutable std::optional<double> iStartTime;
        bool iPaused;
    };

    class animator : public i_animator
    {
    public:
        animator();
    public:
        i_transition& transition(transition_id aTransitionId) override;
        transition_id add_transition(i_transition& aTransition) override;
        void remove_transition(transition_id aTransitionId) override;
        void stop() override;
    public:
        double animation_time() const override;
    protected:
        transition_id allocate_id() override;
    private:
        void next_frame();
    private:
        neolib::callback_timer iTimer;
        neolib::jar<ref_ptr<i_transition>> iTransitions;
        std::chrono::time_point<std::chrono::high_resolution_clock> iZeroHour;
        double iAnimationTime;
    };
}