// native_window.cpp
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

#include <neogfx/neogfx.hpp>

#include <iterator>
#ifdef _WIN32
#include <D2d1.h>
#endif

#include <neolib/core/scoped.hpp>

#include <neogfx/app/i_app.hpp>
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include "native_window.hpp"

namespace neogfx
{
    native_window::native_window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aSurfaceWindow) :
        iRenderingEngine{ aRenderingEngine },
        iSurfaceManager{ aSurfaceManager },
        iSurfaceWindow{ aSurfaceWindow },
        iProcessingEvent{ 0u },
        iNonClientEntered{ false },
        iUpdater{ service<i_async_task>(), *this, [this](neolib::callback_timer& aTimer)
        {
            aTimer.again();

            if (non_client_entered() && 
                surface_window().native_window_hit_test(
                    surface_window().as_window().window_manager().mouse_position(surface_window().as_window())).part == widget_part::Nowhere)
            {
                auto e1 = find_event<window_event>(window_event_type::NonClientLeave);
                auto e2 = find_event<window_event>(window_event_type::NonClientEnter);
                if (e1 == iEventQueue.end() || (e2 != iEventQueue.end() && 
                    std::distance(iEventQueue.cbegin(), e1) < std::distance(iEventQueue.cbegin(), e2)))
                    push_event(window_event{ window_event_type::NonClientLeave });
            }

            if (alert_active())
                surface_window().as_window().update(true);
            else if (iAlert.has_value())
            {
                iAlert = std::nullopt;
                surface_window().as_window().update(true);
            }
        }, std::chrono::milliseconds{ 10 } },
        iInternalWindowActivation{ false },
        iEnteredWindow{ nullptr }
    {
        set_alive();
    }

    native_window::~native_window()
    {
    }

    bool native_window::attached() const
    {
        return iSurface != nullptr;
    }

    i_native_surface& native_window::attachment() const
    {
        if (iSurface == nullptr)
            throw window_not_attached_to_surface();

        return *iSurface;
    }

    void native_window::attach(i_native_surface& aSurface)
    {
        iSurface = aSurface;
        surface_window().as_window().set_surface(surface_window());
    }

    void native_window::detach()
    {
        iSurface = nullptr;
    }

    i_surface_window& native_window::surface_window() const
    {
        return iSurfaceWindow;
    }

    void native_window::display_error_message(std::string const& aTitle, std::string const& aMessage) const
    {
        surface_manager().display_error_message(*this, aTitle, aMessage);
    }

    bool native_window::events_queued() const
    {
        return !iEventQueue.empty();
    }

    void native_window::push_event(const native_event& aEvent)
    {
        if (std::holds_alternative<window_event>(aEvent))
        {
            auto const& windowEvent = static_variant_cast<const window_event&>(aEvent);
            switch (windowEvent.type())
            {
            case window_event_type::Moved:
            case window_event_type::Resized:
                for (auto e = iEventQueue.begin(); e != iEventQueue.end();)
                {
                    if (std::holds_alternative<window_event>(*e) && static_variant_cast<const window_event&>(*e).type() == windowEvent.type())
                        e = iEventQueue.erase(e);
                    else
                        ++e;
                }
                break;
            default:
                break;
            }
        }
        else if (std::holds_alternative<mouse_event>(aEvent))
        {
            auto const& mouseEvent = static_variant_cast<const mouse_event&>(aEvent);
            auto& mouse = service<i_mouse>();
            auto& surface = mouse.capturing() ? mouse.capture_target() :
                service<i_surface_manager>().surface_at_position(surface_window(), mouseEvent.position(), true);
            if (&surface != &surface_window())
            {
                surface.as_surface_window().native_window().push_event(mouseEvent);
                return;
            }
        }

        iEventQueue.push_back(aEvent);
    }

    bool native_window::pump_event()
    {
        neolib::scoped_counter<std::uint32_t> sc{ iProcessingEvent };
        if (iEventQueue.empty())
            return false;
        auto e = iEventQueue.front();
        iEventQueue.pop_front();
        handle_event(e);
        return true;
    }

    void native_window::handle_event(const native_event& aEvent)
    {
        destroyed_flag destroyed{ *this };
        neolib::scoped_counter<std::uint32_t> sc{ iProcessingEvent };
        native_event previousEvent;
        if (has_current_event())
            previousEvent = current_event();
        iCurrentEvent = aEvent;
        handle_event();
        if (!destroyed)
        {
            if (is_mouse_click_event(iCurrentEvent))
                iPreviousMouseClickEvent = iCurrentEvent;
            iCurrentEvent = previousEvent;
        }
        else
            sc.ignore();
    }

    bool native_window::has_current_event() const
    {
        return iCurrentEvent != neolib::none;
    }

    const native_window::native_event& native_window::current_event() const
    {
        if (has_current_event())
            return iCurrentEvent;
        throw no_current_event();
    }

    native_window::native_event& native_window::current_event()
    {
        if (has_current_event())
            return iCurrentEvent;
        throw no_current_event();
    }

    void native_window::handle_event()
    {
        destroyed_flag destroyed{ *this };
        neolib::scoped_counter<std::uint32_t> sc{ iProcessingEvent };
        if (event_consumed(Filter(current_event())))
        {
            if (destroyed)
                sc.ignore();
            return;
        }
        if (std::holds_alternative<window_event>(current_event()))
        {
            auto& windowEvent = static_variant_cast<window_event&>(current_event());
            if (event_consumed(surface_window().as_window().window_event()(windowEvent)))
                return;
            switch (windowEvent.type())
            {
            case window_event_type::Paint:
                render();
                break;
            case window_event_type::Close:
                close();
                break;
            case window_event_type::Resizing:
                surface_window().native_window_resizing();
                break;
            case window_event_type::Resized:
                surface_window().native_window_resized();
                break;
            case window_event_type::Moving:
                surface_window().native_window_moving();
                break;
            case window_event_type::Moved:
                surface_window().native_window_moved();
                break;
            case window_event_type::Enter:
                iNonClientEntered = false;
                surface_window().native_window_mouse_entered(windowEvent.position());
                break;
            case window_event_type::Leave:
                surface_window().native_window_mouse_left();
                break;
            case window_event_type::NonClientEnter:
                iNonClientEntered = true;
                surface_window().native_window_mouse_entered(windowEvent.position());
                break;
            case window_event_type::NonClientLeave:
                iNonClientEntered = false;
                surface_window().native_window_mouse_left();
                break;
            case window_event_type::FocusGained:
                {
                    neolib::scoped_flag sf{ iInternalWindowActivation };
                    if (windowEvent.has_parameter())
                    {
                        auto& surface = service<i_surface_manager>().surface_at_position(surface_window(), windowEvent.position());
                        if (surface.as_surface_window().as_window().enabled())
                        {
                            surface.as_surface_window().native_window_focus_gained();
                            break;
                        }
                    }
                    surface_window().native_window_focus_gained();
                }
                break;
            case window_event_type::FocusLost:
                if (service<i_window_manager>().window_activated() && &service<i_window_manager>().active_window().native_window() != this)
                {
                    auto& activeSurface = service<i_window_manager>().active_window().surface().as_surface_window();
                    if (surface_window().is_owner_of(activeSurface))
                    {
                        activeSurface.native_window_focus_lost();
                        break;
                    }
                }
                surface_window().native_window_focus_lost();
                break;
            case window_event_type::TitleTextChanged:
                surface_window().native_window_title_text_changed(title_text());
                break;
            default:
                /* do nothing */
                break;
            }
        }
        else if (std::holds_alternative<mouse_event>(current_event()))
        {
            auto& mouse = service<i_mouse>();
            if (!mouse.is_enabled())
                return;
            auto const& mouseEvent = static_variant_cast<const mouse_event&>(current_event());
            auto& surfaceWindow = mouse.capturing() ? mouse.capture_target().as_surface_window() :
                service<i_surface_manager>().surface_at_position(surface_window(), mouseEvent.position(), true).as_surface_window();
            if (&surfaceWindow != &surface_window())
            {
                if (mouseEvent.type() == mouse_event_type::Moved)
                    mouse_entered(surfaceWindow);
                surfaceWindow.native_window().handle_event(current_event());
                return;
            }
            auto activate_if = [&]()
            {
                if (!surfaceWindow.native_window().is_active())
                    surfaceWindow.native_window_focus_gained();
            };
            switch (mouseEvent.type())
            {
            case mouse_event_type::WheelScrolled:
                if (!mouse.grabber().mouse_wheel_scrolled(mouseEvent.mouse_wheel(), mouseEvent.position(), mouseEvent.wheel_delta(), mouseEvent.key_modifiers()))
                    surfaceWindow.native_window_mouse_wheel_scrolled(mouseEvent.mouse_wheel(), mouseEvent.position(), mouseEvent.wheel_delta(), mouseEvent.key_modifiers());
                break;
            case mouse_event_type::ButtonClicked:
                activate_if();
                surfaceWindow.native_window_mouse_button_clicked(mouseEvent.mouse_button(), mouseEvent.position(), mouseEvent.key_modifiers());
                break;
            case mouse_event_type::ButtonDoubleClicked:
                activate_if();
                surfaceWindow.native_window_mouse_button_double_clicked(mouseEvent.mouse_button(), mouseEvent.position(), mouseEvent.key_modifiers());
                break;
            case mouse_event_type::ButtonClick:
                surfaceWindow.native_window_mouse_button_click(mouseEvent.mouse_button(), mouseEvent.position(), mouseEvent.key_modifiers());
                break;
            case mouse_event_type::ButtonDoubleClick:
                surfaceWindow.native_window_mouse_button_double_click(mouseEvent.mouse_button(), mouseEvent.position(), mouseEvent.key_modifiers());
                break;
            case mouse_event_type::ButtonReleased:
                surfaceWindow.native_window_mouse_button_released(mouseEvent.mouse_button(), mouseEvent.position());
                break;
            case mouse_event_type::Moved:
                mouse_entered(surfaceWindow);
                surfaceWindow.native_window_mouse_moved(mouseEvent.position(), mouseEvent.key_modifiers());
                if (is_capturing() && !is_non_client_capturing())
                    service<i_window_manager>().update_mouse_cursor(surface_window().as_window());
                break;
            default:
                /* do nothing */
                break;
            }
        }
        else if (std::holds_alternative<non_client_mouse_event>(current_event()))
        {
            auto& mouse = service<i_mouse>();
            if (!mouse.is_enabled())
                return;
            auto const& mouseEvent = static_variant_cast<const non_client_mouse_event&>(current_event());
            auto& surfaceWindow = mouse.capturing() ? mouse.capture_target().as_surface_window() :
                service<i_surface_manager>().surface_at_position(surface_window(), mouseEvent.position(), true).as_surface_window();
            if (&surfaceWindow != &surface_window())
            {
                if (mouseEvent.type() == mouse_event_type::Moved)
                    mouse_entered(surfaceWindow);
                surfaceWindow.native_window().handle_event(current_event());
                return;
            }
            auto activate_if = [&]()
            {
                if (!surfaceWindow.native_window().is_active())
                    surfaceWindow.native_window().activate();
            };
            switch (mouseEvent.type())
            {
            case mouse_event_type::WheelScrolled:
                if (!mouse.grabber().mouse_wheel_scrolled(mouseEvent.mouse_wheel(), mouseEvent.position(), mouseEvent.wheel_delta(), mouseEvent.key_modifiers()))
                    surfaceWindow.native_window_non_client_mouse_wheel_scrolled(mouseEvent.mouse_wheel(), mouseEvent.position(), mouseEvent.wheel_delta(), mouseEvent.key_modifiers());
                break;
            case mouse_event_type::ButtonClicked:
                activate_if();
                surfaceWindow.native_window_non_client_mouse_button_clicked(mouseEvent.mouse_button(), mouseEvent.position(), mouseEvent.key_modifiers());
                break;
            case mouse_event_type::ButtonDoubleClicked:
                activate_if();
                surfaceWindow.native_window_non_client_mouse_button_double_clicked(mouseEvent.mouse_button(), mouseEvent.position(), mouseEvent.key_modifiers());
                break;
            case mouse_event_type::ButtonClick:
                surfaceWindow.native_window_non_client_mouse_button_click(mouseEvent.mouse_button(), mouseEvent.position(), mouseEvent.key_modifiers());
                break;
            case mouse_event_type::ButtonDoubleClick:
                surfaceWindow.native_window_non_client_mouse_button_double_click(mouseEvent.mouse_button(), mouseEvent.position(), mouseEvent.key_modifiers());
                break;
            case mouse_event_type::ButtonReleased:
                surfaceWindow.native_window_non_client_mouse_button_released(mouseEvent.mouse_button(), mouseEvent.position());
                break;
            case mouse_event_type::Moved:
                mouse_entered(surfaceWindow);
                surfaceWindow.native_window_non_client_mouse_moved(mouseEvent.position(), mouseEvent.key_modifiers());
                if (is_capturing() && !is_non_client_capturing())
                    service<i_window_manager>().update_mouse_cursor(surface_window().as_window());
                break;
            default:
                /* do nothing */
                break;
            }
        }
        else if (std::holds_alternative<keyboard_event>(current_event()))
        {
            auto& keyboard = service<i_keyboard>();
            if (!keyboard.is_enabled())
                return;
            auto const& keyboardEvent = static_variant_cast<const keyboard_event&>(current_event());
            if (service<i_window_manager>().window_activated())
            {
                auto& activeSurfaceWindow = service<i_window_manager>().active_window().surface().as_surface_window();
                if (&activeSurfaceWindow != &surface_window())
                {
                    activeSurfaceWindow.native_window().handle_event(current_event());
                    return;
                }
            }
            switch (keyboardEvent.type())
            {
            case keyboard_event_type::KeyPressed:
                keyboard.set_event_modifiers(keyboardEvent.key_modifiers());
                if (!keyboard.grabber().key_pressed(keyboardEvent.scan_code(), keyboardEvent.key_code(), keyboardEvent.key_modifiers()))
                {
                    keyboard.key_pressed()(keyboardEvent.scan_code(), keyboardEvent.key_code(), keyboardEvent.key_modifiers());
                    surface_window().native_window_key_pressed(keyboardEvent.scan_code(), keyboardEvent.key_code(), keyboardEvent.key_modifiers());
                }
                keyboard.clear_event_modifiers();
                break;
            case keyboard_event_type::KeyReleased:
                keyboard.set_event_modifiers(keyboardEvent.key_modifiers());
                if (!keyboard.grabber().key_released(keyboardEvent.scan_code(), keyboardEvent.key_code(), keyboardEvent.key_modifiers()))
                {
                    keyboard.key_released()(keyboardEvent.scan_code(), keyboardEvent.key_code(), keyboardEvent.key_modifiers());
                    surface_window().native_window_key_released(keyboardEvent.scan_code(), keyboardEvent.key_code(), keyboardEvent.key_modifiers());
                }
                keyboard.clear_event_modifiers();
                break;
            case keyboard_event_type::TextInput:
                keyboard.set_event_modifiers(keyboardEvent.key_modifiers());
                if (!keyboard.grabber().text_input(string{ keyboardEvent.text() }))
                {
                    keyboard.text_input()(keyboardEvent.text());
                    surface_window().native_window_text_input(string{ keyboardEvent.text() });
                }
                keyboard.clear_event_modifiers();
                break;
            case keyboard_event_type::SysTextInput:
                keyboard.set_event_modifiers(keyboardEvent.key_modifiers());
                if (!keyboard.grabber().sys_text_input(string{ keyboardEvent.text() }))
                {
                    keyboard.sys_text_input()(keyboardEvent.text());
                    surface_window().native_window_sys_text_input(string{ keyboardEvent.text() });
                }
                keyboard.clear_event_modifiers();
                break;
            default:
                /* do nothing */
                break;
            }
        }
        if (destroyed)
            sc.ignore();
    }

    const i_widget& native_window::widget_for_mouse_event(const point& aPosition, bool aForHitTest) const
    {
        auto& mouse = service<i_mouse>();
        auto& surfaceWindow = mouse.capturing() ? mouse.capture_target().as_surface_window() :
            service<i_surface_manager>().surface_at_position(surface_window(), aPosition, true).as_surface_window();
        return surfaceWindow.widget_for_mouse_event(aPosition, aForHitTest);
    }

    i_widget& native_window::widget_for_mouse_event(const point& aPosition, bool aForHitTest)
    {
        return const_cast<i_widget&>(to_const(*this).widget_for_mouse_event(aPosition, aForHitTest));
    }

    bool native_window::processing_event() const
    {
        return iProcessingEvent != 0;
    }

    bool native_window::event_cause_external() const
    {
        if (has_current_event() && std::holds_alternative<window_event>(current_event()))
            return static_variant_cast<window_event const&>(current_event()).cause_external();
        return false;
    }

    i_string const& native_window::title_text() const
    {
        return iTitleText;
    }

    void native_window::set_title_text(i_string const& aTitleText)
    {
        iTitleText = aTitleText;
    }

    namespace
    {
#ifdef _WIN32
        std::chrono::milliseconds default_blink_time()
        {
            return std::chrono::milliseconds{ GetCaretBlinkTime() };
        }
#else
        std::chrono::milliseconds default_blink_time()
        {
            return std::chrono::milliseconds{ 500 };
        }
#endif
    }

    bool native_window::alert_active() const
    {
        if (!iAlert.has_value())
            return false;
        auto const& alertInfo = iAlert.value();
        if ((alertInfo.alert & window_alert::NoForeground) == window_alert::NoForeground && is_active())
            return false;
        auto const flashInterval_ms = alertInfo.interval.value_or(default_blink_time()) * 2;
        return (!alertInfo.count.has_value() ||
            std::chrono::steady_clock::now() >= alertInfo.startTime + flashInterval_ms * alertInfo.count.value());
    }

    scalar native_window::alert_easing() const
    {
        if (!iAlert.has_value())
            return 0.0;
        auto const& alertInfo = iAlert.value();
        auto const since = std::chrono::steady_clock::now() - alertInfo.startTime;
        auto const flashInterval_ms = alertInfo.interval.value_or(default_blink_time()).count() * 2;
        auto const normalizedFrameTime = ((std::chrono::duration_cast<std::chrono::milliseconds>(since).count()) % flashInterval_ms) / ((flashInterval_ms - 1) * 1.0);
        return partitioned_ease(easing::InvertedInOutQuint, easing::InOutQuint, normalizedFrameTime);
    }

    void native_window::alert(window_alert aAlert, std::optional<std::chrono::milliseconds> const& aInterval, std::optional<std::uint32_t> const& aCount)
    {
        if (aAlert != window_alert::None)
            iAlert.emplace(aAlert, aInterval, aCount, std::chrono::steady_clock::now());
        else
        {
            iAlert = std::nullopt;
            surface_window().as_window().update(true);
        }
    }

    i_rendering_engine& native_window::rendering_engine() const
    {
        return iRenderingEngine;
    }

    i_surface_manager& native_window::surface_manager() const
    {
        return iSurfaceManager;
    }

    bool native_window::non_client_entered() const
    {
        return iNonClientEntered;
    }

    size& native_window::pixel_density() const
    {
        if (iPixelDensityDpi == std::nullopt)
        {
            auto& display = surface_manager().display(surface_window());
            iPixelDensityDpi = size{ display.metrics().horizontal_dpi(), display.metrics().vertical_dpi() };
        }
        return *iPixelDensityDpi;
    }

    void native_window::handle_dpi_changed()
    {
        surface_manager().display(surface_window()).update_dpi();
        iPixelDensityDpi = std::nullopt;
        surface_window().handle_dpi_changed();
        surface_manager().dpi_changed()(surface_window());
    }

    bool native_window::internal_window_activation() const
    {
        return iInternalWindowActivation;
    }

    void native_window::mouse_entered(i_surface_window& aWindow)
    {
        if (iEnteredWindow == &aWindow)
            return;
        if (iEnteredWindow && iEnteredWindow != &surface_window())
            iEnteredWindow->as_window().mouse_left();
        if (&aWindow != &surface_window())
        {
            iEnteredWindow = &aWindow;
            iEnteredWindowEventSink = iEnteredWindow->closed([&]() { iEnteredWindow = nullptr; });
        }
        else
        {
            iEnteredWindow = nullptr;
            iEnteredWindowEventSink.clear();
        }
    }

    bool native_window::is_mouse_click_event(const native_event& aEvent) const
    {
        return
            (std::holds_alternative<mouse_event>(aEvent) &&
                static_variant_cast<mouse_event const&>(aEvent).type() != mouse_event_type::Moved &&
                static_variant_cast<mouse_event const&>(aEvent).type() != mouse_event_type::WheelScrolled) ||
            (std::holds_alternative<non_client_mouse_event>(aEvent) &&
                static_variant_cast<non_client_mouse_event const&>(aEvent).type() != mouse_event_type::Moved &&
                static_variant_cast<non_client_mouse_event const&>(aEvent).type() != mouse_event_type::WheelScrolled);
    }

    bool native_window::has_previous_mouse_click_event() const
    {
        return !std::holds_alternative<std::monostate>(previous_mouse_click_event());
    }

    const native_window::native_event& native_window::previous_mouse_click_event() const
    {
        return iPreviousMouseClickEvent;
    }

    native_window::native_event& native_window::previous_mouse_click_event()
    {
        return iPreviousMouseClickEvent;
    }
}