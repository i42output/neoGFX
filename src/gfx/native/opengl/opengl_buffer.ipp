// opengl_buffer.ipp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015-2026 Leigh Johnston.  All Rights Reserved.
  
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

#include "opengl_buffer.hpp"

namespace neogfx
{
    template <typename T>
    inline opengl_buffer<T>::opengl_buffer(bool aCacheable, size_type aCapacity)
        : iCacheable{ aCacheable }
    {
        if (aCapacity != 0)
        {
            glCheck(glCreateBuffers(1, &iBufferName));
            glCheck(glNamedBufferStorage(iBufferName, aCapacity * sizeof(value_type), nullptr, 
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT));

            iCapacity = aCapacity;

            map();
        }
    }

    template <typename T>
    inline opengl_buffer<T>::opengl_buffer(opengl_buffer_owner& aOwner, bool aCacheable, size_type aCapacity) :
        opengl_buffer{ aCacheable, aCapacity }
    {
        iOwner = &aOwner;
    }

    template <typename T>
    inline opengl_buffer<T>::~opengl_buffer()
    {
        glCheck(glDeleteBuffers(1, &iBufferName));
    }

    template <typename T>
    inline typename opengl_buffer<T>::size_type opengl_buffer<T>::capacity() const
    {
        return iCapacity;
    }

    template <typename T>
    inline bool opengl_buffer<T>::empty() const
    {
        return iSize == 0;
    }

    template <typename T>
    inline typename opengl_buffer<T>::size_type opengl_buffer<T>::size() const
    {
        return iSize;
    }

    template <typename T>
    inline typename opengl_buffer<T>::const_iterator opengl_buffer<T>::cbegin() const
    {
        return map();
    }

    template <typename T>
    inline typename opengl_buffer<T>::const_iterator opengl_buffer<T>::cend() const
    {
        return map() + size();
    }

    template <typename T>
    inline typename opengl_buffer<T>::const_iterator opengl_buffer<T>::begin() const
    {
        return cbegin();
    }

    template <typename T>
    inline typename opengl_buffer<T>::const_iterator opengl_buffer<T>::end() const
    {
        return cend();
    }
    
    template <typename T>
    inline typename opengl_buffer<T>::iterator opengl_buffer<T>::begin()
    {
        return map();
    }

    template <typename T>
    inline typename opengl_buffer<T>::iterator opengl_buffer<T>::end()
    {
        return map() + size();
    }

    template <typename T>
    inline void opengl_buffer<T>::reserve(size_type aCapacity)
    {
        if (aCapacity > capacity())
            grow(aCapacity);
    }

    template <typename T>
    void opengl_buffer<T>::resize(size_type aSize)
    {
        if (aSize > size())
            need(aSize - size());
        iSize = aSize;
    }

    template <typename T>
    inline typename opengl_buffer<T>::const_reference opengl_buffer<T>::operator[](size_type aOffset) const
    {
        return *std::next(cbegin(), aOffset);
    }

    template <typename T>
    inline typename opengl_buffer<T>::reference opengl_buffer<T>::operator[](size_type aOffset)
    {
        return *std::next(begin(), aOffset);
    }

    template <typename T>
    inline typename opengl_buffer<T>::const_reference opengl_buffer<T>::back() const
    {
        return *std::prev(cend());
    }

    template <typename T>
    inline typename opengl_buffer<T>::reference opengl_buffer<T>::back()
    {
        return *std::prev(end());
    }

    template <typename T>
    inline typename opengl_buffer<T>::size_type opengl_buffer<T>::find_space_for(size_type aCount)
    {
        auto maybeFreeBlock = find_free_block(aCount);

        if (!maybeFreeBlock)
            return size();

        auto const freeBlock = *maybeFreeBlock->second;
        std::swap(*maybeFreeBlock->second, maybeFreeBlock->first->back());
        maybeFreeBlock->first->pop_back();

        auto result = freeBlock.first;

        auto leftover = (freeBlock.second - freeBlock.first) - aCount;
        if (leftover > 0)
            iFreeBlocks[std::countr_zero(std::bit_ceil(leftover))].emplace_back(freeBlock.first + aCount, freeBlock.first + aCount + leftover);

        return result;
    }

    template <typename T>
    inline void opengl_buffer<T>::push_back(const_reference aValue)
    {
        need(1);
        new (map() + iSize) value_type{ aValue };
        ++iSize;
    }

    template <typename T>
    inline void opengl_buffer<T>::pop_back()
    {
        --iSize;
    }

    template <typename T>
    inline void opengl_buffer<T>::clear()
    {
        iSize = 0;
        iBlocksToFree = {};
        iFreeBlocks = {};
    }

    template <typename T>
    inline GLuint opengl_buffer<T>::handle() const
    {
        return iBufferName;
    }

    template <typename T>
    inline bool opengl_buffer<T>::mapped() const
    {
        return iMemory != nullptr;
    }

    template <typename T>
    inline typename opengl_buffer<T>::const_pointer opengl_buffer<T>::map() const
    {
        if (iMemory == nullptr)
            glCheck(iMemory = static_cast<value_type*>(glMapNamedBufferRange(handle(), 0, capacity() * sizeof(value_type), 
                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT)));
        return iMemory;
    }

    template <typename T>
    inline typename opengl_buffer<T>::pointer opengl_buffer<T>::map()
    {
        return const_cast<pointer>(to_const(*this).map());
    }

    template <typename T>
    inline void opengl_buffer<T>::flush(size_type aOffset, size_type aElements)
    {
        if (mapped())
        {
            glCheck(glFlushMappedNamedBufferRange(handle(), aOffset * sizeof(value_type), aElements * sizeof(value_type)));
        }
        else
            throw std::logic_error("neogfx::opengl_buffer<T>::flush: buffer not mapped!");
    }

    template <typename T>
    inline void opengl_buffer<T>::unmap()
    {
        if (iMemory != nullptr)
        {
            flush(0, size());
            glCheck(glUnmapNamedBuffer(handle()));
            iMemory = nullptr;
        }
    }

    template <typename T>
    inline typename opengl_buffer<T>::size_type opengl_buffer<T>::room() const
    {
        return capacity() - size();
    }

    template <typename T>
    inline bool opengl_buffer<T>::room_for(size_type aExtra) const
    {
        if (aExtra <= room())
            return true;
        if (find_free_block(aExtra))
            return true;
        return false;
    }

    template <typename T>
    inline void opengl_buffer<T>::need(size_type aExtra)
    {
        if (aExtra > room())
            grow(std::max<size_type>(static_cast<size_type>((capacity() + aExtra) * 1.5), 16384u));
    }

    template <typename T>
    inline void opengl_buffer<T>::reclaim(size_type aStartIndex, size_type aEndIndex)
    {
        if (aEndIndex != aStartIndex)
            blocks_to_free()[std::countr_zero(std::bit_ceil(aEndIndex - aStartIndex))].emplace_back(aStartIndex, aEndIndex);
    }

    template <typename T>
    inline void opengl_buffer<T>::reclaim()
    {
        for (std::size_t bucket = 0u; bucket < iFreeBlocks.size(); ++bucket)
        {
            auto& dst = iFreeBlocks[bucket];
            auto& src = blocks_to_free()[bucket];
            dst.insert(dst.end(),
                std::make_move_iterator(src.begin()),
                std::make_move_iterator(src.end()));
            src.clear();
        }
    }

    template <typename T>
    inline std::array<typename opengl_buffer<T>::free_blocks, 32u>& opengl_buffer<T>::blocks_to_free()
    {
        if (!iCacheable)
            return iBlocksToFree[0u][0u];
        else
        {
            auto const activeTarget = service<i_rendering_engine>().active_target();
            auto const activeTargetType = activeTarget ? activeTarget->target_type() : render_target_type::Surface;
            auto const ringBufferIndex = service<i_rendering_engine>().target_activation_counter(activeTargetType) % kRingBufferSize;
            return iBlocksToFree[static_cast<std::size_t>(activeTargetType)][ringBufferIndex];
        }
    }

    template <typename T>
    inline auto opengl_buffer<T>::find_free_block(size_type aCount) const -> std::optional<std::pair<typename opengl_buffer<T>::free_blocks const*, typename opengl_buffer<T>::free_blocks::const_iterator>>
    {
        auto probe = std::bit_ceil(aCount);
        bool peek = std::countr_zero(probe * 2) < iFreeBlocks.size();
        bool peeked = false;
        while (std::countr_zero(probe) < iFreeBlocks.size())
        {
            auto& freeBlocksProbe = iFreeBlocks[std::countr_zero(peek ? probe * 2 : probe)];
            for (auto freeBlockProbe = freeBlocksProbe.begin(); freeBlockProbe != freeBlocksProbe.end(); ++freeBlockProbe)
                if (freeBlockProbe->second - freeBlockProbe->first >= aCount)
                    return std::make_pair(&freeBlocksProbe, freeBlockProbe);
            if (peek)
            {
                peek = false;
                peeked = true;
            }
            else if (peeked)
            {
                peeked = false;
                probe *= 4;
            }
            else
                probe *= 2;
        }
        return {};
    }

    template <typename T>
    inline auto opengl_buffer<T>::find_free_block(size_type aCount) -> std::optional<std::pair<typename opengl_buffer<T>::free_blocks*, typename opengl_buffer<T>::free_blocks::iterator>>
    {
        auto const result = const_cast<opengl_buffer const&>(*this).find_free_block(aCount);
        if (!result)
            return {};
        auto freeBlocks = const_cast<free_blocks*>(result->first);
        return std::make_pair(freeBlocks, std::next(freeBlocks->begin(), std::distance(freeBlocks->cbegin(), result->second)));
    }

    template <typename T>   
    inline void opengl_buffer<T>::grow(size_type aCapacity)
    {
        unmap();

        {
            opengl_buffer<T> temp{ iCacheable, aCapacity };
            if (!empty())
                glCheck(glCopyNamedBufferSubData(iBufferName, temp.iBufferName,
                    0, 0, size() * sizeof(value_type)));
            std::swap(iBufferName, temp.iBufferName);
            std::swap(iCapacity, temp.iCapacity);
            std::swap(iMemory, temp.iMemory);
        }

        if (iOwner)
            iOwner->buffer_grown();
        else
            throw no_owner{};
    }
}
