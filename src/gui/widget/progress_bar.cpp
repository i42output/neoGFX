// progress_bar.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2022 Leigh Johnston.  All Rights Reserved.
  
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

#include <boost/algorithm/string.hpp>
#include <neogfx/gui/layout/spacer.hpp>
#include <neogfx/gui/widget/i_skin_manager.hpp>
#include <neogfx/gui/widget/progress_bar.hpp>

namespace neogfx
{
	progress_bar::bar::bar(progress_bar& aOwner) :
		widget<>{ aOwner.iLayout },
		iOwner{ aOwner }
	{
	}

	size progress_bar::bar::minimum_size(optional_size const& aAvailableSpace) const
	{
		if (has_minimum_size())
			return widget::minimum_size(aAvailableSpace);
		return service<i_skin_manager>().active_skin().preferred_size(skin_element::ProgressBar);
	}

	void progress_bar::bar::paint(i_graphics_context& aGc) const
	{
		service<i_skin_manager>().active_skin().draw_progress_bar(aGc, iOwner, iOwner);
	}

	progress_bar::progress_bar(progress_bar_style aStyle) :
		widget<>{},
		iStyle{ aStyle },
		iLayout{ *this },
		iBar{ *this },
		iLabel{ iLayout },
		iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false }
	{
		init();
	}

	progress_bar::progress_bar(i_widget& aParent, progress_bar_style aStyle) :
		widget<>{ aParent },
		iStyle{ aStyle },
		iLayout{ *this },
		iBar{ *this },
		iLabel{ iLayout },
		iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false }
	{
		init();
	}

	progress_bar::progress_bar(i_layout& aLayout, progress_bar_style aStyle) :
		widget<>{ aLayout },
		iStyle{ aStyle },
		iLayout{ *this },
		iBar{ *this },
		iLabel{ iLayout },
		iAnimator{ *this, [this](widget_timer&) { animate(); }, std::chrono::milliseconds{ 20 }, false }
	{
		init();
	}

	progress_bar_style progress_bar::style() const
	{
		return iStyle;
	}

	void progress_bar::set_style(progress_bar_style aStyle)
	{
		iStyle = aStyle;
		update_layout();
		update();
	}

	i_string const& progress_bar::text() const
	{
		return iText;
	}

	void progress_bar::set_text(i_string const& aText)
	{
		iText = aText;
	}

	scalar progress_bar::value() const
	{
		return iValue;
	}

	void progress_bar::set_value(scalar aValue)
	{
		if (iValue != aValue)
		{
			iValue = aValue;
			changed();
		}
	}

	scalar progress_bar::minimum() const
	{
		return iMinimum;
	}

	void progress_bar::set_minimum(scalar aMinimum)
	{
		if (iMinimum != aMinimum)
		{
			iMinimum = aMinimum;
			changed();
		}
	}

	scalar progress_bar::maximum() const
	{
		return iMaximum;
	}

	void progress_bar::set_maximum(scalar aMaximum)
	{
		if (iMaximum != aMaximum)
		{
			iMaximum = aMaximum;
			changed();
		}
	}

	rect progress_bar::bar_rect() const
	{
		return iBar.client_rect(false);
	}
		
	i_string const& progress_bar::value_as_text() const
	{
		return iValueAsText;
	} 

	void progress_bar::init()
	{
		placement_changed();
	}

	void progress_bar::placement_changed()
	{
		iLayout.remove_all();
		iLayout.add_item_at_position(0u, 0u, iBar);
		iLayout.add_item_at_position(0u, 1u, iLabel);
	}

	void progress_bar::changed()
	{
		std::string updatedText = text().to_std_string();
		boost::replace_all(updatedText, "%pct%", boost::lexical_cast<std::string>(value() / (maximum() - minimum()) * 100.0));
		boost::replace_all(updatedText, "%%", "%");
		iValueAsText = updatedText;

		update();

		iLabel.set_text(value_as_text());
	}

	void progress_bar::animate()
	{
		iAnimator.again();

		update();
	}
}