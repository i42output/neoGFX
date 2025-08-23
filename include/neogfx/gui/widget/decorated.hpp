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

#include <neogfx/app/i_app.hpp>
#include <neogfx/gui/widget/i_skin_manager.hpp>
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
    enum class decoration : std::uint32_t
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

    enum class decoration_style : std::uint32_t
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
        return static_cast<decoration>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr decoration operator&(decoration aLhs, decoration aRhs)
    {
        return static_cast<decoration>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr decoration& operator|=(decoration& aLhs, decoration aRhs)
    {
        return aLhs = static_cast<decoration>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr decoration& operator&=(decoration& aLhs, decoration aRhs)
    {
        return aLhs = static_cast<decoration>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr decoration_style operator~(decoration_style aValue)
    {
        return static_cast<decoration_style>(~static_cast<std::uint32_t>(aValue));
    }

    inline constexpr decoration_style operator|(decoration_style aLhs, decoration_style aRhs)
    {
        return static_cast<decoration_style>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr decoration_style operator&(decoration_style aLhs, decoration_style aRhs)
    {
        return static_cast<decoration_style>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr decoration_style& operator|=(decoration_style& aLhs, decoration_style aRhs)
    {
        return aLhs = static_cast<decoration_style>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    inline constexpr decoration_style& operator&=(decoration_style& aLhs, decoration_style aRhs)
    {
        return aLhs = static_cast<decoration_style>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    template <Widget WidgetType, typename... OtherBases>
    class decorated : public WidgetType, public virtual i_standard_layout_container, public OtherBases...
    {
        meta_object(WidgetType)
    public:
        typedef WidgetType widget_type;
    private:
        struct tracking
        {
            widget_part_e part;
            point startPosition;
            size startSize;
            point trackFrom;
        };
    public:
        template <typename... Args>
        decorated(neogfx::decoration_style aStyle, Args&&... aArgs) :
            widget_type{ std::forward<Args>(aArgs)... }, 
            iInitialized{ false }, 
            iStyle{ aStyle }, 
            iDecoration{ default_decoration(aStyle) }
        {
            init();
        }
    public:
        neogfx::autoscale autoscale() const
        {
            if (resizing_context().has_parent_layout())
                return resizing_context().parent_layout().autoscale();
            return neogfx::autoscale::Default;
        }
        neogfx::decoration_style decoration_style() const
        {
            return iStyle;
        }
        void set_decoration_style(neogfx::decoration_style aStyle)
        {
            auto& self = widget_type::as_widget();

            if (iStyle != aStyle)
            {
                iStyle = aStyle;
                self.update();
                self.update_layout();
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
            set_client(ref_ptr<i_widget>{ ref_ptr<i_widget>{}, &aClient });
        }
        void set_client(i_ref_ptr<i_widget> const& aClient) override
        {
            if (!iClientLayout)
            {
                if ((decoration() & neogfx::decoration::DockAreas) == neogfx::decoration::DockAreas)
                    iClientLayout = ref_ptr<i_layout>{ ref_ptr<i_layout>{}, &dock_layout(layout_position::Center) };
                else
                {
                    iClientLayout = make_ref<vertical_layout>();
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
            {
                iClientLayout->add(iClient);
                iClient->set_parent(iClientLayout->parent_widget());
            }
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
            set_title_bar(ref_ptr<i_title_bar>{ ref_ptr<i_title_bar>{}, &aTitleBar });
        }
        void set_title_bar(i_ref_ptr<i_title_bar> const& aTitleBar)
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
            set_status_bar(ref_ptr<i_status_bar>{ ref_ptr<i_status_bar>{}, & aStatusBar });
        }
        void set_status_bar(i_ref_ptr<i_status_bar> const& aStatusBar)
        {
            iStatusBar = aStatusBar;
            if (iStatusBarLayout->find(iStatusBar->as_widget()) == std::nullopt)
                iStatusBarLayout->add(iStatusBar->as_widget());
        }
        template <typename... Args>
        ref_ptr<i_title_bar> create_title_bar(Args&&... aArgs)
        {
            if ((decoration_style() & neogfx::decoration_style::Tool) == neogfx::decoration_style::None)
                return create_title_bar<normal_title_bar>(std::forward<Args>(aArgs)...);
            else
                return create_title_bar<tool_title_bar>(std::forward<Args>(aArgs)...);
        }
        template <typename TitleBar, typename... Args>
        ref_ptr<i_title_bar> create_title_bar(Args&&... aArgs)
        {
            set_title_bar(to_abstract(make_ref<TitleBar>(*this, std::forward<Args>(aArgs)...)));
            return iTitleBar;
        }
        template <typename StatusBar, typename... Args>
        ref_ptr<i_status_bar> create_status_bar(Args&&... aArgs)
        {
            set_status_bar(to_abstract(make_ref<StatusBar>(*this, std::forward<Args>(aArgs)...)));
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
            case widget_part::TitleBar:
            case widget_part::Grab:
                return true;
                // todo: the rest
            default:
                return false;
            }
        }
        widget_part part(const point& aPosition) const override
        {
            auto& self = widget_type::as_widget();

#ifdef NEOGFX_DEBUG
            if (debug::layoutItem == this)
                service<debug::logger>() << neolib::logger::severity::Debug << "decorated<>::part(...) --> ";
#endif // NEOGFX_DEBUG
            auto result = widget_type::part(aPosition);
            if (result.part == widget_part::Client || result.part == widget_part::NonClient)
            {
                if (iTitleBar != nullptr && self.to_client_coordinates(iTitleBar->to_window_coordinates(iTitleBar->client_rect())).contains(aPosition))
                    result.part = widget_part::TitleBar;
                else if (iMenuLayout && rect{ iMenuLayout->origin(), iMenuLayout->extents() }.contains(aPosition))
                    result.part = widget_part::Grab;
                else if (iToolbarLayout && rect{ iToolbarLayout->origin(), iToolbarLayout->extents() }.contains(aPosition))
                    result.part = widget_part::Grab;
                if ((decoration() & neogfx::decoration::Border) == neogfx::decoration::Border)
                {
                    enum { left = 1, top = 2, right = 4, bottom = 8 };
                    int hit = 0;
                    auto const nonClientRect = self.to_client_coordinates(self.non_client_rect());
                    auto const nonClientBorder = self.is_root() ? self.root().window_border() : self.internal_spacing();
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
                        point const sizeGripPos = self.to_client_coordinates(iStatusBar->size_grip().non_client_rect().position());
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
#ifdef NEOGFX_DEBUG
            if (debug::layoutItem == this)
                service<debug::logger>() << neolib::logger::severity::Debug << result.part << std::endl;
#endif // NEOGFX_DEBUG
            return result;
        }
    public:
        using widget_type::has_layout;
        using widget_type::layout;
        bool has_layout(standard_layout aStandardLayout) const override
        {
            auto& self = widget_type::as_widget();

            switch (aStandardLayout)
            {
            case standard_layout::Default:
                return self.has_layout();
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
            auto& self = widget_type::as_widget();

            if (!has_layout(aStandardLayout))
                throw standard_layout_not_found();

            switch (aStandardLayout)
            {
            case standard_layout::Default:
                return self.layout();
            case standard_layout::Client:
                return has_client_widget() ? client_widget().layout() : *iClientLayout;
            case standard_layout::NonClient:
                return *iNonClientLayout;
            case standard_layout::TitleBar:
                return *iTitleBarLayout;
            case standard_layout::Menu:
                return *iMenuLayout;
            case standard_layout::Toolbar:
                if (aPosition == layout_position::None)
                    return *iToolbarLayout;
                else
                    return iToolbarLayout->part(aPosition);
            case standard_layout::Dock:
                if (aPosition == layout_position::None)
                    return *iDockLayout;
                else
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
        void fix_weightings(bool aRecalculate = true) override
        {
            widget_type::fix_weightings(aRecalculate);
            if (resizing_context().has_parent_layout())
            {
                resizing_context().parent_layout().fix_weightings(aRecalculate);
                resizing_context().parent_layout().update_layout(false, false);
            }
        }
    protected:
        neogfx::size_policy size_policy() const override
        {
            auto& self = widget_type::as_widget();

            if (self.has_size_policy() || self.is_root())
                return widget_type::size_policy();
            return size_constraint::MinimumExpanding;
        }
        size weight() const override
        {
            auto& self = widget_type::as_widget();

            if (self.has_weight() || self.is_root())
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
            auto& self = widget_type::as_widget();

            widget_type::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
            if (aButton == mouse_button::Left && self.capturing() && 
                (!self.is_root() || self.root().is_nested()))
            {
                auto const clickedPart = part(aPosition);
                if (part_active(clickedPart))
                {
                    iTracking = tracking{ clickedPart.part, resizing_context().position(), resizing_context().extents(), widget_type::to_window_coordinates(aPosition) };
                    if (self.has_root())
                        self.root().window_manager().update_mouse_cursor(self.root());
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
                iTitleBar && (iTitleBar->as_widget().widget_type() & neogfx::widget_type::NonClient) == neogfx::widget_type::NonClient)
                clipRect.y += layout(standard_layout::TitleBar).extents().cy;
            return clipRect;
        }
    protected:
        void init()
        {
            auto& self = widget_type::as_widget();

            if (iInitialized)
                return;

            if ((decoration_style() & neogfx::decoration_style::NestedWindow) == neogfx::decoration_style::Window && !self.is_root())
                return; // surface not yet created

            self.set_padding(neogfx::padding{});

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
                    iDockLayoutContainer->painted([&](i_graphics_context& aGc)
                    {
                        service<i_skin_manager>().active_skin().draw_separators(aGc, *iDockLayoutContainer, dock_layout(dock_area::East).parent_layout());
                        service<i_skin_manager>().active_skin().draw_separators(aGc, *iDockLayoutContainer, dock_layout(dock_area::North).parent_layout());
                    });
                }
                iDockLayout.emplace(*iDockLayoutContainer);
                iDockLayout->set_padding(neogfx::padding{});
                iDockLayout->center().set_padding(service<i_app>().current_style().padding(padding_role::Dock));
            }
            if ((decoration() & neogfx::decoration::StatusBar) == neogfx::decoration::StatusBar)
            {
                iStatusBarLayout.emplace(non_client_layout());
                iStatusBarLayout->set_padding(neogfx::padding{});
            }

            iInitialized = true;
        }
        i_layout_item const& resizing_context() const
        {
            auto& self = widget_type::as_widget();

            return (decoration_style() & neogfx::decoration_style::Dock) == neogfx::decoration_style::Dock ?
                static_cast<i_layout_item const&>(self.parent_layout()) : static_cast<i_layout_item const&>(self);
        }
        i_layout_item& resizing_context()
        {
            return const_cast<i_layout_item&>(to_const(*this).resizing_context());
        }
        void update_tracking(const point& aPosition)
        {
            auto& self = widget_type::as_widget();

            if (iTracking)
            {
                i_layout_item& resizingContext = resizing_context();
                auto const delta = widget_type::to_window_coordinates(aPosition) - iTracking->trackFrom;
                auto const currentPosition = resizingContext.position();
                auto const currentSize = resizingContext.extents();
                optional_point newPosition;
                optional_size newSize;
                switch (iTracking->part)
                {
                case widget_part::TitleBar:
                    newPosition = iTracking->startPosition + delta;
                    break;
                case widget_part::BorderLeft:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx - delta.x, iTracking->startSize.cy });
                    if (newSize != currentSize)
                        newPosition = iTracking->startPosition - (*newSize - iTracking->startSize).with_cy(0.0);
                    break;
                case widget_part::BorderTopLeft:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx - delta.x, iTracking->startSize.cy - delta.y });
                    if (newSize != currentSize)
                        newPosition = iTracking->startPosition - (*newSize - iTracking->startSize);
                    break;
                case widget_part::BorderTop:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx, iTracking->startSize.cy - delta.y });
                    if (newSize != currentSize)
                        newPosition = iTracking->startPosition - (*newSize - iTracking->startSize).with_cx(0.0);
                    break;
                case widget_part::BorderTopRight:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx + delta.x, iTracking->startSize.cy - delta.y });
                    if (newSize != currentSize)
                        newPosition = iTracking->startPosition - (*newSize - iTracking->startSize).with_cx(0.0);
                    break;
                case widget_part::BorderRight:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx + delta.x, iTracking->startSize.cy });
                    break;
                case widget_part::BorderBottomRight:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx + delta.x, iTracking->startSize.cy + delta.y });
                    break;
                case widget_part::BorderBottom:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx, iTracking->startSize.cy + delta.y });
                    break;
                case widget_part::BorderBottomLeft:
                    newSize = resizingContext.minimum_size().max(size{ iTracking->startSize.cx - delta.x, iTracking->startSize.cy + delta.y });
                    if (newSize != currentSize)
                        newPosition = iTracking->startPosition - (*newSize - iTracking->startSize).with_cy(0.0);
                    break;
                }
                if (newPosition && newPosition != currentPosition)
                {
#ifdef NEOGFX_DEBUG
                    if (debug::layoutItem == this)
                        service<debug::logger>() << neolib::logger::severity::Debug << "update_tracking(" << aPosition << "): " << currentPosition << " -> " << newPosition << std::endl;
#endif // NEOGFX_DEBUG
                    if ((decoration_style() & neogfx::decoration_style::Nested) != neogfx::decoration_style::Nested)
                    {
                        // todo - undock/float
                    }
                    else
                        self.move(newPosition.value());
                }
                if (newSize && newSize != currentSize)
                {                
#ifdef NEOGFX_DEBUG
                    if (debug::layoutItem == this)
                        service<debug::logger>() << neolib::logger::severity::Debug << "update_tracking(" << aPosition << "): " << currentSize << " -> " << newSize << std::endl;
#endif // NEOGFX_DEBUG
                    if ((decoration_style() & neogfx::decoration_style::Nested) != neogfx::decoration_style::Nested)
                    {
                        resizingContext.set_fixed_size(newSize, false);
                        fix_weightings();
                    }
                    else
                        self.resize(newSize.value());
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
            if ((aStyle & (neogfx::decoration_style::Window | neogfx::decoration_style::Dialog | neogfx::decoration_style::Dock)) != neogfx::decoration_style::None)
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
        std::optional<layout_manager<scrollable_widget<>>> iDockLayoutContainer;
        std::optional<border_layout> iDockLayout;
        std::optional<vertical_layout> iStatusBarLayout;
        ref_ptr<i_title_bar> iTitleBar;
        ref_ptr<i_status_bar> iStatusBar;
        ref_ptr<i_layout> iClientLayout;
        ref_ptr<i_widget> iClient;
        std::optional<tracking> iTracking;
    };
}