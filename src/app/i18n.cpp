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

    translation_context::translation_context(string const& aContext)
    {
        context_stack().push_back(aContext);
    }

    translation_context::~translation_context()
    {
        context_stack().pop_back();
    }

    string const& translation_context::context()
    {
        if (!context_stack().empty())
            return context_stack().back();
        static const string sDefaultContext;
        return sDefaultContext;
    }

    std::vector<string>& translation_context::context_stack()
    {
        static std::vector<string> sContextStack;
        return sContextStack;
    }

    translatable_string::translatable_string(i_string const& aTranslatableString, i_string const& aContext) : 
        string{ service<i_app>().translate(aTranslatableString, aContext) },
        iTranslatableString{ aTranslatableString },
        iContext{ aContext }
    {
    }

    translatable_string& translatable_string::operator()(std::int64_t aPlurality)
    {
        assign(service<i_app>().translate(iTranslatableString, iContext, aPlurality));
        return *this;
    }

    translatable_string translate(string const& aTranslatableString)
    {
        return translatable_string{ aTranslatableString, translation_context::context() };
    }

    translatable_string translate(string const& aTranslatableString, string const& aContext)
    {
        return translatable_string{ aTranslatableString, aContext };
    }

    translatable_string operator "" _t(const char* aTranslatableString, std::size_t aStringLength)
    {
        return translatable_string{ string{ aTranslatableString, aStringLength }, translation_context::context() };
    }

    translatable_string operator "" _t(const char8_t* aTranslatableString, std::size_t aStringLength)
    {
        return translatable_string{ string{ reinterpret_cast<const char*>(aTranslatableString), aStringLength }, translation_context::context() };
    }
}