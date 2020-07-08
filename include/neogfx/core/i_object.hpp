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

#include <neolib/neolib.hpp>
#include <neolib/app/i_object.hpp>
#include <neogfx/core/object_type.hpp>

namespace neogfx
{
    class i_object : public neolib::i_object
    {
    public:
        virtual ~i_object() = default;
    public:
        virtual i_object& as_object() = 0;
    public:
        virtual neogfx::object_type object_type() const = 0;
    };
}
