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

#include <neogfx/neogfx.hpp>
#include <neolib/string_utils.hpp>
#include <neogfx/hid/surface_manager.hpp>
#include "native/i_native_surface.hpp"
#include "../gui/window/native/i_native_window.hpp"

namespace neogfx
{
	surface_manager::surface_manager(i_basic_services& aBasicServices, i_rendering_engine& aRenderingEngine) :
		iBasicServices(aBasicServices), iRenderingEngine(aRenderingEngine), iRenderingSurfaces(false)
	{
	}

	bool surface_manager::initialising_surface() const
	{
		for (auto const& s : iSurfaces)
		{
			if (!s->destroyed() && s->native_surface().initialising())
				return true;
		}
		return false;
	}

	void surface_manager::add_surface(i_surface& aSurface)
	{
		iSurfaces.insert(&aSurface);
	}
	
	void surface_manager::remove_surface(i_surface& aSurface)
	{
		auto existingSurface = iSurfaces.find(&aSurface);
		if (existingSurface != iSurfaces.end())
		{
			for (auto s = iSurfaces.begin(); s != iSurfaces.end();)
			{
				if (aSurface.is_owner_of(**s))
				{
					auto& childSurface = **s;
					iSurfaces.erase(s);
					childSurface.close();
					s = iSurfaces.begin();
				}
				else
					++s;
			}
			iSurfaces.erase(existingSurface);
		}	
	}

	bool surface_manager::is_surface_attached(void* aNativeSurfaceHandle) const
	{
		for (auto& s : iSurfaces)
			if (!s->destroyed() && s->native_surface().handle() == aNativeSurfaceHandle)
				return true;
		return false;
	}

	i_surface& surface_manager::attached_surface(void* aNativeSurfaceHandle)
	{
		for (auto& s : iSurfaces)
			if (!s->destroyed() && s->native_surface().handle() == aNativeSurfaceHandle)
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

	bool surface_manager::any_strong_surfaces() const
	{
		for (auto& s : iSurfaces)
			if (!s->is_weak())
				return true;
		return false;
	}
		
	bool surface_manager::process_events(bool& aLastWindowClosed)
	{
		bool hadStrong = any_strong_surfaces();
		bool handledEvents = iRenderingEngine.process_events();
		if (hadStrong && !any_strong_surfaces())
			aLastWindowClosed = true;
		return handledEvents;
	}

	void surface_manager::layout_surfaces()
	{
		for (auto i = iSurfaces.begin(); i != iSurfaces.end(); ++i)
			(*i)->layout_surface();
	}

	void surface_manager::invalidate_surfaces()
	{
		for (auto& s : iSurfaces)
			s->invalidate_surface(rect(point{}, s->surface_size()), false);
	}

	void surface_manager::render_surfaces()
	{
		if (iRenderingSurfaces || iRenderingEngine.creating_window())
			return;
		iRenderingSurfaces = true;
		for (auto& s : iSurfaces)
			s->render_surface();
		iRenderingSurfaces = false;
	}

	void surface_manager::display_error_message(const std::string& aTitle, const std::string& aMessage) const
	{
		for (auto i = iSurfaces.begin(); i != iSurfaces.end(); ++i)
		{
			if ((*i)->destroyed())
				continue;
			if ((*i)->surface_type() == surface_type::Window && static_cast<i_native_window&>((*i)->native_surface()).is_active())
			{
				display_error_message((*i)->native_surface(), aTitle, aMessage);
				return;
			}
		}
		iBasicServices.display_error_dialog(aTitle.c_str(), aMessage.c_str(), 0);
	}

	void surface_manager::display_error_message(const i_native_surface& aParent, const std::string& aTitle, const std::string& aMessage) const
	{
		iBasicServices.display_error_dialog(aTitle.c_str(), aMessage.c_str(), aParent.handle());
	}

	uint32_t surface_manager::display_count() const
	{
		return iBasicServices.display_count();
	}

	rect surface_manager::desktop_rect(uint32_t aDisplayIndex) const
	{
		return iBasicServices.desktop_rect(aDisplayIndex);
	}

	rect surface_manager::desktop_rect(const i_surface& aSurface) const
	{
#ifdef WIN32
		HMONITOR monitor = MonitorFromWindow(reinterpret_cast<HWND>(aSurface.native_surface().native_handle()), MONITOR_DEFAULTTONEAREST);
		MONITORINFOEX mi;
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(monitor, &mi);
		return basic_rect<LONG>{ basic_point<LONG>{ mi.rcWork.left, mi.rcWork.top }, basic_size<LONG>{ mi.rcWork.right - mi.rcWork.left, mi.rcWork.bottom - mi.rcWork.top } };
#else
		/* todo */
		rect rectSurface{ aSurface.surface_position(), aSurface.surface_size() };
		std::multimap<double, uint32_t> matches;
		for (uint32_t i = 0; i < display_count(); ++i)
		{
			rect rectDisplay = desktop_rect(i);
			rect rectIntersection = rectDisplay.intersection(rectSurface);
			if (!rectIntersection.empty())
				matches[rectIntersection.width() * rectIntersection.height()] = i;
		}
		if (matches.empty())
			return desktop_rect(0);
		return desktop_rect(matches.rbegin()->second);
#endif
	}
}