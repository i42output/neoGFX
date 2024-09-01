// opengl_shader_program.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2019, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gfx/shader_program.hpp>
#include <neogfx/gfx/standard_shader_program.hpp>
#include "opengl.hpp"

namespace neogfx
{
    template <typename T>
    class opengl_ssbo : public ssbo<T>
    {
        using base_type = ssbo<T>;
    public:
        using typename base_type::value_type;
    public:
        opengl_ssbo(ssbo_id aId, i_shader_uniform& aSizeUniform);
        ~opengl_ssbo();
    public:
        void reserve(std::size_t aCapacity) final;
    private:
        GLuint iHandle = {};
    };

    template <typename Base = shader_program<>>
    class basic_opengl_shader_program : public Base
    {
        using base_type = Base;
    private:
        typedef std::vector<GLubyte> ubo_block_buffer_t;
    public:
        basic_opengl_shader_program(std::string const& aName);
    public:
        void compile() override;
        void link() override;
        void use() override;
        void update_uniform_storage() override;
        void update_uniform_locations() override;
        void update_uniforms(const i_rendering_context& aContext) override;
        i_ssbo& create_ssbo(shader_data_type aDataType, i_shader_uniform& aSizeUniform) override;
        void destroy_ssbo(i_ssbo& aSsbo) override;
        void deactivate() override;
    private:
        GLuint gl_handle() const;
        ubo_block_buffer_t& ubo_block_buffer(shader_type aShaderType);
        GLuint ubo_handle(shader_type aShaderType) const;
        struct ubo
        {
            ubo_block_buffer_t uniformBlockBuffer;
            mutable std::optional<GLuint> uboHandle;
        } iUbos[static_cast<std::size_t>(shader_type::COUNT)];
        neolib::std_vector_jar<std::unique_ptr<i_ssbo>> iSsbos;
    };

    using opengl_shader_program = basic_opengl_shader_program<>;

    class opengl_standard_shader_program : public basic_opengl_shader_program<standard_shader_program>
    {
        using base_type = basic_opengl_shader_program<standard_shader_program>;
    public:
        opengl_standard_shader_program() :
            base_type{ "standard_shader_program" }
        {
        }
    };
}