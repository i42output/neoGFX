// opengl_buffer.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <vector>
#include <bit>

#include <neogfx/gfx/color.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/i_rendering_context.hpp>
#include <neogfx/gfx/i_shader_program.hpp>
#include <neogfx/gfx/vertex_buffer.hpp>
#include "opengl.hpp"

namespace neogfx
{
    class opengl_buffer_owner
    {
    public:
        virtual void buffer_grown() = 0;
    };

    template <typename T>
    class opengl_buffer
    {
    private:
        static constexpr std::size_t kRingBufferSize = 3u;
    public:
        using value_type = T;
        using const_reference = value_type const&;
        using reference = value_type&;
        using const_pointer = value_type const*;
        using pointer = value_type*;
        using const_iterator = const_pointer;
        using iterator = pointer;
        using size_type = std::size_t;
    public:
        struct no_owner : std::logic_error { no_owner() : std::logic_error{ "neogfx::opengl_buffer::no_owner" } {} };
    private:
        using free_block = std::pair<size_type, size_type>;
        using free_blocks = std::vector<free_block>;
    public:
        opengl_buffer(bool aCacheable, size_type aCapacity);
        opengl_buffer(opengl_buffer_owner& aOwner, bool aCacheable, size_type aCapacity = 0u);
        ~opengl_buffer();
    public:
        size_type capacity() const;
        bool empty() const;
        size_type size() const;
        const_iterator cbegin() const;
        const_iterator cend() const;
        const_iterator begin() const;
        const_iterator end() const;
        iterator begin();
        iterator end();
    public:
        void reserve(size_type aCapacity);
        void resize(size_type aSize);
    public:
        const_reference operator[](size_type aOffset) const;
        reference operator[](size_type aOffset);
        const_reference back() const;
        reference back();
        size_type find_space_for(size_type aCount);
        void push_back(const_reference aValue);
        template <typename... Args>
        void emplace_back(Args&&... aArgs)
        {
            need(1);
            new (map() + iSize) value_type{ std::forward<Args>(aArgs)... };
            ++iSize;
        }
        void pop_back();
        void clear();
    public:
        GLuint handle() const;
        bool mapped() const;
        const_pointer map() const;
        pointer map();
        void flush(size_type aOffset, size_type aElements);
        void unmap();
    public:
        size_type room() const;
        bool room_for(size_type aExtra) const;
        void need(size_type aExtra);
    public:
        void reclaim(size_type aStartIndex, size_type aEndIndex);
        void reclaim();
    private:
        std::array<free_blocks, 32u>& blocks_to_free();
        std::optional<std::pair<free_blocks const*, free_blocks::const_iterator>> find_free_block(size_type aCount) const;
        std::optional<std::pair<free_blocks*, free_blocks::iterator>> find_free_block(size_type aCount);
        void grow(size_type aCapacity);
    private:
        GLuint iBufferName = 0;
        size_type iCapacity = 0;
        size_type iSize = 0;
        mutable pointer iMemory = nullptr;
        opengl_buffer_owner* iOwner = nullptr;
        bool iCacheable;
        std::array<std::array<std::array<free_blocks, 32u>, kRingBufferSize>, static_cast<std::size_t>(render_target_type::COUNT)> iBlocksToFree;
        std::array<free_blocks, 32u> iFreeBlocks;
    };
}
