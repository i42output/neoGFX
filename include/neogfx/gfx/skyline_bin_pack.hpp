// skyline_bin_pack.hpp
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
#include <neogfx/core/geometry.hpp>

#pragma once

namespace neogfx
{
	class skyline_bin_pack
	{
	public:
		enum level_choice_heuristic_e
		{
			LevelBottomLeft,
			LevelMinWasteFit
		};

	private:
		struct skyline_node
		{
			dimension x;
			dimension y;
			dimension width;
		};
		typedef std::vector<skyline_node> skyline_type;
		typedef skyline_type::size_type size_type;

	public:
		/// Instantiates a bin of the given size.
		skyline_bin_pack(const size& aDimensions, bool aAllowRotations = false);

		/// (Re)initializes the packer to an empty bin of width x height units. Call whenever
		/// you need to restart with a new bin.
		void init();

		void insert(std::vector<size>& aRects, std::vector<rect>& aResult, level_choice_heuristic_e aMethod = LevelBottomLeft);
		bool insert(const size& aRectSize, rect& aResult, level_choice_heuristic_e aMethod = LevelBottomLeft);
		dimension occupancy() const;

	private:
		bool insert_bottom_left(const size& aDimensions, rect& aResult);
		bool insert_min_waste(const size& aDimensions, rect& aResult);
		rect find_position_for_new_node_min_waste(const size& aDimensions, dimension& aBestHeight, dimension& aBestWastedArea, size_type &aBestIndex) const;
		rect find_position_for_new_node_bottom_left(const size& aDimensions, dimension& aBestHeight, dimension& aBestWidth, size_type &bestIndex) const;
		bool rectangle_fits(size_type aIndex, const size& aDimensions, dimension& aY) const;
		bool rectangle_fits(size_type aIndex, const size& aDimensions, dimension& aY, dimension& aWastedArea) const;
		dimension compute_wasted_area(size_type aIndex, const size& aDimensions, dimension aY) const;
		void add_waste_map_area(size_type aIndex, const size& aDimensions, dimension aY);
		void add_skyline_level(size_type aIndex, const rect& rect);
		void merge_skylines();

	private:
		size iDimensions;
		bool iAllowRotations;
		skyline_type iSkyLine;
		dimension iUsedSurfaceArea;
	};
}