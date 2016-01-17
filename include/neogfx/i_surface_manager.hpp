// i_surface_manager.hpp
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

#include "neogfx.hpp"
#include "i_surface.hpp"

namespace neogfx
{
	class i_surface_manager
	{
	public:
		struct surface_not_found : std::logic_error { surface_not_found() : std::logic_error("neogfx::i_surface_manager::surface_not_found") {} };
	public:
		virtual ~i_surface_manager() {}
	public:
		virtual void add_surface(i_surface& aSurface) = 0;
		virtual void remove_surface(i_surface& aSurface) = 0;
		virtual i_surface& surface_from_handle(void* aHandle) = 0;
		virtual std::size_t surface_count() const = 0;
		virtual i_surface& surface(std::size_t aIndex) = 0;
		virtual bool process_events(bool& aLastWindowClosed) = 0;
		virtual void layout_surfaces() = 0;
		virtual void invalidate_surfaces() = 0;
		virtual void clear_rendering_flags() = 0;
		virtual void display_error_message(const std::string& aTitle, const std::string& aMessage) const = 0;
		virtual void display_error_message(const i_native_surface& aParent, const std::string& aTitle, const std::string& aMessage) const = 0;
	};
}