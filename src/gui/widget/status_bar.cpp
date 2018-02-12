// status_bar.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015-present, Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/status_bar.hpp>

namespace neogfx
{
	status_bar::size_grip::size_grip(i_layout& aLayout)
		: image_widget{ aLayout }
	{
		set_ignore_mouse_events(false);
		set_placement(cardinal_placement::SouthEast);
	}

	neogfx::size_policy status_bar::size_grip::size_policy() const
	{
		return neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Expanding };
	}

	widget_part status_bar::size_grip::hit_test(const point&) const
	{
		return widget_part::NonClientBorderBottomRight;
	}

	bool status_bar::size_grip::ignore_non_client_mouse_events() const
	{
		return false;
	}

	status_bar::status_bar(i_widget& aParent, style aStyle) :
		widget{ aParent },
		iStyle{ aStyle },
		iLayout{ *this },
		iNormalLayout{ iLayout },
		iMessageWidget{ iNormalLayout },
		iNormalWidgetContainer{ iNormalLayout },
		iNormalWidgetContainerLayout{ iNormalWidgetContainer },
		iIdleWidget{ iNormalWidgetContainerLayout, "Ready" },
		iSpacer{ iNormalWidgetContainerLayout },
		iNormalWidgetLayout{ iNormalWidgetContainerLayout },
		iPermanentWidgetLayout{ iLayout },
		iSizeGrip{ iLayout }
	{
		init();
	}

	status_bar::status_bar(i_layout& aLayout, style aStyle) :
		widget{ aLayout },
		iStyle{ aStyle },
		iLayout{ *this },
		iNormalLayout{ iLayout },
		iMessageWidget{ iNormalLayout },
		iNormalWidgetContainer{ iNormalLayout },
		iNormalWidgetContainerLayout{ iNormalWidgetContainer },
		iIdleWidget{ iNormalWidgetContainerLayout, "Ready" },
		iSpacer{ iNormalWidgetContainerLayout },
		iNormalWidgetLayout{ iNormalWidgetContainerLayout },
		iPermanentWidgetLayout{ iLayout },
		iSizeGrip{ iLayout }
	{
		init();
	}

	bool status_bar::have_message() const
	{
		return iMessage != boost::none;
	}

	const std::string& status_bar::message() const
	{
		if (have_message())
			return *iMessage;
		throw no_message();
	}

	void status_bar::set_message(const std::string& aMessage)
	{
		iMessage = aMessage;
		update_widgets();
	}

	void status_bar::clear_message()
	{
		iMessage = boost::none;
		update_widgets();
	}

	void status_bar::add_normal_widget(i_widget& aWidget)
	{
	}

	void status_bar::add_normal_widget_at(widget_index aPosition, i_widget& aWidget)
	{
	}

	void status_bar::add_normal_widget(std::shared_ptr<i_widget> aWidget)
	{
	}

	void status_bar::add_normal_widget_at(widget_index aPosition, std::shared_ptr<i_widget> aWidget)
	{
	}

	void status_bar::add_permanent_widget(i_widget& aWidget)
	{
	}

	void status_bar::add_permanent_widget_at(widget_index aPosition, i_widget& aWidget)
	{
	}

	void status_bar::add_permanent_widget(std::shared_ptr<i_widget> aWidget)
	{
	}

	void status_bar::add_permanent_widget_at(widget_index aPosition, std::shared_ptr<i_widget> aWidget)
	{
	}

	label& status_bar::message_widget()
	{
		return iMessageWidget;
	}

	label& status_bar::idle_widget()
	{
		return iIdleWidget;
	}

	neogfx::size_policy status_bar::size_policy() const
	{
		return neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Minimum };
	}

	void status_bar::paint(graphics_context& aGraphicsContext) const
	{
	}

	void status_bar::init()
	{
		set_margins(neogfx::margins{});
		iLayout.set_margins(neogfx::margins{});
		iNormalLayout.set_margins(neogfx::margins{});
		iNormalWidgetContainerLayout.set_margins(neogfx::margins{});
		iNormalWidgetLayout.set_margins(neogfx::margins{});
		iPermanentWidgetLayout.set_margins(neogfx::margins{});
		auto update_size_grip = [this](style_aspect)
		{
			auto ink1 = (has_foreground_colour() ? foreground_colour() : app::instance().current_style().palette().foreground_colour());
			ink1 = ink1.light() ? ink1.darker(0x40) : ink1.lighter(0x40);
			auto ink2 = ink1.darker(0x20);
			if (iSizeGripTexture == boost::none || iSizeGripTexture->first != ink1)
			{
				const uint8_t sSizeGripTextureImagePattern[13][13]
				{
					{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
					{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 0 },
					{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0 },
					{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
					{ 0, 0, 0, 0, 0, 0, 0, 2, 1, 0, 2, 1, 0 },
					{ 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0 },
					{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
					{ 0, 0, 0, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0 },
					{ 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0 },
					{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
					{ 0, 2, 1, 0, 2, 1, 0, 2, 1, 0, 2, 1, 0 },
					{ 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0 },
					{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
				};
				iSizeGripTexture = std::make_pair(ink1, neogfx::image{ "neogfx::status_bar::iSizeGripTexture::" + ink1.to_string(), sSizeGripTextureImagePattern, { { 0_u8, colour{} }, { 1_u8, ink1 }, { 2_u8, ink2 } } });
			}
			iSizeGrip.set_image(iSizeGripTexture->second);
		};
		iSink += app::instance().current_style_changed(update_size_grip);
		update_size_grip(style_aspect::Colour);
		update_widgets();
	}

	void status_bar::update_widgets()
	{
		bool showMessage = (iStyle & style::DisplayMessage) == style::DisplayMessage && have_message();
		iMessageWidget.text().set_text(have_message() ? message() : std::string{});
		iMessageWidget.show(showMessage);
		iNormalWidgetContainer.show(!showMessage);
		iSizeGrip.show((iStyle & style::DisplaySizeGrip) == style::DisplaySizeGrip);
	}
}