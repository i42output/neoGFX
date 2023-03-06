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

#define meta_object( ... ) \
        typedef __VA_ARGS__ base_type; \
    public: \
        void class_name(neolib::i_string& aClassName) const override \
        { \
            aClassName.append(boost::typeindex::type_id<decltype(*this)>().pretty_name()); \
            aClassName.append("::"sv); \
            base_type::class_name(aClassName); \
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
    };

    inline std::string class_names(i_object const& aObject)
    {
        neolib::string temp;
        aObject.class_name(temp);
        auto result = temp.to_std_string();
        boost::replace_all(result, "neogfx::", "");
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
