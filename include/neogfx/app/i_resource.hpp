// i_resource.hpp
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
#include <neogfx/core/i_event.hpp>

namespace neogfx
{
    class i_resource : public i_reference_counted
    {
    public:
        declare_event(downloaded)
        declare_event(failed_to_download)
    public:
        typedef i_resource abstract_type;
    public:
        typedef neolib::i_vector<uint8_t> data_type;
        typedef data_type hash_digest_type;
    public:
        struct not_available : std::logic_error { not_available() : std::logic_error("neogfx::i_resource::not_available") {} };
        struct no_data : std::logic_error { no_data() : std::logic_error("neogfx::i_resource::no_data") {} };
        struct const_data : std::logic_error { const_data() : std::logic_error("neogfx::i_resource::const_data") {} };
    public:
        virtual ~i_resource() = default;
    public:
        virtual bool available() const = 0;
        virtual bool downloading() const = 0;
        virtual double downloading_progress() const = 0;
        virtual bool error() const = 0;
        virtual i_string const& error_string() const = 0;
    public:
        virtual i_string const& uri() const = 0;
        virtual const void* cdata() const = 0;
        virtual const void* data() const = 0;
        virtual void* data() = 0;
        virtual std::size_t size() const = 0;
        virtual hash_digest_type const& hash() const = 0;
    };
}