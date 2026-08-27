// animation_filter.hpp
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

#include <span>

#include <neolib/core/uuid.hpp>
#include <neolib/core/string.hpp>

#include <neogfx/game/i_ecs.hpp>
#include <neogfx/game/time.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/mesh_filter.hpp>
#include <neogfx/game/animation.hpp>
#include <neogfx/game/entity_life_span.hpp>

namespace neogfx::game
{
    // @todo add meta
    struct animation_timer
    {
        i64 epoch = 0;
        i64 lastElapsed = 0;
        std::optional<i64> duration;    // one-shot when set; nullopt runs indefinitely
        bool paused = false;

        bool complete() const
        {
            return duration && lastElapsed >= *duration;
        }

        bool running() const
        {
            return !paused && !complete();
        }

        i64 elapsed(i64 aStepTime)
        {
            if (!paused)
            {
                lastElapsed = aStepTime - epoch;
                if (duration && lastElapsed > *duration)
                    lastElapsed = *duration;
            }
            return lastElapsed;
        }

        void pause(i64 aStepTime)
        {
            lastElapsed = elapsed(aStepTime);
            paused = true;
        }

        void resume(i64 aStepTime)
        {
            epoch = aStepTime - lastElapsed;
            paused = false;
        }

        void reset(i64 aStepTime)
        {
            epoch = aStepTime;
            lastElapsed = 0;
            paused = false;
        }
    };

    using animation_timer_ptr = std::shared_ptr<animation_timer>;
    using animation_timer_weak_ptr = std::weak_ptr<animation_timer>;

    // @todo make componenent data (add meta)
    struct frame_animation_state
    {
        define_event(Changed, changed)

        bool active = false;
        u32 currentFrame = 0u;
        bool autoDestroy = false;
        std::optional<i64> currentFrameStartTime;
        bool stopFilterOnComplete = false;

        void start(i64 aStepTime)
        {
            if (!active)
            {
                active = true;
                currentFrame = 0u;
                currentFrameStartTime = aStepTime;
                changed();
            }
        }

        void stop()
        {
            if (active)
            {
                active = false;
                changed();
            }
        }
    };

    // @todo make componenent data (add meta)
    struct tween_animation_state
    {
        define_event(Changed, changed)

        bool active = false;
        animation_timer_ptr timer;
        optional_time_interval timerDuration;    // when set and timer is null, animator creates a one-shot timer of this length

        void start()
        {
            if (!active)
            {
                active = true;
                changed();
            }
        }

        void stop()
        {
            if (active)
            {
                active = false;
                changed();
            }
        }
    };

    struct animation_filter
    {
        shared<animation> sharedAnimation;
        std::optional<animation> animation;
        std::optional<mat44f> transformation;

        // @todo add to meta
        frame_animation_state frameAnimationState;
        // @todo add to meta
        std::unordered_map<animation_tween_ptr, tween_animation_state> tweenAnimationStates;
        // @todo add to meta
        std::set<std::variant<animation_tween_ptr, animation_timer_ptr>> completionTimers;

        animation_tweens const& asset_tweens() const
        {
            static animation_tweens const sNoTweens;
            if (animation && animation->tweens)
                return *animation->tweens;
            else if (sharedAnimation && sharedAnimation->tweens)
                return *sharedAnimation->tweens;
            return sNoTweens;
        }

        auto active_tweens() const
        {
            return asset_tweens() | std::views::filter([this](animation_tween_ptr const& aTween)
                { return tweenAnimationStates.contains(aTween); });
        }

        auto active_tweens(patch_ptr const& aPatch) const
        {
            return active_tweens() | std::views::filter([aPatch](animation_tween_ptr const& aTween)
                { return std::ranges::contains(aTween->patches, aPatch); });
        }

        bool any_active_tweens() const
        {
            return std::ranges::any_of(tweenAnimationStates, [](auto const& aTweenState)
                { return aTweenState.second.active && aTweenState.second.timer && aTweenState.second.timer->running(); });
        }

        void start_frames(i64 aStepTime)
        {
            frameAnimationState.start(aStepTime);
        }

        void stop_frames()
        {
            frameAnimationState.stop();
        }

        void start_tweens()
        {
            for (auto& tween : active_tweens())
                tweenAnimationStates[tween].start();
        }

        void start_tweens(patch_ptr const& aPatch)
        {
            for (auto& tween : active_tweens(aPatch))
                tweenAnimationStates[tween].start();
        }

        void stop_tweens()
        {
            for (auto& tween : active_tweens())
                tweenAnimationStates[tween].stop();
        }

        void stop_tweens(patch_ptr const& aPatch)
        {
            for (auto& tween : active_tweens(aPatch))
                tweenAnimationStates[tween].stop();
        }

        mat44f operator()(i64 aStepTime, patch_ptr const& aPatch) const
        {
            auto result = mat44f::identity();

            for (auto& tween : active_tweens(aPatch))
            {
                auto& tweenState = tweenAnimationStates.at(tween);
                if (tweenState.active)
                    result *= (*tween)(tweenState.timer ? from_step_time(tweenState.timer->elapsed(aStepTime)) : 0.0s);
            }

            return result;
        }

        struct meta : i_component_data::meta
        {
            static const neolib::uuid& id()
            {
                static const neolib::uuid sId = { 0xe876ec93, 0x21b2, 0x410e, 0xbd81, { 0x8c, 0x1d, 0x9, 0xec, 0xbc, 0x45 } };
                return sId;
            }
            static const i_string& name()
            {
                static const string sName = "Animation Filter";
                return sName;
            }
            static std::uint32_t field_count()
            {
                return 3;
            }
            static component_data_field_type field_type(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                    return component_data_field_type::ComponentData | component_data_field_type::Shared;
                case 1:
                    return component_data_field_type::ComponentData | component_data_field_type::Optional;
                case 2:
                    return component_data_field_type::Mat44f | component_data_field_type::Optional;
                default:
                    throw invalid_field_index();
                }
            }
            static neolib::uuid field_type_id(std::uint32_t aFieldIndex)
            {
                switch (aFieldIndex)
                {
                case 0:
                case 1:
                    return animation::meta::id();
                case 2:
                    return neolib::uuid{};
                default:
                    throw invalid_field_index();
                }
            }
            static const i_string& field_name(std::uint32_t aFieldIndex)
            {
                static const string sFieldNames[] =
                {
                    "Shared Animation",
                    "Animation",
                    "Transformation"
                };
                return sFieldNames[aFieldIndex];
            }
        };
    };

    inline bool has_animation(animation_filter const& aAnimationFilter)
    {
        return aAnimationFilter.animation || aAnimationFilter.sharedAnimation;
    }

    inline animation& to_animation(animation_filter& aAnimationFilter)
    {
        if (aAnimationFilter.animation)
            return *aAnimationFilter.animation;
        else if (aAnimationFilter.sharedAnimation)
            return *aAnimationFilter.sharedAnimation;
        throw std::logic_error("neogfx::game::to_animation: no animation!");
    }

    inline animation const& to_animation(animation_filter const& aAnimationFilter)
    {
        if (aAnimationFilter.animation)
            return *aAnimationFilter.animation;
        else if (aAnimationFilter.sharedAnimation)
            return *aAnimationFilter.sharedAnimation;
        throw std::logic_error("neogfx::game::to_animation: no animation!");
    }

    inline void start_animation(animation_filter& aAnimationFilter, i64 aStepTime)
    {
        if (has_animation(aAnimationFilter))
        {
            aAnimationFilter.start_frames(aStepTime);
            aAnimationFilter.start_tweens();
        }
    }

    inline void start_animation(i_ecs& aEcs, animation_filter& aAnimationFilter)
    {
        start_animation(aAnimationFilter, aEcs.system<game::time>().world_time());
    }

    inline void start_frame_animation(animation_filter& aAnimationFilter, i64 aStepTime)
    {
        if (has_animation(aAnimationFilter))
            aAnimationFilter.start_frames(aStepTime);
    }

    inline void start_frame_animation(i_ecs& aEcs, animation_filter& aAnimationFilter)
    {
        if (has_animation(aAnimationFilter))
            aAnimationFilter.start_frames(aEcs.system<game::time>().world_time());
    }

    inline void start_tween_animation(animation_filter& aAnimationFilter)
    {
        if (has_animation(aAnimationFilter))
            aAnimationFilter.start_tweens();
    }

    inline void start_tween_animation(animation_filter& aAnimationFilter, patch_ptr const& aPatch)
    {
        if (has_animation(aAnimationFilter))
            aAnimationFilter.start_tweens(aPatch);
    }

    inline void stop_animation(animation_filter& aAnimationFilter)
    {
        if (has_animation(aAnimationFilter))
        {
            aAnimationFilter.stop_frames();
            aAnimationFilter.stop_tweens();
        }
    }

    inline void stop_frame_animation(animation_filter& aAnimationFilter)
    {
        if (has_animation(aAnimationFilter))
            aAnimationFilter.stop_frames();
    }

    inline void stop_tween_animation(animation_filter& aAnimationFilter)
    {
        if (has_animation(aAnimationFilter))
            aAnimationFilter.stop_tweens();
    }

    inline void stop_tween_animation(animation_filter& aAnimationFilter, patch_ptr const& aPatch)
    {
        if (has_animation(aAnimationFilter))
            aAnimationFilter.stop_tweens(aPatch);
    }

    inline void stop_animation_on_tween_complete(animation_filter& aAnimationFilter, animation_tween_ptr const& aTween)
    {
        auto existing = aAnimationFilter.tweenAnimationStates.find(aTween);
        if (existing == aAnimationFilter.tweenAnimationStates.end())
            throw std::logic_error("neogfx::game::stop_animation_on_tween_complete: tween not present in filter!");
        if (existing->second.timer)
            aAnimationFilter.completionTimers.insert(existing->second.timer);
        else
            aAnimationFilter.completionTimers.insert(aTween);
    }

    inline void stop_animation_on_tween_complete(animation_filter& aAnimationFilter, patch_ptr const& aPatch)
    {
        for (auto const& tween : aAnimationFilter.active_tweens(aPatch))
        {
            if (aAnimationFilter.tweenAnimationStates.at(tween).timer)
                aAnimationFilter.completionTimers.insert(aAnimationFilter.tweenAnimationStates.at(tween).timer);
            else
                aAnimationFilter.completionTimers.insert(tween);
        }
    }

    inline void stop_animation_on_tween_complete(animation_filter& aAnimationFilter, u32 aTweenIndex = 0u)
    {
        auto const& tweens = aAnimationFilter.asset_tweens();
        if (aTweenIndex >= tweens.size())
            throw std::logic_error("neogfx::game::stop_animation_on_tween_complete: bad tween index!");
        stop_animation_on_tween_complete(aAnimationFilter, tweens[aTweenIndex]);
    }

    inline bool has_animation_frames(animation_filter const& aAnimationFilter)
    {
        return has_animation(aAnimationFilter) && to_animation(aAnimationFilter).frames;
    }

    inline animation_frames const& to_animation_frames(animation_filter const& aAnimationFilter)
    {
        if (has_animation_frames(aAnimationFilter))
            return *to_animation(aAnimationFilter).frames;
        throw std::logic_error("neogfx::game::to_animation_frames: no animation frames!");
    }

    inline mesh_filter const& current_animation_frame(animation_filter const& aAnimationFilter)
    {
        if (has_animation_frames(aAnimationFilter))
            return to_animation_frames(aAnimationFilter)[aAnimationFilter.frameAnimationState.currentFrame].filter;
        throw std::logic_error("neogfx::game::to_animation_frames: no animation frames!");
    }

    inline bool is_tweening_animation(animation_filter const& aAnimationFilter)
    {
        return has_animation(aAnimationFilter) && to_animation(aAnimationFilter).tweens;
    }

    inline bool has_active_tweens(animation_filter const& aAnimationFilter)
    {
        return aAnimationFilter.any_active_tweens();
    }

    inline mat44f const& to_transformation_matrix(animation_filter const& aAnimationFilter)
    {
        return aAnimationFilter.transformation ? *aAnimationFilter.transformation : mat44f::identity();
    }

    inline mat44f to_transformation_matrix(animation_filter& aAnimationFilter, i64 aStepTime, patch_ptr const& aPatch = mesh_filter_patch)
    {
        if (has_animation(aAnimationFilter))
            return aAnimationFilter(aStepTime, aPatch);
        return mat44f::identity();
    }

    inline mat44f to_transformation_matrix(i_ecs const& aEcs, animation_filter& aAnimationFilter, patch_ptr const& aPatch = mesh_filter_patch)
    {
        return to_transformation_matrix(aAnimationFilter, aEcs.system<game::time>().world_time(), aPatch);
    }

    inline animation_tween& add_tween(animation_filter& aAnimationFilter, time_interval const& aDuration, patches const& aPatches)
    {
        auto tween = add_tween(to_animation(aAnimationFilter), aDuration, aPatches);
        (void)aAnimationFilter.tweenAnimationStates[tween];
        return *tween;
    }

    inline animation_tween& add_tween(animation_filter& aAnimationFilter, time_interval const& aDuration)
    {
        return add_tween(aAnimationFilter, aDuration, { mesh_filter_patch });
    }

    inline animation_tween& translate(animation_filter& aAnimationFilter, time_interval const& aDuration, vec3_range const& aRange, patches const& aPatches)
    {
        return translate(add_tween(aAnimationFilter, aDuration, aPatches), aRange);
    }

    inline animation_tween& translate(animation_filter& aAnimationFilter, time_interval const& aDuration, vec3_range const& aRange)
    {
        return translate(aAnimationFilter, aDuration, aRange, { mesh_filter_patch });
    }

    inline animation_tween& scale(animation_filter& aAnimationFilter, time_interval const& aDuration, vec3_range const& aRange, patches const& aPatches)
    {
        return scale(add_tween(aAnimationFilter, aDuration, aPatches), aRange);
    }

    inline animation_tween& scale(animation_filter& aAnimationFilter, time_interval const& aDuration, vec3_range const& aRange)
    {
        return scale(aAnimationFilter, aDuration, aRange, { mesh_filter_patch });
    }

    inline animation_tween& rotate(animation_filter& aAnimationFilter, time_interval const& aDuration, vec3_range const& aRange, patches const& aPatches)
    {
        return rotate(add_tween(aAnimationFilter, aDuration, aPatches), aRange);
    }

    inline animation_tween& rotate(animation_filter& aAnimationFilter, time_interval const& aDuration, vec3_range const& aRange)
    {
        return rotate(aAnimationFilter, aDuration, aRange, { mesh_filter_patch });
    }

    inline animation_tween& rotate_deg(animation_filter& aAnimationFilter, time_interval const& aDuration, vec3_range const& aRange, patches const& aPatches)
    {
        return rotate(aAnimationFilter, aDuration, { aRange.start * std::numbers::pi / 180.0, aRange.end * std::numbers::pi / 180.0 }, aPatches);
    }

    inline animation_tween& rotate_deg(animation_filter& aAnimationFilter, time_interval const& aDuration, vec3_range const& aRange)
    {
        return rotate_deg(aAnimationFilter, aDuration, aRange, { mesh_filter_patch });
    }

    inline patches all_patches(mesh_renderer const& aMeshRenderer)
    {
        patches allPatches;
        allPatches.reserve(aMeshRenderer.patches.size() + 1u);
        allPatches.push_back(mesh_filter_patch);
        allPatches.insert(allPatches.end(), aMeshRenderer.patches.begin(), aMeshRenderer.patches.end());
        return allPatches;
    }

    inline animation_tween& translate(animation_filter& aAnimationFilter, time_interval const& aDuration, vec3_range const& aRange, mesh_renderer const& aMeshRenderer)
    {
        return translate(aAnimationFilter, aDuration, aRange, all_patches(aMeshRenderer));
    }

    inline animation_tween& scale(animation_filter& aAnimationFilter, time_interval const& aDuration, vec3_range const& aRange, mesh_renderer const& aMeshRenderer)
    {
        return scale(aAnimationFilter, aDuration, aRange, all_patches(aMeshRenderer));
    }

    inline animation_tween& rotate(animation_filter& aAnimationFilter, time_interval const& aDuration, vec3_range const& aRange, mesh_renderer const& aMeshRenderer)
    {
        return rotate(aAnimationFilter, aDuration, aRange, all_patches(aMeshRenderer));
    }

    inline animation_tween& rotate_deg(animation_filter& aAnimationFilter, time_interval const& aDuration, vec3_range const& aRange, mesh_renderer const& aMeshRenderer)
    {
        return rotate_deg(aAnimationFilter, aDuration, aRange, all_patches(aMeshRenderer));
    }

    enum class tween_type : std::uint32_t { Translate, Scale, Rotate, RotateDeg };

    struct tween_info
    {
        tween_type type;
        time_interval duration;
        vec3_range range;
        game::patches patches;
        tween_cycle cycle;
        std::optional<vec3f> pivot;
    };

    inline animation_tween& add_tween(animation_filter& aAnimationFilter, tween_info const& aInfo)
    {
        auto& tween = [&]() -> animation_tween&
            {
                switch (aInfo.type)
                {
                case tween_type::Translate:
                    return translate(aAnimationFilter, aInfo.duration, aInfo.range, aInfo.patches);
                case tween_type::Scale:
                    return scale(aAnimationFilter, aInfo.duration, aInfo.range, aInfo.patches);
                case tween_type::Rotate:
                    return rotate(aAnimationFilter, aInfo.duration, aInfo.range, aInfo.patches);
                case tween_type::RotateDeg:
                    return rotate_deg(aAnimationFilter, aInfo.duration, aInfo.range, aInfo.patches);
                default:
                    throw std::logic_error("neogfx::game::add_tween: unknown tween type");
                }
            }();
        tween.cycle = aInfo.cycle;
        tween.pivot = aInfo.pivot;
        return tween;
    }

    inline void add_tweens(animation_filter& aAnimationFilter, std::initializer_list<tween_info> const& aInfos)
    {
        for (auto i = aInfos.begin(); i != aInfos.end(); ++i)
            add_tween(aAnimationFilter, *i);
    }

    using tween_duration_info = std::variant<std::monostate, time_interval, animation_timer_ptr>;

    inline animation_tween& add_tween(animation_filter& aAnimationFilter, tween_info const& aInfo, tween_duration_info const& aDurationInfo)
    {
        auto& tween = add_tween(aAnimationFilter, aInfo);
        auto const& tweenPtr = to_animation(aAnimationFilter).tweens->back();
        if (tweenPtr.get() != &tween)
            throw std::logic_error("neogfx::game::add_tween");
        auto& tweenState = aAnimationFilter.tweenAnimationStates[tweenPtr];
        if (auto timer = std::get_if<animation_timer_ptr>(&aDurationInfo))
            tweenState.timer = *timer;
        else if (auto duration = std::get_if<time_interval>(&aDurationInfo))
            tweenState.timerDuration = *duration;
        return tween;
    }

    inline void add_tweens(animation_filter& aAnimationFilter, std::initializer_list<tween_info> const& aInfos,
        std::initializer_list<tween_duration_info> const& aDurationInfos)
    {
        if (aInfos.size() != aDurationInfos.size())
            throw std::logic_error("neogfx::game::add_tweens");
        auto durationInfo = aDurationInfos.begin();
        for (auto i = aInfos.begin(); i != aInfos.end(); ++i, ++durationInfo)
            add_tween(aAnimationFilter, *i, *durationInfo);
    }

    inline animation_filter& create_animation(i_ecs& aEcs, entity_id aId, vec3f const& aOrigin, std::span<tween_info> aTweens, std::span<tween_duration_info const> aDurationInfos, std::optional<time_interval> const& aDuration = {}, i32 aLayer = 0)
    {
        if (!aDurationInfos.empty() && aDurationInfos.size() != aTweens.size())
            throw std::logic_error("neogfx::game::create_animation");

        scoped_component_data_lock<mesh_renderer, mesh_filter, animation_filter, entity_life_span> lock{ aEcs };

        if (aDuration)
            aEcs.populate(aId, entity_life_span{ to_step_time(aEcs, *aDuration) });

        auto& mr = aEcs.component<mesh_renderer>().entity_record(aId);
        mr.layer = aLayer;

        auto& mf = aEcs.component<mesh_filter>().entity_record(aId);
        mf.transformation.emplace(mat44f::identity());
        neolib::apply_translation(*mf.transformation, aOrigin);

        aEcs.component<animation_filter>().populate(aId, {});
        auto& af = aEcs.component<animation_filter>().entity_record(aId);
        af.animation.emplace();

        std::optional<decltype(all_patches(mr))> defaultPatches;
        auto durationInfo = aDurationInfos.begin();
        for (auto& tween : aTweens)
        {
            if (tween.patches.empty())
            {
                if (!defaultPatches)
                    defaultPatches.emplace(all_patches(mr));
                tween.patches = *defaultPatches;
            }
            if (aDurationInfos.empty())
                add_tween(af, tween);
            else
                add_tween(af, tween, *durationInfo++);
        }

        return af;
    }

    inline animation_filter& create_animation(i_ecs& aEcs, entity_id aId, vec3f const& aOrigin, std::span<tween_info> aTweens, std::optional<time_interval> const& aDuration = {}, i32 aLayer = 0)
    {
        return create_animation(aEcs, aId, aOrigin, aTweens, std::span<tween_duration_info const>{}, aDuration, aLayer);
    }

    template <typename Tweens> requires (!std::is_lvalue_reference_v<Tweens>)
    inline animation_filter& create_animation(i_ecs& aEcs, entity_id aId, vec3f const& aOrigin, Tweens&& aTweens, std::optional<time_interval> const& aDuration = {}, i32 aLayer = 0)
    {
        return create_animation(aEcs, aId, aOrigin, std::span<tween_info>{ aTweens }, aDuration, aLayer);
    }

    template <typename Tweens, typename DurationInfos>
        requires (!std::is_lvalue_reference_v<Tweens> && !std::is_lvalue_reference_v<DurationInfos>&&
    std::is_constructible_v<std::span<tween_info>, Tweens&>&&
        std::is_constructible_v<std::span<tween_duration_info const>, DurationInfos&>)
        inline animation_filter& create_animation(i_ecs& aEcs, entity_id aId, vec3f const& aOrigin, Tweens&& aTweens, DurationInfos&& aDurationInfos, std::optional<time_interval> const& aDuration = {}, i32 aLayer = 0)
    {
        return create_animation(aEcs, aId, aOrigin, std::span<tween_info>{ aTweens }, std::span<tween_duration_info const>{ aDurationInfos }, aDuration, aLayer);
    }
}
