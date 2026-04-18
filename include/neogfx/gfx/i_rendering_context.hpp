// i_rendering_context.hpp
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

#include <neogfx/core/geometrical.hpp>
#include <neogfx/hid/i_display.hpp>
#include <neogfx/gfx/primitives.hpp>
#include <neogfx/gfx/graphics_operations.hpp>

namespace neogfx
{
    class i_rendering_engine;
    class i_render_target;
    class i_gradient_shader;

    struct rendering_context_state
    {
        std::uint64_t generation = 0;

        bool operator==(rendering_context_state const&) const { return true; }
        auto operator<=>(rendering_context_state const&) const { return std::strong_ordering::equal; }
    };

    struct rendering_context_fast_state : rendering_context_state
    {
        point origin;
        std::optional<rect> clipRegion;
        double opacity = 1.0;

        bool operator==(rendering_context_fast_state const&) const = default;
        auto operator<=>(rendering_context_fast_state const&) const = default;
    };

    struct rendering_context_slow_state : rendering_context_state
    {
        std::optional<logical_coordinate_system> logicalCoordinateSystem;
        std::optional<logical_coordinates> logicalCoordinates;
        std::optional<bool> multisample;
        std::optional<double> sampleShadingRate;
        std::optional<front_face> frontFace;
        std::optional<face_culling> faceCulling;
        std::optional<blending_mode> blendingMode;
        std::optional<smoothing_mode> smoothingMode;

        bool operator==(rendering_context_slow_state const&) const = default;
        auto operator<=>(rendering_context_slow_state const&) const = default;
    };

    using i_rendering_queue = i_vector<maybe_abstract_t<graphics_operation::operation>>;
    using rendering_queue = vector<graphics_operation::operation>;

    struct queue_batch_item
    {
        graphics_operation::operation const* op;
        std::int32_t ordinal;
        rendering_context_fast_state const* fastState;
        rendering_context_slow_state const* slowState;
        graphics_operation::operation const& operator*() const { return *op; }
        graphics_operation::operation const* operator->() const { return op; }
    };

    using i_optimised_rendering_queue = i_vector<maybe_abstract_t<queue_batch_item>>;
    using optimised_rendering_queue = vector<queue_batch_item>;

    using render_batch = std::ranges::subrange<queue_batch_item const*>;

    class i_rendering_context
    {
    public:
        struct texture_not_resident : std::runtime_error { texture_not_resident() : std::runtime_error("neogfx::i_rendering_context::texture_not_resident") {} };
    public:
        virtual ~i_rendering_context() = default;
        virtual std::unique_ptr<i_rendering_context> clone() const = 0;
    public:
        virtual i_rendering_engine& rendering_engine() const = 0;
        virtual const i_render_target& render_target() const = 0;
        virtual rect rendering_area(bool aConsiderScissor = true) const = 0;
        virtual i_rendering_queue& queue() const = 0;
        virtual void enqueue(graphics_operation::operation const& aOperation) = 0;
        virtual void flush() = 0;
    public:
        virtual bool redirecting() const = 0;
        virtual point redirect_origin() const = 0;
        virtual void begin_redirect(i_rendering_context& aRcBase, point const& aOrigin) = 0;
        virtual void end_redirect() = 0;
    public:
        virtual neogfx::logical_coordinate_system logical_coordinate_system() const = 0;
        virtual void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) = 0;
        virtual neogfx::logical_coordinates logical_coordinates() const = 0;
        virtual void set_logical_coordinates(neogfx::logical_coordinates const& aCoordinates) = 0;
    public:
        virtual point origin() const = 0;
        virtual void set_origin(point const& aOrigin) = 0;
        virtual vec2 offset() const = 0;
        virtual void set_offset(optional_vec2 const& aOffset) = 0;
        virtual void blit(rect const& aDestinationRect, i_texture const& aTexture, rect const& aSourceRect, blending_mode aBlendingMode = blending_mode::Blit) = 0;
        virtual bool gradient_set() const = 0;
        virtual void apply_gradient(i_gradient_shader& aShader) = 0;
    public:
        virtual neogfx::subpixel_format subpixel_format() const = 0;
    };
}