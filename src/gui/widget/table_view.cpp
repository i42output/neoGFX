// table_view.cpp
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

#include <neogfx/gui/widget/table_view.hpp>
#include <neogfx/gui/widget/item_selection_model.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/widget/item_model.hpp>

namespace neogfx
{
    table_view::table_view(bool aCreateDefaultModels, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        item_view{ aFrameStyle, aScrollbarStyle },
        iLayout{ *this },
        iColumnHeader{ iLayout, *this },
        iSpacer{ iLayout }
    {
        layout().set_padding(neogfx::padding{});
        if (aCreateDefaultModels)
        {
            set_selection_model(make_ref<item_selection_model>());
            set_presentation_model(make_ref<item_presentation_model>());
            set_model(make_ref<item_model>());
        }
    }

    table_view::table_view(i_widget& aParent, bool aCreateDefaultModels, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        item_view{ aParent, aFrameStyle, aScrollbarStyle },
        iLayout{ *this },
        iColumnHeader{ iLayout, *this },
        iSpacer{ iLayout }
    {
        layout().set_padding(neogfx::padding{});
        if (aCreateDefaultModels)
        {
            set_selection_model(make_ref<item_selection_model>());
            set_presentation_model(make_ref<item_presentation_model>());
            set_model(make_ref<item_model>());
        }
    }

    table_view::table_view(i_layout& aLayout, bool aCreateDefaultModels, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        item_view{ aLayout, aFrameStyle, aScrollbarStyle },
        iLayout{ *this },
        iColumnHeader{ iLayout, *this },
        iSpacer{ iLayout }
    {
        layout().set_padding(neogfx::padding{});
        if (aCreateDefaultModels)
        {
            set_selection_model(make_ref<item_selection_model>());
            set_presentation_model(make_ref<item_presentation_model>());
            set_model(make_ref<item_model>());
        }
    }

    table_view::~table_view()
    {
    }

    bool table_view::is_managing_layout() const
    {
        return true;
    }

    const header_view& table_view::column_header() const
    {
        return iColumnHeader;
    }

    header_view& table_view::column_header()
    {
        return iColumnHeader;
    }

    void table_view::model_changed()
    {
        column_header().set_model(model());
        update_scrollbar_visibility();
    }

    void table_view::presentation_model_changed()
    {
        if (has_presentation_model())
            column_header().set_presentation_model(presentation_model());
        else
            column_header().set_presentation_model(ref_ptr<i_item_presentation_model>{});
        update_scrollbar_visibility();
    }

    void table_view::selection_model_changed()
    {
    }

    void table_view::tree_changed()
    {
        update_scrollbar_visibility();
    }

    rect table_view::item_display_rect(bool aIntersectItemArea) const
    {
        auto const clientRect = client_rect(false);
        rect result{
            clientRect.top_left() + point{ 0.0, column_header().visible() ? column_header().extents().cy : 0.0 },
            size{ std::min(clientRect.width(), column_header().total_width()), clientRect.height() - (column_header().visible() ? column_header().extents().cy : 0.0) } };
        if (aIntersectItemArea)
            result.extents().cy = std::min(result.extents().cy, total_item_area(*this).cy);
        return result;
    }

    size table_view::total_item_area(i_units_context const& aUnitsContext) const
    {
        return size(column_header().total_width(), has_presentation_model() ? presentation_model().total_height(aUnitsContext) : 0.0);
    }

    dimension table_view::column_width(std::uint32_t aColumn) const
    {
        return column_header().section_width(aColumn);
    }

    scrolling_disposition table_view::scrolling_disposition(const i_widget& aChildWidget) const
    {
        if (&aChildWidget == &column_header())
            return neogfx::scrolling_disposition::ScrollChildWidgetHorizontally;
        else
            return item_view::scrolling_disposition(aChildWidget);
    }
}