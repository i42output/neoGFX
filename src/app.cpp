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

#include "neogfx.hpp"
#include <atomic>
#include <boost/locale.hpp> 
#include "app.hpp"
#include "sdl_basic_services.hpp"
#include "sdl_renderer.hpp"
#include "surface_manager.hpp"
#include "sdl_keyboard.hpp"
#include "i_native_window.hpp"

namespace neogfx
{
	namespace
	{
		std::atomic<app*> sFirstInstance;
	}

	app::app(const std::string& aName)
		try :
		iName(aName),
		iQuitWhenLastWindowClosed(true),
		neolib::io_thread("neogfx::app", true),
		iBasicServices(new neogfx::sdl_basic_services(*this)),
		iKeyboard(new neogfx::sdl_keyboard()),
		iRenderingEngine(new neogfx::sdl_renderer(*iBasicServices, *iKeyboard)),
		iSurfaceManager(new neogfx::surface_manager(*iBasicServices, *iRenderingEngine)),
		iCurrentStyle(iStyles.begin())
	{
		app* np = nullptr;
		sFirstInstance.compare_exchange_strong(np, this);
		create_message_queue([this]() -> bool 
		{ 
			bool result = process_events(); 
			rendering_engine().render_now();
			return result;
		});
		iKeyboard->grab_keyboard(*this);
		style whiteStyle("Default");
		register_style(whiteStyle);
		style slateStyle("Slate");
		slateStyle.set_colour(colour(0x35, 0x35, 0x35));
		register_style(slateStyle);
	}
	catch (std::exception& e)
	{
		std::cerr << "neogfx::app::exec: terminating with exception: " << e.what() << std::endl;
		sdl_basic_services(*this).display_error_dialog(aName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + aName, std::string("main: terminating with exception: ") + e.what());
		throw;
	}
	catch (...)
	{
		std::cerr << "neogfx::app::exec: terminating with unknown exception" << std::endl;
		sdl_basic_services(*this).display_error_dialog(aName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + aName, "main: terminating with unknown exception");
		throw;
	}

	app::~app()
	{
		iKeyboard->ungrab_keyboard(*this);
		app* tp = this;
		app* np = nullptr;
		sFirstInstance.compare_exchange_strong(tp, np);
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
			{
				process_events();
				rendering_engine().render_now();
			}
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

	i_action& app::add_action(const std::string& aText)
	{
		iActions.emplace_back(aText);
		return iActions.back();
	}

	i_action& app::add_action(const std::string& aText, const std::string& aImageUri)
	{
		iActions.emplace_back(aText, aImageUri);
		return iActions.back();
	}

	i_action& app::add_action(const std::string& aText, const i_texture& aImage)
	{
		iActions.emplace_back(aText, aImage);
		return iActions.back();
	}

	i_action& app::add_action(const std::string& aText, const i_image& aImage)
	{
		iActions.emplace_back(aText, aImage);
		return iActions.back();
	}

	void app::remove_action(i_action& aAction)
	{
		for (auto i = iActions.begin(); i != iActions.end(); ++i)
			if (&*i == &aAction)
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

	bool app::process_events()
	{
		bool didSome = false;
		try
		{
			didSome = pump_messages();
			didSome = (do_io(neolib::yield_type::Sleep) || didSome);
			didSome = (do_process_events() || didSome);
		}
		catch (std::exception& e)
		{
			if (!halted())
			{
				halt();
				std::cerr << "neogfx::app::exec: terminating with exception: " << e.what() << std::endl;
				iSurfaceManager->display_error_message(iName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + iName, std::string("neogfx::app::exec: terminating with exception: ") + e.what());
				std::exit(EXIT_FAILURE);
			}
		}
		catch (...)
		{
			if (!halted())
			{
				halt();
				std::cerr << "neogfx::app::exec: terminating with unknown exception" << std::endl;
				iSurfaceManager->display_error_message(iName.empty() ? "Abnormal Program Termination" : "Abnormal Program Termination - " + iName, "neogfx::app::exec: terminating with unknown exception");
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
			if (a.shortcut() != boost::none && a.shortcut()->matches(aKeyCode, aKeyModifiers))
			{
				a.triggered.trigger();
				if (a.is_checkable())
					a.toggle();
				return true;
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
		for (auto& m : iMnemonics)
			if (boost::locale::to_lower(m->mnemonic(), loc) == boost::locale::to_lower(aInput, loc))
			{
				m->mnemonic_execute();
				return true;
			}
		return false;
	}
}