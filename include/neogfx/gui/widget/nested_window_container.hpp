// nested_window_container.hpp
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

#include <neogfx/gui/widget/i_nested_window_container.hpp>

namespace neogfx
{
	class nested_window_container : public i_nested_window_container
	{
	public:
		nested_window_container(i_widget& aWidget);
	public:
		void add(i_nested_window& aWindow) override;
		void remove(i_nested_window& aWindow) override;
	public:
		const i_widget& as_widget() const override;
		i_widget& as_widget() override;
	private:
		i_widget& iWidget;
	};
}