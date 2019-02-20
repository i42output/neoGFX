// i18n.hpp
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

#include <neogfx/neogfx.hpp>

namespace neogfx
{
    class translation_context
    {
    public:
        translation_context(const std::string& aContext);
        ~translation_context();
    public:
        static const std::string& context();
    private:
        static std::vector<std::string>& context_stack();
    };

    std::string operator "" _t(const char* aTranslatableString, std::size_t aStringLength);

    const std::string& translate(const std::string& aTranslatableString);

    const std::string& translate(const std::string& aTranslatableString, const std::string& aContext);
}

using neogfx::operator "" _t;