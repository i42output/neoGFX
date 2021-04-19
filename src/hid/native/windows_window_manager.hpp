// windows_window_manager.hpp
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
#include <neogfx/hid/window_manager.hpp>

namespace neogfx
{
    namespace native::windows
    {
        class window_manager : public neogfx::window_manager
        {
        public:
            struct no_cursors_saved : std::logic_error { no_cursors_saved() : std::logic_error("neogfx::native::windows::window_manager::no_cursors_saved") {} };
        private:
            class cursor
            {
            public:
                cursor(HCURSOR aHandle, bool aShared = true) : 
                    iHandle{ aHandle }, iShared{ aShared }
                {
                }
                ~cursor()
                {
                    if (!iShared)
                        ::DestroyCursor(iHandle);
                }
            public:
                HCURSOR handle() const
                {
                    return iHandle;
                }
            private:
                HCURSOR iHandle;
                bool iShared;
            };
            typedef std::shared_ptr<cursor> cursor_pointer;
        public:
            window_manager();
            ~window_manager();
        public:
            i_window* window_from_position(const point& aPosition) const override;
        public:
            void save_mouse_cursor() override;
            void set_mouse_cursor(mouse_system_cursor aSystemCursor) override;
            void restore_mouse_cursor(const i_window& aWindow) override;
            void update_mouse_cursor(const i_window& aWindow) override;
        private:
            cursor_pointer iCurrentCursor;
            std::vector<cursor_pointer> iSavedCursors;
        };
    }
}