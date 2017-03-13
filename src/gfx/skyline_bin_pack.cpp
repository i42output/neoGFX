// skyline_bin_pack.cpp
/*
 *  Based on public domain code written by Jukka Jylänki.
 *
 *  This implementation written by Leigh Johnston.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above authorship
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above authorship
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of Leigh Johnston nor the names of any
 *       other contributors to this software may be used to endorse or
 *       promote products derived from this software without specific prior
 *       written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/skyline_bin_pack.hpp>

namespace neogfx
{	
	skyline_bin_pack::skyline_bin_pack(const size& aDimensions, bool aAllowRotations) :
		iDimensions(aDimensions), iAllowRotations(aAllowRotations)
	{
		init();
	}

	void skyline_bin_pack::init()
	{
		iUsedSurfaceArea = 0.0;
		iSkyLine.clear();
		skyline_node node = {};
		node.width = iDimensions.cx;
		iSkyLine.push_back(node);
	}

	void skyline_bin_pack::insert(std::vector<size>& aRects, std::vector<rect> &aResult, level_choice_heuristic_e aMethod)
	{
		aResult.clear();

		while (aRects.size() > 0)
		{
			rect bestNode;
			dimension bestScore1 = std::numeric_limits<dimension>::max();
			dimension bestScore2 = std::numeric_limits<dimension>::max();
			size_t bestSkylineIndex = static_cast<size_t>(-1);
			size_t bestrectIndex = static_cast<size_t>(-1);
			for (size_t i = 0; i < aRects.size(); ++i)
			{
				rect newNode;
				dimension score1;
				dimension score2;
				size_type index;
				switch (aMethod)
				{
				case LevelBottomLeft:
				default:
					newNode = find_position_for_new_node_bottom_left(aRects[i], score1, score2, index);
					break;
				case LevelMinWasteFit:
					newNode = find_position_for_new_node_min_waste(aRects[i], score2, score1, index);
					break;
				}
				if (newNode.cy != 0)
				{
					if (score1 < bestScore1 || (score1 == bestScore1 && score2 < bestScore2))
					{
						bestNode = newNode;
						bestScore1 = score1;
						bestScore2 = score2;
						bestSkylineIndex = index;
						bestrectIndex = i;
					}
				}
			}

			if (bestrectIndex == static_cast<size_t>(-1))
				return;

			add_skyline_level(bestSkylineIndex, bestNode);
			iUsedSurfaceArea += aRects[bestrectIndex].cx * aRects[bestrectIndex].cy;
			aRects.erase(aRects.begin() + bestrectIndex);
			aResult.push_back(bestNode);
		}
	}

	bool skyline_bin_pack::insert(const size& aRectSize, rect& aResult, level_choice_heuristic_e aMethod)
	{
		switch (aMethod)
		{
		case LevelBottomLeft:
		default:
			return insert_bottom_left(aRectSize, aResult);
		case LevelMinWasteFit: 
			return insert_min_waste(aRectSize, aResult);
		}
	}

	bool skyline_bin_pack::rectangle_fits(size_type aIndex, const size& aDimensions, dimension& aY) const
	{
		dimension x = iSkyLine[aIndex].x;
		if (x + aDimensions.cx > iDimensions.cx)
			return false;
		dimension widthLeft = aDimensions.cx;
		size_type i = aIndex;
		aY = iSkyLine[aIndex].y;
		while (widthLeft > 0)
		{
			aY = std::max(aY, iSkyLine[i].y);
			if (aY + aDimensions.cy > iDimensions.cy)
				return false;
			widthLeft -= iSkyLine[i].width;
			++i;
		}
		return true;
	}

	dimension skyline_bin_pack::compute_wasted_area(size_type aIndex, const size& aDimensions, dimension y) const
	{
		dimension wastedArea = 0;
		const dimension rectLeft = iSkyLine[aIndex].x;
		const dimension rectRight = rectLeft + aDimensions.cx;
		for (; aIndex < iSkyLine.size() && iSkyLine[aIndex].x < rectRight; ++aIndex)
		{
			if (iSkyLine[aIndex].x >= rectRight || iSkyLine[aIndex].x + iSkyLine[aIndex].width <= rectLeft)
				break;

			dimension leftSide = iSkyLine[aIndex].x;
			dimension rightSide = std::min(rectRight, leftSide + iSkyLine[aIndex].width);
			wastedArea += (rightSide - leftSide) * (y - iSkyLine[aIndex].y);
		}
		return wastedArea;
	}

	bool skyline_bin_pack::rectangle_fits(size_type aIndex, const size& aDimensions, dimension& aY, dimension& aWastedArea) const
	{
		bool fits = rectangle_fits(aIndex, aDimensions, aY);
		if (fits)	
			aWastedArea = compute_wasted_area(aIndex, aDimensions, aY);
		return fits;
	}

	void skyline_bin_pack::add_skyline_level(size_type aIndex, const rect& aRect)
	{
		skyline_node newNode;
		newNode.x = aRect.x;
		newNode.y = aRect.y + aRect.height();
		newNode.width = aRect.width();
		iSkyLine.insert(iSkyLine.begin() + aIndex, newNode);

		for (size_t i = aIndex + 1; i < iSkyLine.size(); ++i)
		{
			if (iSkyLine[i].x < iSkyLine[i - 1].x + iSkyLine[i - 1].width)
			{
				dimension shrink = iSkyLine[i - 1].x + iSkyLine[i - 1].width - iSkyLine[i].x;

				iSkyLine[i].x += shrink;
				iSkyLine[i].width -= shrink;

				if (iSkyLine[i].width <= 0)
				{
					iSkyLine.erase(iSkyLine.begin() + i);
					--i;
				}
				else
					break;
			}
			else
				break;
		}
		merge_skylines();
	}

	void skyline_bin_pack::merge_skylines()
	{
		for (size_t i = 0; i < iSkyLine.size() - 1; ++i)
			if (iSkyLine[i].y == iSkyLine[i + 1].y)
			{
				iSkyLine[i].width += iSkyLine[i + 1].width;
				iSkyLine.erase(iSkyLine.begin() + (i + 1));
				--i;
			}
	}

	bool skyline_bin_pack::insert_bottom_left(const size& aDimensions, rect& aResult)
	{
		dimension bestHeight;
		dimension bestWidth;
		size_type bestIndex;
		aResult = find_position_for_new_node_bottom_left(aDimensions, bestHeight, bestWidth, bestIndex);
		if (bestIndex == -1)
			return false;
		add_skyline_level(bestIndex, aResult);
		iUsedSurfaceArea += aDimensions.cx * aDimensions.cy;
		return true;
	}

	rect skyline_bin_pack::find_position_for_new_node_bottom_left(const size& aDimensions, dimension& aBestHeight, dimension& aBestWidth, size_type &aBestIndex) const
	{
		aBestHeight = std::numeric_limits<dimension>::max();
		aBestWidth = std::numeric_limits<dimension>::max();
		aBestIndex = static_cast<size_t>(-1);
		rect newNode = {};
		for (size_t i = 0; i < iSkyLine.size(); ++i)
		{
			dimension y;
			if (rectangle_fits(i, aDimensions, y))
			{
				if (y + aDimensions.cy < aBestHeight || (y + aDimensions.cy == aBestHeight && iSkyLine[i].width < aBestWidth))
				{
					aBestHeight = y + aDimensions.cy;
					aBestIndex = i;
					aBestWidth = iSkyLine[i].width;
					newNode.x = iSkyLine[i].x;
					newNode.y = y;
					newNode.cx = aDimensions.cx;
					newNode.cy = aDimensions.cy;
				}
			}
			if (iAllowRotations)
			{
				if (rectangle_fits(i, aDimensions, y))
				{
					if (y + aDimensions.cx < aBestHeight || (y + aDimensions.cx == aBestHeight && iSkyLine[i].width < aBestWidth))
					{
						aBestHeight = y + aDimensions.cx;
						aBestIndex = i;
						aBestWidth = iSkyLine[i].width;
						newNode.x = iSkyLine[i].x;
						newNode.y = y;
						newNode.cx = aDimensions.cy;
						newNode.cy = aDimensions.cx;
					}
				}
			}
		}

		return newNode;
	}

	bool skyline_bin_pack::insert_min_waste(const size& aDimensions, rect& aResult)
	{
		dimension bestHeight;
		dimension bestWastedArea;
		size_type bestIndex;
		aResult = find_position_for_new_node_min_waste(aDimensions, bestHeight, bestWastedArea, bestIndex);
		if (bestIndex == -1)
			return false;
		add_skyline_level(bestIndex, aResult);
		iUsedSurfaceArea += aDimensions.cx * aDimensions.cy;
		return true;
	}

	rect skyline_bin_pack::find_position_for_new_node_min_waste(const size& aDimensions, dimension& aBestHeight, dimension& aBestWastedArea, size_type &aBestIndex) const
	{
		aBestHeight = std::numeric_limits<dimension>::max();
		aBestWastedArea = std::numeric_limits<dimension>::max();
		aBestIndex = static_cast<size_t>(-1);
		rect newNode = {};
		for (size_t i = 0; i < iSkyLine.size(); ++i)
		{
			dimension y;
			dimension wastedArea;

			if (rectangle_fits(i, aDimensions, y, wastedArea))
			{
				if (wastedArea < aBestWastedArea || (wastedArea == aBestWastedArea && y + aDimensions.cy < aBestHeight))
				{
					aBestHeight = y + aDimensions.cy;
					aBestWastedArea = wastedArea;
					aBestIndex = i;
					newNode.x = iSkyLine[i].x;
					newNode.y = y;
					newNode.cx = aDimensions.cx;
					newNode.cy = aDimensions.cy;
				}
			}
			if (iAllowRotations)
			{
				if (rectangle_fits(i, aDimensions, y, wastedArea))
				{
					if (wastedArea < aBestWastedArea || (wastedArea == aBestWastedArea && y + aDimensions.cx < aBestHeight))
					{
						aBestHeight = y + aDimensions.cx;
						aBestWastedArea = wastedArea;
						aBestIndex = i;
						newNode.x = iSkyLine[i].x;
						newNode.y = y;
						newNode.cx = aDimensions.cy;
						newNode.cy = aDimensions.cx;
					}
				}
			}
		}
		return newNode;
	}

	dimension skyline_bin_pack::occupancy() const
	{
		return iUsedSurfaceArea / (iDimensions.cx * iDimensions.cy);
	}
}