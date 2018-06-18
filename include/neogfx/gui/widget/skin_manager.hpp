// skin_manager.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <unordered_set>
#include <boost/pool/pool_alloc.hpp>
#include <neogfx/gui/widget/i_skin_manager.hpp>

namespace neogfx
{
	class skin_manager : public i_skin_manager
	{
	private:
		typedef std::unordered_set<i_skin*> skin_list;
		typedef std::unordered_set<i_skinnable_item*, boost::hash<i_skinnable_item*>, std::equal_to<i_skinnable_item*>, boost::fast_pool_allocator<i_skinnable_item*>> item_list;
	public:
		skin_manager();
	public:
		uint32_t skin_count() const override;
		const i_skin& skin(uint32_t aIndex) const override;
		i_skin& skin(uint32_t aIndex) override;
		void register_skin(i_skin& aSkin) override;
		void unregister_skin(i_skin& aSkin) override;
	public:
		const i_skin& active_skin() const override;
		i_skin& active_skin() override;
		void activate_skin(i_skin& aSkin) override;
	public:
		bool is_item_registered(const i_skinnable_item& aItem) const override;
		void register_item(i_skinnable_item& aItem) override;
		void unregister_item(i_skinnable_item& aItem) override;
	private:
		skin_list iSkins;
		item_list iItems;
	};
}