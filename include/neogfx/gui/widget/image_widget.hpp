// image_widget.hpp
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
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gfx/image.hpp>
#include <neogfx/gfx/texture.hpp>

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
		event<> image_changed;
	public:
		image_widget(const i_texture& aTexture, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal_placement aPlacement = cardinal_placement::Centre);
		image_widget(const i_image& aImage, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal_placement aPlacement = cardinal_placement::Centre);
		image_widget(i_widget& aParent, const i_texture& aTexture, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal_placement aPlacement = cardinal_placement::Centre);
		image_widget(i_widget& aParent, const i_image& aImage, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal_placement aPlacement = cardinal_placement::Centre);
		image_widget(i_layout& aLayout, const i_texture& aTexture, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal_placement aPlacement = cardinal_placement::Centre);
		image_widget(i_layout& aLayout, const i_image& aImage, aspect_ratio aAspectRatio = aspect_ratio::Keep, cardinal_placement aPlacement = cardinal_placement::Centre);
	public:
		virtual neogfx::size_policy size_policy() const;
		virtual size minimum_size(const optional_size& aAvailableSpace = optional_size()) const;
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
	public:
		const texture& image() const;
		void set_image(const i_texture& aImage);
		void set_image(const i_image& aImage);
		void set_aspect_ratio(aspect_ratio aAspectRatio);
		void set_placement(cardinal_placement aPlacement);
		void set_snap(dimension aSnap);
	private:
		texture iTexture;
		aspect_ratio iAspectRatio;
		cardinal_placement iPlacement;
		dimension iSnap;
	};
}