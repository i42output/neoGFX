// app.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <string>
#include <atomic>
#include <filesystem>
#include <boost/locale.hpp> 

#include <neolib/file/file.hpp>
#include <neolib/file/xml.hpp>
#include <neolib/core/scoped.hpp>
#include <neolib/core/string_utils.hpp>
#include <neolib/task/event.hpp>
#include <neolib/app/power.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/gfx/i_gradient_manager.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/hid/surface_manager.hpp>
#include <neogfx/hid/i_hid_devices.hpp>
#include <neogfx/app/resource_manager.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/widget/i_menu.hpp>
#include <neogfx/app/i_clipboard.hpp>
#include <neogfx/core/i_transition_animator.hpp>
#include <neogfx/gui/window/i_native_window.hpp>

template<> neolib::i_async_task& services::start_service<neolib::i_async_task>()
{
    return neogfx::app::instance().thread();
}

template<> neogfx::i_app& services::start_service<neogfx::i_app>()
{
    return neogfx::app::instance();
}

namespace neogfx
{
    program_options::program_options(int argc, char* argv[])
    {
        boost::program_options::options_description description{ "Allowed options" };
        description.add_options()
            ("console", "open console")
            ("fullscreen", boost::program_options::value<std::string>()->implicit_value(""s), "run full screen")
            ("dpi", boost::program_options::value<std::string>()->implicit_value(""s), "DPI override")
            ("nest", "display child windows nested within main window rather than using the main desktop")
            ("vulkan", "use Vulkan renderer")
            ("directx", "use DirectX (ANGLE) renderer")
            ("software", "use software renderer")
            ("turbo", "use turbo mode")
            ("double-buffer", "enable window double buffering");
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), iOptions);
        if (options().count("vulkan") + options().count("directx") + options().count("software") > 1)
            throw invalid_options("more than one renderer specified");
    }

    const boost::program_options::variables_map& program_options::options() const
    {
        return iOptions;
    }

    bool program_options::console() const
    {
        return options().count("console") == 1;
    }

    neogfx::renderer program_options::renderer() const
    {
        if (options().count("vulkan") == 1)
            return neogfx::renderer::Vulkan;
        else if (options().count("directx") == 1)
            return neogfx::renderer::DirectX;
        else if (options().count("software") == 1)
            return neogfx::renderer::Software;
        else
            return neogfx::renderer::OpenGL;
    }

    std::optional<size_u32> program_options::full_screen() const
    {
        if (options().count("fullscreen") == 1)
        {
            auto screenResolution = iOptions["fullscreen"].as<std::string>();
            if (screenResolution.empty())
                return size_u32{};
            else
            {
                neolib::vecarray<std::string, 2> bits;
                neolib::tokens(screenResolution, ","s, bits, 2, false, false);
                if (bits.size() == 2)
                {
                    auto result = size_u32{ boost::lexical_cast<std::uint32_t>(bits[0]), boost::lexical_cast<std::uint32_t>(bits[1]) };
                    if (result.cx != 0 && result.cx != 0)
                        return result;
                }
            }
            throw invalid_options("invalid fullscreen resolution");
        }
        return std::optional<size_u32>{};
    }

    std::optional<size_u32> program_options::dpi_override() const
    {
        if (options().count("dpi") == 1)
        {
            auto dpiOverride = iOptions["dpi"].as<std::string>();
            if (dpiOverride.empty())
                return std::optional<size_u32>{};
            else
            {
                neolib::vecarray<std::string, 2> bits;
                neolib::tokens(dpiOverride, ","s, bits, 2, false, false);
                if (bits.size() == 2)
                {
                    auto result = size_u32{ boost::lexical_cast<std::uint32_t>(bits[0]), boost::lexical_cast<std::uint32_t>(bits[1]) };
                    if (result.cx != 0 && result.cx != 0)
                        return result;
                }
            }
            throw invalid_options("invalid dpi override");
        }
        return std::optional<size_u32>{};
    }

    bool program_options::turbo() const
    {
        return options().count("turbo") == 1;
    }

    bool program_options::nest() const
    {
        return options().count("nest") == 1;
    }

    namespace
    {
        std::atomic<app*> sFirstInstance;
    }

    app::loader::loader(const neogfx::program_options& aProgramOptions, app& aApp) : iApp(aApp)
    {
        app* np = nullptr;
        sFirstInstance.compare_exchange_strong(np, &aApp);
        if (sFirstInstance == &aApp)
        {
#if defined(_WIN32)
            if (aProgramOptions.console())
            {
                AllocConsole();
                freopen("CONOUT$", "w", stdout);
                freopen("CONOUT$", "w", stderr);
                std::cout.sync_with_stdio(false);
                std::cout.sync_with_stdio(true);
                std::cerr.sync_with_stdio(false);
                std::cerr.sync_with_stdio(true);
            }
#endif
            service<i_rendering_engine>().initialize();
        }
    }

    app::loader::~loader()
    {
        iApp.iThread->async_task::cancel();
        iApp.plugin_manager().unload_plugins();
        iApp.iThread.reset();
        teardown_service<i_animator>();
        teardown_service<i_gradient_manager>();
        teardown_service<i_rendering_engine>();
        app* tp = &iApp;
        app* np = nullptr;
        sFirstInstance.compare_exchange_strong(tp, np);
    }

    app::app(const neolib::i_application_info& aAppInfo)
        try :
        neolib::application<object<i_app>>{ aAppInfo },
        iThread{ std::make_unique<app_thread>("neogfx::app", true) },
        iProgramOptions{ aAppInfo.arguments().argc(), aAppInfo.arguments().argv() },
        iLoader{ std::make_unique<loader>(iProgramOptions, *this) },
        iName{ aAppInfo.name() },
        iQuitWhenLastWindowClosed{ true },
        iInExec{ false },
        iDefaultWindowIcon{ image{ ":/neogfx/resources/icons/neoGFX.png" } },
        iCurrentStyle{ iStyles.begin() },
        iStandardActionManager{ thread(), *this, [this](neolib::callback_timer& aTimer)
        {
            aTimer.again();
            if (service<i_clipboard>().sink_active())
            {
                auto& sink = service<i_clipboard>().active_sink();
                if (sink.can_undo())
                    actionUndo.enable();
                else
                    actionUndo.disable();
                if (sink.can_redo())
                    actionRedo.enable();
                else
                    actionRedo.disable();
                if (sink.can_cut())
                    actionCut.enable();
                else
                    actionCut.disable();
                if (sink.can_copy())
                    actionCopy.enable();
                else
                    actionCopy.disable();
                if (sink.can_paste())
                    actionPaste.enable();
                else
                    actionPaste.disable();
                if (sink.can_delete_selected())
                    actionDelete.enable();
                else
                    actionDelete.disable();
                if (sink.can_select_all())
                    actionSelectAll.enable();
                else
                    actionSelectAll.disable();
            }
            else
            {
                actionUndo.disable();
                actionRedo.disable();
                actionCut.disable();
                actionCopy.disable();
                actionPaste.disable();
                actionDelete.disable();
                actionSelectAll.disable();
            }
        }, std::chrono::milliseconds{ 100 } },
        iAppContext{ thread(), "neogfx::app::iAppContext" },
        actionFileNew{ "&New..."_t, ":/neogfx/resources/icons/new.png" },
        actionFileOpen{ "&Open..."_t, ":/neogfx/resources/icons/open.png" },
        actionFileClose{ "&Close"_t },
        actionFileCloseAll{ "Close All"_t },
        actionFileSave{ "&Save"_t, ":/neogfx/resources/icons/save.png" },
        actionFileSaveAll{ "Save A&ll"_t },
        actionFileExit{ "E&xit"_t },
        actionUndo{ "Undo"_t, ":/neogfx/resources/icons/undo.png" },
        actionRedo{ "Redo"_t, ":/neogfx/resources/icons/redo.png" },
        actionCut{ "Cut"_t, ":/neogfx/resources/icons/cut.png" },
        actionCopy{ "Copy"_t, ":/neogfx/resources/icons/copy.png" },
        actionPaste{ "Paste"_t, ":/neogfx/resources/icons/paste.png" },
        actionDelete{ "Delete"_t },
        actionSelectAll{ "Select All"_t }
    {
        base_type::pin();
        thread().pin();
            
        neolib::event_mutex().set_multi_threaded_spinlock();

        actionFileNew.set_shortcut("Ctrl+Shift+N");
        actionFileOpen.set_shortcut("Ctrl+Shift+O");
        actionFileClose.set_shortcut("Ctrl+F4");
        actionFileSave.set_shortcut("Ctrl+S");
        actionFileSaveAll.set_shortcut("Ctrl+Shift+S");
        actionFileExit.set_shortcut("Alt+F4");
        actionUndo.set_shortcut("Ctrl+Z");
        actionRedo.set_shortcut("Ctrl+Shift+Z");
        actionCut.set_shortcut("Ctrl+X");
        actionCopy.set_shortcut("Ctrl+C");
        actionPaste.set_shortcut("Ctrl+V");
        actionDelete.set_shortcut("Del");
        actionSelectAll.set_shortcut("Ctrl+A");

        service<i_basic_services>();
        service<i_hid_devices>().enumerate_devices();
        service<i_keyboard>().grab_keyboard(*this);

        if (program_options().full_screen())
            service<i_surface_manager>().display().enter_fullscreen(video_mode{ *program_options().full_screen() });

        if (program_options().turbo())
            neolib::service<neolib::i_power>().enable_turbo_mode();

        style lightStyle("Light");
        register_style(lightStyle);
        style darkStyle("Dark");
        darkStyle.palette().set_color(color_role::Theme, color{ 0x35, 0x35, 0x35 });
        register_style(darkStyle);

        add_action(actionFileNew);
        add_action(actionFileOpen);
        add_action(actionFileClose);
        add_action(actionFileCloseAll);
        add_action(actionFileSave);
        add_action(actionFileSaveAll);
        add_action(actionFileExit);
        add_action(actionUndo);
        add_action(actionRedo);
        add_action(actionCut);
        add_action(actionCopy);
        add_action(actionPaste);
        add_action(actionDelete);
        add_action(actionSelectAll);

        actionFileExit.triggered([this]() { quit(0); });
        actionUndo.triggered([this]() { service<i_clipboard>().active_sink().undo(service<i_clipboard>()); });
        actionRedo.triggered([this]() { service<i_clipboard>().active_sink().redo(service<i_clipboard>()); });
        actionCut.triggered([this]() { service<i_clipboard>().cut(); });
        actionCopy.triggered([this]() { service<i_clipboard>().copy(); });
        actionPaste.triggered([this]() { service<i_clipboard>().paste(); });
        actionDelete.triggered([this]() { service<i_clipboard>().delete_selected(); });
        actionSelectAll.triggered([this]() { service<i_clipboard>().select_all(); });
    
        load_translations();

        plugin_manager().plugin_load_failure([](i_string const& reason)
        {
            service<i_basic_services>().display_error_dialog("Plugin Load Failure"_t, reason);
        });
    }
    catch (std::exception& e)
    {
        service<debug::logger>() << neolib::logger::severity::Debug << "neogfx::app::app: terminating with exception: " << e.what() << std::endl;
        service<i_basic_services>().display_error_dialog(aAppInfo.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + aAppInfo.name(), std::string("main: terminating with exception: ") + e.what());
        throw;
    }
    catch (...)
    {
        service<debug::logger>() << neolib::logger::severity::Debug << "neogfx::app::app: terminating with unknown exception" << std::endl;
        service<i_basic_services>().display_error_dialog(aAppInfo.name().empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + aAppInfo.name(), "main: terminating with unknown exception");
        throw;
    }

    app::~app()
    {
        service<i_keyboard>().ungrab_keyboard(*this);
        service<i_resource_manager>().clean();
    }

    app& app::instance()
    {
        app* instance = sFirstInstance.load();
        if (instance == nullptr)
            throw no_instance();
        return *instance;
    }

    app_thread& app::thread() const noexcept
    {
        return *iThread;
    }

    const i_program_options& app::program_options() const noexcept
    {
        return iProgramOptions;
    }

    std::string const& app::name() const noexcept
    {
        return iName;
    }

    void app::set_name(std::string const& aName)
    {
        iName = aName;
        NameChanged();
    }
    
    int app::exec(bool aQuitWhenLastWindowClosed)
    {
        neolib::scoped_flag sf{ iInExec };
        try
        {
            service<i_surface_manager>().layout_surfaces();
            service<i_surface_manager>().invalidate_surfaces();
            iQuitWhenLastWindowClosed = aQuitWhenLastWindowClosed;
            ExecutionStarted();
            while (iQuitResultCode == std::nullopt)
            {
                if (!process_events(iAppContext))
                {
                    if (neolib::service<neolib::i_power>().turbo_mode_active())
                        neolib::this_thread::relax();
                    else
                        neolib::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
                }
            }
            return *iQuitResultCode;
        }
        catch (std::exception& e)
        {
            thread().halt();
            service<debug::logger>() << neolib::logger::severity::Debug << "neogfx::app::exec: terminating with exception: " << e.what() << std::endl;
            service<i_surface_manager>().display_error_message(iName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + iName, std::string("neogfx::app::exec: terminating with exception: ") + e.what());
            std::exit(EXIT_FAILURE);
        }
        catch (...)
        {
            thread().halt();
            service<debug::logger>() << neolib::logger::severity::Debug << "neogfx::app::exec: terminating with unknown exception" << std::endl;
            service<i_surface_manager>().display_error_message(iName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + iName, "neogfx::app::exec: terminating with unknown exception");
            std::exit(EXIT_FAILURE);
        }
    }

    bool app::in_exec() const
    {
        return iInExec;
    }

    void app::quit(int aResultCode)
    {
        if (iQuitResultCode == std::nullopt)
        {
            iQuitResultCode = aResultCode;
            QuitRequested.trigger(aResultCode);
        }
    }

    void app::cancel_quit()
    {
        iQuitResultCode = std::nullopt;
    }

    dimension app::x2_dpi_scale_factor() const
    {
        return neogfx::x2_dpi_scale_factor(service<i_surface_manager>().display().metrics().ppi());
    }

    dimension app::xn_dpi_scale_factor() const
    {
        return neogfx::xn_dpi_scale_factor(service<i_surface_manager>().display().metrics().ppi());
    }

    const i_texture& app::default_window_icon() const
    {
        return iDefaultWindowIcon;
    }

    void app::set_default_window_icon(const i_texture& aIcon)
    {
        iDefaultWindowIcon = aIcon;
    }

    void app::set_default_window_icon(const i_image& aIcon)
    {
        iDefaultWindowIcon = aIcon;
    }

    const i_style& app::current_style() const
    {
        if (iCurrentStyle == iStyles.end())
            throw style_not_found();
        return iCurrentStyle->second;;
    }

    i_style& app::current_style()
    {
        if (iCurrentStyle == iStyles.end())
            throw style_not_found();
        return iCurrentStyle->second;
    }

    i_style& app::change_style(std::string const& aStyleName)
    {
        style_list::iterator existingStyle = iStyles.find(aStyleName);
        if (existingStyle == iStyles.end())
            throw style_not_found();
        if (iCurrentStyle != existingStyle)
        {
            iCurrentStyle = existingStyle;
            CurrentStyleChanged(style_aspect::Style);
            service<i_surface_manager>().layout_surfaces();
            service<i_surface_manager>().invalidate_surfaces();
        }
        return iCurrentStyle->second;
    }

    i_style& app::register_style(const i_style& aStyle)
    {
        if (iStyles.find(aStyle.name()) != iStyles.end())
            throw style_exists();
        style_list::iterator newStyle = iStyles.insert(std::make_pair(aStyle.name(), style(aStyle.name(), aStyle))).first;
        if (iCurrentStyle == iStyles.end())
        {
            iCurrentStyle = newStyle;
            service<i_surface_manager>().invalidate_surfaces();
        }
        return newStyle->second;
    }

    void app::clear_translations()
    {
        iTranslations.clear();
    }

    void app::load_translations()
    {
        for (auto const& file : std::filesystem::directory_iterator{ std::filesystem::path{ neolib::program_directory() } })
            if (file.path().extension() == ".xneol")
                load_translations(file.path());
    }

    void app::load_translations(std::filesystem::path const& aTranslationFile)
    {
        neolib::xml translationFile{ aTranslationFile.generic_string() };
        if (translationFile.root().name() == "xneol")
        {
            auto const& language = translationFile.root().attribute_value("trgLang").to_std_string();
            for (auto const& item : translationFile.root())
            {
                if (item.name() == "text")
                {
                    std::optional<std::string> source;
                    std::vector<std::pair<std::pair<std::int64_t, std::int64_t>, std::string>> targets;
                    for (auto const& part : item)
                    {
                        if (part.name() == "source")
                            source = part.text();
                        else if (part.name() == "target")
                        {
                            auto plurality = std::make_pair(std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max());
                            if (part.has_attribute("n"))
                            {
                                auto const& n = part.attribute_value("n").to_std_string();
                                neolib::vecarray<std::string, 2> bits;
                                neolib::tokens(n, ".."s, bits, 2, false, true);
                                if (bits.size() == 1)
                                    plurality.second = (plurality.first = boost::lexical_cast<std::int64_t>(bits[0]));
                                else if (bits.size() == 2)
                                {
                                    if (!bits[0].empty())
                                        plurality.first = boost::lexical_cast<std::int64_t>(bits[0]);
                                    if (!bits[1].empty())
                                        plurality.second = boost::lexical_cast<std::int64_t>(bits[1]);
                                }
                            }
                            targets.push_back(std::make_pair(plurality, part.text()));
                        }
                    }
                    if (source)
                        for (auto const& target : targets)
                            iTranslations[language][source.value()][target.first] = target.second;
                }
            }
        }
    }

    i_string const& app::translate(i_string const& aTranslatableString, i_string const& aContext, std::int64_t aPlurality) const
    {
        if (iTranslations.empty())
            return aTranslatableString;
        /// @todo select language
        /// @todo context
        auto existing = iTranslations.begin()->second.find(aTranslatableString);
        if (existing == iTranslations.begin()->second.end())
            return aTranslatableString;
        for (auto const& target : existing->second)
            if (aPlurality >= target.first.first && aPlurality <= target.first.second)
                return target.second;
        return existing->second.begin()->second;
    }

    i_action& app::action_file_new()
    {
        return actionFileNew;
    }

    i_action& app::action_file_open()
    {
        return actionFileOpen;
    }

    i_action& app::action_file_close()
    {
        return actionFileClose;
    }

    i_action& app::action_file_close_all()
    {
        return actionFileCloseAll;
    }

    i_action& app::action_file_save()
    {
        return actionFileSave;
    }

    i_action& app::action_file_save_all()
    {
        return actionFileSaveAll;
    }

    i_action& app::action_file_exit()
    {
        return actionFileExit;
    }

    i_action& app::action_undo()
    {
        return actionUndo;
    }

    i_action& app::action_redo()
    {
        return actionRedo;
    }

    i_action& app::action_cut()
    {
        return actionCut;
    }

    i_action& app::action_copy()
    {
        return actionCopy;
    }

    i_action& app::action_paste()
    {
        return actionPaste;
    }

    i_action& app::action_delete()
    {
        return actionDelete;
    }

    i_action& app::action_select_all()
    {
        return actionSelectAll;
    }

    i_action& app::add_action(i_action& aAction)
    {
        auto& a = iActions.emplace(aAction.text(), ref_ptr<i_action>{ref_ptr<i_action>{}, &aAction});
        return *a.second();
    }

    i_action& app::add_action(i_ref_ptr<i_action> const& aAction)
    {
        auto& a = iActions.emplace(aAction->text(), aAction);
        return *a.second();
    }

    i_action& app::add_action(i_string const& aText)
    {
        auto& a = iActions.emplace(aText, make_ref<action>(aText));
        return *a.second();
    }

    i_action& app::add_action(i_string const& aText, i_string const& aImageUri, dimension aDpiScaleFactor, texture_sampling aSampling)
    {
        auto& a = iActions.emplace(aText, make_ref<action>(aText, aImageUri, aDpiScaleFactor, aSampling));
        return *a.second();
    }

    i_action& app::add_action(i_string const& aText, const i_texture& aImage)
    {
        auto& a = iActions.emplace(aText, make_ref<action>(aText, aImage));
        return *a.second();
    }

    i_action& app::add_action(i_string const& aText, const i_image& aImage)
    {
        auto& a = iActions.emplace(aText, make_ref<action>(aText, aImage));
        return *a.second();
    }

    void app::remove_action(i_action& aAction)
    {
        for (auto i = iActions.begin(); i != iActions.end(); ++i)
            if (&*i->second() == &aAction)
            {
                iActions.erase(i);
                break;
            }
    }

    i_action& app::find_action(i_string const& aText)
    {
        auto a = iActions.find(aText);
        if (a == iActions.end())
            throw action_not_found();
        return *a->second();
    }

    void app::add_mnemonic(i_mnemonic& aMnemonic)
    {
        iMnemonics.push_back(&aMnemonic);
    }

    void app::remove_mnemonic(i_mnemonic& aMnemonic)
    {
        auto n = std::find(iMnemonics.begin(), iMnemonics.end(), &aMnemonic);
        if (n != iMnemonics.end())
            iMnemonics.erase(n);
    }

    i_menu& app::add_standard_menu(i_menu& aParentMenu, standard_menu aStandardMenu)
    {
        switch (aStandardMenu)
        {
        case standard_menu::File:
            {
                auto& fileMenu = aParentMenu.add_sub_menu("&File"_t);
                fileMenu.add_action(action_file_new());
                fileMenu.add_action(action_file_open());
                fileMenu.add_separator();
                fileMenu.add_action(action_file_close());
                fileMenu.add_separator();
                fileMenu.add_action(action_file_save());
                fileMenu.add_separator();
                fileMenu.add_action(action_file_exit());
                return fileMenu;
            }
        case standard_menu::Edit:
            {
                auto& editMenu = aParentMenu.add_sub_menu("&Edit"_t);
                editMenu.add_action(action_undo());
                editMenu.add_action(action_redo());
                editMenu.add_separator();
                editMenu.add_action(action_cut());
                editMenu.add_action(action_copy());
                editMenu.add_action(action_paste());
                editMenu.add_action(action_delete());
                editMenu.add_separator();
                editMenu.add_action(action_select_all());
                return editMenu;
            }
        default:
            throw unknown_standard_menu();
        }
    }

    class help : public i_help
    {
    public:
        define_declared_event(HelpActivated, help_activated, const i_help_source&)
        define_declared_event(HelpDeactivated, help_deactivated, const i_help_source&)
    public:
        bool help_active() const override
        {
            return !iActiveSources.empty();
        }
        const i_help_source& active_help() const override
        {
            if (help_active())
                return *iActiveSources.back();
            throw help_not_active();
        }
    public:
        void activate(const i_help_source& aSource) override
        {
            iActiveSources.push_back(&aSource);
            HelpActivated(aSource);
        }
        void deactivate(const i_help_source& aSource) override
        {
            auto existing = std::find(iActiveSources.rbegin(), iActiveSources.rend(), &aSource);
            if (existing == iActiveSources.rend())
                throw invalid_help_source();
            iActiveSources.erase(existing.base() - 1);
            HelpDeactivated(aSource);
        }
    private:
        std::vector<const i_help_source*> iActiveSources;
    };

    i_help& app::help() const
    {
        if (iHelp == nullptr)
            iHelp = std::make_unique<neogfx::help>();
        return *iHelp;
    }

    bool app::process_events()
    {
        neogfx::event_processing_context epc{ thread() };
        return process_events(epc);
    }

    bool app::process_events(i_event_processing_context&)
    {
        bool didSome = false;
        try
        {
            if (!thread().in()) // not app thread
                return didSome;
            
            if (service<i_rendering_engine>().creating_window() || service<i_surface_manager>().initialising_surface())
                return didSome;

            bool hadStrongSurfaces = service<i_surface_manager>().any_strong_surfaces();

            didSome = (thread().do_work(neolib::yield_type::NoYield) || didSome);
            didSome = (do_process_events() || didSome);

            bool lastWindowClosed = hadStrongSurfaces && !service<i_surface_manager>().any_strong_surfaces();
            if (!in_exec() && lastWindowClosed)
                throw main_window_closed_prematurely();
            if (lastWindowClosed && iQuitWhenLastWindowClosed)
                quit(0);

            service<i_rendering_engine>().render_now();
        }
        catch (std::exception& e)
        {
            if (!thread().halted())
            {
                thread().halt();
                service<debug::logger>() << neolib::logger::severity::Debug << "neogfx::app::process_events: terminating with exception: " << e.what() << std::endl;
                service<i_surface_manager>().display_error_message(iName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + iName, std::string("neogfx::app::process_events: terminating with exception: ") + e.what());
                std::exit(EXIT_FAILURE);
            }
        }
        catch (...)
        {
            if (!thread().halted())
            {
                thread().halt();
                service<debug::logger>() << neolib::logger::severity::Debug << "neogfx::app::process_events: terminating with unknown exception" << std::endl;
                service<i_surface_manager>().display_error_message(iName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + iName, "neogfx::app::process_events: terminating with unknown exception");
                std::exit(EXIT_FAILURE);
            }
        }
        return didSome;
    }

    i_event_processing_context& app::event_processing_context()
    {
        return iAppContext;
    }

    bool app::discover(const uuid& aId, void*& aObject)
    {
        aObject = nullptr;
        if (aId == i_async_task::iid())
            aObject = &static_cast<i_async_task&>(thread());
        if (aObject)
            return true;
        return base_type::discover(aId, aObject);
    }

    bool app::do_process_events()
    {
        bool lastWindowClosed = false;
        bool didSome = service<i_surface_manager>().process_events(lastWindowClosed);
        return didSome;
    }

    bool app::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        if (aScanCode == ScanCode_LALT)
            for (auto& m : iMnemonics)
                m->mnemonic_widget().update();
        bool partialMatches = false;
        iKeySequence.push_back(std::make_pair(aKeyCode, aKeyModifiers));
        for (auto& a : iActions)
            if (a.second()->is_enabled() && a.second()->shortcut() != std::nullopt)
            {
                auto matchResult = a.second()->shortcut()->matches(iKeySequence.begin(), iKeySequence.end());
                if (matchResult == key_sequence::match::Full)
                {
                    iKeySequence.clear();
                    if (service<i_keyboard>().is_front_grabber(*this))
                    {
                        a.second()->triggered()();
                        if (a.second()->is_checkable())
                            a.second()->toggle();
                        return true;
                    }
                    else
                    {
                        service<i_basic_services>().system_beep();
                        return false;
                    }
                }
                else if (matchResult == key_sequence::match::Partial)
                    partialMatches = true;
            }
        if (!partialMatches)
            iKeySequence.clear();
        return false;
    }

    bool app::key_released(scan_code_e aScanCode, key_code_e, key_modifiers_e)
    {
        if (aScanCode == ScanCode_LALT)
            for (auto& m : iMnemonics)
                m->mnemonic_widget().update();
        return false;
    }

    namespace
    {
        struct mnemonic_sorter
        {
            bool operator()(i_mnemonic* lhs, i_mnemonic* rhs) const
            {
                if (!lhs->mnemonic_widget().has_surface() && !rhs->mnemonic_widget().has_surface())
                    return lhs < rhs;
                else if (lhs->mnemonic_widget().has_surface() && !rhs->mnemonic_widget().has_surface())
                    return true;
                else if (!lhs->mnemonic_widget().has_surface() && rhs->mnemonic_widget().has_surface())
                    return false;
                else if (lhs->mnemonic_widget().same_surface(rhs->mnemonic_widget()))
                    return lhs < rhs;
                else if (lhs->mnemonic_widget().surface().is_owner_of(rhs->mnemonic_widget().surface()))
                    return false;
                else if (rhs->mnemonic_widget().surface().is_owner_of(lhs->mnemonic_widget().surface()))
                    return true;
                else if (lhs->mnemonic_widget().has_surface() && lhs->mnemonic_widget().surface().surface_type() == surface_type::Window && lhs->mnemonic_widget().surface().as_surface_window().native_window().is_active())
                    return true;
                else if (rhs->mnemonic_widget().has_surface() && rhs->mnemonic_widget().surface().surface_type() == surface_type::Window && rhs->mnemonic_widget().surface().as_surface_window().native_window().is_active())
                    return false;
                else
                    return &lhs->mnemonic_widget().surface() < &rhs->mnemonic_widget().surface();
            }
        };
    }

    bool app::text_input(i_string const&)
    {
        return false;
    }

    bool app::sys_text_input(i_string const& aInput)
    {
        static boost::locale::generator gen;
        static std::locale loc = gen("en_US.UTF-8");
        std::sort(iMnemonics.begin(), iMnemonics.end(), mnemonic_sorter());
        i_surface* lastWindowSurface = nullptr;
        for (auto& m : iMnemonics)
        {
            if (lastWindowSurface == nullptr)
            {
                if (m->mnemonic_widget().has_surface() && m->mnemonic_widget().surface().surface_type() == surface_type::Window)
                    lastWindowSurface = &m->mnemonic_widget().surface();
            }
            else if (m->mnemonic_widget().has_surface() && m->mnemonic_widget().surface().surface_type() == surface_type::Window && lastWindowSurface != &m->mnemonic_widget().surface())
            {
                continue;
            }
            if (boost::locale::to_lower(m->mnemonic(), loc) == boost::locale::to_lower(aInput.to_std_string(), loc))
            {
                m->mnemonic_execute();
                return true;
            }
        }
        return false;
    }
}