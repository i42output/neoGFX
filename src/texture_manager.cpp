// texture_manager.cpp
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
#include "texture_manager.hpp"

namespace neogfx
{
	class texture_wrapper : public i_native_texture
	{
	public:
		texture_wrapper(texture_manager& aParent, texture_manager::texture_list::iterator aTexture) :
			iParent(aParent), iTexture(aTexture), iTextureReference(aTexture->lock())
		{
		}
		~texture_wrapper()
		{
			iTextureReference.reset();
			iParent.cleanup(iTexture);
		}
	public:
		virtual size extents() const
		{
			return iTextureReference->extents();
		}
		virtual size storage_extents() const
		{
			return iTextureReference->storage_extents();
		}
	public:
		virtual void* handle() const
		{
			return iTextureReference->handle();
		}
		virtual const std::string& uri() const
		{
			return iTextureReference->uri();
		}
	private:
		texture_manager& iParent;
		texture_manager::texture_list::iterator iTexture;
		std::shared_ptr<i_native_texture> iTextureReference;
	};

	std::unique_ptr<i_native_texture> texture_manager::join_texture(const i_native_texture& aTexture)
	{
		for (auto i = iTextures.begin(); i != iTextures.end(); ++i)
		{
			auto p = i->lock();
			if (aTexture.handle() == p->handle())
				return std::make_unique<texture_wrapper>(*this, i);
		}
		throw texture_not_found();
	}

	std::unique_ptr<i_native_texture> texture_manager::join_texture(const i_texture& aTexture)
	{
		return join_texture(*aTexture.native_texture());
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
			auto p = i->lock();
			if (!aImage.uri().empty() && aImage.uri() == p->uri())
				return i;
		}
		return iTextures.end();
	}

	std::unique_ptr<i_native_texture> texture_manager::add_texture(std::shared_ptr<i_native_texture> aTexture)
	{
		auto newTexture = iTextures.insert(iTextures.end(), aTexture);
		return std::make_unique<texture_wrapper>(*this, newTexture);
	}

	void texture_manager::cleanup(texture_list::iterator aTexture)
	{
		if (aTexture->expired())
			iTextures.erase(aTexture);
	}
}