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
    private:
        void* iItem = nullptr;
        i_layout_item* iLayoutItem = nullptr;
        i_widget* iRenderItem = nullptr;
        bool iRenderGeometryText = false;
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

