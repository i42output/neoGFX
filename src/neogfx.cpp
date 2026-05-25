// neogfx.cpp
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

#include <neogfx/neogfx.hpp>

#include <neolib/app/ostream_logger.hpp>

namespace neogfx::debug
{
#ifdef NEOGFX_DEBUG
    class debug_service : public i_debug
    {
    public:
        void*& item() final
        {
            return iItem;
        }
        i_layout_item*& layout_item() final
        {
            return iLayoutItem;
        }
        i_widget*& render_item() final
        {
            return iRenderItem;
        }
        bool& render_geometry_text() final
        {
            return iRenderGeometryText;
        }
        i_texture*& texture() final
        {
            return iTexture;
        }
        bool& flag1() final
        {
            return iFlag1;
        }
        bool& flag2() final
        {
            return iFlag2;
        }
        bool& flag3() final
        {
            return iFlag3;
        }
        bool& flag4() final
        {
            return iFlag4;
        }
        bool& flag5() final
        {
            return iFlag5;
        }
        void*& ptr1() final
        {
            return iPtr1;
        }
        void*& ptr2() final
        {
            return iPtr2;
        }
        void*& ptr3() final
        {
            return iPtr3;
        }
        void*& ptr4() final
        {
            return iPtr4;
        }
        void*& ptr5() final
        {
            return iPtr5;
        }
        std::int64_t& int1() final
        {
            return iInt1;
        }
        std::int64_t& int2() final
        {
            return iInt2;
        }
        std::int64_t& int3() final
        {
            return iInt3;
        }
        std::int64_t& int4() final
        {
            return iInt4;
        }
        std::int64_t& int5() final
        {
            return iInt5;
        }
        std::uint64_t& uint1() final
        {
            return iUint1;
        }
        std::uint64_t& uint2() final
        {
            return iUint2;
        }
        std::uint64_t& uint3() final
        {
            return iUint3;
        }
        std::uint64_t& uint4() final
        {
            return iUint4;
        }
        std::uint64_t& uint5() final
        {
            return iUint5;
        }
        uuid& uuid1() final
        {
            return iUuid1;
        }
        uuid& uuid2() final
        {
            return iUuid2;
        }
        uuid& uuid3() final
        {
            return iUuid3;
        }
        uuid& uuid4() final
        {
            return iUuid4;
        }
        uuid& uuid5() final
        {
            return iUuid5;
        }
    private:
        void* iItem = nullptr;
        i_layout_item* iLayoutItem = nullptr;
        i_widget* iRenderItem = nullptr;
        bool iRenderGeometryText = false;
        i_texture* iTexture = nullptr;
        bool iFlag1 = false;
        bool iFlag2 = false;
        bool iFlag3 = false;
        bool iFlag4 = false;
        bool iFlag5 = false;
        void* iPtr1 = nullptr;
        void* iPtr2 = nullptr;
        void* iPtr3 = nullptr;
        void* iPtr4 = nullptr;
        void* iPtr5 = nullptr;
        std::int64_t iInt1 = {};
        std::int64_t iInt2 = {};
        std::int64_t iInt3 = {};
        std::int64_t iInt4 = {};
        std::int64_t iInt5 = {};
        std::uint64_t iUint1 = {};
        std::uint64_t iUint2 = {};
        std::uint64_t iUint3 = {};
        std::uint64_t iUint4 = {};
        std::uint64_t iUint5 = {};
        uuid iUuid1 = {};
        uuid iUuid2 = {};
        uuid iUuid3 = {};
        uuid iUuid4 = {};
        uuid iUuid5 = {};
    };
#endif // NEOGFX_DEBUG
}

template<> neogfx::debug::logger& services::start_service<neogfx::debug::logger>()
{
    static neolib::logger::ostream_logger<9999> sLogger{ std::cerr };
    sLogger.create_logging_thread();
    return sLogger;
}

#ifdef NEOGFX_DEBUG
template<> neogfx::debug::i_debug& services::start_service<neogfx::debug::i_debug>()
{
    static neogfx::debug::debug_service sDebug;
    return sDebug;
}
#endif

