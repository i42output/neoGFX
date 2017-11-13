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
#include <neogfx/hid/surface_window_proxy.hpp>
#include "native/i_native_window.hpp"
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/window/window.hpp>

namespace neogfx
{
	pause_rendering::pause_rendering(i_window& aWindow) :
		iSurface{ app::instance().window_manager().has_window(aWindow) && aWindow.has_native_surface() ? &aWindow.surface() : nullptr }
	{
		if (iSurface != nullptr)
		{
			iSurfaceDestroyed.emplace(iSurface->as_destroyable());
			iWindowDestroyed.emplace(aWindow.as_destroyable());
			iSurface->pause_rendering();
		}
	}

	pause_rendering::~pause_rendering()
	{
		if (iSurface != nullptr && !*iWindowDestroyed && !*iSurfaceDestroyed)
			iSurface->resume_rendering();
	}

	class window::nested_details : public i_nested_window
	{
	public:
		nested_details(i_window& aSurrogate) : iSurrogate{ aSurrogate }
		{
		}
	public:
		const i_nested_window_container& nested_root() const override
		{
			return iSurrogate.nested_container();
		}
		i_nested_window_container& nested_root() override
		{
			return iSurrogate.nested_container();
		}
		bool has_nested_parent() const override
		{
			return iSurrogate.has_parent_window() && iSurrogate.parent_window().is_nested();
		}
		const i_nested_window& nested_parent() const override
		{
			if (has_nested_parent())
				return iSurrogate.parent_window().as_nested();
			throw no_nested_parent();
		}
		i_nested_window& nested_parent() override
		{
			return const_cast<i_nested_window&>(const_cast<const nested_details*>(this)->nested_parent());
		}
	public:
		const i_window& as_window() const override
		{
			return iSurrogate;
		}
		i_window& as_window() override
		{
			return iSurrogate;
		}
	private:
		i_window& iSurrogate;
	};

	window::window(window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		scrollable_widget{ aScrollbarStyle, aFrameStyle },
		iWindowManager{ app::instance().window_manager() },
		iClosed{ false },
		iStyle{ aStyle },
		iCountedEnable{ 0 },
		iEnteredWidget{ nullptr },
		iFocusedWidget{ nullptr },
		iDismissingChildren{ false },
		iNonClientLayout{ *this },
		iTitleBarLayout{ iNonClientLayout },
		iMenuLayout{ iNonClientLayout },
		iToolbarLayout{ iNonClientLayout },
		iClientLayout{ iNonClientLayout },
		iStatusBarLayout{ iNonClientLayout }
	{
		window_manager().add_window(*this);
	}

	window::window(const video_mode& aVideoMode, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		if ((style() & window_style::Nested) != window_style::Nested)
			iSurfaceWindow = std::make_unique<surface_window_proxy>(
				*this, 
				[&](i_surface_window& aProxy) { return app::instance().rendering_engine().create_window(app::instance().surface_manager(), aProxy, aVideoMode, app::instance().name(), aStyle); });
		init();
	}

	window::window(const size& aDimensions, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		if ((style() & window_style::Nested) != window_style::Nested)
			iSurfaceWindow = std::make_unique<surface_window_proxy>(
				*this, 
				[&](i_surface_window& aProxy) { return app::instance().rendering_engine().create_window(app::instance().surface_manager(), aProxy, aDimensions, app::instance().name(), aStyle); });
		init();
		centre_on_parent(false);
	}

	window::window(const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		if ((style() & window_style::Nested) != window_style::Nested)
			iSurfaceWindow = std::make_unique<surface_window_proxy>(
				*this, 
				[&](i_surface_window& aProxy) { return app::instance().rendering_engine().create_window(app::instance().surface_manager(), aProxy, aDimensions, aWindowTitle, aStyle); });
		init();
		centre_on_parent(false);
	}

	window::window(const point& aPosition, const size& aDimensions, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		if ((style() & window_style::Nested) != window_style::Nested)
			iSurfaceWindow = std::make_unique<surface_window_proxy>(
				*this, 
				[&](i_surface_window& aProxy) { return app::instance().rendering_engine().create_window(app::instance().surface_manager(), aProxy, aPosition, aDimensions, app::instance().name(), aStyle); });
		init();
	}

	window::window(const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		if ((style() & window_style::Nested) != window_style::Nested)
			iSurfaceWindow = std::make_unique<surface_window_proxy>(
				*this, 
				[&](i_surface_window& aProxy) { return app::instance().rendering_engine().create_window(app::instance().surface_manager(), aProxy, aPosition, aDimensions, aWindowTitle, aStyle); });
		init();
	}

	window::window(i_widget& aParent, const size& aDimensions, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		if ((style() & window_style::Nested) != window_style::Nested)
			iSurfaceWindow = std::make_unique<surface_window_proxy>(
				*this, 
				[&](i_surface_window& aProxy) { return app::instance().rendering_engine().create_window(app::instance().surface_manager(), aProxy, aParent.surface().native_surface(), aDimensions, app::instance().name(), aStyle); });
		set_parent(aParent.root().as_widget());
		init();
		centre_on_parent(false);
	}

	window::window(i_widget& aParent, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		if ((style() & window_style::Nested) != window_style::Nested)
			iSurfaceWindow = std::make_unique<surface_window_proxy>(
				*this, 
				[&](i_surface_window& aProxy) { return app::instance().rendering_engine().create_window(app::instance().surface_manager(), aProxy, aParent.surface().native_surface(), aDimensions, aWindowTitle, aStyle); });
		set_parent(aParent.root().as_widget());
		init();
		centre_on_parent(false);
	}

	window::window(i_widget& aParent, const point& aPosition, const size& aDimensions, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		if ((style() & window_style::Nested) != window_style::Nested)
			iSurfaceWindow = std::make_unique<surface_window_proxy>(
				*this, 
				[&](i_surface_window& aProxy) { return app::instance().rendering_engine().create_window(app::instance().surface_manager(), aProxy, aParent.surface().native_surface(), aPosition, aDimensions, app::instance().name(), aStyle); });
		set_parent(aParent.root().as_widget());
		init();
	}

	window::window(i_widget& aParent, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		window{ aStyle, aScrollbarStyle, aFrameStyle }
	{
		if ((style() & window_style::Nested) != window_style::Nested)
			iSurfaceWindow = std::make_unique<surface_window_proxy>(
				*this, 
				[&](i_surface_window& aProxy) { return app::instance().rendering_engine().create_window(app::instance().surface_manager(), aProxy, aParent.surface().native_surface(), aPosition, aDimensions, aWindowTitle, aStyle); });
		set_parent(aParent.root().as_widget());
		init();
	}

	window::~window()
	{
		update_modality(true);
		window_manager().remove_window(*this);
		destroyable::set_destroyed();
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
		if (has_surface())
			return native_window().fps();
		else
			return 0.0;
	}

	const i_window_manager& window::window_manager() const
	{
		return iWindowManager;
	}

	i_window_manager& window::window_manager()
	{
		return iWindowManager;
	}

	bool window::is_surface() const
	{
		return iSurfaceWindow != nullptr;
	}

	bool window::has_surface() const
	{
		return find_surface() != nullptr;
	}

	const i_surface_window& window::surface() const
	{
		auto result = find_surface();
		if (result != nullptr)
			return *result;
		throw no_surface();
	}

	i_surface_window& window::surface()
	{
		return const_cast<i_surface_window&>(const_cast<const window*>(this)->surface());
	}

	bool window::has_native_surface() const
	{
		return is_surface() && !*iSurfaceDestroyed;
	}

	const i_native_surface& window::native_surface() const
	{
		if (!has_native_surface())
			throw no_native_surface();
		return surface().native_surface();
	}

	i_native_surface& window::native_surface()
	{
		return const_cast<i_native_surface&>(const_cast<const window*>(this)->native_surface());
	}

	bool window::has_native_window() const
	{
		return has_native_surface() && surface().surface_type() == neogfx::surface_type::Window;
	}

	const i_native_window& window::native_window() const
	{
		return static_cast<const i_native_window&>(native_surface());
	}

	i_native_window& window::native_window()
	{
		return const_cast<i_native_window&>(const_cast<const window*>(this)->native_window());
	}

	bool window::has_parent_window(bool aSameSurface) const
	{
		return has_parent(aSameSurface);
	}

	const i_window& window::parent_window() const
	{
		if (parent().is_root())
			return parent().root();
		throw no_parent_window();
	}

	i_window& window::parent_window()
	{
		return const_cast<i_window&>(const_cast<const window*>(this)->parent_window());
	}

	bool window::is_owner_of(const i_window& aChildWindow) const
	{
		const i_window* w = &aChildWindow;
		if (w == this)
			return false;
		while (w->has_parent_window())
		{
			w = &w->parent_window();
			if (w == this)
				return true;
		}
		return false;
	}

	bool window::is_nested() const
	{
		return iNestedWindowDetails != nullptr;
	}

	const i_nested_window_container& window::nested_container() const
	{
		if (!has_parent_window())
			throw not_contained();
		const i_window* w = &parent_window();
		while (!w->is_nested_container() && w->has_parent_window())
			w = &w->parent_window();
		if (w->is_nested_container())
			return w->as_nested_container();
		throw not_contained();
	}

	i_nested_window_container& window::nested_container()
	{
		return const_cast<i_nested_window_container&>(const_cast<const window*>(this)->nested_container());
	}

	const i_nested_window& window::as_nested() const
	{
		return *iNestedWindowDetails;
	}

	i_nested_window& window::as_nested()
	{
		return const_cast<i_nested_window&>(const_cast<const window*>(this)->as_nested());
	}

	bool window::is_nested_container() const
	{
		return iNestedWindowContainer != boost::none;
	}

	const i_nested_window_container& window::as_nested_container() const
	{
		if (is_nested_container())
			return *iNestedWindowContainer;
		throw not_nested_container();
	}

	i_nested_window_container& window::as_nested_container()
	{
		return const_cast<i_nested_window_container&>(const_cast<const window*>(this)->as_nested_container());
	}

	bool window::is_strong() const
	{
		return !is_weak();
	}

	bool window::is_weak() const
	{
		return (window_style() & window_style::Weak) == window_style::Weak;
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
		destroyed_flag destroyed{ *this };
		if (is_surface())
			surface().close();
		if (destroyed)
			return;
		iClosed = true;
		closed.trigger();
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

	const i_window& window::root() const
	{
		return *this;
	}

	i_window& window::root()
	{
		return *this;
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
		i_widget& widgetUnderMouse = (!surface().has_capturing_widget() ? widget_for_mouse_event(mouse_position()) : surface().capturing_widget());
		if (iEnteredWidget != &widgetUnderMouse)
			mouse_entered();
	}

	bool window::device_metrics_available() const
	{
		return scrollable_widget::device_metrics_available();
	}

	const i_device_metrics& window::device_metrics() const
	{
		return scrollable_widget::device_metrics();
	}

	units window::units() const
	{
		return scrollable_widget::units();
	}

	units window::set_units(neogfx::units aUnits) const
	{
		return scrollable_widget::set_units(aUnits);
	}

	void window::resized()
	{
		window_manager().resize_window(*this, widget::extents());
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

	void window::render(graphics_context& aGraphicsContext) const
	{
		aGraphicsContext.set_extents(extents());
		aGraphicsContext.set_origin(origin());
		scrollable_widget::render(aGraphicsContext);
		aGraphicsContext.set_extents(extents());
		aGraphicsContext.set_origin(origin());
		paint_overlay.trigger(aGraphicsContext);
	}

	colour window::background_colour() const
	{
		if (has_background_colour())
			return scrollable_widget::background_colour();
		else
			return container_background_colour();
	}

	bool window::is_dismissing_children() const
	{
		return iDismissingChildren;
	}

	bool window::can_dismiss(const i_widget*) const
	{
		return true;
	}

	window::dismissal_type_e window::dismissal_type() const
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

	void window::centre(bool aSetMinimumSize)
	{
		layout_items();
		if (aSetMinimumSize)
			window_manager().resize_window(*this, minimum_size());
		rect desktopRect{ window_manager().desktop_rect(*this) };
		window_manager().move_window(*this, (desktopRect.extents() - window_manager().window_rect(*this).extents()) / 2.0);
	}

	void window::centre_on_parent(bool aSetMinimumSize)
	{
		if (has_parent_window(false))
		{
			layout_items();
			if (aSetMinimumSize)
				window_manager().resize_window(*this, minimum_size());
			rect desktopRect{ window_manager().desktop_rect(*this) };
			rect parentRect{ window_manager().window_rect(parent_window()) };
			rect ourRect{ window_manager().window_rect(*this) };
			point position = point{ (parentRect.extents() - ourRect.extents()) / 2.0 } + parentRect.top_left();
			if (position.x < 0.0)
				position.x = 0.0;
			if (position.y < 0.0)
				position.y = 0.0;
			if (position.x + ourRect.cx > desktopRect.right())
				position.x = desktopRect.right() - ourRect.cx;
			if (position.y + ourRect.cy > desktopRect.bottom())
				position.y = desktopRect.bottom() - ourRect.cy;
			window_manager().move_window(*this, position);
		}
		else
			centre(aSetMinimumSize);
	}

	void window::widget_added(i_widget&)
	{
		layout_items(true);
	}

	void window::widget_removed(i_widget& aWidget)
	{
		if (iEnteredWidget == &aWidget)
			iEnteredWidget = nullptr;
		if (surface().has_capturing_widget() && &surface().capturing_widget() == &aWidget)
			surface().release_capture(aWidget);
		if (iFocusedWidget == &aWidget)
			iFocusedWidget = nullptr;
		layout_items(true);
	}

	bool window::show(bool aVisible)
	{
		bool result = widget::show(aVisible);
		if (has_native_surface())
		{
			if (aVisible)
				native_window().show();
			else
				native_window().hide();
		}
		return result;
	}

	bool window::requires_owner_focus() const
	{
		return (iStyle & window_style::RequiresOwnerFocus) == window_style::RequiresOwnerFocus;
	}

	bool window::has_entered_widget() const
	{
		return iEnteredWidget != nullptr;
	}

	i_widget& window::entered_widget() const
	{
		if (iEnteredWidget == nullptr)
			throw widget_not_entered();
		return *iEnteredWidget;
	}

	bool window::has_focused_widget() const
	{
		return iFocusedWidget != nullptr;
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
		if (previouslyFocused != nullptr)
			previouslyFocused->focus_lost(aFocusReason);
		iFocusedWidget->focus_gained(aFocusReason);
	}

	void window::release_focused_widget(i_widget& aWidget)
	{
		if (iFocusedWidget != &aWidget)
			throw widget_not_focused();
		iFocusedWidget = nullptr;
		aWidget.focus_lost(focus_reason::Other);
	}

	void window::update_modality(bool aEnableAncestors)
	{
		for (std::size_t i = 0; i < window_manager().window_count(); ++i)
		{
			i_window& w = window_manager().window(i);
			if (&w != this)
			{
				if ((iStyle & window_style::ApplicationModal) == window_style::ApplicationModal)
					w.counted_window_enable(aEnableAncestors);
				else if ((iStyle & window_style::Modal) == window_style::Modal && w.as_widget().is_ancestor_of(*this, false))
					w.counted_window_enable(aEnableAncestors);
			}
		}
		if (aEnableAncestors && has_parent_window(false) && (style() & window_style::NoActivate) != window_style::NoActivate)
			parent_window().activate();
	}

	scrolling_disposition window::scrolling_disposition(const i_widget& aChildWidget) const
	{
		if (iTitleBar != boost::none && &aChildWidget == &*iTitleBar)
			return neogfx::scrolling_disposition::DontScrollChildWidget;
		return scrollable_widget::scrolling_disposition(aChildWidget);
	}

	const std::string& window::title_text() const
	{
		return iTitleText;
	}

	void window::set_title_text(const std::string& aTitleText)
	{
		if (iTitleText != aTitleText)
		{
			iTitleText = aTitleText;
			if (iTitleBar != boost::none)
				iTitleBar->title().set_text(iTitleText);
			if (has_native_window())
				native_window().set_title_text(aTitleText);
		}
	}

	bool window::is_active() const
	{
		return has_native_window() && native_window().is_active();
	}

	void window::activate()
	{
		if (has_native_window())
			native_window().activate();
	}

	bool window::is_iconic() const
	{
		return has_native_window() && native_window().is_iconic();
	}

	void window::iconize()
	{
		if (has_native_window())
			native_window().iconize();
	}

	bool window::is_maximized() const
	{
		return has_native_window() && native_window().is_maximized();
	}

	void window::maximize()
	{
		if (has_native_window())
			native_window().maximize();
	}

	bool window::is_restored() const
	{
		return has_native_window() && native_window().is_restored();
	}

	void window::restore()
	{
		if (has_native_window())
			native_window().restore();
	}

	point window::window_position() const
	{
		return window_manager().window_rect(*this).position();
	}

	window_placement window::window_placement() const
	{
		return neogfx::window_placement{}; // todo
	}

	void window::set_window_placement(const neogfx::window_placement& aPlacement)
	{
		// todo
	}

	bool window::window_enabled() const
	{
		if (is_surface())
			return has_native_window() && native_window().is_enabled();
		else
			return enabled();
	}

	void window::counted_window_enable(bool aEnable)
	{
		if (aEnable)
			++iCountedEnable;
		else
			--iCountedEnable;
		if (is_surface() && has_native_window())
			native_window().enable(iCountedEnable >= 0);
		else if (!is_surface())
			enable(iCountedEnable >= 0);
	}

	void window::init()
	{
		iSurfaceDestroyed.emplace(surface().native_surface().as_destroyable());

		if ((style() & window_style::Nested) == window_style::Nested)
			iNestedWindowDetails = std::make_unique<nested_details>(*this);

		update_modality(false);

		if ((style() & window_style::TitleBar) == window_style::TitleBar)
			iTitleBar.emplace(*this, title_bar_layout(), app::instance().default_window_icon(), native_window().title_text());

		set_margins(neogfx::margins{});
		iNonClientLayout.set_margins(neogfx::margins{});
		iNonClientLayout.set_spacing(0.0);
		iTitleBarLayout.set_margins(neogfx::margins{});
		iMenuLayout.set_margins(neogfx::margins{});
		iToolbarLayout.set_margins(neogfx::margins{});
		iClientLayout.set_margins(neogfx::optional_margins{});
		iStatusBarLayout.set_margins(neogfx::margins{});

		if (!is_nested())
			resize(native_surface().surface_size());
		else
			layout_items(true);

		iSink += app::instance().current_style_changed([this](style_aspect aAspect)
		{
			if ((aAspect & style_aspect::Colour) == style_aspect::Colour)
				surface().native_surface().invalidate(surface().surface_size());
		});

		init_scrollbars();
	}

	point window::mouse_position() const
	{
		return window_manager().mouse_position(*this);
	}

	rect window::widget_part_rect(widget_part aWidgetPart) const
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

	const i_widget& window::as_widget() const
	{
		return *this;
	}

	i_widget& window::as_widget()
	{
		return *this;
	}

	neolib::i_destroyable& window::as_destroyable()
	{
		return *this;
	}

	void window::mouse_entered()
	{
		i_widget& widgetUnderMouse = (!surface().has_capturing_widget() ? widget_for_mouse_event(mouse_position()) : surface().capturing_widget());
		i_widget* newEnteredWidget = &widgetUnderMouse;
		i_widget* oldEnteredWidget = iEnteredWidget;
		if (newEnteredWidget != oldEnteredWidget)
		{
			if (oldEnteredWidget != nullptr)
				oldEnteredWidget->mouse_left();
			iEnteredWidget = newEnteredWidget;
			iEnteredWidget->mouse_entered();
		}
	}

	void window::mouse_left()
	{
		i_widget* oldEnteredWidget = iEnteredWidget;
		if (oldEnteredWidget != nullptr)
		{
			iEnteredWidget = nullptr;
			oldEnteredWidget->mouse_left();
		}
	}

	const i_surface_window* window::find_surface() const
	{
		if (iSurfaceWindow != nullptr)
			return &*iSurfaceWindow;
		else if (has_parent_window())
			return parent_window().find_surface();
		else
			return nullptr;
	}

	void window::dismiss_children(const i_widget* aClickedWidget)
	{
		dismissing_children.trigger(aClickedWidget);
		neolib::scoped_flag sf{ iDismissingChildren };
		if ((iStyle & window_style::RequiresOwnerFocus) != window_style::RequiresOwnerFocus)
		{
			for (std::size_t i = 0; i < window_manager().window_count();)
			{
				i_window& w = window_manager().window(i);
				if (!w.dismissed() && is_owner_of(w) && w.can_dismiss(aClickedWidget))
				{
					if (w.dismissal_type() == CloseOnDismissal)
						i = 0;
					else
						++i;
					w.dismiss();
				}
				else
					++i;
			}
		}
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
}