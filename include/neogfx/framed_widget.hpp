// framed_widget.hpp
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
#include "widget.hpp"

namespace neogfx
{
	class framed_widget : public widget
	{
	public:
		enum style_e
		{
			NoFrame,
			DottedFrame,
			DashedFrame,
			SolidFrame,
			ContainerFrame,
			DoubleFrame,
			GrooveFrame,
			RidgeFrame,
			InsetFrame,
			OutsetFrame,
			HiddenFrame
		};
	public:
		framed_widget(style_e aStyle = SolidFrame, dimension aLineWidth = 1.0);
		framed_widget(const framed_widget&) = delete;
		framed_widget(i_widget& aParent, style_e aStyle = SolidFrame, dimension aLineWidth = 1.0);
		framed_widget(i_layout& aLayout, style_e aStyle = SolidFrame, dimension aLineWidth = 1.0);
		~framed_widget();
	public:
		virtual rect client_rect(bool aIncludeMargins = true) const;
	public:
		virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
	public:
		virtual bool transparent_background() const;
		virtual void paint_non_client(graphics_context& aGraphicsContext) const;
		virtual void paint(graphics_context& aGraphicsContext) const;
	protected:
		dimension line_width() const;
		dimension effective_frame_width() const;
	private:
		style_e iStyle;
		dimension iLineWidth;
	};
}