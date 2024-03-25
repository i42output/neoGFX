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
#include <string_view>
#include <boost/multiprecision/cpp_int.hpp>
using namespace boost::multiprecision;

#include <neolib/core/stdint.hpp>
#include <neolib/core/uuid.hpp>
#include <neolib/core/lifetime.hpp>
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/optional.hpp>
#include <neolib/core/variant.hpp>
#include <neolib/core/any.hpp>
#include <neolib/core/enum.hpp>
#include <neolib/core/vector.hpp>
#include <neolib/core/deque.hpp>
#include <neolib/core/list.hpp>
#include <neolib/core/string.hpp>
#include <neolib/core/string_utils.hpp>
#include <neolib/task/i_async_task.hpp>
#include <neolib/app/services.hpp>
#include <neolib/app/logger.hpp>

#include <neogfx/app/i18n.hpp>

namespace services = neolib::services;

namespace neogfx
{
    using namespace neolib::stdint_suffix;
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    using neolib::sfinae;

    using neolib::to_const;

    using neolib::lifetime;
    using neolib::destroying_flag;
    using neolib::destroyed_flag;

    using neolib::i_reference_counted;
    using neolib::reference_counted;
    using neolib::i_ref_ptr;
    using neolib::ref_ptr;
    using neolib::i_weak_ref_ptr;
    using neolib::weak_ref_ptr;
    using neolib::make_ref;
    using neolib::static_pointer_cast;
    using neolib::const_pointer_cast;
    using neolib::reinterpret_pointer_cast;
    using neolib::dynamic_pointer_cast;

    using neolib::i_discoverable;

    using neolib::to_abstract;
    using neolib::abstract_t;

    using neolib::optional;
    using neolib::i_optional;

    using neolib::variant;
    using neolib::none;
    using neolib::none_t;

    using neolib::cache;
    using neolib::invalid;
    using neolib::clear_cache;

    using neolib::any;
    using neolib::any_cast;

    using neolib::i_enum_t;
    using neolib::enum_t;
    using neolib::enum_to_string;

    using neolib::i_vector;
    using neolib::vector;

    using neolib::i_deque;
    using neolib::deque;

    using neolib::i_list;
    using neolib::list;

    using neolib::i_string;
    using neolib::string;
    using neolib::to_string;
    namespace string_literals
    {
        using namespace neolib::string_literals;
    }
    using namespace string_literals;

    string const empty_string;

    using neolib::uuid;

    using neolib::i_async_task;

    using namespace neolib::services;

    using neolib::operator<<;
    using neolib::operator>>;

    using neolib::logger::endl;
    using neolib::logger::flush;

    // convert strings with different traits and/or character types to neolib::string
    template <typename CharT, typename Traits, typename Allocator>
    inline const string to_string(const std::basic_string<CharT, Traits, Allocator>& aString)
    {
        static_assert(sizeof(CharT) == sizeof(char));
        return string{ reinterpret_cast<const char*>(aString.c_str()), aString.size() };
    }

    // convert character array (primarily for UTF-8 string literals) to neolib::string
    template <typename CharT, std::size_t Size>
    inline const string to_string(const CharT (&aString)[Size])
    {
        static_assert(sizeof(CharT) == sizeof(char));
        auto const correctedSize = (aString[Size - 1] == '\0' ? Size - 1 : Size);
        return correctedSize > 0 ? string{ reinterpret_cast<const char*>(&aString[0]), correctedSize } : string{};
    }

    struct not_yet_implemented : std::runtime_error
    {
        not_yet_implemented(std::string const& aDetail = {}) :
            std::runtime_error{ "neoGFX: Functionality not yet implemented" + (aDetail.empty() ? "" : " (" + aDetail + ")") } {}
    };

    class i_layout_item;
    class i_widget;
    namespace debug
    {
#ifdef NEOGFX_DEBUG
        extern void* item;
        extern i_layout_item* layoutItem;
        extern i_widget* renderItem;
        extern bool renderGeometryText; // todo: make a debug logger category
#endif // NEOGFX_DEBUG
        typedef neolib::logger::logger<9999> logger;
    }
}