// group_box.hpp
/*
  neogfx C++ GUI Library
  Copyright(C) 2017 Leigh Johnston
  
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
#include <neogfx/gui/layout/vertical_layout.hpp>
#include <neogfx/gui/widget/label.hpp>
#include "widget.hpp"

namespace neogfx
{
	class group_box : public widget
	{
	public:
		group_box(const std::string& aText = std::string());
		group_box(i_widget& aParent, const std::string& aText = std::string());
		group_box(i_layout& aLayout, const std::string& aText = std::string());
	public:
		const neogfx::label& label() const;
		neogfx::label& label();
		void set_item_layout(i_layout& aItemLayout);
		void set_item_layout(std::shared_ptr<i_layout> aItemLayout);
		const i_layout& item_layout() const;
		i_layout& item_layout();
	public:
		virtual void paint(graphics_context& aGraphicsContext) const;
	private:
		vertical_layout iLayout;
		neogfx::label iLabel;
		std::shared_ptr<i_layout> iItemLayout;
	};
}