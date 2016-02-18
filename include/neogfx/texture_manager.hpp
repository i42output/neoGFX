// texture_manager.hpp
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
#include "i_texture_manager.hpp"

namespace neogfx
{
	class texture_manager : public i_texture_manager
	{
		friend class texture_wrapper;
	private:
		typedef std::list<std::weak_ptr<i_native_texture>> texture_list;
	public:
		virtual std::unique_ptr<i_native_texture> join_texture(const i_texture& aTexture);
	protected:
		std::unique_ptr<i_native_texture> add_texture(std::shared_ptr<i_native_texture> aTexture);
	private:
		void cleanup(texture_list::iterator aTexture);
	private:
		texture_list iTextures;
	};
}