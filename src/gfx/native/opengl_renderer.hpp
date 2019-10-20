// opengl_renderer.hpp
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
#include <set>
#include <map>
#include "opengl.hpp"
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/text/font_manager.hpp>
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
    public:
        define_declared_event(SubpixelRenderingChanged, subpixel_rendering_changed)
    public:
        struct shader_program_error : i_rendering_engine::shader_program_error {
            shader_program_error(GLenum aErrorCode) :
                i_rendering_engine::shader_program_error(glErrorString(aErrorCode)) {}
        };
        struct failed_to_create_framebuffer : std::runtime_error {
            failed_to_create_framebuffer(GLenum aErrorCode) :
                std::runtime_error("neogfx::opengl_renderer::failed_to_create_framebuffer: Failed to create frame buffer, reason: " + glErrorString(aErrorCode)) {}
        };
    public:
        class shader_program : public i_shader_program
        {
        public:
            typedef std::map<std::string, GLuint> variable_map;
        public:
            shader_program(GLuint aHandle, bool aHasProjectionMatrix, bool aHasTransformationMatrix);
        public:
            void* handle() const override;
            bool has_projection_matrix() const override;
            const optional_mat44& projection_matrix() const override;
            void set_projection_matrix(const i_rendering_context& aGraphicsContext, const optional_mat44& aProjectionMatrix = optional_mat44{}) override;
            bool has_transformation_matrix() const override;
            const optional_mat44& transformation_matrix() const override;
            void set_transformation_matrix(const i_rendering_context& aGraphicsContext, const optional_mat44& aTransformationMatrix = optional_mat44{}) override;
            void* variable(const std::string& aVariableName) const override;
            void set_uniform_variable(const std::string& aName, float aValue) override;
            void set_uniform_variable(const std::string& aName, double aValue) override;
            void set_uniform_variable(const std::string& aName, int aValue) override;
            void set_uniform_variable(const std::string& aName, float aValue1, float aValue2) override;
            void set_uniform_variable(const std::string& aName, double aValue1, double aValue2) override;
            void set_uniform_variable(const std::string& aName, const vec4f& aVector) override;
            void set_uniform_variable(const std::string& aName, const vec4& aVector) override;
            void set_uniform_array(const std::string& aName, uint32_t aSize, const float* aArray) override;
            void set_uniform_array(const std::string& aName, uint32_t aSize, const double* aArray) override;
            void set_uniform_matrix(const std::string& aName, const mat44::template rebind<float>::type& aMatrix) override;
            void set_uniform_matrix(const std::string& aName, const mat44::template rebind<double>::type& aMatrix) override;
        public:
            GLuint register_variable(const std::string& aVariableName);
        public:
            bool operator<(const shader_program& aRhs) const;
        private:
            GLint uniform_location(const std::string& aName);
        private:
            GLuint iHandle;
            bool iHasProjectionMatrix;
            bool iHasTransformationMatrix;
            neogfx::logical_coordinates iLogicalCoordinates;
            optional_mat44 iProjectionMatrix;
            optional_mat44 iTransformationMatrix;
            variable_map iVariables;
        };
    private:
        typedef std::vector<std::pair<std::string, GLenum>> shaders;
        typedef std::list<shader_program> shader_programs;
        typedef std::list<neogfx::gradient_shader_data> gradient_data_cache_t;
        typedef std::map<gradient, gradient_data_cache_t::iterator> gradient_data_cache_map_t;
        typedef std::deque<gradient_data_cache_map_t::iterator> gradient_data_cache_queue_t;
        typedef std::map<std::pair<texture_sampling, size>, texture> ping_pong_buffers_t;
    private:
        static constexpr std::size_t GRADIENT_DATA_CACHE_QUEUE_SIZE = 64;
    public:
        opengl_renderer(neogfx::renderer aRenderer);
        ~opengl_renderer();
    public:
        const i_device_metrics& default_screen_metrics() const override;
    public:
        neogfx::renderer renderer() const override;
        void initialize() override;
        void cleanup() override;
        i_font_manager& font_manager() override;
        i_texture_manager& texture_manager() override;
        bool shader_program_active() const override;
        void activate_shader_program(i_rendering_context& aGraphicsContext, i_shader_program& aProgram, const optional_mat44& aProjectionMatrix = optional_mat44{}, const optional_mat44& aTransformationMatrix = optional_mat44{}) override;
        void deactivate_shader_program() override;
    public:
        const i_shader_program& active_shader_program() const override;
        i_shader_program& active_shader_program() override;
        const i_shader_program& default_shader_program() const override;
        i_shader_program& default_shader_program() override;
        const i_shader_program& mesh_shader_program() const override;
        i_shader_program& mesh_shader_program() override;
        const i_shader_program& glyph_shader_program(bool aSubpixel) const override;
        i_shader_program& glyph_shader_program(bool aSubpixel) override;
        const i_shader_program& gradient_shader_program() const override;
        i_shader_program& gradient_shader_program() override;
    public:
        const opengl_standard_vertex_arrays & vertex_arrays() const override;
        opengl_standard_vertex_arrays& vertex_arrays() override;
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
        neogfx::gradient_shader_data& gradient_shader_data(const gradient& aGradient) override;
    public:
        bool process_events() override;
    public:
        void want_game_mode() override;
        bool game_mode() const override;
    public:
        void register_frame_counter(i_widget& aWidget, uint32_t aDuration) override;
        void unregister_frame_counter(i_widget& aWidget, uint32_t aDuration) override;
        uint32_t frame_counter(uint32_t aDuration) const override;
    private:
        shader_programs::iterator create_shader_program(const shaders& aShaders, const std::vector<std::string>& aVariables);
        i_texture& create_ping_pong_buffer(ping_pong_buffers_t& aBufferList, const size& aExtents, texture_sampling aSampling);
    private:
        neogfx::renderer iRenderer;
        mutable std::optional<opengl_texture_manager> iTextureManager;
        mutable std::optional<neogfx::font_manager> iFontManager;
        shader_programs iShaderPrograms;
        shader_programs::iterator iActiveProgram;
        shader_programs::iterator iDefaultProgram;
        shader_programs::iterator iMeshProgram;
        shader_programs::iterator iGlyphProgram;
        shader_programs::iterator iGlyphSubpixelProgram;
        shader_programs::iterator iGradientProgram;
        bool iLimitFrameRate;
        uint32_t iFrameRateLimit;
        bool iSubpixelRendering;
        std::vector<float> iGradientStopPositions;
        std::vector<std::array<float, 4>> iGradientStopColours;
        gradient_data_cache_t iGradientDataCache;
        gradient_data_cache_map_t iGradientDataCacheMap;
        gradient_data_cache_queue_t iGradientDataCacheQueue;
        std::optional<neogfx::gradient_shader_data> iUncachedGradient;
        mutable std::optional<opengl_standard_vertex_arrays> iVertexArrays;
        uint64_t iLastGameRenderTime;
        std::map<uint32_t, neogfx::frame_counter> iFrameCounters;
        ping_pong_buffers_t iPingPongBuffer1s;
        ping_pong_buffers_t iPingPongBuffer2s;
    };
}