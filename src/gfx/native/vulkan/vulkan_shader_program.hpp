// vulkan_shader_program.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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
#include "../native_buffer.hpp"
#include "vulkan_graphics_backend.hpp"
#include "vulkan_glsl.hpp"

namespace neogfx
{
    // n.b. as opengl_ssbo (the buffer is the shared native_buffer, whose GPU buffer the backend creates); drawing binds
    // the buffer it has at the time (see basic_vulkan_shader_program::link)
    template <typename T>
    class vulkan_ssbo : public ssbo<T>, public native_buffer<ssbo_element_t<T>>, private native_buffer_owner
    {
    public:
        using typename ssbo<T>::value_type;
        using typename ssbo<T>::size_type;
        using buffer_type = native_buffer<ssbo_element_t<T>>;
    public:
        vulkan_ssbo(i_string const& aName, ssbo_id aId, size_type aCapacity = 0u);
        ~vulkan_ssbo();
    public:
        ssbo_range alloc(size_type aSize) final;
        void free(ssbo_range aRange) final;
        void* lock(ssbo_range aRange) final;
        void unlock(ssbo_range aRange) final;
        void reclaim() final;
        void flush() final;
    private:
        void buffer_grown() final;
    private:
        std::uint32_t iLockCount = 0u;
    };

    template <typename Base = shader_program<>>
    class basic_vulkan_shader_program : public Base
    {
        using base_type = Base;
    public:
        basic_vulkan_shader_program(std::string const& aName);
        ~basic_vulkan_shader_program();
    public:
        void compile() final;
        void link() final;
        void use() final;
        void update_uniform_storage() final;
        void update_uniform_locations() final;
        void update_uniforms(const i_rendering_context& aContext) final;
        std::size_t ssbo_count() const final;
        i_ssbo const& ssbo(std::size_t aIndex) const final;
        i_ssbo& ssbo(std::size_t aIndex) final;
        void create_ssbo(i_string const& aName, shader_data_type aDataType, i_ref_ptr<i_ssbo>& aSsbo) final;
        void deactivate() final;
    private:
        vulkan_graphics_backend& backend() const;
        vulkan_program& vk_program() const;
        template <typename T>
        void add_ssbo(i_string const& aName, ssbo_id aId, i_ref_ptr<i_ssbo>& aSsbo);
    private:
        std::array<vulkan_uniform_block, static_cast<std::size_t>(shader_type::COUNT)> iUniformBlocks;
        std::array<spirv_block, static_cast<std::size_t>(shader_type::COUNT)> iUniformBlockLayouts;
        std::vector<std::uint8_t> iScratch;
        vulkan_glsl_samplers iSamplers;
        neolib::std_vector_jar<weak_ref_ptr<i_ssbo>> iSsbos;
        std::vector<std::pair<ssbo_id, std::function<gpu_buffer()>>> iSsboBuffers;
    };

    using vulkan_shader_program = basic_vulkan_shader_program<>;

    class vulkan_standard_shader_program : public basic_vulkan_shader_program<standard_shader_program>
    {
        using base_type = basic_vulkan_shader_program<standard_shader_program>;
    public:
        vulkan_standard_shader_program() :
            base_type{ "standard_shader_program" }
        {
        }
    };
}
