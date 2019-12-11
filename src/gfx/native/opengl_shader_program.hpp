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
#include "opengl.hpp"

namespace neogfx
{
    class opengl_shader_program : public shader_program
    {
    public:
        opengl_shader_program();
    public:
        bool dirty() const override;
        void set_dirty() override;
        void compile() override;
        void link() override;
        void use() override;
    private:
        std::optional<std::string> iCode;
        std::optional<GLuint> iHandle;
    };
}