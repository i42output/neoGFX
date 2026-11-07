// linux_basic_services.cpp
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

#include <neogfx/neogfx.hpp>

#include <thread>
#include <spawn.h>
#include <sys/wait.h>
#include <sys/resource.h>

#include "3rdparty/tinyfiledialogs.h"

#include <boost/algorithm/string.hpp>

#include <neogfx/app/i_app.hpp>
#include "../../hid/native/linux_display.hpp"
#include "i_native_clipboard.hpp"
#include "wayland_display_server.hpp"
#include "x11_display_server.hpp"
#include "linux_basic_services.hpp"

extern char** environ;

namespace neogfx
{
    namespace native::linux
    {
        namespace
        {
            // Wayland unless that is not to be had (or the NEOGFX_WINDOWING_SYSTEM environment variable says "x11"),
            // in which case X11 (which, on a Wayland desktop, is XWayland)
            std::unique_ptr<linux::display_server> connect_to_display_server()
            {
                std::string requested;
                if (auto const variable = std::getenv("NEOGFX_WINDOWING_SYSTEM"); variable != nullptr)
                    requested = boost::algorithm::to_lower_copy(std::string{ variable });
                std::string reasons;
                if (requested != "x11")
                {
                    try
                    {
                        return std::make_unique<wayland::display_server>();
                    }
                    catch (std::exception const& e)
                    {
                        if (requested == "wayland")
                            throw;
                        reasons = e.what();
                    }
                }
                try
                {
                    return std::make_unique<x11::display_server>();
                }
                catch (std::exception const& e)
                {
                    if (!reasons.empty())
                        reasons += "; ";
                    reasons += e.what();
                }
                throw std::runtime_error{ "neoGFX: no display server (Wayland or X11): " + reasons };
            }

            // the app thread's (the main thread's) stack: as the 100MB an app reserves on Windows (see CMakeLists.txt)
            // where its hard limit allows. This runs before main() as main()'s own frame may need it; the main thread's
            // stack grows on demand up to the soft limit current at the time (and the kernel leaves at least 128MB
            // below it to grow into, whatever the limit was when the program started).
            struct reserve_main_thread_stack
            {
                reserve_main_thread_stack()
                {
                    constexpr rlim_t STACK_SIZE = 100u * 1024u * 1024u;
                    rlimit stackLimit = {};
                    if (::getrlimit(RLIMIT_STACK, &stackLimit) == 0 && stackLimit.rlim_cur != RLIM_INFINITY && stackLimit.rlim_cur < STACK_SIZE)
                    {
                        stackLimit.rlim_cur = stackLimit.rlim_max == RLIM_INFINITY ? STACK_SIZE : std::min(STACK_SIZE, stackLimit.rlim_max);
                        ::setrlimit(RLIMIT_STACK, &stackLimit);
                    }
                }
            } const sReserveMainThreadStack;
        }

        basic_services::basic_services(i_async_task& aAppTask) :
            iAppTask{ aAppTask },
            iDisplayServer{ connect_to_display_server() }
        {
        }

        basic_services::~basic_services()
        {
        }

        platform basic_services::platform() const
        {
            return neogfx::platform::Linux;
        }

        windowing_system basic_services::windowing_system() const
        {
            return iDisplayServer->windowing_system();
        }

        environment basic_services::environment() const
        {
            // KDE or, as for every other Linux desktop (whose conventions, such as dialog button order, follow GTK's), GNOME
            if (auto const desktop = std::getenv("XDG_CURRENT_DESKTOP"); desktop != nullptr)
            {
                auto const desktops = boost::algorithm::to_upper_copy(std::string{ desktop });
                if (desktops.find("KDE") != std::string::npos)
                    return neogfx::environment::Kde;
            }
            return neogfx::environment::Gnome;
        }

        i_async_task& basic_services::app_task()
        {
            return iAppTask;
        }

        void* basic_services::helper_window_handle() const
        {
            return nullptr;
        }

        void basic_services::system_beep()
        {
            iDisplayServer->beep();
        }

        void basic_services::display_error_dialog(i_string const& aTitle, i_string const& aMessage, void* aParentWindowHandle) const
        {
            tinyfd_messageBox(aParentWindowHandle, aTitle.c_str(), aMessage.c_str(), "ok", "error", 1);
        }

        std::uint32_t basic_services::display_count() const
        {
            return iDisplayServer->output_count();
        }

        i_display& basic_services::display(std::uint32_t aDisplayIndex) const
        {
            if (iDisplays.size() != display_count())
            {
                iDisplays.clear();
                for (std::uint32_t i = 0u; i < display_count(); ++i)
                    iDisplays.push_back(std::make_unique<native::linux::display>(*iDisplayServer, i));
            }
            if (aDisplayIndex >= iDisplays.size())
                throw bad_display_index();
            return *iDisplays[aDisplayIndex];
        }

        bool basic_services::has_system_clipboard() const
        {
            return true;
        }

        i_native_clipboard& basic_services::system_clipboard()
        {
            return iDisplayServer->clipboard();
        }

        bool basic_services::has_system_menu_bar() const
        {
            return false;
        }

        i_shared_menu_bar& basic_services::system_menu_bar()
        {
            throw no_system_menu_bar();
        }

        bool basic_services::open_uri(std::string const& aUri)
        {
            char const* const arguments[] = { "xdg-open", aUri.c_str(), nullptr };
            pid_t child;
            if (::posix_spawnp(&child, "xdg-open", nullptr, nullptr, const_cast<char* const*>(arguments), environ) != 0)
                return false;
            // (xdg-open's exit is waited for on another thread: it exits once it has handed the URI on)
            std::thread{ [child]() { int status; ::waitpid(child, &status, 0); } }.detach();
            return true;
        }

        linux::display_server& basic_services::display_server() const
        {
            return *iDisplayServer;
        }
    }
}
