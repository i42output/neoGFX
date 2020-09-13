// neogfx.hpp
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

#include <neolib/neolib.hpp>
#include <string>
#include <boost/multiprecision/cpp_int.hpp>
using namespace boost::multiprecision;

#include <neolib/core/stdint.hpp>
#include <neolib/core/uuid.hpp>
#include <neolib/core/lifetime.hpp>
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/enum.hpp>
#include <neolib/core/string.hpp>
#include <neolib/core/string_utils.hpp>
#include <neolib/task/i_async_task.hpp>
#include <neolib/app/services.hpp>
#include <neogfx/app/i18n.hpp>

namespace services = neolib::services;

namespace neogfx
{
    using namespace neolib::stdint_suffix;
    using namespace std::string_literals;

    using neolib::sfinae;

    using neolib::to_const;

    using neolib::destroying_flag;
    using neolib::destroyed_flag;

    using neolib::i_reference_counted;
    using neolib::reference_counted;
    using neolib::i_ref_ptr;
    using neolib::ref_ptr;
    using neolib::i_weak_ref_ptr;
    using neolib::weak_ref_ptr;
    using neolib::make_ref;

    using neolib::to_abstract;
    using neolib::abstract_t;

    using neolib::i_enum_t;
    using neolib::enum_t;
    using neolib::enum_to_string;

    using neolib::i_string;
    using neolib::string;
    using neolib::to_string;
    using namespace neolib::string_literals;

    using neolib::uuid;

    using neolib::i_async_task;

    using namespace neolib::services;

    using neolib::operator<<;
    using neolib::operator>>;

    // convert strings with different traits and/or character types to std::string
    template <typename CharT, typename Traits, typename Allocator>
    inline const std::string to_string(const std::basic_string<CharT, Traits, Allocator>& aString)
    {
        static_assert(sizeof(CharT) == sizeof(char));
        return std::string{ reinterpret_cast<const char*>(aString.c_str()), aString.size() };
    }

    // convert character array to std::string
    template <typename CharT, std::size_t Size>
    inline const std::string to_string(const CharT (&aString)[Size])
    {
        static_assert(sizeof(CharT) == sizeof(char));
        return std::string{ reinterpret_cast<const char*>(&aString[0]), Size };
    }

    struct not_yet_implemented : std::runtime_error
    {
        not_yet_implemented(const std::string& aDetail = {}) :
            std::runtime_error{ "neoGFX: Functionality not yet implemented" + (aDetail.empty() ? "" : " (" + aDetail + ")") } {}
    };

    class i_layout_item;
    extern i_layout_item* debug;
}