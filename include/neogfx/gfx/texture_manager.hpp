// texture_manager.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <variant>
#include <neolib/cookie_jar.hpp>
#include <neogfx/gfx/i_image.hpp>
#include "i_texture_manager.hpp"

namespace neogfx
{
	class texture_manager : public i_texture_manager
	{
		friend class texture_wrapper;
	protected:
		typedef std::shared_ptr<i_texture> texture_pointer;
		typedef neolib::cookie_jar<texture_pointer> texture_list;
	private:
		friend neolib::cookie item_cookie(const texture_pointer&);
	protected:
		texture_id allocate_texture_id() override;
	public:
		std::shared_ptr<i_texture> find_texture(texture_id aId) const override;
		void clear_textures() override;
	public:
		std::unique_ptr<i_texture_atlas> create_texture_atlas(const size& aSize = size{ 1024.0, 1024.0 }) override;
	private:
		void add_sub_texture(i_sub_texture& aSubTexture) override;
		void remove_sub_texture(i_sub_texture& aSubTexture) override;
	protected:
		const texture_list& textures() const;
		texture_list& textures();
		texture_list::const_iterator find_texture(const i_image& aImage) const;
		texture_list::iterator find_texture(const i_image& aImage);
		std::shared_ptr<i_texture> add_texture(std::shared_ptr<i_native_texture> aTexture);
	private:
		texture_list iTextures;
		std::vector<std::unique_ptr<i_texture_atlas>> iTextureAtlases;
	};

	neolib::cookie item_cookie(const texture_manager::texture_pointer& aEntry);
}