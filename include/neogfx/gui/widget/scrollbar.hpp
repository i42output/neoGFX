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

#include <neolib/core/optional.hpp>

#include <neogfx/gui/widget/timer.hpp>
#include <neogfx/core/object.hpp>
#include <neogfx/core/property.hpp>
#include <neogfx/core/i_transition_animator.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gui/widget/i_skinnable_item.hpp>
#include <neogfx/gui/widget/i_scrollbar.hpp>

namespace neogfx
{
    class scrollbar : public object<i_scrollbar>, public i_skinnable_item
    {
        meta_object(object<i_scrollbar>)
    public:
        typedef i_scrollbar abstract_type;
    private:
        typedef scrollbar property_context_type;
    public:
        struct no_transition : std::logic_error { no_transition() : std::logic_error{ "neogfx::scrollbar::no_transition" } {} };
    public:
        scrollbar(i_scrollbar_container& aContainer, scrollbar_orientation aOrientation, scrollbar_style aStyle = scrollbar_style::Normal, bool aIntegerPositions = true);
        ~scrollbar();
    public:
        i_scrollbar_container& container() const override;
    public:
        scrollbar_orientation orientation() const override;
        scrollbar_style style() const override;
        scrollbar_style type() const override;
        void set_style(scrollbar_style aStyle) override;
        bool always_visible() const override;
        bool always_hidden() const override;
        void show() override;
        void hide() override;
        bool visible() const override;
        bool auto_hide() const override;
        void set_auto_hide(bool aAutoHide) override;
        bool auto_hidden() const override;
        scrollbar_zone zone() const override;
        void push_zone() override;
        scrollbar_zone pop_zone() override;
        value_type position() const override;
        value_type effective_position() const override;
        value_type maximum_position() const override;
        bool set_position(value_type aPosition) override;
        value_type minimum() const override;
        void set_minimum(value_type aMinimum) override;
        value_type maximum() const override;
        void set_maximum(value_type aMaximum) override;
        value_type step() const override;
        void set_step(value_type aStep) override;
        value_type page() const override;
        void set_page(value_type aPage) override;
    public:
        bool locked() const override;
        void lock(value_type aPosition) override;
        void unlock() override;
    public:
        dimension width() const override;
        void render(i_graphics_context& aGc) const override;
    public:
        rect element_geometry(scrollbar_element aElement) const override;
        scrollbar_element element_at(const point& aPosition) const override;
    public:
        void update(const update_params_t& aUpdateParams = update_params_t()) override;
        scrollbar_element clicked_element() const override;
        void click_element(scrollbar_element aElement) override;
        void unclick_element() override;
        scrollbar_element hovering_element() const override;
        void hover_element(scrollbar_element aElement) override;
        void unhover_element() override;
        void pause() override;
        void resume() override;
        void track() override;
        void untrack() override;
    public:
        bool transition_set() const noexcept override;
        void set_transition(easing aTransition, double aTransitionDuration = 0.5, bool aOnlyWhenPaging = true) override;
        void clear_transition() override;
    public:
        static dimension width(scrollbar_style aStyle);
    public:
        bool is_widget() const final;
        const i_widget& as_widget() const final;
        i_widget& as_widget();
    public:
        rect element_rect(skin_element aElement) const override;
    private:
        i_scrollbar_container& iContainer;
        scrollbar_orientation iOrientation;
        scrollbar_style iStyle;
        bool iIntegerPositions;
        bool iVisible;
        bool iAutoHide;
        value_type iMinimum;
        value_type iMaximum;
        value_type iStep;
        value_type iPage;
        std::optional<value_type> iLockedPosition;
        scrollbar_element iClickedElement;
        scrollbar_element iHoverElement;
        std::optional<std::shared_ptr<widget_timer>> iTimer;
        bool iPaused;
        point iThumbClickedPosition;
        value_type iThumbClickedValue;
        bool iOnlyTransitionWhenPaging = true;
        optional_point iScrollTrackPosition;
        std::vector<scrollbar_zone> iZoneStack;
        // properties / anchors
    public:
        define_property(property_category::interaction, value_type, Position, position)
    };
}