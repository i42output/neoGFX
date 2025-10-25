// normal_title_bar.cpp
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

#include <neogfx/app/i_app.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gui/widget/normal_title_bar.hpp>

namespace neogfx
{
    normal_title_bar::normal_title_bar(i_standard_layout_container& aContainer, std::string const& aTitle) :
        widget{ aContainer.title_bar_layout() },
        iOuterLayout{ *this },
        iInnerLayout{ iOuterLayout },
        iButtonLayout{ iOuterLayout },
        iIcon{ iInnerLayout, service<i_app>().default_window_icon() },
        iTitle{ iInnerLayout, aTitle, text_widget_type::SingleLine, text_widget_flags::CutOff },
        iButtonSpacer{ iButtonLayout },
        iMinimizeButton{ iButtonLayout, push_button_style::TitleBar },
        iMaximizeButton{ iButtonLayout, push_button_style::TitleBar },
        iRestoreButton{ iButtonLayout, push_button_style::TitleBar },
        iCloseButton{ iButtonLayout, push_button_style::TitleBar }
    {
        init();
    }

    normal_title_bar::normal_title_bar(i_standard_layout_container& aContainer, const i_texture& aIcon, std::string const& aTitle) :
        widget{ aContainer.title_bar_layout() },
        iOuterLayout{ *this },
        iInnerLayout{ iOuterLayout },
        iButtonLayout{ iOuterLayout },
        iIcon{ iInnerLayout, aIcon },
        iTitle{ iInnerLayout, aTitle },
        iButtonSpacer{ iButtonLayout },
        iMinimizeButton{ iButtonLayout, push_button_style::TitleBar },
        iMaximizeButton{ iButtonLayout, push_button_style::TitleBar },
        iRestoreButton{ iButtonLayout, push_button_style::TitleBar },
        iCloseButton{ iButtonLayout, push_button_style::TitleBar }
    {
        init();
    }

    normal_title_bar::normal_title_bar(i_standard_layout_container& aContainer, const i_image& aIcon, std::string const& aTitle) :
        widget{ aContainer.title_bar_layout() },
        iOuterLayout{ *this },
        iInnerLayout{ iOuterLayout },
        iButtonLayout{ iOuterLayout },
        iIcon{ iInnerLayout, aIcon },
        iTitle{ iInnerLayout, aTitle },
        iButtonSpacer{ iButtonLayout },
        iMinimizeButton{ iButtonLayout, push_button_style::TitleBar },
        iMaximizeButton{ iButtonLayout, push_button_style::TitleBar },
        iRestoreButton{ iButtonLayout, push_button_style::TitleBar },
        iCloseButton{ iButtonLayout, push_button_style::TitleBar }
    {
        init();
    }

    void normal_title_bar::set_icon(i_texture const& aIcon)
    {
        icon_widget().set_image(aIcon);
    }

    i_string const& normal_title_bar::title() const
    {
        return iTitle.text();
    }

    void normal_title_bar::set_title(i_string const& aTitle)
    {
        iTitle.set_text(aTitle);
    }

    const image_widget& normal_title_bar::icon_widget() const
    {
        return iIcon;
    }

    image_widget& normal_title_bar::icon_widget()
    {
        return iIcon;
    }

    const text_widget& normal_title_bar::title_widget() const
    {
        return iTitle;
    }

    text_widget& normal_title_bar::title_widget()
    {
        return iTitle;
    }

    neogfx::size_policy normal_title_bar::size_policy() const
    {
        if (has_size_policy())
            return widget::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        else
            return neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum };
    }

    widget_type normal_title_bar::widget_type() const
    {
        return neogfx::widget_type::NonClient;
    }

    widget_part normal_title_bar::part(const point&) const
    {
        return widget_part{ root().as_widget(), widget_part::TitleBar };
    }

    color normal_title_bar::palette_color(color_role aColorRole) const
    {
        std::optional<color> result;
        if (has_palette_color(aColorRole) || !root().is_active())
            result = base_type::palette_color(aColorRole);
        else if (aColorRole == color_role::Background)
            result = palette_color(color_role::Base);
        if (aColorRole == color_role::Background)
        {
            if (root().has_native_window() && root().native_window().alert_active())
                result = mix(result.value(), color::Orange, root().native_window().alert_easing());
        }
        return result.value_or(base_type::palette_color(aColorRole));
    }

    void normal_title_bar::init()
    {
        set_background_opacity(1.0);

        set_padding(neogfx::padding{});
        iOuterLayout.set_padding(neogfx::padding{});
        iOuterLayout.set_spacing(size{});
        iInnerLayout.set_padding(neogfx::padding{ 8.0_dip, 4.0_dip, 8.0_dip, 4.0_dip });
        iInnerLayout.set_spacing(size{ 8.0_dip });
        iButtonLayout.set_padding(neogfx::padding{});
        iButtonLayout.set_spacing(size{});
        iButtonLayout.set_alignment(alignment::Right | alignment::Top);
        iButtonLayout.set_size_policy(size_constraint::Minimum, size_constraint::Expanding);
        icon_widget().set_ignore_mouse_events(false);
        title_widget().set_size_policy(size_constraint::Expanding, size_constraint::Minimum);
        title_widget().set_alignment(alignment::Left | alignment::VCenter);
        title_widget().set_font_role(neogfx::font_role::Caption);

        iSink += service<i_app>().current_style_changed([this](style_aspect aAspect) 
        { 
            if ((aAspect & style_aspect::Color) == style_aspect::Color) 
                update_textures(); 
        });
        
        auto update_widgets = [this]()
        {
            scoped_units su{ *this, units::Pixels };

            auto const idealIconSize = size{ title_widget().font().height(), title_widget().font().height() }.ceil();
            if (icon_widget().image().is_empty())
                icon_widget().set_fixed_size(idealIconSize);
            else
                icon_widget().set_fixed_size(idealIconSize.min(icon_widget().image().extents()));
            auto const idealButtonSize = size{ 12.0_mm, 9.0_mm }.ceil();
            iMinimizeButton.set_fixed_size(idealButtonSize);
            iMaximizeButton.set_fixed_size(idealButtonSize);
            iRestoreButton.set_fixed_size(idealButtonSize);
            iCloseButton.set_fixed_size(idealButtonSize);
            icon_widget().set_size_policy(size_constraint::Fixed);
            iMinimizeButton.set_size_policy(size_constraint::Fixed);
            iMaximizeButton.set_size_policy(size_constraint::Fixed);
            iRestoreButton.set_size_policy(size_constraint::Fixed);
            iCloseButton.set_size_policy(size_constraint::Fixed);
            bool isEnabled = root().window_enabled();
            bool isActive = root().is_effectively_active();
            bool isIconic = root().is_iconic();
            bool isMaximized = root().is_maximized();
            bool isRestored = root().is_restored();
            icon_widget().enable(isActive);
            title_widget().enable(isActive);
            iMinimizeButton.enable(!isIconic && isEnabled);
            iMaximizeButton.enable(!isMaximized && isEnabled);
            iRestoreButton.enable(!isRestored && isEnabled);
            iCloseButton.enable(root().can_close() && isEnabled);
            iMinimizeButton.show(!isIconic && (root().style() & window_style::MinimizeBox) == window_style::MinimizeBox);
            iMaximizeButton.show(!isMaximized && (root().style() & window_style::MaximizeBox) == window_style::MaximizeBox);
            iRestoreButton.show(!isRestored && (root().style() & (window_style::MinimizeBox | window_style::MaximizeBox)) != window_style::Invalid);
            iCloseButton.show((root().style() & window_style::Close) != window_style::Invalid);
            update_textures();
            update_layout();
            update(true);
        };

        iSink += service<i_app>().execution_started([this, update_widgets]()
        {
            update_widgets();
        });

        iSink += root().window_event([this, update_widgets](neogfx::window_event& e)
        {
            switch (e.type())
            {
            case window_event_type::Enabled:
            case window_event_type::Disabled:
            case window_event_type::FocusGained:
            case window_event_type::FocusLost:
            case window_event_type::Iconized:
            case window_event_type::Maximized:
            case window_event_type::Restored:
                update_widgets();
                break;
            default:
                break;
            }
        });

        iSink += root().activated([this, update_widgets]() { update_widgets(); });
        iSink += root().deactivated([this, update_widgets]() { update_widgets(); });
        iSink += service<i_surface_manager>().dpi_changed([this, update_widgets](i_surface&) { update_widgets(); });
        
        update_textures();
        update_widgets();
    }

    void normal_title_bar::update_textures()
    {
        auto ink = service<i_app>().current_style().palette().color(color_role::Text);
        auto paper = background_color();
        const char* sMinimizeTexturePattern
        {
            "[10,10]"
            "{0,paper}"
            "{1,ink}"
            "{2,ink_with_alpha}"

            "0000000000"
            "0000000000"
            "0000000000"
            "0000000000"
            "1111111111"
            "1111111111"
            "0000000000"
            "0000000000"
            "0000000000"
            "0000000000"
        };
        const char* sMaximizeTexturePattern
        {
            "[10,10]"
            "{0,paper}"
            "{1,ink}"
            "{2,ink_with_alpha}"

            "1111111111"
            "1111111111"
            "1000000001"
            "1000000001"
            "1000000001"
            "1000000001"
            "1000000001"
            "1000000001"
            "1000000001"
            "1111111111"
        };
        const char* sRestoreTexturePattern
        {
            "[10,10]"
            "{0,paper}"
            "{1,ink}"
            "{2,ink_with_alpha}"

            "0001111111"
            "0001111111"
            "0001000001"
            "1111111001"
            "1111111001"
            "1000001001"
            "1000001111"
            "1000001000"
            "1000001000"
            "1111111000"
        };
        const char* sCloseTexturePattern
        {
            "[10,10]"
            "{0,paper}"
            "{1,ink}"
            "{2,ink_with_alpha}"

            "1200000021"
            "2120000212"
            "0212002120"
            "0021221200"
            "0002112000"
            "0002112000"
            "0021221200"
            "0212002120"
            "2120000212"
            "1200000021"
        };
        const char* sMinimizeHighDpiTexturePattern
        {
            "[20,20]"
            "{0,paper}"
            "{1,ink}"
            "{2,ink_with_alpha}"

            "00000000000000000000"
            "00000000000000000000"
            "00000000000000000000"
            "00000000000000000000"
            "00000000000000000000"
            "00000000000000000000"
            "00000000000000000000"
            "00000000000000000000"
            "11111111111111111111"
            "11111111111111111111"
            "11111111111111111111"
            "11111111111111111111"
            "00000000000000000000"
            "00000000000000000000"
            "00000000000000000000"
            "00000000000000000000"
            "00000000000000000000"
            "00000000000000000000"
            "00000000000000000000"
            "00000000000000000000"
        };
        const char* sMaximizeHighDpiTexturePattern
        {
            "[20,20]"
            "{0,paper}"
            "{1,ink}"
            "{2,ink_with_alpha}"

            "11111111111111111111"
            "11111111111111111111"
            "11111111111111111111"
            "11111111111111111111"
            "11000000000000000011"
            "11000000000000000011"
            "11000000000000000011"
            "11000000000000000011"
            "11000000000000000011"
            "11000000000000000011"
            "11000000000000000011"
            "11000000000000000011"
            "11000000000000000011"
            "11000000000000000011"
            "11000000000000000011"
            "11000000000000000011"
            "11000000000000000011"
            "11000000000000000011"
            "11111111111111111111"
            "11111111111111111111"
        };
        const char* sRestoreHighDpiTexturePattern
        {
            "[20,20]"
            "{0,paper}"
            "{1,ink}"
            "{2,ink_with_alpha}"

            "00000011111111111111"
            "00000011111111111111"
            "00000011111111111111"
            "00000011111111111111"
            "00000011000000000011"
            "00000011000000000011"
            "11111111111111000011"
            "11111111111111000011"
            "11111111111111000011"
            "11111111111111000011"
            "11000000000011000011"
            "11000000000011000011"
            "11000000000011111111"
            "11000000000011111111"
            "11000000000011000000"
            "11000000000011000000"
            "11000000000011000000"
            "11000000000011000000"
            "11111111111111000000"
            "11111111111111000000"
        };
        const char* sCloseHighDpiTexturePattern
        {
            "[20,20]"
            "{0,paper}"
            "{1,ink}"
            "{2,ink_with_alpha}"

            "11200000000000000211"
            "11120000000000002111"
            "21112000000000021112"
            "02111200000000211120"
            "00211120000002111200"
            "00021112000021112000"
            "00002111200211120000"
            "00000211122111200000"
            "00000021111112000000"
            "00000002111120000000"
            "00000002111120000000"
            "00000021111112000000"
            "00000211122111200000"
            "00002111200211120000"
            "00021112000021112000"
            "00211120000002111200"
            "02111200000000211120"
            "21112000000000021112"
            "11120000000000002111"
            "11200000000000000211"
        };
        if (iTextures[TextureMinimize] == std::nullopt || iTextures[TextureMinimize]->first != ink)
        {
            iTextures[TextureMinimize].emplace(ink,
                image{
                    dpi_select("neogfx::normal_title_bar::iTextures[TextureMinimize]::"s, "neogfx::normal_title_bar::iTextures[HighDpiTextureMinimize]::"s) + ink.to_string(),
                    dpi_select(sMinimizeTexturePattern, sMinimizeHighDpiTexturePattern), { { "paper", color{} }, { "ink", ink }, { "ink_with_alpha", ink.with_alpha(0.5) } }, dpi_select(1.0, 2.0) });
        }
        if (iTextures[TextureMaximize] == std::nullopt || iTextures[TextureMaximize]->first != ink)
        {
            iTextures[TextureMaximize].emplace(ink,
                image{
                    dpi_select("neogfx::normal_title_bar::iTextures[TextureMaximize]::"s, "neogfx::normal_title_bar::iTextures[HighDpiTextureMaximize]::"s) + ink.to_string(),
                    dpi_select(sMaximizeTexturePattern, sMaximizeHighDpiTexturePattern), { { "paper", color{} }, { "ink", ink }, { "ink_with_alpha", ink.with_alpha(0.5) } }, dpi_select(1.0, 2.0) });
        }
        if (iTextures[TextureRestore] == std::nullopt || iTextures[TextureRestore]->first != ink)
        {
            iTextures[TextureRestore].emplace(ink,
                image{
                    dpi_select("neogfx::normal_title_bar::iTextures[TextureRestore]::"s, "neogfx::normal_title_bar::iTextures[HighDpiTextureRestore]::"s) + ink.to_string(),
                    dpi_select(sRestoreTexturePattern, sRestoreHighDpiTexturePattern), { { "paper", color{} }, { "ink", ink }, { "ink_with_alpha", ink.with_alpha(0.5) } }, dpi_select(1.0, 2.0) });
        }
        if (iTextures[TextureClose] == std::nullopt || iTextures[TextureClose]->first != ink)
        {
            iTextures[TextureClose].emplace(ink,
                image{
                    dpi_select("neogfx::normal_title_bar::iTextures[TextureClose]::"s, "neogfx::normal_title_bar::iTextures[HighDpiTextureClose]::"s) + ink.to_string(),
                    dpi_select(sCloseTexturePattern, sCloseHighDpiTexturePattern), { { "paper", color{} }, { "ink", ink }, { "ink_with_alpha", ink.with_alpha(0.5) } }, dpi_select(1.0, 2.0) });
        }
        iMinimizeButton.set_image(iTextures[TextureMinimize]->second);
        iMaximizeButton.set_image(iTextures[TextureMaximize]->second);
        iRestoreButton.set_image(iTextures[TextureRestore]->second);
        iCloseButton.set_image(iTextures[TextureClose]->second);
        if (dpi_scale_factor() >= 1.5)
        {
            scoped_units su{ *this, units::Pixels };

            auto const& idealSize = size{ 3.0_mm, 3.0_mm }.ceil();
            iMinimizeButton.image_widget().set_minimum_size(idealSize);
            iMaximizeButton.image_widget().set_minimum_size(idealSize);
            iRestoreButton.image_widget().set_minimum_size(idealSize);
            iCloseButton.image_widget().set_minimum_size(idealSize);
        }
    }
}