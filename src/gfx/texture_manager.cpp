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
    neolib::cookie item_cookie(const texture_manager::texture_list_entry& aEntry)
    {
        return aEntry.first()->id();
    }

    texture_id texture_manager::allocate_texture_id()
    {
        return textures().next_cookie();
    }

    void texture_manager::find_texture(texture_id aId, i_ref_ptr<i_texture>& aResult) const
    {
        aResult = textures()[aId].first();
    }

    void texture_manager::clear_textures()
    {
        textures().clear();
    }

    void texture_manager::add_ref(texture_id aId)
    {
        ++textures()[aId].second();
    }

    void texture_manager::release(texture_id aId)
    {
        if (textures()[aId].second() == 0u)
            throw invalid_release();
        if (--textures()[aId].second() == 0u)
        {
            if (textures()[aId].first().use_count() == 1)
                textures().remove(aId);
        }
    }

    long texture_manager::use_count(texture_id aId) const
    {
        return textures()[aId].second();
    }

    std::unique_ptr<i_texture_atlas> texture_manager::create_texture_atlas(size const& aSize)
    {
        return std::make_unique<texture_atlas>(aSize);
    }

    void texture_manager::add_sub_texture(i_sub_texture& aSubTexture)
    {
        textures().add(aSubTexture.id(), texture_list_entry{ texture_pointer{ texture_pointer{}, &aSubTexture }, 0u });
    }

    void texture_manager::remove_sub_texture(i_sub_texture& aSubTexture)
    {
        textures().remove(aSubTexture.id());
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
            auto& texture = *i;
            if (texture.first()->type() != texture_type::Texture)   
                continue;
            auto const& textureUri = texture.first()->native_texture().uri();
            if (aImage.uri() == textureUri && aImagePart == texture.first()->part() && aImage.sampling() == texture.first()->sampling())
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
            auto& texture = *i;
            if (texture.first()->type() != texture_type::Texture)
                continue;
            auto const& textureUri = texture.first()->native_texture().uri();
            if (aImage.uri() == textureUri && aImagePart == texture.first()->part() && aImage.sampling() == texture.first()->sampling())
                return i;
        }
        return textures().end();
    }

    ref_ptr<i_texture> texture_manager::add_texture(i_ref_ptr<i_native_texture> const& aTexture)
    {
        // cleanup opportunity
        cleanup();
        return textures().add(aTexture->id(), texture_list_entry{ aTexture, 0u })->first();
    }

    void texture_manager::cleanup()
    {
        for (auto i = textures().begin(); i != textures().end();)
        {
            auto& texture = *i;
            if (texture.first()->type() == texture_type::Texture && texture.first().use_count() == 1 && texture.second() == 0u)
                i = textures().erase(i);
            else
                ++i;
        }
    }
}