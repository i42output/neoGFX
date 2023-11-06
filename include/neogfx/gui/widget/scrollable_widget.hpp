// scrollable_widget.hpp
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
#include <neogfx/gui/widget/framed_widget.hpp>
#include <neogfx/gui/widget/scrollbar.hpp>

namespace neogfx
{
    enum class scrolling_disposition : uint32_t
    {
        DontScrollChildWidget = 0x00,
        ScrollChildWidgetVertically = 0x01,
        ScrollChildWidgetHorizontally = 0x02,
        DontConsiderChildWidgets = 0x08
    };

    inline constexpr scrolling_disposition operator|(scrolling_disposition aLhs, scrolling_disposition aRhs)
    {
        return static_cast<scrolling_disposition>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr scrolling_disposition operator&(scrolling_disposition aLhs, scrolling_disposition aRhs)
    {
        return static_cast<scrolling_disposition>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    template <Widget Base = widget<>>
    class scrollable_widget : public Base, private i_scrollbar_container
    {
        meta_object(Base)
        typedef scrollable_widget<Base> self_type;
    protected:
        enum usv_stage_e
        {
            UsvStageInit,
            UsvStageCheckVertical1,
            UsvStageCheckHorizontal,
            UsvStageCheckVertical2,
            UsvStageDone
        };
        class suppress_scrollbar_visibility_updates : neolib::scoped_flag
        {
        public:
            suppress_scrollbar_visibility_updates(self_type& aParent) :
                neolib::scoped_flag{ aParent.iSuppressScrollbarVisibilityUpdates }
            {}
        };
    public:
        template <typename... Args>
        scrollable_widget(Args&&... aArgs) :
            scrollable_widget{ neogfx::scrollbar_style::Normal, std::forward<Args>(aArgs)... }
        {
        }
        template <typename... Args>
        scrollable_widget(neogfx::scrollbar_style aScrollbarStyle, Args&&... aArgs) :
            base_type{ std::forward<Args>(aArgs)... },
            iScrollbarStyle{ aScrollbarStyle },
            iVerticalScrollbar{ *this, scrollbar_orientation::Vertical, aScrollbarStyle },
            iHorizontalScrollbar{ *this, scrollbar_orientation::Horizontal, aScrollbarStyle },
            iIgnoreScrollbarUpdates{ 0 },
            iUpdatingScrollbarVisibility{ false },
            iMovingWidgets{ false },
            iSuppressScrollbarVisibilityUpdates{ false }
        {
            init();
        }
        template <typename... Args>
        scrollable_widget(i_widget& aParent, Args&&... aArgs) :
            scrollable_widget{ aParent, neogfx::scrollbar_style::Normal, std::forward<Args>(aArgs)... }
        {
        }
        template <typename... Args>
        scrollable_widget(i_widget& aParent, neogfx::scrollbar_style aScrollbarStyle, Args&&... aArgs) :
            base_type{ aParent, std::forward<Args>(aArgs)... },
            iScrollbarStyle{ aScrollbarStyle },
            iVerticalScrollbar{ *this, scrollbar_orientation::Vertical, aScrollbarStyle },
            iHorizontalScrollbar{ *this, scrollbar_orientation::Horizontal, aScrollbarStyle },
            iIgnoreScrollbarUpdates{ 0 },
            iUpdatingScrollbarVisibility{ false },
            iMovingWidgets{ false },
            iSuppressScrollbarVisibilityUpdates{ false }
        {
            init();
        }
        template <typename... Args>
        scrollable_widget(i_layout& aLayout, Args&&... aArgs) :
            scrollable_widget{ aLayout, neogfx::scrollbar_style::Normal, std::forward<Args>(aArgs)... }
        {
        }
        template <typename... Args>
        scrollable_widget(i_layout& aLayout, neogfx::scrollbar_style aScrollbarStyle, Args&&... aArgs) :
            base_type{ aLayout, std::forward<Args>(aArgs)... },
            iScrollbarStyle{ aScrollbarStyle },
            iVerticalScrollbar{ *this, scrollbar_orientation::Vertical, aScrollbarStyle },
            iHorizontalScrollbar{ *this, scrollbar_orientation::Horizontal, aScrollbarStyle },
            iIgnoreScrollbarUpdates{ 0 },
            iUpdatingScrollbarVisibility{ false },
            iMovingWidgets{ false },
            iSuppressScrollbarVisibilityUpdates{ false }
        {
            init();
        }
        ~scrollable_widget();
        scrollable_widget(const scrollable_widget&) = delete;
    public:
        neogfx::scrollbar_style scrollbar_style() const;
    public:
        point scroll_position() const;
        void scroll_to(i_widget& aChild);
    public:
        void layout_items_started() override;
        void layout_items_completed() override;
    public:
        void resized() override;
        rect client_rect(bool aExtendIntoPadding = true) const override;
        widget_part part(point const& aPosition) const override;
    public:
        void paint_non_client_after(i_graphics_context& aGc) const override;
    public:
        bool mouse_wheel_scrolled(mouse_wheel aWheel, point const& aPosition, delta aDelta, key_modifiers_e aKeyModifiers) override;
        void mouse_button_pressed(mouse_button aButton, point const& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_double_clicked(mouse_button aButton, point const& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_button_released(mouse_button aButton, point const& aPosition) override;
        void mouse_moved(point const& aPosition, key_modifiers_e aKeyModifiers) override;
        void mouse_entered(point const& aPosition) override;
        void mouse_left() override;
    public:
        bool key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers) override;
    public:
        virtual scrollbar const& vertical_scrollbar() const;
        virtual scrollbar& vertical_scrollbar();
        virtual scrollbar const& horizontal_scrollbar() const;
        virtual scrollbar& horizontal_scrollbar();
        virtual neogfx::scrolling_disposition scrolling_disposition() const;
        virtual neogfx::scrolling_disposition scrolling_disposition(i_widget const& aChildWidget) const;
    public:
        rect scroll_area() const override;
        size scroll_page() const override;
    protected:
        rect scrollbar_geometry(i_scrollbar const& aScrollbar) const override;
        void scrollbar_updated(i_scrollbar const& aScrollbar, i_scrollbar::update_reason_e aReason) override;
        color scrollbar_color(i_scrollbar const& aScrollbar) const override;
    protected:
        i_widget const& as_widget() const override;
        i_widget& as_widget() override;
    protected:
        virtual bool use_scrollbar_container_updater() const;
        void update_scrollbar_visibility() override;
        virtual bool update_scrollbar_visibility(usv_stage_e aStage);
        virtual void scroll_page_updated();
    protected:
        void init_scrollbars();
    private:
        void init();
    private:
        sink iSink;
        std::optional<widget_timer> iScrollbarUpdater;
        neogfx::scrollbar_style iScrollbarStyle;
        scrollbar iVerticalScrollbar;
        scrollbar iHorizontalScrollbar;
        point iOldScrollPosition;
        uint32_t iIgnoreScrollbarUpdates;
        bool iUpdatingScrollbarVisibility;
        bool iMovingWidgets;
        bool iSuppressScrollbarVisibilityUpdates;
    };

    extern template class scrollable_widget<framed_widget<widget<>>>;
    using framed_scrollable_widget = scrollable_widget<framed_widget<widget<>>>;
}
