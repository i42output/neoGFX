// i_surface_manager.hpp
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
#include "i_surface.hpp"
#include "i_display.hpp"

namespace neogfx
{
    class i_display;

    class i_surface_manager : public i_service
    {
    public:
        declare_event(dpi_changed, i_surface&)
    public:
        struct surface_not_found : std::logic_error { surface_not_found() : std::logic_error("neogfx::i_surface_manager::surface_not_found") {} };
    public:
        virtual ~i_surface_manager() = default;
    public:
        virtual bool initialising_surface() const = 0;
        virtual void add_surface(i_surface& aSurface) = 0;
        virtual void remove_surface(i_surface& aSurface) = 0;
        virtual std::size_t surface_count() const = 0;
        virtual i_surface& surface(std::size_t aIndex) = 0;
        virtual bool any_strong_surfaces() const = 0;
        virtual std::size_t strong_surface_count() const = 0;
        virtual bool process_events(bool& aLastWindowClosed) = 0;
        virtual void layout_surfaces() = 0;
        virtual void invalidate_surfaces() = 0;
        virtual void render_surfaces() = 0;
        virtual i_surface* locate_topmost_usable_surface(const point& aPosition) = 0;
        virtual void display_error_message(const std::string& aTitle, const std::string& aMessage) const = 0;
        virtual void display_error_message(const i_native_surface& aParent, const std::string& aTitle, const std::string& aMessage) const = 0;
        virtual uint32_t display_count() const = 0;
        virtual i_display& display(uint32_t aDisplayIndex = 0) const = 0;
        virtual i_display& display(const i_surface& aSurface) const = 0;
        virtual rect desktop_rect(uint32_t aDisplayIndex = 0) const = 0;
        virtual rect desktop_rect(const i_surface& aSurface) const = 0;
    public:
        virtual bool is_surface_attached(void* aNativeSurfaceHandle) const = 0;
        virtual i_surface& attached_surface(void* aNativeSurfaceHandle) = 0;
    public:
        static uuid const& iid() { static uuid const sIid{ 0xf189b8a1, 0x37d3, 0x4288, 0x9654, { 0xf8, 0xd, 0x19, 0xf1, 0xe, 0x2d } }; return sIid; }
    };
}