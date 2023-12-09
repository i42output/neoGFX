// texture_manager.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gfx/i_rendering_engine.hpp>
#include <neogfx/gfx/texture_manager.hpp>
#include <neogfx/gfx/texture_atlas.hpp>
#include "native/i_native_texture.hpp"

template <>
neogfx::i_texture_manager& services::start_service<neogfx::i_texture_manager>()
{
    return service<neogfx::i_rendering_engine>().texture_manager();
}

namespace neogfx
{
    texture_id texture_manager::allocate_texture_id()
    {
        return textures().next_cookie();
    }

    void texture_manager::find_texture(texture_id aId, i_ref_ptr<i_texture>& aResult) const
    {
        aResult = textures()[aId];
    }

    void texture_manager::clear_textures()
    {
        textures().clear();
    }

    void texture_manager::add_ref(texture_id aId)
    {
        auto const& texture = *textures()[aId];
        texture.add_ref();
    }

    void texture_manager::release(texture_id aId)
    {
        auto const& texturePtr = textures()[aId];
        auto const& texture = *texturePtr;
        texture.release();
        if (texturePtr.unique())
            textures().remove(aId);
    }

    long texture_manager::use_count(texture_id aId) const
    {
        if (!textures().contains(aId))
            return 0;
        auto const& texture = *textures()[aId];
        return texture.use_count();
    }

    std::unique_ptr<i_texture_atlas> texture_manager::create_texture_atlas(size const& aSize)
    {
        return std::make_unique<texture_atlas>(aSize);
    }

    void texture_manager::add_sub_texture(i_sub_texture& aSubTexture)
    {
        add_ref(aSubTexture.atlas_texture().id());
        textures().add(aSubTexture.id(), &aSubTexture);
    }

    const texture_manager::texture_list& texture_manager::textures() const
    {
        return iTextures;
    }

    texture_manager::texture_list& texture_manager::textures()
    {
        return iTextures;
    }

    texture_manager::texture_list::const_iterator texture_manager::find_texture(i_image const& aImage, rect const& aImagePart) const
    {
        if (aImage.uri().empty())
            return textures().end();
        for (auto i = textures().begin(); i != textures().end(); ++i)
        {
            auto& texture = **i;
            if (texture.type() != texture_type::Texture)   
                continue;
            auto const& textureUri = texture.native_texture().uri();
            if (aImage.uri() == textureUri && aImagePart == texture.part() && aImage.sampling() == texture.sampling())
                return i;
        }
        return textures().end();
    }

    texture_manager::texture_list::iterator texture_manager::find_texture(i_image const& aImage, rect const& aImagePart)
    {
        if (aImage.uri().empty())
            return textures().end();
        for (auto i = textures().begin(); i != textures().end(); ++i)
        {
            auto& texture = **i;
            if (texture.type() != texture_type::Texture)
                continue;
            auto const& textureUri = texture.native_texture().uri();
            if (aImage.uri() == textureUri && aImagePart == texture.part() && aImage.sampling() == texture.sampling())
                return i;
        }
        return textures().end();
    }

    ref_ptr<i_texture> texture_manager::add_texture(i_ref_ptr<i_native_texture> const& aTexture)
    {
        // cleanup opportunity
        cleanup();
        return *textures().add(aTexture->id(), texture_pointer{ aTexture });
    }

    void texture_manager::cleanup()
    {
        for (auto i = textures().begin(); i != textures().end();)
        {
            auto& texturePtr = *i;
            auto& texture = *texturePtr;
            if (texture.type() == texture_type::Texture && texturePtr.unique())
                i = textures().erase(i);
            else
                ++i;
        }
    }
}