// i_object.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2018, 2020 Leigh Johnston.  All Rights Reserved.

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

#include <boost/algorithm/string/replace.hpp>
#include <neolib/neolib.hpp>
#include <neolib/core/i_string.hpp>
#include <neolib/app/i_object.hpp>
#include <neogfx/core/object_type.hpp>

namespace neogfx
{
    namespace detail
    {
        template <typename ClassT>
        inline std::string class_pretty_name()
        {
            auto temp = boost::typeindex::type_id<ClassT>().pretty_name();
            boost::replace_all(temp, "::", "--");
            boost::replace_all(temp, "class ", "");
            boost::replace_all(temp, "neogfx--", "");
            std::string::size_type templateStart = temp.find('<');
            std::string::size_type templateEnd = temp.rfind('>');
            if (templateStart != std::string::npos && templateEnd != std::string::npos)
                temp.erase(templateStart, templateEnd - templateStart + 1);
            return temp;
        }
    }
}

#define meta_object( ... ) \
        using base_type = __VA_ARGS__; \
    public: \
        mutable std::optional<std::string> ClassName; \
        using i_object::class_name; \
        void class_name(neolib::i_string& aClassName) const override \
        { \
            if (ClassName == std::nullopt) \
            { \
                aClassName.append(neogfx::detail::class_pretty_name<decltype(*this)>()); \
                aClassName.append(":"sv); \
                base_type::class_name(aClassName); \
                ClassName.emplace(aClassName.to_std_string_view()); \
            } \
            else \
                aClassName = ClassName.value(); \
        }

namespace neogfx
{
    class i_object : public neolib::i_object
    {
    public:
        virtual ~i_object() = default;
    public:
        virtual i_object& as_object() = 0;
    public:
        virtual void class_name(neolib::i_string& aClassName) const = 0;
        virtual neogfx::object_type object_type() const = 0;
        std::string const& class_name() const
        {
            thread_local neolib::string className;
            className.clear();
            class_name(className);
            thread_local std::string className2;
            className2 = className.as_std_string();
            return className2;
        }
    };

    inline std::string class_names(i_object const& aObject)
    {
        neolib::string temp;
        aObject.class_name(temp);
        auto result = temp.to_std_string();
        boost::replace_all(result, "neogfx--", "");
        boost::replace_all(result, "class ", "");
        boost::replace_all(result, " ", "");
        std::size_t templateCounter = 0;
        for (auto i = result.begin(); i != result.end();)
        {
            if (*i == '<')
            {
                ++templateCounter;
                i = result.erase(i);
            }
            else if (*i == '>')
            {
                --templateCounter;
                i = result.erase(i);
            }
            else if (templateCounter != 0)
                i = result.erase(i);
            else
                ++i;
        }
        return result;
    }
}
