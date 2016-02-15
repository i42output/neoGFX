// module_resource.hpp
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
#include "i_resource.hpp"

namespace neogfx
{
	class module_resource : public i_resource
	{
	public:
		module_resource(const std::string& aPath, const void* aData, std::size_t aSize);
	public:
		virtual const std::string& path() const;
		virtual const void* data() const;
		virtual std::size_t size() const;
	private:
		std::string iPath;
		const void* iData;
		std::size_t iSize;
	};
}