// sdl_service_factory.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/i_service_factory.hpp>
#include "sdl_basic_services.hpp"
#include "../../hid/native/sdl_keyboard.hpp"
#include "../../hid/native/sdl_window_manager.hpp"
#include "../../gfx/native/sdl_renderer.hpp"

namespace neogfx
{
	class sdl_service_factory : public i_service_factory
	{
	public:
		virtual std::unique_ptr<i_basic_services> create_basic_services(neolib::io_task& aAppTask)
		{
			return std::make_unique<sdl_basic_services>(aAppTask);
		}
		virtual std::unique_ptr<i_keyboard> create_keyboard()
		{
			return std::make_unique<sdl_keyboard>();
		}
		virtual std::unique_ptr<i_rendering_engine> create_rendering_engine(renderer aRenderer, bool aDoubleBufferedWindows, i_basic_services& aBasicServices, i_keyboard& aKeyboard)
		{
			return std::make_unique<sdl_renderer>(aRenderer, aDoubleBufferedWindows, aBasicServices, aKeyboard);
		}
		virtual std::unique_ptr<i_window_manager> create_window_manager()
		{
			return std::make_unique<sdl_window_manager>();
		}
	};

	i_service_factory& default_service_factory()
	{
		static sdl_service_factory sDefaultServiceFactory;
		return sDefaultServiceFactory;
	}
}