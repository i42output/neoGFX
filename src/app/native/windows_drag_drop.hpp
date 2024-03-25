// windows_drag_drop.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston
  
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

#include <map>
#include <Ole2.h>

#include <neogfx/app/drag_drop.hpp>
#include "windows_com.hpp"

namespace neogfx
{
    namespace native::windows
    {
        class drag_drop : public neogfx::drag_drop, private com_component<::IDropTarget, false>
        {
            typedef neogfx::drag_drop base_type;
        public:
            drag_drop();
        public:
            void register_target(i_drag_drop_target& aTarget) final;
            void unregister_target(i_drag_drop_target& aTarget) final;
        private:
            HRESULT DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) final;
            HRESULT DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) final;
            HRESULT DragLeave() final;
            HRESULT Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) final;
        private:
            i_drag_drop_target* target_for_internal_source(POINTL pt) const;
        private:
            std::map<HWND, std::pair<std::optional<destroyed_flag>, uint32_t>> iNativeDragDropTargets;
            std::unique_ptr<i_drag_drop_source> iActiveInternalDragDropSource;
            std::unique_ptr<i_drag_drop_object> iActiveInternalDragDropObject;
        };
    }
}