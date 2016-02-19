// image_widget.hpp
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
#include "image.hpp"
#include "texture.hpp"

namespace neogfx
{
	enum class aspect_ratio
	{
		Ignore,
		Stretch,
		Keep,
		KeepExpanding
	};

	enum class cardinal_placement
	{
		NorthWest,
		North,
		NorthEast,
		West,
		Centre,
		East,
		SouthWest,
		South,
		SouthEast
	};

	class image_widget : public widget
	{
	public:
		image_widget(const i_texture& aTexture, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal_placement aPlacement = cardinal_placement::Centre);
		image_widget(const i_image& aImage, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal_placement aPlacement = cardinal_placement::Centre);
		image_widget(i_widget& aParent, const i_texture& aTexture, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal_placement aPlacement = cardinal_placement::Centre);
		image_widget(i_widget& aParent, const i_image& aImage, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal_placement aPlacement = cardinal_placement::Centre);
		image_widget(i_layout& aLayout, const i_texture& aTexture, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal_placement aPlacement = cardinal_placement::Centre);
		image_widget(i_layout& aLayout, const i_image& aImage, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal_placement aPlacement = cardinal_placement::Centre);
	public:
		virtual size minimum_size() const;
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
	public:
		void set_image(const i_texture& aTexture);
		void set_image(const i_image& aText);
		void set_aspect_ratio(aspect_ratio aAspectRatio);
		void set_placement(cardinal_placement aPlacement);
	private:
		texture iTexture;
		aspect_ratio iAspectRatio;
		cardinal_placement iPlacement;
	};
}