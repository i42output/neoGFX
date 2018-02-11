// mdi_window.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2015-present Leigh Johnston
  
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
#include <string>
#include <neogfx/gui/widget/nested_window.hpp>
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/widget/title_bar.hpp>

namespace neogfx
{
	class mdi_window : public nested_window
	{
	public:
		mdi_window(i_widget& aParent, const size& aDimensions, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		mdi_window(i_widget& aParent, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		mdi_window(i_widget& aParent, const point& aPosition, const size& aDimensions, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		mdi_window(i_widget& aParent, const point& aPosition, const size& aDimensions, const std::string& aWindowTitle, window_style aStyle = window_style::Default, scrollbar_style aScrollbarStyle = scrollbar_style::Normal, frame_style aFrameStyle = frame_style::WindowFrame);
		~mdi_window();
	public:
		void init();
	private:
	};
}