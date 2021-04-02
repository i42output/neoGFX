// module_resource.cpp
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
#include <openssl/sha.h>
#include <neogfx/app/module_resource.hpp>

namespace neogfx
{
    module_resource::module_resource(std::string const& aUri, const void* aData, std::size_t aSize) : 
        iUri(aUri), iData(aData), iSize(aSize)
    {
    }

    bool module_resource::available() const
    {
        return true;
    }

    bool module_resource::downloading() const
    {
        return false;
    }

    double module_resource::downloading_progress() const
    {
        return 100.0;
    }

    bool module_resource::error() const
    {
        return false;
    }

    i_string const& module_resource::error_string() const
    {
        static const string sNoError;
        return sNoError;
    }

    i_string const& module_resource::uri() const
    {
        return iUri;
    }

    const void* module_resource::cdata() const
    {
        return iData;
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

    module_resource::hash_digest_type const& module_resource::hash() const
    {
        if (!iHash)
        {
            hash_digest_type::std_type result{ SHA256_DIGEST_LENGTH };
            SHA256(static_cast<const uint8_t*>(cdata()), size(), &result[0]);
            iHash = result;
        }
        return *iHash;
    }
}