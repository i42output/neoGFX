// path.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.
  
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

namespace neogfx
{
	class graphics_context;

	template <typename PointType>
	class basic_path
	{
		// types
	public:
		enum shape_type_e
		{
			Vertices,
			Quads,
			Lines,
			LineLoop,
			LineStrip,
			ConvexPolygon,
		};
		typedef PointType point_type;
		typedef typename point_type::coordinate_type coordinate_type;
		typedef typename point_type::coordinate_type coordinate_delta_type;
		typedef typename point_type::coordinate_type dimension_type;
		typedef basic_size<coordinate_type> size_type;
		typedef basic_delta<coordinate_type> delta_type;
		typedef basic_rect<coordinate_type> rect_type;
		typedef basic_line<coordinate_type> line_type;
		typedef std::vector<point_type> path_type;
		typedef std::vector<path_type> paths_type;
		typedef typename paths_type::size_type paths_size_type;
		struct clip_rect_list : std::vector < rect_type >
		{
			bool contains(const point_type& aPoint) const
			{
				for (std::vector<rect_type>::const_iterator i = begin(); i != end(); ++i)
					if (i->contains(aPoint))
						return true;
				return false;
			}
		};
	public:
		struct missing_move_to : std::logic_error { missing_move_to() : std::logic_error("neogfx::basic_path::missing_move_to") {} };
	private:
		struct intersect
		{
			// construction
		public:
			intersect() : iX(), iSkip() {}
			intersect(coordinate_type aX, bool aSkip = false) : iX(aX), iSkip(aSkip) {}
			// operations
		public:
			coordinate_type x() const { return iX; }
			bool skip() const { return iSkip; }
			bool operator==(const intersect& aOther) const { return iX == aOther.iX; }
			bool operator<(const intersect& aOther) const { return iX < aOther.iX; }
			// attributes
		private:
			coordinate_type iX;
			bool iSkip;
		};
		typedef std::vector<intersect> intersect_list;
		// construction
	public:
		basic_path(shape_type_e aShape = ConvexPolygon, paths_size_type aPathCountHint = 0) : iShape(aShape)
		{
			iPaths.reserve(aPathCountHint);
		}
		basic_path(const rect_type& aRect, shape_type_e aShape = ConvexPolygon) : iShape(aShape)
		{
			iPaths.reserve(5);
			move_to(aRect.top_left());
			line_to(aRect.top_right());
			line_to(aRect.bottom_right());
			line_to(aRect.bottom_left());
			line_to(aRect.top_left());
		}
		// operations
	public:
		shape_type_e shape() const 
		{		
			return iShape; 
		}
		void set_shape(shape_type_e aShape) 
		{ 
			iShape = aShape; 
		}
		point_type position() const 
		{ 
			return iPosition; 
		}
		void set_position(point_type aPosition) 
		{ 
			iPosition = aPosition; 
			iBoundingRect.reset();
		}
		const paths_type& paths() const 
		{ 
			return iPaths; 
		}
		paths_type& paths() 
		{ 
			return iPaths; 
		}
		std::vector<xyz> to_vertices(const typename paths_type::value_type& aPath, coordinate_type aPixelAdjust = 0.0) const
		{
			std::vector<xyz> result;
			result.reserve((aPath.size() + 1) * (iShape == Quads ? 6 : 1));
			if (aPath.size() > 2)
			{
				if (iShape == ConvexPolygon)
				{
					result.push_back(xyz{ bounding_rect(false).centre().x + position().x + aPixelAdjust, bounding_rect(false).centre().y + position().y + aPixelAdjust });
				}
				for (auto vi = aPath.begin(); vi != aPath.end(); ++vi)
				{
					switch (iShape)
					{
					case Quads:
						if (vi + 1 != aPath.end())
						{
							result.push_back(xyz{ vi->x + position().x + aPixelAdjust, vi->y + position().y + aPixelAdjust });
							result.push_back(xyz{ (vi + 1)->x + position().x + aPixelAdjust, (vi + 1)->y + position().y + aPixelAdjust });
							result.push_back(xyz{ vi->x + position().x + aPixelAdjust, vi->y + position().y + aPixelAdjust });
							result.push_back(xyz{ (vi + 1)->x + position().x + aPixelAdjust, (vi + 1)->y + position().y + aPixelAdjust });
						}
						break;
					case ConvexPolygon:
					default:
						result.push_back(xyz{ vi->x + position().x + aPixelAdjust, vi->y + position().y + aPixelAdjust });
						break;
					}
				}
				if (iShape == LineLoop && aPath[0] == aPath[aPath.size() - 1])
				{
					result.pop_back();
				}
				else if (iShape == ConvexPolygon && aPath[0] != aPath[aPath.size() - 1])
				{
					result.push_back(xyz{ aPath[0].x + aPixelAdjust, aPath[0].y + aPixelAdjust });
				}
			}
			return result;
		}
		void move_to(const point_type& aPoint, paths_size_type aLineCountHint = 0)
		{
			iPointFrom = aPoint;
			iLineCountHint = aLineCountHint;
		}
		void move_to(coordinate_type aX, coordinate_type aY, paths_size_type aLineCountHint = 0)
		{
			move_to(point_type(aX, aY), aLineCountHint);
		}
		void line_to(const point_type& aPoint)
		{
			if (iPointFrom)
			{
				iPaths.push_back(path_type());
				if (iLineCountHint != 0)
				{
					iPaths.back().reserve(iLineCountHint + 1);
					iLineCountHint = 0;
				}
				iPaths.back().push_back(*iPointFrom);
				iPointFrom.reset();
			}
			else
			{
				if (iPaths.empty())
					throw missing_move_to();
			}
			iPaths.back().push_back(aPoint);
			iBoundingRect.reset();
		}
		void line_to(coordinate_type aX, coordinate_type aY)
		{
			line_to(point_type(aX, aY));
		}
		void add_rect(const rect_type& aRectangle);
		void inflate(const delta_type& aDelta)
		{
			rect_type boundingRect = bounding_rect(false);
			for (paths_type::iterator i = iPaths.begin(); i != iPaths.end(); ++i)
				for (path_type::iterator j = i->begin(); j != i->end(); ++j)
				{
					if (j->x < boundingRect.x + static_cast<coordinate_type>(boundingRect.cx / 2))
						j->x -= aDelta.dx;
					else
						j->x += aDelta.dx;
					if (j->y < boundingRect.y + static_cast<coordinate_type>(boundingRect.cy / 2))
						j->y -= aDelta.dy;
					else
						j->y += aDelta.dy;
				}
			iBoundingRect.reset();
		}
		void inflate(coordinate_delta_type aDeltaX, coordinate_delta_type aDeltaY)
		{
			inflate(delta_type(aDeltaX, aDeltaY));
		}
		void deflate(const delta_type& aDeltas)
		{
			inflate(-aDeltas);
		}
		void deflate(coordinate_delta_type aDeltaX, coordinate_delta_type aDeltaY)
		{
			inflate(delta_type(-aDeltaX, -aDeltaY));
		}
		const rect_type& bounding_rect(bool aOffsetPosition = true, size_type aPixelWidthAdjustment = size_type(1.0, 1.0)) const;
		clip_rect_list clip_rects(const point& aOrigin) const;
		// attributes
	private:
		shape_type_e iShape;
		point_type iPosition;
		boost::optional<point_type> iPointFrom;
		paths_type iPaths;
		paths_size_type iLineCountHint;
		mutable boost::optional<rect_type> iBoundingRect;
	};
}

#include "path.inl"

namespace neogfx
{
	typedef basic_path<point> path;
	typedef boost::optional<path> optional_path;
}