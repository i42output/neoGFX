// tool_title_bar.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/task/timer.hpp>
#include <neogfx/core/color.hpp>
#include <neogfx/gfx/texture.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/i_tool.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/widget/push_button.hpp>
#include <neogfx/gui/widget/i_title_bar.hpp>

namespace neogfx
{
    class tool_title_bar : public widget, public i_title_bar
    {
    public:
        tool_title_bar(i_standard_layout_container& aContainer, const std::string& aTitle = std::string{});
    public:
        const i_widget& as_widget() const override;
        i_widget& as_widget() override;
    public:
        const std::string& title() const override;
        void set_title(const std::string& aTitle) override;
    protected:
        size minimum_size(const optional_size& aAvailableSpace = {}) const override;
    protected:
        neogfx::widget_type widget_type() const override;
    protected:
        bool transparent_background() const override;
    protected:
        color background_color() const override;
    protected:
        neogfx::focus_policy focus_policy() const override;
    private:
        void update_textures();
        void update_state();
    private:
        i_standard_layout_container& iContainer;
        neolib::callback_timer iUpdater;
        horizontal_layout iLayout;
        text_widget iTitle;
        push_button iPinButton;
        push_button iUnpinButton;
        push_button iCloseButton;
        sink iSink;
        mutable std::optional<std::pair<color, texture>> iPinTexture;
        mutable std::optional<std::pair<color, texture>> iUnpinTexture;
        mutable std::optional<std::pair<color, texture>> iCloseTexture;
        bool iStateActive;
    };
}