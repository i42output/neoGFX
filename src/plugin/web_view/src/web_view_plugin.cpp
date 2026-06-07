/*
  web_view_plugin.cpp

  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved

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

#include <neolib/app/version.hpp>
#include <cef/include/cef_app.h>

#include <neogfx/app/i_app.hpp>

#include "web_view_plugin.hpp"
#include "web_view_canvas.hpp"

namespace neogfx
{
    class WebViewApp : public CefApp, public CefBrowserProcessHandler
    {
    public:
        void OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> aCommandLine) final
        {
            aCommandLine->SetProgram("neogfx_cef.exe");
        }
        CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() final 
        {
            return this;
        }
    private:
        IMPLEMENT_REFCOUNTING(WebViewApp);
    };

    web_view_plugin::web_view_plugin(neolib::i_application& aApplication)
    {
    }

    web_view_plugin::~web_view_plugin()
    {
        if (iFactory)
            CefShutdown();
    }

    bool web_view_plugin::discover(const neolib::uuid& aId, void*& aObject)
    {
        if (aId == i_web_view_factory::iid())
        {
            if (!iFactory)
                init();
            aObject = &*iFactory;
            return true;
        }
        return false;
    }

    const neolib::uuid& web_view_plugin::id() const
    {
        static neolib::uuid sId{ 0x91a7cbb7, 0x2cba, 0x4ce1, 0x8125, { 0xb9, 0xe2, 0x1c, 0x39, 0xf5, 0x29 } };
        return sId;
    }

    const neolib::i_string& web_view_plugin::name() const
    {
        static neolib::string sName = "neogfx::web_view_plugin";
        return sName;
    }

    const neolib::i_string& web_view_plugin::description() const
    {
        static neolib::string sDescription = "neoGFX Web View Plugin";
        return sDescription;
    }

    const neolib::i_version& web_view_plugin::version() const
    {
        static neolib::version sVersion{ 0, 1, 0, 0, "Nukem" };
        return sVersion;
    }

    const neolib::i_string& web_view_plugin::copyright() const
    {
        static neolib::string sCopyright = "Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.";
        return sCopyright;
    }

    void web_view_plugin::init()
    {
        CefSettings settings;
        auto const settingsFolder = service<i_app>().info().settings_folder().to_std_string();
        CefString rootCachePath{ settingsFolder };
        settings.root_cache_path = *rootCachePath.GetWritableStruct();
        rootCachePath.Detach();
        settings.windowless_rendering_enabled = true;

        CefRefPtr<WebViewApp> app{ new WebViewApp };
        
        CefInitialize(CefMainArgs{}, settings, app.get(), nullptr);

        iSink = service<i_async_task>().idle_work([](){ CefDoMessageLoopWork(); });

        iFactory = neolib::make_ref<web_view_factory>();
    }

    void web_view_factory::create_canvas(i_widget& aParent, i_ref_ptr<i_web_view>& aWebView, i_optional<i_string> const& aUrl, bool aTransparent)
    {
        aWebView = neolib::make_ref<web_view_canvas>(aParent, aUrl, aTransparent);
    }

    void web_view_factory::create_canvas(i_layout& aLayout, i_ref_ptr<i_web_view>& aWebView, i_optional<i_string> const& aUrl, bool aTransparent)
    {
        aWebView = neolib::make_ref<web_view_canvas>(aLayout, aUrl, aTransparent);
    }
}
