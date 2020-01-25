// i_scrollbar.hpp
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
#include <neolib/optional.hpp>
#include <neolib/timer.hpp>
#include <neogfx/core/object.hpp>
#include <neogfx/core/property.hpp>
#include <neogfx/core/i_animator.hpp>
#include <neogfx/gfx/i_graphics_context.hpp>
#include <neogfx/gui/widget/i_scrollbar.hpp>

namespace neogfx
{
    class scrollbar : public object<>, public i_scrollbar
    {
    public:
        typedef i_scrollbar abstract_type;
    private:
        typedef abstract_type property_context_type;
    public:
        struct no_transition_in_progress : std::logic_error { no_transition_in_progress() : std::logic_error{ "neogfx::scrollbar::no_transition_in_progress" } {} };
    public:
        scrollbar(i_scrollbar_container& aContainer, scrollbar_type aType, scrollbar_style aStyle = scrollbar_style::Normal, bool aIntegerPositions = true);
        ~scrollbar();
    public:
        scrollbar_type type() const override;
        scrollbar_style style() const override;
        void show() override;
        void hide() override;
        bool visible() const override;
        value_type position() const override;
        bool set_position(value_type aPosition, easing aTransition = easing::One) override;
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
        void render(i_graphics_context& aGraphicsContext) const override;
    public:
        rect element_geometry(element_e aElement) const override;
        element_e element_at(const point& aPosition) const override;
    public:
        void update(const update_params_t& aUpdateParams = update_params_t()) override;
        element_e clicked_element() const override;
        void click_element(element_e aElement) override;
        void unclick_element() override;
        void hover_element(element_e aElement) override;
        void unhover_element() override;
        void pause() override;
        void resume() override;
        void track() override;
        void untrack() override;
    public:
        static dimension width(scrollbar_style aStyle);
    private:
        bool transition_active() const;
        bool transition_paused() const;
        i_transition& transition() const;
    private:
        i_scrollbar_container& iContainer;
        scrollbar_type iType;
        scrollbar_style iStyle;
        bool iIntegerPositions;
        bool iVisible;
        value_type iMinimum;
        value_type iMaximum;
        value_type iStep;
        value_type iPage;
        std::optional<value_type> iLockedPosition;
        element_e iClickedElement;
        element_e iHoverElement;
        std::optional<std::shared_ptr<neolib::callback_timer>> iTimer;
        bool iPaused;
        point iThumbClickedPosition;
        value_type iThumbClickedValue;
        optional_point iScrollTrackPosition;
        std::optional<transition_id> iTransition;
        // properties / anchors
    public:
        define_property(property_category::interaction, value_type, Position, position)
    };
}