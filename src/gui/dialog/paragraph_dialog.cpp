// paragraph_dialog.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2023 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/core/scoped.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/dialog/paragraph_dialog.hpp>
#include <neogfx/gui/dialog/message_box.hpp>

namespace neogfx
{
    paragraph_dialog::paragraph_dialog(paragraph_format const& aCurrentParagraphFormat) :
        dialog{ "Paragraph Format"_t, window_style::Dialog | window_style::Modal | window_style::TitleBar | window_style::Close },
        iUpdating{ false },
        iCurrentParagraphFormat{ aCurrentParagraphFormat },
        iSelectedParagraphFormat{ aCurrentParagraphFormat },
        iLayout0{ client_layout() },
        iSampleBox{ iLayout0, "Sample"_t },
        iSample{ iSampleBox.with_item_layout<horizontal_layout>() }
    {
        init();
    }

    paragraph_dialog::paragraph_dialog(i_widget& aParent, paragraph_format const& aCurrentParagraphFormat) :
        dialog{ aParent, "Paragraph Format"_t, window_style::Dialog | window_style::Modal | window_style::TitleBar | window_style::Close },
        iUpdating{ false },
        iCurrentParagraphFormat{ aCurrentParagraphFormat },
        iSelectedParagraphFormat{ aCurrentParagraphFormat },
        iLayout0{ client_layout() },
        iSampleBox{ iLayout0, "Sample"_t },
        iSample{ iSampleBox.with_item_layout<horizontal_layout>() }
    {
        init();
    }

    paragraph_dialog::~paragraph_dialog()
    {
    }

    paragraph_format const& paragraph_dialog::current_format() const
    {
        return iCurrentParagraphFormat;
    }

    paragraph_format const& paragraph_dialog::selected_format() const
    {
        return iSelectedParagraphFormat;
    }
    
    void paragraph_dialog::select_paragraph_format(const neogfx::paragraph_format& aFormat)
    {
        iSelectedParagraphFormat = aFormat;
        update_selected_format(*this);
    }

    size paragraph_dialog::minimum_size(optional_size const& aAvailableSpace) const
    {
        auto result = dialog::minimum_size(aAvailableSpace);
        if (dialog::has_minimum_size())
            return result;
        return result;
    }

    void paragraph_dialog::init()
    {
        iSink += iSample.Painting([&](i_graphics_context& aGc)
        {
            {
                scoped_opacity so1{ aGc, 0.25 };
                draw_alpha_background(aGc, iSample.client_rect(), dpi_scale(4.0));
            }
            {
                scoped_opacity so2{ aGc, 0.75 };
                aGc.fill_rect(iSample.client_rect(), color::White);
            }
        });
        scoped_units su{ *this };
        iSample.set_minimum_size(size{ 3_in, 2_in });
        iSample.set_read_only();
        iSample.set_text(
            "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. "
            "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. "
            "The quick brown fox jumps over the lazy dog.\n"
            "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. "
            "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. "
            "The quick brown fox jumps over the lazy dog.\n"
            "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. "
            "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. "
            "The quick brown fox jumps over the lazy dog.");
        iSample.set_default_style(iSample.default_style().with_text_color(color::Black));
        iSample.set_page_rect(iSample.to_client_rect(iSample.minimum_size() * 2.0, false));
        //auto v = iSample.view();
        //v.zoom(1.0);
        //iSample.set_view(v);
        center_on_parent();
        update_selected_format(*this);
        set_ready_to_render(true);
    }

    void paragraph_dialog::update_selected_format(i_widget const& aUpdatingWidget)
    {
        if (iUpdating)
            return;
        neolib::scoped_flag sf{ iUpdating };
        auto oldSelectedParagraphFormat = iSelectedParagraphFormat;

        // todo

        //iSample.set_paragraph_format(iSelectedParagraphFormat);
        
        if (iSelectedParagraphFormat != oldSelectedParagraphFormat)
            SelectionChanged.trigger();

        update_widgets();
    }
    
    void paragraph_dialog::update_widgets()
    {
        // todo
    }
}