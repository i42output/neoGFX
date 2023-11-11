// i_scrollbar.hpp
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
#include <neolib/core/variant.hpp>
#include <neogfx/core/i_property.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/core/easing.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>

namespace neogfx
{
    enum class scrollbar_orientation : std::uint32_t
    {
        Vertical    = 0x01,
        Horizontal  = 0x02,
    };

    enum class scrollbar_style : std::uint32_t
    {
        None            = 0x00,
        Normal          = 0x01,
        Menu            = 0x02,
        Scroller        = 0x03,

        AlwaysVisible   = 0x80,

        TYPE_MASK    = 0x0F
    };

    inline scrollbar_style operator~(scrollbar_style aStyle)
    {
        return static_cast<scrollbar_style>(~static_cast<std::uint32_t>(aStyle));
    }

    inline scrollbar_style operator&(scrollbar_style aLhs, scrollbar_style aRhs)
    {
        return static_cast<scrollbar_style>(static_cast<std::uint32_t>(aLhs) & static_cast<std::uint32_t>(aRhs));
    }

    inline scrollbar_style operator|(scrollbar_style aLhs, scrollbar_style aRhs)
    {
        return static_cast<scrollbar_style>(static_cast<std::uint32_t>(aLhs) | static_cast<std::uint32_t>(aRhs));
    }

    enum class scrollbar_element : std::uint32_t
    {
        None             = 0x00,
        Scrollbar        = 0x01,
        UpButton         = 0x02,
        LeftButton       = UpButton,
        DownButton       = 0x03,
        RightButton      = DownButton,
        PageUpArea       = 0x04,
        PageLeftArea     = PageUpArea,
        PageDownArea     = 0x05,
        PageRightArea    = PageDownArea,
        Thumb            = 0x06
    };

    enum class scrollbar_zone
    {
        Top,
        Middle,
        Bottom,
        Left = Top,
        Right = Bottom
    };

    class i_transition;

    class i_scrollbar_container;

    class i_scrollbar : public i_property_owner
    {
    public:
        typedef double value_type;
        enum update_reason_e
        {
            Updated                = 0x00,
            Shown                = 0x01,
            Hidden                = 0x02,
            AttributeChanged    = 0x03,
            ScrolledUp            = 0x04,
            ScrolledLeft        = ScrolledUp,
            ScrolledDown        = 0x05,
            ScrolledRight        = ScrolledDown
        };
        typedef neolib::variant<point> update_params_t;
    public:
        struct element_already_clicked : std::logic_error { element_already_clicked() : std::logic_error("neogfx::i_scrollbar::element_already_clicked") {} };
        struct element_not_clicked : std::logic_error{ element_not_clicked() : std::logic_error("neogfx::i_scrollbar::element_not_clicked") {} };
        struct already_locked : std::logic_error { already_locked() : std::logic_error("neogfx::i_scrollbar::already_locked") {} };
        struct not_locked : std::logic_error { not_locked() : std::logic_error("neogfx::i_scrollbar::not_locked") {} };
    public:
        virtual i_scrollbar_container& container() const = 0;
    public:
        virtual scrollbar_orientation orientation() const = 0;
        virtual scrollbar_style style() const = 0;
        virtual scrollbar_style type() const = 0;
        virtual void set_style(scrollbar_style aStyle) = 0;
        virtual bool always_visible() const = 0;
        virtual bool always_hidden() const = 0;
        virtual void show() = 0;
        virtual void hide() = 0;
        virtual bool visible() const = 0;
        virtual bool auto_hide() const = 0;
        virtual void set_auto_hide(bool aAutoHide) = 0;
        virtual bool auto_hidden() const = 0;
        virtual scrollbar_zone zone() const = 0;
        virtual void push_zone() = 0;
        virtual scrollbar_zone pop_zone() = 0;
        virtual value_type position() const = 0;
        virtual value_type effective_position() const = 0;
        virtual value_type maximum_position() const = 0;
        virtual bool set_position(value_type aPosition) = 0;
        virtual value_type minimum() const = 0;
        virtual void set_minimum(value_type aMinimum) = 0;
        virtual value_type maximum() const = 0;
        virtual void set_maximum(value_type aMaximum) = 0;
        virtual value_type step() const = 0;
        virtual void set_step(value_type aStep) = 0;
        virtual value_type page() const = 0;
        virtual void set_page(value_type aPage) = 0;
    public:
        virtual bool locked() const = 0;
        virtual void lock(value_type aPosition) = 0;
        virtual void unlock() = 0;
    public:
        virtual dimension width() const = 0;
        virtual void render(i_graphics_context& aGc) const = 0;
    public:
        virtual rect element_geometry(scrollbar_element aElement) const = 0;
        virtual scrollbar_element element_at(point const& aPosition) const = 0;
    public:
        virtual void update(const update_params_t& aUpdateParams = update_params_t()) = 0;
        virtual scrollbar_element clicked_element() const = 0;
        virtual void click_element(scrollbar_element aElement) = 0;
        virtual void unclick_element() = 0;
        virtual scrollbar_element hovering_element() const = 0;
        virtual void hover_element(scrollbar_element aElement) = 0;
        virtual void unhover_element() = 0;
        virtual void pause() = 0;
        virtual void resume() = 0;
        virtual void track() = 0;
        virtual void untrack() = 0;
    public:
        virtual bool transition_set() const noexcept = 0;
        virtual void set_transition(easing aTransition, double aTransitionDuration = 0.5, bool aOnlyWhenPaging = true) = 0;
        virtual void clear_transition() = 0;
    };

    class i_scrollbar_container
    {
        friend class scrollbar_container_updater;
    public:
        virtual rect scroll_area() const = 0;
        virtual rect scroll_page() const = 0;
    public:
        virtual rect scrollbar_geometry(const i_scrollbar& aScrollbar) const = 0;
        virtual void scrollbar_updated(const i_scrollbar& aScrollbar, i_scrollbar::update_reason_e aReason) = 0;
        virtual color scrollbar_color(const i_scrollbar& aScrollbar) const = 0;
    public:
        virtual const i_widget& as_widget() const = 0;
        virtual i_widget& as_widget() = 0;
    protected:
        virtual void update_scrollbar_visibility() = 0;
    };

    class i_scrollbar_container_updater : public i_service
    {
    public:
        virtual void queue(i_scrollbar_container& aContainer) = 0;
        virtual void unqueue(i_scrollbar_container& aContainer) = 0;
        virtual bool processing() const = 0;
        virtual void process() = 0;
        virtual i_scrollbar_container& current() const = 0;
    public:
        // {76C8BC39-25EA-4E55-B41C-4798E609E5B9}
        static uuid const& iid() { static uuid const sIid{ 0x76c8bc39, 0x25ea, 0x4e55, 0xb41c, { 0x47, 0x98, 0xe6, 0x9, 0xe5, 0xb9 } }; return sIid; }
    };
}