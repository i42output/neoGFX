// i_style_sheet.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.
  
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

#include <neolib/core/i_optional.hpp>

#include <neogfx/core/primitives.hpp>

namespace neogfx
{
    class i_style_sheet
    {
    public:
        using abstract_type = i_style_sheet;
    public:
        struct failed_to_open_style_sheet : std::runtime_error { failed_to_open_style_sheet() : std::runtime_error("neogfx::i_style_sheet::failed_to_open_style_sheet") {} };
    public:
        virtual ~i_style_sheet() = default;
    };

    using i_optional_style_sheet = neolib::i_optional<i_style_sheet>;
}
