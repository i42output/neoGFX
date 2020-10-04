// tab_button.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include "push_button.hpp"
#include "i_tab.hpp"
#include "i_tab_container.hpp"

namespace neogfx
{
    class tab_button : public push_button, public i_tab
    {
    public:
        define_declared_event(Selected, selected)
        define_declared_event(Deselected, deselected)
    private:
        friend class tab_bar;
        class close_button;
    public:
        tab_button(i_tab_container& aContainer, const std::string& aText = std::string(), bool aClosable = false, bool aStandardImageSize = true);
        tab_button(i_widget& aParent, i_tab_container& aContainer, const std::string& aText = std::string(), bool aClosable = false, bool aStandardImageSize = true);
        tab_button(i_layout& aLayout, i_tab_container& aContainer, const std::string& aText = std::string(), bool aClosable = false, bool aStandardImageSize = true);
        ~tab_button();
    public:
        const i_tab_container& container() const override;
        i_tab_container& container() override;
        bool closable() const override;
        void set_closable(bool aClosable) override;
        bool is_selected() const override;
        bool is_deselected() const override;
        void select() override;
    public:
        const std::string& text() const override;
        void set_text(const std::string& aText) override;
        void set_image(const i_texture& aTexture) override;
        void set_image(const i_image& aImage) override;
    public:
        const i_widget& as_widget() const override;
        i_widget& as_widget() override;
    protected:
        rect path_bounding_rect() const override;
        bool spot_color() const override;
        bool perform_hover_animation() const override;
    protected:
        neogfx::padding padding() const override;
        void handle_clicked() override;
    protected:
        color palette_color(color_role aColorRole) const override;
    protected:
        using push_button::update;
        bool update(const rect& aUpdateRect) override;
    protected:
        void mouse_entered(const point& aPosition) override;
        void mouse_left() override;
    protected:
        void set_selected_state(bool aSelectedState);
    private:
        void init();
    private:
        sink iSink;
        i_tab_container& iContainer;
        std::unique_ptr<close_button> iCloseButton;
        bool iStandardImageSize;
        bool iSelectedState;
    };
}