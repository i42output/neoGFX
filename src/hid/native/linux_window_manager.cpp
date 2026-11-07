// linux_window_manager.cpp
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

#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/hid/i_mouse.hpp>
#include <neogfx/gui/window/i_native_window.hpp>
#include "../../app/native/linux_basic_services.hpp"
#include "linux_window_manager.hpp"

namespace neogfx
{
    namespace native::linux
    {
        namespace
        {
            linux::display_server& the_display_server()
            {
                return static_cast<linux::basic_services&>(service<i_basic_services>()).display_server();
            }

            // the (native, i.e. not nested) window at a position
            i_window* native_window_at(point const& aPosition)
            {
                auto& surfaceManager = service<i_surface_manager>();
                // (the window under the pointer is the one that is in front there, if the position is the pointer's)
                if (auto const underPointer = the_display_server().window_under_pointer(); underPointer != nullptr)
                {
                    auto& window = underPointer->surface_window().as_window();
                    if (rect{ underPointer->surface_position(), underPointer->surface_extents() }.contains(aPosition))
                        return &window;
                }
                // otherwise the most recently created visible one there (popups are created after their owners)
                for (std::size_t s = surfaceManager.surface_count(); s-- > 0u;)
                {
                    auto& surface = surfaceManager.surface(s);
                    if (!surface.has_native_surface() || surface.surface_type() != surface_type::Window)
                        continue;
                    auto& nativeWindow = surface.as_surface_window().native_window();
                    if (nativeWindow.is_nested() || !nativeWindow.visible())
                        continue;
                    if (rect{ nativeWindow.surface_position(), nativeWindow.surface_extents() }.contains(aPosition))
                        return &surface.as_surface_window().as_window();
                }
                return nullptr;
            }

            inline i_window* window_from_position(window_manager const& aWindowManager, const point& aPosition, i_widget const* aIgnore)
            {
                auto const nativeWindow = native_window_at(aPosition);
                if (nativeWindow != nullptr)
                {
                    auto& ancestor = *nativeWindow;
                    for (std::size_t i = 0; i < aWindowManager.window_count(); ++i)
                    {
                        auto& w = aWindowManager.window(i);
                        if (aIgnore && w.as_widget().is_descendent_of(*aIgnore))
                            continue;
                        if (w.effectively_visible() && w.is_descendent_of(ancestor) && w.non_client_rect().contains(aPosition - ancestor.position()))
                        {
                            // a nested window only counts if the surface manager agrees it is what is under the mouse
                            // (something in its parent window, e.g. a splitter's separator, may be in front of it)
                            if (!aIgnore && w.is_nested() && &service<i_surface_manager>().surface_at_position(
                                ancestor.surface(), aPosition - ancestor.position()) != &w.surface())
                                continue;
                            return &w;
                        }
                    }
                    return &ancestor;
                }
                return nullptr;
            }
        }

        window_manager::window_manager()
        {
        }

        window_manager::~window_manager()
        {
        }

        i_window* window_manager::window_from_position(const point& aPosition) const
        {
            return linux::window_from_position(*this, aPosition, nullptr);
        }

        i_window* window_manager::window_from_position(const point& aPosition, i_widget const& aIgnore) const
        {
            return linux::window_from_position(*this, aPosition, &aIgnore);
        }

        void window_manager::save_mouse_cursor()
        {
            if (iCurrentCursor)
                iSavedCursors.push_back(iCurrentCursor.value());
            else
                iSavedCursors.push_back(cursor_setting{});
        }

        void window_manager::set_mouse_cursor(mouse_system_cursor aSystemCursor, bool aOverride)
        {
            iCurrentCursor.emplace(aSystemCursor, aOverride);
            if (!cursor_locked())
                the_display_server().set_cursor(aSystemCursor);
        }

        void window_manager::restore_mouse_cursor(const i_window& aWindow)
        {
            if (iSavedCursors.empty())
                throw no_cursors_saved();
            iCurrentCursor = iSavedCursors.back();
            iSavedCursors.pop_back();
            update_mouse_cursor(aWindow);
        }

        void window_manager::update_mouse_cursor(const i_window& aWindow)
        {
            if (iCurrentCursor && iCurrentCursor.value().override)
            {
                if (!cursor_locked())
                    the_display_server().set_cursor(iCurrentCursor.value().cursor);
            }
            else
            {
                auto const mousePosition = mouse_position();
                auto w = window_from_position(mousePosition);
                if (w && w->is_descendent_of(aWindow))
                    set_mouse_cursor(w->surface().as_surface_window().native_window_mouse_cursor().system_cursor());
                else
                    set_mouse_cursor(aWindow.surface().as_surface_window().native_window_mouse_cursor().system_cursor());
            }
        }

        bool window_manager::cursor_locked() const
        {
            return iCursorLock.valid();
        }

        void window_manager::lock_mouse_cursor(i_window& aWindow)
        {
            iCursorLock = &aWindow;
        }

        void window_manager::unlock_mouse_cursor()
        {
            iCursorLock = nullptr;
        }
    }
}
