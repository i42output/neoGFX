// linux_window.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include <neogfx/core/geometrical.hpp>
#include <neogfx/hid/video_mode.hpp>
#include <neogfx/hid/i_surface_window.hpp>
#include <neogfx/gui/window/window_bits.hpp>
#include "native_window.hpp"

namespace neogfx
{
    namespace native::linux
    {
        class display_server;

        // What a Wayland window and an X11 window have in common: neither display server sends anything like the
        // non-client mouse messages of Windows (nor does what Windows' DefWindowProc then does with them, e.g. moving
        // the window when its title bar is dragged), so a window that draws its own decoration (window_style::TitleBar)
        // makes them itself from its own hit testing.
        class window : public native_window
        {
        public:
            window(i_rendering_engine& aRenderingEngine, i_surface_manager& aSurfaceManager, i_surface_window& aWindow, window* aParent, window_style aStyle);
        public:
            void attach(i_native_surface& aSurface) override;
            void detach() override;
        public:
            bool has_parent() const final;
            const i_native_window& parent() const final;
            i_native_window& parent() final;
            bool is_nested() const final;
        public:
            bool initialising() const final;
            void initialisation_complete() final;
            point surface_position() const final;
            size surface_extents() const final;
        public:
            bool visible() const final;
            double transparency() const final;
            void set_transparency(double aTransparency) final;
            bool is_active() const final;
            void activate() override;
            void deactivate() final;
            bool is_iconic() const final;
            bool is_maximized() const final;
            bool is_restored() const final;
            bool is_fullscreen() const final;
            bool enabled() const final;
            void enable(bool aEnable) final;
            bool is_capturing() const final;
            void set_capture(bool aLockCursor = true) final;
            void release_capture() final;
            bool is_non_client_capturing() const final;
            void non_client_set_capture() final;
            void non_client_release_capture() final;
            border border_thickness() const final;
        public:
            void render(bool aOOBRequest = false) final;
        public:
            bool is_popup() const;
            window* toplevel();
            linux::display_server& display_server() const;
            // (from the display server; positions are relative to the window, in pixels)
            void pointer_entered(point const& aPosition);
            void pointer_left();
            void pointer_moved(point const& aPosition);
            void pointer_button(mouse_button aButton, bool aPressed, std::uint32_t aSerial);
            void pointer_wheel(mouse_wheel aWheel, delta const& aDelta);
            void keyboard_focus_gained();
            void keyboard_focus_lost();
        protected:
            void set_destroying() override;
            void set_destroyed() override;
        protected:
            bool custom_decoration() const;
            void request_paint();
            void set_position(point const& aPosition, bool aExternalCause);
            void set_extents(size const& aExtents, bool aExternalCause);
            void set_maximized(bool aMaximized);
            void set_iconic(bool aIconic);
        private:
            // (what DefWindowProc does on Windows; aSerial is that of the button press, where the display server has one)
            virtual void begin_move(point const& aPosition, std::uint32_t aSerial) = 0;
            virtual void begin_resize(widget_part_e aBorder, point const& aPosition, std::uint32_t aSerial) = 0;
            virtual void show_window_menu(point const& aPosition, std::uint32_t aSerial) = 0;
            virtual void extents_changed() {}
            virtual void position_changed() {}
            void handle_non_client_button(mouse_button aButton, bool aPressed, point const& aPosition, std::uint32_t aSerial);
        protected:
            window* iParent;
            window_style iStyle;
            point iPosition;
            size iExtents;
            bool iVisible = false;
            bool iActive = false;
            bool iMaximized = false;
            bool iIconic = false;
            double iOpacity = 1.0;
        private:
            bool iReady = false;
            bool iEnabled = true;
            bool iFirstActivation = true;
            bool iMouseEntered = false;
            bool iCapturingMouse = false;
            bool iNonClientCapturing = false;
            point iPointerPosition;
            struct button_press
            {
                mouse_button button;
                point position;
                std::chrono::steady_clock::time_point time;
            };
            std::optional<button_press> iLastButtonPress;
            struct press
            {
                mouse_button button;
                point position;
                bool doubleClick;
            };
            std::optional<press> iPress;
            widget_part_e iClickedWidgetPart = widget_part::Nowhere;
            mutable border iBorderThickness;
            sink iSurfaceSink;
        };
    }
}
