// decorated.hpp
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
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/layout/border_layout.hpp>
#include <neogfx/gui/widget/widget_bits.hpp>
#include <neogfx/gui/widget/scrollable_widget.hpp>
#include <neogfx/gui/widget/normal_title_bar.hpp>
#include <neogfx/gui/widget/tool_title_bar.hpp>
#include <neogfx/gui/widget/status_bar.hpp>

namespace neogfx
{
    enum class decoration : uint32_t
    {
        None            = 0x00000000,
        Border          = 0x00000001,
        TitleBar        = 0x00010000,
        Menu            = 0x00020000,
        Toolbar         = 0x00040000,
        Dock            = 0x00080000,
        DockAreas       = 0x00100000,
        StatusBar       = 0x00200000
    };

    enum class decoration_style : uint32_t
    {
        None            = 0x00000000,
        Window          = 0x00000001,
        Dialog          = 0x00000002,
        Tool            = 0x00000004,
        Menu            = 0x00000008,
        Dock            = 0x00000010,
        Popup           = 0x00000040,
        Splash          = 0x00000080,
        Resizable       = 0x01000000,
        DockAreas       = 0x02000000,
        Nested          = 0x10000000,
        NestedWindow    = Window | Nested,
        NestedTool      = Tool | Nested,
        NestedMenu      = Menu | Nested,
        NestedPopup     = Popup | Nested,
        NestedSplash    = Splash | Nested
    };

    inline constexpr decoration operator|(decoration aLhs, decoration aRhs)
    {
        return static_cast<decoration>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr decoration operator&(decoration aLhs, decoration aRhs)
    {
        return static_cast<decoration>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    inline constexpr decoration& operator|=(decoration& aLhs, decoration aRhs)
    {
        return aLhs = static_cast<decoration>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr decoration& operator&=(decoration& aLhs, decoration aRhs)
    {
        return aLhs = static_cast<decoration>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    inline constexpr decoration_style operator~(decoration_style aValue)
    {
        return static_cast<decoration_style>(~static_cast<uint32_t>(aValue));
    }

    inline constexpr decoration_style operator|(decoration_style aLhs, decoration_style aRhs)
    {
        return static_cast<decoration_style>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr decoration_style operator&(decoration_style aLhs, decoration_style aRhs)
    {
        return static_cast<decoration_style>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    inline constexpr decoration_style& operator|=(decoration_style& aLhs, decoration_style aRhs)
    {
        return aLhs = static_cast<decoration_style>(static_cast<uint32_t>(aLhs) | static_cast<uint32_t>(aRhs));
    }

    inline constexpr decoration_style& operator&=(decoration_style& aLhs, decoration_style aRhs)
    {
        return aLhs = static_cast<decoration_style>(static_cast<uint32_t>(aLhs) & static_cast<uint32_t>(aRhs));
    }

    template <typename WidgetType, typename... OtherBases>
    class decorated : public WidgetType, public virtual i_standard_layout_container, public OtherBases...
    {
        typedef decorated<WidgetType, OtherBases...> self_type;
    public:
        typedef WidgetType widget_type;
    private:
        struct tracking
        {
            widget_part_e part;
            size startSize;
            point trackFrom;
        };
    public:
        template <typename... Args>
        decorated(neogfx::decoration_style aStyle, Args&&... aArgs) :
            widget_type{ std::forward<Args>(aArgs)... }, 
            iInitialized{ false }, 
            iStyle{ aStyle }, 
            iDecoration{ default_decoration(aStyle) },
            iAutoscale{ false }
        {
            init();
        }
    public:
        neogfx::decoration_style decoration_style() const
        {
            return iStyle;
        }
        void set_decoration_style(neogfx::decoration_style aStyle)
        {
            if (iStyle != aStyle)
            {
                iStyle = aStyle;
                as_widget().update();
                as_widget().layout().invalidate();
            }
        }
        neogfx::decoration decoration() const
        {
            return iDecoration;
        }
    public:
        bool has_client_widget() const override
        {
            return iClient != nullptr;
        }
        const i_widget& client_widget() const override
        {
            if (has_client_widget())
                return *iClient;
            throw no_client_widget();
        }
        i_widget& client_widget() override
        {
            if (has_client_widget())
                return *iClient;
            throw no_client_widget();
        }
        void set_client(i_widget& aClient) override
        {
            set_client(std::shared_ptr<i_widget>{ std::shared_ptr<i_widget>{}, & aClient });
        }
        void set_client(std::shared_ptr<i_widget> aClient) override
        {
            if (!iClientLayout)
            {
                if ((decoration() & neogfx::decoration::DockAreas) == neogfx::decoration::DockAreas)
                    iClientLayout = std::shared_ptr<i_layout>{ std::shared_ptr<i_layout>{}, &dock_layout(layout_position::Center) };
                else
                {
                    iClientLayout = std::make_shared<vertical_layout>();
                    iClientLayout->set_padding(neogfx::padding{});
                    layout_item_index clientLayoutIndex = non_client_layout().count();
                    if ((decoration() & neogfx::decoration::StatusBar) == neogfx::decoration::StatusBar)
                        clientLayoutIndex = non_client_layout().index_of(status_bar_layout());
                    non_client_layout().add_at(clientLayoutIndex, iClientLayout);
                }
            }
            iClientLayout->remove_all();
            iClient = aClient;
            if (iClient != nullptr)
                iClientLayout->add(iClient);
        }
        const i_title_bar& title_bar() const override
        {
            if (iTitleBar)
                return *iTitleBar;
            throw no_title_bar();
        }
        i_title_bar& title_bar() override
        {
            return const_cast<i_title_bar&>(to_const(*this).title_bar());
        }
        void set_title_bar(i_title_bar& aTitleBar) override
        {
            set_title_bar(std::shared_ptr<i_title_bar>{ std::shared_ptr<i_status_bar>{}, & aTitleBar });
        }
        void set_title_bar(std::shared_ptr<i_title_bar> aTitleBar)
        {
            iTitleBar = aTitleBar;
            if (iTitleBarLayout->find(iTitleBar->as_widget()) == std::nullopt)
                iTitleBarLayout->add(iTitleBar->as_widget());
        }
        const i_status_bar& status_bar() const override
        {
            if (iStatusBar)
                return *iStatusBar;
            throw no_status_bar();
        }
        i_status_bar& status_bar() override
        {
            return const_cast<i_status_bar&>(to_const(*this).status_bar());
        }
        void set_status_bar(i_status_bar& aStatusBar) override
        {
            set_status_bar(std::shared_ptr<i_status_bar>{ std::shared_ptr<i_status_bar>{}, & aStatusBar });
        }
        void set_status_bar(std::shared_ptr<i_status_bar> aStatusBar)
        {
            iStatusBar = aStatusBar;
            if (iStatusBarLayout->find(iStatusBar->as_widget()) == std::nullopt)
                iStatusBarLayout->add(iStatusBar->as_widget());
        }
        template <typename... Args>
        std::shared_ptr<i_title_bar> create_title_bar(Args&&... aArgs)
        {
            if ((decoration_style() & neogfx::decoration_style::Tool) == neogfx::decoration_style::None)
                return create_title_bar<normal_title_bar>(std::forward<Args>(aArgs)...);
            else
                return create_title_bar<tool_title_bar>(std::forward<Args>(aArgs)...);
        }
        template <typename TitleBar, typename... Args>
        std::shared_ptr<i_title_bar> create_title_bar(Args&&... aArgs)
        {
            set_title_bar(std::make_shared<TitleBar>(*this, std::forward<Args>(aArgs)...));
            return iTitleBar;
        }
        template <typename StatusBar, typename... Args>
        std::shared_ptr<i_status_bar> create_status_bar(Args&&... aArgs)
        {
            set_status_bar(std::make_shared<StatusBar>(*this, std::forward<Args>(aArgs)...));
            return iStatusBar;
        }
    public:
        bool is_widget() const override
        {
            return true;
        }
        const i_widget& as_widget() const override
        {
            return *this;
        }
        i_widget& as_widget() override
        {
            return *this;
        }
    public:
        bool part_active(widget_part aPart) const override
        {
            switch (aPart.part)
            {
            case widget_part::Border:
            case widget_part::BorderLeft:
            case widget_part::BorderTopLeft:
            case widget_part::BorderTop:
            case widget_part::BorderTopRight:
            case widget_part::BorderRight:
            case widget_part::BorderBottomRight:
            case widget_part::BorderBottom:
            case widget_part::BorderBottomLeft:
                return (decoration_style() & neogfx::decoration_style::Resizable) == neogfx::decoration_style::Resizable &&
                    (decoration() & neogfx::decoration::Border) == neogfx::decoration::Border;
                // todo: the rest
            default:
                return false;
            }
        }
        widget_part part(const point& aPosition) const override
        {
            if (debug == this)
                std::cerr << "decorated<>::part(...) --> ";
            auto result = widget_type::part(aPosition);
            if (result.part == widget_part::Client || result.part == widget_part::NonClient)
            {
                if ((decoration() & neogfx::decoration::Border) == neogfx::decoration::Border)
                {
                    enum { left = 1, top = 2, right = 4, bottom = 8 };
                    int hit = 0;
                    auto const nonClientRect = as_widget().to_client_coordinates(as_widget().non_client_rect());
                    auto const nonClientBorder = as_widget().is_root() ? as_widget().root().border() : as_widget().padding();
                    if (aPosition.x < nonClientRect.left() + nonClientBorder.left)
                        hit |= left;
                    if (aPosition.x > nonClientRect.right() - nonClientBorder.right)
                        hit |= right;
                    if (aPosition.y < nonClientRect.top() + nonClientBorder.top)
                        hit |= top;
                    if (aPosition.y > nonClientRect.bottom() - nonClientBorder.bottom)
                        hit |= bottom;
                    if (iStatusBar != nullptr)
                    {
                        point const sizeGripPos = as_widget().to_client_coordinates(iStatusBar->size_grip().non_client_rect().position());
                        rect const sizeGripRect = { sizeGripPos, size{ nonClientRect.bottom_right() - sizeGripPos } };
                        if (sizeGripRect.contains(aPosition))
                            hit |= (right | bottom);
                    }
                    if (hit & top && hit & left)
                        result.part = widget_part::BorderTopLeft;
                    else if (hit & top && hit & right)
                        result.part = widget_part::BorderTopRight;
                    else if (hit & bottom && hit & left)
                        result.part = widget_part::BorderBottomLeft;
                    else if (hit & bottom && hit & right)
                        result.part = widget_part::BorderBottomRight;
                    else if (hit & left)
                        result.part = widget_part::BorderLeft;
                    else if (hit & top)
                        result.part = widget_part::BorderTop;
                    else if (hit & right)
                        result.part = widget_part::BorderRight;
                    else if (hit & bottom)
                        result.part = widget_part::BorderBottom;
                }
            }
            if (debug == this)
                std::cerr << result.part << std::endl;
            return result;
        }
    public:
        using widget_type::has_layout;
        using widget_type::layout;
        bool has_layout(standard_layout aStandardLayout) const override
        {
            switch (aStandardLayout)
            {
            case standard_layout::Default:
                return as_widget().has_layout();
            case standard_layout::Client:
                return !!iClientLayout;
            case standard_layout::NonClient:
                return !!iNonClientLayout;
            case standard_layout::TitleBar:
                return !!iTitleBarLayout;
            case standard_layout::Menu:
                return !!iMenuLayout;
            case standard_layout::Toolbar:
                return !!iToolbarLayout;
            case standard_layout::Dock:
                return !!iDockLayout;
            case standard_layout::StatusBar:
                return !!iStatusBarLayout;
            default:
                return false;
            }
        }
        const i_layout& layout(standard_layout aStandardLayout, layout_position aPosition = layout_position::None) const override
        {
            if (!has_layout(aStandardLayout))
                throw standard_layout_not_found();
            switch (aStandardLayout)
            {
            case standard_layout::Default:
                return as_widget().layout();
            case standard_layout::Client:
                return has_client_widget() ? client_widget().layout() : *iClientLayout;
            case standard_layout::NonClient:
                return *iNonClientLayout;
            case standard_layout::TitleBar:
                return *iTitleBarLayout;
            case standard_layout::Menu:
                return *iMenuLayout;
            case standard_layout::Toolbar:
                return iToolbarLayout->part(aPosition);
            case standard_layout::Dock:
                return iDockLayout->part(aPosition);
            case standard_layout::StatusBar:
                return *iStatusBarLayout;
            default:
                throw standard_layout_not_found();
            }
        }
        i_layout& layout(standard_layout aStandardLayout, layout_position aPosition = layout_position::None) override
        {
            return const_cast<i_layout&>(to_const(*this).layout(aStandardLayout, aPosition));
        }
    public:
        void fix_resizing_context_weightings(bool aUpdateLayout = true)
        {
            if (resizing_context().has_parent_layout())
                resizing_context().parent_layout().fix_weightings(neogfx::size_policy{ size_constraint::MinimumExpanding, size_constraint::Expanding }, neogfx::size_policy{ size_constraint::Fixed, size_constraint::Expanding });
            if (aUpdateLayout)
            {
                if (resizing_context().has_parent_layout())
                    resizing_context().parent_layout().update_layout(false);
            }
        }
        void clear_resizing_context_weightings(bool aUpdateLayout = true)
        {
            if (resizing_context().has_parent_layout())
                resizing_context().parent_layout().clear_weightings(neogfx::size_policy{ size_constraint::MinimumExpanding, size_constraint::Expanding }, neogfx::size_policy{ size_constraint::Fixed, size_constraint::Expanding });
            if (aUpdateLayout)
            {
                if (resizing_context().has_parent_layout())
                    resizing_context().parent_layout().update_layout(false);
            }
        }
        bool autoscale() const
        {
            return iAutoscale;
        }
        void set_autoscale(bool aEnableAutoscale)
        {
            if (iAutoscale != aEnableAutoscale)
            {
                iAutoscale = aEnableAutoscale;
                if (autoscale())
                    fix_resizing_context_weightings();
                else
                    clear_resizing_context_weightings();
            }
        }
    protected:
        neogfx::size_policy size_policy() const override
        {
            if (as_widget().has_size_policy() || as_widget().is_root())
                return widget_type::size_policy();
            return size_constraint::MinimumExpanding;
        }
        size weight() const override
        {
            if (as_widget().has_weight() || as_widget().is_root())
                return widget_type::weight();
            return size{ 1.0 };
        }
    protected:
        void capture_released() override
        {
            widget_type::capture_released();
            iTracking = std::nullopt;
        }
    protected:
        void mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers) override
        {
            widget_type::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
            if (aButton == mouse_button::Left && as_widget().capturing() && (!as_widget().is_root() || as_widget().root().is_nested()))
            {
                auto const clickedPart = part(aPosition);
                if (part_active(clickedPart))
                {
                    iTracking = tracking{ clickedPart.part, resizing_context().extents(), widget_type::to_window_coordinates(aPosition) };
                    if (as_widget().has_root())
                        as_widget().root().window_manager().update_mouse_cursor(as_widget().root());
                }
            }
        }
        void mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) override
        {
            widget_type::mouse_moved(aPosition, aKeyModifiers);
            update_tracking(aPosition);
        }
    protected:
        rect default_clip_rect(bool aIncludeNonClient = false) const override
        {
            auto clipRect = widget_type::default_clip_rect(aIncludeNonClient);
            if (!aIncludeNonClient && has_layout(standard_layout::TitleBar) && 
                iTitleBar && iTitleBar->as_widget().widget_type() == neogfx::widget_type::NonClient)
                clipRect.y += layout(standard_layout::TitleBar).extents().cy;
            return clipRect;
        }
    protected:
        void init()
        {
            if (iInitialized)
                return;

            if ((decoration_style() & neogfx::decoration_style::Window) == neogfx::decoration_style::Window && !as_widget().is_root())
                return; // surface not yet created

            iNonClientLayout.emplace(*this);
            non_client_layout().set_padding(neogfx::padding{});
            non_client_layout().set_spacing(size{});
            if ((decoration() & neogfx::decoration::TitleBar) == neogfx::decoration::TitleBar)
            {
                iTitleBarLayout.emplace(non_client_layout());
                iTitleBarLayout->set_padding(neogfx::padding{});
            }
            // todo: create widgets for the following decorations
            if ((decoration() & neogfx::decoration::Menu) == neogfx::decoration::Menu)
            {
                iMenuLayout.emplace(non_client_layout());
                iMenuLayout->set_padding(neogfx::padding{});
            }
            if ((decoration() & neogfx::decoration::Toolbar) == neogfx::decoration::Toolbar)
            {
                iToolbarLayout.emplace(non_client_layout());
                iToolbarLayout->set_padding(neogfx::padding{});
            }
            if ((decoration() & neogfx::decoration::DockAreas) == neogfx::decoration::DockAreas)
            {
                if (!iDockLayoutContainer)
                {
                    iDockLayoutContainer.emplace(has_layout(standard_layout::Toolbar) ?
                        toolbar_layout(layout_position::Center) : non_client_layout());
                    iDockLayoutContainer->set_padding(neogfx::padding{});
                }
                iDockLayout.emplace(*iDockLayoutContainer);
                iDockLayout->set_padding(neogfx::padding{});
            }
            if ((decoration() & neogfx::decoration::StatusBar) == neogfx::decoration::StatusBar)
            {
                iStatusBarLayout.emplace(non_client_layout());
                iStatusBarLayout->set_padding(neogfx::padding{});
            }

            iInitialized = true;
        }
        i_layout_item& resizing_context()
        {
            return (decoration_style() & neogfx::decoration_style::Dock) == neogfx::decoration_style::Dock ?
                static_cast<i_layout_item&>(as_widget().parent_layout()) : static_cast<i_layout_item&>(as_widget());
        }
        void update_tracking(const point& aPosition)
        {
            if (iTracking)
            {
                i_layout_item& resizingContext = resizing_context();
                auto const delta = widget_type::to_window_coordinates(aPosition) - iTracking->trackFrom;
                auto const currentSize = resizingContext.extents();
                optional_size newSize;
                switch (iTracking->part)
                {
                case widget_part::BorderLeft:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx - delta.dx, iTracking->startSize.cy });
                    break;
                case widget_part::BorderTopLeft:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx - delta.dx, iTracking->startSize.cy - delta.dy });
                    break;
                case widget_part::BorderTop:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx, iTracking->startSize.cy - delta.dy });
                    break;
                case widget_part::BorderTopRight:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx + delta.dx, iTracking->startSize.cy - delta.dy });
                    break;
                case widget_part::BorderRight:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx + delta.dx, iTracking->startSize.cy });
                    break;
                case widget_part::BorderBottomRight:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx + delta.dx, iTracking->startSize.cy + delta.dy });
                    break;
                case widget_part::BorderBottom:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx, iTracking->startSize.cy + delta.dy });
                    break;
                case widget_part::BorderBottomLeft:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx - delta.dx, iTracking->startSize.cy + delta.dy });
                    break;
                }
                if (newSize != currentSize)
                {                
                    if (debug == this)
                        std::cerr << "update_tracking(" << aPosition << "): " << currentSize << " -> " << newSize << std::endl;
                    clear_resizing_context_weightings(false);
                    resizingContext.set_fixed_size(newSize, false);
                    fix_resizing_context_weightings(autoscale());
                    if (!autoscale())
                        clear_resizing_context_weightings();
                }
            }
        }
    private:
        static neogfx::decoration default_decoration(neogfx::decoration_style aStyle)
        {
            auto result = neogfx::decoration::None;
            if ((aStyle & (neogfx::decoration_style::Window | neogfx::decoration_style::Dialog | neogfx::decoration_style::Tool)) != neogfx::decoration_style::None)
                result |= neogfx::decoration::TitleBar;
            if ((aStyle & (neogfx::decoration_style::Window | neogfx::decoration_style::Tool)) == neogfx::decoration_style::Window)
                result |= (neogfx::decoration::Toolbar | neogfx::decoration::StatusBar);
            if ((aStyle & neogfx::decoration_style::Menu) == neogfx::decoration_style::Menu)
                result |= neogfx::decoration::Menu;
            if ((aStyle & neogfx::decoration_style::Dock) == neogfx::decoration_style::Dock)
                result |= neogfx::decoration::Dock;
            if ((aStyle & neogfx::decoration_style::DockAreas) == neogfx::decoration_style::DockAreas)
                result |= neogfx::decoration::DockAreas;
            if ((aStyle & (neogfx::decoration_style::Window | neogfx::decoration_style::Dock)) != neogfx::decoration_style::None)
                result |= neogfx::decoration::Border;
            return result;
        }                                      
    private:                             
        bool iInitialized;
        neogfx::decoration_style iStyle;
        neogfx::decoration iDecoration;
        bool iAutoscale;
        std::optional<vertical_layout> iNonClientLayout;
        std::optional<vertical_layout> iTitleBarLayout;
        std::optional<vertical_layout> iMenuLayout;
        std::optional<border_layout> iToolbarLayout;
        std::optional<scrollable_widget<>> iDockLayoutContainer;
        std::optional<border_layout> iDockLayout;
        std::optional<vertical_layout> iStatusBarLayout;
        std::shared_ptr<i_title_bar> iTitleBar;
        std::shared_ptr<i_status_bar> iStatusBar;
        std::shared_ptr<i_layout> iClientLayout;
        std::shared_ptr<i_widget> iClient;
        std::optional<tracking> iTracking;
    };
}