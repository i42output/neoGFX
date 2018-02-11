// line_edit.hpp
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
#include "text_edit.hpp"

namespace neogfx
{
	class line_edit : public text_edit
	{
	public:
		line_edit(frame_style aFrameStyle = frame_style::SolidFrame);
		line_edit(i_widget& aParent, frame_style aFrameStyle = frame_style::SolidFrame);
		line_edit(i_layout& aLayout, frame_style aFrameStyle = frame_style::SolidFrame);
	};
}