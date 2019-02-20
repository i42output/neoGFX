// skin.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2018 Leigh Johnston.  All Rights Reserved.
  
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
#include <unordered_set>
#include <boost/pool/pool_alloc.hpp>
#include <neogfx/gui/widget/i_skin.hpp>

namespace neogfx
{
    class skin : public i_skin
    {
    public:
        skin(const std::string aName);
        skin(const std::string aName, i_skin& aFallback);
    public:
        const std::string& name() const override;
        bool has_fallback() const override;
        i_skin& fallback() override;
    public:
        void activate() override;
        void deactivate() override;
    private:
        i_skin* iFallback;
    };
}