// surface_manager.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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
#include <set>
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include "i_surface_manager.hpp"

namespace neogfx
{
	class surface_manager : public i_surface_manager
	{
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
					return left < right;
			}
		};
		typedef std::set<i_surface*, surface_sorter> surface_list;
	public:
		surface_manager(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine);
	public:
		virtual void add_surface(i_surface& aSurface);
		virtual void remove_surface(i_surface& aSurface);
		virtual std::size_t surface_count() const;
		virtual i_surface& surface(std::size_t aIndex);
		virtual bool any_strong_surfaces() const;
		virtual bool process_events(bool& aLastWindowClosed);
		virtual void layout_surfaces();
		virtual void invalidate_surfaces();
		virtual void render_surfaces();
		virtual void display_error_message(const std::string& aTitle, const std::string& aMessage) const;
		virtual void display_error_message(const i_native_surface& aParent, const std::string& aTitle, const std::string& aMessage) const;
		virtual uint32_t display_count() const;
		virtual rect desktop_rect(uint32_t aDisplayIndex = 0) const;
		virtual rect desktop_rect(const i_surface& aSurface) const;
	public:
		virtual bool is_surface_attached(void* aNativeSurfaceHandle) const;
		virtual i_surface& attached_surface(void* aNativeSurfaceHandle);
	private:
		i_basic_services& iBasicServices;
		i_rendering_engine& iRenderingEngine;
		surface_list iSurfaces;
		bool iRenderingSurfaces;
	};
}