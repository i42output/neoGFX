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

#include <neolib/core/uuid.hpp>
#include <neolib/core/string.hpp>

#include <neogfx/game/i_ecs.hpp>
#include <neogfx/game/time.hpp>
#include <neogfx/game/mesh_renderer.hpp>
#include <neogfx/game/animation.hpp>

namespace neogfx::game
{
    // @todo make componenent data (add meta)
    struct frame_animation_state
    {
        bool active = false;
        u32 currentFrame = 0u;
        bool autoDestroy = false;
        std::optional<i64> currentFrameStartTime;

        void start(i64 aStepTime)
        {
            active = true;
            currentFrame = 0u;
            currentFrameStartTime = aStepTime;
        }

        void stop()
        {
            active = false;
        }
    };

    // @todo make componenent data (add meta)
    struct tween_animation_state
    {
        bool active = false;

        void start()
        {
            active = true;
        }

        void stop()
        {
            active = false;
        }
    };

    struct animation_filter
    {
        shared<animation> sharedAnimation;
        std::optional<animation> animation;
        std::optional<mat44f> transformation;

        // @todo add to meta
        frame_animation_state frameState;
        // @todo add to meta
        std::unordered_map<animation_tween_ptr, tween_animation_state> tweenStates;

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
                { return tweenStates.contains(aTween); });
        }

        auto active_tweens(patch_ptr const& aPatch) const
        {
            return active_tweens() | std::views::filter([aPatch](animation_tween_ptr const& aTween)
                { return std::ranges::contains(aTween->patches, aPatch); });
        }

        bool any_active_tweens() const
        {
            return std::ranges::any_of(tweenStates, [](auto const& aTweenState)
                { return aTweenState.second.active && aTweenState.first->clock && !aTweenState.first->clock->paused; });
        }

        void start_frames(i64 aStepTime)
        {
            frameState.start(aStepTime);
        }

        void stop_frames()
        {
            frameState.stop();
        }

        void start_tweens()
        {
            for (auto& tween : active_tweens())
                tweenStates[tween].start();
        }

        void start_tweens(patch_ptr const& aPatch)
        {
            for (auto& tween : active_tweens(aPatch))
                tweenStates[tween].start();
        }

        void stop_tweens()
        {
            for (auto& tween : active_tweens())
                tweenStates[tween].stop();
        }

        void stop_tweens(patch_ptr const& aPatch)
        {
            for (auto& tween : active_tweens(aPatch))
                tweenStates[tween].stop();
        }

        mat44f operator()(i64 aStepTime, patch_ptr const& aPatch) const
        {
            auto result = mat44f::identity();

            for (auto& tween : active_tweens(aPatch))
            {
                auto& tweenState = tweenStates.at(tween);
                if (tweenState.active)
                    result *= (*tween)(tween->clock ? from_step_time(tween->clock->elapsed(aStepTime)) : 0.0);
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
            return to_animation_frames(aAnimationFilter)[aAnimationFilter.frameState.currentFrame].filter;
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

    inline animation_tween& add_tween(animation_filter& aAnimationFilter, scalar aDuration, patches const& aPatches)
    {
        auto tween = add_tween(to_animation(aAnimationFilter), aDuration, aPatches);
        (void)aAnimationFilter.tweenStates[tween];
        return *tween;
    }

    inline animation_tween& add_tween(animation_filter& aAnimationFilter, scalar aDuration)
    {
        return add_tween(aAnimationFilter, aDuration, { mesh_filter_patch });
    }

    inline animation_tween& translate(animation_filter& aAnimationFilter, scalar aDuration, vec3_range const& aRange, patches const& aPatches)
    {
        return translate(add_tween(aAnimationFilter, aDuration, aPatches), aRange);
    }

    inline animation_tween& translate(animation_filter& aAnimationFilter, scalar aDuration, vec3_range const& aRange)
    {
        return translate(aAnimationFilter, aDuration, aRange, { mesh_filter_patch });
    }

    inline animation_tween& scale(animation_filter& aAnimationFilter, scalar aDuration, vec3_range const& aRange, patches const& aPatches)
    {
        return scale(add_tween(aAnimationFilter, aDuration, aPatches), aRange);
    }

    inline animation_tween& scale(animation_filter& aAnimationFilter, scalar aDuration, vec3_range const& aRange)
    {
        return scale(aAnimationFilter, aDuration, aRange, { mesh_filter_patch });
    }

    inline animation_tween& rotate(animation_filter& aAnimationFilter, scalar aDuration, vec3_range const& aRange, patches const& aPatches)
    {
        return rotate(add_tween(aAnimationFilter, aDuration, aPatches), aRange);
    }

    inline animation_tween& rotate(animation_filter& aAnimationFilter, scalar aDuration, vec3_range const& aRange)
    {
        return rotate(aAnimationFilter, aDuration, aRange, { mesh_filter_patch });
    }

    inline animation_tween& rotate_deg(animation_filter& aAnimationFilter, scalar aDuration, vec3_range const& aRange, patches const& aPatches)
    {
        return rotate(aAnimationFilter, aDuration, { aRange.start * std::numbers::pi / 180.0, aRange.end * std::numbers::pi / 180.0 }, aPatches);
    }

    inline animation_tween& rotate_deg(animation_filter& aAnimationFilter, scalar aDuration, vec3_range const& aRange)
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

    inline animation_tween& translate(animation_filter& aAnimationFilter, scalar aDuration, vec3_range const& aRange, mesh_renderer const& aMeshRenderer)
    {
        return translate(aAnimationFilter, aDuration, aRange, all_patches(aMeshRenderer));
    }

    inline animation_tween& scale(animation_filter& aAnimationFilter, scalar aDuration, vec3_range const& aRange, mesh_renderer const& aMeshRenderer)
    {
        return scale(aAnimationFilter, aDuration, aRange, all_patches(aMeshRenderer));
    }

    inline animation_tween& rotate(animation_filter& aAnimationFilter, scalar aDuration, vec3_range const& aRange, mesh_renderer const& aMeshRenderer)
    {
        return rotate(aAnimationFilter, aDuration, aRange, all_patches(aMeshRenderer));
    }

    inline animation_tween& rotate_deg(animation_filter& aAnimationFilter, scalar aDuration, vec3_range const& aRange, mesh_renderer const& aMeshRenderer)
    {
        return rotate_deg(aAnimationFilter, aDuration, aRange, all_patches(aMeshRenderer));
    }
}
