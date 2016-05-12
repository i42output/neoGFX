// native_window.hpp
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
#include "i_native_window.hpp"

namespace neogfx
{
	class i_rendering_engine;
	class i_surface_manager;

	class native_window : public i_native_window
	{
	public:
		native_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager);
		virtual ~native_window();
	public:
		virtual void display_error_message(const std::string& aTitle, const std::string& aMessage) const;
	public:
		i_rendering_engine& rendering_engine() const;
		i_surface_manager& surface_manager() const;
	private:
		i_rendering_engine& iRenderingEngine;
		i_surface_manager& iSurfaceManager;
	};
}