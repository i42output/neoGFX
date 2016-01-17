// native_window.cpp
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

#include "neogfx.hpp"
#include "native_window.hpp"
#include "i_rendering_engine.hpp"
#include "i_surface_manager.hpp"

namespace neogfx
{
	native_window::native_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager) :
		iRenderingEngine(aRenderingEngine), iSurfaceManager(aSurfaceManager)
	{

	}

	native_window::~native_window()
	{
	}

	void native_window::display_error_message(const std::string& aTitle, const std::string& aMessage) const
	{
		iSurfaceManager.display_error_message(*this, aTitle, aMessage);
	}

	i_rendering_engine& native_window::rendering_engine() const
	{
		return iRenderingEngine;
	}
}