// native_texture.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2023 Leigh Johnston.  All Rights Reserved.
  
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

#include <map>

#include "i_native_texture.hpp"

namespace neogfx
{
    class native_texture : public reference_counted<i_native_texture>
    {
    public:
        typedef i_native_texture abstract_type;
    public:
        struct unsupported_sampling_type_for_function : std::logic_error { unsupported_sampling_type_for_function() : std::logic_error("neogfx::opengl_texture::unsupported_sampling_type_for_function") {} };
    public:
        i_vector<texture_line_segment> const& intersection(texture_line_segment const& aLine, rect const& aBoundingBox, vec2 const& aSampleSize = { 1.0, 1.0 }, scalar aTolerance = 0.0) const final;
    private:
        mutable std::map<std::tuple<texture_line_segment, rect, vec2, scalar>, vector<texture_line_segment>> iIntersectionResultCache;
    };
}