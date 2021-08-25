// svg.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2021 Leigh Johnston.  All Rights Reserved.
  
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
#include <vector>
#include <memory>
#include <variant>
#include <optional>
#include <string>
#include <neogfx/core/numerical.hpp>
#include <neogfx/core/units.hpp>

namespace neogfx::file
{
    class svg
    {
    public:
        typedef std::string url;
        typedef std::variant<length, scalar> length_percentage;
        struct basic_shape
        {
            enum class value
            {
                Inset,
                Circle,
                Ellipse,
                Polygon,
                Path
            };

            struct inset
            {
                // todo
            };
            struct circle
            {
                // todo
            };
            struct ellipse
            {
                // todo
            };
            struct polygon
            {
                // todo
            };
            struct path
            {
                // todo
            };

            typedef std::variant<inset, circle, ellipse, polygon, path> value_type;
        };
        struct geometry_box
        {
            enum class value
            {
                MarginBox,
                BorderBox,
                PaddingBox,
                ContentBox,
                FillBox,
                StrokeBox,
                ViewBox
            };
        };

        struct node;
        typedef std::vector<std::unique_ptr<node>> nodes;
        struct node
        {
            nodes children;
        };
        struct attribute
        {
            struct presentation
            {
                struct alignment_baseline
                {
                    enum class value
                    {
                        Auto,
                        Baseline,
                        BeforeEdge,
                        TextBeforeEdge,
                        Middle,
                        Central,
                        AfterEdge,
                        TextAfterEdge,
                        Ideographic,
                        Alphabetic,
                        Hanging,
                        Mathematical,
                        Top,
                        Center,
                        Bottom
                    };
                };
                struct baseline_shift
                {
                    enum class value
                    {
                        LengthPercentage,
                        Sub,
                        Super
                    };
                    typedef std::variant<std::monostate, length_percentage> value_type;
                };
                struct clip
                {
                    enum class value
                    {
                        Auto,
                        Rect
                    };
                    typedef std::variant<std::monostate, basic_rect<length>> value_type;
                };
                struct clip_path
                {
                    enum class value
                    {
                        Url,
                        BasicShapeGeometryBox,
                        None
                    };
                    typedef std::variant<std::monostate, url, std::pair<std::optional<basic_shape>, std::optional<geometry_box>>> value_type;
                };
            };
        };
        struct group : node
        {
        };
    public:
        svg(std::string const& aUri) {}
    public:

    };
}