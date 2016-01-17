// label.hpp
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

#pragma once

#include "neogfx.hpp"
#include "horizontal_layout.hpp"
#include "text_widget.hpp"

namespace neogfx
{
	class label : public widget
	{
	public:
		label(const std::string& aText = std::string(), bool aMultiLine = false, alignment aAlignment = alignment::Left | alignment::VCentre);
		label(i_widget& aParent, const std::string& aText = std::string(), bool aMultiLine = false, alignment aAlignment = alignment::Left | alignment::VCentre);
		label(i_layout& aLayout, const std::string& aText = std::string(), bool aMultiLine = false, alignment aAlignment = alignment::Left | alignment::VCentre);
	public:
		const text_widget& text() const;
		text_widget& text();
	private:
		void handle_alignment();
	private:
		alignment iAlignment;
		horizontal_layout iLayout;
		std::shared_ptr<i_widget> iText;
	};
}