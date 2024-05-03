// window_events.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neolib/core/variant.hpp>

#include <neogfx/core/geometrical.hpp>
#include <neogfx/hid/mouse.hpp>
#include <neogfx/hid/keyboard.hpp>

namespace neogfx
{
    enum class window_event_type
    {
        Paint,
        Close,
        Enabled,
        Disabled,
        Moving,
        Moved,
        Resizing,
        Resized,
        Maximized,
        Iconized,
        Restored,
        Enter,
        Leave,
        NonClientEnter,
        NonClientLeave,
        FocusGained,
        FocusLost,
        TitleTextChanged
    };

    class window_event
    {
    public:
        typedef window_event abstract_type; // todo
    public:
        typedef neolib::variant<neogfx::size, neogfx::point> parameter_type;
    public:
        window_event(window_event_type aType, const parameter_type& aParameter = parameter_type{}) :
            iType(aType), iParameter(aParameter)
        {
        }
    public:
        window_event_type type() const
        {
            return iType;
        }
        bool has_parameter() const
        {
            return !std::holds_alternative<std::monostate>(iParameter);
        }
        neogfx::size extents() const
        {
            return static_variant_cast<neogfx::size>(iParameter);
        }
        neogfx::point position() const
        {
            return static_variant_cast<neogfx::point>(iParameter);
        }
        void set_extents(const neogfx::size& aExtents)
        {
            static_variant_cast<neogfx::size&>(iParameter) = aExtents;
        }
        void set_position(const neogfx::point& aPosition)
        {
            static_variant_cast<neogfx::point&>(iParameter) = aPosition;
        }
    private:
        window_event_type iType;
        parameter_type iParameter;
    };

    enum class mouse_event_location
    {
        None,
        Client,
        NonClient
    };

    enum class mouse_event_type
    {
        WheelScrolled,
        ButtonClicked,
        ButtonDoubleClicked,
        ButtonReleased,
        Moved
    };

    template <mouse_event_location Location>
    class basic_mouse_event
    {
        typedef basic_mouse_event<Location> self_type;
    public:
        typedef self_type abstract_type; // todo
    public:
        typedef neolib::variant<neogfx::point, neogfx::delta, neogfx::mouse_button, neogfx::mouse_wheel, neogfx::key_modifiers_e> parameter_type;
    public:
        basic_mouse_event(
            mouse_event_type aType, 
            const parameter_type& aParameter1 = parameter_type{}, 
            const parameter_type& aParameter2 = parameter_type{}, 
            const parameter_type& aParameter3 = parameter_type{},
            const parameter_type& aParameter4 = parameter_type{}) :
            iType{ aType }, iParameter1{ aParameter1 }, iParameter2{ aParameter2 }, iParameter3{ aParameter3 }, iParameter4{ aParameter4 }
        {
        }
    public:
        mouse_event_type type() const
        {
            return iType;
        }
        neogfx::point position() const
        {
            return static_variant_cast<neogfx::point>(iParameter1);
        }
        self_type with_position(neogfx::point const& aPosition) const
        {
            return self_type{ iType, aPosition, iParameter2, iParameter3, iParameter4 };
        }
        neogfx::mouse_button mouse_button() const
        {
            return static_variant_cast<neogfx::mouse_button>(iParameter2);
        }
        neogfx::mouse_wheel mouse_wheel() const
        {
            return static_variant_cast<neogfx::mouse_wheel>(iParameter2);
        }
        neogfx::key_modifiers_e key_modifiers() const
        {
            return static_variant_cast<neogfx::key_modifiers_e>(iParameter3);
        }
        neogfx::delta wheel_delta() const
        {
            return static_variant_cast<neogfx::delta>(iParameter4);
        }
    public:
        bool is_left_button() const
        {
            return (mouse_button() & neogfx::mouse_button::Left) == neogfx::mouse_button::Left;
        }
        bool is_middle_button() const
        {
            return (mouse_button() & neogfx::mouse_button::Middle) == neogfx::mouse_button::Middle;
        }
        bool is_right_button() const
        {
            return (mouse_button() & neogfx::mouse_button::Right) == neogfx::mouse_button::Right;
        }
        bool is_x1_button() const
        {
            return (mouse_button() & neogfx::mouse_button::X1) == neogfx::mouse_button::X1;
        }
        bool is_x2_button() const
        {
            return (mouse_button() & neogfx::mouse_button::X2) == neogfx::mouse_button::X2;
        }
    private:
        mouse_event_type iType;
        parameter_type iParameter1;
        parameter_type iParameter2;
        parameter_type iParameter3;
        parameter_type iParameter4;
    };

    typedef basic_mouse_event<mouse_event_location::Client> mouse_event;
    typedef basic_mouse_event<mouse_event_location::NonClient> non_client_mouse_event;

    enum class keyboard_event_type
    {
        KeyPressed,
        KeyReleased,
        TextInput,
        SysTextInput
    };

    class keyboard_event
    {
    public:
        using abstract_type = keyboard_event ; // todo
    public:
        using parameter_type = neolib::variant<
            scan_code_e, 
            key_code_e, 
            key_modifiers_e, 
            string,
            native_scan_code_e,
            native_key_code_e>;
    public:
        keyboard_event(
            keyboard_event_type aType,
            const parameter_type& aScanCodeOrText = parameter_type{},
            const parameter_type& aKeyCode = parameter_type{},
            const parameter_type& aKeyModifiers = parameter_type{},
            const parameter_type& aNativeScanCode = parameter_type{},
            const parameter_type& aNativeKeyCode = parameter_type{}) :
            iType{ aType },
            iScanCodeOrText{ aScanCodeOrText },
            iKeyCode{ aKeyCode },
            iKeyModifiers{ aKeyModifiers },
            iNativeScanCode{ aNativeScanCode },
            iNativeKeyCode{ aNativeKeyCode }
        {
        }
    public:
        keyboard_event_type type() const
        {
            return iType;
        }
        scan_code_e scan_code() const
        {
            return static_variant_cast<scan_code_e>(iScanCodeOrText);
        }
        key_code_e key_code() const
        {
            return static_variant_cast<key_code_e>(iKeyCode);
        }
        key_modifiers_e key_modifiers() const
        {
            return static_variant_cast<key_modifiers_e>(iKeyModifiers);
        }
        std::string text() const
        {
            return static_variant_cast<string>(iScanCodeOrText);
        }
        native_scan_code_e native_scan_code() const
        {
            return static_variant_cast<native_scan_code_e>(iNativeScanCode);
        }
        native_key_code_e native_key_code() const
        {
            return static_variant_cast<native_key_code_e>(iNativeKeyCode);
        }
    private:
        keyboard_event_type iType;
        parameter_type iScanCodeOrText;
        parameter_type iKeyCode;
        parameter_type iKeyModifiers;
        parameter_type iNativeScanCode;
        parameter_type iNativeKeyCode;
    };
}