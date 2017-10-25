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
#include <neogfx/gui/window/i_surface_window.hpp>
#include <neogfx/gui/widget/scrollable_widget.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/widget/title_bar.hpp>

namespace neogfx
{
	class i_native_window;

	class window : public i_surface_window, public scrollable_widget
	{
	public:
		event<graphics_context&> paint_overlay;
	private:
		typedef boost::optional<title_bar> optional_title_bar;
	public:
		window(const video_mode& aVideoMode, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		window(const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		window(const size& aDimensions, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		window(const size& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		window(const point& aPosition, const size& aDimensions, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		window(const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		window(i_widget& aParent, const video_mode& aVideoMode, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		window(i_widget& aParent, const video_mode& aVideoMode, const std::string& aWindowTitle, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		window(i_widget& aParent, const size& aDimensions, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		window(i_widget& aParent, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		window(i_widget& aParent, const point& aPosition, const size& aDimensions, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		window(i_widget& aParent, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		~window();
	private:
		window(window_style aStyle, scrollbar_style aScrollbarStyle, frame_style aFrameStyle);
	public:
		window_style style() const;
		void set_style(window_style aStyle);
		double fps() const;
	public:
		colour frame_colour() const override;
	public:
		bool is_root() const override;
		bool can_defer_layout() const override;
		bool is_managing_layout() const override;
		void layout_items_completed() override;
	public:
		bool metrics_available() const override;
		size extents() const override;
		dimension horizontal_dpi() const override;
		dimension vertical_dpi() const override;
		dimension em_size() const override;
	public:
		const i_device_metrics& device_metrics() const override;
		neogfx::units units() const override;
		neogfx::units set_units(neogfx::units aUnits) const override;
	public:
		void resized() override;
		widget_part hit_test(const point& aPosition) const override;
	public:
		neogfx::size_policy size_policy() const override;
	public:
		colour background_colour() const override;
	public:
		i_rendering_engine& rendering_engine() const override;
	public:
		bool is_weak() const override;
		bool can_close() const override;
		bool is_closed() const override;
		void close() override;
		bool has_parent_surface() const override;
		const i_surface& parent_surface() const override;
		i_surface& parent_surface() override;
		bool is_owner_of(const i_surface& aChildSurface) const override;
		bool is_dismissing_children() const override;
		bool can_dismiss(const i_widget* aClickedWidget) const override;
		dismissal_type_e dismissal_type() const override;
		bool dismissed() const override;
		void dismiss() override;
	public:
		bool is_window() const override;
		const i_surface_window& as_window() const override;
		i_surface_window& as_window() override;
	public:
		neogfx::surface_type surface_type() const override;
		neogfx::logical_coordinate_system logical_coordinate_system() const override;
		void set_logical_coordinate_system(neogfx::logical_coordinate_system aSystem) override;
		const std::pair<vec2, vec2>& logical_coordinates() const override;
		void set_logical_coordinates(const std::pair<vec2, vec2>& aCoordinates) override;
		void layout_surface() override;
		void invalidate_surface(const rect& aInvalidatedRect, bool aInternal = true) override;
		bool has_invalidated_area() const override;
		const rect& invalidated_area() const override;
		rect validate() override;
		bool has_rendering_priority() const override;
		void render_surface() override;
		void pause_rendering() override;
		void resume_rendering() override;
		graphics_context create_graphics_context() const override;
		graphics_context create_graphics_context(const i_widget& aWidget) const override;
		const i_native_surface& native_surface() const override;
		i_native_surface& native_surface() override;
		const i_native_window& native_window() const override;
		i_native_window& native_window() override;
		bool destroyed() const override;
	public:
		point surface_position() const override;
		void move_surface(const point& aPosition) override;
		size surface_size() const override;
		void resize_surface(const size& aSize) override;
		void centre() override;
		void centre_on_parent() override;
		void set_transparency(double aAlpha) override;
	public:
		point mouse_position() const override;
		bool is_mouse_button_pressed(mouse_button aButton) const override;
	public:
		void save_mouse_cursor() override;
		void set_mouse_cursor(mouse_system_cursor aSystemCursor) override;
		void restore_mouse_cursor() override;
		void update_mouse_cursor() override;
	public:
		void widget_added(i_widget& aWidget) override;
		void widget_removed(i_widget& aWidget) override;
	public:
		bool show(bool aVisible) override;
		using scrollable_widget::show;
		bool requires_owner_focus() const override;
		bool has_entered_widget() const override;
		i_widget& entered_widget() const override;
		bool has_capturing_widget() const override;
		i_widget& capturing_widget() const override;
		void set_capture(i_widget& aWidget) override;
		void release_capture(i_widget& aWidget) override;
		void non_client_set_capture(i_widget& aWidget) override;
		void non_client_release_capture(i_widget& aWidget) override;
		bool has_focused_widget() const override;
		i_widget& focused_widget() const override;
		void set_focused_widget(i_widget& aWidget, focus_reason aFocusReason) override;
		void release_focused_widget(i_widget& aWidget) override;
	public:
		neogfx::scrolling_disposition scrolling_disposition(const i_widget& aChildWidget) const override;
	public:
		const std::string& title_text() const override;
		void set_title_text(const std::string& aTitleText) override;
	public:
		bool is_active() const override;
		void activate() override;
		bool is_iconic() const override;
		void iconize() override;
		bool is_maximized() const override;
		void maximize() override;
		bool is_restored() const override;
		void restore() override;
		window_placement placement() const override;
		void set_placement(const window_placement& aPlacement) override;
		bool window_enabled() const override;
		void counted_window_enable(bool aEnable) override;
	public:
		const i_surface& surface() const override;
		i_surface& surface() override;
		bool is_surface() const override;
	private:
		bool current_event_is_non_client() const override;
		bool native_window_can_close() const override;
		void native_window_closing() override;
		void native_window_closed() override;
		void native_window_focus_gained() override;
		void native_window_focus_lost() override;
		void native_window_resized() override;
		bool native_window_has_rendering_priority() const override;
		bool native_window_ready_to_render() const override;
		void native_window_render(const rect& aInvalidatedRect) const override;
		void native_window_dismiss_children() override;
		void native_window_mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta) override;
		void native_window_mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
		void native_window_mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
		void native_window_mouse_button_released(mouse_button aButton, const point& aPosition) override;
		void native_window_mouse_moved(const point& aPosition) override;
		void native_window_non_client_mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta) override;
		void native_window_non_client_mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
		void native_window_non_client_mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override;
		void native_window_non_client_mouse_button_released(mouse_button aButton, const point& aPosition) override;
		void native_window_non_client_mouse_moved(const point& aPosition) override;
		void native_window_mouse_entered() override;
		void native_window_mouse_left() override;
		widget_part native_window_hit_test(const point& aPosition) const override;
		rect native_window_widget_part_rect(widget_part aWidgetPart) const override;
		void native_window_key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
		void native_window_key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
		void native_window_text_input(const std::string& aText) override;
		void native_window_sys_text_input(const std::string& aText) override;
		neogfx::mouse_cursor native_window_mouse_cursor() const override;
		void native_window_title_text_changed(const std::string& aTitleText) override;
	public:
		const i_layout& non_client_layout() const override;
		i_layout& non_client_layout() override;
		const i_layout& title_bar_layout() const override;
		i_layout& title_bar_layout() override;
		const i_layout& menu_layout() const override;
		i_layout& menu_layout() override;
		const i_layout& toolbar_layout() const override;
		i_layout& toolbar_layout() override;
		const i_layout& client_layout() const override;
		i_layout& client_layout() override;
		const i_layout& status_bar_layout() const override;
		i_layout& status_bar_layout() override;
	public:
		const i_widget& as_widget() const override;
		i_widget& as_widget() override;
	private:
		void init();
		void update_click_focus(i_widget& aCandidateWidget, const point& aClickPos);
		void update_modality();
		void dismiss_children(const i_widget* aClickedWidget = 0);
		void mouse_entered();
	private:
		sink iSink;
		std::unique_ptr<i_native_window> iNativeWindow;
		window_style iStyle;
		int32_t iCountedEnable;
		bool iNativeWindowClosing;
		bool iClosed;
		i_widget* iEnteredWidget;
		i_widget* iCapturingWidget;
		i_widget* iFocusedWidget;
		bool iDismissingChildren;
		boost::optional<char32_t> iSurrogatePairPart;
		vertical_layout iNonClientLayout;
		vertical_layout iTitleBarLayout;
		vertical_layout iMenuLayout;
		vertical_layout iToolbarLayout;
		vertical_layout iClientLayout;
		vertical_layout iStatusBarLayout;
		optional_title_bar iTitleBar;
	};
}