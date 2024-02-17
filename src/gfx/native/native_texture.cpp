// native_texture.cpp
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

#include <neogfx/neogfx.hpp>

#include <map>

#include "native_texture.hpp"

namespace neogfx
{
    i_vector<texture_line_segment> const& native_texture::intersection(texture_line_segment const& aLine, rect const& aBoundingBox, vec2 const& aSampleSize, scalar aTolerance) const
    {
        switch (sampling())
        {
        case texture_sampling::Normal:
        case texture_sampling::Nearest:
        case texture_sampling::Data:
            // supported
            break;
        default:
            throw unsupported_sampling_type_for_function();
        }

        thread_local vector<texture_line_segment> const empty;
        if (aLine.v1 == aLine.v2)
            return empty;

        auto existingResult = iIntersectionResultCache.find(std::make_tuple(aLine, aBoundingBox, aSampleSize, aTolerance));
        if (existingResult != iIntersectionResultCache.end())
            return existingResult->second;

        std::optional<scoped_render_target> srt;
        if (target_type() == render_target_type::Surface)
            srt.emplace(*this);

        auto& result = iIntersectionResultCache[std::make_tuple(aLine, aBoundingBox, aSampleSize, aTolerance)];

        auto check_9 = [&](scalar x, scalar y, vec2 sampleSize, scalar offset)
        {
            auto to_alpha = [&](color const& pixel)
            {
                switch (data_format())
                {
                case texture_data_format::RGBA:
                    return pixel.alpha() * 1.0;
                case texture_data_format::Red:
                    return pixel.red() * 1.0;
                case texture_data_format::SubPixel:
                    return pixel.magnitude() * 255.0;
                default:
                    return pixel.alpha() * 1.0;
                }
            };

            auto const alphaTolerance = aTolerance * 255.0;

            auto check_1 = [&](scalar x, scalar y)
            {
                if (!aBoundingBox.contains({ x, y }))
                    return false;
                return to_alpha(get_pixel({ x, y })) > alphaTolerance;
            };

            if (offset == 0.0)
                return check_1(x, y);

            auto const xOffset = std::min(offset, sampleSize.x - 1.0);
            auto const yOffset = std::min(offset, sampleSize.y - 1.0);

            if (check_1(x - xOffset, y - yOffset) ||
                check_1(x, y - yOffset) ||
                check_1(x + xOffset, y - yOffset) ||
                check_1(x - xOffset, y) ||
                check_1(x + xOffset, y) ||
                check_1(x - xOffset, y) ||
                check_1(x, y + yOffset) ||
                check_1(x + xOffset, y + yOffset))
                return true;

            return false;
        };

        auto apply = [&](bool& inSegment, bool intersects, scalar x, scalar y)
        {
            if (intersects)
            {
                if (!inSegment)
                {
                    inSegment = true;
                    result.push_back(texture_line_segment{ { x, y }, { x, y } });
                }
                else
                {
                    result.back().v2 = { x, y };
                }
            }
            else
                inSegment = false;
        };

        // as we are caching the result it probably isn't worth optimising this to use Bresenham's or such

        if (std::abs(aLine.v2.x - aLine.v1.x) > std::abs(aLine.v2.y - aLine.v1.y))
        {
            auto start = aLine.v1;
            auto end = aLine.v2;
            if (start.x > end.x)
                std::swap(start, end);
            auto const gradient = (end.y - start.y) / (end.x - start.x);
            bool inSegment = false;
            for (scalar x = start.x; x <= end.x; x += 1.0)
            {
                auto y = gradient * (x - start.x) + start.y;
                bool intersects = false;
                for (scalar offset = 0.0; !intersects && offset < std::max(aSampleSize.x, aSampleSize.y); offset += 1.0)
                {
                    if (check_9(x, y, aSampleSize, offset))
                        intersects = true;
                }
                apply(inSegment, intersects, x, y);
            }
        }
        else
        {
            auto start = aLine.v1;
            auto end = aLine.v2;
            if (start.y > end.y)
                std::swap(start, end);
            auto const gradient = (end.x - start.x) / (end.y - start.y);
            bool inSegment = false;
            for (scalar y = start.y; y <= end.y; y += 1.0)
            {
                auto x = gradient * (y - start.y) + start.x;
                bool intersects = false;
                for (scalar offset = 0.0; !intersects && offset < std::max(aSampleSize.x, aSampleSize.y); offset += 1.0)
                {
                    if (check_9(x, y, aSampleSize, offset))
                        intersects = true;
                }
                apply(inSegment, intersects, x, y);
            }
        }

        return result;
    }
}