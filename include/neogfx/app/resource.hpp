// resource.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <optional>
#include <neogfx/core/event.hpp>
#include <neogfx/app/i_resource.hpp>
#include <neogfx/app/i_resource_manager.hpp>

namespace neogfx
{
    class resource : public reference_counted<i_resource>
    {
    public:
        define_declared_event(Downloaded, downloaded)
        define_declared_event(FailedToDownload, failed_to_download)
    public:
        typedef neolib::vector<std::uint8_t> data_type;
        typedef data_type hash_digest_type;
    public:
        resource() = delete;
        resource(i_resource_manager& aManager, std::string const& aUri);
        resource(i_resource_manager& aManager, std::string const& aUri, const void* aData, std::size_t aSize);
        ~resource();
    public:
        bool available() const override;
        bool downloading() const override;
        double downloading_progress() const override;
        bool error() const override;
        i_string const& error_string() const override;
    public:
        i_string const& uri() const override;
        bool is_empty() const override;
        const void* cdata() const override;
        const void* data() const override;
        void* data() override;
        std::size_t size() const override;
        hash_digest_type const& hash() const override;
    private:
        i_resource_manager& iManager;
        string iUri;
        std::optional<string> iError;
        std::size_t iSize;
        data_type iData;
        mutable std::optional<data_type> iHash;
    };
}