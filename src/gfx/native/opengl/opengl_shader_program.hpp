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
        using typename base_type::not_mapped;
        using typename base_type::value_type;
    public:
        opengl_ssbo(i_string const& aName, ssbo_id aId, i_shader_uniform& aSizeUniform, bool aPersistent = false);
        ~opengl_ssbo();
    public:
        void reserve(std::size_t aCapacity) final;
    public:
        void const* data() const final;
        void const* cdata() const final;
        void* data() final;
        bool mapped() const final;
        void map() const final;
        void unmap() const final;
        void sync() const final;
    private:
        bool iPersistent;
        GLuint iHandle = {};
        mutable value_type* iMapped = nullptr;
        mutable std::uint32_t iMappedCount = 0u;
    };

    template <typename Base = shader_program<>>
    class basic_opengl_shader_program : public Base
    {
        using base_type = Base;
    private:
        typedef std::vector<GLubyte> ubo_block_buffer_t;
    public:
        basic_opengl_shader_program(std::string const& aName);
        ~basic_opengl_shader_program();
    public:
        void compile() final;
        void link() final;
        void use() final;
        void update_uniform_storage() final;
        void update_uniform_locations() final;
        void update_uniforms(const i_rendering_context& aContext) final;
        std::size_t ssbo_count() const final;
        i_ssbo const& ssbo(std::size_t aIndex) const final;
        void create_ssbo(i_string const& aName, shader_data_type aDataType, i_shader_uniform& aSizeUniform, i_ref_ptr<i_ssbo>& aSsbo) final;
        void deactivate() final;
    private:
        GLuint gl_handle() const;
        ubo_block_buffer_t& ubo_block_buffer(shader_type aShaderType);
        GLuint ubo_handle(shader_type aShaderType) const;
        struct ubo
        {
            ubo_block_buffer_t uniformBlockBuffer;
            mutable std::optional<GLuint> uboHandle;
        } iUbos[static_cast<std::size_t>(shader_type::COUNT)];
        neolib::std_vector_jar<weak_ref_ptr<i_ssbo>> iSsbos;
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