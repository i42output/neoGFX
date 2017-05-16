// window.hpp
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
#include <string>
#include <neogfx/hid/video_mode.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include <neogfx/gui/widget/scrollable_widget.hpp>

namespace neogfx
{
	class i_native_window;

	class window : public i_window, public scrollable_widget
	{
	public:
		event<graphics_context&> paint_overlay;
	public:
		enum style_e : uint32_t
		{
			Invalid				= 0x000000,
			None				= 0x000001,	// No decoration at all(useful for splash screens, for example); this style cannot be combined with others
			Titlebar			= 0x000002,	// The window has a titlebar
			MinimizeBox			= 0x000004,
			MaximizeBox			= 0x000008,
			Resize				= 0x000010,	// The window can be resized and has a maximize button
			Close				= 0x000020,	// The window has a close button
			Fullscreen			= 0x000040,	// The window is shown in fullscreen mode; this style cannot be combined with others, and requires a valid video mode
			Modal				= 0x000080,
			ApplicationModal	= 0x001000,
			NoActivate			= 0x002000,
			RequiresOwnerFocus	= 0x004000,
			DismissOnOwnerClick = 0x008000,
			DismissOnParentClick= 0x010000,
			HideOnOwnerClick	= 0x020000,
			HideOnParentClick	= 0x040000,
			InitiallyHidden		= 0x080000,
			Weak				= 0x100000,
			Default				= Titlebar | MinimizeBox | MaximizeBox | Resize | Close
		};
	public:
		window(const video_mode& aVideoMode, style_e aStyle = Default, i_scrollbar::style_e aScrollbarStyle = i_scrollbar::Normal, framed_widget::style_e aFrameStyle = framed_widget::NoFrame);
		window(const video_mode& aVideoMode, const std::string& aWindowTitle, style_e aStyle = Default, i_scrollbar::style_e aScrollbarStyle = i_scrollbar::Normal, framed_widget::style_e aFrameStyle = framed_widget::NoFrame);
		window(const size& aDimensions, style_e aStyle = Default, i_scrollbar::style_e aScrollbarStyle = i_scrollbar::Normal, framed_widget::style_e aFrameStyle = framed_widget::NoFrame);
		window(const size& aDimensions, const std::string& aWindowTitle, style_e aStyle = Default, i_scrollbar::style_e aScrollbarStyle = i_scrollbar::Normal, framed_widget::style_e aFrameStyle = framed_widget::NoFrame);
		window(const point& aPosition, const size& aDimensions, style_e aStyle = Default, i_scrollbar::style_e aScrollbarStyle = i_scrollbar::Normal, framed_widget::style_e aFrameStyle = framed_widget::NoFrame);
		window(const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, style_e aStyle = Default, i_scrollbar::style_e aScrollbarStyle = i_scrollbar::Normal, framed_widget::style_e aFrameStyle = framed_widget::NoFrame);
		window(i_widget& aParent, const video_mode& aVideoMode, style_e aStyle = Default, i_scrollbar::style_e aScrollbarStyle = i_scrollbar::Normal, framed_widget::style_e aFrameStyle = framed_widget::NoFrame);
		window(i_widget& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, style_e aStyle = Default, i_scrollbar::style_e aScrollbarStyle = i_scrollbar::Normal, framed_widget::style_e aFrameStyle = framed_widget::NoFrame);
		window(i_widget& aParent, const size& aDimensions, style_e aStyle = Default, i_scrollbar::style_e aScrollbarStyle = i_scrollbar::Normal, framed_widget::style_e aFrameStyle = framed_widget::NoFrame);
		window(i_widget& aParent, const size& aDimensions, const std::string& aWindowTitle, style_e aStyle = Default, i_scrollbar::style_e aScrollbarStyle = i_scrollbar::Normal, framed_widget::style_e aFrameStyle = framed_widget::NoFrame);
		window(i_widget& aParent, const point& aPosition, const size& aDimensions, style_e aStyle = Default, i_scrollbar::style_e aScrollbarStyle = i_scrollbar::Normal, framed_widget::style_e aFrameStyle = framed_widget::NoFrame);
		window(i_widget& aParent, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, style_e aStyle = Default, i_scrollbar::style_e aScrollbarStyle = i_scrollbar::Normal, framed_widget::style_e aFrameStyle = framed_widget::NoFrame);
		~window();
	public:
		uint32_t style() const;
		void set_style(uint32_t aStyle);
		double fps() const;
	public:
		virtual bool is_root() const;
		virtual bool can_defer_layout() const;
		virtual bool is_managing_layout() const;
		virtual void layout_items_completed();
	public:
		virtual bool metrics_available() const;
		virtual size extents() const;
		virtual dimension horizontal_dpi() const;
		virtual dimension vertical_dpi() const;
		virtual dimension em_size() const;
	public:
		virtual const i_device_metrics& device_metrics() const;
		virtual units_e units() const;
		virtual units_e set_units(units_e aUnits) const;
	public:
		virtual void resized();
	public:
		virtual neogfx::size_policy size_policy() const;
	public:
		virtual colour background_colour() const;
	public:
		virtual bool is_weak() const;
		virtual bool can_close() const;
		virtual bool is_closed() const;
		virtual void close();
		virtual bool has_parent_surface() const;
		virtual const i_surface& parent_surface() const;
		virtual i_surface& parent_surface();
		virtual bool is_owner_of(const i_surface& aChildSurface) const;
		virtual bool is_dismissing_children() const;
		virtual bool can_dismiss(const i_widget* aClickedWidget) const;
		virtual dismissal_type_e dismissal_type() const;
		virtual bool dismissed() const;
		virtual void dismiss();
	public:
		virtual neogfx::surface_type surface_type() const;
		virtual neogfx::logical_coordinate_system logical_coordinate_system() const;
		virtual void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem);
		virtual const std::pair<vec2, vec2>& logical_coordinates() const;
		virtual void set_logical_coordinates(const std::pair<vec2, vec2>& aCoordinates);
		virtual void layout_surface();
		virtual void invalidate_surface(const rect& aInvalidatedRect, bool aInternal = true);
		virtual bool has_rendering_priority() const;
		virtual void render_surface();
		virtual void pause_rendering();
		virtual void resume_rendering();
		virtual graphics_context create_graphics_context() const;
		virtual graphics_context create_graphics_context(const i_widget& aWidget) const;
		virtual const i_native_surface& native_surface() const;
		virtual i_native_surface& native_surface();
		virtual const i_native_window& native_window() const;
		virtual i_native_window& native_window();
		virtual bool destroyed() const;
	public:
		virtual point surface_position() const;
		virtual void move_surface(const point& aPosition);
		virtual size surface_size() const;
		virtual void resize_surface(const size& aSize);
		virtual void centre();
		virtual void centre_on_parent();
	public:
		virtual point mouse_position() const;
		virtual bool is_mouse_button_pressed(mouse_button aButton) const;
	public:
		virtual void save_mouse_cursor();
		virtual void set_mouse_cursor(mouse_system_cursor aSystemCursor);
		virtual void restore_mouse_cursor();
		virtual void update_mouse_cursor();
	public:
		virtual void widget_added(i_widget& aWidget);
		virtual void widget_removed(i_widget& aWidget);
	public:
		virtual void show(bool aVisible);
		using scrollable_widget::show;
		virtual bool requires_owner_focus() const;
		virtual bool has_entered_widget() const;
		virtual i_widget& entered_widget() const;
		virtual bool has_capturing_widget() const;
		virtual i_widget& capturing_widget() const;
		virtual void set_capture(i_widget& aWidget);
		virtual void release_capture(i_widget& aWidget);
		virtual bool has_focused_widget() const;
		virtual i_widget& focused_widget() const;
		virtual void set_focused_widget(i_widget& aWidget);
		virtual void release_focused_widget(i_widget& aWidget);
	public:
		virtual bool is_active() const;
		virtual void activate();
		virtual void counted_enable(bool aEnable);
	public:
		virtual const i_surface& surface() const;
		virtual i_surface& surface();
		virtual bool is_surface() const;
	private:
		virtual bool native_window_can_close() const;
		virtual void native_window_closing();
		virtual void native_window_closed();
		virtual void native_window_focus_gained();
		virtual void native_window_focus_lost();
		virtual void native_window_resized();
		virtual bool native_window_has_rendering_priority() const;
		virtual bool native_window_ready_to_render() const;
		virtual void native_window_render(const rect& aInvalidatedRect) const;
		virtual void native_window_dismiss_children();
		virtual void native_window_mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta);
		virtual void native_window_mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
		virtual void native_window_mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers);
		virtual void native_window_mouse_button_released(mouse_button aButton, const point& aPosition);
		virtual void native_window_mouse_moved(const point& aPosition);
		virtual void native_window_mouse_entered();
		virtual void native_window_mouse_left();
		virtual void native_window_key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual void native_window_key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual void native_window_text_input(const std::string& aText);
		virtual void native_window_sys_text_input(const std::string& aText);
		virtual neogfx::mouse_cursor native_window_mouse_cursor() const;
	private:
		void init();
		void update_click_focus(i_widget& aCandidateWidget);
		void update_modality();
		void dismiss_children(const i_widget* aClickedWidget = 0);
	private:
		sink iSink;
		std::unique_ptr<i_native_window> iNativeWindow;
		style_e iStyle;
		mutable units_e iUnits;
		int32_t iCountedEnable;
		bool iNativeWindowClosing;
		bool iClosed;
		i_widget* iEnteredWidget;
		i_widget* iCapturingWidget;
		i_widget* iFocusedWidget;
		bool iDismissingChildren;
		boost::optional<char32_t> iSurrogatePairPart;
	};

	inline constexpr window::style_e operator|(window::style_e aLhs, window::style_e aRhs)
	{
		return static_cast<window::style_e>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
	}

	inline constexpr window::style_e operator&(window::style_e aLhs, window::style_e aRhs)
	{
		return static_cast<window::style_e>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
	}
}