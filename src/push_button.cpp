// push_button.cpp
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
#include "app.hpp"
#include "pen.hpp"
#include "push_button.hpp"

namespace neogfx
{
	push_button::push_button(const std::string& aText, style_e aStyle) :
		button(aText, (aStyle == ButtonStyleNormal ? alignment::Centre : alignment::Left) | alignment::VCentre),
		iAnimator(app::instance(), [this](neolib::callback_timer&){ animate(); }, 20, false), 
		iAnimationFrame(0),
		iStyle(aStyle)
	{
		if (iStyle == ButtonStyleItemViewHeader)
		{
			set_margins(neogfx::margins(1.0, 2.0));
			layout().set_margins(neogfx::margins(0.0));
			label().set_margins(neogfx::margins(0.0));
			label().text().set_alignment(neogfx::alignment::Left | neogfx::alignment::VCentre);
		}
	}
	
	push_button::push_button(i_widget& aParent, const std::string& aText, style_e aStyle) :
		button(aParent, aText, (aStyle == ButtonStyleNormal ? alignment::Centre : alignment::Left) | alignment::VCentre),
		iAnimator(app::instance(), [this](neolib::callback_timer&){ animate(); }, 20, false), 
		iAnimationFrame(0),
		iStyle(aStyle)
	{
		if (iStyle == ButtonStyleItemViewHeader)
		{
			set_margins(neogfx::margins(1.0, 2.0));
			layout().set_margins(neogfx::margins(0.0));
			label().set_margins(neogfx::margins(0.0));
			label().text().set_alignment(neogfx::alignment::Left | neogfx::alignment::VCentre);
		}
	}

	push_button::push_button(i_layout& aLayout, const std::string& aText, style_e aStyle) :
		button(aLayout, aText, (aStyle == ButtonStyleNormal ? alignment::Centre : alignment::Left) | alignment::VCentre),
		iAnimator(app::instance(), [this](neolib::callback_timer&){ animate(); }, 20, false),
		iAnimationFrame(0),
		iStyle(aStyle)
	{
		if (iStyle == ButtonStyleItemViewHeader)
		{
			set_margins(neogfx::margins(1.0, 2.0));
			layout().set_margins(neogfx::margins(0.0));
			label().set_margins(neogfx::margins(0.0));
			label().text().set_alignment(neogfx::alignment::Left | neogfx::alignment::VCentre);
		}
	}

	void push_button::paint_non_client(graphics_context& aGraphicsContext) const
	{
		button::paint_non_client(aGraphicsContext);
		if (iStyle == ButtonStyleToolbar && capturing())
		{
			colour background = app::instance().current_style().selection_colour();
			background.set_alpha(0x80);
			aGraphicsContext.fill_solid_rect(client_rect(), background);
		}
	}

	void push_button::paint(graphics_context& aGraphicsContext) const
	{
		colour faceColour = animation_colour();
		colour borderColour = border_mid_colour().darker(0x40);
		colour innerBorderColour = border_mid_colour().lighter(capturing() ? 0x20 : 0x40);
		scoped_units su(*this, UnitsPixels);
		neogfx::path outline = path();
		dimension penWidth = device_metrics().horizontal_dpi() / 96;
		if (iStyle == ButtonStyleNormal || iStyle == ButtonStyleTab)
			outline.deflate(penWidth * 2.0, penWidth * 2.0);
		aGraphicsContext.clip_to(outline);
		colour topHalfFrom = faceColour.same_lightness_as(colour::White);
		colour topHalfTo = faceColour;
		colour bottomHalfFrom = faceColour.to_hsl().lighter(-0.125).to_rgb(faceColour.alpha() / 255.0);
		colour bottomHalfTo = faceColour;
		if (iStyle != ButtonStyleTab)
		{
			if (!capturing())
			{
				if (!spot_colour())
				{
					rect topHalf = outline.bounding_rect();
					rect bottomHalf = topHalf;
					topHalf.cy = std::floor(topHalf.cy * 0.5);
					bottomHalf.y = topHalf.bottom();
					bottomHalf.cy -= topHalf.height();
					aGraphicsContext.fill_gradient_rect(topHalf, gradient(topHalfFrom, topHalfTo));
					aGraphicsContext.fill_gradient_rect(bottomHalf, gradient(bottomHalfFrom, bottomHalfTo));
				}
				else
				{
					aGraphicsContext.fill_solid_rect(outline.bounding_rect(), faceColour);
				}
			}
			else 
			{
				if (!spot_colour())
				{
					rect topHalf = outline.bounding_rect();
					rect bottomHalf = topHalf;
					topHalf.cy = std::floor(topHalf.cy * 0.5 + as_units(*this, UnitsMillimetres, 1.0));
					bottomHalf.y = topHalf.bottom();
					bottomHalf.cy -= topHalf.height();
					aGraphicsContext.fill_gradient_rect(topHalf, gradient(topHalfFrom, topHalfTo));
					aGraphicsContext.fill_gradient_rect(bottomHalf, gradient(bottomHalfFrom, bottomHalfTo));
				}
				else
				{
					aGraphicsContext.fill_solid_rect(outline.bounding_rect(), faceColour);
				}
			}
		}
		else
		{
			if (!spot_colour())
			{
				aGraphicsContext.fill_gradient_rect(outline.bounding_rect(), gradient(topHalfTo, bottomHalfFrom));
			}
			else
			{
				aGraphicsContext.fill_solid_rect(outline.bounding_rect(), faceColour);
			}
		}
		aGraphicsContext.reset_clip();
		if (has_focus())
		{
			rect focusRect = outline.bounding_rect();
			focusRect.deflate(2.0, 2.0);
			aGraphicsContext.draw_focus_rect(focusRect);
		}
		if (iStyle == ButtonStyleNormal || iStyle == ButtonStyleTab)
		{
			outline.inflate(penWidth, penWidth);
			aGraphicsContext.draw_path(outline, pen(innerBorderColour, penWidth));
			outline.inflate(penWidth, penWidth);
			aGraphicsContext.draw_path(outline, pen(borderColour, penWidth));
		}
	}

	void push_button::mouse_entered()
	{
		button::mouse_entered();
		iAnimator.again_if();
		update();
	}

	void push_button::mouse_left()
	{
		button::mouse_left();
		iAnimator.again_if();
		update();
	}

	rect push_button::path_bounding_rect() const
	{
		return client_rect();
	}

	path push_button::path() const
	{
		neogfx::path ret;
		size pixel = units_converter(*this).from_device_units(size(1.0, 1.0));
		size currentSize = path_bounding_rect().extents();
		switch (iStyle)
		{
		case ButtonStyleNormal:
		case ButtonStyleTab:
			ret.move_to(pixel.cx, 0, 12);
			ret.line_to(currentSize.cx - pixel.cx, 0);
			ret.line_to(currentSize.cx - pixel.cx, pixel.cy);
			ret.line_to(currentSize.cx - 0, pixel.cy);
			ret.line_to(currentSize.cx - 0, currentSize.cy - pixel.cy);
			ret.line_to(currentSize.cx - pixel.cx, currentSize.cy - pixel.cy);
			ret.line_to(currentSize.cx - pixel.cx, currentSize.cy - 0);
			ret.line_to(pixel.cx, currentSize.cy - 0);
			ret.line_to(pixel.cx, currentSize.cy - pixel.cy);
			ret.line_to(0, currentSize.cy - pixel.cy);
			ret.line_to(0, pixel.cy);
			ret.line_to(pixel.cx, pixel.cy);
			ret.line_to(pixel.cx, 0);
			break;
		case ButtonStyleItemViewHeader:
			ret.move_to(0, 0, 4);
			ret.line_to(currentSize.cx, 0);
			ret.line_to(currentSize.cx, currentSize.cy);
			ret.line_to(0, currentSize.cy);
			ret.line_to(0, 0);
			break;
		}
		ret.set_position(path_bounding_rect().top_left());
		return ret;
	}

	bool push_button::spot_colour() const
	{
		return false;
	}

	colour push_button::border_mid_colour() const
	{
		return animation_colour();
	}

	bool push_button::has_hover_colour() const
	{
		return iHoverColour != boost::none;
	}

	colour push_button::hover_colour() const
	{
		colour hoverColour = (has_hover_colour() ? *iHoverColour : app::instance().current_style().hover_colour());
		if (capturing())
			return hoverColour.light(0x40) ? hoverColour.darker(0x40) : hoverColour.lighter(0x40);
		else 
			return hoverColour;
	}

	void push_button::set_hover_colour(const optional_colour& aHoverColour)
	{
		iHoverColour = aHoverColour;
		update();
	}

	void push_button::animate()
	{
		if (surface().destroyed()) 
			return;

		if (entered())
		{
			if (iAnimationFrame < kMaxAnimationFrame)
			{
				++iAnimationFrame;
				iAnimator.again();
			}
		}
		else
		{
			if (iAnimationFrame > 0)
			{
				--iAnimationFrame;
				iAnimator.again();
			}
		}
		update();
	}

	colour push_button::animation_colour() const
	{
		return animation_colour(iAnimationFrame);
	}

	colour push_button::animation_colour(uint32_t aAnimationFrame) const
	{
		colour faceColour;
		faceColour = foreground_colour();
		if (capturing())
		{
			if (faceColour.light(0x40))
				faceColour.darken(0x40);
			else
				faceColour.lighten(0x40);
		}
		return (enabled() && entered()) || aAnimationFrame > 0 ? gradient(faceColour, hover_colour()).at(static_cast<coordinate>(aAnimationFrame), 0, static_cast<coordinate>(kMaxAnimationFrame)) : faceColour;
	}
}

