// resource.cpp
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
#include "resource.hpp"

namespace neogfx
{
	resource::resource(i_resource_manager& aManager, const std::string& aPath) : iManager(aManager), iPath(aPath), iSize(0)
	{
	}

	resource::~resource()
	{
		iManager.cleanup();
	}

	bool resource::available() const
	{
		return iSize != 0 && iData.size() == iSize;
	}

	std::pair<bool, double> resource::downloading() const
	{
		if (iSize == 0)
			return std::make_pair(false, 0.0);
		else if (iData.size() != iSize)
			return std::make_pair(true, 100.0 * iData.size() / iSize);
		else
			return std::make_pair(false, 100.0);
	}

	bool resource::error() const
	{
		return iError != boost::none;
	}

	const std::string& resource::error_string() const
	{
		if (iError != boost::none)
			return *iError;
		static const std::string sNoError;
		return sNoError;
	}

	const std::string& resource::path() const
	{
		return iPath;
	}
	
	const void* resource::data() const
	{
		if (iData.empty())
			throw no_data();
		return &iData[0];
	}
	
	void* resource::data()
	{
		return const_cast<void*>(const_cast<const resource*>(this)->data());
	}

	std::size_t resource::size() const
	{
		return iData.size();
	}
}