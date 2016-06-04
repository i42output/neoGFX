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
#include <neolib/variant.hpp>
#include "geometry.hpp"
#include "graphics_context.hpp"

namespace neogfx
{
	class i_scrollbar
	{
	public:
		enum type_e
		{
			Vertical	= 0x01,
			Horizontal	= 0x02,
		};
		enum style_e
		{
			Normal,
			Button,
			Invisible
		};
		typedef double value_type;
		enum element_e
		{
			ElementNone				= 0x00,
			ElementUpButton			= 0x01,
			ElementLeftButton		= ElementUpButton,
			ElementDownButton		= 0x02,
			ElementRightButton		= ElementDownButton,
			ElementPageUpArea		= 0x03,
			ElementPageLeftArea		= ElementPageUpArea,
			ElementPageDownArea		= 0x04,
			ElementPageRightArea	= ElementPageDownArea,
			ElementThumb			= 0x05
		};
		enum update_reason_e
		{
			Updated				= 0x00,
			Shown				= 0x01,
			Hidden				= 0x02,
			AttributeChanged	= 0x03,
			ScrolledUp			= 0x04,
			ScrolledLeft		= ScrolledUp,
			ScrolledDown		= 0x05,
			ScrolledRight		= ScrolledDown
		};
		typedef neolib::variant<point> update_params_t;
	public:
		struct element_already_clicked : std::logic_error { element_already_clicked() : std::logic_error("neogfx::i_scrollbar::element_already_clicked") {} };
		struct element_not_clicked : std::logic_error{ element_not_clicked() : std::logic_error("neogfx::i_scrollbar::element_not_clicked") {} };
	public:
		virtual type_e type() const = 0;
		virtual style_e style() const = 0;
		virtual void show() = 0;
		virtual void hide() = 0;
		virtual bool visible() const = 0;
		virtual value_type position() const = 0;
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
		virtual dimension width(const i_units_context& aContext) const = 0;
		virtual void render(graphics_context& aGraphicsContext) const = 0;
	public:
		virtual rect element_geometry(const i_units_context& aContext, element_e aElement) const = 0;
		virtual element_e element_at(const i_units_context& aContext, const point& aPosition) const = 0;
	public:
		virtual void update(const i_units_context& aContext, const update_params_t& aUpdateParams = update_params_t()) = 0;
		virtual element_e clicked_element() const = 0;
		virtual void click_element(element_e aElement) = 0;
		virtual void unclick_element() = 0;
		virtual void pause() = 0;
		virtual void resume() = 0;
		virtual void track() = 0;
		virtual void untrack() = 0;
	};

	class i_surface;

	class i_scrollbar_container
	{
	public:
		virtual rect scrollbar_geometry(const i_units_context& aContext, const i_scrollbar& aScrollbar) const = 0;
		virtual void scrollbar_updated(const i_scrollbar& aScrollbar, i_scrollbar::update_reason_e aReason) = 0;
		virtual colour scrollbar_colour(const i_scrollbar& aScrollbar) const = 0;
		virtual const i_surface& surface() const = 0;
	};
}