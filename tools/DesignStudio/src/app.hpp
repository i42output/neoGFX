// app.hpp
/*
neoGFX Design Studio
Copyright(C) 2021 Leigh Johnston

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

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include "DesignStudio.ui.hpp"
#include <neogfx/tools/DesignStudio/project_manager.hpp>
#include <neogfx/tools/DesignStudio/project.hpp>
#include <neogfx/tools/DesignStudio/settings.hpp>
#include <neogfx/tools/DesignStudio/i_console_client.hpp>
#include <neogfx/audio/i_audio.hpp>
#include "main_window.hpp"

namespace neogfx::DesignStudio
{
    class app : public main_app, public reference_counted<i_console_client_manager>
    {
    public:
        define_declared_event(StartConsoleClientSession, start_console_client_session, i_terminal&, i_string const&, i_ref_ptr<i_console_client>&)
    public:
        template <typename... ArgsT>
        app(ArgsT&&... aArgs) :
            main_app{ std::forward<ArgsT>(aArgs)... }
        {
            reference_counted<i_console_client_manager>::add_ref();

            change_style("Dark");
            current_style().set_spacing(ng::size{ 4.0 });

            std::cout << "Loading element libraries..." << std::endl;
            plugin_manager().load_plugins();

            for (auto const& plugin : plugin_manager().plugins())
                std::cout << "Element library '" << plugin->name() << "' loaded." << std::endl;

            iSettings.emplace();
            iSettings->add_ref();
            iProjectManager.emplace();
            iMainWindow.emplace(*this, *iSettings, *iProjectManager);
            iMainWindow->reference_counted<i_ide>::add_ref();

            for (auto const& plugin : plugin_manager().plugins())
            {
                ref_ptr<i_element_library> elementLibrary;
                plugin->discover(elementLibrary);
                if (elementLibrary)
                    elementLibrary->ide_ready(*iMainWindow);
            }
        }
    protected:
        bool discover(const uuid& aId, void*& aObject) override
        {
            if (aId == i_ide::iid())
            {
                aObject = static_cast<i_ide*>(&*iMainWindow);
                return true;
            }
            else if (aId == i_settings::iid())
            {
                aObject = static_cast<i_settings*>(&*iSettings);
                return true;
            }
            else if (aId == i_console_client_manager::iid())
            {
                aObject = static_cast<i_console_client_manager*>(this);
                return true;
            }
            return main_app::discover(aId, aObject);
        }
    private:
        std::optional<settings> iSettings;
        std::optional<project_manager> iProjectManager;
        std::optional<main_window_ex> iMainWindow;
    };
}

