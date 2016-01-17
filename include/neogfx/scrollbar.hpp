// i_scrollbar.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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

#include "neogfx.hpp"
#include <neolib/optional.hpp>
#include <neolib/timer.hpp>
#include "i_scrollbar.hpp"
#include "graphics_context.hpp"

namespace neogfx
{
	class scrollbar : public i_scrollbar
	{
	public:
		scrollbar(i_scrollbar_container& aContainer, type_e aType, bool aIntegerPositions = true);
	public:
		virtual type_e type() const;
		virtual void show();
		virtual void hide();
		virtual bool visible() const;
		virtual value_type position() const;
		virtual bool set_position(value_type aPosition);
		virtual value_type minimum() const;
		virtual void set_minimum(value_type aMinimum);
		virtual value_type maximum() const;
		virtual void set_maximum(value_type aMaximum);
		virtual value_type step() const;
		virtual void set_step(value_type aStep);
		virtual value_type page() const;
		virtual void set_page(value_type aPage);
	public:
		virtual dimension width(const i_units_context& aContext) const;
		virtual void render(graphics_context& aGraphicsContext) const;
	public:
		virtual rect element_geometry(const i_units_context& aContext, element_e aElement) const;
		virtual element_e element_at(const i_units_context& aContext, const point& aPosition) const;
	public:
		virtual void update(const i_units_context& aContext, const update_params_t& aUpdateParams = update_params_t());
		virtual element_e clicked_element() const;
		virtual void click_element(element_e aElement);
		virtual void unclick_element();
		virtual void hover_element(element_e aElement);
		virtual void unhover_element();
		virtual void pause();
		virtual void resume();
		virtual void track();
		virtual void untrack();
	private:
		i_scrollbar_container& iContainer;
		type_e iType;
		bool iIntegerPositions;
		bool iVisible;
		value_type iPosition;
		value_type iMinimum;
		value_type iMaximum;
		value_type iStep;
		value_type iPage;
		element_e iClickedElement;
		element_e iHoverElement;
		boost::optional<std::shared_ptr<neolib::callback_timer>> iTimer;
		bool iPaused;
		point iThumbClickedPosition;
		value_type iThumbClickedValue;
		optional_point iScrollTrackPosition;
	};
}