// element.hpp
/*
neoGFX Design Studio
Copyright(C) 2020 Leigh Johnston

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
#include <algorithm>
#include <boost/format.hpp>
#include <neolib/core/reference_counted.hpp>
#include <neolib/core/optional.hpp>
#include <neolib/core/vector.hpp>
#include <neolib/core/string.hpp>
#include <neogfx/core/units.hpp>
#include <neogfx/tools/DesignStudio/i_element.hpp>

namespace neogfx::DesignStudio
{
    template <typename Base = i_element>
    class element : public neolib::reference_counted<Base>
    {
        typedef element<Base> self_type;
        typedef neolib::reference_counted<Base> base_type;
    public:
        typedef abstract_t<Base> abstract_type;
        typedef neolib::vector<ref_ptr<abstract_type>> children_t;
    public:
        element(element_group aGroup, const std::string& aType) :
            iParent{ nullptr }, iGroup{ aGroup }, iType{ aType }
        {
        }
        element(element_group aGroup, const std::string& aType, const std::string& aId) :
            iParent{ nullptr }, iGroup{ aGroup }, iType{ aType }, iId{ aId }
        {
        }
        element(i_element& aParent, element_group aGroup, const std::string& aType) :
            iParent{ &aParent }, iGroup{ aGroup }, iType{ aType }
        {
            parent().children().push_back(neolib::ref_ptr<i_element>{ this });
        }
        element(i_element& aParent, element_group aGroup, const std::string& aType, const std::string& aId) :
            iParent{ &aParent }, iGroup{ aGroup }, iType{ aType }, iId{ aId }
        {
            parent().children().push_back(neolib::ref_ptr<i_element>{ this });
        }
        ~element()
        {
        }
    public:
        element_group group() const override
        {
            return iGroup;
        }
        const neolib::i_string& type() const override
        {
            return iType;
        }
        const neolib::i_string& id() const override
        {
            return *iId;
            return anonymous_id();
        }
    public:
        bool has_parent() const override
        {
            return iParent != nullptr;
        }
        const i_element& parent() const override
        {
            if (has_parent())
                return *iParent;
            throw no_parent();
        }
        i_element& parent() override
        {
            return const_cast<i_element&>(to_const(*this).parent());
        }
        const children_t& children() const override
        {
            return iChildren;
        }
        children_t& children() override
        {
            return iChildren;
        }
    public:
        i_element* iParent;
        element_group iGroup;
        neolib::string iType;
        neolib::string iId;
    };
}
