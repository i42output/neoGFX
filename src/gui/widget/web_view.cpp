// web_view.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/widget/widget.ipp>
#include <neogfx/gui/widget/web_view.hpp>

namespace neogfx
{
    template class widget<i_web_view>;

    web_view::web_view(i_widget& aParent) : 
        base_type{ aParent },
        iLayout{ *this }
    {
        init();
    }

    web_view::web_view(i_layout& aLayout) :
        base_type{ aLayout },
        iLayout{ *this }
    {
        init();
    }

    web_view::~web_view()
    {
    }

    void web_view::load_url(i_string const& aUrl, bool aSetFocus)
    {
        if (iCanvas)
            iCanvas->load_url(aUrl, aSetFocus);
    }

    void web_view::paint(i_graphics_context& aGc) const
    {
        base_type::paint(aGc);

        rect canvasRect{ iLayout.position(), iLayout.extents() };
        canvasRect.deflate(iLayout.margin());
        canvasRect.deflate(iLayout.border());
        canvasRect.deflate(iLayout.padding());
        draw_alpha_background(aGc, canvasRect, 8.0_dip);
    }

    void web_view::init()
    {
        set_size_policy(size_constraint::Expanding);
        iLayout.set_size_policy(size_constraint::Expanding);

        try
        {
            ref_ptr<i_web_view_factory> factory{ service<i_app>() };
            iCanvas = factory->create_canvas(iLayout);
        }
        catch(...)
        {
            iLayout.add_spacer();
            auto& errorBoxLayout = iLayout.add<horizontal_layout>();
            errorBoxLayout.set_size_policy(size_constraint::Expanding, size_constraint::Minimum);
            errorBoxLayout.add_spacer();
            iErrorBox.emplace(errorBoxLayout, "ERROR: Web browser unavailable"_t);
            errorBoxLayout.add_spacer();
            iLayout.add_spacer();
            iErrorBox.value().set_padding(neogfx::padding{ 4.0_dip });
            iErrorBox.value().set_font(iErrorBox.value().font().with_style(font_style::Bold).with_size(iErrorBox.value().font().size() * 1.5));
            iErrorBox.value().set_background_color(color::Black);
            iErrorBox.value().set_text_color(color::White);
            return;
        }

        iCanvas->navigated_to([&](i_string const& aUrl)
        {
            NavigatedTo.trigger(aUrl);
        });
    }
}

