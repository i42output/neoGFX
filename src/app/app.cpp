// app.cpp
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

#include <neogfx/neogfx.hpp>
#include <atomic>
#include <boost/locale.hpp> 
#include <neogfx/app/app.hpp>
#include <neogfx/hid/surface_manager.hpp>
#include <neogfx/gui/window/window.hpp>
#include "../gui/window/native/i_native_window.hpp"

namespace neogfx
{
	namespace
	{
		std::atomic<app*> sFirstInstance;
	}

	app::loader::loader(int argc, char* argv[], app& aApp) : iApp(aApp)
	{
		app* np = nullptr;
		sFirstInstance.compare_exchange_strong(np, &aApp);
		if (sFirstInstance == &aApp && argc > 1 && std::string(argv[1]) == "-debug")
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
		iContext(aParent.message_queue()),
		iName(aName)
	{
	}

	const std::string& app::event_processing_context::name() const
	{
		return iName;
	}

	app::app(const std::string& aName, i_service_factory& aServiceFactory) :
		app(0, nullptr, aName, aServiceFactory)
	{
	}
	
	app::app(int argc, char* argv[], const std::string& aName, i_service_factory& aServiceFactory)
		try :
		iLoader(argc, argv, *this),
		iName(aName),
		iQuitWhenLastWindowClosed(true),
		neolib::io_thread("neogfx::app", true),
		iBasicServices(aServiceFactory.create_basic_services(*this)),
		iKeyboard(aServiceFactory.create_keyboard()),
		iClipboard(new neogfx::clipboard(basic_services().system_clipboard())),
		iRenderingEngine(aServiceFactory.create_rendering_engine(basic_services(), keyboard())),
		iSurfaceManager(new neogfx::surface_manager(basic_services(), *iRenderingEngine)),
		iCurrentStyle(iStyles.begin()),
		iActionFileNew{ add_action("&New").set_shortcut("Ctrl+Shift+N") },
		iActionFileOpen{ add_action("&Open").set_shortcut("Ctrl+Shift+O") },
		iActionFileClose{ add_action("&Close").set_shortcut("Ctrl+F4") },
		iActionFileCloseAll{ add_action("Close All") },
		iActionFileSave{ add_action("&Save").set_shortcut("Ctrl+S") },
		iActionFileSaveAll{ add_action("Save A&ll").set_shortcut("Ctrl+Shift+S") },
		iActionFileExit{ add_action("E&xit").set_shortcut("Alt+F4") },
		iActionUndo{ add_action("Undo").set_shortcut("Ctrl+Z") },
		iActionRedo{ add_action("Redo").set_shortcut("Ctrl+Shift+Z") },
		iActionCut{ add_action("Cut").set_shortcut("Ctrl+X") },
		iActionCopy{ add_action("Copy").set_shortcut("Ctrl+C") },
		iActionPaste{ add_action("Paste").set_shortcut("Ctrl+V") },
		iActionDelete{ add_action("Delete").set_shortcut("Del") },
		iActionSelectAll{ add_action("Select All").set_shortcut("Ctrl+A") },
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
		}, 100 }
	{
		create_message_queue([this]() -> bool 
		{ 
			auto result = process_events(*iContext); 
			rendering_engine().render_now();
			return result;
		});

		iContext = std::make_unique<event_processing_context>(*this, "neogfx::app");

		iKeyboard->grab_keyboard(*this);

		style whiteStyle("Default");
		register_style(whiteStyle);
		style slateStyle("Slate");
		slateStyle.set_colour(colour(0x35, 0x35, 0x35));
		register_style(slateStyle);

		iSystemCache.reset(new window{ point{}, size{}, "neogfx::system_cache", window::InitiallyHidden | window::Weak });

		iActionFileExit.triggered([this]() { quit(0); });
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
		rendering_engine().texture_manager().clear_textures();
		iKeyboard->ungrab_keyboard(*this);
		iSystemCache.reset();
	}

	app& app::instance()
	{
		app* instance = sFirstInstance.load();
		if (instance == 0)
			throw no_instance();
		return *instance;
	}

	const std::string& app::name() const
	{
		return iName;
	}
	
	int app::exec(bool aQuitWhenLastWindowClosed)
	{
		try
		{
			surface_manager().layout_surfaces();
			surface_manager().invalidate_surfaces();
			iQuitWhenLastWindowClosed = aQuitWhenLastWindowClosed;
			while (!iQuitResultCode.is_initialized())
				process_events(*iContext);
			return *iQuitResultCode;
		}
		catch (std::exception& e)
		{
			halt();
			std::cerr << "neogfx::app::exec: terminating with exception: " << e.what() << std::endl;
			iSurfaceManager->display_error_message(iName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + iName, std::string("neogfx::app::exec: terminating with exception: ") + e.what());
			std::exit(EXIT_FAILURE);
		}
		catch (...)
		{
			halt();
			std::cerr << "neogfx::app::exec: terminating with unknown exception" << std::endl;
			iSurfaceManager->display_error_message(iName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + iName, "neogfx::app::exec: terminating with unknown exception");
			std::exit(EXIT_FAILURE);
		}
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
		return iCurrentStyle->second;;
	}

	i_style& app::change_style(const std::string& aStyleName)
	{
		style_list::iterator existingStyle = iStyles.find(aStyleName);
		if (existingStyle == iStyles.end())
			throw style_not_found();
		if (iCurrentStyle != existingStyle)
		{
			iCurrentStyle = existingStyle;
			current_style_changed.trigger();
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

	i_action& app::add_action(const std::string& aText, const std::string& aImageUri, texture_sampling aSampling)
	{
		auto a = iActions.emplace(aText, action{ aText, aImageUri, aSampling });
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

	bool app::process_events(i_event_processing_context&)
	{
		bool didSome = false;
		try
		{
			didSome = pump_messages();
			didSome = (do_io(neolib::yield_type::Sleep) || didSome);
			didSome = (do_process_events() || didSome);
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
		if (lastWindowClosed && iQuitWhenLastWindowClosed)
		{
			if (!iQuitResultCode.is_initialized())
				iQuitResultCode = 0;
		}
		return didSome;
	}

	bool app::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
		if (aScanCode == ScanCode_LALT || aScanCode == ScanCode_RALT)
			for (auto& m : iMnemonics)
				m->mnemonic_widget().update();
		for (auto& a : iActions)
			if (a.second.is_enabled() && a.second.shortcut() != boost::none && a.second.shortcut()->matches(aKeyCode, aKeyModifiers))
			{
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
		i_surface* lastWindowSurface = 0;
		for (auto& m : iMnemonics)
		{
			if (lastWindowSurface == 0)
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