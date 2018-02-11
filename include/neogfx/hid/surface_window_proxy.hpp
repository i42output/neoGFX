// surface_window_proxy.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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
#include <neolib/destroyable.hpp>
#include <neogfx/hid/i_surface_window.hpp>

namespace neogfx
{
	class i_native_window;

	class surface_window_proxy : public i_surface_window, protected neolib::destroyable
	{
	public:
		surface_window_proxy(i_window& aWindow, std::function<std::unique_ptr<i_native_window>(i_surface_window&)> aNativeWindowCreator);
		~surface_window_proxy();
	public:
		dimension horizontal_dpi() const override;
		dimension vertical_dpi() const override;
		bool metrics_available() const override;
		size extents() const override;
		dimension em_size() const override;
	public:
		i_rendering_engine& rendering_engine() const override;
	public:
		bool is_strong() const override;
		bool is_weak() const override;
		bool can_close() const override;
		bool is_closed() const override;
		void close() override;
		bool has_parent_surface() const override;
		const i_surface& parent_surface() const override;
		i_surface& parent_surface() override;
		bool is_owner_of(const i_surface& aChildSurface) const override;
	public:
		bool is_window() const override;
		const i_surface_window& as_surface_window() const override;
		i_surface_window& as_surface_window() override;
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
		bool has_native_surface() const override;
		const i_native_surface& native_surface() const override;
		i_native_surface& native_surface() override;
		bool has_native_window() const override;
		const i_native_window& native_window() const override;
		i_native_window& native_window() override;
	public:
		point surface_position() const override;
		void move_surface(const point& aPosition) override;
		size surface_size() const override;
		void resize_surface(const size& aSize) override;
		double surface_opacity() const override;
		void set_surface_opacity(double aOpacity) override;
		double surface_transparency() const override;
		void set_surface_transparency(double aTransparency) override;
	public:
		bool has_clicked_widget() const override;
		i_widget& clicked_widget() const override;
		bool has_capturing_widget() const override;
		i_widget& capturing_widget() const override;
		void set_capture(i_widget& aWidget) override;
		void release_capture(i_widget& aWidget) override;
		void non_client_set_capture(i_widget& aWidget) override;
		void non_client_release_capture(i_widget& aWidget) override;
	private:
		bool current_event_is_non_client() const override;
		bool is_closing() const override;
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
		window_style style() const override;
		void set_style(window_style aStyle) override;
	public:
		const i_window& as_window() const override;
		i_window& as_window() override;
		const i_widget& as_widget() const override;
		i_widget& as_widget()  override;
	public:
		neolib::i_destroyable& as_destroyable() override;
	private:
		i_window& iWindow;
		i_rendering_engine& iRenderingEngine;
		std::unique_ptr<i_native_window> iNativeWindow;
		destroyed_flag iNativeSurfaceDestroyed;
		bool iNativeWindowClosing;
		bool iClosed;
		boost::optional<char32_t> iSurrogatePairPart;
		i_widget* iCapturingWidget;
		i_widget* iClickedWidget;
	};
}