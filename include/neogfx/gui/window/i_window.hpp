// i_window.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/hid/i_window_manager.hpp>
#include <neogfx/gui/window/window_events.hpp>
#include <neogfx/gui/window/window_bits.hpp>
#include <neogfx/gui/layout/layout_bits.hpp>
#include <neogfx/gui/widget/widget_bits.hpp>

namespace neogfx
{
	class i_native_surface;
	class i_native_window;
	class i_nested_window;
	class i_nest;
	class i_widget;
	class i_layout;

	class i_window
	{
		friend class window;
		friend class surface_window_proxy;
	public:
		event<window_event&> window_event;
		event<const i_widget*> dismissing_children;
		event<> closed;
	public:
		enum dismissal_type_e
		{
			CannotDismiss,
			CloseOnDismissal,
			HideOnDismissal
		};
	public:
		struct no_native_surface : std::logic_error { no_native_surface() : std::logic_error("neogfx::i_window::no_native_surface") {} };
		struct no_parent_window : std::logic_error { no_parent_window() : std::logic_error("neogfx::i_window::no_parent_window") {} };
		struct not_in_nest : std::logic_error { not_in_nest() : std::logic_error("neogfx::i_window::not_in_nest") {} };
		struct not_nested : std::logic_error { not_nested() : std::logic_error("neogfx::i_window::not_nested") {} };
		struct not_a_nest : std::logic_error { not_a_nest() : std::logic_error("neogfx::i_window::not_a_nest") {} };
	public:
		virtual const i_window_manager& window_manager() const = 0;
		virtual i_window_manager& window_manager() = 0;
	public:
		virtual bool is_surface() const = 0;
		virtual bool has_surface() const = 0;
		virtual const i_surface& surface() const = 0;
		virtual i_surface& surface() = 0;
		virtual bool has_native_surface() const = 0;
		virtual const i_native_surface& native_surface() const = 0;
		virtual i_native_surface& native_surface() = 0;
		virtual bool has_native_window() const = 0;
		virtual const i_native_window& native_window() const = 0;
		virtual i_native_window& native_window() = 0;
	public:
		virtual bool has_parent_window(bool aSameSurface = true) const = 0;
		virtual const i_window& parent_window() const = 0;
		virtual i_window& parent_window() = 0;
		virtual bool is_owner_of(const i_window& aChildWindow) const = 0;
		virtual const i_window& ultimate_ancestor() const = 0;
		virtual i_window& ultimate_ancestor() = 0;
	public:
		virtual const i_nest& nest() const = 0;
		virtual i_nest& nest() = 0;
		virtual bool is_nested() const = 0;
		virtual const i_nested_window& as_nested() const = 0;
		virtual i_nested_window& as_nested() = 0;
		virtual bool is_nest() const = 0;
		virtual const i_nest& as_nest() const = 0;
		virtual i_nest& as_nest() = 0;
	public:
		virtual bool is_strong() const = 0;
		virtual bool is_weak() const = 0;
	public:
		virtual bool can_close() const = 0;
		virtual bool is_closed() const = 0;
		virtual void close() = 0;
	public:
		virtual void widget_added(i_widget&) = 0;
		virtual void widget_removed(i_widget& aWidget) = 0;
	public:
		virtual window_style style() const = 0;
		virtual void set_style(window_style aStyle) = 0;
	public:
		virtual const std::string& title_text() const = 0;
		virtual void set_title_text(const std::string& aTitleText) = 0;
	public:
		virtual bool is_active() const = 0;
		virtual void activate() = 0;
		virtual bool is_iconic() const = 0;
		virtual void iconize() = 0;
		virtual bool is_maximized() const = 0;
		virtual void maximize() = 0;
		virtual bool is_restored() const = 0;
		virtual void restore() = 0;
		virtual bool is_fullscreen() const = 0;
		virtual point window_position() const = 0;
		virtual neogfx::window_placement window_placement() const = 0;
		virtual void set_window_placement(const neogfx::window_placement& aPlacement) = 0;
		virtual void centre(bool aSetMinimumSize = true) = 0;
		virtual void centre_on_parent(bool aSetMinimumSize = true) = 0;
		virtual bool window_enabled() const = 0;
		virtual void counted_window_enable(bool aEnable) = 0;
	public:
		virtual bool is_dismissing_children() const = 0;
		virtual bool can_dismiss(const i_widget* aClickedWidget) const = 0;
		virtual dismissal_type_e dismissal_type() const = 0;
		virtual bool dismissed() const = 0;
		virtual void dismiss() = 0;
	public:
		virtual bool has_rendering_priority() const = 0;
	public:
		virtual point mouse_position() const = 0;
	public:
		virtual rect widget_part_rect(widget_part aWidgetPart) const = 0;
	public:
		virtual const i_layout& non_client_layout() const = 0;
		virtual i_layout& non_client_layout() = 0;
		virtual const i_layout& title_bar_layout() const = 0;
		virtual i_layout& title_bar_layout() = 0;
		virtual const i_layout& menu_layout() const = 0;
		virtual i_layout& menu_layout() = 0;
		virtual const i_layout& toolbar_layout(layout_position aPosition = layout_position::Top) const = 0;
		virtual i_layout& toolbar_layout(layout_position aPosition = layout_position::Top) = 0;
		virtual const i_layout& dock_layout(layout_position aPosition = layout_position::Left) const =0;
		virtual i_layout& dock_layout(layout_position aPosition = layout_position::Left) = 0;
		virtual const i_layout& client_layout() const = 0;
		virtual i_layout& client_layout() = 0;
		virtual const i_layout& status_bar_layout() const = 0;
		virtual i_layout& status_bar_layout() = 0;
	public:
		virtual const i_widget& client_widget() const = 0;
		virtual i_widget& client_widget() = 0;
	public:
		virtual bool requires_owner_focus() const = 0;
		virtual bool has_entered_widget() const = 0;
		virtual i_widget& entered_widget() const = 0;
	public:
		virtual bool has_focused_widget() const = 0;
		virtual i_widget& focused_widget() const = 0;
		virtual void set_focused_widget(i_widget& aWidget, focus_reason aFocusReason) = 0;
		virtual void release_focused_widget(i_widget& aWidget) = 0;
	protected:
		virtual void update_modality(bool aEnableAncestors) = 0;
		virtual void update_click_focus(i_widget& aCandidateWidget, const point& aClickPos) = 0;
		virtual void dismiss_children(const i_widget* aClickedWidget = nullptr) = 0;
	public:
		virtual const i_widget& as_widget() const = 0;
		virtual i_widget& as_widget() = 0;
		// helpers
	public:
		const i_surface_window* find_surface() const
		{
			if (is_surface())
				return &surface().as_surface_window();
			else if (has_parent_window(false))
				return parent_window().find_surface();
			else
				return nullptr;
		}
		i_surface_window* find_surface()
		{
			return const_cast<i_surface_window*>(const_cast<const i_window*>(this)->find_surface());
		}
		bool is_ancestor_of(const i_window& aWindow) const
		{
			const i_window* w = &aWindow;
			while (w != this && w->has_parent_window(false))
				w = &w->parent_window();
			return w == this;
		}
	};

	rect corrected_popup_rect(i_window& aPopup, rect aPopupRect);
	rect corrected_popup_rect(i_window& aPopup);
	void correct_popup_rect(i_window& aPopup);

	class pause_rendering
	{
	public:
		pause_rendering(i_window& aWindow);
		~pause_rendering();
	private:
		i_surface* iSurface;
		neolib::optional_destroyed_flag iSurfaceDestroyed;
		neolib::optional_destroyed_flag iWindowDestroyed;
	};
}