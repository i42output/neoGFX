// label.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/text_widget.hpp>
#include <neogfx/gui/layout/i_spacer.hpp>

namespace neogfx
{
    label::label(label_type aType, alignment aAlignment, label_placement aPlacement) :
        widget{}, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ iLayout, neogfx::texture{} },
        iCentreSpacer{ nullptr }
    {
        layout().set_alignment(aAlignment);
        text_widget().set_alignment(aAlignment);
        init();
    }

    label::label(const std::string& aText, label_type aType, alignment aAlignment, label_placement aPlacement) :
        widget{}, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ iLayout, aText, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ iLayout, neogfx::texture{} },
        iCentreSpacer{ nullptr }
    {
        layout().set_alignment(aAlignment);
        text_widget().set_alignment(aAlignment);
        init();
    }

    label::label(const i_texture& aTexture, label_type aType, alignment aAlignment, label_placement aPlacement) :
        widget{}, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ iLayout, aTexture },
        iCentreSpacer{ nullptr }
    {
        layout().set_alignment(aAlignment);
        text_widget().set_alignment(aAlignment);
        init();
    }

    label::label(const i_image& aImage, label_type aType, alignment aAlignment, label_placement aPlacement) :
        widget{}, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ iLayout, aImage },
        iCentreSpacer{ nullptr }
    {
        layout().set_alignment(aAlignment);
        text_widget().set_alignment(aAlignment);
        init();
    }

    label::label(i_widget& aParent, label_type aType, alignment aAlignment, label_placement aPlacement) :
        widget{ aParent }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ iLayout, neogfx::texture{} },
        iCentreSpacer{ nullptr }
    {
        layout().set_alignment(aAlignment);
        text_widget().set_alignment(aAlignment);
        init();
    }

    label::label(i_widget& aParent, const std::string& aText, label_type aType, alignment aAlignment, label_placement aPlacement) :
        widget{ aParent }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ iLayout, aText, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ iLayout, neogfx::texture{} },
        iCentreSpacer{ nullptr }
    {
        layout().set_alignment(aAlignment);
        text_widget().set_alignment(aAlignment);
        init();
    }

    label::label(i_widget& aParent, const i_texture& aTexture, label_type aType, alignment aAlignment, label_placement aPlacement) :
        widget{ aParent }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ iLayout, aTexture },
        iCentreSpacer{ nullptr }
    {
        layout().set_alignment(aAlignment);
        text_widget().set_alignment(aAlignment);
        init();
    }

    label::label(i_widget& aParent, const i_image& aImage, label_type aType, alignment aAlignment, label_placement aPlacement) :
        widget{ aParent }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ iLayout, aImage },
        iCentreSpacer{ nullptr }
    {
        layout().set_alignment(aAlignment);
        text_widget().set_alignment(aAlignment);
        init();
    }

    label::label(i_layout& aLayout, label_type aType, alignment aAlignment, label_placement aPlacement) :
        widget{ aLayout }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ iLayout, neogfx::texture{} },
        iCentreSpacer{ nullptr }
    {
        layout().set_alignment(aAlignment);
        text_widget().set_alignment(aAlignment);
        init();
    }

    label::label(i_layout& aLayout, const std::string& aText, label_type aType, alignment aAlignment, label_placement aPlacement) :
        widget{ aLayout }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ iLayout, aText, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ iLayout, neogfx::texture{} },
        iCentreSpacer{ nullptr }
    {
        layout().set_alignment(aAlignment);
        text_widget().set_alignment(aAlignment);
        init();
    }

    label::label(i_layout& aLayout, const i_texture& aTexture, label_type aType, alignment aAlignment, label_placement aPlacement) :
        widget{ aLayout }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ iLayout, aTexture },
        iCentreSpacer{ nullptr }
    {
        layout().set_alignment(aAlignment);
        text_widget().set_alignment(aAlignment);
        init();
    }

    label::label(i_layout& aLayout, const i_image& aImage, label_type aType, alignment aAlignment, label_placement aPlacement) :
        widget{ aLayout }, 
        iAlignment{ aAlignment }, 
        iPlacement{ aPlacement }, 
        iLayout{ *this }, 
        iText{ iLayout, std::string{}, aType, text_widget_flags::TakesSpaceWhenEmpty },
        iImage{ iLayout, aImage },
        iCentreSpacer{ nullptr }
    {
        layout().set_alignment(aAlignment);
        text_widget().set_alignment(aAlignment);
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

    neogfx::size_policy label::size_policy() const
    {
        if (widget::has_size_policy())
            return widget::size_policy();
        return size_constraint::Minimum;
    }

    void label::set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout)
    {
        widget::set_size_policy(aSizePolicy, aUpdateLayout);
        text_widget().set_size_policy(aSizePolicy, aUpdateLayout);
        image_widget().set_size_policy(aSizePolicy, aUpdateLayout);
    }

    const std::string& label::text() const
    {
        return text_widget().text();
    }

    void label::set_text(const std::string& aText)
    {
        text_widget().set_text(aText);
    }

    const texture& label::image() const
    {
        return image_widget().image();
    }

    void label::set_image(const std::string& aImageUri)
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

    const i_spacer& label::centre_spacer() const
    {
        if (iCentreSpacer != nullptr)
            return *iCentreSpacer;
        throw no_centre_spacer();
    }

    i_spacer& label::centre_spacer()
    {
        if (iCentreSpacer != nullptr)
            return *iCentreSpacer;
        throw no_centre_spacer();
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
        image_widget().set_dpi_auto_scale(true);
        layout().set_margins(neogfx::margins{});
        text_widget().set_margins(neogfx::margins{});
        image_widget().set_margins(neogfx::margins{});
        set_ignore_mouse_events(true);
        text_widget().set_ignore_mouse_events(true);
        image_widget().set_ignore_mouse_events(true);
        handle_placement_change();
        iSink += layout().AlignmentChanged([this]() { handle_placement_change(); });
        iSink += text_widget().VisibilityChanged([this](){ handle_placement_change(); });
        iSink += text_widget().TextChanged([this]() { handle_placement_change(); });
        iSink += image_widget().VisibilityChanged([this]() { handle_placement_change(); });
        iSink += image_widget().ImageChanged([this]() { handle_placement_change(); });
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
        iCentreSpacer = nullptr;

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
            case neogfx::alignment::Centre:
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
            case neogfx::alignment::VCentre:
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
            case neogfx::alignment::Centre:
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
            case neogfx::alignment::VCentre:
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
                iCentreSpacer = &layout().add_spacer_at_position(0, 1);
                layout().add_item_at_position(0, 2, image_widget());
                layout().add_spacer_at_position(0, 3);
                break;
            case neogfx::alignment::Centre:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(0, 1, text_widget());
                iCentreSpacer = &layout().add_spacer_at_position(0, 2);
                layout().add_item_at_position(0, 3, image_widget());
                layout().add_spacer_at_position(0, 3);
                break;
            case neogfx::alignment::Right:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(0, 1, text_widget());
                iCentreSpacer = &layout().add_spacer_at_position(0, 2);
                layout().add_item_at_position(0, 3, image_widget());
                break;
            }
            break;
        case label_placement::TextSpacerImageVertical:
            switch (layout().alignment() & neogfx::alignment::Vertical)
            {
            case neogfx::alignment::Top:
                layout().add_item_at_position(0, 0, text_widget());
                iCentreSpacer = &layout().add_spacer_at_position(1, 0);
                layout().add_item_at_position(2, 0, image_widget());
                layout().add_spacer_at_position(3, 0);
                break;
            case neogfx::alignment::VCentre:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(1, 0, text_widget());
                iCentreSpacer = &layout().add_spacer_at_position(2, 0);
                layout().add_item_at_position(3, 0, image_widget());
                layout().add_spacer_at_position(4, 0);
                break;
            case neogfx::alignment::Bottom:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(1, 0, text_widget());
                iCentreSpacer = &layout().add_spacer_at_position(2, 0);
                layout().add_item_at_position(3, 0, image_widget());
                break;
            }
            break;
        case label_placement::ImageSpacerTextHorizontal:
            switch (layout().alignment() & neogfx::alignment::Horizontal)
            {
            case neogfx::alignment::Left:
                layout().add_item_at_position(0, 0, image_widget());
                iCentreSpacer = &layout().add_spacer_at_position(0, 1);
                layout().add_item_at_position(0, 2, text_widget());
                layout().add_spacer_at_position(0, 3);
                break;
            case neogfx::alignment::Centre:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(0, 1, image_widget());
                iCentreSpacer = &layout().add_spacer_at_position(0, 2);
                layout().add_item_at_position(0, 3, text_widget());
                layout().add_spacer_at_position(0, 4);
                break;
            case neogfx::alignment::Right:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(0, 1, image_widget());
                iCentreSpacer = &layout().add_spacer_at_position(0, 2);
                layout().add_item_at_position(0, 3, text_widget());
                break;
            }
            break;
        case label_placement::ImageSpacerTextVertical:
            switch (layout().alignment() & neogfx::alignment::Vertical)
            {
            case neogfx::alignment::Top:
                layout().add_item_at_position(0, 0, image_widget());
                iCentreSpacer = &layout().add_spacer_at_position(1, 0);
                layout().add_item_at_position(2, 0, text_widget());
                layout().add_spacer_at_position(3, 0);
                break;
            case neogfx::alignment::VCentre:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(1, 0, image_widget());
                iCentreSpacer = &layout().add_spacer_at_position(2, 0);
                layout().add_item_at_position(3, 0, text_widget());
                layout().add_spacer_at_position(4, 0);
                break;
            case neogfx::alignment::Bottom:
                layout().add_spacer_at_position(0, 0);
                layout().add_item_at_position(1, 0, image_widget());
                iCentreSpacer = &layout().add_spacer_at_position(2, 0);
                layout().add_item_at_position(3, 0, text_widget());
                break;
            }
            break;
        }
    }
}