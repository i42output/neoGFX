// shader_program.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2019 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/vector.hpp>
#include <neolib/map.hpp>
#include <neolib/reference_counted.hpp>
#include <neogfx/gfx/i_shader_program.hpp>

namespace neogfx
{
    class shader_program : public neolib::reference_counted<i_shader_program>
    {
    public:
        typedef neolib::vector<neolib::ref_ptr<i_shader>> shaders_t;
        typedef neolib::map<shader_type, shaders_t> stages_t;
    private:
        typedef neolib::map<neolib::string, neolib::ref_ptr<i_shader>> shader_index;
    public:
        shader_program(const std::string aName);
        ~shader_program();
    public:
        const i_string& name() const override;
        bool created() const override;
        void* handle() const override;
        const stages_t& stages() const override;
        const i_shader& shader(const neolib::i_string& aName) const override;
        i_shader& shader(const neolib::i_string& aName) override;
        const i_vertex_shader& vertex_shader() const override;
        i_vertex_shader& vertex_shader() override;
        bool is_first_in_stage(const i_shader& aShader) const override;
        bool is_last_in_stage(const i_shader& aShader) const override;
        const i_shader& first_in_stage(shader_type aStage) const override;
        const i_shader& next_in_stage(const i_shader& aPreviousShader) const override;
        const i_shader& last_in_stage(shader_type aStage) const override;
        i_shader& add_shader(const neolib::i_ref_ptr<i_shader>& aShader) override;
        bool dirty() const override;
        void set_clean() override;
        void prepare_uniforms(const i_rendering_context& aRenderingContext) override;
        void activate(const i_rendering_context& aRenderingContext) override;
    private:
        string iName;
        mutable std::optional<void*> iHandle;
        stages_t iStages;
        shader_index iShaderIndex;
    };
}