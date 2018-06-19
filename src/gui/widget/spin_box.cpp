// spin_box.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/spin_box.hpp>
#include <neogfx/app/app.hpp>

namespace neogfx
{
	spin_box_impl::spin_box_impl() :
		iNormalizedValue{ 0.0 },
		iPrimaryLayout{ *this },
		iTextBox{ iPrimaryLayout },
		iSecondaryLayout{ iPrimaryLayout },
		iStepUpButton{ iSecondaryLayout, std::string{}, push_button_style::SpinBox },
		iStepDownButton{ iSecondaryLayout, std::string{}, push_button_style::SpinBox }
	{
		init();
	}

	spin_box_impl::spin_box_impl(i_widget& aParent) :
		framed_widget(aParent),
		iNormalizedValue{ 0.0 },
		iPrimaryLayout{ *this },
		iTextBox{ iPrimaryLayout },
		iSecondaryLayout{ iPrimaryLayout },
		iStepUpButton{ iSecondaryLayout, std::string{}, push_button_style::SpinBox },
		iStepDownButton{ iSecondaryLayout, std::string{}, push_button_style::SpinBox }
	{
		init();
	}

	spin_box_impl::spin_box_impl(i_layout& aLayout) :
		framed_widget(aLayout),
		iNormalizedValue{ 0.0 },
		iPrimaryLayout{ *this },
		iTextBox{ iPrimaryLayout },
		iSecondaryLayout{ iPrimaryLayout },
		iStepUpButton{ iSecondaryLayout, std::string{}, push_button_style::SpinBox },
		iStepDownButton{ iSecondaryLayout, std::string{}, push_button_style::SpinBox }
	{
		init();
	}

	spin_box_impl::~spin_box_impl()
	{
	}
	
	neogfx::size_policy spin_box_impl::size_policy() const
	{
		if (framed_widget::has_size_policy())
			return framed_widget::size_policy();
		return neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Minimum };
	}

	colour spin_box_impl::frame_colour() const
	{
		if (app::instance().current_style().palette().colour().similar_intensity(background_colour(), 0.03125))
			return framed_widget::frame_colour();
		return app::instance().current_style().palette().colour().mid(background_colour());
	}

	void spin_box_impl::mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta)
	{
		if (aWheel == mouse_wheel::Vertical)
			set_normalized_value(std::max(0.0, std::min(1.0, normalized_value() + (aDelta.dy * normalized_step_value()))), true);
		else
			framed_widget::mouse_wheel_scrolled(aWheel, aDelta);
	}

	bool spin_box_impl::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
		if (aScanCode == ScanCode_UP)
		{
			set_normalized_value(std::max(0.0, std::min(1.0, normalized_value() + normalized_step_value())), true);
			return true;
		}
		else if (aScanCode == ScanCode_DOWN)
		{
			set_normalized_value(std::max(0.0, std::min(1.0, normalized_value() - normalized_step_value())), true);
			return true;
		}
		else
			return framed_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
	}

	const std::string& spin_box_impl::text()
	{
		return iText;
	}

	line_edit& spin_box_impl::text_box()
	{
		return iTextBox;
	}

	void spin_box_impl::set_normalized_value(double aValue, bool aUpdateTextBox)
	{
		aValue = std::max(0.0, std::min(1.0, aValue));
		if (iNormalizedValue != aValue || (iTextBox.text().empty() && aUpdateTextBox))
		{
			iNormalizedValue = aValue;
			if (aUpdateTextBox)
				iTextBox.set_text(value_to_string());
		}
	}

	void spin_box_impl::init()
	{
		set_margins(neogfx::margins{});
		iPrimaryLayout.set_margins(neogfx::margins{});
		iSecondaryLayout.set_margins(neogfx::margins{});
		iSecondaryLayout.set_spacing(size{});
		iStepUpButton.set_margins(neogfx::margins{});
		iStepUpButton.set_minimum_size(dpi_scale(size{15, 7}));
		iStepUpButton.label().image().set_minimum_size(size{ 3.0, 3.0 });
		iStepUpButton.label().image().set_snap(2.0); // up and down buttons want to draw arrow texture at same size so use a snap of 2 pixels
		iStepUpButton.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Expanding });
		iStepUpButton.clicked.set_trigger_type(event_trigger_type::Synchronous);
		iStepUpButton.double_clicked.set_trigger_type(event_trigger_type::Synchronous);
		iStepDownButton.set_margins(neogfx::margins{});
		iStepDownButton.set_minimum_size(dpi_scale(size{15, 7}));
		iStepDownButton.label().image().set_minimum_size(size{ 3.0, 3.0 });
		iStepDownButton.label().image().set_snap(2.0);
		iStepDownButton.set_size_policy(neogfx::size_policy{ neogfx::size_policy::Minimum, neogfx::size_policy::Expanding });
		iStepDownButton.clicked.set_trigger_type(event_trigger_type::Synchronous);
		iStepDownButton.double_clicked.set_trigger_type(event_trigger_type::Synchronous);
		iTextBox.set_style(frame_style::NoFrame);

		iSink += iTextBox.text_filter([this](const std::string& aText, bool& aAccept)
		{
			aAccept = aText.find_first_not_of(valid_text_characters()) == std::string::npos;
			if (!aAccept)
				app::instance().basic_services().system_beep();
		});

		iSink += iTextBox.text_changed([this]()
		{
			auto text = iTextBox.text();
			auto newNormalizedValue = string_to_normalized_value(text);
			if (newNormalizedValue)
			{
				iText = text;
				iTextCursorPos = iTextBox.cursor().position();
				if (*newNormalizedValue < 0.0)
					set_normalized_value(0.0);
				else if (*newNormalizedValue > 1.0)
					set_normalized_value(1.0);
				else
					set_normalized_value(*newNormalizedValue);
			}
			else if (!text.empty())
			{
				iTextBox.set_text(iText);
				iTextBox.cursor().set_position(iTextCursorPos);
				app::instance().basic_services().system_beep();
			}
			else
			{
				iText = text;
				iTextCursorPos = iTextBox.cursor().position();
				set_normalized_value(0.0);
			}
		});

		auto step_up = [this]()
		{
			set_normalized_value(std::max(0.0, std::min(1.0, normalized_value() + normalized_step_value())), true);
			iStepper.emplace(app::instance(), [this](neolib::callback_timer& aTimer)
			{
				aTimer.set_duration(125, true);
				aTimer.again();
				set_normalized_value(std::max(0.0, std::min(1.0, normalized_value() + normalized_step_value())), true);
			}, 500);
		};
		iSink += iStepUpButton.pressed(step_up);
		iSink += iStepUpButton.clicked([this]()
		{
			if (iStepper == boost::none) // key press?
				set_normalized_value(std::max(0.0, std::min(1.0, normalized_value() + normalized_step_value())), true);
		});
		iSink += iStepUpButton.double_clicked(step_up);
		iSink += iStepUpButton.released([this]()
		{
			iStepper = boost::none;
		});

		auto step_down = [this]()
		{
			set_normalized_value(std::max(0.0, std::min(1.0, normalized_value() - normalized_step_value())), true);
			iStepper.emplace(app::instance(), [this](neolib::callback_timer& aTimer)
			{
				aTimer.set_duration(125, true);
				aTimer.again();
				set_normalized_value(std::max(0.0, std::min(1.0, normalized_value() - normalized_step_value())), true);
			}, 500);
		};
		iSink += iStepDownButton.pressed(step_down);
		iSink += iStepDownButton.clicked([this]()
		{
			if (iStepper == boost::none) // key press?
				set_normalized_value(std::max(0.0, std::min(1.0, normalized_value() - normalized_step_value())), true);
		});
		iSink += iStepDownButton.double_clicked(step_down);
		iSink += iStepDownButton.released([this]()
		{
			iStepper = boost::none;
		});

		update_arrows();
		iSink += app::instance().current_style_changed([this](style_aspect aAspect)
		{
			if ((aAspect & style_aspect::Colour) == style_aspect::Colour)
				update_arrows();
		});

		iSink += app::instance().surface_manager().dpi_changed([this](i_surface&) { update_arrows(); });
	}

	void spin_box_impl::update_arrows()
	{
		auto ink = app::instance().current_style().palette().text_colour();
		const char* sUpArrowImagePattern
		{
			"[9,5]"
			"{0,paper}"
			"{1,ink}"

			"000010000"
			"000111000"
			"001111100"
			"011111110"
			"111111111"
		};
		const char* sUpArrowHighDpiImagePattern
		{
			"[18,9]"
			"{0,paper}"
			"{1,ink}"

			"000000001100000000"
			"000000011110000000"
			"000000111111000000"
			"000001111111100000"
			"000011111111110000"
			"000111111111111000"
			"001111111111111100"
			"011111111111111110"
			"111111111111111111"
		};
		iUpArrow = std::make_pair(ink,
			!high_dpi() ?
				image{ "neogfx::spin_box_impl::iUpArrow::" + ink.to_string(), sUpArrowImagePattern,{ { "paper", colour{} },{ "ink", ink } } } :
				image{ "neogfx::spin_box_impl::iUpArrowHighDpi::" + ink.to_string(), sUpArrowHighDpiImagePattern,{ { "paper", colour{} },{ "ink", ink } }, 2.0 });
		const char* sDownArrowImagePattern
		{
			"[9,5]"
			"{0,paper}"
			"{1,ink}"

			"111111111"
			"011111110"
			"001111100"
			"000111000"
			"000010000"
		};
		const char* sDownArrowHighDpiImagePattern
		{
			"[18,9]"
			"{0,paper}"
			"{1,ink}"

			"111111111111111111"
			"011111111111111110"
			"001111111111111100"
			"000111111111111000"
			"000011111111110000"
			"000001111111100000"
			"000000111111000000"
			"000000011110000000"
			"000000001100000000"
		};
		iDownArrow = std::make_pair(ink, 
			!high_dpi() ? 
				image{ "neogfx::spin_box_impl::iDownArrow::" + ink.to_string(), sDownArrowImagePattern,{ { "paper", colour{} },{ "ink", ink } } } :
				image{ "neogfx::spin_box_impl::iDownArrowHighDpi::" + ink.to_string(), sDownArrowHighDpiImagePattern,{ { "paper", colour{} },{ "ink", ink } }, 2.0 });
		iStepUpButton.label().set_placement(label_placement::ImageVertical);
		iStepDownButton.label().set_placement(label_placement::ImageVertical);
		iStepUpButton.image().set_image(iUpArrow->second);
		iStepDownButton.image().set_image(iDownArrow->second);
	}
}

