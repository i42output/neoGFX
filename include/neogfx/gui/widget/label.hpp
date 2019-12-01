// label.hpp
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
#include <neolib/i_enum.hpp>
#include <neogfx/gui/layout/grid_layout.hpp>
#include <neogfx/gui/widget/text_widget.hpp>
#include <neogfx/gui/widget/image_widget.hpp>

namespace neogfx
{
    class i_spacer;

    enum class label_placement
    {
        TextHorizontal,
        TextVertical,
        ImageHorizontal,
        ImageVertical,
        TextImageHorizontal,
        TextImageVertical,
        ImageTextHorizontal,
        ImageTextVertical,
        TextSpacerImageHorizontal,
        TextSpacerImageVertical,
        ImageSpacerTextHorizontal,
        ImageSpacerTextVertical
    };
}

template <>
const neolib::enum_enumerators_t<neogfx::label_placement> neolib::enum_enumerators_v<neogfx::label_placement>
{
    declare_enum_string(neogfx::label_placement, TextHorizontal)
    declare_enum_string(neogfx::label_placement, TextVertical)
    declare_enum_string(neogfx::label_placement, ImageHorizontal)
    declare_enum_string(neogfx::label_placement, ImageVertical)
    declare_enum_string(neogfx::label_placement, TextImageHorizontal)
    declare_enum_string(neogfx::label_placement, TextImageVertical)
    declare_enum_string(neogfx::label_placement, ImageTextHorizontal)
    declare_enum_string(neogfx::label_placement, ImageTextVertical)
    declare_enum_string(neogfx::label_placement, TextSpacerImageHorizontal)
    declare_enum_string(neogfx::label_placement, TextSpacerImageVertical)
    declare_enum_string(neogfx::label_placement, ImageSpacerTextHorizontal)
    declare_enum_string(neogfx::label_placement, ImageSpacerTextVertical)
};

namespace neogfx
{
    typedef text_widget_type label_type;

    class label : public widget
    {
    public:
        struct no_centre_spacer : std::logic_error { no_centre_spacer() : std::logic_error("neogfx::label::no_centre_spacer") {} };
        struct no_buddy : std::logic_error { no_buddy() : std::logic_error("neogfx::label::no_buddy") {} };
    public:
        label(label_type aType = label_type::MultiLine, alignment aAlignment = alignment::Left | alignment::VCentre, label_placement aPlacement = label_placement::ImageTextHorizontal);
        label(const std::string& aText, label_type aType = label_type::MultiLine, alignment aAlignment = alignment::Left | alignment::VCentre, label_placement aPlacement = label_placement::ImageTextHorizontal);
        label(const i_texture& aTexture, label_type aType = label_type::MultiLine, alignment aAlignment = alignment::Left | alignment::VCentre, label_placement aPlacement = label_placement::ImageTextHorizontal);
        label(const i_image& aImage, label_type aType = label_type::MultiLine, alignment aAlignment = alignment::Left | alignment::VCentre, label_placement aPlacement = label_placement::ImageTextHorizontal);
        label(i_widget& aParent, label_type aType = label_type::MultiLine, alignment aAlignment = alignment::Left | alignment::VCentre, label_placement aPlacement = label_placement::ImageTextHorizontal);
        label(i_widget& aParent, const std::string& aText, label_type aType = label_type::MultiLine, alignment aAlignment = alignment::Left | alignment::VCentre, label_placement aPlacement = label_placement::ImageTextHorizontal);
        label(i_widget& aParent, const i_texture& aTexture, label_type aType = label_type::MultiLine, alignment aAlignment = alignment::Left | alignment::VCentre, label_placement aPlacement = label_placement::ImageTextHorizontal);
        label(i_widget& aParent, const i_image& aImage, label_type aType = label_type::MultiLine, alignment aAlignment = alignment::Left | alignment::VCentre, label_placement aPlacement = label_placement::ImageTextHorizontal);
        label(i_layout& aLayout, label_type aType = label_type::MultiLine, alignment aAlignment = alignment::Left | alignment::VCentre, label_placement aPlacement = label_placement::ImageTextHorizontal);
        label(i_layout& aLayout, const std::string& aText, label_type aType = label_type::MultiLine, alignment aAlignment = alignment::Left | alignment::VCentre, label_placement aPlacement = label_placement::ImageTextHorizontal);
        label(i_layout& aLayout, const i_texture& aTexture, label_type aType = label_type::MultiLine, alignment aAlignment = alignment::Left | alignment::VCentre, label_placement aPlacement = label_placement::ImageTextHorizontal);
        label(i_layout& aLayout, const i_image& aImage, label_type aType = label_type::MultiLine, alignment aAlignment = alignment::Left | alignment::VCentre, label_placement aPlacement = label_placement::ImageTextHorizontal);
        ~label();
    public:
        const grid_layout& layout() const override;
        grid_layout& layout() override;
    public:
        neogfx::size_policy size_policy() const override;
        using widget::set_size_policy;
        void set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout = true) override;
    public:
        const std::string& text() const;
        void set_text(const std::string& aText);
        const texture& image() const;
        void set_image(const std::string& aImageUri);
        void set_image(const neogfx::image& aImage);
        void set_image(const texture& aImage);
        label_placement placement() const;
        void set_placement(label_placement aPlacement);
        const neogfx::text_widget& text_widget() const;
        neogfx::text_widget& text_widget();
        const neogfx::image_widget& image_widget() const;
        neogfx::image_widget& image_widget();
        const i_spacer& centre_spacer() const;
        i_spacer& centre_spacer();
        bool has_buddy() const;
        i_widget& buddy() const;
        void set_buddy(i_widget& aBuddy);
        void set_buddy(std::shared_ptr<i_widget> aBuddy);
        void unset_buddy();
    private:
        void init();
        label_placement effective_placement() const;
        void handle_placement_change();
    private:
        sink iSink;
        alignment iAlignment;
        label_placement iPlacement;
        grid_layout iLayout;
        neogfx::text_widget iText;
        neogfx::image_widget iImage;
        i_spacer* iCentreSpacer;
        std::shared_ptr<i_widget> iBuddy;
    };
}