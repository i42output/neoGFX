// sub_texture.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
#include <boost/optional.hpp>
#include <neogfx/gfx/i_image.hpp>
#include <neogfx/gfx/i_sub_texture.hpp>

namespace neogfx
{
	class sub_texture : public i_sub_texture
	{
		// construction
	public:
		sub_texture(id aAtlasId, i_texture& aAtlasTexture, const rect& aAtlasLocation, const size& aExtents);
		sub_texture(const i_sub_texture& aSubTexture);
		~sub_texture();
		// operations
	public:
		virtual type_e type() const;
		virtual texture_sampling sampling() const;
		virtual bool is_empty() const;
		virtual size extents() const;
		virtual size storage_extents() const;
		virtual void set_pixels(const rect& aRect, const void* aPixelData);
		virtual void set_pixels(const i_image& aImage);
	public:
		virtual std::shared_ptr<i_native_texture> native_texture() const;
	public:
		virtual id atlas_id() const;
		virtual i_texture& atlas_texture() const;
		virtual const rect& atlas_location() const;
		// attributes
	private:
		id iAtlasId;
		i_texture* iAtlasTexture;
		rect iAtlasLocation;
		size iExtents;
	};

	typedef boost::optional<sub_texture> optional_sub_texture;
}