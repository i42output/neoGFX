// settings.hpp
/*
  neogfx C++ App/Game Engine
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
#include <neolib/app/settings.hpp>

namespace neogfx
{
    using neolib::i_setting;
    using neolib::setting;
    using neolib::i_setting_value;
    using neolib::setting_value;
    using neolib::i_setting_constraints;
    using neolib::setting_constraints;
    using neolib::i_settings;

    class i_setting_icons : public i_reference_counted
    {
    public:
        typedef i_setting_icons abstract_type;
    public:
        virtual ~i_setting_icons() = default;
    public:
        virtual i_texture const& default_icon() const = 0;
        virtual i_texture const& category_icon(i_string const& aCategorySubkey) const = 0;
        virtual i_texture const& group_icon(i_string const& aGroupSubkey) const = 0;
        virtual i_texture const& setting_icon(neolib::i_setting const& aSetting) const = 0;
    };

    class settings : public neolib::settings
    {
        typedef neolib::settings base_type;
    public:
        settings(std::string const& aFileName = "settings.xml");
    };
}