// module_resource.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/app/module_resource.hpp>

namespace neogfx
{
	module_resource::module_resource(const std::string& aUri, const void* aData, std::size_t aSize) : 
		iUri(aUri), iData(aData), iSize(aSize)
	{
	}

	bool module_resource::available() const
	{
		return true;
	}

	std::pair<bool, double> module_resource::downloading() const
	{
		return std::make_pair(false, 100.0);
	}

	bool module_resource::error() const
	{
		return false;
	}

	const std::string& module_resource::error_string() const
	{
		static const std::string sNoError;
		return sNoError;
	}

	const std::string& module_resource::uri() const
	{
		return iUri;
	}

	const void* module_resource::data() const
	{
		return iData;
	}

	void* module_resource::data()
	{
		throw const_data();
	}

	std::size_t module_resource::size() const
	{
		return iSize;
	}
}