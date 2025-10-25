// i_surface_window.hpp
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

#include <neogfx/hid/i_surface.hpp>
#include <neogfx/hid/i_keyboard.hpp>
#include <neogfx/gui/widget/widget_bits.hpp>
#include <neogfx/gui/window/window_events.hpp>

namespace neogfx
{
    class i_native_window;
    class i_window;
    class i_widget;

    class i_surface_window : public i_surface
    {
    public:
        typedef i_surface_window abstract_type;
    public:
        struct no_native_window : std::logic_error { no_native_window() : std::logic_error("neogfx::i_surface::no_native_window") {} };
        struct widget_not_clicked : std::logic_error { widget_not_clicked() : std::logic_error("neogfx::i_surface_window::widget_not_clicked") {} };
        struct widget_not_capturing : std::logic_error { widget_not_capturing() : std::logic_error("neogfx::i_surface_window::widget_not_capturing") {} };
    public:
        virtual bool has_native_window() const = 0;
        virtual const i_native_window& native_window() const = 0;
        virtual i_native_window& native_window() = 0;
        virtual void set_native_window(i_native_window& aNativeWindow) = 0;
    public:
        virtual void handle_dpi_changed() = 0;
    public:
        virtual bool has_clicked_widget() const = 0;
        virtual i_widget& clicked_widget() const = 0;
        virtual bool has_capturing_widget() const = 0;
        virtual i_widget& capturing_widget() const = 0;
        virtual void set_capture(i_widget& aWidget) = 0;
        virtual void release_capture(i_widget& aWidget) = 0;
        virtual void non_client_set_capture(i_widget& aWidget) = 0;
        virtual void non_client_release_capture(i_widget& aWidget) = 0;
    public:
        virtual mouse_event_location current_mouse_event_location() const = 0;
        virtual bool is_closing() const = 0;
        virtual bool native_window_can_close() const = 0;
        virtual void native_window_closing() = 0;
        virtual void native_window_closed() = 0;
        virtual void native_window_focus_gained() = 0;
        virtual void native_window_focus_lost() = 0;
        virtual void native_window_resizing() = 0;
        virtual void native_window_resized() = 0;
        virtual void native_window_moving() = 0;
        virtual void native_window_moved() = 0;
        virtual double native_window_rendering_priority() const = 0;
        virtual bool native_window_ready_to_render() const = 0;
        virtual void native_window_render(const rect& aInvalidatedRect) const = 0;
        virtual void native_window_dismiss_children() = 0;
        virtual void native_window_mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifiers_e aKeyModifiers) = 0;
        virtual void native_window_mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
        virtual void native_window_mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
        virtual void native_window_mouse_button_click(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
        virtual void native_window_mouse_button_double_click(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
        virtual void native_window_mouse_button_released(mouse_button aButton, const point& aPosition) = 0;
        virtual void native_window_mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
        virtual void native_window_non_client_mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifiers_e aKeyModifiers) = 0;
        virtual void native_window_non_client_mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
        virtual void native_window_non_client_mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
        virtual void native_window_non_client_mouse_button_click(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
        virtual void native_window_non_client_mouse_button_double_click(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
        virtual void native_window_non_client_mouse_button_released(mouse_button aButton, const point& aPosition) = 0;
        virtual void native_window_non_client_mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
        virtual void native_window_mouse_entered(const point& aPosition) = 0;
        virtual void native_window_mouse_left() = 0;
        virtual widget_part native_window_hit_test(const point& aPosition) const = 0;
        virtual rect native_window_widget_part_rect(widget_part_e aWidgetPart) const = 0;
        virtual void native_window_key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) = 0;
        virtual void native_window_key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) = 0;
        virtual void native_window_text_input(i_string const& aText) = 0;
        virtual void native_window_sys_text_input(i_string const& aText) = 0;
        virtual neogfx::mouse_cursor native_window_mouse_cursor() const = 0;
        virtual void native_window_title_text_changed(i_string const& aTitleText) = 0;
    public:
        virtual window_style style() const = 0;
        virtual void set_style(window_style aStyle) = 0;
    public:
        virtual const i_window& as_window() const = 0;
        virtual i_window& as_window() = 0;
        virtual const i_widget& as_widget() const = 0;
        virtual i_widget& as_widget() = 0;
    public:
        virtual const i_window& window_at_position(const point& aPosition, bool aForMouseEvent = false) const = 0;
        virtual i_window& window_at_position(const point& aPosition, bool aForMouseEvent = false) = 0;
        virtual const i_widget& widget_for_mouse_event(const point& aPosition, bool aForHitTest = false) const = 0;
        virtual i_widget& widget_for_mouse_event(const point& aPosition, bool aForHitTest = false) = 0;
    };
}