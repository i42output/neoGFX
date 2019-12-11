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
#include <neogfx/gfx/i_shader_program.hpp>

namespace neogfx
{
    class shader_program : public i_shader_program
    {
    public:
        typedef neolib::vector<neolib::ref_ptr<i_shader>> shader_list;
    private:
        typedef neolib::map<neolib::string, neolib::ref_ptr<i_shader>> shader_index;
    public:
        shader_program();
    public:
        const shader_list& shaders() const override;
        const i_shader& shader(const neolib::i_string& aName) const override;
        i_shader& shader(const neolib::i_string& aName) override;
        const i_vertex_shader& vertex_shader() const override;
        i_vertex_shader& vertex_shader() override;
        i_shader_program& add_shader(neolib::i_ref_ptr<i_shader>& aShader) = 0;
        void compile() override;
        void link() override;
        void use() override;
    private:
        shader_list iShaders;
        shader_index iShaderIndex;
    };
}