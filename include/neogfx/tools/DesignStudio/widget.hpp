// widget.cpp
/*
neoGFX Design Studio
Copyright(C) 2020 Leigh Johnston

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

#include <DesignStudio/DesignStudio.hpp>

namespace neogfx::DesignStudio
{
    class widget_property_model : public ng::item_tree_model
    {
    public:
    };

    class widget_property_presentation_model : public ng::item_tree_presentation_model
    {
    public:
        widget_property_presentation_model() : 
            ng::item_tree_presentation_model{}
        {
        }
    public:
        ng::optional_color cell_color(const ng::item_presentation_model_index& aIndex, ng::color_role aColorRole) const override
        {
            if (aColorRole == ng::color_role::Background)
                return ng::service<ng::i_app>().current_style().palette().color(ng::color_role::Background).shade(aIndex.row() % 2 == 0 ? 0x00 : 0x08);
            return {};
        }
    public:
    };
}

