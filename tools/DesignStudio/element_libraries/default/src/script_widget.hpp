// script_widget.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2021 Leigh Johnston
  
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
#include <boost/format.hpp>
#include <neogfx/gui/window/window.hpp>
#include <neogfx/gui/widget/text_edit.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>
#include <neogfx/tools/DesignStudio/i_element_library.hpp>
#include <neogfx/tools/DesignStudio/script.hpp>

namespace neogfx::DesignStudio
{
    class script_widget : public window
    {
        typedef window base_type;
    public:
        script_widget(i_element& aElement, i_widget& aParent, window_style aStyle) :
            window{ aParent, window_placement{ rect{ point{}, size{ 256.0_dip, 160.0_dip } } }, aStyle },
            iEditor{ client_layout() }
        {
            set_minimum_size(size{ 256.0_dip, 160.0_dip });
            title_bar().set_icon(aElement.library().element_icon(aElement.type()));
            title_bar().set_title(""_s);
            create_status_bar<neogfx::status_bar>(
                neogfx::status_bar::style::DisplayMessage | neogfx::status_bar::style::DisplaySizeGrip | neogfx::status_bar::style::BackgroundAsWindowBorder );
            iDocInfo = status_bar().add_permanent_widget<text_widget>();
            iCursorInfo = status_bar().add_permanent_widget<text_widget>();
            iDocInfo->set_font_role(font_role::StatusBar);
            iCursorInfo->set_font_role(font_role::StatusBar);
            auto text_changed = [&]()
            {
                iDocInfo->set_text(string{ boost::str(boost::format("Length: %1%  Lines: %2%") %
                    iEditor.plain_text().size() %
                    (std::count(iEditor.plain_text().begin(), iEditor.plain_text().end(), '\n') + 1)) });
            };
            iEditor.TextChanged(text_changed);
            auto position_changed = [&]()
            {
                auto currentPos = std::next(iEditor.plain_text().rbegin(), iEditor.plain_text().size() - iEditor.cursor().position());
                auto startLine = std::find(currentPos, iEditor.plain_text().rend(), '\n');
                iCursorInfo->set_text(string{ boost::str(boost::format("Ln: %1%  Col: %2%  Pos: %3%") %
                    (std::count(iEditor.plain_text().begin(), std::next(iEditor.plain_text().begin(), iEditor.cursor().position()), '\n') + 1) %
                    (std::distance(currentPos, startLine) + 1) %
                    (iEditor.cursor().position() + 1)) });
            };
            iEditor.cursor().PositionChanged(position_changed); 
            iEditor.set_word_wrap(false);
            text_changed();
            position_changed();
        }
        ~script_widget()
        {
        }
    private:
        text_edit iEditor;
        ref_ptr<text_widget> iDocInfo;
        ref_ptr<text_widget> iCursorInfo;
        sink iSink;
    };

    template <>
    struct element_traits<script_widget> : element_traits<i_element>
    {
        typedef script base;
        static constexpr bool needsCaddy = false;
    };
}
