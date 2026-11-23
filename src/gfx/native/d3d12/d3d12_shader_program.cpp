// d3d12_shader_program.cpp
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

#include <neogfx/neogfx.hpp>

#include "d3d12_shader_program.ipp"

namespace neogfx
{
    template <typename T>
    d3d12_ssbo<T>::d3d12_ssbo(i_string const& aName, ssbo_id aId, size_type aCapacity) :
        ssbo<T>{ aName, aId }, buffer_type{ *this, true, static_cast<typename buffer_type::size_type>(aCapacity) }
    {
    }

    template <typename T>
    d3d12_ssbo<T>::~d3d12_ssbo()
    {
    }

    template <typename T>
    ssbo_range d3d12_ssbo<T>::alloc(size_type aSize)
    {
        auto const maybeSpace = buffer_type::find_space_for(aSize);

        if (maybeSpace != buffer_type::size())
            return { static_cast<size_type>(maybeSpace), static_cast<size_type>(maybeSpace) + aSize };

        if (iLockCount != 0u && aSize > buffer_type::room())
            throw ssbo_locked{};

        buffer_type::resize(buffer_type::size() + aSize);

        return { static_cast<size_type>(buffer_type::size()) - aSize, static_cast<size_type>(buffer_type::size()) };
    }

    template <typename T>
    void d3d12_ssbo<T>::free(ssbo_range aRange)
    {
        buffer_type::reclaim(aRange.first, aRange.last);
    }

    template <typename T>
    void* d3d12_ssbo<T>::lock(ssbo_range aRange)
    {
        if (!buffer_type::mapped())
            throw std::logic_error("neogfx::d3d12_ssbo<T>::lock: not mapped");
        ++iLockCount;
        return buffer_type::map() + aRange.first;
    }

    template <typename T>
    void d3d12_ssbo<T>::unlock(ssbo_range aRange)
    {
        buffer_type::flush(aRange.first, aRange.last - aRange.first);
        --iLockCount;
    }

    template <typename T>
    void d3d12_ssbo<T>::reclaim()
    {
        buffer_type::reclaim();
    }

    template <typename T>
    void d3d12_ssbo<T>::flush()
    {
        buffer_type::flush(0u, buffer_type::size());
    }

    template <typename T>
    void d3d12_ssbo<T>::buffer_grown()
    {
        // nothing to do: drawing binds the buffer the SSBO has at the time (see d3d12_graphics_backend::prepare_draw)
    }

    template class d3d12_ssbo<bool>;
    template class d3d12_ssbo<float>;
    template class d3d12_ssbo<double>;
    template class d3d12_ssbo<std::int32_t>;
    template class d3d12_ssbo<std::uint32_t>;
    template class d3d12_ssbo<vec2f>;
    template class d3d12_ssbo<vec2>;
    template class d3d12_ssbo<vec2i32>;
    template class d3d12_ssbo<vec2u32>;
    template class d3d12_ssbo<vec3f>;
    template class d3d12_ssbo<vec3>;
    template class d3d12_ssbo<vec3i32>;
    template class d3d12_ssbo<vec3u32>;
    template class d3d12_ssbo<vec4f>;
    template class d3d12_ssbo<vec4>;
    template class d3d12_ssbo<vec4i32>;
    template class d3d12_ssbo<vec4u32>;
    template class d3d12_ssbo<mat4f>;
    template class d3d12_ssbo<mat4>;

    template class basic_d3d12_shader_program<>;
    template class basic_d3d12_shader_program<standard_shader_program>;
}
