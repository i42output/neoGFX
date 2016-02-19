// image_widget.cpp
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
#include "image_widget.hpp"

namespace neogfx
{
	image_widget::image_widget(const i_texture& aTexture, aspect_ratio aAspectRatio, cardinal_placement aPlacement) :
		iTexture(aTexture), iAspectRatio(aAspectRatio), iPlacement(aPlacement)
	{
		set_margins(neogfx::margins{});
	}

	image_widget::image_widget(const i_image& aImage, aspect_ratio aAspectRatio, cardinal_placement aPlacement) :
		iTexture(aImage), iAspectRatio(aAspectRatio), iPlacement(aPlacement)
	{
		set_margins(neogfx::margins{});
	}

	image_widget::image_widget(i_widget& aParent, const i_texture& aTexture, aspect_ratio aAspectRatio, cardinal_placement aPlacement) :
		widget(aParent), iTexture(aTexture), iAspectRatio(aAspectRatio), iPlacement(aPlacement)
	{
		set_margins(neogfx::margins{});
	}

	image_widget::image_widget(i_widget& aParent, const i_image& aImage, aspect_ratio aAspectRatio, cardinal_placement aPlacement) :
		widget(aParent), iTexture(aImage), iAspectRatio(aAspectRatio), iPlacement(aPlacement)
	{
		set_margins(neogfx::margins{});
	}

	image_widget::image_widget(i_layout& aLayout, const i_texture& aTexture, aspect_ratio aAspectRatio, cardinal_placement aPlacement) :
		widget(aLayout), iTexture(aTexture), iAspectRatio(aAspectRatio), iPlacement(aPlacement)
	{
		set_margins(neogfx::margins{});
	}

	image_widget::image_widget(i_layout& aLayout, const i_image& aImage, aspect_ratio aAspectRatio, cardinal_placement aPlacement) :
		widget(aLayout), iTexture(aImage), iAspectRatio(aAspectRatio), iPlacement(aPlacement)
	{
		set_margins(neogfx::margins{});
	}

	size image_widget::minimum_size() const
	{
		if (has_minimum_size())
			return widget::minimum_size();
		scoped_units su(*this, UnitsPixels);
		size result = iTexture.extents();
		return convert_units(*this, su.saved_units(), result);
	}

	void image_widget::paint(graphics_context& aGraphicsContext) const
	{
		scoped_units su(*this, UnitsPixels);
		rect placementRect(point{}, iTexture.extents());
		if (iAspectRatio == aspect_ratio::Stretch)
		{
			placementRect.cx = client_rect().width();
			placementRect.cy = client_rect().height();
		}
		else if (placementRect.width() >= placementRect.height())
		{
			switch (iAspectRatio)
			{
			case aspect_ratio::Ignore:
				if (placementRect.width() > client_rect().width())
					placementRect.cx = client_rect().width();
				if (placementRect.height() > client_rect().height())
					placementRect.cy = client_rect().height();
				break;
			case aspect_ratio::Keep:
				if (placementRect.width() > client_rect().width())
				{
					placementRect.cx = client_rect().width();
					placementRect.cy = placementRect.cx * iTexture.extents().cy / iTexture.extents().cx;
				}
				if (placementRect.height() > client_rect().height())
				{
					placementRect.cy = client_rect().height();
					placementRect.cx = placementRect.cy * iTexture.extents().cx / iTexture.extents().cy;
				}
				break;
			case aspect_ratio::KeepExpanding:
				if (placementRect.height() > client_rect().height())
				{
					placementRect.cy = client_rect().height();
					placementRect.cx = placementRect.cy * iTexture.extents().cx / iTexture.extents().cy;
				}
				break;
			}
		}
		else
		{
			switch (iAspectRatio)
			{
			case aspect_ratio::Ignore:
				if (placementRect.width() > client_rect().width())
					placementRect.cx = client_rect().width();
				if (placementRect.height() > client_rect().height())
					placementRect.cy = client_rect().height();
				break;
			case aspect_ratio::Keep:
				if (placementRect.height() > client_rect().height())
				{
					placementRect.cy = client_rect().height();
					placementRect.cx = placementRect.cy * iTexture.extents().cx / iTexture.extents().cy;
				}
				if (placementRect.width() > client_rect().width())
				{
					placementRect.cx = client_rect().width();
					placementRect.cy = placementRect.cx * iTexture.extents().cy / iTexture.extents().cx;
				}
				break;
			case aspect_ratio::KeepExpanding:
				if (placementRect.width() > client_rect().width())
				{
					placementRect.cx = client_rect().width();
					placementRect.cy = placementRect.cx * iTexture.extents().cy / iTexture.extents().cx;
				}
				break;
			}
		}
		switch (iPlacement)
		{
		case cardinal_placement::NorthWest:
			placementRect.position() = point{};
			break;
		case cardinal_placement::North:
			placementRect.position() = point{ std::floor((client_rect().width() - placementRect.cx) / 2.0), 0.0 };
			break;
		case cardinal_placement::NorthEast:
			placementRect.position() = point{ client_rect().width() - placementRect.width(), 0.0 };
			break;
		case cardinal_placement::West:
			placementRect.position() = point{ 0.0, std::floor((client_rect().height() - placementRect.cy) / 2.0) };
			break;
		case cardinal_placement::Centre:
			placementRect.position() = point{ std::floor((client_rect().width() - placementRect.cx) / 2.0), std::floor((client_rect().height() - placementRect.cy) / 2.0) };
			break;
		case cardinal_placement::East:
			placementRect.position() = point{ client_rect().width() - placementRect.width(), std::floor((client_rect().height() - placementRect.cy) / 2.0) };
			break;
		case cardinal_placement::SouthWest:
			placementRect.position() = point{ 0.0, client_rect().height() - placementRect.height() };
			break;
		case cardinal_placement::South:
			placementRect.position() = point{ std::floor((client_rect().width() - placementRect.cx) / 2.0), client_rect().height() - placementRect.height() };
			break;
		case cardinal_placement::SouthEast:
			placementRect.position() = point{ client_rect().width() - placementRect.width(), client_rect().height() - placementRect.height() };
			break;
		}
		aGraphicsContext.draw_texture(placementRect, iTexture);
	}

	void image_widget::set_image(const i_texture& aTexture)
	{
		size oldSize = minimum_size();
		iTexture = aTexture;
		if (oldSize != minimum_size() && has_managing_layout())
			managing_layout().layout_items(true);
		update();
	}

	void image_widget::set_image(const i_image& aImage)
	{
		size oldSize = minimum_size();
		iTexture = aImage;
		if (oldSize != minimum_size() && has_managing_layout())
			managing_layout().layout_items(true);
		update();
	}

	void image_widget::set_aspect_ratio(aspect_ratio aAspectRatio)
	{
		if (iAspectRatio != aAspectRatio)
		{
			iAspectRatio = aAspectRatio;
			update();
		}
	}

	void image_widget::set_placement(cardinal_placement aPlacement)
	{
		if (iPlacement != aPlacement)
		{
			iPlacement = aPlacement;
			update();
		}
	}
}