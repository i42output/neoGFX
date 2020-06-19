// group_box.cpp
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
#include <neogfx/gui/widget/group_box.hpp>

namespace neogfx
{
    group_box::group_box(const std::string& aText) : 
        widget(), iLayout{ *this }, iTitle{ std::make_unique<neogfx::label>(iLayout, aText) }
    {
        init();
    }

    group_box::group_box(i_widget& aParent, const std::string& aText) :
        widget(aParent), iLayout{ *this }, iTitle{ std::make_unique<neogfx::label>(iLayout, aText) }
    {
        init();
    }

    group_box::group_box(i_layout& aLayout, const std::string& aText) :
        widget(aLayout), iLayout{ *this }, iTitle{ std::make_unique<neogfx::label>(iLayout, aText) }
    {
        init();
    }

    const std::string& group_box::text() const
    {
        return label().text();
    }

    void group_box::set_text(const std::string& aText)
    {
        label().set_text(aText);
    }

    bool group_box::is_checkable() const
    {
        return std::holds_alternative<check_box_ptr>(iTitle);
    }

    void group_box::set_checkable(bool aCheckable)
    {
        if (is_checkable() != aCheckable)
        {
            auto const text = label().text();
            iTitle = neolib::none;
            if (aCheckable)
            {
                iTitle = std::make_unique<neogfx::check_box>(text);
                iLayout.add_at(0, *static_variant_cast<check_box_ptr&>(iTitle));
            }
            else
            {
                iTitle = std::make_unique<neogfx::label>(text);
                iLayout.add_at(0, *static_variant_cast<label_ptr&>(iTitle));
            }
        }
    }

    const label& group_box::label() const
    {
        if (std::holds_alternative<label_ptr>(iTitle))
            return *static_variant_cast<const label_ptr&>(iTitle);
        else
            return static_variant_cast<const check_box_ptr&>(iTitle)->label();
    }

    label& group_box::label()
    {
        if (std::holds_alternative<label_ptr>(iTitle))
            return *static_variant_cast<label_ptr&>(iTitle);
        else
            return static_variant_cast<check_box_ptr&>(iTitle)->label();
    }

    bool group_box::has_check_box() const
    {
        return std::holds_alternative<check_box_ptr>(iTitle);
    }

    const check_box& group_box::check_box() const
    {
        if (has_check_box())
            return *static_variant_cast<const check_box_ptr&>(iTitle);
        throw not_checkable();
    }

    check_box& group_box::check_box()
    {
        if (has_check_box())
            return *static_variant_cast<check_box_ptr&>(iTitle);
        throw not_checkable();
    }

    void group_box::set_item_layout(i_layout& aItemLayout)
    {
        set_item_layout(std::shared_ptr<i_layout>{std::shared_ptr<i_layout>{}, &aItemLayout});
    }

    void group_box::set_item_layout(std::shared_ptr<i_layout> aItemLayout)
    {
        if (iItemLayout != nullptr)
            iLayout.remove(*iItemLayout);
        iItemLayout = aItemLayout;
        if (!aItemLayout->has_parent_layout() || &aItemLayout->parent_layout() != &iLayout)
            iLayout.add(iItemLayout);
    }

    const i_layout& group_box::item_layout() const
    {
        return *iItemLayout;
    }

    i_layout& group_box::item_layout()
    {
        return *iItemLayout;
    }

    neogfx::size_policy group_box::size_policy() const
    {
        if (widget::has_size_policy())
            return widget::size_policy();
        else
            return size_constraint::Minimum;
    }

    void group_box::paint(i_graphics_context& aGc) const
    {
        widget::paint(aGc);
        rect lr{ item_layout().position(), item_layout().extents() };
        lr.inflate(size{ 5.0 });
        aGc.draw_rounded_rect(lr, 4.0, pen{ border_color() }, brush{ fill_color() });
    }

    color group_box::background_color() const
    {
        if (container_background_color().light())
            return parent().background_color().darker(24);
        else
            return parent().background_color().lighter(24);
    }

    bool group_box::has_border_color() const
    {
        return BorderColor != std::nullopt;
    }

    color group_box::border_color() const
    {
        if (has_border_color())
            return *BorderColor;
        else if (container_background_color().light())
            return background_color().darker(24);
        else
            return background_color().lighter(24);
    }

    void group_box::set_border_color(const optional_color& aBorderColor)
    {
        BorderColor = aBorderColor;
        update();
    }

    bool group_box::has_fill_color() const
    {
        return FillColor != std::nullopt;
    }

    color group_box::fill_color() const
    {
        return (has_fill_color() ? *FillColor : container_background_color().light() ? border_color().lighter(24) : border_color().darker(24)).with_alpha(fill_opacity());
    }

    void group_box::set_fill_color(const optional_color& aFillColor)
    {
        FillColor = aFillColor;
    }

    double group_box::fill_opacity() const
    {
        return FillOpacity;
    }

    void group_box::set_fill_opacity(double aFillOpacity)
    {
        FillOpacity = aFillOpacity;
    }

    class group_box_item_layout : public vertical_layout
    {
    public:
        size minimum_size(const optional_size& aAvailableSpace) const override
        {
            auto result = vertical_layout::minimum_size(aAvailableSpace);
            if (result == size{})
                result = size{ 10.0, 10.0 };
            return result;
        }
    };

    void group_box::init()
    {
        set_padding(neogfx::padding{});
        iLayout.set_padding(neogfx::padding{ 5.0 });
        iLayout.set_spacing(neogfx::size{ 5.0 });
        set_item_layout(std::make_shared<group_box_item_layout>());
    }
}