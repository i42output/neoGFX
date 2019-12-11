// shader_program.cpp
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

#include <neogfx/gfx/shader_program.hpp>

namespace neogfx
{
    shader_program::shader_program()
    {
    }

    const shader_program::shader_list& shader_program::shaders() const
    {
        return iShaders;
    }

    const i_shader& shader_program::shader(const neolib::i_string& aName) const
    {
        auto s = iShaderIndex.find(aName);
        if (s != iShaderIndex.end())
            return *s->second();
        throw shader_not_found();
    }

    i_shader& shader_program::shader(const neolib::i_string& aName)
    {
        return const_cast<i_shader&>(to_const(*this).shader(aName));
    }

    const i_vertex_shader& shader_program::vertex_shader() const
    {
        for (auto& s : shaders())
            if (s->type() == shader_type::Vertex)
                return static_cast<const i_vertex_shader&>(*s);
        throw no_vertex_shader();
    }

    i_vertex_shader& shader_program::vertex_shader()
    {
        return const_cast<i_vertex_shader&>(to_const(*this).vertex_shader());
    }

    i_shader_program& shader_program::add_shader(neolib::i_ref_ptr<i_shader>& aShader)
    {
        iShaders.push_back(aShader);
        iShaderIndex.insert(aShader->name(), aShader);
        set_dirty();
        return *this;
    }

    bool shader_program::dirty() const
    {
        for (auto& s : shaders())
            if (s->dirty())
                return true;
    }

    void shader_program::compile()
    {
    }

    void shader_program::link()
    {
    }

    void shader_program::use()
    {
        if (dirty())
        {
            compile();
            link();
        }
    }
}