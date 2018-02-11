// resource_manager.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/variant.hpp>
#include "i_resource_manager.hpp"

namespace neogfx
{
	class resource_manager : public i_resource_manager
	{
	public:
		resource_manager();
		static resource_manager& instance();
	public:
		virtual void add_resource(const std::string& aUri, const void* aResourceData, std::size_t aResourceSize);
		virtual void add_module_resource(const std::string& aUri, const void* aResourceData, std::size_t aResourceSize);
		virtual i_resource::pointer load_resource(const std::string& aUri);
	public:
		virtual void cleanup();
		virtual void clean();
	private:
		std::map<std::string, neolib::variant<i_resource::pointer, i_resource::weak_pointer>> iResources;
		std::map<std::string, neolib::variant<i_resource::pointer, i_resource::weak_pointer>> iResourceArchives;
	};
}