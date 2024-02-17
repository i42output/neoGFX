// vertex_buffer.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gfx/i_vertex_buffer.hpp>
#include <neogfx/gfx/i_shader_program.hpp>

namespace neogfx
{
    // todo
    class vertex_buffer : public i_vertex_buffer
    {
    public:
        vertex_buffer(i_vertex_provider& aProvider, vertex_buffer_type aType) :
            iProvider{ aProvider }, iType { aType }, iAttachedShader{ nullptr }
        {
        }
    public:
        i_vertex_provider& vertex_provider() const override
        {
            return iProvider;
        }
        vertex_buffer_type buffer_type() const override
        {
            return iType;
        }
    public:
        i_shader_program& attached_shader() const override
        {
            if (iAttachedShader)
                return *iAttachedShader;
            throw shader_not_attached();
        }
        void attach_shader(i_rendering_context& aContext, i_shader_program& aShaderProgram) override
        {
            aShaderProgram.instantiate(aContext);
            iAttachedShader = &aShaderProgram;
        }
        void detach_shader() override
        {
            iAttachedShader = nullptr;
        }
    private:
        i_vertex_provider& iProvider;
        vertex_buffer_type iType;
        i_shader_program* iAttachedShader;
    };
}