// status_bar.hpp
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
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include <neogfx/gui/layout/horizontal_layout.hpp>
#include <neogfx/gui/layout/stack_layout.hpp>
#include <neogfx/gui/widget/label.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/image_widget.hpp>
#include <neogfx/gui/widget/i_status_bar.hpp>

namespace neogfx
{
    extern template class widget<i_status_bar>;

    class status_bar : public widget<i_status_bar>
    {
    public:
        typedef i_status_bar abstract_type;
        enum class style : uint32_t
        {
            DisplayNone                 = 0x0000,
            DisplayMessage              = 0x0001,
            DisplayKeyboardLocks        = 0x0010,
            DisplaySizeGrip             = 0x4000,
            BackgroundAsWindowBorder    = 0x8000
        };
        friend constexpr style operator|(style aLhs, style aRhs)
        {
            return static_cast<style>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
        }
        friend constexpr style operator&(style aLhs, style aRhs)
        {
            return static_cast<style>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
        }
    public:
        class separator : public widget<>
        {
        public:
            separator(i_widget& aStatusBar);
            separator(i_layout& aStatusBarLayout);
        public:
            neogfx::size_policy size_policy() const override;
            size minimum_size(optional_size const& aAvailableSpace) const override;
        public:
            void paint(i_graphics_context& aGc) const override;
        private:
            i_widget& iStatusBar;
        };
    private:
        class keyboard_lock_status : public widget<>
        {
        public:
            keyboard_lock_status(i_layout& aLayout);
        public:
            neogfx::size_policy size_policy() const override;
        private:
            horizontal_layout iLayout;
            std::unique_ptr<widget_timer> iUpdater;
        };
        class size_grip_widget : public image_widget
        {
        public:
            size_grip_widget(i_layout& aLayout);
        public:
            neogfx::size_policy size_policy() const override;
        public:
            widget_part part(const point& aPosition) const override;
        };
    public:
        struct style_conflict : std::runtime_error { style_conflict() : std::runtime_error("neogfx::status_bar::style_conflict") {} };
        struct no_message : std::runtime_error { no_message() : std::runtime_error("neogfx::status_bar::no_message") {} };
    public:
        status_bar(i_standard_layout_container& aContainer, style aStyle = style::DisplayMessage | style::DisplayKeyboardLocks | style::DisplaySizeGrip);
    public:
        bool have_message() const override;
        i_string const& message() const override;
        void set_message(i_string const& aMessage) override;
        void clear_message() override;
        void add_normal_widget(i_widget& aWidget) override;
        void add_normal_widget_at(widget_index aPosition, i_widget& aWidget) override;
        void add_normal_widget(i_ref_ptr<i_widget> const &aWidget) override;
        void add_normal_widget_at(widget_index aPosition, i_ref_ptr<i_widget> const& aWidget) override;
        void add_permanent_widget(i_widget& aWidget) override;
        void add_permanent_widget_at(widget_index aPosition, i_widget& aWidget) override;
        void add_permanent_widget(i_ref_ptr<i_widget> const& aWidget) override;
        void add_permanent_widget_at(widget_index aPosition, i_ref_ptr<i_widget> const& aWidget) override;
    public:
        i_layout& normal_layout() override;
        i_layout& permanent_layout() override;
    public:
        label& message_widget() override;
        label& idle_widget() override;
    protected:
        neogfx::size_policy size_policy() const override;
    protected:
        widget_part part(const point& aPosition) const override;
    protected:
        bool is_managing_layout() const override;
    protected:
        bool has_palette_color(color_role aColorRole) const override;
        color palette_color(color_role aColorRole) const override;
    public:
        void set_font(optional_font const& aFont) override;
    protected:
        const i_widget& size_grip() const override;
    private:
        void init();
        void update_widgets();
    private:
        sink iSink;
        style iStyle;
        std::optional<string> iMessage;
        horizontal_layout iLayout;
        stack_layout iNormalLayout;
        horizontal_layout iMessageLayout;
        label iMessageLabel;
        horizontal_layout iIdleLayout;
        label iIdleLabel;
        widget<> iNormalWidgetContainer;
        horizontal_layout iNormalWidgetLayout;
        horizontal_layout iPermanentWidgetLayout;
        keyboard_lock_status iKeyboardLockStatus;
        mutable std::map<color, std::optional<texture>> iSizeGripTexture;
        size_grip_widget iSizeGrip;
    };
}