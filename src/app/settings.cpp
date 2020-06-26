// settings.cpp
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
#include <neolib/core/custom_type.hpp>
#include <neogfx/core/color.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/settings.hpp>

namespace neogfx
{
    class default_custom_type_factory : public reference_counted<neolib::i_custom_type_factory>
    {
    public:
        default_custom_type_factory(ref_ptr<neolib::i_custom_type_factory> aUserCustomSettingTypeFactory) :
            iUserCustomSettingTypeFactory{ aUserCustomSettingTypeFactory }
        {
        }
    public:
        void create(const i_string& aType, const i_string& aValue, i_ref_ptr<neolib::i_custom_type>& aObject) const override
        {
            if (iUserCustomSettingTypeFactory)
                iUserCustomSettingTypeFactory->create(aType, aValue, aObject);
            if (!aObject)
            {
                if (aType == "neogfx::color")
                    aObject = make_ref<neolib::custom_type<color>>("neogfx::color", aValue);
                else if (aType == "neogfx::gradient")
                    aObject = make_ref<neolib::custom_type<gradient>>("neogfx::color", aValue);
                // todo: supported neogfx types
            }
            if (!aObject)
                throw unsupported_custom_type(aType.to_std_string());
        }
    private:
        ref_ptr<neolib::i_custom_type_factory> iUserCustomSettingTypeFactory;
    };

    settings::settings(const std::string& aFileName, ref_ptr<neolib::i_custom_type_factory> aCustomSettingTypeFactory) :
        base_type{ service<i_app>(), string{ aFileName }, make_ref<default_custom_type_factory>(aCustomSettingTypeFactory) }
    {
    }
}