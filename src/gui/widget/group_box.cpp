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
    namespace
    {
        size max_border_radius(group_box const& aOwner, size const& aDefault)
        {
            auto const radii = aOwner.effective_border_radius();
            if (radii == std::nullopt)
                return aDefault;
            scoped_units su{ aOwner, units::Pixels };
            size result;
            for (auto const& corner : radii.value())
            {
                result.cx = std::max(result.cx, corner[0].value());
                result.cy = std::max(result.cy, corner[1].value());
            }
            return result;
        }
    }

    class group_box_title_layout : public horizontal_layout
    {
    public:
        group_box_title_layout(group_box& aOwner) :
            horizontal_layout{ aOwner.layout() },
            iOwner{ aOwner }
        {
            set_size_policy(size_constraint::Expanding);
            set_padding({});
            set_spacing({});
        }
    public:
        neogfx::margin margin() const override
        {
            if (has_margin())
                return horizontal_layout::margin();
            switch (iOwner.border_style())
            {
            case group_box_border_style::None:
            default:
                return horizontal_layout::margin();
            case group_box_border_style::Line:
                {
                    auto const titleRect = iOwner.to_client_coordinates(iOwner.has_check_box() ?
                        iOwner.check_box().non_client_rect() : iOwner.label().non_client_rect());
                    return horizontal_layout::margin().with_bottom(-std::ceil(titleRect.extents().cy / 2.0));
                }
            }
        }
        neogfx::padding padding() const override
        {
            if (has_padding())
                return horizontal_layout::padding();
            switch (iOwner.border_style())
            {
            case group_box_border_style::None:
            default:
                return dpi_scale(neogfx::padding{ group_box::DEFAULT_PADDING, group_box::DEFAULT_PADDING, group_box::DEFAULT_PADDING, 0.0 });
            case group_box_border_style::Line:
                {
                    auto const defaultPadding = dpi_scale(group_box::DEFAULT_PADDING);
                    auto const radii = max_border_radius(iOwner, size{ defaultPadding * 2.0, defaultPadding });
                    return neogfx::padding{ radii.cx, 0.0, radii.cx, 0.0 };
                }
            }
        }
        size spacing() const override
        {
            if (has_spacing())
                return horizontal_layout::spacing();
            return dpi_scale(size{ group_box::DEFAULT_PADDING });
        }
    private:
        group_box& iOwner;
    };

    class group_box_item_layout : public vertical_layout
    {
    public:
        group_box_item_layout(group_box& aOwner) :
            iOwner{ aOwner }
        {
            set_padding({});
            set_spacing({});
        }
    public:
        size minimum_size(optional_size const& aAvailableSpace) const override
        {
            auto result = vertical_layout::minimum_size(aAvailableSpace);
            if (result == size{})
                result = dpi_scale(size{ 10.0, 10.0 });
            return result;
        }
        neogfx::padding padding() const override
        {
            if (has_padding())
                return vertical_layout::padding();
            auto const defaultPadding = dpi_scale(group_box::DEFAULT_PADDING);
            auto const radii = max_border_radius(iOwner, size{ defaultPadding, defaultPadding });
            auto const cx = std::max(radii.cx, defaultPadding);
            auto const cy = std::max(radii.cy, defaultPadding);
            return neogfx::padding{ cx, cy, cx, cy };
        }
        size spacing() const override
        {
            if (has_spacing())
                return vertical_layout::spacing();
            return dpi_scale(size{ group_box::DEFAULT_SPACING });
        }
    private:
        group_box& iOwner;
    };

    group_box::box::box(group_box& aOwner) :
        widget{ aOwner.layout() }, iOwner{ aOwner }
    {
        set_padding({});
    }

    void group_box::box::paint(i_graphics_context& aGc) const
    {
        widget::paint(aGc);

        if (iOwner.border_style() == group_box_border_style::None)
            aGc.draw_rounded_rect(client_rect(), 4.0, pen{ iOwner.border_color() }, brush{ iOwner.fill_color() });
    }

    color group_box::box::palette_color(color_role aColorRole) const
    {
        return iOwner.palette_color(aColorRole);
    }

    group_box::group_box(std::string const& aText) : 
        widget(), iLayout{ *this }, iTitleLayout{ make_ref<group_box_title_layout>(*this) }, iTitle{ std::make_unique<neogfx::label>(title_layout(), aText)}, iBox{*this}
    {
        init();
    }

    group_box::group_box(i_widget& aParent, std::string const& aText) :
        widget(aParent), iLayout{ *this }, iTitleLayout{ make_ref<group_box_title_layout>(*this) }, iTitle{ std::make_unique<neogfx::label>(title_layout(), aText) }, iBox{ *this }
    {
        init();
    }

    group_box::group_box(i_layout& aLayout, std::string const& aText) :
        widget(aLayout), iLayout{ *this }, iTitleLayout{ make_ref<group_box_title_layout>(*this) }, iTitle{ std::make_unique<neogfx::label>(title_layout(), aText) }, iBox{ *this }
    {
        init();
    }

    void group_box::set_title_layout(i_layout& aTitleLayout)
    {
        set_title_layout(ref_ptr<i_layout>{ref_ptr<i_layout>{}, & aTitleLayout});
    }

    void group_box::set_title_layout(i_ref_ptr<i_layout> const& aTitleLayout)
    {
        auto oldLayout = iTitleLayout;
        iTitleLayout = aTitleLayout;
        if (oldLayout)
            oldLayout->move_all_to(title_layout());
        iLayout.replace_item_at(0u, iTitleLayout);
    }

    const i_layout& group_box::title_layout() const
    {
        return *iTitleLayout;
    }

    i_layout& group_box::title_layout()
    {
        return *iTitleLayout;
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
                title_layout().add_at(0, *static_variant_cast<check_box_ptr&>(iTitle));
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
                title_layout().add_at(0, *static_variant_cast<label_ptr&>(iTitle));
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

    const i_widget& group_box::item_box() const
    {
        return iBox;
    }

    i_widget& group_box::item_box()
    {
        return iBox;
    }

    void group_box::set_item_layout(i_layout& aItemLayout)
    {
        set_item_layout(ref_ptr<i_layout>{ref_ptr<i_layout>{}, &aItemLayout});
    }

    void group_box::set_item_layout(i_ref_ptr<i_layout> const& aItemLayout)
    {
        iItemLayout = aItemLayout;
        iBox.set_layout(iItemLayout);
    }

    const i_layout& group_box::item_layout() const
    {
        return *iItemLayout;
    }

    i_layout& group_box::item_layout()
    {
        return *iItemLayout;
    }

    void group_box::paint(i_graphics_context& aGc) const
    {
        widget::paint(aGc);

        if (border_style() != group_box_border_style::Line)
            return;

        auto const cr = client_rect();
        auto const titleRect = to_client_coordinates(has_check_box() ?
            check_box().non_client_rect() : label().non_client_rect());
        auto const thickness = dpi_scale(border_thickness());
        auto const gap = std::ceil(from_mm(DEFAULT_LABEL_GAP_MM));

        // the border passes through the middle of the title...
        rect const borderRect{
            point{ cr.left() + thickness / 2.0, titleRect.top() + titleRect.cy / 2.0 }.ceil(),
            point{ cr.right() - thickness / 2.0, cr.bottom() - thickness / 2.0 }.ceil() };
        // ...but is masked out where the title (plus its gap) sits
        rect const titleMask{
            point{ titleRect.left() - gap, titleRect.top() },
            point{ titleRect.right() + gap, titleRect.bottom() } };

        aGc.clear_stencil_buffer();
        aGc.enable_stencil_test();
        aGc.enable_stencil_update(STENCIL_BORDER);
        aGc.fill_rect(cr, color::White);
        aGc.disable_stencil_update();
        aGc.enable_stencil_update(STENCIL_LABEL);
        aGc.fill_rect(titleMask, color::White);
        aGc.disable_stencil_update();
        // the stencil test uses the ref most recently set
        aGc.enable_stencil_update(STENCIL_BORDER);
        aGc.disable_stencil_update();

        pen const borderPen{ border_color(), thickness };

        auto const borderRadii = effective_border_radius();

        if (borderRadii.has_value())
        {
            auto to_vec2 = [&](std::array<length, 2u> const& l)
            {
                basic_length<vec2> lx{ vec2{ l[0].unconverted_value(), 0.0 }, l[0].units() };
                basic_length<vec2> ly{ vec2{ 0.0, l[1].unconverted_value(), }, l[1].units() };
                return vec2{ lx.value().x, ly.value().y };
            };
            aGc.draw_ellipse_rect(
                borderRect,
                vec4{ to_vec2(borderRadii.value()[0]).x, to_vec2(borderRadii.value()[1]).x, to_vec2(borderRadii.value()[2]).x, to_vec2(borderRadii.value()[3]).x },
                vec4{ to_vec2(borderRadii.value()[0]).y, to_vec2(borderRadii.value()[1]).y, to_vec2(borderRadii.value()[2]).y, to_vec2(borderRadii.value()[3]).y },
                borderPen);
        }
        else
            aGc.draw_rect(borderRect, borderPen);

        aGc.disable_stencil_test();
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

    group_box_border_style group_box::border_style() const
    {
        return iBorderStyle;
    }

    void group_box::set_border_style(group_box_border_style aBorderStyle)
    {
        if (iBorderStyle != aBorderStyle)
        {
            iBorderStyle = aBorderStyle;
            update_layout();
            update();
        }
    }

    dimension group_box::border_thickness() const
    {
        return iBorderThickness;
    }

    void group_box::set_border_thickness(dimension aBorderThickness)
    {
        if (iBorderThickness != aBorderThickness)
        {
            iBorderThickness = aBorderThickness;
            update();
        }
    }

    std::optional<border_radii> const& group_box::border_radius() const
    {
        return iBorderRadii;
    }

    void group_box::set_border_radius(std::optional<border_radii> const& aBorderRadii)
    {
        if (iBorderRadii != aBorderRadii)
        {
            iBorderRadii = aBorderRadii;
            update_layout();
            update();
        }
    }

    void group_box::set_border_radius(length const& aBorderRadius)
    {
        set_border_radius(border_radii{ { { aBorderRadius, aBorderRadius }, { aBorderRadius, aBorderRadius }, { aBorderRadius, aBorderRadius }, { aBorderRadius, aBorderRadius } } });
    }

    std::optional<border_radii> group_box::effective_border_radius() const
    {
        return style_sheet_value("." + class_name(), "border-radius", border_radius());
    }

    bool group_box::has_border_color() const
    {
        return BorderColor != std::nullopt;
    }

    color group_box::border_color() const
    {
        if (has_border_color())
            return *BorderColor;
        if (border_style() == group_box_border_style::Line)
            return palette_color(color_role::Text);
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

    void group_box::init()
    {
        set_padding(neogfx::padding{});
        iLayout.set_padding(neogfx::padding{});
        set_item_layout(make_ref<group_box_item_layout>(*this));
    }

    void group_box::update_widgets()
    {
        iBox.enable(!is_checkable() || check_box().is_checked());
    }
}