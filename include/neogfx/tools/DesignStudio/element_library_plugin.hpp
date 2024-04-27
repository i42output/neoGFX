/*
  element_library_plugin.hpp

  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.

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

#include <boost/dll.hpp>

#include <neolib/core/string.hpp>
#include <neolib/core/reference_counted.hpp>
#include <neolib/app/version.hpp>
#include <neolib/app/i_application.hpp>
#include <neolib/plugin/plugin.hpp>

#include <neogfx/tools/DesignStudio/i_element_library.hpp>

namespace neogfx::DesignStudio
{
    template <typename ElementLibrary>
    using element_library_plugin = neolib::simple_plugin<ElementLibrary>;
}