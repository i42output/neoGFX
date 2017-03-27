// i_widget.hpp
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
#include <unordered_set>
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/hid/mouse.hpp>
#include <neogfx/hid/i_keyboard.hpp>
#include "../layout/i_widget_geometry.hpp"

namespace neogfx
{
	class i_surface;
	class i_layout;

	enum class focus_policy : uint32_t
	{
		NoFocus				= 0x00000000,
		ClickFocus			= 0x00000001,
		TabFocus			= 0x00000002,
		ClickTabFocus		= ClickFocus | TabFocus,
		StrongFocus			= ClickTabFocus,
		WheelFocus			= 0x00000004,
		PointerFocus		= 0x00000008,
		SloppyFocus			= 0x00000010,
		ConsumeTabKey		= 0x10000000,
		ConsumeReturnKey	= 0x20000000
	};

	class i_widget : public i_widget_geometry, public i_units_context, public i_keyboard_handler
	{
	public:
		event<> visibility_changed;
		event<graphics_context&> painting;
	public:
		typedef std::vector<std::shared_ptr<i_widget>> widget_list;
	protected:
		typedef std::unordered_set<rect> update_rect_list;
	public:
		struct no_parent : std::logic_error { no_parent() : std::logic_error("neogfx::i_widget::no_parent") {} };
		struct no_children : std::logic_error { no_children() : std::logic_error("neogfx::i_widget::no_children") {} };
		struct not_child : std::logic_error { not_child() : std::logic_error("neogfx::i_widget::not_child") {} };
		struct no_update_rect : std::logic_error { no_update_rect() : std::logic_error("neogfx::i_widget::no_update_rect") {} };
		struct widget_not_entered : std::logic_error { widget_not_entered() : std::logic_error("neogfx::i_widget::widget_not_entered") {} };
		struct widget_not_capturing : std::logic_error { widget_not_capturing() : std::logic_error("neogfx::i_widget::widget_not_capturing") {} };
		struct widget_not_focused : std::logic_error { widget_not_focused() : std::logic_error("neogfx::i_widget::widget_not_focused") {} };
		struct widget_cannot_defer_layout : std::logic_error { widget_cannot_defer_layout() : std::logic_error("neogfx::i_widget::widget_cannot_defer_layout") {} };
		struct no_managing_layout : std::logic_error { no_managing_layout() : std::logic_error("neogfx::i_widget::no_managing_layout") {} };
		struct no_layout : std::logic_error { no_layout() : std::logic_error("neogfx::i_widget::no_layout") {} };
	public:
		virtual ~i_widget() {}
	public:
		virtual bool is_root() const = 0;
		virtual bool has_parent() const = 0;
		virtual const i_widget& parent() const = 0;
		virtual i_widget& parent() = 0;
		virtual void set_parent(i_widget& aParent) = 0;
		virtual void parent_changed() = 0;
		virtual const i_widget& ultimate_ancestor(bool aSameSurface = true) const = 0;
		virtual i_widget& ultimate_ancestor(bool aSameSurface = true) = 0;
		virtual bool is_ancestor_of(const i_widget& aWidget, bool aSameSurface = true) const = 0;
		virtual bool is_descendent_of(const i_widget& aWidget, bool aSameSurface = true) const = 0;
		virtual bool is_sibling_of(const i_widget& aWidget) const = 0;
		virtual void add_widget(i_widget& aWidget) = 0;
		virtual void add_widget(std::shared_ptr<i_widget> aWidget) = 0;
		virtual void remove_widget(i_widget& aWidget) = 0;
		virtual void remove_widgets() = 0;
		virtual bool has_children() const = 0;
		virtual const widget_list& children() const = 0;
		virtual widget_list::const_iterator last_child() const = 0;
		virtual widget_list::iterator last_child() = 0;
		virtual widget_list::const_iterator find_child(const i_widget& aChild, bool aThrowIfNotFound = true) const = 0;
		virtual widget_list::iterator find_child(const i_widget& aChild, bool aThrowIfNotFound = true) = 0;
		virtual const i_widget& before() const = 0;
		virtual i_widget& before() = 0;
		virtual const i_widget& after() const = 0;
		virtual i_widget& after() = 0;
		virtual void link_before(i_widget* aPreviousWidget) = 0;
		virtual void link_after(i_widget* aNextWidget) = 0;
		virtual void unlink() = 0;
		virtual bool has_surface() const = 0;
		virtual const i_surface& surface() const = 0;
		virtual i_surface& surface() = 0;
		virtual bool has_layout() const = 0;
		virtual void set_layout(i_layout& aLayout) = 0;
		virtual void set_layout(std::shared_ptr<i_layout> aLayout) = 0;
		virtual const i_layout& layout() const = 0;
		virtual i_layout& layout() = 0;
		virtual bool can_defer_layout() const = 0;
		virtual bool has_managing_layout() const = 0;
		virtual const i_widget& managing_layout() const = 0;
		virtual i_widget& managing_layout() = 0;
		virtual bool is_managing_layout() const = 0;
		virtual void layout_items(bool aDefer = false) = 0;
		virtual void layout_items_started() = 0;
		virtual bool layout_items_in_progress() const = 0;
		virtual void layout_items_completed() = 0;
	public:
		virtual neogfx::logical_coordinate_system logical_coordinate_system() const = 0;
		virtual point position() const = 0;
		virtual point origin(bool aNonClient = false) const = 0;
		virtual void move(const point& aPosition) = 0;
		virtual void moved() = 0;
		virtual size extents() const = 0;
		virtual void resize(const size& aSize) = 0;
		virtual void resized() = 0;
		virtual rect window_rect() const = 0;
		virtual rect client_rect(bool aIncludeMargins = true) const = 0;
		virtual const i_widget& widget_at(const point& aPosition) const = 0;
		virtual i_widget& widget_at(const point& aPosition) = 0;
	public:
		virtual void update(bool aIncludeNonClient = false) = 0;
		virtual void update(const rect& aUpdateRect) = 0;
		virtual bool requires_update() const = 0;
		virtual rect update_rect() const = 0;
		virtual rect default_clip_rect(bool aIncludeNonClient = false) const = 0;
		virtual bool ready_to_render() const = 0;
		virtual void render(graphics_context& aGraphicsContext) const = 0;
		virtual bool transparent_background() const = 0;
		virtual void paint_non_client(graphics_context& aGraphicsContext) const = 0;
		virtual void paint(graphics_context& aGraphicsContext) const = 0;
	public:
		virtual bool has_foreground_colour() const = 0;
		virtual colour foreground_colour() const = 0;
		virtual void set_foreground_colour(const optional_colour& aForegroundColour = optional_colour()) = 0;
		virtual bool has_background_colour() const = 0;
		virtual colour background_colour() const = 0;
		virtual void set_background_colour(const optional_colour& aBackgroundColour = optional_colour()) = 0;
		virtual bool has_font() const = 0;
		virtual const neogfx::font& font() const = 0;
		virtual void set_font(const optional_font& aFont) = 0;
	public:
		virtual bool visible() const = 0;
		virtual bool effectively_visible() const = 0;
		virtual bool hidden() const = 0;
		virtual bool effectively_hidden() const = 0;
		virtual void show(bool aVisible) = 0;
		virtual void show() = 0;
		virtual void hide() = 0;
		virtual bool enabled() const = 0;
		virtual bool effectively_enabled() const = 0;
		virtual bool disabled() const = 0;
		virtual bool effectively_disabled() const = 0;
		virtual void enable(bool aEnable) = 0;
		virtual void enable() = 0;
		virtual void disable() = 0;
		virtual bool entered() const = 0;
		virtual bool capturing() const = 0;
		virtual void set_capture() = 0;
		virtual void release_capture() = 0;
		virtual void captured() = 0;
		virtual void released() = 0;
		virtual neogfx::focus_policy focus_policy() const = 0;
		virtual void set_focus_policy(neogfx::focus_policy aFocusPolicy) = 0;
		virtual bool has_focus() const = 0;
		virtual void set_focus() = 0;
		virtual void release_focus() = 0;
		virtual void focus_gained() = 0;
		virtual void focus_lost() = 0;
	public:
		virtual bool ignore_mouse_events() const = 0;
		virtual void set_ignore_mouse_events(bool aIgnoreMouseEvents) = 0;
		virtual void mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta) = 0;
		virtual void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
		virtual void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) = 0;
		virtual void mouse_button_released(mouse_button aButton, const point& aPosition) = 0;
		virtual void mouse_moved(const point& aPosition) = 0;
		virtual void mouse_entered() = 0;
		virtual void mouse_left() = 0;
		virtual neogfx::mouse_cursor mouse_cursor() const = 0;
	public:
		virtual graphics_context create_graphics_context() const = 0;
	protected:
		virtual const update_rect_list& update_rects() const = 0;
	protected:
		virtual const i_widget& widget_for_mouse_event(const point& aPosition) const = 0;
		virtual i_widget& widget_for_mouse_event(const point& aPosition) = 0;
		// helpers
	public:
		bool same_surface(const i_widget& aWidget) const
		{
			if (has_surface() && aWidget.has_surface())
				return &surface() == &aWidget.surface();
			else
				return false;
		}
	};

	inline focus_policy operator|(focus_policy aLhs, focus_policy aRhs)
	{
		return static_cast<focus_policy>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline focus_policy& operator|=(focus_policy& aLhs, focus_policy aRhs)
	{
		aLhs = static_cast<focus_policy>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
		return aLhs;
	}

	inline focus_policy operator&(focus_policy aLhs, focus_policy aRhs)
	{
		return static_cast<focus_policy>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
	}

	inline focus_policy& operator&=(focus_policy& aLhs, focus_policy aRhs)
	{
		aLhs = static_cast<focus_policy>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
		return aLhs;
	}

	inline focus_policy operator~(focus_policy aLhs)
	{
		return static_cast<focus_policy>(~static_cast<uint32_t>(aLhs));
	}
}