// i18n.cpp
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

#include <neogfx/neogfx.hpp>
#include <string>
#include <neolib/core/map.hpp>
#include <neolib/core/string.hpp>
#include <neogfx/app/i18n.hpp>
#include <neogfx/app/i_app.hpp>

namespace neogfx
{
    const neolib::i_map<i_string, i_string>& language_codes()
    {
        static const neolib::map<string, string> sLanguageCodes
        {
        };
        return sLanguageCodes;
    }

    translation_context::translation_context(const std::string& aContext)
    {
        context_stack().push_back(aContext);
    }

    translation_context::~translation_context()
    {
        context_stack().pop_back();
    }

    const std::string& translation_context::context()
    {
        if (!context_stack().empty())
            return context_stack().back();
        static const std::string sDefaultContext;
        return sDefaultContext;
    }

    std::vector<std::string>& translation_context::context_stack()
    {
        static std::vector<std::string> sContextStack;
        return sContextStack;
    }

    std::string operator "" _t(const char* aTranslatableString, std::size_t aStringLength)
    {
        return translate(std::string{ aTranslatableString, aStringLength });
    }

    std::string operator "" _t(const char8_t* aTranslatableString, std::size_t aStringLength)
    {
        return translate(std::string{ reinterpret_cast<const char*>(aTranslatableString), aStringLength });
    }

    const std::string& translate(const std::string& aTranslatableString)
    {
        return translate(aTranslatableString, translation_context::context());
    }

    const std::string& translate(const std::string& aTranslatableString, const std::string& aContext)
    {
        return service<i_app>().translate(aTranslatableString, aContext);
    }
}