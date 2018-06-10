// surface_window_proxy.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/raii.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/hid/surface_window_proxy.hpp>
#include "../gui/window/native/i_native_window.hpp"
#include "native/i_native_surface.hpp"

namespace neogfx
{
	surface_window_proxy::surface_window_proxy(i_window& aWindow, std::function<std::unique_ptr<i_native_window>(i_surface_window&)> aNativeWindowCreator) :
		iWindow{ aWindow }, 
		iRenderingEngine{ app::instance().rendering_engine() },
		iNativeWindow{ aNativeWindowCreator(*this) },
		iNativeSurfaceDestroyed{ iNativeWindow->as_lifetime() },
		iClosed{ false },
		iCapturingWidget{ nullptr },
		iClickedWidget{ nullptr }
	{
		app::instance().surface_manager().add_surface(*this);
	}

	surface_window_proxy::~surface_window_proxy()
	{
		close();
	}

	window_style surface_window_proxy::style() const
	{
		return as_window().style();
	}

	void surface_window_proxy::set_style(window_style aStyle)
	{
		as_window().set_style(aStyle);
	}

	dimension surface_window_proxy::horizontal_dpi() const
	{
		return native_window().horizontal_dpi();
	}

	dimension surface_window_proxy::vertical_dpi() const
	{
		return native_window().vertical_dpi();
	}

	dimension surface_window_proxy::ppi() const
	{
		return native_window().ppi();
	}

	bool surface_window_proxy::metrics_available() const
	{
		return native_window().metrics_available();
	}

	size surface_window_proxy::extents() const
	{
		return native_window().extents();
	}

	dimension surface_window_proxy::em_size() const
	{
		return 0; /* todo */
	}

	i_rendering_engine& surface_window_proxy::rendering_engine() const
	{
		return iRenderingEngine;
	}

	bool surface_window_proxy::is_strong() const
	{
		return as_window().is_strong();
	}

	bool surface_window_proxy::is_weak() const
	{
		return as_window().is_weak();
	}

	bool surface_window_proxy::can_close() const
	{
		return as_window().can_close();
	}

	bool surface_window_proxy::is_closed() const
	{
		return iClosed;
	}

	void surface_window_proxy::close()
	{
		if (iClosed)
			return;
		destroyed_flag destroyed{ *this };
		if (has_native_surface())
			native_surface().close();
		else
		{
			auto ptr = std::move(iNativeWindow);
		}
		if (destroyed)
			return;
		iClosed = true;
		closed.trigger();
		if (!destroyed)
			set_destroyed();
	}

	bool surface_window_proxy::has_parent_surface() const
	{
		return native_window().has_parent();
	}

	const i_surface& surface_window_proxy::parent_surface() const
	{
		return native_window().parent().surface_window();
	}
	
	i_surface& surface_window_proxy::parent_surface()
	{
		return const_cast<i_surface&>(const_cast<const surface_window_proxy*>(this)->parent_surface());
	}

	bool surface_window_proxy::is_owner_of(const i_surface& aChildSurface) const
	{
		const i_surface* s = &aChildSurface;
		if (s == this)
			return false;
		while (s->has_parent_surface())
		{
			s = &s->parent_surface();
			if (s == this)
				return true;
		}
		return false;
	}

	bool surface_window_proxy::is_window() const
	{
		return true;
	}

	const i_surface_window& surface_window_proxy::as_surface_window() const
	{
		return *this;
	}

	i_surface_window& surface_window_proxy::as_surface_window()
	{
		return *this;
	}

	surface_type surface_window_proxy::surface_type() const
	{
		return neogfx::surface_type::Window;
	}

	neogfx::logical_coordinate_system surface_window_proxy::logical_coordinate_system() const
	{
		return native_surface().logical_coordinate_system();
	}

	void surface_window_proxy::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem)
	{
		native_surface().set_logical_coordinate_system(aSystem);
	}

	const std::pair<vec2, vec2>& surface_window_proxy::logical_coordinates() const
	{
		return native_surface().logical_coordinates();
	}

	void surface_window_proxy::set_logical_coordinates(const std::pair<vec2, vec2>& aCoordinates)
	{
		native_surface().set_logical_coordinates(aCoordinates);
	}

	void surface_window_proxy::layout_surface()
	{
		as_widget().layout_items();
	}

	void surface_window_proxy::invalidate_surface(const rect& aInvalidatedRect, bool aInternal)
	{
		native_surface().invalidate(aInvalidatedRect);
		if (!aInternal)
			as_widget().update(aInvalidatedRect);
	}

	bool surface_window_proxy::has_invalidated_area() const
	{
		return native_surface().has_invalidated_area();
	}

	const rect& surface_window_proxy::invalidated_area() const
	{
		return native_surface().invalidated_area();
	}

	rect surface_window_proxy::validate()
	{
		return native_surface().validate();
	}

	bool surface_window_proxy::has_rendering_priority() const
	{
		return as_window().has_rendering_priority();
	}

	void surface_window_proxy::render_surface()
	{
		if (has_native_surface())
			native_surface().render();
	}

	void surface_window_proxy::pause_rendering()
	{
		if (has_native_surface())
			native_surface().pause();
	}

	void surface_window_proxy::resume_rendering()
	{
		if (has_native_surface())
			native_surface().resume();
	}

	bool surface_window_proxy::has_native_surface() const
	{
		return iNativeWindow != nullptr && !iNativeSurfaceDestroyed;
	}

	const i_native_surface& surface_window_proxy::native_surface() const
	{
		if (has_native_surface())
			return *iNativeWindow;
		throw no_native_surface();
	}

	i_native_surface& surface_window_proxy::native_surface()
	{
		return const_cast<i_native_surface&>(const_cast<const surface_window_proxy*>(this)->native_surface());
	}

	bool surface_window_proxy::has_native_window() const
	{
		return has_native_surface();
	}

	const i_native_window& surface_window_proxy::native_window() const
	{
		if (has_native_window())
			return static_cast<const i_native_window&>(native_surface());
		throw no_native_window();
	}

	i_native_window& surface_window_proxy::native_window()
	{
		return const_cast<i_native_window&>(const_cast<const surface_window_proxy*>(this)->native_window());
	}

	void surface_window_proxy::handle_dpi_changed()
	{
		as_window().surface().dpi_changed.trigger();
	}

	point surface_window_proxy::surface_position() const
	{
		return native_surface().surface_position();
	}

	void surface_window_proxy::move_surface(const point& aPosition)
	{
		native_surface().move_surface(aPosition);
	}

	size surface_window_proxy::surface_size() const
	{
		return native_surface().surface_size();
	}

	void surface_window_proxy::resize_surface(const size& aSize)
	{
		native_surface().resize_surface(aSize);
	}

	double surface_window_proxy::surface_opacity() const
	{
		return native_window().opacity();
	}

	void surface_window_proxy::set_surface_opacity(double aOpacity)
	{
		native_window().set_opacity(aOpacity);
	}

	double surface_window_proxy::surface_transparency() const
	{
		return native_window().transparency();
	}

	void surface_window_proxy::set_surface_transparency(double aTransparency)
	{
		native_window().set_transparency(aTransparency);
	}

	bool surface_window_proxy::has_clicked_widget() const
	{
		return iClickedWidget != nullptr;
	}

	i_widget& surface_window_proxy::clicked_widget() const
	{
		if (iClickedWidget == nullptr)
			throw widget_not_clicked();
		return *iClickedWidget;
	}

	bool surface_window_proxy::has_capturing_widget() const
	{
		return iCapturingWidget != nullptr;
	}

	i_widget& surface_window_proxy::capturing_widget() const
	{
		if (iCapturingWidget == nullptr)
			throw widget_not_capturing();
		return *iCapturingWidget;
	}

	void surface_window_proxy::set_capture(i_widget& aWidget)
	{
		if (iCapturingWidget != &aWidget)
		{
			iCapturingWidget = &aWidget;
			native_window().set_capture();
			aWidget.captured();
			as_widget().mouse_entered(as_window().mouse_position());
		}
	}

	void surface_window_proxy::release_capture(i_widget& aWidget)
	{
		if (iCapturingWidget != &aWidget)
			throw widget_not_capturing();
		native_window().release_capture();
		iCapturingWidget = nullptr;
		aWidget.released();
		as_widget().mouse_entered(as_window().mouse_position());
	}

	void surface_window_proxy::non_client_set_capture(i_widget& aWidget)
	{
		if (iCapturingWidget != &aWidget)
		{
			iCapturingWidget = &aWidget;
			native_window().non_client_set_capture();
			aWidget.captured();
			as_widget().mouse_entered(as_window().mouse_position());
		}
	}

	void surface_window_proxy::non_client_release_capture(i_widget& aWidget)
	{
		if (iCapturingWidget != &aWidget)
			throw widget_not_capturing();
		native_window().non_client_release_capture();
		iCapturingWidget = nullptr;
		aWidget.released();
		as_widget().mouse_entered(as_window().mouse_position());
	}

	bool surface_window_proxy::current_event_is_non_client() const
	{
		return native_window().has_current_event() && native_window().current_event().is<neogfx::non_client_mouse_event>();
	}

	bool surface_window_proxy::is_closing() const
	{
		return iNativeWindowClosing;
	}

	bool surface_window_proxy::native_window_can_close() const
	{
		return can_close();
	}

	void surface_window_proxy::native_window_closing()
	{
		iNativeWindowClosing = true;
		app::instance().surface_manager().remove_surface(*this);
	}

	void surface_window_proxy::native_window_closed()
	{
		iNativeWindowClosing = true;
		destroyed_flag destroyed{ *this };
		{
			auto ptr = std::move(iNativeWindow);
		}
		if (destroyed)
			return;
		if (!iClosed)
		{
			iClosed = true;
			closed.trigger();
			if (!destroyed)
				set_destroyed();
		}
	}

	void surface_window_proxy::native_window_focus_gained()
	{
		as_widget().update(true);
		if (as_window().has_focused_widget())
			as_window().focused_widget().focus_gained(focus_reason::WindowActivation);
	}

	void surface_window_proxy::native_window_focus_lost()
	{
		as_widget().update(true);
		for (std::size_t i = 0; i < app::instance().window_manager().window_count();)
		{
			i_window& w = app::instance().window_manager().window(i);
			if (!w.dismissed() && as_window().is_owner_of(w) && w.requires_owner_focus())
			{
				if (w.dismissal_type() == i_window::CloseOnDismissal)
					i = 0;
				else
					++i;
				w.dismiss();
			}
			else
				++i;
		}
		if (as_window().has_focused_widget())
			as_window().focused_widget().focus_lost(focus_reason::WindowActivation);
	}

	void surface_window_proxy::native_window_resized()
	{
		as_widget().resize(native_surface().surface_size());
	}

	bool surface_window_proxy::native_window_has_rendering_priority() const
	{
		return has_rendering_priority();
	}

	bool surface_window_proxy::native_window_ready_to_render() const
	{
		return as_widget().ready_to_render();
	}

	void surface_window_proxy::native_window_render(const rect&) const
	{
		graphics_context gc{ *this };
		as_widget().render(gc);
		gc.flush();
	}

	void surface_window_proxy::native_window_dismiss_children()
	{
		as_window().dismiss_children();
	}

	void surface_window_proxy::native_window_mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta)
	{
		i_widget& w = as_widget().widget_for_mouse_event(as_window().mouse_position());
		if (w.mouse_event.trigger(native_window().current_event()))
			as_widget().widget_for_mouse_event(as_window().mouse_position()).mouse_wheel_scrolled(aWheel, aDelta);
	}

	void surface_window_proxy::native_window_mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		i_widget& w = as_widget().widget_for_mouse_event(aPosition);
		neolib::scoped_pointer<i_widget> sp{ iClickedWidget, &w };
		as_window().dismiss_children(&w);
		as_window().update_click_focus(w, aPosition - w.origin());
		if (w.mouse_event.trigger(native_window().current_event()))
			w.mouse_button_pressed(aButton, aPosition - w.origin(), aKeyModifiers);
	}

	void surface_window_proxy::native_window_mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		i_widget& w = as_widget().widget_for_mouse_event(aPosition);
		neolib::scoped_pointer<i_widget> sp{ iClickedWidget, &w };
		as_window().dismiss_children(&w);
		as_window().update_click_focus(w, aPosition - w.origin());
		if (w.mouse_event.trigger(native_window().current_event()))
			w.mouse_button_double_clicked(aButton, aPosition - w.origin(), aKeyModifiers);
	}

	void surface_window_proxy::native_window_mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		i_widget& w = (!has_capturing_widget() ? as_widget().widget_for_mouse_event(aPosition) : capturing_widget());
		if (w.mouse_event.trigger(native_window().current_event()))
			w.mouse_button_released(aButton, aPosition - w.origin());
	}

	void surface_window_proxy::native_window_mouse_moved(const point& aPosition)
	{
		as_widget().mouse_entered(aPosition);
		i_widget& w = (!has_capturing_widget() ? as_widget().widget_for_mouse_event(aPosition) : capturing_widget());
		if (w.mouse_event.trigger(native_window().current_event()))
		{
			point widgetPos = aPosition - w.origin();
			w.mouse_moved(widgetPos);
		}
	}

	void surface_window_proxy::native_window_non_client_mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta)
	{
		i_widget& w = as_widget().widget_for_mouse_event(as_window().mouse_position());
		if (!w.ignore_non_client_mouse_events() && w.non_client_mouse_event.trigger(native_window().current_event()))
			as_widget().widget_for_mouse_event(as_window().mouse_position()).mouse_wheel_scrolled(aWheel, aDelta);
	}

	void surface_window_proxy::native_window_non_client_mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		i_widget& w = as_widget().widget_for_mouse_event(aPosition);
		as_window().dismiss_children(&w);
		if (!w.ignore_non_client_mouse_events() && w.non_client_mouse_event.trigger(native_window().current_event()))
			w.mouse_button_pressed(aButton, aPosition - w.origin(), aKeyModifiers);
	}

	void surface_window_proxy::native_window_non_client_mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		i_widget& w = as_widget().widget_for_mouse_event(aPosition);
		as_window().dismiss_children(&w);
		if (!w.ignore_non_client_mouse_events() && w.non_client_mouse_event.trigger(native_window().current_event()))
			w.mouse_button_double_clicked(aButton, aPosition - w.origin(), aKeyModifiers);
	}

	void surface_window_proxy::native_window_non_client_mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		i_widget& w = (!has_capturing_widget() ? as_widget().widget_for_mouse_event(aPosition) : capturing_widget());
		if (!w.ignore_non_client_mouse_events() && w.non_client_mouse_event.trigger(native_window().current_event()))
			w.mouse_button_released(aButton, aPosition - w.origin());
	}

	void surface_window_proxy::native_window_non_client_mouse_moved(const point& aPosition)
	{
		as_widget().mouse_entered(aPosition);
		i_widget& w = (!has_capturing_widget() ? as_widget().widget_for_mouse_event(aPosition) : capturing_widget());
		if (!w.ignore_non_client_mouse_events() && w.non_client_mouse_event.trigger(native_window().current_event()))
			w.mouse_moved(aPosition - w.origin());
	}

	void surface_window_proxy::native_window_mouse_entered(const point& aPosition)
	{
		as_widget().mouse_entered(aPosition);
	}

	void surface_window_proxy::native_window_mouse_left()
	{
		as_widget().mouse_left();
	}

	widget_part surface_window_proxy::native_window_hit_test(const point& aPosition) const
	{
		const i_widget& w = as_widget().widget_for_mouse_event(aPosition, true);
		return w.hit_test(aPosition - w.origin());
	}

	rect surface_window_proxy::native_window_widget_part_rect(widget_part aWidgetPart) const
	{
		return as_window().widget_part_rect(aWidgetPart);
	}

	void surface_window_proxy::native_window_key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
		i_widget* start = &as_widget();
		if (as_window().has_focused_widget())
			start = &as_window().focused_widget();
		if (aScanCode == ScanCode_TAB)
		{
			i_widget* w = start;
			while (w->has_parent() && (w->focus_policy() & focus_policy::ConsumeTabKey) != focus_policy::ConsumeTabKey)
				w = &w->parent();
			if ((w->focus_policy() & focus_policy::ConsumeTabKey) == focus_policy::ConsumeTabKey)
				start = w;
		}
		if (aScanCode == ScanCode_TAB && (start->focus_policy() & focus_policy::ConsumeTabKey) != focus_policy::ConsumeTabKey)
		{
			i_widget* w = start;
			if ((aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE)
			{
				for (w = &w->after(); 
					w != start && (w->effectively_hidden() || w->effectively_disabled() || (w->focus_policy() & focus_policy::TabFocus) != focus_policy::TabFocus); 
					w = &w->after())
					;
			}
			else
			{
				for (w = &w->before();
					w != start && (w->effectively_hidden() || w->effectively_disabled() || (w->focus_policy() & focus_policy::TabFocus) != focus_policy::TabFocus);
					w = &w->before())
					;
			}
			if ((w->focus_policy() & focus_policy::TabFocus) == focus_policy::TabFocus)
				w->set_focus(focus_reason::Tab);
		}
		else
		{
			auto can_consume = [aScanCode](const i_widget& aWidget)
			{
				if (aScanCode == ScanCode_TAB)
					return (aWidget.focus_policy() & focus_policy::ConsumeTabKey) == focus_policy::ConsumeTabKey;
				else if (aScanCode == ScanCode_RETURN)
					return (aWidget.focus_policy() & focus_policy::ConsumeReturnKey) == focus_policy::ConsumeReturnKey;
				else
					return true;
			};
			auto reject = [this, can_consume, aScanCode, aKeyCode, aKeyModifiers](i_widget*& w)
			{
				if (w == &as_widget())
					return false;
				i_widget& check = *w;
				w = &w->parent();
				if (!can_consume(check))
					return true;
				destroyed_flag parentDestroyed{ w->as_lifetime() };
				destroyed_flag destroyed{ check.as_lifetime() };
				if (!check.keyboard_event.trigger(native_window().current_event()))
					return false;
				if (destroyed)
				{
					if (parentDestroyed)
						w = nullptr;
					return false;
				}
				if (!check.key_pressed(aScanCode, aKeyCode, aKeyModifiers))
					return true;
				if (destroyed)
				{
					if (parentDestroyed)
						w = nullptr;
					return false;
				}
				return false;
			};
			if (aScanCode != ScanCode_TAB || !can_consume(*start) || reject(start))
			{
				if (as_window().has_focused_widget())
				{
					i_widget* w = &as_window().focused_widget();
					while (reject(w))
						;
					if (w == nullptr)
						return;
					destroyed_flag destroyed{ *this };
					if (w == &as_widget() && can_consume(as_widget()) && as_widget().keyboard_event.trigger(native_window().current_event()) && !destroyed)
						as_widget().key_pressed(aScanCode, aKeyCode, aKeyModifiers);
				}
				else if (can_consume(as_widget()) && as_widget().keyboard_event.trigger(native_window().current_event()))
					as_widget().key_pressed(aScanCode, aKeyCode, aKeyModifiers);
			}
		}
	}

	void surface_window_proxy::native_window_key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
		auto can_consume = [aScanCode](const i_widget& aWidget)
		{
			if (aScanCode == ScanCode_TAB)
				return (aWidget.focus_policy() & focus_policy::ConsumeTabKey) == focus_policy::ConsumeTabKey;
			else if (aScanCode == ScanCode_RETURN)
				return (aWidget.focus_policy() & focus_policy::ConsumeReturnKey) == focus_policy::ConsumeReturnKey;
			else
				return true;
		};
		if (as_window().has_focused_widget())
		{
			i_widget* w = &as_window().focused_widget();
			while ((!can_consume(*w) || !w->keyboard_event.trigger(native_window().current_event()) || !w->key_released(aScanCode, aKeyCode, aKeyModifiers)) && w != &as_widget())
				w = &w->parent();
			if (w == &as_widget() && can_consume(as_widget()) && as_widget().keyboard_event.trigger(native_window().current_event()))
				as_widget().key_released(aScanCode, aKeyCode, aKeyModifiers);
		}
		else if (can_consume(as_widget()) && as_widget().keyboard_event.trigger(native_window().current_event()))
			as_widget().key_released(aScanCode, aKeyCode, aKeyModifiers);
	}

	void surface_window_proxy::native_window_text_input(const std::string& aText)
	{
		auto send = [this](const std::string& aText)
		{
			auto can_consume = [&aText](i_widget& aWidget)
			{
				if (aText == "\t" && (aWidget.focus_policy() & focus_policy::ConsumeTabKey) != focus_policy::ConsumeTabKey)
					return false;
				else if (aText == "\n" && (aWidget.focus_policy() & focus_policy::ConsumeReturnKey) != focus_policy::ConsumeReturnKey)
					return false;
				return true;
			};
			if (as_window().has_focused_widget())
			{
				i_widget* w = &as_window().focused_widget();
				while ((!can_consume(*w) || !as_widget().keyboard_event.trigger(native_window().current_event()) || !w->text_input(aText)) && w != &as_widget())
					w = &w->parent();
				if (w == &as_widget() && can_consume(as_widget()) && as_widget().keyboard_event.trigger(native_window().current_event()))
					as_widget().text_input(aText);
			}
			else if (can_consume(as_widget()) && as_widget().keyboard_event.trigger(native_window().current_event()))
				as_widget().text_input(aText);
		};
		auto utf32 = neolib::utf8_to_utf32(aText);
		if (neolib::utf16::is_high_surrogate(utf32[0]))
			iSurrogatePairPart = utf32[0];
		else if (neolib::utf16::is_low_surrogate(utf32[0]) && iSurrogatePairPart != boost::none)
		{
			char16_t utf16[] = { static_cast<char16_t>(*iSurrogatePairPart), static_cast<char16_t>(neolib::utf8_to_utf32(aText)[0]) };
			iSurrogatePairPart = boost::none;
			send(neolib::utf16_to_utf8(std::u16string(&utf16[0], 2)));
		}
		else
			send(aText);
	}

	void surface_window_proxy::native_window_sys_text_input(const std::string& aText)
	{
		if (as_window().has_focused_widget())
		{
			i_widget* w = &as_window().focused_widget();
			while ((!as_widget().keyboard_event.trigger(native_window().current_event()) || !w->sys_text_input(aText)) && w != &as_widget())
				w = &w->parent();
			if (w == &as_widget() && as_widget().keyboard_event.trigger(native_window().current_event()))
				as_widget().sys_text_input(aText);
		}
		else
			as_widget().sys_text_input(aText);
	}

	neogfx::mouse_cursor surface_window_proxy::native_window_mouse_cursor() const
	{
		const i_widget& widgetUnderMouse = (!has_capturing_widget() ? as_widget().widget_for_mouse_event(as_window().mouse_position(), true) : capturing_widget());
		return widgetUnderMouse.mouse_cursor();
	}

	void surface_window_proxy::native_window_title_text_changed(const std::string& aTitleText)
	{
		as_window().set_title_text(aTitleText);
	}

	const i_window& surface_window_proxy::as_window() const
	{
		return iWindow;
	}

	i_window& surface_window_proxy::as_window()
	{
		return iWindow;
	}

	const i_widget& surface_window_proxy::as_widget() const
	{
		return as_window().as_widget();
	}

	i_widget& surface_window_proxy::as_widget()
	{
		return as_window().as_widget();
	}
}