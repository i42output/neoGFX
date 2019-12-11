// shader.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2019 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/map.hpp>
#include <neogfx/gfx/i_shader.hpp>

namespace neogfx
{
    template <typename Base>
    class shader : public neolib::reference_counted<Base>
    {
        typedef shader<Base> self_type;
        typedef neolib::reference_counted<Base> base_type;
    public:
        using typename base_type::abstract_type;
        typedef i_shader::value_type abstract_value_type;
        typedef shader_value_type value_type;
    private:
        typedef neolib::map<string, value_type> uniform_map;
    public:
        shader(shader_type aType, const std::string& aName, bool aEnabled = true) : 
            iType{ aType },
            iName{ aName },
            iEnabled{ aEnabled },
            iInvalidated{ true }
        {
        }
    public:
        shader_type type() const override
        {
            return iType;
        }
        const i_string& name() const override
        {
            return iName;
        }
        bool enabled() const override
        {
            return iEnabled;
        }
        void enable() override
        {
            iEnabled = true;
        }
        void disable() override
        {
            iEnabled = false;
        }
        bool invalidated() const override
        {
            return iInvalidated;
        }
        void invalidate() override
        {
            iInvalidated = true;
        }
    public:
        const uniform_map& uniforms() const override
        {
            return iUniforms;
        }
        void clear_uniform(const i_string& aName) override
        {
            auto u = iUniforms.find(aName);
            if (u != iUniforms.end())
            {
                iUniforms.erase(u);
                invalidate();
            }
        }
        void set_uniform(const i_string& aName, const abstract_value_type& aValue) override
        {
            if (iUniforms.find(aName) == iUniforms.end() || iUniforms[aName] != aValue)
            {
                iUniforms.emplace(aName, aValue);
                invalidate();
            }
        }
    private:
        shader_type iType;
        string iName;
        bool iEnabled;
        bool iInvalidated;
        uniform_map iUniforms;
    };
}