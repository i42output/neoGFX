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
	image_widget::image_widget(const i_texture& aTexture) :
		iTexture(aTexture)
	{
	}

	image_widget::image_widget(const i_image& aImage) :
		iTexture(aImage)
	{
	}

	image_widget::image_widget(i_widget& aParent, const i_texture& aTexture) : 
		widget(aParent), iTexture(aTexture)
	{
	}

	image_widget::image_widget(i_widget& aParent, const i_image& aImage) :
		widget(aParent), iTexture(aImage)
	{
	}

	image_widget::image_widget(i_layout& aLayout, const i_texture& aTexture) :
		widget(aLayout), iTexture(aTexture)
	{
	}

	image_widget::image_widget(i_layout& aLayout, const i_image& aImage) :
		widget(aLayout), iTexture(aImage)
	{
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
		point imagePosition(std::floor((client_rect().width() - iTexture.extents().cx) / 2.0), std::floor((client_rect().height() - iTexture.extents().cy) / 2.0));
		aGraphicsContext.draw_texture(imagePosition, iTexture);
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
}