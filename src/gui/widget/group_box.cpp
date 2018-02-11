// group_box.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present Leigh Johnston
  
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
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/group_box.hpp>

namespace neogfx
{
	group_box::group_box(const std::string& aText) : 
		widget(), iLayout{ *this }, iTitle{ std::make_unique<neogfx::label>(iLayout, aText) }
	{
		init();
	}

	group_box::group_box(i_widget& aParent, const std::string& aText) :
		widget(aParent), iLayout{ *this }, iTitle{ std::make_unique<neogfx::label>(iLayout, aText) }
	{
		init();
	}

	group_box::group_box(i_layout& aLayout, const std::string& aText) :
		widget(aLayout), iLayout{ *this }, iTitle{ std::make_unique<neogfx::label>(iLayout, aText) }
	{
		init();
	}

	bool group_box::is_checkable() const
	{
		return iTitle.is<check_box_ptr>();
	}

	void group_box::set_checkable(bool aCheckable)
	{
		if (is_checkable() != aCheckable)
		{
			std::string text = label().text().text();
			iTitle.clear();
			if (aCheckable)
			{
				iTitle = std::make_unique<neogfx::check_box>(text);
				iLayout.add_at(0, *static_variant_cast<check_box_ptr&>(iTitle));
			}
			else
			{
				iTitle = std::make_unique<neogfx::label>(text);
				iLayout.add_at(0, *static_variant_cast<label_ptr&>(iTitle));
			}
		}
	}

	const label& group_box::label() const
	{
		if (iTitle.is<label_ptr>())
			return *static_variant_cast<const label_ptr&>(iTitle);
		else
			return static_variant_cast<const check_box_ptr&>(iTitle)->label();
	}

	label& group_box::label()
	{
		if (iTitle.is<label_ptr>())
			return *static_variant_cast<label_ptr&>(iTitle);
		else
			return static_variant_cast<check_box_ptr&>(iTitle)->label();
	}

	bool group_box::has_check_box() const
	{
		return iTitle.is<check_box_ptr>();
	}

	const check_box& group_box::check_box() const
	{
		if (has_check_box())
			return *static_variant_cast<const check_box_ptr&>(iTitle);
		throw not_checkable();
	}

	check_box& group_box::check_box()
	{
		if (has_check_box())
			return *static_variant_cast<check_box_ptr&>(iTitle);
		throw not_checkable();
	}

	void group_box::set_item_layout(i_layout& aItemLayout)
	{
		if (iItemLayout != nullptr)
			iLayout.remove(*iItemLayout);
		iItemLayout = std::shared_ptr<i_layout>(std::shared_ptr<i_layout>(), &aItemLayout);
		if (aItemLayout.parent() != &iLayout)
			iLayout.add(iItemLayout);
	}

	void group_box::set_item_layout(std::shared_ptr<i_layout> aItemLayout)
	{
		if (iItemLayout != nullptr)
			iLayout.remove(*iItemLayout);
		iItemLayout = aItemLayout;
		if (aItemLayout->parent() != &iLayout)
			iLayout.add(iItemLayout);
	}

	const i_layout& group_box::item_layout() const
	{
		return *iItemLayout;
	}

	i_layout& group_box::item_layout()
	{
		return *iItemLayout;
	}

	neogfx::size_policy group_box::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		else
			return size_policy::Minimum;
	}

	void group_box::paint(graphics_context& aGraphicsContext) const
	{
		widget::paint(aGraphicsContext);
		rect lr{ item_layout().position(), item_layout().extents() };
		lr.inflate(size{ 5.0 });
		aGraphicsContext.fill_rounded_rect(lr, 4.0, border_colour());
		lr.deflate(size{ 1.0 });
		aGraphicsContext.fill_rounded_rect(lr, 4.0, fill_colour());
	}

	colour group_box::border_colour() const
	{
		if (container_background_colour().light())
			return background_colour().darker(24);
		else
			return background_colour().lighter(24);
	}

	colour group_box::fill_colour() const
	{
		return container_background_colour().light() ? border_colour().lighter(24) : border_colour().darker(24);
	}

	colour group_box::background_colour() const
	{
		if (container_background_colour().light())
			return parent().background_colour().darker(24);
		else
			return parent().background_colour().lighter(24);
	}

	class group_box_item_layout : public vertical_layout
	{
	public:
		size minimum_size(const optional_size& aAvailableSpace) const override
		{
			auto result = vertical_layout::minimum_size(aAvailableSpace);
			if (result == size{})
				result = size{ 10.0, 10.0 };
			return result;
		}
	};

	void group_box::init()
	{
		set_margins(neogfx::margins{});
		iLayout.set_margins(neogfx::margins{ 5.0 });
		iLayout.set_spacing(neogfx::size{ 5.0 });
		set_item_layout(std::make_shared<group_box_item_layout>());
	}
}