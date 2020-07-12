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
#include <neogfx/gui/widget/normal_title_bar.hpp>
#include <neogfx/gui/widget/tool_title_bar.hpp>
#include <neogfx/gui/widget/i_title_bar.hpp>

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
        StatusBar       = 0x00100000
    };

    enum class decoration_style : uint32_t
    {
        None            = 0x00000000,
        Window          = 0x00000001,
        Tool            = 0x00000002,
        Dock            = 0x00000004,
        Popup           = 0x00000008,
        Splash          = 0x00000010,
        Nested          = 0x10000000,
        NestedWindow    = Window | Nested,
        NestedTool      = Tool | Nested,
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
            widget_part part;
            point trackFrom;
        };
    public:
        template <typename... Args>
        decorated(neogfx::decoration_style aStyle, Args&&... aArgs) :
            widget_type{ std::forward<Args>(aArgs)... }, iInitialized{ false }, iStyle{ aStyle }, iDecoration{ default_decoration(aStyle) }
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
        const i_title_bar& title_bar() const
        {
            return *iTitleBar;
        }
        i_title_bar& title_bar()
        {
            return *iTitleBar;
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
                if ((decoration() & neogfx::decoration::Dock) == neogfx::decoration::Dock)
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
        template <typename TitleBar, typename... Args>
        std::shared_ptr<i_title_bar> create_title_bar(Args&&... aArgs)
        {
            return std::make_shared<TitleBar>(*this, std::forward<Args>(aArgs)...);
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
            switch (aPart)
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
                return (decoration() & neogfx::decoration::Border) == neogfx::decoration::Border;
            // todo: the rest
            default:
                return false;
            }
        }
        widget_part part(const point& aPosition) const override
        {
            if (widget_type::debug() == this)
                std::cerr << "decorated<>::part(...) --> ";
            auto result = widget_type::part(aPosition);
            if (result == widget_part::Client)
            {
                if ((decoration() & neogfx::decoration::Border) == neogfx::decoration::Border)
                {
                    enum { left = 1, top = 2, right = 4, bottom = 8 };
                    int hit = 0;
                    auto const clientRect = as_widget().client_rect();
                    auto const clientPadding = as_widget().padding();
                    if (aPosition.x < clientPadding.left)
                        hit |= left;
                    if (aPosition.x > clientRect.right() - clientPadding.right)
                        hit |= right;
                    if (aPosition.y < clientPadding.top)
                        hit |= top;
                    if (aPosition.y > clientRect.bottom() - clientPadding.bottom)
                        hit |= bottom;
                    if (hit & top && hit & left)
                        result = widget_part::BorderTopLeft;
                    else if (hit & top && hit & right)
                        result = widget_part::BorderTopRight;
                    else if (hit & bottom && hit & left)
                        result = widget_part::BorderBottomLeft;
                    else if (hit & bottom && hit & right)
                        result = widget_part::BorderBottomRight;
                    else if (hit & left)
                        result = widget_part::BorderLeft;
                    else if (hit & top)
                        result = widget_part::BorderTop;
                    else if (hit & right)
                        result = widget_part::BorderRight;
                    else if (hit & bottom)
                        result = widget_part::BorderBottom;
                }
            }
            if (widget_type::debug() == this)
                std::cerr << result << std::endl;
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
    protected:
        void fix_weightings() override
        {
            widget_type::fix_weightings();
            as_widget().template ancestor_layout<border_layout>().fix_weightings(); // todo: is this layout relationship assumption good idea?
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
            if (aButton == mouse_button::Left && as_widget().capturing())
            {
                auto const clickedPart = part(aPosition);
                if (part_active(clickedPart))
                {
                    iTracking = tracking{ clickedPart, aPosition };
                    if (as_widget().has_root())
                        as_widget().root().window_manager().update_mouse_cursor(as_widget().root());
                    update_tracking(aPosition);
                }
            }
        }
        void mouse_moved(const point& aPosition, key_modifiers_e aKeyModifiers) override
        {
            widget_type::mouse_moved(aPosition, aKeyModifiers);
            update_tracking(aPosition);
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
                if ((decoration_style() & neogfx::decoration_style::Tool) == neogfx::decoration_style::None)
                    iTitleBar = create_title_bar<normal_title_bar>();
                else
                    iTitleBar = create_title_bar<tool_title_bar>();
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
            if ((decoration() & neogfx::decoration::Dock) == neogfx::decoration::Dock)
            {
                iDockLayout.emplace(has_layout(standard_layout::Toolbar) ? 
                    toolbar_layout(layout_position::Center) : non_client_layout());
                iDockLayout->set_padding(neogfx::padding{});
            }
            if ((decoration() & neogfx::decoration::StatusBar) == neogfx::decoration::StatusBar)
            {
                iStatusBarLayout.emplace(non_client_layout());
                iStatusBarLayout->set_padding(neogfx::padding{});
            }

            iInitialized = true;
        }
        void update_tracking(const point& aPosition)
        {
            if (iTracking)
            {
                auto const delta = aPosition - iTracking->trackFrom;
                auto const currentSize = as_widget().extents();
                as_widget().set_fixed_size({}, false);
                optional_size newSize;
                switch (iTracking->part)
                {
                case widget_part::BorderLeft:
                    newSize = as_widget().minimum_size().max(size{ currentSize.cx - delta.dx, currentSize.cy });
                    break;
                case widget_part::BorderTopLeft:
                    newSize = as_widget().minimum_size().max(size{ currentSize.cx - delta.dx, currentSize.cy - delta.dy });
                    break;
                case widget_part::BorderTop:
                    newSize = as_widget().minimum_size().max(size{ currentSize.cx, currentSize.cy - delta.dy });
                    break;
                case widget_part::BorderTopRight:
                    newSize = as_widget().minimum_size().max(size{ currentSize.cx + delta.dx, currentSize.cy - delta.dy });
                    break;
                case widget_part::BorderRight:
                    newSize = as_widget().minimum_size().max(size{ currentSize.cx + delta.dx, currentSize.cy });
                    break;
                case widget_part::BorderBottomRight:
                    newSize = as_widget().minimum_size().max(size{ currentSize.cx + delta.dx, currentSize.cy + delta.dy });
                    break;
                case widget_part::BorderBottom:
                    newSize = as_widget().minimum_size().max(size{ currentSize.cx, currentSize.cy + delta.dy });
                    break;
                case widget_part::BorderBottomLeft:
                    newSize = as_widget().minimum_size().max(size{ currentSize.cx - delta.dx, currentSize.cy + delta.dy });
                    break;
                }
                as_widget().set_fixed_size(newSize, false);
                if (as_widget().has_layout_manager())
                {
                    as_widget().layout_manager().layout_items();
                    // todo: not sure I am happy with this, assumes border layout...
                    if ((decoration_style() & neogfx::decoration_style::Dock) == neogfx::decoration_style::Dock)
                    {
                        as_widget().set_fixed_size({}, false);
                        fix_weightings();
                    }
                }
            }
        }
    private:
        static neogfx::decoration default_decoration(neogfx::decoration_style aStyle)
        {
            auto result = neogfx::decoration::None;
            if ((aStyle & (neogfx::decoration_style::Window | neogfx::decoration_style::Tool)) != neogfx::decoration_style::None)
                result |= neogfx::decoration::TitleBar;
            if ((aStyle & (neogfx::decoration_style::Window | neogfx::decoration_style::Popup)) != neogfx::decoration_style::None)
                result |= (neogfx::decoration::Menu | neogfx::decoration::Toolbar | neogfx::decoration::Dock | neogfx::decoration::StatusBar);
            if ((aStyle & (neogfx::decoration_style::Window | neogfx::decoration_style::Dock)) != neogfx::decoration_style::None)
                result |= neogfx::decoration::Border;
            return result;
        }                                      
    private:                             
        bool iInitialized;
        neogfx::decoration_style iStyle;
        neogfx::decoration iDecoration;
        std::optional<vertical_layout> iNonClientLayout;
        std::optional<vertical_layout> iTitleBarLayout;
        std::optional<vertical_layout> iMenuLayout;
        std::optional<border_layout> iToolbarLayout;
        std::optional<border_layout> iDockLayout;
        std::optional<vertical_layout> iStatusBarLayout;
        std::shared_ptr<i_title_bar> iTitleBar;
        std::shared_ptr<i_layout> iClientLayout;
        std::shared_ptr<i_widget> iClient;
        std::optional<tracking> iTracking;
    };
}