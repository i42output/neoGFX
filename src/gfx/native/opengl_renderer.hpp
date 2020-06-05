// opengl_renderer.hpp
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
#include <set>
#include <map>
#include <neolib/task/timer.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/text/font_manager.hpp>
#include <neogfx/gfx/i_standard_shader_program.hpp>
#include "opengl.hpp"
#include "opengl_texture_manager.hpp"
#include "opengl_helpers.hpp"

std::string glErrorString(GLenum aErrorCode);
GLenum glCheckError(const char* file, unsigned int line);

namespace neogfx
{
    class frame_counter
    {
    public:
        frame_counter(uint32_t aDuration);
    public:
        uint32_t counter() const;
    public:
        void add(i_widget& aWidget);
        void remove(i_widget& aWidget);
    private:
        neolib::callback_timer iTimer;
        uint32_t iCounter;
        std::vector<i_widget*> iWidgets;
    };

    class opengl_renderer : public i_rendering_engine
    {
        // events
    public:
        define_declared_event(SubpixelRenderingChanged, subpixel_rendering_changed)
        // exceptions
    public:
        struct shader_program_error : i_rendering_engine::shader_program_error {
            shader_program_error(GLenum aErrorCode) :
                i_rendering_engine::shader_program_error(glErrorString(aErrorCode)) {}
        };
        struct failed_to_create_framebuffer : std::runtime_error {
            failed_to_create_framebuffer(GLenum aErrorCode) :
                std::runtime_error("neogfx::opengl_renderer::failed_to_create_framebuffer: Failed to create frame buffer, reason: " + glErrorString(aErrorCode)) {}
        };
        // types
    public:
        typedef neolib::vector<neolib::ref_ptr<i_shader_program>> shader_program_list;
        typedef std::map<std::pair<texture_sampling, size>, texture> ping_pong_buffers_t;
        typedef i_rendering_engine::handle opengl_context;
        // construction
    public:
        opengl_renderer(neogfx::renderer aRenderer);
        ~opengl_renderer();
    public:
        const i_device_metrics& default_screen_metrics() const override;
    public:
        neogfx::renderer renderer() const override;
        void initialize() override;
        void cleanup() override;
    public:
        const shader_program_list& shader_programs() const override;
        const i_shader_program& shader_program(const neolib::i_string& aName) const override;
        i_shader_program& shader_program(const neolib::i_string& aName) override;
        i_shader_program& add_shader_program(const neolib::i_ref_ptr<i_shader_program>& aShaderProgram) override;
        bool is_shader_program_active() const override;
        i_shader_program& active_shader_program() override;
    public:
        const i_standard_shader_program& default_shader_program() const override;
        i_standard_shader_program& default_shader_program() override;
    public:
        handle create_shader_program_object() override;
        void destroy_shader_program_object(handle aShaderProgramObject) override;
        handle create_shader_object(shader_type aShaderType) override;
        void destroy_shader_object(handle aShaderObject) override;
    public:
        i_font_manager& font_manager() override;
        i_texture_manager& texture_manager() override;
    public:
        void allocate_vertex_arrays(void const* aConsumer) override;
        void deallocate_vertex_arrays(void const* aConsumer) override;
        const opengl_standard_vertex_arrays& vertex_arrays(void const* aConsumer) const override;
        opengl_standard_vertex_arrays& vertex_arrays(void const* aConsumer) override;
        void execute_vertex_arrays() override;
    public:
        i_texture& ping_pong_buffer1(const size& aExtents, texture_sampling aSampling = texture_sampling::Multisample) override;
        i_texture& ping_pong_buffer2(const size& aExtents, texture_sampling aSampling = texture_sampling::Multisample) override;
    public:
        bool is_subpixel_rendering_on() const override;
        void subpixel_rendering_on() override;
        void subpixel_rendering_off() override;
        bool frame_rate_limited() const override;
        void enable_frame_rate_limiter(bool aEnable) override;
        uint32_t frame_rate_limit() const override;
        void set_frame_rate_limit(uint32_t aFps) override;
    public:
        bool process_events() override;
    public:
        void register_frame_counter(i_widget& aWidget, uint32_t aDuration) override;
        void unregister_frame_counter(i_widget& aWidget, uint32_t aDuration) override;
        uint32_t frame_counter(uint32_t aDuration) const override;
        i_texture& create_ping_pong_buffer(ping_pong_buffers_t& aBufferList, const size& aExtents, texture_sampling aSampling);
    private:
        neogfx::renderer iRenderer;
        mutable std::optional<opengl_texture_manager> iTextureManager;
        mutable std::optional<neogfx::font_manager> iFontManager;
        mutable shader_program_list iShaderPrograms;
        bool iLimitFrameRate;
        uint32_t iFrameRateLimit;
        bool iSubpixelRendering;
        typedef std::unordered_map<void const*, opengl_standard_vertex_arrays> vertex_arrays_map;
        mutable vertex_arrays_map iVertexArrays;
        mutable std::optional<vertex_arrays_map::iterator> iLastVertexArraysUsed;
        std::map<uint32_t, neogfx::frame_counter> iFrameCounters;
        ping_pong_buffers_t iPingPongBuffer1s;
        ping_pong_buffers_t iPingPongBuffer2s;
        ref_ptr<i_standard_shader_program> iDefaultShaderProgram;
    };
}