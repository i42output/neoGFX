// check_box.hpp
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
#include "spacer.hpp"

namespace neogfx
{
	class check_box : public button
	{
	public:
		event<> checked;
		event<> unchecked;
		event<> tristate;
	public:
		enum style_e
		{
			BiState,
			TriState
		};
	public:
		struct not_tri_state : public std::logic_error { not_tri_state() : std::logic_error("neogfx::check_box::not_tri_state") {} };
	private:
		class box : public widget
		{
		public:
			box(check_box& aParent);
		public:
			virtual size minimum_size() const;
			virtual size maximum_size() const;
		public:
			virtual void paint(graphics_context& aGraphicsContext) const;
		};
	public:
		check_box(const std::string& aText = std::string(), style_e aStyle = BiState);
		check_box(i_widget& aParent, const std::string& aText = std::string(), style_e aStyle = BiState);
		check_box(i_layout& aLayout, const std::string& aText = std::string(), style_e aStyle = BiState);
	public:
		bool is_checked() const;
		void set_checked(bool aChecked);
		bool is_tristate() const;
		void set_tristate();
		void toggle();
	protected:
		virtual void handle_pressed();
	private:
		style_e iStyle;
		box iBox;
		boost::optional<bool> iChecked;
	};
}