// list_view.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gfx/graphics_context.hpp>
#include <neogfx/gui/widget/list_view.hpp>
#include <neogfx/gui/widget/item_selection_model.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/widget/item_model.hpp>

namespace neogfx
{
    list_view::list_view(bool aCreateDefaultModels, frame_style aFrameStyle, scrollbar_style aScrollbarStyle) :
        item_view{ aFrameStyle, aScrollbarStyle },
        iLayout{ *this },
        iSpacer{ iLayout }
    {
        layout().set_size_policy(size_constraint::Expanding);
        layout().set_margins(neogfx::margins{});
        if (aCreateDefaultModels)
        {
            set_selection_model(std::make_shared<item_selection_model>());
            set_presentation_model(std::make_shared<item_presentation_model>());
            set_model(std::make_shared<item_model>());
        }
    }

    list_view::list_view(i_widget& aParent, bool aCreateDefaultModels, frame_style aFrameStyle, scrollbar_style aScrollbarStyle) :
        item_view{ aParent, aFrameStyle, aScrollbarStyle },
        iLayout{ *this },
        iSpacer{ iLayout }
    {
        layout().set_size_policy(size_constraint::Expanding);
        layout().set_margins(neogfx::margins{});
        if (aCreateDefaultModels)
        {
            set_selection_model(std::make_shared<item_selection_model>());
            set_presentation_model(std::make_shared<item_presentation_model>());
            set_model(std::make_shared<item_model>());
        }
    }

    list_view::list_view(i_layout& aLayout, bool aCreateDefaultModels, frame_style aFrameStyle, scrollbar_style aScrollbarStyle) :
        item_view{ aLayout, aFrameStyle, aScrollbarStyle },
        iLayout{ *this },
        iSpacer{ iLayout }
    {
        layout().set_size_policy(size_constraint::Expanding);
        layout().set_margins(neogfx::margins{});
        if (aCreateDefaultModels)
        {
            set_selection_model(std::make_shared<item_selection_model>());
            set_presentation_model(std::make_shared<item_presentation_model>());
            set_model(std::make_shared<item_model>());
        }
    }

    list_view::~list_view()
    {
    }

    bool list_view::can_defer_layout() const
    {
        return true;
    }

    bool list_view::is_managing_layout() const
    {
        return true;
    }

    void list_view::model_changed()
    {
        update_scrollbar_visibility();
    }

    void list_view::presentation_model_changed()
    {
        update_scrollbar_visibility();
    }

    void list_view::selection_model_changed()
    {
    }

    rect list_view::item_display_rect() const
    {
        return client_rect(false);
    }

    size list_view::total_item_area(const i_units_context& aUnitsContext) const
    {
        return has_presentation_model() ?
            size{ 
                column_width(0) + (column_width(0) > 0 ? presentation_model().cell_spacing(*this).cx : 0.0),
                presentation_model().total_height(aUnitsContext)} : 
            size{};
    }

    dimension list_view::column_width(uint32_t) const
    {
        return has_presentation_model() ? 
            presentation_model().column_width(0, graphics_context{ *this, graphics_context::type::Unattached }) : 0.0;
    }

    scrolling_disposition list_view::scrolling_disposition(const i_widget& aChildWidget) const
    {
        return item_view::scrolling_disposition(aChildWidget);
    }
}