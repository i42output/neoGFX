// i_app.hpp
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

namespace neogfx
{
	class i_basic_services;
	class i_rendering_engine;
	class i_surface_manager;
	class i_keyboard;
	class i_style;
	class i_action;
	class i_mnemonic;
	class i_texture;
	class i_image;

	class event_processing_context;

	class i_app
	{
	public:
		class i_event_processing_context
		{
		public:
			virtual ~i_event_processing_context() {}
		public:
			virtual const std::string& name() const = 0;
		};
	public:
		virtual const std::string& name() const = 0;
		virtual int exec(bool aQuitWhenLastWindowClosed = true) = 0;
		virtual void quit(int aResultCode) = 0;
		virtual i_basic_services& basic_services() const = 0;
		virtual i_rendering_engine& rendering_engine() const = 0;
		virtual i_surface_manager& surface_manager() const = 0;
		virtual const i_keyboard& keyboard() const = 0;
	public:
		virtual const i_style& current_style() const = 0;
		virtual i_style& current_style() = 0;
		virtual i_style& change_style(const std::string& aStyleName) = 0;
		virtual i_style& register_style(const i_style& aStyle) = 0;
	public:
		virtual i_action& add_action(const std::string& aText) = 0;
		virtual i_action& add_action(const std::string& aText, const std::string& aImageUri) = 0;
		virtual i_action& add_action(const std::string& aText, const i_texture& aImage) = 0;
		virtual i_action& add_action(const std::string& aText, const i_image& aImage) = 0;
		virtual void remove_action(i_action& aAction) = 0;
		virtual void add_mnemonic(i_mnemonic& aMnemonic) = 0;
		virtual void remove_mnemonic(i_mnemonic& aMnemonic) = 0;
	public:
		virtual bool process_events(i_event_processing_context& aContext) = 0;
	};
}