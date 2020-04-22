// message_box.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/dialog/dialog.hpp>

namespace neogfx
{
    class message_box : public dialog
    {
    public:
        struct no_response : std::logic_error { no_response() : std::logic_error("neogfx::message_box::no_response") {} };
    public:
        message_box(const std::string& aTitle, const image& aIcon, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        message_box(const std::string& aTitle, const image& aIcon, const std::string& aText, const std::string& aDetailedText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        message_box(i_widget& aParent, const std::string& aTitle, const image& aIcon, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        message_box(i_widget& aParent, const std::string& aTitle, const image& aIcon, const std::string& aText, const std::string& aDetailedText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
    public:
        static standard_button information(const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button information(const std::string& aTitle, const std::string& aText, const std::string& aDetailedText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button information(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button information(i_widget& aParent, const std::string& aTitle, const std::string& aText, const std::string& aDetailedText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button question(const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button question(const std::string& aTitle, const std::string& aText, const std::string& aDetailedText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button question(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button question(i_widget& aParent, const std::string& aTitle, const std::string& aText, const std::string& aDetailedText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button warning(const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button warning(const std::string& aTitle, const std::string& aText, const std::string& aDetailedText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button warning(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button warning(i_widget& aParent, const std::string& aTitle, const std::string& aText, const std::string& aDetailedText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button stop(const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button stop(const std::string& aTitle, const std::string& aText, const std::string& aDetailedText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button stop(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button stop(i_widget& aParent, const std::string& aTitle, const std::string& aText, const std::string& aDetailedText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button error(const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button error(const std::string& aTitle, const std::string& aText, const std::string& aDetailedText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button error(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button error(i_widget& aParent, const std::string& aTitle, const std::string& aText, const std::string& aDetailedText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button critical(const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button critical(const std::string& aTitle, const std::string& aText, const std::string& aDetailedText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button critical(i_widget& aParent, const std::string& aTitle, const std::string& aText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
        static standard_button critical(i_widget& aParent, const std::string& aTitle, const std::string& aText, const std::string& aDetailedText, standard_button aButtons = standard_button::Ok | standard_button::Cancel);
    public:
        const image_widget& icon() const;
        image_widget& icon();
        const text_widget& text() const;
        text_widget& text();
        const text_widget& detailed_text() const;
        text_widget& detailed_text();
    public:
        bool has_response() const;
        standard_button response() const;
        void set_response(standard_button aResponse);
    public:
        dialog_result exec() override;
    protected:
        size maximum_size(const optional_size& aAvailableSpace = {}) const override;
    private:
        void init();
    private:
        horizontal_layout iLayout1;
        image_widget iIcon;
        vertical_layout iLayout2;
        text_widget iText;
        text_widget iDetailedText;
        std::optional<standard_button> iResponse;
    };
}