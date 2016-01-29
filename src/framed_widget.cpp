// framed_widget.cpp
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

#include "neogfx.hpp"
#include "framed_widget.hpp"

namespace neogfx
{
	framed_widget::framed_widget(style_e aStyle, dimension aLineWidth) : 
		iStyle(aStyle), iLineWidth(aLineWidth)
	{
	}

	framed_widget::framed_widget(i_widget& aParent, style_e aStyle, dimension aLineWidth) :
		widget(aParent), iStyle(aStyle), iLineWidth(aLineWidth)
	{
	}

	framed_widget::framed_widget(i_layout& aLayout, style_e aStyle, dimension aLineWidth) :
		widget(aLayout), iStyle(aStyle), iLineWidth(aLineWidth)
	{
	}

	framed_widget::~framed_widget()
	{
	}

	point framed_widget::origin(bool aNonClient) const
	{
		point o = widget::origin(aNonClient);
		if (!aNonClient)
		{
			o.x += effective_frame_width();
			o.y += effective_frame_width();
		}
		return o;
	}

	rect framed_widget::client_rect(bool aIncludeMargins) const
	{
		rect cr = widget::client_rect(aIncludeMargins);
		cr.cx -= effective_frame_width() * 2.0;
		cr.cy -= effective_frame_width() * 2.0;
		return cr;
	}

	bool framed_widget::transparent_background() const
	{
		return false;
	}

	void framed_widget::paint_non_client(graphics_context& aGraphicsContext) const
	{
		colour frameColour = (background_colour().dark() ? background_colour().lighter(0x60) : background_colour().darker(0x60));
		widget::paint_non_client(aGraphicsContext);
		switch (iStyle)
		{
		case NoFrame:
		case HiddenFrame:
		default:
			break;
		case DottedFrame:
			break;
		case DashedFrame:
			break;
		case SolidFrame:
			aGraphicsContext.draw_rect(rect(point(0.0, 0.0), window_rect().extents()), pen(frameColour, effective_frame_width()));
			break;
		case DoubleFrame:
			break;
		case GrooveFrame:
			break;
		case RidgeFrame:
			break;
		case InsetFrame:
			break;
		case OutsetFrame:
			break;
		}
	}

	void framed_widget::paint(graphics_context& aGraphicsContext) const
	{
		widget::paint(aGraphicsContext);
	}

	dimension framed_widget::effective_frame_width() const
	{
		switch (iStyle)
		{
		case NoFrame:
		default:
			return 0.0;
		case DottedFrame:
		case DashedFrame:
		case SolidFrame:
			return units_converter(*this).from_device_units(iLineWidth);
		case DoubleFrame:
		case GrooveFrame:
		case RidgeFrame:
			return units_converter(*this).from_device_units(iLineWidth) * 3.0;
		case InsetFrame:
		case OutsetFrame:
		case HiddenFrame:
			return units_converter(*this).from_device_units(iLineWidth);
		}
	}
}