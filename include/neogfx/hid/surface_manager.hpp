// surface_manager.hpp
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

#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/nest.hpp>

namespace neogfx
{
    class surface_manager : public i_surface_manager
    {
    public:
        define_declared_event(DpiChanged, dpi_changed, i_surface&)
    public:
        struct already_rendering_surfaces : std::logic_error { already_rendering_surfaces() : std::logic_error("neogfx::surface_manager::already_rendering_surfaces") {} };
    private:
        struct surface_sorter
        {
            bool operator()(i_surface* left, i_surface* right) const
            {
                if (left->is_owner_of(*right))
                    return true;
                else if (right->is_owner_of(*left))
                    return false;
                else
                    return left->z_order() > right->z_order();
            }
        };
        typedef std::vector<i_surface*> surface_list;
        typedef neolib::mutable_set<nest> nest_list;
    public:
        surface_manager(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine);
    public:
        bool initialising_surface() const override;
        void add_surface(i_surface& aSurface) override;
        void remove_surface(i_surface& aSurface) override;
        std::size_t surface_count() const override;
        i_surface& surface(std::size_t aIndex) override;
        bool any_strong_surfaces() const override;
        std::size_t strong_surface_count() const override;
        bool process_events(bool& aLastWindowClosed) override;
        void layout_surfaces() override;
        void invalidate_surfaces() override;
        void render_surfaces() override;
        void display_error_message(std::string const& aTitle, std::string const& aMessage) const override;
        void display_error_message(const i_native_window& aParent, std::string const& aTitle, std::string const& aMessage) const override;
        std::uint32_t display_count() const override;
        i_display& display(std::uint32_t aDisplayIndex = 0) const override;
        i_display& display(const i_surface& aSurface) const override;
        rect desktop_rect(std::uint32_t aDisplayIndex = 0) const override;
        rect desktop_rect(const i_surface& aSurface) const override;
    public:
        const i_surface& surface_at_position(const i_surface& aProgenitor, const point& aPosition, bool aForMouseEvent = false) const override;
        i_surface& surface_at_position(const i_surface& aProgenitor, const point& aPosition, bool aForMouseEvent = false) override;
    public:
        i_nest& nest_for(i_widget& aNestWidget, nest_type aNestType) const override;
        virtual i_nest& find_nest(i_native_window const& aNestedWindow) const override;
        void destroy_nest(i_nest& aNest) override;
        i_nest& active_nest() const override;
        void activate_nest(i_nest& aNest) override;
        void deactivate_nest(i_nest& aNest) override;
    public:
        bool is_surface_attached(void* aNativeSurfaceHandle) const override;
        i_surface& attached_surface(void* aNativeSurfaceHandle) override;
    private:
        i_basic_services& iBasicServices;
        i_rendering_engine& iRenderingEngine;
        surface_list iSurfaces;
        bool iRenderingSurfaces;
        sink iSink;
        mutable std::vector<std::unique_ptr<i_nest>> iNests;
        std::vector<i_nest*> iActiveNest;
    };
}