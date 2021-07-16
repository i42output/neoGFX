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
    group_box::item_container::item_container(group_box& aParent) :
        widget{ aParent.layout() }, iParent{ aParent }
    {
        set_padding(dpi_scale(size{ DEFAULT_PADDING }));
    }

    void group_box::item_container::paint(i_graphics_context& aGc) const
    {
        widget::paint(aGc);
        aGc.draw_rounded_rect(client_rect(), 4.0, pen{ iParent.border_color() }, brush{ iParent.fill_color() });
    }

    color group_box::item_container::palette_color(color_role aColorRole) const
    {
        return iParent.palette_color(aColorRole);
    }

    group_box::group_box(std::string const& aText) : 
        widget(), iLayout{ *this }, iTitleLayout{ iLayout }, iTitle{ std::make_unique<neogfx::label>(iTitleLayout, aText) }, iItemContainer{ *this }
    {
        init();
    }

    group_box::group_box(i_widget& aParent, std::string const& aText) :
        widget(aParent), iLayout{ *this }, iTitleLayout{ iLayout }, iTitle{ std::make_unique<neogfx::label>(iTitleLayout, aText) }, iItemContainer{ *this }
    {
        init();
    }

    group_box::group_box(i_layout& aLayout, std::string const& aText) :
        widget(aLayout), iLayout{ *this }, iTitleLayout{ iLayout }, iTitle{ std::make_unique<neogfx::label>(iTitleLayout, aText) }, iItemContainer{ *this }
    {
        init();
    }

    i_string const& group_box::text() const
    {
        return label().text();
    }

    void group_box::set_text(i_string const& aText)
    {
        label().set_text(aText);
    }

    bool group_box::is_checkable() const
    {
        return std::holds_alternative<check_box_ptr>(iTitle);
    }

    void group_box::set_checkable(bool aCheckable, bool aUpdateItemsEnabledState)
    {
        if (is_checkable() != aCheckable)
        {
            string const text = label().text();
            iTitle = neolib::none;
            if (aCheckable)
            {
                iTitle = std::make_unique<neogfx::check_box>(text);
                iTitleLayout.add_at(0, *static_variant_cast<check_box_ptr&>(iTitle));
                if (aUpdateItemsEnabledState)
                {
                    iSink += std::get<check_box_ptr>(iTitle)->Checked([&]() { update_widgets(); });
                    iSink += std::get<check_box_ptr>(iTitle)->Unchecked([&]() { update_widgets(); });
                    update_widgets();
                }
            }
            else
            {
                iTitle = std::make_unique<neogfx::label>(text);
                iTitleLayout.add_at(0, *static_variant_cast<label_ptr&>(iTitle));
                if (aUpdateItemsEnabledState)
                    update_widgets();
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
        set_item_layout(ref_ptr<i_layout>{ref_ptr<i_layout>{}, &aItemLayout});
    }

    void group_box::set_item_layout(i_ref_ptr<i_layout> const& aItemLayout)
    {
        iItemLayout = aItemLayout;
        iItemContainer.set_layout(iItemLayout);
        if (!iItemLayout->has_padding())
            iItemLayout->set_padding(neogfx::padding{});
        if (!iItemLayout->has_spacing())
            iItemLayout->set_spacing(dpi_scale(size{ DEFAULT_PADDING }));
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
        if (has_size_policy())
            return widget::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return size_constraint::Minimum;
    }

    color group_box::palette_color(color_role aColorRole) const
    {
        if (aColorRole == color_role::Background)
        {
            if (container_background_color().light())
                return parent().background_color().darker(0x0C);
            else
                return parent().background_color().lighter(0x0C);
        }
        return widget::palette_color(aColorRole);
    }

    bool group_box::has_border_color() const
    {
        return BorderColor != std::nullopt;
    }

    color group_box::border_color() const
    {
        if (has_border_color())
            return *BorderColor;
        return background_color().shaded(0x0A);
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
        return has_fill_color() ? *FillColor : background_color().with_alpha(fill_opacity());
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
        size minimum_size(optional_size const& aAvailableSpace) const override
        {
            auto result = vertical_layout::minimum_size(aAvailableSpace);
            if (result == size{})
                result = dpi_scale(size{ 10.0, 10.0 });
            return result;
        }
    };

    void group_box::init()
    {
        set_padding(neogfx::padding{});
        iLayout.set_padding(neogfx::padding{});
        iTitleLayout.set_padding(dpi_scale(neogfx::padding{ DEFAULT_PADDING, DEFAULT_PADDING, DEFAULT_PADDING, 0.0 }));
        iTitleLayout.set_spacing(dpi_scale(size{ DEFAULT_PADDING }));
        set_item_layout(make_ref<group_box_item_layout>());
    }

    void group_box::update_widgets()
    {
        iItemContainer.enable(!is_checkable() || check_box().is_checked());
    }
}