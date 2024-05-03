/*
  web_view_canvas.hpp

  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.

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

#include <cef/include/views/cef_window.h>

#include <neogfx/gui/window/i_native_window.hpp>

#include "web_view_canvas.hpp"

namespace neogfx
{
    web_view_canvas::web_view_canvas(i_widget& aParent, i_optional<i_string> const& aUrl) :
        base_type{ aParent },
        iUrl{ aUrl.has_value() ? decltype(iUrl){ aUrl.value().to_std_string_view() } : std::nullopt }
    {
        init();
    }

    web_view_canvas::web_view_canvas(i_layout& aLayout, i_optional<i_string> const& aUrl) :
        base_type{ aLayout },
        iUrl{ aUrl.has_value() ? decltype(iUrl){ aUrl.value().to_std_string_view() } : std::nullopt }
    {
        init();
    }

    web_view_canvas::~web_view_canvas()
    {
    }

    logical_coordinate_system web_view_canvas::logical_coordinate_system() const
    {
        return neogfx::logical_coordinate_system::AutomaticGame;
    }

    void web_view_canvas::resized()
    {
        base_type::resized();

        iBrowser->GetHost()->WasResized();
    }

    void web_view_canvas::paint(i_graphics_context& aGc) const
    {
        base_type::paint(aGc);

        auto const clientRect = client_rect(false);
        aGc.draw_texture(clientRect, back_buffer(), rect{ point{}, clientRect.extents() });
    }

    namespace
    {
        std::uint32_t convert_key_modifiers(key_modifiers_e aKeyModifiers, mouse_button aMouseButton)
        {
            cef_event_flags_t result = EVENTFLAG_NONE;
            if (aKeyModifiers & KeyModifier_LSHIFT) result = static_cast<cef_event_flags_t>(result | EVENTFLAG_SHIFT_DOWN);
            if (aKeyModifiers & KeyModifier_RSHIFT) result = static_cast<cef_event_flags_t>(result | EVENTFLAG_SHIFT_DOWN);
            if (aKeyModifiers & KeyModifier_LCTRL) result = static_cast<cef_event_flags_t>(result | EVENTFLAG_CONTROL_DOWN);
            if (aKeyModifiers & KeyModifier_RCTRL) result = static_cast<cef_event_flags_t>(result | EVENTFLAG_CONTROL_DOWN);
            if (aKeyModifiers & KeyModifier_LALT) result = static_cast<cef_event_flags_t>(result | EVENTFLAG_ALT_DOWN);
            if (aKeyModifiers & KeyModifier_RALT) result = static_cast<cef_event_flags_t>(result | EVENTFLAG_ALTGR_DOWN);
            if (aKeyModifiers & KeyModifier_NUM) result = static_cast<cef_event_flags_t>(result | EVENTFLAG_NUM_LOCK_ON);
            if (aKeyModifiers & KeyModifier_CAPS) result = static_cast<cef_event_flags_t>(result | EVENTFLAG_CAPS_LOCK_ON);
            if ((aMouseButton & mouse_button::Left) == mouse_button::Left) 
                result = static_cast<cef_event_flags_t>(result | EVENTFLAG_LEFT_MOUSE_BUTTON);
            if ((aMouseButton & mouse_button::Middle) == mouse_button::Middle)
                result = static_cast<cef_event_flags_t>(result | EVENTFLAG_MIDDLE_MOUSE_BUTTON);
            if ((aMouseButton & mouse_button::Right) == mouse_button::Right)
                result = static_cast<cef_event_flags_t>(result | EVENTFLAG_RIGHT_MOUSE_BUTTON);
            return static_cast<std::uint32_t>(result);
        }

        cef_mouse_button_type_t convert_mouse_button(mouse_button aButton)
        {
            switch (aButton)
            {
            case mouse_button::Left:
                return MBT_LEFT;
            case mouse_button::Middle:
                return MBT_MIDDLE;
            case mouse_button::Right:
                return MBT_RIGHT;
            default:
                return MBT_LEFT;
            }
        }

        basic_point<int> to_cef_mouse_position(web_view_canvas const& aCanvas, point const& aMousePosition)
        {
            scoped_units_context suc{ aCanvas };
            return basic_point<int>{ aMousePosition / 1.0_dip };
        }
    }

    bool web_view_canvas::mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifiers_e aKeyModifiers)
    {
        auto const& mousePosition = to_cef_mouse_position(*this, aPosition);
        CefMouseEvent cefEvent{ cef_mouse_event_t{ mousePosition.x, mousePosition.y, 
            convert_key_modifiers(aKeyModifiers, mouse_button::None) } };
        iBrowser->GetHost()->SendMouseWheelEvent(cefEvent, static_cast<int>(aDelta.dx), static_cast<int>(aDelta.dy));
        return true;
    }

    void web_view_canvas::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        base_type::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
        auto const& mousePosition = to_cef_mouse_position(*this, aPosition);
        CefMouseEvent cefEvent{ cef_mouse_event_t{ mousePosition.x, mousePosition.y, 
            convert_key_modifiers(aKeyModifiers, aButton) } };
        iBrowser->GetHost()->SendMouseClickEvent(cefEvent, convert_mouse_button(aButton), false, 1);
        iLastClickCount = 1u;
    }

    void web_view_canvas::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        base_type::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
        auto const& mousePosition = to_cef_mouse_position(*this, aPosition);
        CefMouseEvent cefEvent{ cef_mouse_event_t{ mousePosition.x, mousePosition.y, 
            convert_key_modifiers(aKeyModifiers, aButton) } };
        iBrowser->GetHost()->SendMouseClickEvent(cefEvent, convert_mouse_button(aButton), false, 2);
        iLastClickCount = 2u;
    }

    void web_view_canvas::mouse_button_released(mouse_button aButton, const point& aPosition)
    {
        base_type::mouse_button_released(aButton, aPosition);
        auto const& mousePosition = to_cef_mouse_position(*this, aPosition);
        CefMouseEvent cefEvent{ cef_mouse_event_t{ mousePosition.x, mousePosition.y, 
            convert_key_modifiers(service<i_keyboard>().modifiers(), service<i_mouse>().button_state()) } };
        iBrowser->GetHost()->SendMouseClickEvent(cefEvent, convert_mouse_button(aButton), true, static_cast<int>(iLastClickCount));
    }

    void web_view_canvas::mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers)
    {
        base_type::mouse_moved(aPosition, aKeyModifiers);
        auto const& mousePosition = to_cef_mouse_position(*this, aPosition);
        CefMouseEvent cefEvent{ cef_mouse_event_t{ mousePosition.x, mousePosition.y, 
            convert_key_modifiers(aKeyModifiers, service<i_mouse>().button_state()) } };
        iBrowser->GetHost()->SendMouseMoveEvent(cefEvent, false);
    }

    void web_view_canvas::mouse_entered(const point& aPosition)
    {
        base_type::mouse_entered(aPosition);
        auto const& mousePosition = to_cef_mouse_position(*this, aPosition);
        CefMouseEvent cefEvent{ cef_mouse_event_t{ mousePosition.x, mousePosition.y, 
            convert_key_modifiers(service<i_keyboard>().modifiers(), service<i_mouse>().button_state()) } };
        iBrowser->GetHost()->SendMouseMoveEvent(cefEvent, false);
    }

    void web_view_canvas::mouse_left()
    {
        base_type::mouse_left();
        auto const& mousePosition = to_cef_mouse_position(*this, mouse_position());
        CefMouseEvent cefEvent{ cef_mouse_event_t{ mousePosition.x, mousePosition.y,
            convert_key_modifiers(service<i_keyboard>().modifiers(), service<i_mouse>().button_state()) } };
        iBrowser->GetHost()->SendMouseMoveEvent(cefEvent, true);
    }

    mouse_cursor web_view_canvas::mouse_cursor() const
    {
        switch (iCursorType)
        {
        case CT_POINTER:
            return mouse_system_cursor::Arrow;
        case CT_CROSS:
            return mouse_system_cursor::Crosshair;
        case CT_HAND:
            return mouse_system_cursor::Hand;
        case CT_IBEAM:
            return mouse_system_cursor::IBeam;
        case CT_WAIT:
            return mouse_system_cursor::Wait;
        case CT_HELP:
            return mouse_system_cursor::Arrow;
        case CT_EASTRESIZE:
            return mouse_system_cursor::SizeWE;
        case CT_NORTHRESIZE:
            return mouse_system_cursor::SizeNS;
        case CT_NORTHEASTRESIZE:
            return mouse_system_cursor::SizeNESW;
        case CT_NORTHWESTRESIZE:
            return mouse_system_cursor::SizeNWSE;
        case CT_SOUTHRESIZE:
            return mouse_system_cursor::SizeNS;
        case CT_SOUTHEASTRESIZE:
            return mouse_system_cursor::SizeNWSE;
        case CT_SOUTHWESTRESIZE:
            return mouse_system_cursor::SizeNESW;
        case CT_WESTRESIZE:
            return mouse_system_cursor::SizeWE;
        case CT_NORTHSOUTHRESIZE:
            return mouse_system_cursor::SizeNS;
        case CT_EASTWESTRESIZE:
            return mouse_system_cursor::SizeWE;
        case CT_NORTHEASTSOUTHWESTRESIZE:
            return mouse_system_cursor::SizeNESW;
        case CT_NORTHWESTSOUTHEASTRESIZE:
            return mouse_system_cursor::SizeNWSE;
        case CT_COLUMNRESIZE:
        case CT_ROWRESIZE:
        case CT_MIDDLEPANNING:
        case CT_EASTPANNING:
        case CT_NORTHPANNING:
        case CT_NORTHEASTPANNING:
        case CT_NORTHWESTPANNING:
        case CT_SOUTHPANNING:
        case CT_SOUTHEASTPANNING:
        case CT_SOUTHWESTPANNING:
        case CT_WESTPANNING:
        case CT_MOVE:
        case CT_VERTICALTEXT:
        case CT_CELL:
        case CT_CONTEXTMENU:
        case CT_ALIAS:
        case CT_PROGRESS:
        case CT_NODROP:
        case CT_COPY:
        case CT_NONE:
        case CT_NOTALLOWED:
        case CT_ZOOMIN:
        case CT_ZOOMOUT:
        case CT_GRAB:
        case CT_GRABBING:
        case CT_MIDDLE_PANNING_VERTICAL:
        case CT_MIDDLE_PANNING_HORIZONTAL:
        case CT_CUSTOM:
        case CT_DND_NONE:
        case CT_DND_MOVE:
        case CT_DND_COPY:
        case CT_DND_LINK:
        default:
            return mouse_system_cursor::Arrow;
        }
    }

    focus_policy web_view_canvas::focus_policy() const
    {
        if (has_focus_policy())
            return base_type::focus_policy();
        return neogfx::focus_policy::StrongFocus;
    }

    bool web_view_canvas::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        //CefKeyEvent cefEvent{};
        //iBrowser->GetHost()->SendKeyEvent(cefEvent);
        return true;
    }

    bool web_view_canvas::key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        //CefKeyEvent cefEvent{};
        //iBrowser->GetHost()->SendKeyEvent(cefEvent);
        return true;
    }

    bool web_view_canvas::text_input(i_string const& aText)
    {
        //CefKeyEvent cefEvent{};
        //iBrowser->GetHost()->SendKeyEvent(cefEvent);
        return true;
    }

    bool web_view_canvas::sys_text_input(i_string const& aText)
    {
        //CefKeyEvent cefEvent{};
        //iBrowser->GetHost()->SendKeyEvent(cefEvent);
        return true;
    }

    void web_view_canvas::load_url(i_string const& aUrl)
    {
        iUrl = aUrl.to_std_string_view();
        iBrowser->GetMainFrame()->LoadURL(CefString{ aUrl.to_std_string() });
    }

    void web_view_canvas::init()
    {
        set_margin(neogfx::margin{});
        set_padding(neogfx::padding{});

        CefWindowInfo window_info;
#ifdef _WIN32
        window_info.SetAsWindowless(static_cast<HWND>(root().native_window().native_handle()));
#else
        window_info.SetAsWindowless(nullptr);
#endif

        CefBrowserSettings browser_settings;
        browser_settings.windowless_frame_rate = 60;

        iBrowser = CefBrowserHost::CreateBrowserSync(window_info, this, !iUrl ? CefString{} : CefString{ iUrl.value() }, browser_settings, nullptr, nullptr);
    }

    i_texture& web_view_canvas::back_buffer() const
    {
        scoped_units_context suc{ *this };
        auto const clientRect = client_rect(false);
        auto const desiredBackBufferExtents = dpi_scale(clientRect.extents());
        auto const currentBackBufferExtents = iBackBuffer ? iBackBuffer->extents() : size{};
        if (!iBackBuffer || 
            currentBackBufferExtents.cx < desiredBackBufferExtents.cx || 
            currentBackBufferExtents.cy < desiredBackBufferExtents.cy)
            iBackBuffer = service<i_texture_manager>().create_texture(
                desiredBackBufferExtents.max(currentBackBufferExtents), 1.0, texture_sampling::Normal, texture_data_format::BGRA);
        return *iBackBuffer;
    }

    CefRefPtr<CefDisplayHandler> web_view_canvas::GetDisplayHandler()
    {
        return this;
    }

    CefRefPtr<CefRenderHandler> web_view_canvas::GetRenderHandler()
    {
        return this;
    }

    bool web_view_canvas::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info)
    {
        scoped_units_context suc{ *this };
        CefRect view_rect;
        GetViewRect(browser, view_rect);
        screen_info.device_scale_factor = static_cast<float>(1.0_dip);
        screen_info.rect = view_rect;
        screen_info.available_rect = view_rect;        
        return true;
    }

    void web_view_canvas::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
    {
        scoped_units_context suc{ *this };
        auto const clientRect = client_rect(false);
        basic_size<int> const viewExtents = (clientRect.extents() / dpi_scale(1.0f)).ceil();
        rect = CefRect(0, 0, viewExtents.cx, viewExtents.cy);
    }

    void web_view_canvas::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height)
    {
        for (auto const& dirtyRect : dirtyRects)
        {
            rect const updateRect{ basic_point<int>{ dirtyRect.x, dirtyRect.y }, basic_size<int>{ dirtyRect.width, dirtyRect.height } };
            back_buffer().set_pixels(
                updateRect, 
                static_cast<std::byte const*>(buffer) + 4 * (width * dirtyRect.y + dirtyRect.x), width);
            update(updateRect);
        }
    }

    bool web_view_canvas::OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, cef_cursor_type_t type, const CefCursorInfo& custom_cursor_info)
    {
        iCursorType = type;
        return true;
    }
}
