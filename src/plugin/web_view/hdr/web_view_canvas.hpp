/*
  web_view_canvas.hpp

  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.

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

#include <cef/include/cef_client.h>
#include <cef/include/cef_browser.h>
#include <cef/include/cef_load_handler.h>
#include <cef/include/cef_render_handler.h>
#include <cef/include/cef_context_menu_handler.h>
#include <cef/include/cef_focus_handler.h>

#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/widget/i_web_view.hpp>

namespace neogfx
{
    class web_view_canvas : public widget<i_web_view>, 
        public CefClient, public CefLoadHandler, public CefKeyboardHandler, public CefContextMenuHandler,
        public CefRenderHandler, public CefDisplayHandler, public CefFocusHandler
    {
        using base_type = widget<i_web_view>;
        // events
    public:
        define_declared_event(NavigatedTo, navigated_to, i_string const&)
        // types
    public:
        using abstract_type = i_web_view;
        // construction
    public:
        web_view_canvas(i_widget& aParent, i_optional<i_string> const& aUrl);
        web_view_canvas(i_layout& aLayout, i_optional<i_string> const& aUrl);
        ~web_view_canvas();
        // i_widget
    public:
        neogfx::logical_coordinate_system logical_coordinate_system() const final;
        void resized() final;
        void paint(i_graphics_context& aGc) const final;
        bool mouse_wheel_scrolled(mouse_wheel aWheel, const point& aPosition, delta aDelta, key_modifiers_e aKeyModifiers) final;
        void mouse_button_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) final;
        void mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) final;
        void mouse_button_released(mouse_button aButton, const point& aPosition) final;
        void mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) final;
        void mouse_entered(const point& aPosition) final;
        void mouse_left() final;
        neogfx::mouse_cursor mouse_cursor() const final;
        void capture_released() final;
        neogfx::focus_policy focus_policy() const final;
        void focus_gained(focus_reason aFocusReason) final;
        void focus_lost(focus_reason aFocusReason) final;
        // i_web_view
    public:
        void load_url(i_string const& aUrl, bool aSetFocus = true) final;
        // implementation
    private:
        void init();
        i_texture& back_buffer() const;
        // CefClient
    private:
        IMPLEMENT_REFCOUNTING(web_view_canvas);
        CefRefPtr<CefLoadHandler> GetLoadHandler() final;
        CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() final;
        CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() final;
        CefRefPtr<CefDisplayHandler> GetDisplayHandler() final;
        CefRefPtr<CefRenderHandler> GetRenderHandler() final;
        CefRefPtr<CefFocusHandler> GetFocusHandler() final;
        // CefLoadHandler
    private:
        void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, TransitionType transition_type) final;
        void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) final;
        void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, 
            const CefString& failedUrl) final;
        // CefKeyboardHandler
    private:
        bool OnPreKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event, 
            bool* is_keyboard_shortcut) final;
        // CefContextMenuHandler
    private:
        void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) final;
        // CefRenderHandler
    private:
        bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) final;
        void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) final;
        bool GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY) final;
        void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects,
            const void* buffer, int width, int height) final;
        bool OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, cef_cursor_type_t type, 
            const CefCursorInfo& custom_cursor_info) final;
        // CefFocusHandler
    private:
        void OnTakeFocus(CefRefPtr<CefBrowser> browser, bool next) final;
        bool OnSetFocus(CefRefPtr<CefBrowser> browser, FocusSource source) final;
        void OnGotFocus(CefRefPtr<CefBrowser> browser) final;
    private:
        std::optional<std::string> iUrl;
        std::optional<bool> iSetFocusAfterLoad;
        CefRefPtr<CefBrowser> iBrowser;
        mutable ref_ptr<i_texture> iBackBuffer;
        std::uint32_t iLastClickCount = 1u;
        cef_cursor_type_t iCursorType = CT_POINTER;
        sink iSink;
    };
}
