// i_rendering_engine.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gfx/shader_array.hpp>

namespace neogfx
{
	class i_surface_manager;
	class i_native_surface;
	class i_native_window;
	class i_graphics_context;
	class i_font_manager;
	class i_texture_manager;
	class i_render_target;


	class opengl_standard_vertex_arrays; // todo: abstract

	enum class renderer
	{
		OpenGL,
		DirectX,
		Vulkan,
		Software
	};

	static const uint32_t GRADIENT_FILTER_SIZE = 15;
	struct gradient_arrays
	{
		shader_array<float> stops = { size_u32{gradient::MaxStops, 1} };
		shader_array<std::array<float, 4>> stopColours = { size_u32{gradient::MaxStops, 1} };
		shader_array<float> filter = { size_u32{GRADIENT_FILTER_SIZE, GRADIENT_FILTER_SIZE} };
	};

	class i_rendering_engine
	{
	public:
		event<> subpixel_rendering_changed;
	public:
		typedef void* opengl_context;
		class i_shader_program
		{
		public:
			struct variable_not_found : std::logic_error { variable_not_found() : std::logic_error("neogfx::i_rendering_engine::i_shader_program::variable_not_found") {} };
		public:
			virtual void* handle() const = 0;
			virtual bool has_projection_matrix() const = 0;
			virtual const optional_mat44& projection_matrix() const = 0;
			virtual void set_projection_matrix(const i_graphics_context& aGraphicsContext, const optional_mat44& aProjectionMatrix = optional_mat44{}) = 0;
			virtual bool has_transformation_matrix() const = 0;
			virtual const optional_mat44& transformation_matrix() const = 0;
			virtual void set_transformation_matrix(const i_graphics_context& aGraphicsContext, const optional_mat44& aProjectionMatrix = optional_mat44{}) = 0;
			virtual void* variable(const std::string& aVariableName) const = 0;
			virtual void set_uniform_variable(const std::string& aName, float aValue) = 0;
			virtual void set_uniform_variable(const std::string& aName, double aValue) = 0;
			virtual void set_uniform_variable(const std::string& aName, int aValue) = 0;
			virtual void set_uniform_variable(const std::string& aName, float aValue1, float aValue2) = 0;
			virtual void set_uniform_variable(const std::string& aName, double aValue1, double aValue2) = 0;
			virtual void set_uniform_variable(const std::string& aName, const vec4f& aVector) = 0;
			virtual void set_uniform_variable(const std::string& aName, const vec4& aVector) = 0;
			virtual void set_uniform_array(const std::string& aName, uint32_t aSize, const float* aArray) = 0;
			virtual void set_uniform_array(const std::string& aName, uint32_t aSize, const double* aArray) = 0;
			virtual void set_uniform_matrix(const std::string& aName, const mat44::template rebind<float>::type& aMatrix) = 0;
			virtual void set_uniform_matrix(const std::string& aName, const mat44::template rebind<double>::type& aMatrix) = 0;
		};
	public:
		virtual ~i_rendering_engine() {}
	public:
		struct failed_to_initialize : std::runtime_error { failed_to_initialize() : std::runtime_error("neogfx::i_rendering_engine::failed_to_initialize") {} };
		struct context_exists : std::logic_error { context_exists() : std::logic_error("neogfx::i_rendering_engine::context_exists") {} };
		struct context_not_found : std::logic_error { context_not_found() : std::logic_error("neogfx::i_rendering_engine::context_not_found") {} };
		struct failed_to_create_shader_program : std::runtime_error { failed_to_create_shader_program(const std::string& aReason) : std::runtime_error("neogfx::i_rendering_engine::failed_to_create_shader_program: " + aReason) {} };
		struct no_shader_program_active : std::logic_error { no_shader_program_active() : std::logic_error("neogfx::i_rendering_engine::no_shader_program_active") {} };
		struct shader_program_not_found : std::logic_error { shader_program_not_found() : std::logic_error("neogfx::i_rendering_engine::shader_program_not_found") {} };
		struct shader_program_error : std::runtime_error { shader_program_error(const std::string& aError) : std::runtime_error("neogfx::i_rendering_engine::shader_program_error: " + aError) {} };
	public:
		virtual const i_device_metrics& default_screen_metrics() const = 0;
	public:
		virtual neogfx::renderer renderer() const = 0;
		virtual bool double_buffering() const = 0;
		virtual void initialize() = 0;
		virtual const i_render_target* active_target() const = 0;
		virtual void activate_context(const i_render_target& aTarget) = 0;
		virtual void deactivate_context() = 0;
		virtual opengl_context create_context(const i_render_target& aTarget) = 0;
		virtual void destroy_context(opengl_context aContext) = 0;
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
		virtual bool shader_program_active() const = 0;
		virtual void activate_shader_program(i_graphics_context& aGraphicsContext, i_shader_program& aProgram, const optional_mat44& aProjectionMatrix = optional_mat44{}, const optional_mat44& aTransformationMatrix = optional_mat44{}) = 0;
		virtual void deactivate_shader_program() = 0;
	public:
		virtual const i_shader_program& active_shader_program() const = 0;
		virtual i_shader_program& active_shader_program() = 0;
		virtual const i_shader_program& default_shader_program() const = 0;
		virtual i_shader_program& default_shader_program() = 0;
		virtual const i_shader_program& mesh_shader_program() const = 0;
		virtual i_shader_program& mesh_shader_program() = 0;
		virtual const i_shader_program& glyph_shader_program(bool aSubpixel) const = 0;
		virtual i_shader_program& glyph_shader_program(bool aSubpixel) = 0;
		virtual const i_shader_program& gradient_shader_program() const = 0;
		virtual i_shader_program& gradient_shader_program() = 0;
	public:
		virtual const opengl_standard_vertex_arrays& vertex_arrays() const = 0;
		virtual opengl_standard_vertex_arrays& vertex_arrays() = 0;
	public:
		virtual i_texture& ping_pong_buffer1(const size& aExtents, texture_sampling aSampling = texture_sampling::Multisample) = 0;
		virtual i_texture& ping_pong_buffer2(const size& aExtents, texture_sampling aSampling = texture_sampling::Multisample) = 0;
	public:
		virtual bool is_subpixel_rendering_on() const = 0;
		virtual void subpixel_rendering_on() = 0;
		virtual void subpixel_rendering_off() = 0;
	public:
		virtual neogfx::gradient_arrays& gradient_arrays() = 0;
	public:
		virtual void render_now() = 0;
		virtual bool use_rendering_priority() const = 0;
	public:
		virtual bool process_events() = 0;
	public:
		virtual void want_game_mode() = 0;
		virtual bool game_mode() const = 0;
	public:
		virtual void register_frame_counter(i_widget& aWidget, uint32_t aDuration) = 0;
		virtual void unregister_frame_counter(i_widget& aWidget, uint32_t aDuration) = 0;
		virtual uint32_t frame_counter(uint32_t aDuration) const = 0;
	};
}