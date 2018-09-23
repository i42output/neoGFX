// i_palette.hpp
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
#include <neogfx/core/event.hpp>
#include <neogfx/core/colour.hpp>
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
	class i_palette
	{
	public:
		event<> changed;
	public:
		virtual ~i_palette() {}
	public:
		virtual bool has_colour() const = 0;
		virtual neogfx::colour colour() const = 0;
		virtual void set_colour(const optional_colour& aDefaultColour) = 0;
		virtual bool has_background_colour() const = 0;
		virtual neogfx::colour background_colour() const = 0;
		virtual void set_background_colour(const optional_colour& aBackgroundColour) = 0;
		virtual bool has_foreground_colour() const = 0;
		virtual neogfx::colour foreground_colour() const = 0;
		virtual void set_foreground_colour(const optional_colour& aForegroundColour) = 0;
		virtual bool has_text_colour() const = 0;
		virtual neogfx::colour text_colour() const = 0;
		virtual void set_text_colour(const optional_colour& aTextColour) = 0;
		virtual bool has_selection_colour() const = 0;
		virtual neogfx::colour selection_colour() const = 0;
		virtual void set_selection_colour(const optional_colour& aSelectionColour) = 0;
		virtual bool has_hover_colour() const = 0;
		virtual neogfx::colour hover_colour() const = 0;
		virtual void set_hover_colour(const optional_colour& aHoverColour) = 0;
		virtual bool has_widget_detail_primary_colour() const = 0;
		virtual neogfx::colour widget_detail_primary_colour() const = 0;
		virtual void set_widget_detail_primary_colour(const optional_colour& aWidgetDetailPrimaryColour) = 0;
		virtual bool has_widget_detail_secondary_colour() const = 0;
		virtual neogfx::colour widget_detail_secondary_colour() const = 0;
		virtual void set_widget_detail_secondary_colour(const optional_colour& aWidgetDetailSecondaryColour) = 0;
		// helpers
	public:
		neogfx::colour text_colour_for_widget(const i_widget& aWidget) const
		{
			optional_colour textColour;
			const i_widget* w = nullptr;
			do
			{
				if (w == nullptr)
					w = &aWidget;
				else
					w = &w->parent();
				if (w->has_background_colour())
				{
					textColour = w->background_colour().luma() >= 0.5 ? colour::Black : colour::White;
					break;
				}
				else if (w->has_foreground_colour())
				{
					textColour = w->foreground_colour().luma() >= 0.5 ? colour::Black : colour::White;
					break;
				}
			} while (w->has_parent());
			auto defaultTextColour = text_colour();
			if (textColour == std::nullopt || textColour->similar_intensity(defaultTextColour))
				textColour = defaultTextColour;
			return *textColour;
		}
	};
}