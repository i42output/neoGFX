// radio_button.hpp
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
#include "button.hpp"

namespace neogfx
{
	class radio_button : public button
	{
	public:
		event<> on;
		event<> off;
	private:
		class disc : public widget
		{
		public:
			disc(radio_button& aParent);
		public:
			virtual size minimum_size() const;
			virtual size maximum_size() const;
		public:
			virtual void paint(graphics_context& aGraphicsContext) const;
		};
	public:
		radio_button(const std::string& aText = std::string());
		radio_button(i_widget& aParent, const std::string& aText = std::string());
		radio_button(i_layout& aLayout, const std::string& aText = std::string());
	public:
		bool is_on() const;
		bool is_off() const;
		void set_on();
	public:
		virtual neogfx::size_policy size_policy() const;
	protected:
		virtual void paint(graphics_context& aGraphicsContext) const;
		virtual void handle_pressed();
	protected:
		virtual const radio_button* next_radio_button() const;
		virtual radio_button* next_radio_button();
		virtual bool any_siblings_on() const;
		virtual bool set_checked_state(const boost::optional<bool>& aCheckedState);
	private:
		disc iDisc;
	};
}