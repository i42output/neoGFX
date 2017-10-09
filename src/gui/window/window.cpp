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

#include <neogfx/neogfx.hpp>
#include <boost/format.hpp>
#include <neolib/string_utils.hpp>
#include <neolib/raii.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include "native/i_native_window.hpp"
#include "../../hid/native/i_native_surface.hpp"

namespace neogfx
{
	window::window(window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		scrollable_widget{ aScrollbarStyle, aFrameStyle },
		iStyle{ aStyle },
		iUnits{ units::Pixels },
		iCountedEnable{ 0 },
		iNativeWindowClosing{ false },
		iClosed{ false },
		iEnteredWidget{ 0 },
		iCapturingWidget{ 0 },
		iFocusedWidget{ 0 },
		iDismissingChildren{ false },
		iNonClientLayout{ *this },
		iTitleBarLayout{ iNonClientLayout },
		iMenuLayout{ iNonClientLayout },
		iToolbarLayout{ iNonClientLayout },
		iClientLayout{ iNonClientLayout },
		iStatusBarLayout{ iNonClientLayout }
	{
	}

	window::window(const video_mode& aVideoMode, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		iNativeWindow = app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aVideoMode, app::instance().name(), aStyle);
		init();
	}

	window::window(const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		iNativeWindow = app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aVideoMode, aWindowTitle, aStyle);
		init();
	}

	window::window(const size& aDimensions, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		iNativeWindow = app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aDimensions, app::instance().name(), aStyle);
		init();
	}

	window::window(const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		iNativeWindow = app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aDimensions, aWindowTitle, aStyle);
		init();
	}

	window::window(const point& aPosition, const size& aDimensions, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		iNativeWindow = app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aPosition, aDimensions, app::instance().name(), aStyle);
		init();
	}

	window::window(const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		iNativeWindow = app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aPosition, aDimensions, aWindowTitle, aStyle);
		init();
	}

	window::window(i_widget& aParent, const video_mode& aVideoMode, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		iNativeWindow = app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aParent.surface().native_surface(), aVideoMode, app::instance().name(), aStyle);
		set_parent(aParent.ultimate_ancestor());
		init();
	}

	window::window(i_widget& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		iNativeWindow = app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aParent.surface().native_surface(), aVideoMode, aWindowTitle, aStyle);
		set_parent(aParent.ultimate_ancestor());
		init();
	}

	window::window(i_widget& aParent, const size& aDimensions, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		iNativeWindow = app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aParent.surface().native_surface(), aDimensions, app::instance().name(), aStyle);
		set_parent(aParent.ultimate_ancestor());
		init();
	}

	window::window(i_widget& aParent, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		iNativeWindow = app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aParent.surface().native_surface(), aDimensions, aWindowTitle, aStyle);
		set_parent(aParent.ultimate_ancestor());
		init();
	}

	window::window(i_widget& aParent, const point& aPosition, const size& aDimensions, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		iNativeWindow = app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aParent.surface().native_surface(), aPosition, aDimensions, app::instance().name(), aStyle);
		set_parent(aParent.ultimate_ancestor());
		init();
	}

	window::window(i_widget& aParent, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		iNativeWindow = app::instance().rendering_engine().create_window(app::instance().surface_manager(), *this, aParent.surface().native_surface(), aPosition, aDimensions, aWindowTitle, aStyle);
		set_parent(aParent.ultimate_ancestor());
		init();
	}

	window::~window()
	{
		close();
	}

	window_style window::style() const
	{
		return iStyle;
	}

	void window::set_style(window_style aStyle)
	{
		iStyle = aStyle;
	}

	double window::fps() const
	{
		if (iNativeWindow)
			return iNativeWindow->fps();
		else
			return 0.0;
	}

	colour window::frame_colour() const
	{
		if (!scrollable_widget::has_frame_colour() && is_active())
			return app::instance().current_style().palette().selection_colour();
		else
			return scrollable_widget::frame_colour();
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
		i_widget& widgetUnderMouse = (iCapturingWidget == 0 ? widget_for_mouse_event(native_surface().mouse_position()) : *iCapturingWidget);
		if (iEnteredWidget != &widgetUnderMouse)
			mouse_entered();
	}

	bool window::metrics_available() const
	{
		return native_window().metrics_available();
	}

	size window::extents() const
	{
		return native_window().extents();
	}

	dimension window::horizontal_dpi() const
	{
		return native_window().horizontal_dpi();
	}

	dimension window::vertical_dpi() const
	{
		return native_window().vertical_dpi();
	}

	dimension window::em_size() const
	{
		return 0; /* todo */
	}

	const i_device_metrics& window::device_metrics() const
	{
		return *iNativeWindow;
	}

	units window::units() const
	{
		return iUnits;
	}

	units window::set_units(neogfx::units aUnits) const
	{
		neogfx::units oldUnits = iUnits;
		iUnits = aUnits;
		return oldUnits;
	}

	void window::resized()
	{
		resize_surface(widget::extents());
		scrollable_widget::resized();
		update(true);
	}

	widget_part window::hit_test(const point& aPosition) const
	{
		auto result = scrollable_widget::hit_test(aPosition);
		if (result == widget_part::Client)
			result = widget_part::NonClientGrab;
		return result;
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

	i_rendering_engine& window::rendering_engine() const
	{
		return app::instance().rendering_engine();
	}

	bool window::is_weak() const
	{
		return (iStyle & window_style::Weak) == window_style::Weak;
	}

	bool window::can_close() const
	{
		return true;
	}

	bool window::is_closed() const
	{
		return iClosed;
	}

	void window::close()
	{
		if (iClosed)
			return;
		if (has_layout())
			layout().remove_items();
		remove_widgets();
		destroyed_flag destroyed(*this);
		if (!window::destroyed())
			native_surface().close();
		else
		{
			auto ptr = std::move(iNativeWindow);
		}
		if (destroyed)
			return;
		iClosed = true;
		closed.trigger();
	}

	bool window::has_parent_surface() const
	{
		return has_parent(false);
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

	bool window::is_dismissing_children() const
	{
		return iDismissingChildren;
	}

	bool window::can_dismiss(const i_widget*) const
	{
		return true;
	}

	i_surface::dismissal_type_e window::dismissal_type() const
	{
		return CloseOnDismissal;
	}

	bool window::dismissed() const
	{
		return iClosed;
	}

	void window::dismiss()
	{
		close();
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

	const std::pair<vec2, vec2>& window::logical_coordinates() const
	{
		return native_surface().logical_coordinates();
	}

	void window::set_logical_coordinates(const std::pair<vec2, vec2>& aCoordinates)
	{
		native_surface().set_logical_coordinates(aCoordinates);
	}

	void window::layout_surface()
	{
		widget::layout_items();
	}

	void window::invalidate_surface(const rect& aInvalidatedRect, bool aInternal)
	{
		native_surface().invalidate(aInvalidatedRect);
		if (!aInternal)
			update(aInvalidatedRect);
	}

	bool window::has_invalidated_area() const
	{
		return native_surface().has_invalidated_area();
	}

	const rect& window::invalidated_area() const
	{
		return native_surface().invalidated_area();
	}

	bool window::has_rendering_priority() const
	{
		return is_active();
	}

	void window::render_surface()
	{
		if (!destroyed())
			native_surface().render();
	}

	void window::pause_rendering()
	{
		if (!destroyed())
			native_surface().pause();
	}

	void window::resume_rendering()
	{
		if (!destroyed())
			native_surface().resume();
	}
	
	graphics_context window::create_graphics_context() const
	{
		return graphics_context(static_cast<const i_widget&>(*this));
	}

	graphics_context window::create_graphics_context(const i_widget& aWidget) const
	{
		return graphics_context(aWidget);
	}

	const i_native_surface& window::native_surface() const
	{
		if (iNativeWindow == nullptr)
			throw no_native_surface();
		return *iNativeWindow;
	}

	i_native_surface& window::native_surface()
	{
		if (iNativeWindow == nullptr)
			throw no_native_surface();
		return *iNativeWindow;
	}

	const i_native_window& window::native_window() const
	{
		return static_cast<const i_native_window&>(native_surface());
	}

	i_native_window& window::native_window()
	{
		return static_cast<i_native_window&>(native_surface());
	}

	bool window::destroyed() const
	{
		return !iNativeWindow || native_window().is_destroyed();
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

	void window::centre()
	{
		resize(minimum_size());
		rect desktopRect{ app::instance().surface_manager().desktop_rect(surface()) };
		move_surface((desktopRect.extents() - surface_size()) / 2.0);
	}

	void window::centre_on_parent()
	{
		if (has_parent_surface())
		{
			resize(minimum_size());
			rect desktopRect{ app::instance().surface_manager().desktop_rect(surface()) };
			rect parentRect{ parent_surface().surface_position(), parent_surface().surface_size() };
			point position = point{ (parentRect.extents() - surface_size()) / 2.0 } + parentRect.top_left();
			if (position.x < 0.0)
				position.x = 0.0;
			if (position.y < 0.0)
				position.y = 0.0;
			if (position.x + surface_size().cx > desktopRect.right())
				position.x = desktopRect.right() - surface_size().cx;
			if (position.y + surface_size().cy > desktopRect.bottom())
				position.y = desktopRect.bottom() - surface_size().cy;
			move_surface(position);
		}
		else
			centre();
	}

	void window::set_transparency(double aAlpha)
	{
		native_window().set_transparency(aAlpha);
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

	void window::update_mouse_cursor()
	{
		native_surface().update_mouse_cursor();
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

	void window::show(bool aVisible)
	{
		widget::show(aVisible);
		if (!destroyed())
		{
			if (aVisible)
				native_window().show();
			else
				native_window().hide();
		}
	}

	bool window::requires_owner_focus() const
	{
		return (iStyle & window_style::RequiresOwnerFocus) == window_style::RequiresOwnerFocus;
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
			native_window().set_capture();
			aWidget.captured();
			mouse_entered();
		}
	}

	void window::release_capture(i_widget& aWidget)
	{
		if (iCapturingWidget != &aWidget)
			throw widget_not_capturing();
		native_window().release_capture();
		iCapturingWidget = 0;
		aWidget.released();
		mouse_entered();
	}

	bool window::has_focused_widget() const
	{
		return iFocusedWidget != 0;
	}

	i_widget& window::focused_widget() const
	{
		return *iFocusedWidget;
	}

	void window::set_focused_widget(i_widget& aWidget, focus_reason aFocusReason)
	{
		if (iFocusedWidget == &aWidget)
			return;
		i_widget* previouslyFocused = iFocusedWidget;
		iFocusedWidget = &aWidget;
		if (previouslyFocused != 0)
			previouslyFocused->focus_lost(aFocusReason);
		iFocusedWidget->focus_gained(aFocusReason);
	}

	void window::release_focused_widget(i_widget& aWidget)
	{
		if (iFocusedWidget != &aWidget)
			throw widget_not_focused();
		iFocusedWidget = 0;
		aWidget.focus_lost(focus_reason::Other);
	}

	window::child_widget_scrolling_disposition_e window::scrolling_disposition(const i_widget& aChildWidget) const
	{
		if (iTitleBar != boost::none && &aChildWidget == &*iTitleBar)
			return DontScrollChildWidget;
		return scrollable_widget::scrolling_disposition(aChildWidget);
	}

	const std::string& window::title_text() const
	{
		return native_window().title_text();
	}

	void window::set_title_text(const std::string& aTitleText)
	{
		native_window().set_title_text(aTitleText);
	}

	bool window::is_active() const
	{
		return iNativeWindow && native_window().is_active();
	}

	void window::activate()
	{
		if (iNativeWindow)
			native_window().activate();
	}

	void window::counted_enable(bool aEnable)
	{
		if (aEnable)
			++iCountedEnable;
		else
			--iCountedEnable;
		if (iNativeWindow)
			native_window().enable(iCountedEnable >= 0);
	}

	const i_surface& window::surface() const
	{
		return *this;
	}

	i_surface& window::surface()
	{
		return *this;
	}

	bool window::is_surface() const
	{
		return true;
	}

	void window::init()
	{
		app::instance().surface_manager().add_surface(*this);
		update_modality();
		scrollable_widget::init();

		if ((style() & window_style::TitleBar) == window_style::TitleBar)
			iTitleBar.emplace(title_bar_layout(), app::instance().default_window_icon(), native_window().title_text());

		set_margins(neogfx::margins{});
		iNonClientLayout.set_margins(neogfx::margins{});
		iNonClientLayout.set_spacing(0.0);
		iTitleBarLayout.set_margins(neogfx::margins{});
		iMenuLayout.set_margins(neogfx::margins{});
		iToolbarLayout.set_margins(neogfx::margins{});
		iClientLayout.set_margins(neogfx::optional_margins{});
		iStatusBarLayout.set_margins(neogfx::margins{});

		layout_items(true);

		iSink += app::instance().current_style_changed([this](style_aspect aAspect)
		{
			if ((aAspect & style_aspect::Colour) == style_aspect::Colour)
				native_surface().invalidate(surface_size());
		});
	}

	bool window::native_window_can_close() const
	{
		return can_close();
	}

	void window::native_window_closing()
	{
		if (!iNativeWindowClosing)
		{
			iNativeWindowClosing = true;
			update_modality();
		}
		if (has_parent(false) && (static_cast<window&>(parent()).iStyle & window_style::NoActivate) != window_style::NoActivate)
			static_cast<window&>(parent()).activate();
	}

	void window::native_window_closed()
	{
		if (!iNativeWindowClosing)
		{
			iNativeWindowClosing = true;
			update_modality();
		}
		destroyed_flag destroyed(*this);
		{
			auto ptr = std::move(iNativeWindow);
		}
		if (destroyed)
			return;
		app::instance().surface_manager().remove_surface(*this);
		if (has_parent(false) && (static_cast<window&>(parent()).iStyle & window_style::NoActivate) != window_style::NoActivate)
			static_cast<window&>(parent()).activate();
		if (!iClosed)
		{
			iClosed = true;
			closed.trigger();
		}
	}

	void window::native_window_focus_gained()
	{
		update(true);
		if (has_focused_widget())
			focused_widget().focus_gained(focus_reason::WindowActivation);
	}

	void window::native_window_focus_lost()
	{
		update(true);
		for (std::size_t i = 0; i < app::instance().surface_manager().surface_count();)
		{
			auto& s = app::instance().surface_manager().surface(i);
			if (!s.dismissed() && is_owner_of(s) && s.requires_owner_focus())
			{
				if (s.dismissal_type() == CloseOnDismissal)
					i = 0;
				else
					++i;
				s.dismiss();
			}
			else
				++i;
		}
		if (has_focused_widget())
			focused_widget().focus_lost(focus_reason::WindowActivation);
	}

	void window::native_window_resized()
	{
		resize(native_surface().surface_size());
	}

	bool window::native_window_has_rendering_priority() const
	{
		return has_rendering_priority();
	}

	bool window::native_window_ready_to_render() const
	{
		return ready_to_render();
	}

	void window::native_window_render(const rect&) const
	{
		graphics_context gc(surface());
		gc.set_extents(extents());
		gc.set_origin(origin());
		render(gc);
		gc.set_extents(extents());
		gc.set_origin(origin());
		paint_overlay.trigger(gc);
		gc.flush();
	}

	void window::native_window_dismiss_children()
	{
		dismiss_children();
	}

	void window::native_window_mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta)
	{
		i_widget& w = widget_for_mouse_event(native_surface().mouse_position());
		if (w.mouse_event.trigger(native_window().current_event()))
			widget_for_mouse_event(native_surface().mouse_position()).mouse_wheel_scrolled(aWheel, aDelta);
	}

	void window::native_window_mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		i_widget& w = widget_for_mouse_event(aPosition);
		dismiss_children(&w);
		update_click_focus(w, aPosition - w.origin());
		if (w.mouse_event.trigger(native_window().current_event()))
			w.mouse_button_pressed(aButton, aPosition - w.origin(), aKeyModifiers);
	}

	void window::native_window_mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		i_widget& w = widget_for_mouse_event(aPosition);
		dismiss_children(&w);
		update_click_focus(w, aPosition - w.origin());
		if (w.mouse_event.trigger(native_window().current_event()))
			w.mouse_button_double_clicked(aButton, aPosition - w.origin(), aKeyModifiers);
	}

	void window::native_window_mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		i_widget& w = (iCapturingWidget == 0 ? widget_for_mouse_event(aPosition) : *iCapturingWidget);
		if (w.mouse_event.trigger(native_window().current_event()))
			w.mouse_button_released(aButton, aPosition - w.origin());
	}

	void window::native_window_mouse_moved(const point& aPosition)
	{
		mouse_entered();
		i_widget& w = (iCapturingWidget == 0 ? widget_for_mouse_event(aPosition) : *iCapturingWidget);
		if (w.mouse_event.trigger(native_window().current_event()))
			w.mouse_moved(aPosition - w.origin());
	}

	void window::native_window_mouse_entered()
	{
		mouse_entered();
	}

	void window::native_window_mouse_left()
	{
		i_widget* previousEnteredWidget = iEnteredWidget;
		iEnteredWidget = 0;
		if (previousEnteredWidget != 0)
			previousEnteredWidget->mouse_left();
	}

	widget_part window::native_window_hit_test(const point& aPosition) const
	{
		const i_widget& w = widget_for_mouse_event(aPosition, true);
		return w.hit_test(aPosition - w.origin());
	}

	rect window::native_window_widget_part_rect(widget_part aWidgetPart) const
	{
		switch (aWidgetPart)
		{
		case widget_part::NonClientTitleBar:
			if (iTitleBar != boost::none)
				return to_client_coordinates(iTitleBar->window_rect());
			else
				return rect{};
		default:
			return rect{};
		}
	}

	void window::native_window_key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
		i_widget* start = this;
		if (has_focused_widget())
			start = &focused_widget();
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
				if (w == this)
					return false;
				i_widget& check = *w;
				w = &w->parent();
				if (!can_consume(check))
					return true;
				destroyed_flag parentDestroyed{ w->as_destroyable() };
				destroyed_flag destroyed{ check.as_destroyable() };
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
				if (has_focused_widget())
				{
					i_widget* w = &focused_widget();
					while (reject(w))
						;
					if (w == nullptr)
						return;
					destroyed_flag destroyed{ *this };
					if (w == this && can_consume(*this) && keyboard_event.trigger(native_window().current_event()) && !destroyed)
						key_pressed(aScanCode, aKeyCode, aKeyModifiers);
				}
				else if (can_consume(*this) && keyboard_event.trigger(native_window().current_event()))
					key_pressed(aScanCode, aKeyCode, aKeyModifiers);
			}
		}
	}

	void window::native_window_key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
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
		if (has_focused_widget())
		{
			i_widget* w = &focused_widget();
			while ((!can_consume(*w) || !w->keyboard_event.trigger(native_window().current_event()) || !w->key_released(aScanCode, aKeyCode, aKeyModifiers)) && w != this)
				w = &w->parent();
			if (w == this && can_consume(*this) && keyboard_event.trigger(native_window().current_event()))
				key_released(aScanCode, aKeyCode, aKeyModifiers);
		}
		else if (can_consume(*this) && keyboard_event.trigger(native_window().current_event()))
			key_released(aScanCode, aKeyCode, aKeyModifiers);
	}

	void window::native_window_text_input(const std::string& aText)
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
			if (has_focused_widget())
			{
				i_widget* w = &focused_widget();
				while ((!can_consume(*w) || !keyboard_event.trigger(native_window().current_event()) || !w->text_input(aText)) && w != this)
					w = &w->parent();
				if (w == this && can_consume(*this) && keyboard_event.trigger(native_window().current_event()))
					text_input(aText);
			}
			else if (can_consume(*this) && keyboard_event.trigger(native_window().current_event()))
				text_input(aText);
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

	void window::native_window_sys_text_input(const std::string& aText)
	{
		if (has_focused_widget())
		{
			i_widget* w = &focused_widget();
			while ((!keyboard_event.trigger(native_window().current_event()) || !w->sys_text_input(aText)) && w != this)
				w = &w->parent();
			if (w == this && keyboard_event.trigger(native_window().current_event()))
				sys_text_input(aText);
		}
		else
			sys_text_input(aText);
	}

	neogfx::mouse_cursor window::native_window_mouse_cursor() const
	{
		const i_widget& widgetUnderMouse = (iCapturingWidget == 0 ? widget_for_mouse_event(native_surface().mouse_position(), true) : *iCapturingWidget);
		return widgetUnderMouse.mouse_cursor();
	}

	void window::native_window_title_text_changed(const std::string& aTitleText)
	{
		if (iTitleBar != boost::none)
			iTitleBar->title().set_text(aTitleText);
	}

	const i_layout& window::non_client_layout() const
	{
		return iNonClientLayout;
	}

	i_layout& window::non_client_layout()
	{
		return iNonClientLayout;
	}

	const i_layout& window::title_bar_layout() const
	{
		return iTitleBarLayout;
	}

	i_layout& window::title_bar_layout()
	{
		return iTitleBarLayout;
	}

	const i_layout& window::menu_layout() const
	{
		return iMenuLayout;
	}

	i_layout& window::menu_layout()
	{
		return iMenuLayout;
	}

	const i_layout& window::toolbar_layout() const
	{
		return iToolbarLayout;
	}

	i_layout& window::toolbar_layout()
	{
		return iToolbarLayout;
	}

	const i_layout& window::client_layout() const
	{
		return iClientLayout;
	}

	i_layout& window::client_layout()
	{
		return iClientLayout;
	}

	const i_layout& window::status_bar_layout() const
	{
		return iStatusBarLayout;
	}

	i_layout& window::status_bar_layout()
	{
		return iStatusBarLayout;
	}

	void window::update_click_focus(i_widget& aCandidateWidget, const point& aClickPos)
	{
		if (aCandidateWidget.enabled() && (aCandidateWidget.focus_policy() & focus_policy::ClickFocus) == focus_policy::ClickFocus)
		{
			bool inClientArea = (aCandidateWidget.hit_test(aClickPos) == widget_part::Client);
			if (inClientArea ||
				(aCandidateWidget.focus_policy() & focus_policy::IgnoreNonClient) != focus_policy::IgnoreNonClient && (!has_focused_widget() || !focused_widget().is_descendent_of(aCandidateWidget)))
				aCandidateWidget.set_focus(inClientArea ? focus_reason::ClickClient : focus_reason::ClickNonClient);
		}
		else if (aCandidateWidget.has_parent() && (!has_focused_widget() || !focused_widget().is_descendent_of(aCandidateWidget)))
			update_click_focus(aCandidateWidget.parent(), aClickPos + aCandidateWidget.origin() - aCandidateWidget.parent().origin());
	}

	void window::update_modality()
	{
		for (std::size_t i = 0; i < app::instance().surface_manager().surface_count(); ++i)
		{
			i_surface& surface = app::instance().surface_manager().surface(i);
			if (surface.surface_type() == surface_type::Window && &surface != this)
			{
				window& windowSurface = static_cast<window&>(surface);
				if ((iStyle & window_style::ApplicationModal) == window_style::ApplicationModal)
					windowSurface.counted_enable(!iNativeWindow || iNativeWindowClosing);
				else if ((iStyle & window_style::Modal) == window_style::Modal && windowSurface.is_ancestor_of(*this, false))
					windowSurface.counted_enable(!iNativeWindow || iNativeWindowClosing);
			}
		}
	}

	void window::dismiss_children(const i_widget* aClickedWidget)
	{
		dismissing_children.trigger(aClickedWidget);
		neolib::scoped_flag sf{ iDismissingChildren };
		if ((iStyle & window_style::RequiresOwnerFocus) != window_style::RequiresOwnerFocus)
		{
			for (std::size_t i = 0; i < app::instance().surface_manager().surface_count();)
			{
				auto& s = app::instance().surface_manager().surface(i);
				if (!s.dismissed() && is_owner_of(s) && s.can_dismiss(aClickedWidget))
				{
					if (s.dismissal_type() == CloseOnDismissal)
						i = 0;
					else
						++i;
					s.dismiss();
				}
				else
					++i;
			}
		}
	}

	void window::mouse_entered()
	{
		i_widget& widgetUnderMouse = (iCapturingWidget == 0 ? widget_for_mouse_event(native_surface().mouse_position()) : *iCapturingWidget);
		i_widget* previousEnteredWidget = iEnteredWidget;
		iEnteredWidget = &widgetUnderMouse;
		if (iEnteredWidget != previousEnteredWidget)
		{
			if (previousEnteredWidget != 0)
				previousEnteredWidget->mouse_left();
			iEnteredWidget->mouse_entered();
		}
	}
}