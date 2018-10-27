// neogfx.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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

#include <neolib/stdint.hpp>
#include <neogfx/app/i18n.hpp>

namespace neogfx
{
	using namespace neolib::stdint_suffix;
	using namespace std::string_literals;

	template <typename Component>
	struct service
	{
		static Component& instance();
	};
}