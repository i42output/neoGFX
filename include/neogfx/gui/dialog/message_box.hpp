// message_box.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/dialog/dialog.hpp>

namespace neogfx
{
	class message_box : public dialog
	{
	public:
		message_box(const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
		message_box(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
	public:
		static standard_button information(const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
		static standard_button information(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
		static standard_button question(const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
		static standard_button question(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
		static standard_button warning(const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
		static standard_button warning(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
		static standard_button stop(const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
		static standard_button stop(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
		static standard_button error(const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
		static standard_button error(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
		static standard_button critical(const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
		static standard_button critical(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
	public:
		const image_widget& icon() const;
		image_widget& icon();
		const text_widget& text() const;
		text_widget& text();
		const text_widget& detailed_text() const;
		text_widget& detailed_text();
	private:
		void init();
	private:
		horizontal_layout iLayout1;
		image_widget iIcon;
		vertical_layout iLayout2;
		text_widget iText;
		text_widget iDetailedText;
	};
}