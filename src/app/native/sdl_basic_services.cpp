// sdl_basic_services.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
#include <SDL_messagebox.h>
#include <SDL_clipboard.h>
#include <SDL_syswm.h>
#ifdef WIN32
#include <Windows.h>
#endif
#include "sdl_basic_services.hpp"
#include "i_native_clipboard.hpp"


namespace neogfx
{
	sdl_basic_services::sdl_basic_services(neolib::io_thread& aAppThread) : 
		iAppThread(aAppThread)
	{
	}

	neogfx::platform sdl_basic_services::platform() const
	{
		return neogfx::platform::Windows;
	}

	neolib::io_thread& sdl_basic_services::app_thread()
	{
		return iAppThread;
	}


	void sdl_basic_services::system_beep()
	{
#if defined(WIN32) 
		MessageBeep(MB_OK);
#elif defined(__APPLE__) 
		SysBeep(1);
#elif defined(SDL_VIDEO_DRIVER_X11) 
		SDL_SysWMInfo info;
		SDL_VERSION(&info.version);
		SDL_GetWMInfo(&info);
		XBell(info.info.x11.display, 100);
#else 
		std::cerr << '\a' << std::flush;
#endif 
	}

	void sdl_basic_services::display_error_dialog(const std::string& aTitle, const std::string& aMessage, void* aParentWindowHandle) const
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, aTitle.c_str(), aMessage.c_str(), static_cast<SDL_Window*>(aParentWindowHandle));
	}

	uint32_t sdl_basic_services::display_count() const
	{
		return SDL_GetNumVideoDisplays();
	}

	rect sdl_basic_services::desktop_rect(uint32_t aDisplayIndex) const
	{
		iDesktopWorkAreas.clear();
#ifdef WIN32
		EnumDisplayMonitors(NULL, NULL, &enum_display_monitors_proc, reinterpret_cast<LPARAM>(this));
#else
		for (int i = 0; i < display_count(); ++i)
		{
			SDL_Rect rectDisplayBounds;
			SDL_GetDisplayBounds(i, &rectDisplayBounds);
			iDesktopWorkAreas.push_back(rect{ point{ rectDisplayBounds.x, rectDisplayBounds.y }, size{ rectDisplayBounds.w, rectDisplayBounds.h } });
		}
#endif
		if (aDisplayIndex >= iDesktopWorkAreas.size())
			throw bad_display_index();
		return iDesktopWorkAreas[aDisplayIndex];
	}

	class sdl_clipboard : public i_native_clipboard
	{
	public:
		virtual bool has_text() const
		{
			return SDL_HasClipboardText() == SDL_TRUE;
		}
		virtual std::string text() const
		{
			char* clipboardText = SDL_GetClipboardText();
			if (clipboardText == NULL)
				return std::string{};
			else
				return std::string{clipboardText};
		}
		virtual void set_text(const std::string& aText)
		{
			SDL_SetClipboardText(aText.c_str());
		}
	};

	bool sdl_basic_services::has_system_clipboard() const
	{
		return true;
	}

	i_native_clipboard& sdl_basic_services::system_clipboard()
	{
		static sdl_clipboard sSystemClipboard;
		return sSystemClipboard;
	}

	bool sdl_basic_services::has_system_menu_bar() const
	{
		return false;
	}

	i_shared_menu_bar& sdl_basic_services::system_menu_bar()
	{
		throw no_system_menu_bar();
	}

#ifdef WIN32
	BOOL CALLBACK sdl_basic_services::enum_display_monitors_proc(HMONITOR aMonitor, HDC, LPRECT, LPARAM aThis)
	{
		rect rectDisplay;
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		GetMonitorInfo(aMonitor, &mi);
		basic_rect<LONG> workAreaRect{ basic_point<LONG>{ mi.rcWork.left, mi.rcWork.top }, basic_size<LONG>{ mi.rcWork.right - mi.rcWork.left, mi.rcWork.bottom - mi.rcWork.top } };
		reinterpret_cast<const sdl_basic_services*>(aThis)->iDesktopWorkAreas.push_back(workAreaRect);
		return true;
	}
#endif
}