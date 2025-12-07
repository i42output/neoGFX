// resource.cpp
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

#include <sstream>
#include <filesystem>
#include <openssl/sha.h>

#include <neolib/io/uri.hpp>
#include <neolib/file/zip.hpp>
#include <neogfx/app/resource.hpp>

namespace neogfx
{
    resource::resource(i_resource_manager& aManager, string const& aUri) : 
        iManager{aManager}, iUri{aUri}, iSize{0}
    {
        neolib::uri uri{ aUri.to_std_string() };
        if (uri.scheme() == "file")
        {
            if (uri.fragment().empty()) // individual asset file
            { 
                iData.resize(static_cast<std::size_t>(std::filesystem::file_size(uri.path())));
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
                        archive.extract_to(i, iData.as_std_vector());
                        iSize = iData.size();
                    }
                }
            }
        }
        else if (uri.scheme().empty())
        {
            if (!uri.fragment().empty()) // asset archive
            {
                auto assetArchive = aManager.load_resource(string{ ":/" + uri.path() });
                neolib::zip archive{assetArchive->cdata(), assetArchive->size()};
                for (std::size_t i = 0; i < archive.file_count(); ++i)
                {
                    if (archive.file_path(i) == uri.fragment())
                    {
                        archive.extract_to(i, iData.as_std_vector());
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
                        aManager.add_resource(string{ oss.str() }, & buffer[0], buffer.size());
                    }
                }
            }
        }
    }

    resource::resource(i_resource_manager& aManager, string const& aUri, const void* aData, std::size_t aSize) : 
        iManager{aManager}, iUri{aUri}, iSize{aSize}, iData{reinterpret_cast<const std::uint8_t*>(aData), reinterpret_cast<const std::uint8_t*>(aData) + aSize}
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

    bool resource::downloading() const
    {
        if (iSize == 0)
            return false;
        else if (iData.size() != iSize)
            return true;
        else
            return false;
    }

    double resource::downloading_progress() const
    {
        if (iSize == 0)
            return 0.0;
        else if (iData.size() != iSize)
            return 100.0 * iData.size() / iSize;
        else
            return 100.0;
    }

    bool resource::error() const
    {
        return iError != std::nullopt;
    }

    i_string const& resource::error_string() const
    {
        if (iError != std::nullopt)
            return *iError;
        static const string sNoError;
        return sNoError;
    }

    i_string const& resource::uri() const
    {
        return iUri;
    }

    bool resource::is_empty() const
    {
        return iData.empty();
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
        return const_cast<void*>(to_const(*this).data());
    }

    std::size_t resource::size() const
    {
        return iData.size();
    }

    resource::hash_digest_type const& resource::hash() const
    {
        if (!iHash)
        {
            hash_digest_type::std_type result{ SHA256_DIGEST_LENGTH };
            SHA256(static_cast<const std::uint8_t*>(cdata()), size(), &result[0]);
            iHash = result;
        }
        return *iHash;
    }
}