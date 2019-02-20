// resource.hpp
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
#include <optional>
#include "i_resource.hpp"
#include "i_resource_manager.hpp"

namespace neogfx
{
    class resource : public i_resource
    {
    public:
        resource() = delete;
        resource(i_resource_manager& aManager, const std::string& aUri);
        resource(i_resource_manager& aManager, const std::string& aUri, const void* aData, std::size_t aSize);
        ~resource();
    public:
        virtual bool available() const;
        virtual std::pair<bool, double> downloading() const;
        virtual bool error() const;
        virtual const std::string& error_string() const;
    public:
        virtual const std::string& uri() const;
        virtual const void* cdata() const;
        virtual const void* data() const;
        virtual void* data();
        virtual std::size_t size() const;
        virtual hash_digest_type hash() const;
    private:
        i_resource_manager& iManager;
        std::string iUri;
        std::optional<std::string> iError;
        std::size_t iSize;
        std::vector<uint8_t> iData;
    };
}