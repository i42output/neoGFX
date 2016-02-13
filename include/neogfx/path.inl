// path.inl
/*
  neogfx C++ GUI Library
  Copyright(C) 2016 Leigh Johnston
  
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
	inline void basic_path<PointType>::add_rect(const rect_type& aRectangle)
	{
		size_type pixelSize = pixel();
		move_to(aRectangle.left(), aRectangle.top());
		line_to(aRectangle.right() - pixelSize.cx, aRectangle.top());
		line_to(aRectangle.right() - pixelSize.cx, aRectangle.bottom() - pixelSize.cy);
		line_to(aRectangle.left(), aRectangle.bottom() - pixelSize.cy);
		line_to(aRectangle.left(), aRectangle.top());
	}

	template <typename PointType>
	inline const typename basic_path<PointType>::rect_type& basic_path<PointType>::bounding_rect(bool aOffsetPosition, size_type aPixelWidthAdjustment) const
	{
		if (iBoundingRect)
			return *iBoundingRect;
		coordinate_type minX = std::numeric_limits<coordinate_type>::max();
		coordinate_type minY = std::numeric_limits<coordinate_type>::max();
		coordinate_type maxX = std::numeric_limits<coordinate_type>::min();
		coordinate_type maxY = std::numeric_limits<coordinate_type>::min();
		for (paths_type::const_iterator i = iPaths.begin(); i != iPaths.end(); ++i)
			for (path_type::const_iterator j = i->begin(); j != i->end(); ++j)
			{
				minX = std::min(minX, j->x);
				minY = std::min(minY, j->y);
				maxX = std::max(maxX, j->x);
				maxY = std::max(maxY, j->y);
			}
		iBoundingRect = rect_type(point_type(minX, minY) + (aOffsetPosition ? iPosition : point(0.0, 0.0)), size_type(maxX - minX + aPixelWidthAdjustment.cx, maxY - minY + aPixelWidthAdjustment.cy));
		return *iBoundingRect;
	}

	namespace
	{
		template <typename PointType>
		inline void add_clip_rect(typename basic_path<PointType>::clip_rect_list& aClipRects, const typename basic_path<PointType>::rect_type& aRect)
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
		rect_type boundingRect = bounding_rect() + aOrigin;
		typedef std::vector<line_type> lines_t;
		lines_t::size_type lineCount = 0;
		for (paths_type::const_iterator i = iPaths.begin(); i != iPaths.end(); ++i)
			lineCount += !i->empty() ? i->size() - 1 : 0;
		lines_t lines;
		lines.reserve(lineCount);
		for (paths_type::const_iterator i = iPaths.begin(); i != iPaths.end(); ++i)
		{
			if (i->size() > 1)
			{
				point_type ptPrevious = (*i)[0] + aOrigin;
				for (path_type::const_iterator j = i->begin() + 1; j != i->end(); ++j)
				{
					point_type pt = *j + aOrigin;
					lines.push_back(line_type(ptPrevious, pt));
					ptPrevious = pt;
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
			for (lines_t::const_iterator i = lines.begin(); i != lines.end(); ++i)
			{
				const line_type& line1 = *i;
				point_type pt;
				if (line1.intersection(scanLine, pt))
				{
					if (!line1.is_vertex(pt))
						xIntersects.push_back(intersect(pt.x));
					else
						for (lines_t::const_iterator j = lines.begin(); j != lines.end(); ++j)
						{
							const line_type& line2 = *j;
							if (line2.is_vertex(pt) && j != i)
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
			intersect_list::const_iterator end = std::unique(xIntersects.begin(), xIntersects.end());
			bool hadFirst = false;
			bool parity = true;
			intersect previousIntersect;
			for (intersect_list::const_iterator i = xIntersects.begin(); i != end; ++i)
			{
				intersect currentIntersect = *i;
				if (hadFirst)
				{
					bool isHorizontalEdge = false;
					for (lines_t::const_iterator i = lines.begin(); !isHorizontalEdge && i != lines.end(); ++i)
					{
						if (*i == line_type(point_type(previousIntersect.x(), y), point_type(currentIntersect.x(), y)) && i->delta_y() == 0)
						{
							isHorizontalEdge = true;
							bool found = false;
							for (lines_t::const_iterator j = lines.begin(); !found && j != lines.end(); ++j)
								if (j->is_vertex(i->a) && j != i)
									for (lines_t::const_iterator k = lines.begin(); !found && k != lines.end(); ++k)
										if (k->is_vertex(i->b) && k != i)
										{
											found = true;
											if ((j->delta_y() > 0 && k->delta_y() < 0) || (j->delta_y() < 0 && k->delta_y() > 0))
												parity = !parity;
										}
						}
					}
					if (!isHorizontalEdge && currentIntersect.x() - previousIntersect.x() >= 2)
					{
						if (parity)
						{
							add_clip_rect<point_type>(clipRects, 
								rect_type(point_type(previousIntersect.x() + 1, y), size_type(currentIntersect.x() - previousIntersect.x() - 1, 1)));
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