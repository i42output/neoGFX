// tree_view.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/tree_view.hpp>
#include <neogfx/gui/widget/item_selection_model.hpp>
#include <neogfx/gui/widget/item_presentation_model.hpp>
#include <neogfx/gui/widget/item_model.hpp>

namespace neogfx
{
    tree_view::tree_view(bool aCreateDefaultModels, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        list_view{ false, aFrameStyle, aScrollbarStyle }
    {
        if (aCreateDefaultModels)
        {
            set_selection_model(std::make_shared<item_selection_model>());
            set_presentation_model(std::make_shared<item_tree_presentation_model>());
            set_model(std::make_shared<item_tree_model>());
        }
    }

    tree_view::tree_view(i_widget& aParent, bool aCreateDefaultModels, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        list_view{ aParent, false, aFrameStyle, aScrollbarStyle }
    {
        if (aCreateDefaultModels)
        {
            set_selection_model(std::make_shared<item_selection_model>());
            set_presentation_model(std::make_shared<item_tree_presentation_model>());
            set_model(std::make_shared<item_tree_model>());
        }
    }

    tree_view::tree_view(i_layout& aLayout, bool aCreateDefaultModels, frame_style aFrameStyle, neogfx::scrollbar_style aScrollbarStyle) :
        list_view{ aLayout, false, aFrameStyle, aScrollbarStyle }
    {
        if (aCreateDefaultModels)
        {
            set_selection_model(std::make_shared<item_selection_model>());
            set_presentation_model(std::make_shared<item_tree_presentation_model>());
            set_model(std::make_shared<item_tree_model>());
        }
    }

    tree_view::~tree_view()
    {
    }
}