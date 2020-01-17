// path.inl
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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

namespace neogfx
{
    template <typename PointType>
    inline void basic_path<PointType>::add_rect(const mesh_type& aRectangle)
    {
        size_type const pixelSize = { 1px, 1px };
        move_to(aRectangle.left(), aRectangle.top());
        line_to(aRectangle.right() - pixelSize.cx, aRectangle.top());
        line_to(aRectangle.right() - pixelSize.cx, aRectangle.bottom() - pixelSize.cy);
        line_to(aRectangle.left(), aRectangle.bottom() - pixelSize.cy);
        line_to(aRectangle.left(), aRectangle.top());
    }

    template <typename PointType>
    inline typename basic_path<PointType>::mesh_type basic_path<PointType>::bounding_rect(bool aOffsetPosition, size_type aPixelWidthAdjustment) const
    {
        if (iBoundingRect && std::get<0>(*iBoundingRect) == aOffsetPosition && std::get<1>(*iBoundingRect) == aPixelWidthAdjustment)
            return std::get<2>(*iBoundingRect);
        coordinate_type minX = std::numeric_limits<coordinate_type>::max();
        coordinate_type minY = std::numeric_limits<coordinate_type>::max();
        coordinate_type maxX = std::numeric_limits<coordinate_type>::min();
        coordinate_type maxY = std::numeric_limits<coordinate_type>::min();
        for (auto const& sub_path : iSubPaths)
            for (auto const& point : sub_path)
            {
                minX = std::min(minX, point.x);
                minY = std::min(minY, point.y);
                maxX = std::max(maxX, point.x);
                maxY = std::max(maxY, point.y);
            }
        iBoundingRect = std::make_tuple(
            aOffsetPosition,
            aPixelWidthAdjustment,
            mesh_type{ point_type{ minX, minY } +(aOffsetPosition ? iPosition : point{}), size_type{ maxX - minX + aPixelWidthAdjustment.cx, maxY - minY + aPixelWidthAdjustment.cy } });
        return std::get<2>(*iBoundingRect);
    }

    namespace
    {
        template <typename PointType>
        inline void add_clip_rect(typename basic_path<PointType>::clip_rect_list& aClipRects, const typename basic_path<PointType>::mesh_type& aRect)
        {
            if (aClipRects.empty())
                aClipRects.push_back(aRect);
            else if (aRect.x == aClipRects.back().x && aRect.cx == aClipRects.back().cx && aRect.y == aClipRects.back().bottom())
                aClipRects.back().cy += aRect.cy;
            else if (aRect.contains(aClipRects.back()))
            {
                aClipRects.pop_back();
                add_clip_rect<PointType>(aClipRects, aRect);
            }
            else
            {
                for (typename basic_path<PointType>::clip_rect_list::const_iterator i = aClipRects.begin(); i != aClipRects.end(); ++i)
                    if (i->contains(aRect))
                        return;
                aClipRects.push_back(aRect);
            }
        }
    }

    template <typename PointType>
    inline typename basic_path<PointType>::clip_rect_list basic_path<PointType>::clip_rects(const point& aOrigin) const
    {
        mesh_type boundingRect = bounding_rect() + aOrigin;
        typedef std::vector<line_type> lines_t;
        typename lines_t::size_type lineCount = 0;
        for (auto const& sub_path : iSubPaths)
            lineCount += !sub_path.empty() ? sub_path.size() - 1 : 0;
        lines_t lines;
        lines.reserve(lineCount);
        for (auto const& sub_path : iSubPaths)
        {
            if (sub_path.size() > 1)
            {
                point_type ptPrevious = sub_path[0] + aOrigin;
                for (auto pt = sub_path.begin() + 1; pt != sub_path.end(); ++pt)
                {
                    auto const ptAdjusted = *pt + aOrigin;
                    lines.push_back(line_type(ptPrevious, ptAdjusted));
                    ptPrevious = ptAdjusted;
                }
            }
        }
        clip_rect_list clipRects;
        clipRects.reserve(16);
        intersect_list xIntersects;
        for (coordinate_type y = boundingRect.top(); y != boundingRect.bottom(); ++y)
        {
            xIntersects.clear();
            line_type scanLine(point_type(boundingRect.left(), y), point_type(boundingRect.right() - 1, y));
            for (auto const& line1 : lines)
            {
                point_type pt;
                if (line1.intersection(scanLine, pt))
                {
                    if (!line1.is_vertex(pt))
                        xIntersects.push_back(intersect(pt.x));
                    else
                        for (auto const& line2 : lines)
                        {
                            if (line2.is_vertex(pt) && &line2 != &line1)
                            {
                                line_type cornerLine1 = line1.from(pt);
                                line_type cornerLine2 = line2.from(pt);
                                if ((cornerLine1.delta_y() > 0 && cornerLine2.delta_y() < 0) || (cornerLine1.delta_y() < 0 && cornerLine2.delta_y() > 0))
                                    xIntersects.push_back(intersect(pt.x));
                                else
                                    xIntersects.push_back(intersect(pt.x, cornerLine2.delta_y() != 0));
                            }
                        }
                }                
            }
            std::sort(xIntersects.begin(), xIntersects.end());
            auto end = std::unique(xIntersects.begin(), xIntersects.end());
            bool hadFirst = false;
            bool parity = true;
            intersect previousIntersect;
            for (auto const& currentIntersect : xIntersects)
            {
                if (hadFirst)
                {
                    bool isHorizontalEdge = false;
                    for (auto line1 = lines.begin(); !isHorizontalEdge && line1 != lines.end(); ++line1)
                    {
                        if (*line1 == line_type{ point_type{ previousIntersect.x(), y }, point_type{ currentIntersect.x(), y } } && line1->delta_y() == 0)
                        {
                            isHorizontalEdge = true;
                            bool found = false;
                            for (auto line2 = lines.begin(); !found && line2 != lines.end(); ++line2)
                                if (line2->is_vertex(line1.a) && line2 != line1)
                                    for (auto line3 = lines.begin(); !found && line3 != lines.end(); ++line3)
                                        if (line3->is_vertex(line1->b) && line3 != line1)
                                        {
                                            found = true;
                                            if ((line2->delta_y() > 0 && line3->delta_y() < 0) || (line2->delta_y() < 0 && line3->delta_y() > 0))
                                                parity = !parity;
                                        }
                        }
                    }
                    if (!isHorizontalEdge && currentIntersect.x() - previousIntersect.x() >= 2)
                    {
                        if (parity)
                        {
                            add_clip_rect<point_type>(clipRects, 
                                mesh_type{ point_type{ previousIntersect.x() + 1, y }, size_type{ currentIntersect.x() - previousIntersect.x() - 1, 1 } });
                        }
                        if (!currentIntersect.skip())
                            parity = !parity;
                    }
                    else if (!isHorizontalEdge && !currentIntersect.skip() && !previousIntersect.skip())
                        parity = !parity;
                }
                hadFirst = true;
                previousIntersect = currentIntersect;
            }
        }
        return clipRects;
    }
}