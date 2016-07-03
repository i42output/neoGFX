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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include "native_window.hpp"

namespace neogfx
{
	native_window::native_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager) :
		iRenderingEngine(aRenderingEngine), iSurfaceManager(aSurfaceManager), iProcessingEvent{ false }
	{

	}

	native_window::~native_window()
	{
	}

	void native_window::display_error_message(const std::string& aTitle, const std::string& aMessage) const
	{
		iSurfaceManager.display_error_message(*this, aTitle, aMessage);
	}

	void native_window::push_event(const native_event& aEvent)
	{
		iEventQueue.push_back(aEvent);
	}

	bool native_window::pump_event()
	{
		if (iEventQueue.empty())
			return false;
		iProcessingEvent = true;
		auto nativeEvent = iEventQueue.front();
		iEventQueue.pop_front();
		if (nativeEvent.is<native_window_event>())
		{
			const auto& windowEvent = static_variant_cast<const native_window_event&>(nativeEvent);
			switch (windowEvent.type())
			{
			case native_window_event::Close:
				close();
				break;
			case native_window_event::Resized:
				event_handler().native_window_resized();
				break;
			case native_window_event::SizeChanged:
				event_handler().native_window_resized();
				break;
			case native_window_event::Enter:
				event_handler().native_window_mouse_entered();
				break;
			case native_window_event::Leave:
				event_handler().native_window_mouse_left();
				break;
			case native_window_event::FocusGained:
				event_handler().native_window_focus_gained();
				break;
			case native_window_event::FocusLost:
				event_handler().native_window_focus_lost();
				break;
			default:
				/* do nothing */
				break;
			}
		}
		else if (nativeEvent.is<native_mouse_event>())
		{
			const auto& mouseEvent = static_variant_cast<const native_mouse_event&>(nativeEvent);
			switch (mouseEvent.type())
			{
			case native_mouse_event::WheelScrolled:
				event_handler().native_window_mouse_wheel_scrolled(mouseEvent.mouse_wheel(), mouseEvent.delta());
				break;
			case native_mouse_event::ButtonPressed:
				event_handler().native_window_mouse_button_pressed(mouseEvent.mouse_button(), mouseEvent.position(), mouseEvent.key_modifiers());
				break;
			case native_mouse_event::ButtonDoubleClicked:
				event_handler().native_window_mouse_button_double_clicked(mouseEvent.mouse_button(), mouseEvent.position(), mouseEvent.key_modifiers());
				break;
			case native_mouse_event::ButtonReleased:
				event_handler().native_window_mouse_button_released(mouseEvent.mouse_button(), mouseEvent.position());
				break;
			case native_mouse_event::Moved:
				event_handler().native_window_mouse_moved(mouseEvent.position());
				break;
			default:
				/* do nothing */
				break;
			}
		}
		else if (nativeEvent.is<native_keyboard_event>())
		{
			auto& keyboard = app::instance().keyboard();
			const auto& keyboardEvent = static_variant_cast<const native_keyboard_event&>(nativeEvent);
			switch (keyboardEvent.type())
			{
			case native_keyboard_event::KeyPressed:
				if (!keyboard.grabber().key_pressed(keyboardEvent.scan_code(), keyboardEvent.key_code(), keyboardEvent.key_modifiers()))
				{
					keyboard.key_pressed.trigger(keyboardEvent.scan_code(), keyboardEvent.key_code(), keyboardEvent.key_modifiers());
					event_handler().native_window_key_pressed(keyboardEvent.scan_code(), keyboardEvent.key_code(), keyboardEvent.key_modifiers());
				}
				break;
			case native_keyboard_event::KeyReleased:
				if (!keyboard.grabber().key_released(keyboardEvent.scan_code(), keyboardEvent.key_code(), keyboardEvent.key_modifiers()))
				{
					keyboard.key_released.trigger(keyboardEvent.scan_code(), keyboardEvent.key_code(), keyboardEvent.key_modifiers());
					event_handler().native_window_key_released(keyboardEvent.scan_code(), keyboardEvent.key_code(), keyboardEvent.key_modifiers());
				}
				break;
			case native_keyboard_event::TextInput:
				if (!keyboard.grabber().text_input(keyboardEvent.text()))
				{
					keyboard.text_input.trigger(keyboardEvent.text());
					event_handler().native_window_text_input(keyboardEvent.text());
				}
				break;
			case native_keyboard_event::SysTextInput:
				if (!keyboard.grabber().sys_text_input(keyboardEvent.text()))
				{
					keyboard.sys_text_input.trigger(keyboardEvent.text());
					event_handler().native_window_sys_text_input(keyboardEvent.text());
				}
				break;
			default:
				/* do nothing */
				break;
			}
		}
		iProcessingEvent = false;
		return true;
	}

	bool native_window::processing_event() const
	{
		return iProcessingEvent;
	}

	i_rendering_engine& native_window::rendering_engine() const
	{
		return iRenderingEngine;
	}

	i_surface_manager& native_window::surface_manager() const
	{
		return iSurfaceManager;
	}
}