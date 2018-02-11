// texture_manager.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.
  
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/texture_manager.hpp>
#include <neogfx/gfx/texture_atlas.hpp>
#include "native/i_native_texture.hpp"

namespace neogfx
{
	class texture_wrapper : public i_native_texture
	{
	public:
		texture_wrapper(texture_manager::texture_list::iterator aTexture) :
			iTexture(aTexture->lock())
		{
		}
		~texture_wrapper()
		{
		}
	public:
		virtual texture_sampling sampling() const
		{
			return iTexture->sampling();
		}
		virtual size extents() const
		{
			return iTexture->extents();
		}
		virtual size storage_extents() const
		{
			return iTexture->storage_extents();
		}
		virtual void set_pixels(const rect& aRect, const void* aPixelData)
		{
			iTexture->set_pixels(aRect, aPixelData);
		}
	public:
		virtual void* handle() const
		{
			return iTexture->handle();
		}
		virtual bool is_resident() const
		{
			return iTexture->is_resident();
		}
		virtual const std::string& uri() const
		{
			return iTexture->uri();
		}
	private:
		std::shared_ptr<i_native_texture> iTexture;
	};

	std::unique_ptr<i_native_texture> texture_manager::join_texture(const i_native_texture& aTexture)
	{
		for (auto i = iTextures.begin(); i != iTextures.end(); ++i)
		{
			if (i->expired())
				continue;
			auto p = i->lock();
			if (aTexture.handle() == p->handle())
				return std::make_unique<texture_wrapper>(i);
		}
		throw texture_not_found();
	}

	std::unique_ptr<i_native_texture> texture_manager::join_texture(const i_texture& aTexture)
	{
		return join_texture(*aTexture.native_texture());
	}

	void texture_manager::clear_textures()
	{
		iTextures.clear();
	}

	std::unique_ptr<i_texture_atlas> texture_manager::create_texture_atlas(const size& aSize)
	{
		return std::make_unique<texture_atlas>(*this, aSize);
	}

	const texture_manager::texture_list& texture_manager::textures() const
	{
		return iTextures;
	}

	texture_manager::texture_list& texture_manager::textures()
	{
		return iTextures;
	}

	texture_manager::texture_list::const_iterator texture_manager::find_texture(const i_image& aImage) const
	{
		for (auto i = iTextures.begin(); i != iTextures.end(); ++i)
		{
			auto p = i->lock();
			if (!aImage.uri().empty() && aImage.uri() == p->uri())
				return i;
		}
		return iTextures.end();
	}

	texture_manager::texture_list::iterator texture_manager::find_texture(const i_image& aImage)
	{
		for (auto i = iTextures.begin(); i != iTextures.end(); ++i)
		{
			if (i->expired())
				continue;
			auto p = i->lock();
			if (!aImage.uri().empty() && aImage.uri() == p->uri())
				return i;
		}
		return iTextures.end();
	}

	std::unique_ptr<i_native_texture> texture_manager::add_texture(std::shared_ptr<i_native_texture> aTexture)
	{
		// cleanup opportunity
		for (auto i = iTextures.begin(); i != iTextures.end();)
		{
			if (i->expired())
				i = iTextures.erase(i);
			else
				++i;
		}
		auto newTexture = iTextures.insert(iTextures.end(), aTexture);
		return std::make_unique<texture_wrapper>(newTexture);
	}
}