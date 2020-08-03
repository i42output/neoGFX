// text_field.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include "label.hpp"
#include "line_edit.hpp"

namespace neogfx
{
    enum class text_field_placement : uint32_t
    {
        LabelLeft,
        LabelAbove
    };
}

template <>
const neolib::enum_enumerators_t<neogfx::text_field_placement> neolib::enum_enumerators_v<neogfx::text_field_placement>
{
    declare_enum_string(neogfx::text_field_placement, LabelLeft)
    declare_enum_string(neogfx::text_field_placement, LabelAbove)
};

namespace neogfx
{
    class text_field : public widget
    {
    private:
        class input_box_container : public framed_widget<>
        {
            typedef framed_widget<> base_type;
        public:
            input_box_container(i_layout& aParentLayout, frame_style aFrameStyle);
        public:
            color frame_color() const override;
        public:
            color palette_color(color_role aColorRole) const override;
        };
    public:
        text_field(const std::string& aLabel = std::string{}, const std::string& aHint = std::string{}, text_field_placement aPlacement = text_field_placement::LabelAbove, frame_style aFrameStyle = frame_style::SolidFrame);
        text_field(i_widget& aParent, const std::string& aLabel = std::string{}, const std::string& aHint = std::string{}, text_field_placement aPlacement = text_field_placement::LabelAbove, frame_style aFrameStyle = frame_style::SolidFrame);
        text_field(i_layout& aLayout, const std::string& aLabel = std::string{}, const std::string& aHint = std::string{}, text_field_placement aPlacement = text_field_placement::LabelAbove, frame_style aFrameStyle = frame_style::SolidFrame);
    public:
        const std::string& text() const;
        void set_text(const std::string& aText);
        const neogfx::label& label() const;
        neogfx::label& label();
        const line_edit& input_box() const;
        line_edit& input_box();
        const text_widget& hint() const;
        text_widget& hint();
        text_field_placement placement() const;
        void set_placement(text_field_placement aPlacement);
        const text_widget& help() const;
        text_widget& help();
    protected:
        neogfx::size_policy size_policy() const override;
    public:
        size minimum_size(const optional_size& aAvailableSpace = optional_size()) const override;
        size maximum_size(const optional_size& aAvailableSpace = optional_size()) const override;  
    private:
        void init();
    private:
        sink iSink;
        text_field_placement iPlacement;
        vertical_layout iLayout;
        neogfx::label iLabel;
        horizontal_layout iInputLayout;
        input_box_container iInputBoxContainer;
        horizontal_layout iInputBoxContainerLayout;
        line_edit iInputBox;
        horizontal_layout iInputBoxLayout;
        text_widget iHint;
        text_widget iHelp;
    };
}