// group_box.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gui/widget/group_box.hpp>

namespace neogfx
{
	group_box::group_box(const std::string& aText) : 
		widget(), iLayout{ *this }, iLabel{ iLayout, aText }
	{
		iLayout.set_margins(neogfx::margins{ 4.0 });
		iLayout.set_spacing(neogfx::size{ 6.0 });
		set_item_layout(std::make_shared<vertical_layout>());
	}

	group_box::group_box(i_widget& aParent, const std::string& aText) :
		widget(aParent), iLayout{ *this }, iLabel{ iLayout, aText }
	{
		iLayout.set_margins(neogfx::margins{ 4.0 });
		iLayout.set_spacing(neogfx::size{ 6.0 });
		set_item_layout(std::make_shared<vertical_layout>());
	}

	group_box::group_box(i_layout& aLayout, const std::string& aText) :
		widget(aLayout), iLayout{ *this }, iLabel{ iLayout, aText }
	{
		iLayout.set_margins(neogfx::margins{ 4.0 });
		iLayout.set_spacing(neogfx::size{ 6.0 });
		set_item_layout(std::make_shared<vertical_layout>());
	}

	const label& group_box::label() const
	{
		return iLabel;
	}

	label& group_box::label()
	{
		return iLabel;
	}

	void group_box::set_item_layout(i_layout& aItemLayout)
	{
		iItemLayout = std::shared_ptr<i_layout>(std::shared_ptr<i_layout>(), &aItemLayout);
		iLayout.add_item(iItemLayout);
	}

	void group_box::set_item_layout(std::shared_ptr<i_layout> aItemLayout)
	{
		iItemLayout = aItemLayout;
		iLayout.add_item(iItemLayout);
	}

	const i_layout& group_box::item_layout() const
	{
		return *iItemLayout;
	}

	i_layout& group_box::item_layout()
	{
		return *iItemLayout;
	}

	void group_box::paint(graphics_context& aGraphicsContext) const
	{
		widget::paint(aGraphicsContext);
		rect lr{ item_layout().position(), item_layout().extents() };
		lr.inflate(size{ 4.0 });
		aGraphicsContext.fill_rounded_rect(lr, 4.0,
			background_colour().dark() ? background_colour().lighter(64) : background_colour().darker(64));
		lr.deflate(size{ 1.0 });
		aGraphicsContext.fill_rounded_rect(lr, 1.0,
			background_colour().dark() ? background_colour().lighter(48) : background_colour().darker(48));
	}
}