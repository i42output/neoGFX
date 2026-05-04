// textue_atlas.cpp
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

#include <neogfx/gfx/texture_atlas.hpp>
#include <neogfx/gfx/image.hpp>

namespace neogfx
{
    texture_atlas::texture_atlas(const size& aPageSize) :
        iTextureManager{ service<i_texture_manager>() }, iPageSize{ aPageSize }
    {
    }

    dimension texture_atlas::bleed_guard() const
    {
        return iBleedGuard.value_or(1.0);
    }
    void texture_atlas::set_bleed_guard(i_optional<dimension> const& aWidth)
    {
        if (aWidth.has_value())
            iBleedGuard = aWidth.value();
        else
            iBleedGuard.reset();
    }

    const i_sub_texture& texture_atlas::sub_texture(texture_id aSubTextureId) const
    {
        auto iterEntry = iEntries.find(aSubTextureId);
        if (iterEntry == iEntries.end())
            throw sub_texture_not_found();
        return *iterEntry->second.texture;
    }

    i_sub_texture& texture_atlas::sub_texture(texture_id aSubTextureId)
    {
        auto iterEntry = iEntries.find(aSubTextureId);
        if (iterEntry == iEntries.end())
            throw sub_texture_not_found();
        return *iterEntry->second.texture;
    }

    i_sub_texture& texture_atlas::create_sub_texture(const size& aSize, dimension aDpiScaleFactor, texture_sampling aSampling, texture_data_format aDataFormat)
    {
        auto newSpace = allocate_space(aSize, aDpiScaleFactor, aSampling, aDataFormat);
        auto nextId = iTextureManager.allocate_texture_id();
        auto entry = iEntries.emplace(std::piecewise_construct, std::forward_as_tuple(nextId), std::forward_as_tuple(newSpace.first, nextId, newSpace.first->first, newSpace.second, aSize));
        auto& newTexture = *entry.first->second.texture;
        return newTexture;
    }

    i_sub_texture& texture_atlas::create_sub_texture(const i_image& aImage)
    {
        auto newSpace = allocate_space(aImage.extents(), aImage.dpi_scale_factor(), aImage.sampling(), aImage.data_format());
        auto nextId = iTextureManager.allocate_texture_id();
        auto entry = iEntries.emplace(std::piecewise_construct, std::forward_as_tuple(nextId), std::forward_as_tuple(newSpace.first, aImage.uri(), nextId, newSpace.first->first, newSpace.second, aImage.extents()));
        auto& newTexture = *entry.first->second.texture;
        newTexture.set_pixels(aImage);
        return newTexture;
    }

    i_sub_texture& texture_atlas::create_sub_texture(const i_image& aImage, const rect& aImagePart)
    {
        auto newSpace = allocate_space(aImagePart.extents(), aImage.dpi_scale_factor(), aImage.sampling(), aImage.data_format());
        auto nextId = iTextureManager.allocate_texture_id();
        auto entry = iEntries.emplace(std::piecewise_construct, std::forward_as_tuple(nextId), std::forward_as_tuple(newSpace.first, aImage.uri(), nextId, newSpace.first->first, newSpace.second, aImagePart.extents()));
        auto& newTexture = *entry.first->second.texture;
        newTexture.set_pixels(aImage, aImagePart);
        return newTexture;
    }

    void texture_atlas::destroy_sub_texture(i_sub_texture& aSubTexture)
    {
        auto iterEntry = iEntries.find(aSubTexture.atlas_id());
        if (iterEntry == iEntries.end() || &aSubTexture != &*iterEntry->second.texture)
            throw sub_texture_not_found();
        auto rectEntry = iterEntry->second.texture->atlas_location();
        auto space = iterEntry->second.page->second.used.find(rectEntry);
        if (space != iterEntry->second.page->second.used.end())
            iterEntry->second.page->second.used.erase(space);
        iterEntry->second.page->second.freed.insert(rectEntry);
        iEntries.erase(iterEntry);
    }

    const size& texture_atlas::page_size() const
    {
        return iPageSize;
    }

    texture_atlas::pages::iterator texture_atlas::create_page(dimension aDpiScaleFactor, texture_sampling aSampling, texture_data_format aDataFormat)
    {
        return iPages.insert(iPages.end(), page{ texture{ page_size(), aDpiScaleFactor, aSampling, aDataFormat }, fragments{ page_size() } });
    }

    std::pair<texture_atlas::pages::iterator, rect> texture_atlas::allocate_space(const size& aSize, dimension aDpiScaleFactor, texture_sampling aSampling, texture_data_format aDataFormat)
    {
        if (iPages.empty())
            create_page(aDpiScaleFactor, aSampling, aDataFormat);
        rect result;
        for (auto iterPage = iPages.begin(); iterPage != iPages.end(); ++iterPage)
            if (iterPage->first.dpi_scale_factor() == aDpiScaleFactor && iterPage->first.sampling() == aSampling && iterPage->first.data_format() == aDataFormat && iterPage->second.insert(aSize + size{ bleed_guard() * 2.0 }, result))
                return std::make_pair(iterPage, result + point{ bleed_guard() } + size{ bleed_guard() * -2.0 });
        auto iterPage = create_page(aDpiScaleFactor, aSampling, aDataFormat);
        if (iterPage->second.insert(aSize + size{ bleed_guard() * 2.0 }, result))
            return std::make_pair(iterPage, result + point{ bleed_guard() } + size{ bleed_guard() * -2.0 });
        iPages.erase(iterPage);
        throw texture_too_big_for_atlas();
    }
}