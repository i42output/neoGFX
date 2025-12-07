// i_resource_manager.hpp
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

#include <neogfx/app/i_resource.hpp>

namespace neogfx
{
    struct embedded_resource_not_found : std::runtime_error { embedded_resource_not_found(std::string const& aResource) : std::runtime_error{ "neogfx::embedded_resource_not_found: " + aResource } {} };

    class i_resource_manager : public i_service
    {
    public:
        virtual void merge(i_resource_manager& aResourceManager) = 0;
    public:
        virtual void add_resource(i_string const& aUri, const void* aResourceData, std::size_t aResourceSize) = 0;
        virtual void add_module_resource(i_string const& aUri, const void* aResourceData, std::size_t aResourceSize) = 0;
        virtual void load_resource(i_string const& aUri, i_ref_ptr<i_resource>& aResult) = 0;
    public:
        virtual void cleanup() = 0;
        virtual void clean() = 0;
    public:
        virtual neolib::i_map<i_string, neolib::i_variant<i_ref_ptr<i_resource>, i_weak_ref_ptr<i_resource>>> const& resources() = 0;
        virtual neolib::i_map<i_string, neolib::i_variant<i_ref_ptr<i_resource>, i_weak_ref_ptr<i_resource>>> const& resource_archives() = 0;
    public:
        void add_resource(std::string const& aUri, const void* aResourceData, std::size_t aResourceSize)
        {
            add_resource(string{ aUri }, aResourceData, aResourceSize);
        }
        void add_module_resource(std::string const& aUri, const void* aResourceData, std::size_t aResourceSize)
        {
            add_module_resource(string{ aUri }, aResourceData, aResourceSize);
        }
        ref_ptr<i_resource> load_resource(i_string const& aUri)
        {
            ref_ptr<i_resource> result;
            load_resource(aUri, result);
            return result;
        }
        ref_ptr<i_resource> load_resource(std::string const& aUri)
        {
            ref_ptr<i_resource> result;
            load_resource(string{ aUri }, result);
            return result;
        }
    public:
        static uuid const& iid() { static uuid const sIid{ 0xe5f11ade, 0x7596, 0x4179, 0x8d77, { 0x1e, 0xb9, 0x9d, 0x6f, 0x3b, 0x96 } }; return sIid; }
    };
}