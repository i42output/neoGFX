// app.hpp
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

#pragma once

#include "neogfx.hpp"
#include <boost/optional.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <neolib/io_thread.hpp>
#include "i_app.hpp"
#include "i_basic_services.hpp"
#include "i_rendering_engine.hpp"
#include "i_surface_manager.hpp"
#include "i_widget.hpp"
#include "keyboard.hpp"
#include "clipboard.hpp"
#include "style.hpp"
#include "action.hpp"
#include "i_mnemonic.hpp"

namespace neogfx
{
	class app : public i_app, public neolib::io_thread, private i_keyboard_handler
	{
	private:
		typedef std::map<std::string, style> style_list;
		typedef std::list<action, boost::fast_pool_allocator<action>> action_list;
		typedef std::vector<i_mnemonic*> mnemonic_list;
	public:
		struct no_instance : std::logic_error { no_instance() : std::logic_error("neogfx::app::no_instance") {} };
		struct no_basic_services : std::logic_error { no_basic_services() : std::logic_error("neogfx::app::no_basic_services") {} };
		struct no_renderer : std::logic_error { no_renderer() : std::logic_error("neogfx::app::no_renderer") {} };
		struct no_surface_manager : std::logic_error { no_surface_manager() : std::logic_error("neogfx::app::no_surface_manager") {} };
		struct no_keyboard : std::logic_error { no_keyboard() : std::logic_error("neogfx::app::no_keyboard") {} };
		struct no_clipboard : std::logic_error { no_clipboard() : std::logic_error("neogfx::app::no_clipboard") {} };
		struct style_not_found : std::runtime_error { style_not_found() : std::runtime_error("neogfx::app::style_not_found") {} };
		struct style_exists : std::runtime_error { style_exists() : std::runtime_error("neogfx::app::style_exists") {} };
	public:
		app(const std::string& aName = std::string());
		~app();
	public:
		static app& instance();
		virtual const std::string& name() const;
		virtual int exec(bool aQuitWhenLastWindowClosed = true);
		virtual void quit(int aResultCode);
		virtual i_basic_services& basic_services() const;
		virtual i_rendering_engine& rendering_engine() const;
		virtual i_surface_manager& surface_manager() const;
		virtual i_keyboard& keyboard() const;
		virtual i_clipboard& clipboard() const;
	public:
		virtual const i_style& current_style() const;
		virtual i_style& current_style();
		virtual i_style& change_style(const std::string& aStyleName);
		virtual i_style& register_style(const i_style& aStyle);
	public:
		virtual i_action& add_action(const std::string& aText);
		virtual i_action& add_action(const std::string& aText, const std::string& aImageUri);
		virtual i_action& add_action(const std::string& aText, const i_texture& aImage);
		virtual i_action& add_action(const std::string& aText, const i_image& aImage);
		virtual void remove_action(i_action& aAction);
		virtual void add_mnemonic(i_mnemonic& aMnemonic);
		virtual void remove_mnemonic(i_mnemonic& aMnemonic);
	public:
		virtual bool process_events();
	private:
		virtual void task() {}
		bool do_process_events();
	private:
		virtual bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual bool key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers);
		virtual bool text_input(const std::string& aText);
		virtual bool sys_text_input(const std::string& aText);
	private:
		std::string iName;
		bool iQuitWhenLastWindowClosed;
		std::unique_ptr<i_basic_services> iBasicServices;
		std::unique_ptr<i_keyboard> iKeyboard;
		std::unique_ptr<i_clipboard> iClipboard;
		std::unique_ptr<i_rendering_engine> iRenderingEngine;
		std::unique_ptr<i_surface_manager> iSurfaceManager;
		boost::optional<int> iQuitResultCode;
		style_list iStyles;
		style_list::iterator iCurrentStyle;
		action_list iActions;
		mnemonic_list iMnemonics;
		std::unique_ptr<i_widget> iSystemCache;
	};
}