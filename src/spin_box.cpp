// spin_box.cpp
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

#include "neogfx.hpp"
#include "spin_box.hpp"
#include "app.hpp"

namespace neogfx
{
	spin_box_impl::spin_box_impl() :
		iPrimaryLayout{ *this },
		iTextBox{ iPrimaryLayout },
		iSecondaryLayout{ iPrimaryLayout },
		iStepUpButton{ iSecondaryLayout },
		iStepDownButton{ iSecondaryLayout }
	{
		init();
	}

	spin_box_impl::spin_box_impl(i_widget& aParent) :
		framed_widget(aParent),
		iPrimaryLayout{ *this },
		iTextBox{ iPrimaryLayout },
		iSecondaryLayout{ iPrimaryLayout },
		iStepUpButton{ iSecondaryLayout },
		iStepDownButton{ iSecondaryLayout }
	{
		init();
	}

	spin_box_impl::spin_box_impl(i_layout& aLayout) :
		framed_widget(aLayout),
		iPrimaryLayout{ *this },
		iTextBox{ iPrimaryLayout },
		iSecondaryLayout{ iPrimaryLayout },
		iStepUpButton{ iSecondaryLayout },
		iStepDownButton{ iSecondaryLayout }
	{
		init();
	}

	colour spin_box_impl::frame_colour() const
	{
		if (app::instance().current_style().colour().similar_intensity(background_colour(), 0.03125))
			return framed_widget::frame_colour();
		return app::instance().current_style().colour().mid(background_colour());
	}

	void spin_box_impl::init()
	{
		set_margins(neogfx::margins{});
		iPrimaryLayout.set_margins(neogfx::margins{});
		iSecondaryLayout.set_margins(neogfx::margins{});
		iSecondaryLayout.set_spacing(size{});
		iStepUpButton.set_minimum_size(size{16, 8});
		iStepUpButton.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Expanding });
		iStepDownButton.set_minimum_size(size{16, 8});
		iStepDownButton.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Expanding });
		iTextBox.set_style(framed_widget::NoFrame);
	}
}

