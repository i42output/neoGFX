// i_rendering_engine.hpp
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
#include <neogfx/core/numerical.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/gui/window/window_bits.hpp>
#include <neogfx/hid/video_mode.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/gfx/i_shader.hpp>
#include <neogfx/gfx/i_standard_shader_program.hpp>
#include <neogfx/gfx/i_vertex_buffer.hpp>

namespace neogfx
{
    class i_surface_manager;
    class i_native_surface;
    class i_native_window;
    class i_rendering_context;
    class i_font_manager;
    class i_texture_manager;
    class i_render_target;
    class i_vertex_buffer;
    class i_vertex_provider;

    enum class renderer
    {
        OpenGL,
        DirectX,
        Vulkan,
        Software,
        None
    };

    class i_rendering_engine
    {
        // events
    public:
        declare_event(subpixel_rendering_changed)
        // exceptions
    public:
        struct failed_to_initialize : std::runtime_error { failed_to_initialize() : std::runtime_error("neogfx::i_rendering_engine::failed_to_initialize") {} };
        struct context_exists : std::logic_error { context_exists() : std::logic_error("neogfx::i_rendering_engine::context_exists") {} };
        struct context_not_found : std::logic_error { context_not_found() : std::logic_error("neogfx::i_rendering_engine::context_not_found") {} };
        struct consumer_exists : std::logic_error { consumer_exists() : std::logic_error("neogfx::i_rendering_engine::consumer_exists") {} };
        struct consumer_not_found : std::logic_error { consumer_not_found() : std::logic_error("neogfx::i_rendering_engine::consumer_not_found") {} };
        struct failed_to_create_shader_program : std::runtime_error { failed_to_create_shader_program(const std::string& aReason) : std::runtime_error("neogfx::i_rendering_engine::failed_to_create_shader_program: " + aReason) {} };
        struct no_shader_program_active : std::logic_error { no_shader_program_active() : std::logic_error("neogfx::i_rendering_engine::no_shader_program_active") {} };
        struct shader_program_not_found : std::logic_error { shader_program_not_found() : std::logic_error("neogfx::i_rendering_engine::shader_program_not_found") {} };
        struct shader_program_error : std::runtime_error { shader_program_error(const std::string& aError) : std::runtime_error("neogfx::i_rendering_engine::shader_program_error: " + aError) {} };
        // types
    public:
        typedef void* handle;
        typedef int pixel_format_t;
        typedef neolib::i_vector<neolib::i_ref_ptr<i_shader_program>> shader_program_list;
        // construction
    public:
        virtual ~i_rendering_engine() = default;
        // operations
    public:
        virtual const i_device_metrics& default_screen_metrics() const = 0;
    public:
        virtual neogfx::renderer renderer() const = 0;
        virtual bool double_buffering() const = 0;
        virtual bool vsync_enabled() const = 0;
        virtual void enable_vsync() = 0;
        virtual void disable_vsync() = 0;
        virtual void initialize() = 0;
        virtual void cleanup() = 0;
        virtual pixel_format_t set_pixel_format(const i_render_target& aTarget) = 0;
        virtual const i_render_target* active_target() const = 0;
        virtual void activate_context(const i_render_target& aTarget) = 0;
        virtual void deactivate_context() = 0;
        virtual handle create_context(const i_render_target& aTarget) = 0;
        virtual void destroy_context(handle aContext) = 0;
    public:
        virtual const shader_program_list& shader_programs() const = 0;
        virtual const i_shader_program& shader_program(const neolib::i_string& aName) const = 0;
        virtual i_shader_program& shader_program(const neolib::i_string& aName) = 0;
        virtual i_shader_program& add_shader_program(const neolib::i_ref_ptr<i_shader_program>& aShaderProgram) = 0;
        virtual bool is_shader_program_active() const = 0;
        virtual i_shader_program& active_shader_program() = 0;
    public:
        virtual const i_standard_shader_program& default_shader_program() const = 0;
        virtual i_standard_shader_program& default_shader_program() = 0;
    public:
        virtual handle create_shader_program_object() = 0;
        virtual void destroy_shader_program_object(handle aShaderProgramObject) = 0;
        virtual handle create_shader_object(shader_type aShaderType) = 0;
        virtual void destroy_shader_object(handle aShaderObject) = 0;
    public:
        virtual std::unique_ptr<i_native_window> create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle = window_style::Default) = 0;
        virtual std::unique_ptr<i_native_window> create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default) = 0;
        virtual std::unique_ptr<i_native_window> create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default) = 0;
        virtual std::unique_ptr<i_native_window> create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_surface& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle = window_style::Default) = 0;
        virtual std::unique_ptr<i_native_window> create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_surface& aParent, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default) = 0;
        virtual std::unique_ptr<i_native_window> create_window(i_surface_manager& aSurfaceManager, i_surface_window& aWindow, i_native_surface& aParent, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default) = 0;
        virtual bool creating_window() const = 0;
        virtual i_font_manager& font_manager() = 0;
        virtual i_texture_manager& texture_manager() = 0;
    public:
        virtual i_vertex_buffer& allocate_vertex_buffer(i_vertex_provider& aProvider, vertex_buffer_type aType = vertex_buffer_type::Default) = 0;
        virtual void deallocate_vertex_buffer(i_vertex_provider& aProvider) = 0;
        virtual const i_vertex_buffer& vertex_buffer(i_vertex_provider& aProvider) const = 0;
        virtual i_vertex_buffer& vertex_buffer(i_vertex_provider& aProvider) = 0;
        virtual void execute_vertex_buffers() = 0;
    public:
        virtual i_texture& ping_pong_buffer1(const size& aExtents, texture_sampling aSampling = texture_sampling::Multisample) = 0;
        virtual i_texture& ping_pong_buffer2(const size& aExtents, texture_sampling aSampling = texture_sampling::Multisample) = 0;
    public:
        virtual bool is_subpixel_rendering_on() const = 0;
        virtual void subpixel_rendering_on() = 0;
        virtual void subpixel_rendering_off() = 0;
    public:
        virtual void render_now() = 0;
        virtual bool frame_rate_limited() const = 0;
        virtual void enable_frame_rate_limiter(bool aEnable) = 0;
        virtual uint32_t frame_rate_limit() const = 0;
        virtual void set_frame_rate_limit(uint32_t aFps) = 0;
        virtual bool use_rendering_priority() const = 0;
    public:
        virtual bool process_events() = 0;
    public:
        virtual void register_frame_counter(i_widget& aWidget, uint32_t aDuration) = 0;
        virtual void unregister_frame_counter(i_widget& aWidget, uint32_t aDuration) = 0;
        virtual uint32_t frame_counter(uint32_t aDuration) const = 0;
    };
}