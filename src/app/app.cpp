// cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <boost/locale.hpp> 
#include <neolib/raii.hpp>
#include <neolib/string_utils.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/hid/surface_manager.hpp>
#include <neogfx/app/resource_manager.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/widget/i_menu.hpp>
#include "../gui/window/native/i_native_window.hpp"

namespace nrc
{
	// we need to explicitly reference the resource object here otherwise the resource object file will be omitted at link stage 
	// when linking with neoGFX (as a static .lib).  This is because static objects are not instantiated in static libraries
	// if they are not referenced outside of the library.
	extern void* neogfx_icons;
	extern void* neogfx_resources;
	void* ref_neogfx_icons = neogfx_icons;
	void* ref_neogfx_resources = neogfx_resources;
}

namespace neogfx
{
	template <> neolib::async_task& service<neolib::async_task>::instance() { return app::instance(); }
	template <> i_basic_services& service<i_basic_services>::instance() { return app::instance().basic_services(); }
	template <> i_surface_manager& service<i_surface_manager>::instance() { return app::instance().surface_manager(); }
	template <> i_window_manager& service<i_window_manager>::instance() { return app::instance().window_manager(); }
	template <> i_keyboard& service<i_keyboard>::instance() { return app::instance().keyboard(); }
	template <> i_clipboard& service<i_clipboard>::instance() { return app::instance().clipboard(); }
	template <> i_audio& service<i_audio>::instance() { return app::instance().audio(); }

	program_options::program_options(int argc, char* argv[])
	{
		boost::program_options::options_description description{ "Allowed options" };
		description.add_options()
			("debug", "open debug console")
			("fullscreen", boost::program_options::value<std::string>()->implicit_value(""s), "run full screen")
			("nest", "display child windows nested within main window rather than using the main desktop")
			("vulkan", "use Vulkan renderer")
			("directx", "use DirectX (ANGLE) renderer")
			("software", "use software renderer")
			("double", "enable window double buffering");
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), *this);
		if (count("vulkan") + count("directx") + count("software") > 1)
			throw invalid_options("more than one renderer specified");
	}

	bool program_options::debug() const
	{
		return count("debug") == 1;
	}

	neogfx::renderer program_options::renderer() const
	{
		if (count("vulkan") == 1)
			return neogfx::renderer::Vulkan;
		else if (count("directx") == 1)
			return neogfx::renderer::DirectX;
		else if (count("software") == 1)
			return neogfx::renderer::Software;
		else
			return neogfx::renderer::OpenGL;
	}

	std::optional<std::pair<uint32_t, uint32_t>> program_options::full_screen() const
	{
		if (count("fullscreen") == 1)
		{
			auto screenResolution = (*this)["fullscreen"].as<std::string>();
			if (screenResolution.empty())
				return std::make_pair(0, 0);
			else
			{
				neolib::vecarray<std::string, 2> bits;
				neolib::tokens(screenResolution, ","s, bits, 2, false, false);
				if (bits.size() == 2)
				{
					auto result = std::make_pair(boost::lexical_cast<uint32_t>(bits[0]), boost::lexical_cast<uint32_t>(bits[1]));
					if (result.first != 0 && result.second != 0)
						return result;
				}
			}
			throw invalid_options("invalid fullscreen resolution");
		}
		return std::optional<std::pair<uint32_t, uint32_t>>{};
	}

	bool program_options::double_buffering() const
	{
		return count("double") == 1;
	}

	bool program_options::nest() const
	{
		return count("nest") == 1;
	}

	namespace
	{
		std::atomic<app*> sFirstInstance;
	}

	app::loader::loader(const neogfx::program_options& aProgramOptions, app& aApp) : iApp(aApp)
	{
		app* np = nullptr;
		sFirstInstance.compare_exchange_strong(np, &aApp);
		if (sFirstInstance == &aApp && aProgramOptions.debug())
		{
#if defined(_WIN32)
			AllocConsole();
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);
			std::cout.sync_with_stdio(false);
			std::cout.sync_with_stdio(true);
			std::cerr.sync_with_stdio(false);
			std::cerr.sync_with_stdio(true);
#endif
		}
	}

	app::loader::~loader()
	{
		app* tp = &iApp;
		app* np = nullptr;
		sFirstInstance.compare_exchange_strong(tp, np);
	}

	app::event_processing_context::event_processing_context(app& aParent, const std::string& aName) :
		iContext{ aParent.have_message_queue() ? 
			aParent.message_queue() : 
			aParent.create_message_queue([this, &aParent]() 
			{ 
				return aParent.process_events(aParent.iAppMessageQueueContext); 
			}) },
		iName{ aName }
	{
	}

	const std::string& app::event_processing_context::name() const
	{
		return iName;
	}

	app::app(const std::string& aName, i_service_factory& aServiceFactory) :
		app{ 0, nullptr, aName, aServiceFactory }
	{
	}

	app::app(int argc, char* argv[], const std::string& aName, i_service_factory& aServiceFactory)
		try :
		neolib::async_thread{ "neogfx::app", true },
		iAsyncEventQueue{ static_cast<neolib::async_task&>(*this) },
		iProgramOptions{ argc, argv },
		iLoader{ iProgramOptions, *this },
		iName{ aName },
		iQuitWhenLastWindowClosed{ true },
		iInExec{ false },
		iBasicServices{ aServiceFactory.create_basic_services(*this) },
		iKeyboard{ aServiceFactory.create_keyboard() },
		iClipboard{ new neogfx::clipboard(basic_services().system_clipboard()) },
		iRenderingEngine{ aServiceFactory.create_rendering_engine(iProgramOptions.renderer(), iProgramOptions.double_buffering(), basic_services(), keyboard()) },
		iSurfaceManager{ new neogfx::surface_manager(basic_services(), *iRenderingEngine) },
		iWindowManager{ aServiceFactory.create_window_manager() },
		iAudio{ aServiceFactory.create_audio() },
		iDefaultWindowIcon{ image{ ":/neogfx/resources/icons/neoGFX.png" } },
		iCurrentStyle{ iStyles.begin() },
		iActionFileNew{ add_action("&New..."_t, ":/neogfx/resources/icons.naa#new.png").set_shortcut("Ctrl+Shift+N") },
		iActionFileOpen{ add_action("&Open..."_t, ":/neogfx/resources/icons.naa#open.png").set_shortcut("Ctrl+Shift+O") },
		iActionFileClose{ add_action("&Close"_t).set_shortcut("Ctrl+F4") },
		iActionFileCloseAll{ add_action("Close All"_t) },
		iActionFileSave{ add_action("&Save"_t, ":/neogfx/resources/icons.naa#save.png").set_shortcut("Ctrl+S") },
		iActionFileSaveAll{ add_action("Save A&ll"_t).set_shortcut("Ctrl+Shift+S") },
		iActionFileExit{ add_action("E&xit"_t).set_shortcut("Alt+F4") },
		iActionUndo{ add_action("Undo"_t, ":/neogfx/resources/icons.naa#undo.png").set_shortcut("Ctrl+Z") },
		iActionRedo{ add_action("Redo"_t, ":/neogfx/resources/icons.naa#redo.png").set_shortcut("Ctrl+Shift+Z") },
		iActionCut{ add_action("Cut"_t, ":/neogfx/resources/icons.naa#cut.png").set_shortcut("Ctrl+X") },
		iActionCopy{ add_action("Copy"_t, ":/neogfx/resources/icons.naa#copy.png").set_shortcut("Ctrl+C") },
		iActionPaste{ add_action("Paste"_t, ":/neogfx/resources/icons.naa#paste.png").set_shortcut("Ctrl+V") },
		iActionDelete{ add_action("Delete"_t).set_shortcut("Del") },
		iActionSelectAll{ add_action("Select All"_t).set_shortcut("Ctrl+A") },
		iStandardActionManager{ *this, [this](neolib::callback_timer& aTimer)
		{
			aTimer.again();
			if (clipboard().sink_active())
			{
				auto& sink = clipboard().active_sink();
				if (sink.can_undo())
					iActionUndo.enable();
				else
					iActionUndo.disable();
				if (sink.can_redo())
					iActionRedo.enable();
				else
					iActionRedo.disable();
				if (sink.can_cut())
					iActionCut.enable();
				else
					iActionCut.disable();
				if (sink.can_copy())
					iActionCopy.enable();
				else
					iActionCopy.disable();
				if (sink.can_paste())
					iActionPaste.enable();
				else
					iActionPaste.disable();
				if (sink.can_delete_selected())
					iActionDelete.enable();
				else
					iActionDelete.disable();
				if (sink.can_select_all())
					iActionSelectAll.enable();
				else
					iActionSelectAll.disable();
			}
			else
			{
				iActionUndo.disable();
				iActionRedo.disable();
				iActionCut.disable();
				iActionCopy.disable();
				iActionPaste.disable();
				iActionDelete.disable();
				iActionSelectAll.disable();
			}
		}, 100 },
		iAppContext{ *this, "neogfx::app::iAppContext" },
		iAppMessageQueueContext{ *this, "neogfx::app::iAppMessageQueueContext" }
	{
		iKeyboard->grab_keyboard(*this);

		style whiteStyle("Default");
		register_style(whiteStyle);
		style slateStyle("Slate");
		slateStyle.palette().set_colour(colour(0x35, 0x35, 0x35));
		register_style(slateStyle);

		iActionFileExit.triggered([this]() { quit(0); });
		iActionUndo.triggered([this]() { clipboard().active_sink().undo(clipboard()); });
		iActionRedo.triggered([this]() { clipboard().active_sink().redo(clipboard()); });
		iActionCut.triggered([this]() { clipboard().cut(); });
		iActionCopy.triggered([this]() { clipboard().copy(); });
		iActionPaste.triggered([this]() { clipboard().paste(); });
		iActionDelete.triggered([this]() { clipboard().delete_selected(); });
		iActionSelectAll.triggered([this]() { clipboard().select_all(); });
	}
	catch (std::exception& e)
	{
		std::cerr << "neogfx::app::app: terminating with exception: " << e.what() << std::endl;
		aServiceFactory.create_basic_services(*this)->display_error_dialog(aName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + aName, std::string("main: terminating with exception: ") + e.what());
		throw;
	}
	catch (...)
	{
		std::cerr << "neogfx::app::app: terminating with unknown exception" << std::endl;
		aServiceFactory.create_basic_services(*this)->display_error_dialog(aName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + aName, "main: terminating with unknown exception");
		throw;
	}

	app::~app()
	{
		iKeyboard->ungrab_keyboard(*this);
		resource_manager::instance().clean();
	}

	app& app::instance()
	{
		app* instance = sFirstInstance.load();
		if (instance == nullptr)
			throw no_instance();
		return *instance;
	}

	const program_options& app::program_options() const
	{
		return iProgramOptions;
	}

	const std::string& app::name() const
	{
		return iName;
	}
	
	int app::exec(bool aQuitWhenLastWindowClosed)
	{
		neolib::scoped_flag sf{ iInExec };
		try
		{
			surface_manager().layout_surfaces();
			surface_manager().invalidate_surfaces();
			iQuitWhenLastWindowClosed = aQuitWhenLastWindowClosed;
			while (iQuitResultCode == std::nullopt)
			{
				if (!process_events(iAppContext))
				{
					if (rendering_engine().game_mode())
						thread::yield();
					else
						thread::sleep(1);
				}
			}
			iAsyncEventQueue.terminate();
			return *iQuitResultCode;
		}
		catch (std::exception& e)
		{
			iAsyncEventQueue.terminate();
			halt();
			std::cerr << "neogfx::app::exec: terminating with exception: " << e.what() << std::endl;
			iSurfaceManager->display_error_message(iName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + iName, std::string("neogfx::app::exec: terminating with exception: ") + e.what());
			std::exit(EXIT_FAILURE);
		}
		catch (...)
		{
			iAsyncEventQueue.terminate();
			halt();
			std::cerr << "neogfx::app::exec: terminating with unknown exception" << std::endl;
			iSurfaceManager->display_error_message(iName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + iName, "neogfx::app::exec: terminating with unknown exception");
			std::exit(EXIT_FAILURE);
		}
	}

	bool app::in_exec() const
	{
		return iInExec;
	}

	void app::quit(int aResultCode)
	{
		iQuitResultCode = aResultCode;
	}

	i_basic_services& app::basic_services() const
	{
		if (iBasicServices)
			return *iBasicServices;
		else
			throw no_basic_services();
	}

	i_rendering_engine& app::rendering_engine() const
	{
		if (iRenderingEngine)
			return *iRenderingEngine;
		else
			throw no_renderer();
	}

	i_surface_manager& app::surface_manager() const
	{
		if (iSurfaceManager)
			return *iSurfaceManager;
		else
			throw no_surface_manager();
	}

	i_window_manager& app::window_manager() const
	{
		if (iWindowManager)
			return *iWindowManager;
		else
			throw no_window_manager();
	}
	
	i_keyboard& app::keyboard() const
	{
		if (iKeyboard)
			return *iKeyboard;
		else
			throw no_keyboard();
	}

	i_clipboard& app::clipboard() const
	{
		if (iClipboard)
			return *iClipboard;
		else
			throw no_clipboard();
	}

	i_audio& app::audio() const
	{
		if (iAudio)
			return *iAudio;
		else
			throw no_audio();
	}

	dimension app::default_dpi_scale_factor() const
	{
		return neogfx::default_dpi_scale_factor(surface_manager().display().metrics().ppi());
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

	i_style& app::change_style(const std::string& aStyleName)
	{
		style_list::iterator existingStyle = iStyles.find(aStyleName);
		if (existingStyle == iStyles.end())
			throw style_not_found();
		if (iCurrentStyle != existingStyle)
		{
			iCurrentStyle = existingStyle;
			current_style_changed.trigger(style_aspect::Style);
			surface_manager().layout_surfaces();
			surface_manager().invalidate_surfaces();
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
			surface_manager().invalidate_surfaces();
		}
		return newStyle->second;
	}

	const std::string& app::translate(const std::string& aTranslatableString, const std::string& aContext) const
	{
		// todo: i18n
		return aTranslatableString;
	}

	i_action& app::action_file_new()
	{
		return iActionFileNew;
	}

	i_action& app::action_file_open()
	{
		return iActionFileOpen;
	}

	i_action& app::action_file_close()
	{
		return iActionFileClose;
	}

	i_action& app::action_file_close_all()
	{
		return iActionFileCloseAll;
	}

	i_action& app::action_file_save()
	{
		return iActionFileSave;
	}

	i_action& app::action_file_save_all()
	{
		return iActionFileSaveAll;
	}

	i_action& app::action_file_exit()
	{
		return iActionFileExit;
	}

	i_action& app::action_undo()
	{
		return iActionUndo;
	}

	i_action& app::action_redo()
	{
		return iActionRedo;
	}

	i_action& app::action_cut()
	{
		return iActionCut;
	}

	i_action& app::action_copy()
	{
		return iActionCopy;
	}

	i_action& app::action_paste()
	{
		return iActionPaste;
	}

	i_action& app::action_delete()
	{
		return iActionDelete;
	}

	i_action& app::action_select_all()
	{
		return iActionSelectAll;
	}

	i_action& app::find_action(const std::string& aText)
	{
		auto a = iActions.find(aText);
		if (a == iActions.end())
			throw action_not_found();
		return a->second;
	}

	i_action& app::add_action(const std::string& aText)
	{
		auto a = iActions.emplace(aText, action{ aText });
		return a->second;
	}

	i_action& app::add_action(const std::string& aText, const std::string& aImageUri, dimension aDpiScaleFactor, texture_sampling aSampling)
	{
		auto a = iActions.emplace(aText, action{ aText, aImageUri, aDpiScaleFactor, aSampling });
		return a->second;
	}

	i_action& app::add_action(const std::string& aText, const i_texture& aImage)
	{
		auto a = iActions.emplace(aText, action{ aText, aImage });
		return a->second;
	}

	i_action& app::add_action(const std::string& aText, const i_image& aImage)
	{
		auto a = iActions.emplace(aText, action{ aText, aImage });
		return a->second;
	}

	void app::remove_action(i_action& aAction)
	{
		for (auto i = iActions.begin(); i != iActions.end(); ++i)
			if (&i->second == &aAction)
			{
				iActions.erase(i);
				break;
			}
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
			help_activated.trigger(aSource);
		}
		void deactivate(const i_help_source& aSource) override
		{
			auto existing = std::find(iActiveSources.rbegin(), iActiveSources.rend(), &aSource);
			if (existing == iActiveSources.rend())
				throw invalid_help_source();
			iActiveSources.erase(existing.base() - 1);
			help_deactivated.trigger(aSource);
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
		event_processing_context epc(*this);
		return process_events(epc);
	}

	bool app::process_events(i_event_processing_context&)
	{
		bool didSome = false;
		try
		{
			didSome = iAsyncEventQueue.exec();
			if (!in()) // not app thread
				return didSome;
			
			if (rendering_engine().creating_window() || surface_manager().initialising_surface())
				return didSome;

			bool hadStrongSurfaces = surface_manager().any_strong_surfaces();
			didSome = pump_messages();
			didSome = (do_io(neolib::yield_type::NoYield) || didSome);
			didSome = (do_process_events() || didSome);
			bool lastWindowClosed = hadStrongSurfaces && !surface_manager().any_strong_surfaces();
			if (!in_exec() && lastWindowClosed)
				throw main_window_closed_prematurely();
			if (lastWindowClosed && iQuitWhenLastWindowClosed)
			{
				if (iQuitResultCode == std::nullopt)
					iQuitResultCode = 0;
			}

			rendering_engine().render_now();
		}
		catch (std::exception& e)
		{
			if (!halted())
			{
				halt();
				std::cerr << "neogfx::app::process_events: terminating with exception: " << e.what() << std::endl;
				iSurfaceManager->display_error_message(iName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + iName, std::string("neogfx::app::process_events: terminating with exception: ") + e.what());
				std::exit(EXIT_FAILURE);
			}
		}
		catch (...)
		{
			if (!halted())
			{
				halt();
				std::cerr << "neogfx::app::process_events: terminating with unknown exception" << std::endl;
				iSurfaceManager->display_error_message(iName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + iName, "neogfx::app::process_events: terminating with unknown exception");
				std::exit(EXIT_FAILURE);
			}
		}
		return didSome;
	}

	bool app::do_process_events()
	{
		bool lastWindowClosed = false;
		bool didSome = surface_manager().process_events(lastWindowClosed);
		return didSome;
	}

	bool app::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
		if (aScanCode == ScanCode_LALT || aScanCode == ScanCode_RALT)
			for (auto& m : iMnemonics)
				m->mnemonic_widget().update();
		bool partialMatches = false;
		iKeySequence.push_back(std::make_pair(aKeyCode, aKeyModifiers));
		for (auto& a : iActions)
			if (a.second.is_enabled() && a.second.shortcut() != std::nullopt)
			{
				auto matchResult = a.second.shortcut()->matches(iKeySequence.begin(), iKeySequence.end());
				if (matchResult == key_sequence::match::Full)
				{
					iKeySequence.clear();
					if (keyboard().is_front_grabber(*this))
					{
						a.second.triggered.trigger();
						if (a.second.is_checkable())
							a.second.toggle();
						return true;
					}
					else
					{
						basic_services().system_beep();
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
		if (aScanCode == ScanCode_LALT || aScanCode == ScanCode_RALT)
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
				else if (lhs->mnemonic_widget().has_surface() && lhs->mnemonic_widget().surface().surface_type() == surface_type::Window && static_cast<i_native_window&>(lhs->mnemonic_widget().surface().native_surface()).is_active())
					return true;
				else if (rhs->mnemonic_widget().has_surface() && rhs->mnemonic_widget().surface().surface_type() == surface_type::Window && static_cast<i_native_window&>(rhs->mnemonic_widget().surface().native_surface()).is_active())
					return false;
				else
					return &lhs->mnemonic_widget().surface() < &rhs->mnemonic_widget().surface();
			}
		};
	}

	bool app::text_input(const std::string&)
	{
		return false;
	}

	bool app::sys_text_input(const std::string& aInput)
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
			if (boost::locale::to_lower(m->mnemonic(), loc) == boost::locale::to_lower(aInput, loc))
			{
				m->mnemonic_execute();
				return true;
			}
		}
		return false;
	}
}