// windows_window_manager.cpp
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

#include <neogfx/neogfx.hpp>

#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/hid/i_mouse.hpp>
#include "windows_window_manager.hpp"

namespace neogfx
{
    namespace native::windows
    {
        window_manager::window_manager()
        {
        }

        window_manager::~window_manager()
        {
        }

        inline i_window* window_from_position(window_manager const& aWindowManager, const point& aPosition, i_widget const* aIgnore)
        {
            HWND hwnd = ::WindowFromPoint(POINT{ static_cast<LONG>(aPosition.x), static_cast<LONG>(aPosition.y) });
            if (service<i_surface_manager>().is_surface_attached(hwnd))
            {
                auto& ancestor = service<i_surface_manager>().attached_surface(hwnd).as_surface_window().as_window();
                for (std::size_t i = 0; i < aWindowManager.window_count(); ++i)
                {
                    auto& w = aWindowManager.window(i);
                    if (aIgnore && w.as_widget().is_descendent_of(*aIgnore))
                        continue;
                    if (w.effectively_visible() && w.is_descendent_of(ancestor) && w.non_client_rect().contains(aPosition - ancestor.position()))
                        return &w;
                }
                return &ancestor;
            }
            return nullptr;
        }

        i_window* window_manager::window_from_position(const point& aPosition) const
        {
            return windows::window_from_position(*this, aPosition, nullptr);
        }

        i_window* window_manager::window_from_position(const point& aPosition, i_widget const& aIgnore) const
        {
            return windows::window_from_position(*this, aPosition, &aIgnore);
        }

        void window_manager::save_mouse_cursor()
        {
            iSavedCursors.push_back(std::make_shared<cursor>(::GetCursor()));
        }

        void window_manager::set_mouse_cursor(mouse_system_cursor aSystemCursor)
        {
            auto cursorResource = IDC_ARROW;
            switch (aSystemCursor)
            {
            case mouse_system_cursor::Arrow:
                cursorResource = IDC_ARROW;
                break;
            case mouse_system_cursor::IBeam:
                cursorResource = IDC_IBEAM;
                break;
            case mouse_system_cursor::Wait:
                cursorResource = IDC_WAIT;
                break;
            case mouse_system_cursor::Crosshair:
                cursorResource = IDC_CROSS;
                break;
            case mouse_system_cursor::WaitArrow:
                cursorResource = IDC_APPSTARTING;
                break;
            case mouse_system_cursor::SizeNWSE:
                cursorResource = IDC_SIZENWSE;
                break;
            case mouse_system_cursor::SizeNESW:
                cursorResource = IDC_SIZENESW;
                break;
            case mouse_system_cursor::SizeWE:
                cursorResource = IDC_SIZEWE;
                break;
            case mouse_system_cursor::SizeNS:
                cursorResource = IDC_SIZENS;
                break;
            case mouse_system_cursor::SizeAll:
                cursorResource = IDC_SIZEALL;
                break;
            case mouse_system_cursor::No:
                cursorResource = IDC_NO;
                break;
            case mouse_system_cursor::Hand:
                cursorResource = IDC_HAND;
                break;
            }
            iCurrentCursor = std::make_shared<cursor>(::LoadCursor(NULL, cursorResource));
            ::SetCursor(iCurrentCursor->handle());
        }

        void window_manager::restore_mouse_cursor(const i_window& aWindow)
        {
            if (iSavedCursors.empty())
                throw no_cursors_saved();
            iCurrentCursor = iSavedCursors.back();
            iSavedCursors.pop_back();
            ::SetCursor(iCurrentCursor->handle());
            update_mouse_cursor(aWindow);
        }

        void window_manager::update_mouse_cursor(const i_window& aWindow)
        {
            if (iSavedCursors.empty())
            {
                auto const mousePosition = mouse_position();
                auto w = window_from_position(mousePosition);
                if (w && w->is_descendent_of(aWindow))
                    set_mouse_cursor(w->surface().as_surface_window().native_window_mouse_cursor().system_cursor());
                else
                    set_mouse_cursor(aWindow.surface().as_surface_window().native_window_mouse_cursor().system_cursor());
            }
        }
    }
}