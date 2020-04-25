// title_bar.hpp
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
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/widget/image_widget.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/push_button.hpp>

namespace neogfx
{
    class title_bar : public widget
    {
    private:
        template <typename WidgetType, widget_part WidgetPart>
        class non_client_item : public WidgetType
        {
        public:
            using WidgetType::WidgetType;
        public:
            widget_part hit_test(const point&) const override
            {
                return WidgetPart;
            }
            bool ignore_non_client_mouse_events() const override
            {
                return false;
            }
        };
    private:
        enum texture_index_e
        {
            TextureMinimize,
            TextureMaximize,
            TextureRestore,
            TextureClose
        };
    public:
        title_bar(i_standard_layout_container& aContainer, const std::string& aTitle = std::string{});
        title_bar(i_standard_layout_container& aContainer, const i_texture& aIcon, const std::string& aTitle = std::string{});
        title_bar(i_standard_layout_container& aContainer, const i_image& aIcon, const std::string& aTitle = std::string{});
    public:
        const image_widget& icon() const;
        image_widget& icon();
        const text_widget& title() const;
        text_widget& title();
    public:
        neogfx::size_policy size_policy() const override;
    public:
        widget_part hit_test(const point& aPosition) const override;
    private:
        void init();
        void update_textures();
    private:
        horizontal_layout iLayout;
        non_client_item<image_widget, widget_part::SystemMenu> iIcon;
        non_client_item<text_widget, widget_part::TitleBar> iTitle;
        horizontal_spacer iSpacer;
        non_client_item<push_button, widget_part::MinimizeButton> iMinimizeButton;
        non_client_item<push_button, widget_part::MaximizeButton> iMaximizeButton;
        non_client_item<push_button, widget_part::MaximizeButton> iRestoreButton;
        non_client_item<push_button, widget_part::CloseButton> iCloseButton;
        sink iSink;
        mutable std::optional<std::pair<color, texture>> iTextures[4];
    };

    template <>
    inline widget_part title_bar::non_client_item<image_widget, widget_part::SystemMenu>::hit_test(const point&) const
    {
        if ((static_cast<const title_bar&>(parent()).root().style() & window_style::SystemMenu) == window_style::SystemMenu)
            return widget_part::SystemMenu;
        return widget_part::TitleBar;
    }
}