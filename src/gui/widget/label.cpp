// label.cpp
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

#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/text_widget.hpp>
#include <neogfx/gui/layout/i_spacer.hpp>

namespace neogfx
{
    label::label(label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{}, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ *this, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, neogfx::texture{} },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(std::string const& aText, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{}, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ *this, aText, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, neogfx::texture{} },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(const i_texture& aTexture, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{}, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ *this, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, aTexture },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(const i_image& aImage, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{}, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ *this, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, aImage },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(std::string const& aText, const i_texture& aTexture, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{},
        iAlignment{ aAlignment },
        iPlacement{ aPlacement },
        iLayout{ *this },
        iText{ *this, aText, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, aTexture },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(std::string const& aText, const i_image& aImage, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{},
        iAlignment{ aAlignment },
        iPlacement{ aPlacement },
        iLayout{ *this },
        iText{ *this, aText, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, aImage },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(i_widget& aParent, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{ aParent }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ *this, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, neogfx::texture{} },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(i_widget& aParent, std::string const& aText, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{ aParent }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ *this, aText, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, neogfx::texture{} },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(i_widget& aParent, const i_texture& aTexture, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{ aParent }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ *this, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, aTexture },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(i_widget& aParent, const i_image& aImage, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{ aParent }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ *this, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, aImage },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(i_widget& aParent, std::string const& aText, const i_texture& aTexture, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{ aParent },
        iAlignment{ aAlignment },
        iPlacement{ aPlacement },
        iLayout{ *this },
        iText{ *this, aText, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, aTexture },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(i_widget& aParent, std::string const& aText, const i_image& aImage, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{ aParent },
        iAlignment{ aAlignment },
        iPlacement{ aPlacement },
        iLayout{ *this },
        iText{ *this, aText, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, aImage },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(i_layout& aLayout, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{ aLayout }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ *this, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, neogfx::texture{} },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(i_layout& aLayout, std::string const& aText, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{ aLayout }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ *this, aText, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, neogfx::texture{} },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(i_layout& aLayout, const i_texture& aTexture, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{ aLayout }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ *this, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, aTexture },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(i_layout& aLayout, const i_image& aImage, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{ aLayout }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ *this, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, aImage },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(i_layout& aLayout, std::string const& aText, const i_texture& aTexture, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{ aLayout },
        iAlignment{ aAlignment },
        iPlacement{ aPlacement },
        iLayout{ *this },
        iText{ *this, aText, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, aTexture },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::label(i_layout& aLayout, std::string const& aText, const i_image& aImage, label_type aType, neogfx::alignment aAlignment, label_placement aPlacement) :
        widget{ aLayout },
        iAlignment{ aAlignment },
        iPlacement{ aPlacement },
        iLayout{ *this },
        iText{ *this, aText, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ *this, aImage },
        iCenterSpacer{ nullptr }
    {
        init();
    }

    label::~label()
    {
    }

    const grid_layout& label::layout() const
    {
        return iLayout;
    }

    grid_layout& label::layout()
    {
        return iLayout;
    }

    size_policy label::size_policy() const
    {
        if (has_size_policy())
            return widget::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return size_constraint::Minimum;
    }

    void label::set_font_role(const optional_font_role& aFontRole)
    {
        widget::set_font_role(aFontRole);
        text_widget().set_font_role(aFontRole);
        image_widget().set_font_role(aFontRole);
    }

    void label::set_font(optional_font const& aFont)
    {
        widget::set_font(aFont);
        text_widget().set_font(aFont);
        image_widget().set_font(aFont);
    }

    alignment label::alignment() const
    {
        return iAlignment;
    }

    void label::set_alignment(neogfx::alignment aAlignment)
    {
        if (iAlignment != aAlignment)
        {
            iAlignment = aAlignment;
            layout().set_alignment(alignment());
            text_widget().set_alignment(alignment());
            update_layout();
        }
    }

    i_string const& label::text() const
    {
        return text_widget().text();
    }

    void label::set_text(i_string const& aText)
    {
        text_widget().set_text(aText);
    }

    const texture& label::image() const
    {
        return image_widget().image();
    }

    void label::set_image(i_string const& aImageUri)
    {
        image_widget().set_image(aImageUri);
    }

    void label::set_image(const neogfx::image& aImage)
    {
        image_widget().set_image(aImage);
    }

    void label::set_image(const texture& aImage)
    {
        image_widget().set_image(aImage);
    }

    label_placement label::placement() const
    {
        return iPlacement;
    }

    void label::set_placement(label_placement aPlacement)
    {
        if (iPlacement == aPlacement)
            return;
        iPlacement = aPlacement;
        handle_placement_change();
    }

    const text_widget& label::text_widget() const
    {
        return iText;
    }

    text_widget& label::text_widget()
    {
        return iText;
    }

    const image_widget& label::image_widget() const
    {
        return iImage;
    }

    image_widget& label::image_widget()
    {
        return iImage;
    }

    const i_spacer& label::center_spacer() const
    {
        if (iCenterSpacer != nullptr)
            return *iCenterSpacer;
        throw no_center_spacer();
    }

    i_spacer& label::center_spacer()
    {
        if (iCenterSpacer != nullptr)
            return *iCenterSpacer;
        throw no_center_spacer();
    }

    bool label::has_buddy() const
    {
        return iBuddy != nullptr;
    }

    i_widget& label::buddy() const
    {
        if (has_buddy())
            return *iBuddy;
        throw no_buddy();
    }

    void label::set_buddy(i_widget& aBuddy)
    {
        iBuddy = std::shared_ptr<i_widget>(std::shared_ptr<i_widget>(), &aBuddy);
    }

    void label::set_buddy(std::shared_ptr<i_widget> aBuddy)
    {
        iBuddy = aBuddy;
    }

    void label::unset_buddy()
    {
        iBuddy.reset();
    }

    void label::init()
    {
        layout().set_alignment(alignment());
        text_widget().set_alignment(alignment());
        image_widget().set_dpi_auto_scale(true);
        layout().set_padding(neogfx::padding{});
        text_widget().set_padding(neogfx::padding{});
        image_widget().set_padding(neogfx::padding{});
        set_ignore_mouse_events(true);
        text_widget().set_ignore_mouse_events(true);
        image_widget().set_ignore_mouse_events(true);
        handle_placement_change();
        iSink += layout().AlignmentChanged([this]() { handle_placement_change(); });
        iSink += text_widget().VisibilityChanged([this](){ handle_placement_change(); });
        iSink += text_widget().TextGeometryChanged([this]() { if (text_widget().visible()) handle_placement_change(); });
        iSink += image_widget().VisibilityChanged([this]() { handle_placement_change(); });
        iSink += image_widget().ImageGeometryChanged([this]() { if (image_widget().visible()) handle_placement_change(); });
    }

    label_placement label::effective_placement() const
    {
        switch (iPlacement)
        {
        case label_placement::TextImageHorizontal:
            if (image().is_empty() || image_widget().hidden())
                return label_placement::TextHorizontal;
            else if (text().empty() || text_widget().hidden())
                return label_placement::ImageHorizontal;
            break;
        case label_placement::TextImageVertical:
            if (image().is_empty() || image_widget().hidden())
                return label_placement::TextVertical;
            else if (text().empty() || text_widget().hidden())
                return label_placement::ImageVertical;
            break;
        case label_placement::ImageTextHorizontal:
            if (image().is_empty() || image_widget().hidden())
                return label_placement::TextHorizontal;
            else if (text().empty() || text_widget().hidden())
                return label_placement::ImageHorizontal;
            break;
        case label_placement::ImageTextVertical:
            if (image().is_empty() || image_widget().hidden())
                return label_placement::TextVertical;
            else if (text().empty() || text_widget().hidden())
                return label_placement::ImageVertical;
            break;
        default:
            break;
        }
        return iPlacement;
    }

    void label::handle_placement_change()
    {
        layout().remove_all();
        iCenterSpacer = nullptr;
        switch (effective_placement())
        {
        case label_placement::TextHorizontal:
            layout().add_item_at_position(0, 0, text_widget());
            break;
        case label_placement::TextVertical:
            layout().add_item_at_position(0, 0, text_widget());
            break;
        case label_placement::ImageHorizontal:
            layout().add_item_at_position(0, 0, image_widget());
            break;
        case label_placement::ImageVertical:
            layout().add_item_at_position(0, 0, image_widget());
            break;
        case label_placement::TextImageHorizontal:
            switch (layout().alignment() & neogfx::alignment::Horizontal)
            {
            case neogfx::alignment::Left:
                layout().add_item_at_position(0, 0, text_widget());
                layout().add_item_at_position(0, 1, image_widget());
                layout().add_spacer_at_position(0, 2);
                break;
            case neogfx::alignment::Center:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(0, 1, text_widget());
                layout().add_item_at_position(0, 2, image_widget());
                layout().add_spacer_at_position(0, 3);
                break;
            case neogfx::alignment::Right:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(0, 1, text_widget());
                layout().add_item_at_position(0, 2, image_widget());
                break;
            }
            break;
        case label_placement::TextImageVertical:
            switch (layout().alignment() & neogfx::alignment::Vertical)
            {
            case neogfx::alignment::Top:
                layout().add_item_at_position(0, 0, text_widget());
                layout().add_item_at_position(1, 0, image_widget());
                layout().add_spacer_at_position(2, 0);
                break;
            case neogfx::alignment::VCenter:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(1, 0, text_widget());
                layout().add_item_at_position(2, 0, image_widget());
                layout().add_spacer_at_position(3, 0);
                break;
            case neogfx::alignment::Bottom:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(1, 0, text_widget());
                layout().add_item_at_position(2, 0, image_widget());
                break;
            }
            break;
        case label_placement::ImageTextHorizontal:
            switch (layout().alignment() & neogfx::alignment::Horizontal)
            {
            case neogfx::alignment::Left:
                layout().add_item_at_position(0, 0, image_widget());
                layout().add_item_at_position(0, 1, text_widget());
                layout().add_spacer_at_position(0, 2);
                break;
            case neogfx::alignment::Center:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(0, 1, image_widget());
                layout().add_item_at_position(0, 2, text_widget());
                layout().add_spacer_at_position(0, 3);
                break;
            case neogfx::alignment::Right:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(0, 1, image_widget());
                layout().add_item_at_position(0, 2, text_widget());
                break;
            }
            break;
        case label_placement::ImageTextVertical:
            switch (layout().alignment() & neogfx::alignment::Vertical)
            {
            case neogfx::alignment::Top:
                layout().add_item_at_position(0, 0, image_widget());
                layout().add_item_at_position(1, 0, text_widget());
                layout().add_spacer_at_position(2, 0);
                break;
            case neogfx::alignment::VCenter:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(1, 0, image_widget());
                layout().add_item_at_position(2, 0, text_widget());
                layout().add_spacer_at_position(3, 0);
                break;
            case neogfx::alignment::Bottom:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(1, 0, image_widget());
                layout().add_item_at_position(2, 0, text_widget());
                break;
            }
            break;
        case label_placement::TextSpacerImageHorizontal:
            switch (layout().alignment() & neogfx::alignment::Horizontal)
            {
            case neogfx::alignment::Left:
                layout().add_item_at_position(0, 0, text_widget());
                iCenterSpacer = &layout().add_spacer_at_position(0, 1);
                layout().add_item_at_position(0, 2, image_widget());
                layout().add_spacer_at_position(0, 3);
                break;
            case neogfx::alignment::Center:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(0, 1, text_widget());
                iCenterSpacer = &layout().add_spacer_at_position(0, 2);
                layout().add_item_at_position(0, 3, image_widget());
                layout().add_spacer_at_position(0, 3);
                break;
            case neogfx::alignment::Right:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(0, 1, text_widget());
                iCenterSpacer = &layout().add_spacer_at_position(0, 2);
                layout().add_item_at_position(0, 3, image_widget());
                break;
            }
            break;
        case label_placement::TextSpacerImageVertical:
            switch (layout().alignment() & neogfx::alignment::Vertical)
            {
            case neogfx::alignment::Top:
                layout().add_item_at_position(0, 0, text_widget());
                iCenterSpacer = &layout().add_spacer_at_position(1, 0);
                layout().add_item_at_position(2, 0, image_widget());
                layout().add_spacer_at_position(3, 0);
                break;
            case neogfx::alignment::VCenter:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(1, 0, text_widget());
                iCenterSpacer = &layout().add_spacer_at_position(2, 0);
                layout().add_item_at_position(3, 0, image_widget());
                layout().add_spacer_at_position(4, 0);
                break;
            case neogfx::alignment::Bottom:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(1, 0, text_widget());
                iCenterSpacer = &layout().add_spacer_at_position(2, 0);
                layout().add_item_at_position(3, 0, image_widget());
                break;
            }
            break;
        case label_placement::ImageSpacerTextHorizontal:
            switch (layout().alignment() & neogfx::alignment::Horizontal)
            {
            case neogfx::alignment::Left:
                layout().add_item_at_position(0, 0, image_widget());
                iCenterSpacer = &layout().add_spacer_at_position(0, 1);
                layout().add_item_at_position(0, 2, text_widget());
                layout().add_spacer_at_position(0, 3);
                break;
            case neogfx::alignment::Center:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(0, 1, image_widget());
                iCenterSpacer = &layout().add_spacer_at_position(0, 2);
                layout().add_item_at_position(0, 3, text_widget());
                layout().add_spacer_at_position(0, 4);
                break;
            case neogfx::alignment::Right:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(0, 1, image_widget());
                iCenterSpacer = &layout().add_spacer_at_position(0, 2);
                layout().add_item_at_position(0, 3, text_widget());
                break;
            }
            break;
        case label_placement::ImageSpacerTextVertical:
            switch (layout().alignment() & neogfx::alignment::Vertical)
            {
            case neogfx::alignment::Top:
                layout().add_item_at_position(0, 0, image_widget());
                iCenterSpacer = &layout().add_spacer_at_position(1, 0);
                layout().add_item_at_position(2, 0, text_widget());
                layout().add_spacer_at_position(3, 0);
                break;
            case neogfx::alignment::VCenter:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(1, 0, image_widget());
                iCenterSpacer = &layout().add_spacer_at_position(2, 0);
                layout().add_item_at_position(3, 0, text_widget());
                layout().add_spacer_at_position(4, 0);
                break;
            case neogfx::alignment::Bottom:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(1, 0, image_widget());
                iCenterSpacer = &layout().add_spacer_at_position(2, 0);
                layout().add_item_at_position(3, 0, text_widget());
                break;
            }
            break;
        }
    }
}