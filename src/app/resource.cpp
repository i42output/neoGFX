// resource.cpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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
#include <sstream>
#include <boost/filesystem.hpp>
#include <openssl/sha.h>
#include <neolib/uri.hpp>
#include <neolib/zip.hpp>
#include <neogfx/app/resource.hpp>

namespace neogfx
{
	resource::resource(i_resource_manager& aManager, const std::string& aUri) : 
		iManager{aManager}, iUri{aUri}, iSize{0}
	{
		neolib::uri uri{aUri};
		if (uri.scheme() == "file")
		{
			if (uri.fragment().empty()) // individual asset file
			{ 
				iData.resize(static_cast<std::size_t>(boost::filesystem::file_size(uri.path())));
				std::ifstream input(uri.path(), std::ios::binary | std::ios::in);
				input.read(reinterpret_cast<char*>(data()), iData.size());
				iSize = iData.size();
			}
			else // asset archive
			{
				neolib::zip archive{uri.path()};
				for (std::size_t i = 0; i < archive.file_count(); ++i)
				{
					if (archive.file_path(i) == uri.fragment())
					{
						archive.extract_to(i, iData);
						iSize = iData.size();
					}
				}
			}
		}
		else if (uri.scheme().empty())
		{
			if (!uri.fragment().empty()) // asset archive
			{
				auto assetArchive = aManager.load_resource(":/" + uri.path());
				neolib::zip archive{assetArchive->cdata(), assetArchive->size()};
				for (std::size_t i = 0; i < archive.file_count(); ++i)
				{
					if (archive.file_path(i) == uri.fragment())
					{
						archive.extract_to(i, iData);
						iSize = iData.size();
					}
					else
					{
						neolib::uri otherResource(uri);
						otherResource.set_fragment(archive.file_path(i));
						std::ostringstream oss;
						oss << otherResource;
						neolib::zip::buffer_type buffer;
						archive.extract_to(i, buffer);
						aManager.add_resource(oss.str(), &buffer[0], buffer.size());
					}
				}
			}
		}
	}

	resource::resource(i_resource_manager& aManager, const std::string& aUri, const void* aData, std::size_t aSize) : 
		iManager{aManager}, iUri{aUri}, iSize{aSize}, iData{reinterpret_cast<const uint8_t*>(aData), reinterpret_cast<const uint8_t*>(aData) + aSize}
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

	const std::string& resource::uri() const
	{
		return iUri;
	}
	
	const void* resource::cdata() const
	{
		if (iData.empty())
			throw no_data();
		return &iData[0];
	}

	const void* resource::data() const
	{
		return cdata();
	}
	
	void* resource::data()
	{
		return const_cast<void*>(const_cast<const resource*>(this)->data());
	}

	std::size_t resource::size() const
	{
		return iData.size();
	}

	resource::hash_digest_type resource::hash() const
	{
		hash_digest_type result{ SHA256_DIGEST_LENGTH };
		SHA256(static_cast<const uint8_t*>(cdata()), size(), &result[0]);
		return result;
	}
}