// surface_manager.cpp
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
#include "surface_manager.hpp"
#include <SDL_messagebox.h>
#include <neolib/string_utils.hpp>
#include "i_native_surface.hpp"
#include "i_native_window.hpp"

namespace neogfx
{
	surface_manager::surface_manager(i_rendering_engine& aRenderingEngine) :
		iRenderingEngine(aRenderingEngine)
	{
	}

	void surface_manager::add_surface(i_surface& aSurface)
	{
		iSurfaces.insert(&aSurface);
	}
	
	void surface_manager::remove_surface(i_surface& aSurface)
	{
		iSurfaces.erase(iSurfaces.find(&aSurface));
	}

	i_surface& surface_manager::surface_from_handle(void* aHandle)
	{
		for (auto& s : iSurfaces)
			if (s->native_surface().handle() == aHandle)
				return *s;
		throw surface_not_found();
	}

	std::size_t surface_manager::surface_count() const
	{
		return iSurfaces.size();
	}

	i_surface& surface_manager::surface(std::size_t aIndex)
	{
		return **std::next(iSurfaces.begin(), aIndex);
	}
	
	bool surface_manager::process_events(bool& aLastWindowClosed)
	{
		bool hadWindows = !iSurfaces.empty();
		bool handledEvents = iRenderingEngine.process_events();
		if (hadWindows && iSurfaces.empty())
			aLastWindowClosed = true;
		clear_rendering_flags();
		return handledEvents;
	}

	void surface_manager::layout_surfaces()
	{
		for (auto i = iSurfaces.begin(); i != iSurfaces.end(); ++i)
			(*i)->layout_surface();
	}

	void surface_manager::invalidate_surfaces()
	{
		for (auto i = iSurfaces.begin(); i != iSurfaces.end(); ++i)
			(*i)->invalidate_surface(rect(point{}, (*i)->surface_size()), false);
	}

	void surface_manager::clear_rendering_flags()
	{
		for (auto& s : iSurfaces)
			if (!s->destroyed())
				s->native_surface().clear_rendering_flag();
	}

	void surface_manager::display_error_message(const std::string& aTitle, const std::string& aMessage) const
	{
		for (auto i = iSurfaces.begin(); i != iSurfaces.end(); ++i)
		{
			if ((*i)->destroyed())
				continue;
			if ((*i)->surface_type() == i_surface::SurfaceTypeWindow && static_cast<i_native_window&>((*i)->native_surface()).is_active())
			{
				display_error_message((*i)->native_surface(), aTitle, aMessage);
				return;
			}
		}
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, aTitle.c_str(), aMessage.c_str(), NULL);
	}

	void surface_manager::display_error_message(const i_native_surface& aParent, const std::string& aTitle, const std::string& aMessage) const
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, aTitle.c_str(), aMessage.c_str(), static_cast<SDL_Window*>(aParent.handle()));
	}
}