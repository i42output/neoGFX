// window_events.hpp
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
#include <neolib/variant.hpp>
#include <neogfx/core/geometry.hpp>
#include <neogfx/hid/mouse.hpp>
#include <neogfx/hid/keyboard.hpp>

namespace neogfx
{
	class window_event
	{
	public:
		typedef neolib::variant<neogfx::size> parameter_type;
		enum type_e
		{
			Paint,
			Close,
			Enabled,
			Disabled,
			Resizing,
			Resized,
			SizeChanged,
			Maximized,
			Iconized,
			Restored,
			Enter,
			Leave,
			FocusGained,
			FocusLost,
			TitleTextChanged
		};
	public:
		window_event(type_e aType, const parameter_type& aParameter = parameter_type()) :
			iType(aType), iParameter(aParameter)
		{
		}
	public:
		type_e type() const
		{
			return iType;
		}
		neogfx::size size() const
		{
			return static_variant_cast<neogfx::size>(iParameter);
		}
		void set_size(const neogfx::size& aSize)
		{
			static_variant_cast<neogfx::size&>(iParameter) = aSize;;
		}
	private:
		type_e iType;
		parameter_type iParameter;
	};

	class mouse_event
	{
	public:
		typedef neolib::variant<neogfx::mouse_wheel, neogfx::delta, neogfx::mouse_button, neogfx::point, neogfx::key_modifiers_e> parameter_type;
		enum type_e
		{
			WheelScrolled,
			ButtonPressed,
			ButtonDoubleClicked,
			ButtonReleased,
			Moved
		};
	public:
		mouse_event(type_e aType, const parameter_type& aParameter1 = parameter_type(), const parameter_type& aParameter2 = parameter_type(), const parameter_type& aParameter3 = parameter_type()) :
			iType(aType), iParameter1(aParameter1), iParameter2(aParameter2), iParameter3(aParameter3)
		{
		}
	public:
		type_e type() const
		{
			return iType;
		}
		neogfx::mouse_wheel mouse_wheel() const
		{
			return static_variant_cast<neogfx::mouse_wheel>(iParameter1);
		}
		neogfx::delta delta() const
		{
			return static_variant_cast<neogfx::delta>(iParameter2);
		}
		neogfx::mouse_button mouse_button() const
		{
			return static_variant_cast<neogfx::mouse_button>(iParameter1);
		}
		neogfx::point position() const
		{
			return static_variant_cast<neogfx::point>(iType != Moved ? iParameter2 : iParameter1);
		}
		neogfx::key_modifiers_e key_modifiers() const
		{
			return static_variant_cast<neogfx::key_modifiers_e>(iParameter3);
		}
	private:
		type_e iType;
		parameter_type iParameter1;
		parameter_type iParameter2;
		parameter_type iParameter3;
	};

	class keyboard_event
	{
	public:
		typedef neolib::variant<neogfx::scan_code_e, neogfx::key_code_e, neogfx::key_modifiers_e, std::string> parameter_type;
		enum type_e
		{
			KeyPressed,
			KeyReleased,
			TextInput,
			SysTextInput
		};
	public:
		keyboard_event(type_e aType, const parameter_type& aParameter1 = parameter_type(), const parameter_type& aParameter2 = parameter_type(), const parameter_type& aParameter3 = parameter_type()) :
			iType(aType), iParameter1(aParameter1), iParameter2(aParameter2), iParameter3(aParameter3)
		{
		}
	public:
		type_e type() const
		{
			return iType;
		}
		neogfx::scan_code_e scan_code() const
		{
			return static_variant_cast<neogfx::scan_code_e>(iParameter1);
		}
		neogfx::key_code_e key_code() const
		{
			return static_variant_cast<neogfx::key_code_e>(iParameter2);
		}
		neogfx::key_modifiers_e key_modifiers() const
		{
			return static_variant_cast<neogfx::key_modifiers_e>(iParameter3);
		}
		std::string text() const
		{
			return static_variant_cast<std::string>(iParameter1);
		}
	private:
		type_e iType;
		parameter_type iParameter1;
		parameter_type iParameter2;
		parameter_type iParameter3;
	};
}