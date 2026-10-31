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

#include <unordered_map>

#include <neogfx/app/i_app.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gui/widget/normal_title_bar.hpp>

namespace neogfx
{
    namespace
    {
        // the title bar merged into each menu bar (see normal_title_bar::merged_title_bar): the windows of a nest are maximized 
        // together but only the one in front has its title bar merged
        std::unordered_map<i_widget const*, normal_title_bar*>& merged_title_bars()
        {
            static std::unordered_map<i_widget const*, normal_title_bar*> sMergedTitleBars;
            return sMergedTitleBars;
        }

        // the window a maximized nested window's title bar is merged into (see normal_title_bar::merged_title_bar), and its menu bar: 
        // the window hosting the nested window's MDI nest provided it is the only MDI nest (with windows) the window hosts and the 
        // window has a (visible) menu bar
        i_window* mdi_merge_host(i_window& aNestedWindow, i_widget*& aMenuBar)
        {
            auto& surfaceManager = service<i_surface_manager>();
            auto& windowNest = surfaceManager.find_nest(aNestedWindow.native_window());
            if (windowNest.type() != nest_type::MDI || !aNestedWindow.has_parent_window() || !windowNest.widget().has_root())
                return nullptr;
            auto& host = aNestedWindow.parent_window();
            if (&windowNest.widget().root() != &host)
                return nullptr;
            // only the window in front of the nest's windows (the nest widget's first child that is one) is merged
            for (auto const& child : windowNest.widget().children())
                if (child->is_root() && child->root().is_nested() && child->root().has_native_window() && windowNest.has(child->root().native_window()))
                {
                    if (&child->root() != &aNestedWindow)
                        return nullptr;
                    break;
                }
            for (std::size_t n = 0; n < surfaceManager.nest_count(); ++n)
            {
                auto const& otherNest = surfaceManager.nest_at(n);
                if (&otherNest != &windowNest && otherNest.type() == nest_type::MDI && otherNest.nested_window_count() > 0 &&
                    otherNest.widget().has_root() && &otherNest.widget().root() == &host)
                    return nullptr;
            }
            i_standard_layout_container& hostContainer = host;
            if (!hostContainer.has_layout(standard_layout::Menu))
                return nullptr;
            auto& menuLayout = hostContainer.menu_layout();
            for (layout_item_index i = 0; i < menuLayout.count(); ++i)
                if (menuLayout.is_widget_at(i) && menuLayout.get_widget_at(i).object_type() == object_type::MenuBar && menuLayout.get_widget_at(i).visible())
                {
                    aMenuBar = &menuLayout.get_widget_at(i);
                    return &host;
                }
            return nullptr;
        }
    }

    normal_title_bar::merged_title_bar::merged_title_bar(normal_title_bar& aOwner, i_window& aHost, i_widget& aMenuBar, i_texture const& aIcon, i_string const& aTitle) :
        owner{ aOwner },
        host{ aHost },
        menuBar{ aMenuBar },
        hostDestroyed{ aHost },
        menuBarDestroyed{ aMenuBar },
        originalMenuBarLayoutPadding{ aMenuBar.layout().has_padding() ? optional_padding{ aMenuBar.layout().padding() } : optional_padding{} },
        menuBarLayoutPadding{ aMenuBar.layout().padding() },
        originalMenuBarSizePolicy{ aMenuBar.has_size_policy() ? optional_size_policy{ aMenuBar.size_policy() } : optional_size_policy{} },
        hostTitle{ aHost.title_text() },
        icon{ aMenuBar, aIcon },
        spacer{ aMenuBar.layout() },
        minimizeButton{ aMenuBar.layout(), push_button_style::TitleBar },
        restoreButton{ aMenuBar.layout(), push_button_style::TitleBar },
        closeButton{ aMenuBar.layout(), push_button_style::TitleBar }
    {
        // the buttons go at the end of the menu bar's row, not just after its items
        menuBar.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, menuBar.size_policy().vertical_constraint() });
        set_title(aTitle);
        sinks += menuBar.size_changed([this]() { update_geometry(); });
        update_geometry();
        merged_title_bars()[&menuBar] = &owner;
    }

    normal_title_bar::merged_title_bar::~merged_title_bar()
    {
        auto existing = merged_title_bars().find(&menuBar);
        if (existing != merged_title_bars().end() && existing->second == &owner)
            merged_title_bars().erase(existing);
        if (!menuBarDestroyed)
        {
            menuBar.layout().set_padding(originalMenuBarLayoutPadding);
            menuBar.set_size_policy(originalMenuBarSizePolicy);
        }
        // (the host's title is left alone if its owner has changed it since)
        if (!hostDestroyed && host.title_text().to_std_string_view() == mergedTitle.to_std_string_view())
            host.set_title_text(hostTitle);
    }

    void normal_title_bar::merged_title_bar::set_title(i_string const& aTitle)
    {
        if (hostDestroyed)
            return;
        if (!mergedTitle.to_std_string_view().empty() && host.title_text().to_std_string_view() != mergedTitle.to_std_string_view())
            hostTitle = string{ host.title_text() }; // (its owner has changed the host's title since merging)
        if (aTitle.to_std_string_view().empty())
            mergedTitle = hostTitle;
        else
            mergedTitle = string{ hostTitle.to_std_string() + " - [" + aTitle.to_std_string() + "]" };
        host.set_title_text(mergedTitle);
    }

    void normal_title_bar::merged_title_bar::update_geometry()
    {
        if (menuBarDestroyed)
            return;
        auto const menuBarRect = menuBar.client_rect(false);
        auto const height = menuBarRect.cy;
        if (height <= 0.0)
            return;
        size const buttonExtents{ std::ceil(height * 4.0 / 3.0), height };
        minimizeButton.set_fixed_size(buttonExtents);
        restoreButton.set_fixed_size(buttonExtents);
        closeButton.set_fixed_size(buttonExtents);
        icon.move(menuBarRect.top_left());
        icon.resize(size{ height, height });
        // the menu bar's items are kept clear of the icon (which isn't in the menu bar's layout as the layout's items are 
        // indexed by menu item)
        neogfx::padding const mergedPadding{ 
            menuBarLayoutPadding.left + std::ceil(height * 1.25), menuBarLayoutPadding.top, 
            menuBarLayoutPadding.right, menuBarLayoutPadding.bottom };
        if (menuBar.layout().padding() != mergedPadding)
            menuBar.layout().set_padding(mergedPadding);
    }

    normal_title_bar::normal_title_bar(i_standard_layout_container& aContainer, std::string const& aTitle) :
        widget{ aContainer.title_bar_layout() },
        iOuterLayout{ *this },
        iInnerLayout{ iOuterLayout },
        iButtonLayout{ iOuterLayout },
        iIcon{ iInnerLayout, service<i_app>().default_window_icon() },
        iTitle{ iInnerLayout, aTitle, text_widget_type::SingleLine, text_widget_flags::UseFade },
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
        if (iMerged)
            iMerged->set_title(aTitle);
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

    size_policy normal_title_bar::size_policy() const
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

        // a top-level window's title bar buttons are handled by the native window's non-client hit testing; a nested 
        // window has no native non-client area so its title bar buttons are ordinary button clicks
        auto nested = [this]() { return root().has_native_window() && root().native_window().is_nested(); };
        iMinimizeButton.clicked([this, nested]()
        {
            if (nested())
                root().iconize();
        });
        iMaximizeButton.clicked([this, nested]()
        {
            if (nested())
                root().maximize();
        });
        iRestoreButton.clicked([this, nested]()
        {
            if (nested())
                root().restore();
        });
        iCloseButton.clicked([this, nested]()
        {
            if (nested() && root().can_close())
                root().close();
        });

        iSink += service<i_app>().current_style_changed([this](style_aspect aAspect) 
        { 
            if ((aAspect & style_aspect::Color) == style_aspect::Color) 
                update_textures(); 
        });
        
        auto update_widgets = [this](bool aUpdateLayout = false)
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
            // can_close() is false for a strong window until app::exec() is running, but the window's
            // first paint can happen before that; don't flash a disabled close box (execution_started
            // re-runs this with the real value).
            iCloseButton.enable((root().can_close() || !service<i_app>().in_exec()) && isEnabled);
            iMinimizeButton.show(!isIconic && (root().style() & window_style::MinimizeBox) == window_style::MinimizeBox);
            iMaximizeButton.show(!isMaximized && (root().style() & window_style::MaximizeBox) == window_style::MaximizeBox);
            iRestoreButton.show(!isRestored && (root().style() & (window_style::MinimizeBox | window_style::MaximizeBox)) != window_style::Invalid);
            iCloseButton.show((root().style() & window_style::Close) != window_style::Invalid);
            update_textures();
            if (aUpdateLayout)
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
                update_widgets();
                break;
            case window_event_type::Iconized:
            case window_event_type::Maximized:
            case window_event_type::Restored:
                update_widgets(true);
                update_merge();
                break;
            default:
                break;
            }
        });

        iSink += root().activated([this, update_widgets]() { update_widgets(); update_merge(); });
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
        if (iMerged)
        {
            iMerged->minimizeButton.set_image(iTextures[TextureMinimize]->second);
            iMerged->restoreButton.set_image(iTextures[TextureRestore]->second);
            iMerged->closeButton.set_image(iTextures[TextureClose]->second);
        }
        if (dpi_scale_factor() >= 1.5)
        {
            scoped_units su{ *this, units::Pixels };

            auto const& idealSize = size{ 3.0_mm, 3.0_mm }.ceil();
            iMinimizeButton.image_widget().set_minimum_size(idealSize);
            iMaximizeButton.image_widget().set_minimum_size(idealSize);
            iRestoreButton.image_widget().set_minimum_size(idealSize);
            iCloseButton.image_widget().set_minimum_size(idealSize);
            if (iMerged)
            {
                iMerged->minimizeButton.image_widget().set_minimum_size(idealSize);
                iMerged->restoreButton.image_widget().set_minimum_size(idealSize);
                iMerged->closeButton.image_widget().set_minimum_size(idealSize);
            }
        }
        if (iMerged)
            iMerged->update_geometry();
    }

    void normal_title_bar::update_merge()
    {
        i_window* host = nullptr;
        i_widget* menuBar = nullptr;
        if (root().has_native_window() && root().native_window().is_nested() && root().is_maximized())
            host = mdi_merge_host(root(), menuBar);
        if (iMerged && (host == nullptr || &iMerged->host != host || &iMerged->menuBar != menuBar))
        {
            iMerged.reset();
            show();
        }
        if (host == nullptr || iMerged)
            return;
        // (another maximized window's title bar merged into the menu bar is unmerged first so the menu bar is as it was)
        auto existing = merged_title_bars().find(menuBar);
        if (existing != merged_title_bars().end() && existing->second != this)
        {
            auto& other = *existing->second;
            other.iMerged.reset();
            other.show();
        }
        iMerged = std::make_unique<merged_title_bar>(*this, *host, *menuBar, icon_widget().image(), title());
        iMerged->minimizeButton.show((root().style() & window_style::MinimizeBox) == window_style::MinimizeBox);
        iMerged->closeButton.show((root().style() & window_style::Close) != window_style::Invalid);
        iMerged->closeButton.enable(root().can_close());
        // (unmerging first, so the window's own title bar is back when its geometry is changed, destroys the button that was clicked)
        iMerged->minimizeButton.clicked([this]()
        {
            auto& self = *this;
            self.iMerged.reset();
            self.show();
            self.root().iconize();
        });
        iMerged->restoreButton.clicked([this]()
        {
            auto& self = *this;
            self.iMerged.reset();
            self.show();
            self.root().restore();
        });
        iMerged->closeButton.clicked([this]()
        {
            if (root().can_close())
                root().close();
        });
        iMerged->update_geometry();
        update_textures();
        hide();
    }
}