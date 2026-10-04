// native_buffer.ipp
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

#include "native_buffer.hpp"

namespace neogfx
{
    template <typename T>
    inline native_buffer<T>::native_buffer(bool aCacheable, size_type aCapacity, bool aDeviceLocal)
        : iCacheable{ aCacheable }, iDeviceLocal{ aDeviceLocal }
    {
        if (aCapacity != 0)
        {
            iBufferName = graphics_backend().create_buffer(aCapacity * sizeof(value_type), iDeviceLocal);

            iCapacity = aCapacity;

            if (!iDeviceLocal)
                map();
        }
    }

    template <typename T>
    inline native_buffer<T>::native_buffer(native_buffer_owner& aOwner, bool aCacheable, size_type aCapacity, bool aDeviceLocal) :
        native_buffer{ aCacheable, aCapacity, aDeviceLocal }
    {
        iOwner = &aOwner;
    }

    template <typename T>
    inline native_buffer<T>::~native_buffer()
    {
        if (iBufferName != no_gpu_buffer)
            graphics_backend().destroy_buffer(iBufferName);
    }

    template <typename T>
    inline typename native_buffer<T>::size_type native_buffer<T>::capacity() const
    {
        return iCapacity;
    }

    template <typename T>
    inline bool native_buffer<T>::empty() const
    {
        return iSize == 0;
    }

    template <typename T>
    inline typename native_buffer<T>::size_type native_buffer<T>::size() const
    {
        return iSize;
    }

    template <typename T>
    inline typename native_buffer<T>::const_iterator native_buffer<T>::cbegin() const
    {
        return map();
    }

    template <typename T>
    inline typename native_buffer<T>::const_iterator native_buffer<T>::cend() const
    {
        return map() + size();
    }

    template <typename T>
    inline typename native_buffer<T>::const_iterator native_buffer<T>::begin() const
    {
        return cbegin();
    }

    template <typename T>
    inline typename native_buffer<T>::const_iterator native_buffer<T>::end() const
    {
        return cend();
    }
    
    template <typename T>
    inline typename native_buffer<T>::iterator native_buffer<T>::begin()
    {
        return map();
    }

    template <typename T>
    inline typename native_buffer<T>::iterator native_buffer<T>::end()
    {
        return map() + size();
    }

    template <typename T>
    inline void native_buffer<T>::reserve(size_type aCapacity)
    {
        if (aCapacity > capacity())
            grow(aCapacity);
    }

    template <typename T>
    void native_buffer<T>::resize(size_type aSize)
    {
        if (aSize > size())
            need(aSize - size());
        iSize = aSize;
    }

    template <typename T>
    inline typename native_buffer<T>::const_reference native_buffer<T>::operator[](size_type aOffset) const
    {
        return *std::next(cbegin(), aOffset);
    }

    template <typename T>
    inline typename native_buffer<T>::reference native_buffer<T>::operator[](size_type aOffset)
    {
        return *std::next(begin(), aOffset);
    }

    template <typename T>
    inline typename native_buffer<T>::const_reference native_buffer<T>::back() const
    {
        return *std::prev(cend());
    }

    template <typename T>
    inline typename native_buffer<T>::reference native_buffer<T>::back()
    {
        return *std::prev(end());
    }

    template <typename T>
    inline typename native_buffer<T>::size_type native_buffer<T>::find_space_for(size_type aCount)
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
    inline void native_buffer<T>::push_back(const_reference aValue)
    {
        need(1);
        new (map() + iSize) value_type{ aValue };
        ++iSize;
    }

    template <typename T>
    inline void native_buffer<T>::pop_back()
    {
        --iSize;
    }

    template <typename T>
    inline void native_buffer<T>::clear()
    {
        iSize = 0;
        iBlocksToFree = {};
        iFreeBlocks = {};
    }

    template <typename T>
    inline gpu_buffer native_buffer<T>::handle() const
    {
        return iBufferName;
    }

    template <typename T>
    inline bool native_buffer<T>::mapped() const
    {
        return iMemory != nullptr;
    }

    template <typename T>
    inline typename native_buffer<T>::const_pointer native_buffer<T>::map() const
    {
        if (iDeviceLocal)
            throw std::logic_error("neogfx::native_buffer<T>::map: device local buffer cannot be mapped");
        if (iMemory == nullptr)
            iMemory = static_cast<value_type*>(graphics_backend().map_buffer(handle(), capacity() * sizeof(value_type)));
        return iMemory;
    }

    template <typename T>
    inline typename native_buffer<T>::pointer native_buffer<T>::map()
    {
        return const_cast<pointer>(to_const(*this).map());
    }

    template <typename T>
    inline void native_buffer<T>::flush(size_type aOffset, size_type aElements)
    {
        if (iDeviceLocal || aElements == 0)
            return; // n.b. a device local buffer is written directly (see write()) and a buffer never written may not be mapped
        if (mapped())
        {
            graphics_backend().flush_buffer(handle(), aOffset * sizeof(value_type), aElements * sizeof(value_type));
        }
        else
            throw std::logic_error("neogfx::native_buffer<T>::flush: buffer not mapped!");
    }

    template <typename T>
    inline void native_buffer<T>::write(size_type aOffset, const_pointer aData, size_type aElements)
    {
        if (aElements == 0)
            return;
        if (aOffset + aElements > size())
            throw std::logic_error("neogfx::native_buffer<T>::write: out of range");
        if (iDeviceLocal)
            graphics_backend().write_buffer(handle(), aOffset * sizeof(value_type), aData, aElements * sizeof(value_type));
        else
        {
            std::copy(aData, aData + aElements, map() + aOffset);
            flush(aOffset, aElements);
        }
    }

    template <typename T>
    inline void native_buffer<T>::unmap()
    {
        if (iMemory != nullptr)
        {
            flush(0, size());
            graphics_backend().unmap_buffer(handle());
            iMemory = nullptr;
        }
    }

    template <typename T>
    inline typename native_buffer<T>::size_type native_buffer<T>::room() const
    {
        return capacity() - size();
    }

    template <typename T>
    inline bool native_buffer<T>::room_for(size_type aExtra) const
    {
        if (aExtra <= room())
            return true;
        if (find_free_block(aExtra))
            return true;
        return false;
    }

    template <typename T>
    inline void native_buffer<T>::need(size_type aExtra)
    {
        if (aExtra > room())
        {
            if (!iDeviceLocal)
                grow(std::max<size_type>(static_cast<size_type>((capacity() + aExtra) * 1.5), 16384u));
            else
                // no slack the first time (use reserve() to size a device local buffer up front)
                grow(std::max<size_type>(size() + aExtra, capacity() + capacity() / 2u));
        }
    }

    template <typename T>
    inline void native_buffer<T>::reclaim(size_type aStartIndex, size_type aEndIndex)
    {
        if (aEndIndex != aStartIndex)
            blocks_to_free()[std::countr_zero(std::bit_ceil(aEndIndex - aStartIndex))].emplace_back(aStartIndex, aEndIndex);
    }

    template <typename T>
    inline void native_buffer<T>::reclaim()
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
    inline std::array<typename native_buffer<T>::free_blocks, 32u>& native_buffer<T>::blocks_to_free()
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
    inline auto native_buffer<T>::find_free_block(size_type aCount) const -> std::optional<std::pair<typename native_buffer<T>::free_blocks const*, typename native_buffer<T>::free_blocks::const_iterator>>
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
    inline auto native_buffer<T>::find_free_block(size_type aCount) -> std::optional<std::pair<typename native_buffer<T>::free_blocks*, typename native_buffer<T>::free_blocks::iterator>>
    {
        auto const result = const_cast<native_buffer const&>(*this).find_free_block(aCount);
        if (!result)
            return {};
        auto freeBlocks = const_cast<free_blocks*>(result->first);
        return std::make_pair(freeBlocks, std::next(freeBlocks->begin(), std::distance(freeBlocks->cbegin(), result->second)));
    }

    template <typename T>   
    inline void native_buffer<T>::grow(size_type aCapacity)
    {
        unmap();

        {
            native_buffer<T> temp{ iCacheable, aCapacity, iDeviceLocal };
            if (!empty())
                graphics_backend().copy_buffer(iBufferName, temp.iBufferName, size() * sizeof(value_type));
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
