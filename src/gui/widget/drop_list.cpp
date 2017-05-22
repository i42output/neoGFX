// drop_list.cpp
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
#include <neogfx/app/app.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/drop_list.hpp>

namespace neogfx
{
	drop_list::drop_list() :
		push_button{ "Drop list widget", push_button_style::DropList }, iEditable{ false }, iDownArrow{ texture{} }
	{
		init();
	}

	drop_list::drop_list(i_widget& aParent) :
		push_button{ aParent, "Drop list widget", push_button_style::DropList }, iEditable{ false }, iDownArrow{ texture{} }
	{
		init();
	}

	drop_list::drop_list(i_layout& aLayout) :
		push_button{ aLayout, "Drop list widget", push_button_style::DropList }, iEditable{ false }, iDownArrow{ texture{} }
	{
		init();
	}

	drop_list::~drop_list()
	{
	}

	bool drop_list::editable() const
	{
		return iEditable;
	}

	void drop_list::set_editable(bool aEditable)
	{
		if (iEditable != aEditable)
		{
			iEditable = aEditable;
		}
	}

	size drop_list::minimum_size(const optional_size& aAvailableSpace) const
	{
		return push_button::minimum_size(aAvailableSpace);
	}

	void drop_list::handle_clicked()
	{
	}

	void drop_list::init()
	{
		set_size_policy(neogfx::size_policy::Minimum);
		label().layout().set_alignment(neogfx::alignment::Left | neogfx::alignment::VCentre);
		auto& s = layout().add_spacer();
		scoped_units su{ s, UnitsPixels };
		s.set_minimum_width(std::max(0.0, 8.0 - label().layout().spacing().cx * 2));
		layout().add_item(iDownArrow);
		update_arrow();
		iSink += app::instance().current_style_changed([this]() { update_arrow(); });
	}

	void drop_list::update_arrow()
	{
		auto ink = app::instance().current_style().text_colour();
		if (iDownArrowTexture == boost::none || iDownArrowTexture->first != ink)
		{
			const uint8_t sDownArrowImagePattern[4][8]
			{
				{ 1, 1, 1, 1, 1, 1, 1, 1 },
				{ 0, 1, 1, 1, 1, 1, 1, 0 },
				{ 0, 0, 1, 1, 1, 1, 0, 0 },
				{ 0, 0, 0, 1, 1, 0, 0, 0 },
			};
			iDownArrowTexture = std::make_pair(ink, neogfx::image{ "neogfx::drop_list::iDownArrowTexture::" + ink.to_string(), sDownArrowImagePattern,{ { 0, colour{} },{ 1, ink } } });
		}
		iDownArrow.set_image(iDownArrowTexture->second);
	}
}