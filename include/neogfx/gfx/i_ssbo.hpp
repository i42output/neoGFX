// i_shader_program.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2019, 2020, 2024 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/gfx/i_shader.hpp>

namespace neogfx
{
    struct ssbo_locked : std::logic_error { ssbo_locked() : std::logic_error{ "neogfx::ssbo_locked" } {} };

    using ssbo_id = neolib::cookie;
    constexpr ssbo_id no_sbbo = ssbo_id{};

    struct ssbo_range
    {
        std::uint32_t first;
        std::uint32_t last;
    };

    class i_ssbo : public i_reference_counted
    {
    public:
        using abstract_type = i_ssbo;
    public:
        virtual ~i_ssbo() = default;
    public:
        virtual i_string const& name() const = 0;
        virtual ssbo_id id() const = 0;
        virtual shader_data_type data_type() const = 0;
    public:
        virtual ssbo_range alloc(std::uint32_t aSize) = 0;
        virtual void free(ssbo_range aRange) = 0;
        virtual void* lock(ssbo_range aRange) = 0;
        virtual void unlock(ssbo_range aRange) = 0;
    public:
        template <typename T>
        i_ssbo& as()
        {
            if (data_type() != shader_data_type_v<T>)
                throw std::logic_error("neogfx::i_ssbo::as: invalid data type");
            return *this;
        }
    public:
        template <typename T>
        T* lock(ssbo_range aRange)
        {
            return static_cast<T*>(as<T>().lock(aRange));
        }
    };

    template <typename T>
    class scoped_lock_ssbo
    {
    public:
        scoped_lock_ssbo(i_ssbo& aSsbo, ssbo_range aRange) :
            iSsbo{ aSsbo.as<T>() }, iRange{ aRange }, iData{ aSsbo.lock<T>(aRange) }
        {
        }
        scoped_lock_ssbo(i_ssbo& aSsbo, std::uint32_t aAllocSize) :
            iSsbo{ aSsbo.as<T>() }, iRange{ aSsbo.alloc(aAllocSize) }, iData{ aSsbo.lock<T>(iRange) }
        {
        }
        ~scoped_lock_ssbo()
        {
            iSsbo.unlock(iRange);
        }
    public:
        ssbo_range range() const
        {
            return iRange;
        }
        T* data() const
        {
            return iData;
        }
    private:
        i_ssbo& iSsbo;
        ssbo_range iRange;
        T* iData;
    };
}
 