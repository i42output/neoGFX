// i_vertex_buffer.hpp
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

#include <neogfx/gfx/i_vertex_provider.hpp>

namespace neogfx
{
    class i_rendering_context;
    class i_shader_program;

    // todo
    enum class vertex_buffer_type : std::uint32_t
    {
        Invalid     = 0x00000000,
        Vertices    = 0x00000001,
        UV          = 0x00000002,
        Color       = 0x00000004,
        Function0   = 0x00000008,
        Function1   = 0x00000010,
        Function2   = 0x00000020,
        Function3   = 0x00000040,
        Function4   = 0x00000080,
        // todo
        Custom1     = 0x00010000,
        Custom2     = 0x00020000,
        Custom3     = 0x00040000,
        Custom4     = 0x00080000,
        Persist     = 0x10000000,
        Default     = Vertices | UV | Color | Function0 | Function1 | Function2 | Function3 | Function4,
        DefaultECS  = Vertices | UV | Color | Function0 | Function1 | Function2 | Function3 | Function4 | Persist
    };

    inline std::string const& standard_vertex_attribute_name(vertex_buffer_type aType)
    {
        switch (aType)
        {
        case vertex_buffer_type::Vertices:
        {
            static const std::string sName = "VertexPosition";
            return sName;
        }
        case vertex_buffer_type::Color:
        {
            static const std::string sName = "VertexColor";
            return sName;
        }
        case vertex_buffer_type::UV:
        {
            static const std::string sName = "VertexTextureCoord";
            return sName;
        }
        case vertex_buffer_type::Function0:
        {
            static const std::string sName = "VertexFunction0";
            return sName;
        }
        case vertex_buffer_type::Function1:
        {
            static const std::string sName = "VertexFunction1";
            return sName;
        }
        case vertex_buffer_type::Function2:
        {
            static const std::string sName = "VertexFunction2";
            return sName;
        }
        case vertex_buffer_type::Function3:
        {
            static const std::string sName = "VertexFunction3";
            return sName;
        }
        case vertex_buffer_type::Function4:
        {
            static const std::string sName = "VertexFunction4";
            return sName;
        }
        default:
            throw std::logic_error("neogfx::standard_vertex_attribute_name");
        }
    }

    inline constexpr vertex_buffer_type operator|(vertex_buffer_type aLhs, vertex_buffer_type aRhs)
    {
        return static_cast<vertex_buffer_type>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr vertex_buffer_type operator&(vertex_buffer_type aLhs, vertex_buffer_type aRhs)
    {
        return static_cast<vertex_buffer_type>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    // todo
    class i_vertex_buffer
    {
    public:
        struct shader_not_attached : std::logic_error { shader_not_attached() : std::logic_error{ "neogfx::i_vertex_buffer::shader_not_attached" } {} };
    public:
        virtual ~i_vertex_buffer() = default;
    public:
        virtual i_vertex_provider& vertex_provider() const = 0;
        virtual vertex_buffer_type buffer_type() const = 0;
    public:
        virtual i_shader_program& attached_shader() const = 0;
        virtual void attach_shader(i_rendering_context& aContext, i_shader_program& aShaderProgram) = 0;
        virtual void detach_shader() = 0;
    public:
        virtual void reclaim(std::size_t aStartIndex, std::size_t aEndIndex) = 0;
    };
}