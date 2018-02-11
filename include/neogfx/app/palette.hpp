// palette.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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
#include "i_palette.hpp"

namespace neogfx
{
	class palette : public i_palette
	{
	public:
		palette();
		palette(const i_palette& aOther);
		palette(const palette& aOther);
	public:
		palette& operator=(const i_palette& aOther);
	public:
		bool operator==(const i_palette& aOther) const;
		bool operator!=(const i_palette& aOther) const;
	public:
		bool has_colour() const override;
		neogfx::colour colour() const override;
		void set_colour(const optional_colour& aColour) override;
		bool has_background_colour() const override;
		neogfx::colour background_colour() const override;
		void set_background_colour(const optional_colour& aBackgroundColour) override;
		bool has_foreground_colour() const override;
		neogfx::colour foreground_colour() const override;
		void set_foreground_colour(const optional_colour& aForegroundColour) override;
		bool has_text_colour() const override;
		neogfx::colour text_colour() const override;
		void set_text_colour(const optional_colour& aTextColour) override;
		bool has_selection_colour() const override;
		neogfx::colour selection_colour() const override;
		void set_selection_colour(const optional_colour& aSelectionColour) override;
		bool has_hover_colour() const override;
		neogfx::colour hover_colour() const override;
		void set_hover_colour(const optional_colour& aHoverColour) override;
		bool has_widget_detail_primary_colour() const override;
		neogfx::colour widget_detail_primary_colour() const override;
		void set_widget_detail_primary_colour(const optional_colour& aWidgetDetailPrimaryColour) override;
		bool has_widget_detail_secondary_colour() const override;
		neogfx::colour widget_detail_secondary_colour() const override;
		void set_widget_detail_secondary_colour(const optional_colour& aWidgetDetailSecondaryColour) override;
	private:
		optional_colour iColour;
		optional_colour iBackgroundColour;
		optional_colour iForegroundColour;
		optional_colour iTextColour;
		optional_colour iSelectionColour;
		optional_colour iHoverColour;
		optional_colour iWidgetDetailPrimaryColour;
		optional_colour iWidgetDetailSecondaryColour;
	};
}