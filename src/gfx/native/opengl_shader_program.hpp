// opengl_shader_program.hpp
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
#include <neogfx/gfx/shader_program.hpp>
#include <neogfx/gfx/standard_shader_program.hpp>
#include "opengl.hpp"

namespace neogfx
{
    // todo: this should adapt a generic (non-rendering API specific) shader program object
    class opengl_shader_program : public standard_shader_program
    {
    public:
        opengl_shader_program(const std::string& aName = "standard_shader_program");
    public:
        void compile() override;
        void link() override;
        void use() override;
        void update_uniform_locations() override;
        void update_uniforms(const i_rendering_context& aContext) override;
        bool active() const override;
        void deactivate() override;
    private:
        GLuint gl_handle() const;
    };
}