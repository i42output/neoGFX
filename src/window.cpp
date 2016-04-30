// window.cpp
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

#include "neogfx.hpp"
#include <boost/format.hpp>
#include "window.hpp"
#include "app.hpp"
#include "i_native_window.hpp"
#include "i_layout.hpp"

#include "opengl_error.hpp"

namespace neogfx
{
	window::window(const video_mode& aVideoMode, uint32_t aStyle, framed_widget::style_e aFrameStyle) :
		scrollable_widget(aFrameStyle),
		iNativeWindow(app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aVideoMode, app::instance().name(), aStyle)), 
		iStyle(aStyle), 
		iUnits(UnitsPixels), 
		iCountedEnable(0), 
		iClosing(false), 
		iEnteredWidget(0), 
		iCapturingWidget(0),
		iFocusedWidget(0)
	{
		init();
	}

	window::window(const video_mode& aVideoMode, const std::string& aWindowTitle, uint32_t aStyle, framed_widget::style_e aFrameStyle) :
		scrollable_widget(aFrameStyle),
		iNativeWindow(app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aVideoMode, aWindowTitle, aStyle)), 
		iStyle(aStyle), 
		iUnits(UnitsPixels), 
		iCountedEnable(0), 
		iClosing(false), 
		iEnteredWidget(0), 
		iCapturingWidget(0),
		iFocusedWidget(0)
	{
		init();
	}

	window::window(dimension aWidth, dimension aHeight, uint32_t aStyle, framed_widget::style_e aFrameStyle) :
		scrollable_widget(aFrameStyle),
		iNativeWindow(app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, video_mode(static_cast<uint32_t>(aWidth), static_cast<uint32_t>(aHeight)), app::instance().name(), aStyle)), 
		iStyle(aStyle), 
		iUnits(UnitsPixels), 
		iCountedEnable(0), 
		iClosing(false), 
		iEnteredWidget(0), 
		iCapturingWidget(0),
		iFocusedWidget(0)
	{
		init();
	}

	window::window(dimension aWidth, dimension aHeight, const std::string& aWindowTitle, uint32_t aStyle, framed_widget::style_e aFrameStyle) :
		scrollable_widget(aFrameStyle),
		iNativeWindow(app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, video_mode(static_cast<uint32_t>(aWidth), static_cast<uint32_t>(aHeight)), aWindowTitle, aStyle)), 
		iStyle(aStyle), 
		iUnits(UnitsPixels), 
		iCountedEnable(0), 
		iClosing(false), 
		iEnteredWidget(0), 
		iCapturingWidget(0),
		iFocusedWidget(0)
	{
		init();
	}

	window::window(const point& aPosition, const size& aDimensions, uint32_t aStyle, framed_widget::style_e aFrameStyle) :
		scrollable_widget(aFrameStyle),
		iNativeWindow(app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aPosition, aDimensions, app::instance().name(), aStyle)),
		iStyle(aStyle),
		iUnits(UnitsPixels),
		iCountedEnable(0),
		iClosing(false),
		iEnteredWidget(0),
		iCapturingWidget(0),
		iFocusedWidget(0)
	{
		init();
	}

	window::window(const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, uint32_t aStyle, framed_widget::style_e aFrameStyle) :
		scrollable_widget(aFrameStyle),
		iNativeWindow(app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aPosition, aDimensions, aWindowTitle, aStyle)),
		iStyle(aStyle),
		iUnits(UnitsPixels),
		iCountedEnable(0),
		iClosing(false),
		iEnteredWidget(0),
		iCapturingWidget(0),
		iFocusedWidget(0)
	{
		init();
	}

	window::window(i_widget& aParent, const video_mode& aVideoMode, uint32_t aStyle, framed_widget::style_e aFrameStyle) :
		scrollable_widget(aFrameStyle),
		iNativeWindow(app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aParent.surface().native_surface(), aVideoMode, app::instance().name(), aStyle)),
		iStyle(aStyle), 
		iUnits(UnitsPixels), 
		iCountedEnable(0), 
		iClosing(false), 
		iEnteredWidget(0), 
		iCapturingWidget(0),
		iFocusedWidget(0)
	{
		set_parent(aParent.ultimate_ancestor());
		init();
	}

	window::window(i_widget& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, uint32_t aStyle, framed_widget::style_e aFrameStyle) :
		scrollable_widget(aFrameStyle),
		iNativeWindow(app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aParent.surface().native_surface(), aVideoMode, aWindowTitle, aStyle)),
		iStyle(aStyle), 
		iUnits(UnitsPixels), 
		iCountedEnable(0), 
		iClosing(false), 
		iEnteredWidget(0), 
		iCapturingWidget(0),
		iFocusedWidget(0)
	{
		set_parent(aParent.ultimate_ancestor());
		init();
	}

	window::window(i_widget& aParent, dimension aWidth, dimension aHeight, uint32_t aStyle, framed_widget::style_e aFrameStyle) :
		scrollable_widget(aFrameStyle),
		iNativeWindow(app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aParent.surface().native_surface(), video_mode(static_cast<uint32_t>(aWidth), static_cast<uint32_t>(aHeight)), app::instance().name(), aStyle)),
		iStyle(aStyle), 
		iUnits(UnitsPixels), 
		iCountedEnable(0), 
		iClosing(false), 
		iEnteredWidget(0), 
		iCapturingWidget(0),
		iFocusedWidget(0)
	{
		set_parent(aParent.ultimate_ancestor());
		init();
	}

	window::window(i_widget& aParent, dimension aWidth, dimension aHeight, const std::string& aWindowTitle, uint32_t aStyle, framed_widget::style_e aFrameStyle) :
		scrollable_widget(aFrameStyle),
		iNativeWindow(app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aParent.surface().native_surface(), video_mode(static_cast<uint32_t>(aWidth), static_cast<uint32_t>(aHeight)), aWindowTitle, aStyle)),
		iStyle(aStyle), 
		iUnits(UnitsPixels), 
		iCountedEnable(0), 
		iClosing(false), 
		iEnteredWidget(0), 
		iCapturingWidget(0),
		iFocusedWidget(0)
	{
		set_parent(aParent.ultimate_ancestor());
		init();
	}

	window::window(i_widget& aParent, const point& aPosition, const size& aDimensions, uint32_t aStyle, framed_widget::style_e aFrameStyle) :
		scrollable_widget(aFrameStyle),
		iNativeWindow(app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aParent.surface().native_surface(), aPosition, aDimensions, app::instance().name(), aStyle)),
		iStyle(aStyle),
		iUnits(UnitsPixels),
		iCountedEnable(0),
		iClosing(false),
		iEnteredWidget(0),
		iCapturingWidget(0),
		iFocusedWidget(0)
	{
		set_parent(aParent.ultimate_ancestor());
		init();
	}

	window::window(i_widget& aParent, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, uint32_t aStyle, framed_widget::style_e aFrameStyle) :
		scrollable_widget(aFrameStyle),
		iNativeWindow(app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aParent.surface().native_surface(), aPosition, aDimensions, aWindowTitle, aStyle)),
		iStyle(aStyle),
		iUnits(UnitsPixels),
		iCountedEnable(0),
		iClosing(false),
		iEnteredWidget(0),
		iCapturingWidget(0),
		iFocusedWidget(0)
	{
		set_parent(aParent.ultimate_ancestor());
		init();
	}

	window::~window()
	{
		if (iNativeWindow)
			iNativeWindow->close();
		app::instance().surface_manager().remove_surface(*this);
	}

	uint32_t window::style() const
	{
		return iStyle;
	}

	bool window::is_root() const
	{
		return true;
	}

	bool window::can_defer_layout() const
	{
		return true;
	}

	bool window::is_managing_layout() const
	{
		return true;
	}

	void window::layout_items_completed()
	{
		scrollable_widget::layout_items_completed();
		i_widget& widgetUnderMouse = (iCapturingWidget == 0 ? widget_for_mouse_event(iNativeWindow->mouse_position()) : *iCapturingWidget);
		if (iEnteredWidget != &widgetUnderMouse)
			native_window_mouse_entered();
	}

	size window::extents() const
	{
		return iNativeWindow->extents();
	}

	dimension window::horizontal_dpi() const
	{
		return iNativeWindow->horizontal_dpi();
	}

	dimension window::vertical_dpi() const
	{
		return iNativeWindow->vertical_dpi();
	}

	dimension window::em_size() const
	{
		return 0; /* todo */
	}

	const i_device_metrics& window::device_metrics() const
	{
		return *iNativeWindow;
	}

	units_e window::units() const
	{
		return iUnits;
	}

	units_e window::set_units(units_e aUnits) const
	{
		units_e oldUnits = iUnits;
		iUnits = aUnits;
		return oldUnits;
	}

	void window::resized()
	{
		resize_surface(widget::extents());
		scrollable_widget::resized();
	}

	neogfx::size_policy window::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		return neogfx::size_policy::Manual;
	}

	colour window::background_colour() const
	{
		if (has_background_colour())
			return scrollable_widget::background_colour();
		else
			return container_background_colour();
	}

	void window::close()
	{
		layout().remove_items();
		remove_widgets();
		native_surface().close();
		closed.trigger();
	}

	bool window::has_parent_surface() const
	{
		return has_parent();
	}

	const i_surface& window::parent_surface() const
	{
		return parent().surface();
	}
	
	i_surface& window::parent_surface()
	{
		return parent().surface();
	}

	bool window::is_owner_of(const i_surface& aChildSurface) const
	{
		const i_surface* parent = &aChildSurface;
		while (parent->has_parent_surface())
		{
			parent = &parent->parent_surface();
			if (parent == this)
				return true;
		}
		return false;
	}

	surface_type window::surface_type() const
	{
		return neogfx::surface_type::Window;
	}

	neogfx::logical_coordinate_system window::logical_coordinate_system() const
	{
		return native_surface().logical_coordinate_system();
	}

	void window::set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem)
	{
		native_surface().set_logical_coordinate_system(aSystem);
	}

	const vector4& window::logical_coordinates() const
	{
		return native_surface().logical_coordinates();
	}

	void window::set_logical_coordinates(const vector4& aCoordinates)
	{
		native_surface().set_logical_coordinates(aCoordinates);
	}

	void window::layout_surface()
	{
		widget::layout_items();
	}

	void window::invalidate_surface(const rect& aInvalidatedRect, bool aInternal)
	{
		iNativeWindow->invalidate(aInvalidatedRect);
		if (!aInternal)
			update(aInvalidatedRect);
	}

	void window::render_surface()
	{
		iNativeWindow->render();
	}
	
	graphics_context window::create_graphics_context() const
	{
		return graphics_context(static_cast<const i_widget&>(*this));
	}

	graphics_context window::create_graphics_context(const i_widget& aWidget) const
	{
		return graphics_context(aWidget);
	}

	const i_native_window& window::native_surface() const
	{
		return *iNativeWindow;
	}

	i_native_window& window::native_surface()
	{
		return *iNativeWindow;
	}

	bool window::destroyed() const
	{
		return !iNativeWindow;
	}

	point window::surface_position() const
	{
		return native_surface().surface_position();
	}

	void window::move_surface(const point& aPosition)
	{
		native_surface().move_surface(aPosition);
	}

	size window::surface_size() const
	{
		return native_surface().surface_size();
	}

	void window::resize_surface(const size& aSize)
	{
		native_surface().resize_surface(aSize);
	}

	point window::mouse_position() const
	{
		return native_surface().mouse_position();
	}

	bool window::is_mouse_button_pressed(mouse_button aButton) const
	{
		return native_surface().is_mouse_button_pressed(aButton);
	}

	void window::save_mouse_cursor()
	{
		native_surface().save_mouse_cursor();
	}

	void window::set_mouse_cursor(mouse_system_cursor aSystemCursor)
	{
		native_surface().set_mouse_cursor(aSystemCursor);
	}

	void window::restore_mouse_cursor()
	{
		native_surface().restore_mouse_cursor();
	}

	void window::widget_added(i_widget&)
	{
		layout_items(true);
	}

	void window::widget_removed(i_widget& aWidget)
	{
		if (iEnteredWidget == &aWidget)
			iEnteredWidget = 0;
		if (iCapturingWidget == &aWidget)
			release_capture(aWidget);
		if (iFocusedWidget == &aWidget)
			iFocusedWidget = 0;
		layout_items(true);
	}

	bool window::requires_owner_focus() const
	{
		return (iStyle & RequiresOwnerFocus) == RequiresOwnerFocus;
	}

	bool window::has_entered_widget() const
	{
		return iEnteredWidget != 0;
	}

	i_widget& window::entered_widget() const
	{
		if (iEnteredWidget == 0)
			throw widget_not_entered();
		return *iEnteredWidget;
	}

	bool window::has_capturing_widget() const
	{
		return iCapturingWidget != 0;
	}

	i_widget& window::capturing_widget() const
	{
		if (iCapturingWidget == 0)
			throw widget_not_capturing();
		return *iCapturingWidget;
	}

	void window::set_capture(i_widget& aWidget)
	{
		if (iCapturingWidget != &aWidget)
		{
			iCapturingWidget = &aWidget;
			iNativeWindow->set_capture();
			aWidget.captured();
			native_window_mouse_entered();
		}
	}

	void window::release_capture(i_widget& aWidget)
	{
		if (iCapturingWidget != &aWidget)
			throw widget_not_capturing();
		iNativeWindow->release_capture();
		iCapturingWidget = 0;
		aWidget.released();
		native_window_mouse_entered();
	}

	bool window::has_focused_widget() const
	{
		return iFocusedWidget != 0;
	}

	i_widget& window::focused_widget() const
	{
		return *iFocusedWidget;
	}

	void window::set_focused_widget(i_widget& aWidget)
	{
		if (iFocusedWidget == &aWidget)
			return;
		i_widget* previouslyFocused = iFocusedWidget;
		iFocusedWidget = &aWidget;
		if (previouslyFocused != 0)
			previouslyFocused->focus_lost();
		iFocusedWidget->focus_gained();
	}

	void window::release_focused_widget(i_widget& aWidget)
	{
		if (iFocusedWidget != &aWidget)
			throw widget_not_focused();
		iFocusedWidget = 0;
		aWidget.focus_lost();
	}

	void window::activate()
	{
		if (iNativeWindow)
			iNativeWindow->activate();
	}

	void window::counted_enable(bool aEnable)
	{
		if (aEnable)
			++iCountedEnable;
		else
			--iCountedEnable;
		if (iNativeWindow)
			iNativeWindow->enable(iCountedEnable >= 0);
	}

	bool window::has_surface() const
	{
		return true;
	}

	const i_surface& window::surface() const
	{
		return *this;
	}

	i_surface& window::surface()
	{
		return *this;
	}

	void window::init()
	{
		app::instance().surface_manager().add_surface(*this);
		update_modality();
		scrollable_widget::init();
	}

	void window::native_window_closing()
	{
		if (!iClosing)
		{
			iClosing = true;
			update_modality();
		}
		if (has_parent() && !(static_cast<window&>(parent()).style() & window::NoActivate))
			static_cast<window&>(parent()).activate();
	}

	void window::native_window_closed()
	{
		if (!iClosing)
		{
			iClosing = true;
			update_modality();
		}
		iNativeWindow.reset();
		app::instance().surface_manager().remove_surface(*this);
		if (has_parent() && !(static_cast<window&>(parent()).style() & window::NoActivate))
			static_cast<window&>(parent()).activate();
	}

	void window::native_window_focus_gained()
	{
	}

	void window::native_window_focus_lost()
	{
		for (std::size_t i = 0; i < app::instance().surface_manager().surface_count();)
		{
			auto& s = app::instance().surface_manager().surface(i);
			if (is_owner_of(s) && s.requires_owner_focus())
			{
				s.close();
				i = 0;
			}
			else
				++i;
		}
	}

	void window::native_window_resized()
	{
		resize(native_surface().surface_size());
	}	

	void window::native_window_render(const rect&) const
	{
		graphics_context gc(surface());
		gc.set_extents(extents());
		gc.set_origin(origin());
		render(gc);
	}

	void window::native_window_mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta)
	{
		widget_for_mouse_event(iNativeWindow->mouse_position()).mouse_wheel_scrolled(aWheel, aDelta);
	}

	void window::native_window_mouse_button_pressed(mouse_button aButton, const point& aPosition)
	{
		i_widget& w = widget_for_mouse_event(aPosition);
		update_click_focus(w);
		w.mouse_button_pressed(aButton, aPosition - w.origin());
	}

	void window::native_window_mouse_button_double_clicked(mouse_button aButton, const point& aPosition)
	{
		i_widget& w = widget_for_mouse_event(aPosition);
		update_click_focus(w);
		w.mouse_button_double_clicked(aButton, aPosition - w.origin());
	}

	void window::native_window_mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		if (iCapturingWidget == 0)
			widget_for_mouse_event(aPosition).mouse_button_released(aButton, aPosition - widget_for_mouse_event(aPosition).origin());
		else
			iCapturingWidget->mouse_button_released(aButton, aPosition - iCapturingWidget->origin());
	}

	void window::native_window_mouse_moved(const point& aPosition)
	{
		native_window_mouse_entered();
		if (iCapturingWidget == 0)
			widget_for_mouse_event(aPosition).mouse_moved(aPosition - widget_for_mouse_event(aPosition).origin());
		else
			iCapturingWidget->mouse_moved(aPosition - iCapturingWidget->origin());
	}

	void window::native_window_mouse_entered()
	{
		i_widget& widgetUnderMouse = (iCapturingWidget == 0 ? widget_for_mouse_event(iNativeWindow->mouse_position()) : *iCapturingWidget);
		i_widget* previousEnteredWidget = iEnteredWidget;
		iEnteredWidget = &widgetUnderMouse;
		if (iEnteredWidget != previousEnteredWidget)
		{
			if (previousEnteredWidget != 0)
				previousEnteredWidget->mouse_left();
			iEnteredWidget->mouse_entered();
		}
	}

	void window::native_window_mouse_left()
	{
		i_widget* previousEnteredWidget = iEnteredWidget;
		iEnteredWidget = 0;
		if (previousEnteredWidget != 0)
			previousEnteredWidget->mouse_left();
	}

	void window::native_window_key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
		i_widget* start = this;
		if (has_focused_widget())
			start = &focused_widget();
		if (aScanCode == ScanCode_TAB && (start->focus_policy() & focus_policy::ConsumeTabKey) != focus_policy::ConsumeTabKey)
		{
			i_widget* w = start;
			if ((aKeyModifiers & KeyModifier_SHIFT) == KeyModifier_NONE)
			{
				for (w = &w->link_after(); 
					w != start && (w->effectively_hidden() || w->effectively_disabled() || (w->focus_policy() & focus_policy::TabFocus) != focus_policy::TabFocus); 
					w = &w->link_after())
					;
			}
			else
			{
				for (w = &w->link_before();
					w != start && (w->effectively_hidden() || w->effectively_disabled() || (w->focus_policy() & focus_policy::TabFocus) != focus_policy::TabFocus);
					w = &w->link_before())
					;
			}
			if ((w->focus_policy() & focus_policy::TabFocus) == focus_policy::TabFocus)
				w->set_focus();
		}
		else
		{
			if (has_focused_widget())
				focused_widget().key_pressed(aScanCode, aKeyCode, aKeyModifiers);
			else
				key_pressed(aScanCode, aKeyCode, aKeyModifiers);
		}
	}

	void window::native_window_key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
		if (has_focused_widget())
			focused_widget().key_released(aScanCode, aKeyCode, aKeyModifiers);
		else
			key_released(aScanCode, aKeyCode, aKeyModifiers);
	}

	void window::native_window_text_input(const std::string& aText)
	{
		if (has_focused_widget())
			focused_widget().text_input(aText);
		else
			text_input(aText);
	}

	void window::native_window_set_default_mouse_cursor()
	{
		i_widget& widgetUnderMouse = (iCapturingWidget == 0 ? widget_for_mouse_event(iNativeWindow->mouse_position()) : *iCapturingWidget);
		widgetUnderMouse.set_default_mouse_cursor();
	}

	void window::update_click_focus(i_widget& aCandidateWidget)
	{
		if (aCandidateWidget.enabled() && (aCandidateWidget.focus_policy() & focus_policy::ClickFocus) == focus_policy::ClickFocus)
			aCandidateWidget.set_focus();
		else if (aCandidateWidget.has_parent())
			update_click_focus(aCandidateWidget.parent());
	}

	void window::update_modality()
	{
		for (std::size_t i = 0; i < app::instance().surface_manager().surface_count(); ++i)
		{
			i_surface& surface = app::instance().surface_manager().surface(i);
			if (surface.surface_type() == surface_type::Window && &surface != this)
			{
				window& windowSurface = static_cast<window&>(surface);
				if (iStyle & ApplicationModal)
					windowSurface.counted_enable(!iNativeWindow || iClosing);
				else if ((iStyle & Modal) && windowSurface.is_ancestor(*this))
					windowSurface.counted_enable(!iNativeWindow || iClosing);
			}
		}
	}
}