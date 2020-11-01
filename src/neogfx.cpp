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

#ifdef NEOGFX_DEBUG
namespace neogfx
{
    namespace debug
    {
        void* item;
        i_layout_item* layoutItem;
        i_widget* renderItem;
        bool renderGeometryText;
    }
}

template<> neogfx::debug::logger& services::start_service<neogfx::debug::logger>()
{
    static neolib::logger::ostream_logger<9999> sLogger{ std::cerr };
    sLogger.create_logging_thread();
    return sLogger;
}

#endif // NEOGFX_DEBUG

