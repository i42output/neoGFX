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
	status_bar::separator::separator() : 
		widget{}
	{
	}

	neogfx::size_policy status_bar::separator::size_policy() const
	{
		if (widget::has_size_policy())
			return widget::size_policy();
		return neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Expanding };
	}

	size status_bar::separator::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (widget::has_minimum_size())
			return widget::minimum_size(aAvailableSpace);
		return units_converter(*this).from_device_units(size{ 2.0, 2.0 }) + margins().size();
	}

	void status_bar::separator::paint(graphics_context& aGraphicsContext) const
	{
		scoped_units su(*this, aGraphicsContext, units::Pixels);
		rect line = client_rect(false);
		line.deflate(dpi_scale(size{ 0.0, 2.0 }));
		line.cx = 1.0;
		colour ink = (has_foreground_colour() ? foreground_colour() : app::instance().current_style().palette().foreground_colour());
		aGraphicsContext.fill_rect(line, ink.darker(0x40).with_alpha(0x80));
		++line.x;
		aGraphicsContext.fill_rect(line, ink.lighter(0x40).with_alpha(0x80));
	}

	status_bar::keyboard_lock_status::keyboard_lock_status(i_layout& aLayout) :
		widget{ aLayout },
		iLayout{ *this }
	{
		iLayout.add(std::make_shared<separator>());
		auto insertLock = std::make_shared<label>();
		insertLock->text().set_size_hint("Insert");
		iLayout.add(insertLock);
		iLayout.add(std::make_shared<separator>());
		auto capsLock = std::make_shared<label>();
		capsLock->text().set_size_hint("CAP");
		iLayout.add(capsLock);
		iLayout.add(std::make_shared<separator>());
		auto numLock = std::make_shared<label>();
		numLock->text().set_size_hint("NUM");
		iLayout.add(numLock);
		iLayout.add(std::make_shared<separator>());
		auto scrlLock = std::make_shared<label>();
		scrlLock->text().set_size_hint("SCRL");
		iLayout.add(scrlLock);
		iUpdater = std::make_unique<neolib::callback_timer>(app::instance(), [insertLock, capsLock, numLock, scrlLock](neolib::callback_timer& aTimer)
		{
			aTimer.again();
			const auto& keyboard = app::instance().keyboard();
			insertLock->text().set_text((keyboard.locks() & keyboard_locks::InsertLock) == keyboard_locks::InsertLock ?
				"Insert" : std::string{});
			capsLock->text().set_text((keyboard.locks() & keyboard_locks::CapsLock) == keyboard_locks::CapsLock ?
				"CAP" : std::string{});
			numLock->text().set_text((keyboard.locks() & keyboard_locks::NumLock) == keyboard_locks::NumLock ?
				"NUM" : std::string{});
			scrlLock->text().set_text((keyboard.locks() & keyboard_locks::ScrollLock) == keyboard_locks::ScrollLock ?
				"SCRL" : std::string{});
		}, 100);
	}

	neogfx::size_policy status_bar::keyboard_lock_status::size_policy() const
	{
		return neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Expanding };
	}

	status_bar::size_grip::size_grip(i_layout& aLayout) : 
		image_widget{ aLayout }
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
		iMessageLayout{ iNormalLayout },
		iMessageWidget{ iMessageLayout },
		iIdleLayout{ iNormalLayout },
		iIdleWidget{ iIdleLayout },
		iNormalWidgetContainer{ iLayout },
		iNormalWidgetLayout{ iNormalWidgetContainer },
		iPermanentWidgetLayout{ iLayout },
		iKeyboardLockStatus{ iLayout },
		iSizeGrip{ iLayout }
	{
		init();
	}

	status_bar::status_bar(i_layout& aLayout, style aStyle) :
		widget{ aLayout },
		iStyle{ aStyle },
		iLayout{ *this },
		iNormalLayout{ iLayout },
		iMessageLayout{ iNormalLayout },
		iMessageWidget{ iMessageLayout },
		iIdleLayout{ iNormalLayout },
		iIdleWidget{ iIdleLayout },
		iNormalWidgetContainer{ iLayout },
		iNormalWidgetLayout{ iNormalWidgetContainer },
		iPermanentWidgetLayout{ iLayout },
		iKeyboardLockStatus{ iLayout },
		iSizeGrip{ iLayout }
	{
		init();
	}

	bool status_bar::have_message() const
	{
		return iMessage != boost::none || app::instance().help().help_active();
	}

	std::string status_bar::message() const
	{
		if (app::instance().help().help_active())
			return app::instance().help().active_help().help_text();
		else if (iMessage != boost::none)
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
		iNormalWidgetLayout.add(aWidget);
	}

	void status_bar::add_normal_widget_at(widget_index aPosition, i_widget& aWidget)
	{
		iNormalWidgetLayout.add_at(aPosition, aWidget);
	}

	void status_bar::add_normal_widget(std::shared_ptr<i_widget> aWidget)
	{
		iNormalWidgetLayout.add(aWidget);
	}

	void status_bar::add_normal_widget_at(widget_index aPosition, std::shared_ptr<i_widget> aWidget)
	{
		iNormalWidgetLayout.add_at(aPosition, aWidget);
	}

	void status_bar::add_permanent_widget(i_widget& aWidget)
	{
		iPermanentWidgetLayout.add(aWidget);
	}

	void status_bar::add_permanent_widget_at(widget_index aPosition, i_widget& aWidget)
	{
		iPermanentWidgetLayout.add_at(aPosition, aWidget);
	}

	void status_bar::add_permanent_widget(std::shared_ptr<i_widget> aWidget)
	{
		iPermanentWidgetLayout.add(aWidget);
	}

	void status_bar::add_permanent_widget_at(widget_index aPosition, std::shared_ptr<i_widget> aWidget)
	{
		iPermanentWidgetLayout.add_at(aPosition, aWidget);
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

	bool status_bar::can_defer_layout() const
	{
		return true;
	}

	bool status_bar::is_managing_layout() const
	{
		return true;
	}

	void status_bar::init()
	{
		set_margins(neogfx::margins{});
		iLayout.set_margins(neogfx::margins{});
		iLayout.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Minimum });
		iNormalLayout.set_margins(neogfx::margins{});
		iNormalLayout.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Minimum });
		iMessageLayout.set_margins(neogfx::margins{});
		iMessageLayout.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Minimum });
		iMessageWidget.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Minimum });
		iIdleLayout.set_margins(neogfx::margins{});
		iIdleLayout.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Minimum });
		iIdleWidget.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Minimum });
		iNormalWidgetContainer.set_margins(neogfx::margins{});
		iNormalWidgetLayout.set_margins(neogfx::margins{});
		iPermanentWidgetLayout.set_margins(neogfx::margins{});
		auto update_size_grip = [this](style_aspect)
		{
			auto ink1 = (has_foreground_colour() ? foreground_colour() : app::instance().current_style().palette().foreground_colour());
			ink1 = ink1.light() ? ink1.darker(0x40) : ink1.lighter(0x40);
			auto ink2 = ink1.darker(0x30);
			if (iSizeGripTexture == boost::none || iSizeGripTexture->first != ink1)
			{
				const char* sSizeGripTextureImagePattern
				{
					"[13,13]"
					"{0,paper}"
					"{1,ink1}"
					"{2,ink2}"

					"0000000000000"
					"0000000000210"
					"0000000000110"
					"0000000000000"
					"0000000210210"
					"0000000110110"
					"0000000000000"
					"0000210210210"
					"0000110110110"
					"0000000000000"
					"0210210210210"
					"0110110110110"
					"0000000000000"
				};
				const char* sSizeGripHighDpiTextureImagePattern
				{
					"[26,26]"
					"{0,paper}"
					"{1,ink1}"
					"{2,ink2}"

					"00000000000000000000000000"
					"00000000000000000000000000"
					"00000000000000000000221100"
					"00000000000000000000221100"
					"00000000000000000000111100"
					"00000000000000000000111100"
					"00000000000000000000000000"
					"00000000000000000000000000"
					"00000000000000221100221100"
					"00000000000000221100221100"
					"00000000000000111100111100"
					"00000000000000111100111100"
					"00000000000000000000000000"
					"00000000000000000000000000"
					"00000000221100221100221100"
					"00000000221100221100221100"
					"00000000111100111100111100"
					"00000000111100111100111100"
					"00000000000000000000000000"
					"00000000000000000000000000"
					"00221100221100221100221100"
					"00221100221100221100221100"
					"00111100111100111100111100"
					"00111100111100111100111100"
					"00000000000000000000000000"
					"00000000000000000000000000"
				};
				iSizeGripTexture = std::make_pair(ink1, !high_dpi() ?
					neogfx::image{ "neogfx::status_bar::iSizeGripTexture::" + ink1.to_string(), sSizeGripTextureImagePattern, { { "paper", colour{} }, { "ink1", ink1 }, { "ink2", ink2 } } } :
					neogfx::image{ "neogfx::status_bar::iSizeGripHighDpiTexture::" + ink1.to_string(), sSizeGripHighDpiTextureImagePattern, { { "paper", colour{} },{ "ink1", ink1 },{ "ink2", ink2 } }, 2.0 });
			}
			iSizeGrip.set_image(iSizeGripTexture->second);
		};
		iSink += app::instance().surface_manager().dpi_changed([update_size_grip](i_surface&) { update_size_grip(style_aspect::Geometry); });
		iSink += app::instance().current_style_changed(update_size_grip);
		update_size_grip(style_aspect::Colour);
		iSink += app::instance().help().help_activated([this](const i_help_source&) { update_widgets();	});
		iSink += app::instance().help().help_deactivated([this](const i_help_source&) { update_widgets(); });
		update_widgets();
	}

	void status_bar::update_widgets()
	{
		bool showMessage = (iStyle & style::DisplayMessage) == style::DisplayMessage && have_message();
		iMessageWidget.text().set_text(have_message() ? message() : std::string{});
		iMessageWidget.show(showMessage);
		iIdleWidget.show(!showMessage);
		iNormalWidgetContainer.show(!showMessage);
		iKeyboardLockStatus.show((iStyle & style::DisplayKeyboardLocks) == style::DisplayKeyboardLocks);
		iSizeGrip.show((iStyle & style::DisplaySizeGrip) == style::DisplaySizeGrip);
	}
}