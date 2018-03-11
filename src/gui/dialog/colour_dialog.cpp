// colour_dialog.cpp
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
#include <neogfx/gfx/image.hpp>
#include <neogfx/gui/dialog/colour_dialog.hpp>

namespace neogfx
{
	namespace
	{
		void draw_alpha_background(graphics_context& aGraphicsContext, const rect& aRect)
		{
			const dimension ALPHA_PATTERN_SIZE = 4.0;
			for (coordinate x = 0; x < aRect.width(); x += ALPHA_PATTERN_SIZE)
			{
				bool alt = false;
				if (static_cast<uint32_t>((x / ALPHA_PATTERN_SIZE)) % 2 == 1)
					alt = !alt;
				for (coordinate y = 0; y < aRect.height(); y += ALPHA_PATTERN_SIZE)
				{
					aGraphicsContext.fill_rect(rect{ aRect.top_left() + point{ x, y }, size{ ALPHA_PATTERN_SIZE, ALPHA_PATTERN_SIZE } }, alt ? colour{ 160, 160, 160 } : colour{ 255, 255, 255 });
					alt = !alt;
				}
			}
		}
	}

	colour_dialog::colour_box::colour_box(colour_dialog& aParent, const colour& aColour, const optional_custom_colour_list_iterator& aCustomColour) :
		framed_widget(aParent, frame_style::SolidFrame), iParent(aParent), iColour(aColour), iCustomColour(aCustomColour)
	{
		set_margins(neogfx::margins{});
	}

	size colour_dialog::colour_box::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return framed_widget::minimum_size(aAvailableSpace);
		scoped_units su(*this, units::Millimetres);
		auto result = convert_units(*this, units::Pixels, framed_widget::minimum_size(aAvailableSpace) + size{ 4, 3.5 }).ceil();
		su.restore_saved_units();
		return units_converter(*this).from_device_units(result);
	}

	size colour_dialog::colour_box::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (has_maximum_size())
			return framed_widget::maximum_size(aAvailableSpace);
		return minimum_size();
	}

	void colour_dialog::colour_box::paint(graphics_context& aGraphicsContext) const
	{
		framed_widget::paint(aGraphicsContext);
		aGraphicsContext.fill_rect(client_rect(false), iCustomColour == boost::none ? iColour : **iCustomColour);
		if (iCustomColour != boost::none && iParent.current_custom_colour() == *iCustomColour)
		{
			aGraphicsContext.fill_circle(client_rect(false).centre(), 3, colour::White);
			aGraphicsContext.fill_circle(client_rect(false).centre(), 2, colour::Black);
		}
	}

	void colour_dialog::colour_box::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		framed_widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left)
		{
			if (iCustomColour == boost::none)
				iParent.select_colour(iColour.with_alpha(iParent.selected_colour().alpha()));
			else
			{
				iParent.select_colour(**iCustomColour);
				iParent.set_current_custom_colour(*iCustomColour);
			}
		}
	}

	namespace
	{
		const char* sLeftXPickerCursorImage
		{
			"[9,9]"
			"{0,paper}"
			"{1,black}"
			"{2,white}"

			"111110000"
			"122221000"
			"122222100"
			"122222210"
			"122222221"
			"122222210"
			"122222100"
			"122221000"
			"111110000"
		};
		const char* sLeftXPickerCursorHighDpiImage
		{
			"[19,19]"
			"{0,paper}"
			"{1,black}"
			"{2,white}"

			"1111111111000000000"
			"1222222222100000000"
			"1222222222210000000"
			"1222222222221000000"
			"1222222222222100000"
			"1222222222222210000"
			"1222222222222221000"
			"1222222222222222100"
			"1222222222222222210"
			"1222222222222222221"
			"1222222222222222210"
			"1222222222222222100"
			"1222222222222221000"
			"1222222222222210000"
			"1222222222222100000"
			"1222222222221000000"
			"1222222222210000000"
			"1222222222100000000"
			"1111111111000000000"
		};
		const char* sRightXPickerCursorImage
		{
			"[9,9]"
			"{0,paper}"
			"{1,black}"
			"{2,white}"

			"000011111"
			"000122221"
			"001222221"
			"012222221"
			"122222221"
			"012222221"
			"001222221"
			"000122221"
			"000011111"
		};
		const char* sRightXPickerCursorHighDpiImage
		{
			"[19,19]"
			"{0,paper}"
			"{1,black}"
			"{2,white}"

			"0000000001111111111"
			"0000000012222222221"
			"0000000122222222221"
			"0000001222222222221"
			"0000012222222222221"
			"0000122222222222221"
			"0001222222222222221"
			"0012222222222222221"
			"0122222222222222221"
			"1222222222222222221"
			"0122222222222222221"
			"0012222222222222221"
			"0001222222222222221"
			"0000122222222222221"
			"0000012222222222221"
			"0000001222222222221"
			"0000000122222222221"
			"0000000012222222221"
			"0000000001111111111"
		};

	}

	colour_dialog::x_picker::cursor_widget::cursor_widget(x_picker& aParent, type_e aType) :
		image_widget(
			aParent.iParent.client_widget(),
			neogfx::image{ aType == LeftCursor ? 
				aParent.dpi_select(sLeftXPickerCursorImage, sLeftXPickerCursorHighDpiImage) :
				aParent.dpi_select(sRightXPickerCursorImage, sRightXPickerCursorHighDpiImage),
			{ { "paper", colour{} }, { "black", colour::Black } , { "white", colour::White } } }),
		iParent(aParent)
	{
		set_ignore_mouse_events(false);
		resize(minimum_size());
	}

	void colour_dialog::x_picker::cursor_widget::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		image_widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left)
			iDragOffset = point{ aPosition - client_rect().centre() };
	}

	void colour_dialog::x_picker::cursor_widget::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		image_widget::mouse_button_released(aButton, aPosition);
		if (!capturing())
			iDragOffset = boost::none;
	}

	void colour_dialog::x_picker::cursor_widget::mouse_moved(const point& aPosition)
	{
		if (iDragOffset != boost::none)
		{
			point pt{ aPosition - *iDragOffset };
			pt += position();
			pt -= iParent.position();
			pt += size{ iParent.effective_frame_width() };
			iParent.select(point{ 0.0, pt.y});
		}
	}

	colour_dialog::x_picker::x_picker(colour_dialog& aParent) :
		framed_widget(aParent.iRightTopLayout), 
		iParent(aParent), 
		iTracking(false),
		iLeftCursor(*this, cursor_widget::LeftCursor),
		iRightCursor(*this, cursor_widget::RightCursor)
	{
		set_margins(neogfx::margins{});
		iSink = iParent.selection_changed([this]()
		{
			update_cursors();
			update();
		});
		update_cursors();
	}

	size colour_dialog::x_picker::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return framed_widget::minimum_size(aAvailableSpace);
		scoped_units su{ *this, units::Pixels };
		size result = framed_widget::minimum_size(aAvailableSpace);
		result += dpi_scale(size{ 32, 256 });
		return result;
	}

	size colour_dialog::x_picker::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (has_maximum_size())
			return framed_widget::maximum_size(aAvailableSpace);
		return minimum_size();
	}

	void colour_dialog::x_picker::moved()
	{
		framed_widget::moved();
		update_cursors();
	}

	void colour_dialog::x_picker::resized()
	{
		framed_widget::resized();
		update_cursors();
	}

	void colour_dialog::x_picker::paint(graphics_context& aGraphicsContext) const
	{
		framed_widget::paint(aGraphicsContext);
		scoped_units su{ *this, units::Pixels };
		rect cr = client_rect(false);
		if (iParent.current_channel() == ChannelAlpha)
			draw_alpha_background(aGraphicsContext, cr);
		for (uint32_t y = 0; y < cr.height(); ++y)
		{
			rect line{ cr.top_left() + point{ 0.0, static_cast<coordinate>(y) }, size{ cr.width(), 1.0 } };
			auto r = colour_at_position(point{ 0.0, static_cast<coordinate>(y) * (1.0 / dpi_scale_factor())});
			colour rgb;
			if (r.is<hsv_colour>())
			{
				hsv_colour hsv = static_variant_cast<hsv_colour>(r);
				if (iParent.current_channel() == ChannelHue)
				{
					hsv.set_saturation(1.0);
					hsv.set_value(1.0);
				}
				rgb = hsv.to_rgb();
			}
			else
				rgb = static_variant_cast<colour>(r);
			if (iParent.current_channel() != ChannelAlpha)
				rgb.set_alpha(255);
			aGraphicsContext.fill_rect(line, rgb);
		}
	}

	void colour_dialog::x_picker::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		framed_widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left)
		{
			select(aPosition - client_rect(false).top_left());
			iTracking = true;
		}
	}

	void colour_dialog::x_picker::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		framed_widget::mouse_button_released(aButton, aPosition);
		if (!capturing())
			iTracking = false;
	}

	void colour_dialog::x_picker::mouse_moved(const point& aPosition)
	{
		if (iTracking)
			select(aPosition - client_rect(false).top_left());
	}

	void colour_dialog::x_picker::select(const point& aPosition)
	{
		iParent.select_colour(colour_at_position(aPosition * (1.0 / dpi_scale_factor())), *this);
	}

	colour_dialog::representations colour_dialog::x_picker::colour_at_position(const point& aCursorPos) const
	{
		point pos = aCursorPos.min(point{ 255.0, 255.0 }).max(point{ 0.0, 0.0 });
		switch (iParent.current_channel())
		{
		case ChannelHue:
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				hsv.set_hue((255.0 - pos.y) / 255.0 * 360.0);
				return hsv;
			}
			break;
		case ChannelSaturation:
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				hsv.set_saturation((255.0 - pos.y) / 255.0);
				return hsv;
			}
			break;
		case ChannelValue:
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				hsv.set_value((255.0 - pos.y) / 255.0);
				return hsv;
			}
			break;
		case ChannelRed:
			{
				auto rgb = iParent.selected_colour();
				rgb.set_red(static_cast<colour::component>(255.0 - pos.y));
				return rgb;
			}
			break;
		case ChannelGreen:
			{
				auto rgb = iParent.selected_colour();
				rgb.set_green(static_cast<colour::component>(255.0 - pos.y));
				return rgb;
			}
			break;
		case ChannelBlue:
			{
				auto rgb = iParent.selected_colour();
				rgb.set_blue(static_cast<colour::component>(255.0 - pos.y));
				return rgb;
			}
			break;
		case ChannelAlpha:
			if (iParent.current_mode() == ModeHSV)
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				hsv.set_alpha((255.0 - pos.y) / 255.0);
				return hsv;
			}
			else
			{
				auto rgb = iParent.selected_colour();
				rgb.set_alpha(static_cast<colour::component>(255.0 - pos.y));
				return rgb;
			}
			break;
		default:
			return colour::Black;
		}
	}

	void colour_dialog::x_picker::update_cursors()
	{
		iLeftCursor.move(dpi_scale(current_cursor_position()) + position() + client_rect(false).top_left() + point{ -iLeftCursor.extents().cx - effective_frame_width(), -std::floor(iLeftCursor.client_rect().centre().y) });
		iRightCursor.move(dpi_scale(current_cursor_position()) + position() + client_rect(false).top_right() + point{ effective_frame_width(), -std::floor(iLeftCursor.client_rect().centre().y) });
	}

	point colour_dialog::x_picker::current_cursor_position() const
	{
		switch (iParent.current_channel())
		{
		case ChannelHue:
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				return point{ 0.0, 255.0 - hsv.hue() / 360.0 * 255.0};
			}
			break;
		case ChannelSaturation:
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				return point{ 0.0, 255.0 - hsv.saturation() * 255.0 };
			}
			break;
		case ChannelValue:
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				return point{ 0.0, 255.0 - hsv.value() * 255.0 };
			}
			break;
		case ChannelRed:
			{
				auto rgb = iParent.selected_colour();
				return point{ 0.0, 255.0 - static_cast<coordinate>(rgb.red()) };
			}
			break;
		case ChannelGreen:
			{
				auto rgb = iParent.selected_colour();
				return point{ 0.0, 255.0 - static_cast<coordinate>(rgb.green()) };
			}
			break;
		case ChannelBlue:
			{
				auto rgb = iParent.selected_colour();
				return point{ 0.0, 255.0 - static_cast<coordinate>(rgb.blue()) };
			}
			break;
		case ChannelAlpha:
			if (iParent.current_mode() == ModeHSV)
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				return point{ 0.0, 255.0 - static_cast<coordinate>(hsv.alpha() * 255.0) };
			}
			else
			{
				auto rgb = iParent.selected_colour();
				return point{ 0.0, 255.0 - static_cast<coordinate>(rgb.alpha()) };
			}
			break;
		default:
			return point{};
		}
	}

	colour_dialog::yz_picker::yz_picker(colour_dialog& aParent) :
		framed_widget(aParent.iRightTopLayout), iParent(aParent), iTexture{ image{ size{256, 256}, colour::Black } }, iUpdateTexture{true}, iTracking {
		false
	}
	{
		set_margins(neogfx::margins{});
		iParent.selection_changed([this]
		{
			iUpdateTexture = true;
			update();
		});
	}

	size colour_dialog::yz_picker::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return framed_widget::minimum_size(aAvailableSpace);
		scoped_units su{ *this, units::Pixels };
		size result = framed_widget::minimum_size(aAvailableSpace);
		result += dpi_scale(size{ 256, 256 });
		return result;
	}

	size colour_dialog::yz_picker::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (has_maximum_size())
			return framed_widget::maximum_size(aAvailableSpace);
		return minimum_size();
	}

	void colour_dialog::yz_picker::paint(graphics_context& aGraphicsContext) const
	{
		framed_widget::paint(aGraphicsContext);
		rect cr = client_rect(false);
		if (iUpdateTexture)
		{
			iUpdateTexture = false;
			for (uint32_t y = 0; y < 256; ++y)
			{
				for (uint32_t z = 0; z < 256; ++z)
				{
					auto r = colour_at_position(point{ static_cast<coordinate>(y), static_cast<coordinate>(255 - z) });
					colour rgbColour = (r.is<hsv_colour>() ? static_variant_cast<const hsv_colour&>(r).to_rgb() : static_variant_cast<const colour&>(r));
					iPixels[255 - z][y][0] = rgbColour.red();
					iPixels[255 - z][y][1] = rgbColour.green();
					iPixels[255 - z][y][2] = rgbColour.blue();
					iPixels[255 - z][y][3] = 255; // alpha
				}
			}
			iTexture.set_pixels(rect{ point{}, size{256, 256} }, &iPixels[0][0][0]);
		}
		aGraphicsContext.draw_texture(rect{ cr.top_left(), dpi_scale(size{256.0, 256.0}) }, iTexture);
		point cursor = dpi_scale(current_cursor_position());
		aGraphicsContext.fill_circle(cr.top_left() + cursor, 4.0, iParent.selected_colour());
		aGraphicsContext.draw_circle(cr.top_left() + cursor, 4.0, pen{ iParent.selected_colour().light(0x80) ? colour::Black : colour::White });
	}

	void colour_dialog::yz_picker::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		framed_widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left)
		{
			select(aPosition - client_rect(false).top_left());
			iTracking = true;
		}
	}

	void colour_dialog::yz_picker::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		framed_widget::mouse_button_released(aButton, aPosition);
		if (!capturing())
			iTracking = false;
	}

	void colour_dialog::yz_picker::mouse_moved(const point& aPosition)
	{
		if (iTracking)
			select(aPosition - client_rect(false).top_left());
	}

	void colour_dialog::yz_picker::select(const point& aPosition)
	{
		iParent.select_colour(colour_at_position(aPosition * (1.0 / dpi_scale_factor())), *this);
		iUpdateTexture = false;
	}

	colour_dialog::representations colour_dialog::yz_picker::colour_at_position(const point& aCursorPos) const
	{
		point pos{ std::max(std::min(aCursorPos.x, 255.0), 0.0), std::max(std::min(aCursorPos.y, 255.0), 0.0) };
		switch (iParent.current_channel())
		{
		case ChannelHue:
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				hsv.set_saturation(std::max(std::min(pos.x / 255.0, 1.0), 0.0));
				hsv.set_value(std::max(std::min((255.0 - pos.y) / 255.0, 1.0), 0.0));
				return hsv;
			}
			break;
		case ChannelSaturation:
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				hsv.set_hue(std::max(std::min(pos.x / 255.0 * 360.0, 360.0), 0.0));
				hsv.set_value(std::max(std::min((255.0 - pos.y) / 255.0, 1.0), 0.0));
				return hsv;
			}
			break;
		case ChannelValue:
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				hsv.set_hue(std::max(std::min(pos.x / 255.0 * 360.0, 360.0), 0.0));
				hsv.set_saturation(std::max(std::min((255.0 - pos.y) / 255.0, 1.0), 0.0));
				return hsv;
			}
			break;
		case ChannelRed:
			{
				auto rgb = iParent.selected_colour();
				rgb.set_blue(static_cast<colour::component>(pos.x));
				rgb.set_green(static_cast<colour::component>(255.0 - pos.y));
				return rgb;
			}
			break;
		case ChannelGreen:
			{
				auto rgb = iParent.selected_colour();
				rgb.set_blue(static_cast<colour::component>(pos.x));
				rgb.set_red(static_cast<colour::component>(255.0 - pos.y));
				return rgb;
			}
			break;
		case ChannelBlue:
			{
				auto rgb = iParent.selected_colour();
				rgb.set_red(static_cast<colour::component>(pos.x));
				rgb.set_green(static_cast<colour::component>(255.0 - pos.y));
				return rgb;
			}
			break;
		case ChannelAlpha:
			if (iParent.current_mode() == ModeHSV)
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				hsv.set_saturation(std::max(std::min(pos.x / 255.0, 1.0), 0.0));
				hsv.set_value(std::max(std::min((255.0 - pos.y) / 255.0, 1.0), 0.0));
				return hsv;
			}
			else
			{
				auto rgb = iParent.selected_colour();
				rgb.set_blue(static_cast<colour::component>(pos.x));
				rgb.set_green(static_cast<colour::component>(255.0 - pos.y));
				return rgb;
			}
			break;
		}
		return colour::Black;
	}

	point colour_dialog::yz_picker::current_cursor_position() const
	{
		switch (iParent.current_channel())
		{
		case ChannelHue:
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				return point{ hsv.saturation() * 255.0, (1.0 - hsv.value()) * 255.0 };
			}
			break;
		case ChannelSaturation:
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				return point{ hsv.hue() / 360.0 * 255.0, (1.0 - hsv.value()) * 255.0 };
			}
			break;
		case ChannelValue:
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				return point{ hsv.hue() / 360.0 * 255.0, (1.0 - hsv.saturation()) * 255.0 };
			}
			break;
		case ChannelRed:
			{
				auto rgb = iParent.selected_colour();
				return point{ static_cast<coordinate>(rgb.blue()), static_cast<coordinate>(255 - rgb.green()) };
			}
			break;
		case ChannelGreen:
			{
				auto rgb = iParent.selected_colour();
				return point{ static_cast<coordinate>(rgb.blue()), static_cast<coordinate>(255 - rgb.red()) };
			}
			break;
		case ChannelBlue:
			{
				auto rgb = iParent.selected_colour();
				return point{ static_cast<coordinate>(rgb.red()), static_cast<coordinate>(255 - rgb.green()) };
			}
			break;
		case ChannelAlpha:
			if (iParent.current_mode() == ModeHSV)
			{
				auto hsv = iParent.selected_colour_as_hsv(true);
				return point{ hsv.saturation() * 255.0, (1.0 - hsv.value()) * 255.0 };
			}
			else
			{
				auto rgb = iParent.selected_colour();
				return point{ static_cast<coordinate>(rgb.blue()), static_cast<coordinate>(255 - rgb.green()) };
			}
			break;
		default:
			return point{};
		}
	}
	
	colour_dialog::colour_selection::colour_selection(colour_dialog& aParent) :
		framed_widget(aParent.iRightBottomLayout), iParent(aParent)
	{
		set_margins(neogfx::margins{});
		iParent.selection_changed([this]
		{
			update();
		});
	}

	size colour_dialog::colour_selection::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return framed_widget::minimum_size(aAvailableSpace);
		scoped_units su{ *this, units::Pixels };
		size result = framed_widget::minimum_size(aAvailableSpace);
		result += dpi_scale(size{ 60, 80 });
		return result;
	}

	size colour_dialog::colour_selection::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (has_maximum_size())
			return framed_widget::maximum_size(aAvailableSpace);
		return minimum_size();
	}

	void colour_dialog::colour_selection::paint(graphics_context& aGraphicsContext) const
	{
		framed_widget::paint(aGraphicsContext);
		scoped_units su(*this, aGraphicsContext, units::Pixels);
		rect cr = client_rect(false);
		draw_alpha_background(aGraphicsContext, cr);
		rect top = cr;
		rect bottom = top;
		top.cy = top.cy / 2.0;
		bottom.y = top.bottom();
		bottom.cy = bottom.cy / 2.0;
		aGraphicsContext.fill_rect(top, iParent.selected_colour());
		aGraphicsContext.fill_rect(bottom, iParent.current_colour());
	}

	colour_dialog::colour_dialog(const colour& aCurrentColour) :
		dialog{ "Select Colour"_t, window_style::Modal | window_style::TitleBar | window_style::Close },
		iCurrentChannel{ ChannelHue },
		iCurrentColour{ aCurrentColour },
		iSelectedColour{ aCurrentColour.to_hsv() },
		iCurrentCustomColour{ iCustomColours.end() },
		iUpdatingWidgets{ false },
		iScreenPickerActive{ false },
		iLayout{ client_layout() },
		iLayout2{ iLayout },
		iLeftLayout{ iLayout2 },
		iRightLayout{ iLayout2 },
		iRightTopLayout{ iRightLayout },
		iRightBottomLayout{ iRightLayout, alignment::Left | alignment::Top },
		iColourSelection{ *this },
		iScreenPicker{ iRightBottomLayout },
		iChannelLayout{ iRightBottomLayout },
		iBasicColoursLabel{ iLeftLayout, "&Basic colours"_t },
		iBasicColoursLayout{ iLeftLayout },
		iSpacer{ iLeftLayout },
		iCustomColoursLayout{ iLeftLayout },
		iCustomColoursLabel{ iLeftLayout, "&Custom colours"_t },
		iYZPicker{ *this },
		iXPicker{ *this },
		iH{ *this, *this },
		iS{ *this, *this },
		iV{ *this, *this },
		iR{ *this, *this },
		iG{ *this, *this },
		iB{ *this, *this },
		iA{ *this, *this },
		iRgb{ *this },
		iAddToCustomColours{ iRightLayout, "&Add to Custom Colours"_t }
	{
		init();
	}

	colour_dialog::colour_dialog(i_widget& aParent, const colour& aCurrentColour) :
		dialog{ aParent, "Select Colour"_t, window_style::Modal | window_style::TitleBar | window_style::Close },
		iCurrentChannel{ChannelHue },
		iCurrentColour{aCurrentColour },
		iSelectedColour{aCurrentColour.to_hsv() },
		iCurrentCustomColour{ iCustomColours.end() },
		iUpdatingWidgets{ false },
		iScreenPickerActive{ false },
		iLayout{ client_layout() },
		iLayout2{ iLayout },
		iLeftLayout{ iLayout2 },
		iRightLayout{ iLayout2 },
		iRightTopLayout{ iRightLayout },
		iRightBottomLayout{ iRightLayout, alignment::Left | alignment::Top },
		iColourSelection{ *this },
		iScreenPicker{ iRightBottomLayout },
		iChannelLayout{ iRightBottomLayout },
		iBasicColoursLabel{ iLeftLayout, "&Basic colours"_t },
		iBasicColoursLayout{ iLeftLayout},
		iSpacer{ iLeftLayout},
		iCustomColoursLayout{ iLeftLayout},
		iCustomColoursLabel{ iLeftLayout, "&Custom colours"_t },
		iYZPicker{ *this },
		iXPicker{ *this },
		iH{ *this, *this },
		iS{ *this, *this },
		iV{ *this, *this },
		iR{ *this, *this },
		iG{ *this, *this },
		iB{ *this, *this },
		iA{ *this, *this },
		iRgb{ *this },
		iAddToCustomColours{ iRightLayout, "&Add to Custom Colours"_t }
	{
		init();
	}

	colour_dialog::~colour_dialog()
	{
	}

	colour colour_dialog::current_colour() const
	{
		return iCurrentColour;
	}

	colour colour_dialog::selected_colour() const
	{
		if (iSelectedColour.is<colour>())
			return static_variant_cast<const colour&>(iSelectedColour);
		else
			return static_variant_cast<const hsv_colour&>(iSelectedColour).to_rgb();
	}

	hsv_colour colour_dialog::selected_colour_as_hsv() const
	{
		return selected_colour_as_hsv(true);
	}

	void colour_dialog::select_colour(const colour& aColour)
	{
		select_colour(aColour, *this);
	}

	const colour_dialog::custom_colour_list& colour_dialog::custom_colours() const
	{
		return iCustomColours;
	}

	colour_dialog::custom_colour_list& colour_dialog::custom_colours()
	{
		return iCustomColours;
	}

	void colour_dialog::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		if (aButton == mouse_button::Left && iScreenPickerActive)
		{
			iScreenPickerActive = false;
			surface().as_surface_window().release_capture(*this);
		}
		else
			dialog::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
	}

	neogfx::mouse_cursor colour_dialog::mouse_cursor() const
	{
		if (iScreenPickerActive)
			return mouse_system_cursor::Crosshair;
		else
			return dialog::mouse_cursor();
	}

	void colour_dialog::init()
	{
		scoped_units su(static_cast<framed_widget&>(*this), units::Pixels);
		static const std::set<colour> sBasicColours
		{
			colour::AliceBlue, colour::AntiqueWhite, colour::Aquamarine, colour::Azure, colour::Beige, colour::Bisque, colour::Black, colour::BlanchedAlmond, 
			colour::Blue, colour::BlueViolet, colour::Brown, colour::Burlywood, colour::CadetBlue, colour::Chartreuse, colour::Chocolate, colour::Coral, 
			colour::CornflowerBlue, colour::Cornsilk, colour::Cyan, colour::DarkBlue, colour::DarkCyan, colour::DarkGoldenrod, colour::DarkGray, colour::DarkGreen, 
			colour::DarkKhaki, colour::DarkMagenta, colour::DarkOliveGreen, colour::DarkOrange, colour::DarkOrchid, colour::DarkRed, colour::DarkSalmon, 
			colour::DarkSeaGreen, colour::DarkSlateBlue, colour::DarkSlateGray, colour::DarkTurquoise, colour::DarkViolet, colour::DebianRed, colour::DeepPink, 
			colour::DeepSkyBlue, colour::DimGray, colour::DodgerBlue, colour::Firebrick, colour::FloralWhite, colour::ForestGreen, colour::Gainsboro, 
			colour::GhostWhite, colour::Gold, colour::Goldenrod, colour::Gray, colour::Green, colour::GreenYellow, colour::Honeydew, colour::HotPink, 
			colour::IndianRed, colour::Ivory, colour::Khaki, colour::Lavender, colour::LavenderBlush, colour::LawnGreen, colour::LemonChiffon, colour::LightBlue, 
			colour::LightCoral, colour::LightCyan, colour::LightGoldenrod, colour::LightGoldenrodYellow, colour::LightGray, colour::LightGreen, colour::LightPink, 
			colour::LightSalmon, colour::LightSeaGreen, colour::LightSkyBlue, colour::LightSlateBlue, colour::LightSlateGray, colour::LightSteelBlue, 
			colour::LightYellow, colour::LimeGreen, colour::Linen, colour::Magenta, colour::Maroon, colour::MediumAquamarine, colour::MediumBlue, colour::MediumOrchid, 
			colour::MediumPurple, colour::MediumSeaGreen, colour::MediumSlateBlue, colour::MediumSpringGreen, colour::MediumTurquoise, colour::MediumVioletRed,
			colour::MidnightBlue, colour::MintCream, colour::MistyRose, colour::Moccasin, colour::NavajoWhite, colour::Navy, colour::NavyBlue, colour::OldLace, 
			colour::OliveDrab, colour::Orange, colour::OrangeRed, colour::Orchid, colour::PaleGoldenrod, colour::PaleGreen, colour::PaleTurquoise, colour::PaleVioletRed, 
			colour::PapayaWhip, colour::PeachPuff, colour::Peru, colour::Pink, colour::Plum, colour::PowderBlue, colour::Purple, colour::Red, colour::RosyBrown, 
			colour::RoyalBlue, colour::SaddleBrown, colour::Salmon, colour::SandyBrown, colour::SeaGreen, colour::Seashell, colour::Sienna, colour::SkyBlue, 
			colour::SlateBlue, colour::SlateGray, colour::Snow, colour::SpringGreen, colour::SteelBlue, colour::Tan, colour::Thistle, colour::Tomato, 
			colour::Turquoise, colour::Violet, colour::VioletRed, colour::Wheat, colour::White, colour::WhiteSmoke, colour::Yellow, colour::YellowGreen 
		};
		auto standardSpacing = set_standard_layout(16.0);
		iLayout.set_margins(neogfx::margins{});
		iLayout.set_spacing(standardSpacing);
		iLayout2.set_margins(neogfx::margins{});
		iLayout2.set_spacing(standardSpacing);
		iRightLayout.set_spacing(standardSpacing);
		iRightTopLayout.set_spacing(standardSpacing);
		iRightBottomLayout.set_spacing(standardSpacing / 2.0);
		iChannelLayout.set_spacing(standardSpacing / 2.0);
		iScreenPicker.set_size_policy(size_policy::Minimum);
		iSink += iScreenPicker.async_clicked([&, this]()
		{
			iScreenPickerActive = true;
			surface().as_surface_window().set_capture(*this);
		});
		iH.first.label().text().set_text("&Hue:"_t); iH.second.set_size_policy(size_policy::Minimum); iH.second.text_box().set_hint("359.9"); iH.second.set_minimum(0.0); iH.second.set_maximum(359.9); iH.second.set_step(1);
		iS.first.label().text().set_text("&Sat:"_t); iS.second.set_size_policy(size_policy::Minimum); iS.second.text_box().set_hint("100.0"); iS.second.set_minimum(0.0); iS.second.set_maximum(100.0); iS.second.set_step(1);
		iV.first.label().text().set_text("&Val:"_t); iV.second.set_size_policy(size_policy::Minimum); iV.second.text_box().set_hint("100.0"); iV.second.set_minimum(0.0); iV.second.set_maximum(100.0); iV.second.set_step(1);
		iR.first.label().text().set_text("&Red:"_t); iR.second.set_size_policy(size_policy::Minimum); iR.second.text_box().set_hint("255"); iR.second.set_minimum(0); iR.second.set_maximum(255); iR.second.set_step(1);
		iG.first.label().text().set_text("&Green:"_t); iG.second.set_size_policy(size_policy::Minimum); iG.second.text_box().set_hint("255"); iG.second.set_minimum(0); iG.second.set_maximum(255); iG.second.set_step(1);
		iB.first.label().text().set_text("&Blue:"_t); iB.second.set_size_policy(size_policy::Minimum); iB.second.text_box().set_hint("255"); iB.second.set_minimum(0); iB.second.set_maximum(255); iB.second.set_step(1);
		iA.first.label().text().set_text("&Alpha:"_t); iA.second.set_size_policy(size_policy::Minimum); iA.second.text_box().set_hint("255"); iA.second.set_minimum(0); iA.second.set_maximum(255); iA.second.set_step(1);
		iChannelLayout.set_dimensions(4, 4);
		iChannelLayout.add(iH.first); iChannelLayout.add(iH.second);
		iChannelLayout.add(iR.first); iChannelLayout.add(iR.second);
		iChannelLayout.add(iS.first); iChannelLayout.add(iS.second);
		iChannelLayout.add(iG.first); iChannelLayout.add(iG.second);
		iChannelLayout.add(iV.first); iChannelLayout.add(iV.second);
		iChannelLayout.add(iB.first); iChannelLayout.add(iB.second);
		iChannelLayout.add_span(grid_layout::cell_coordinates{ 0, 3 }, grid_layout::cell_coordinates{ 1, 3 });
		iChannelLayout.add(iRgb); iChannelLayout.add_spacer().set_size_policy(size_policy::Minimum);
		iChannelLayout.add(iA.first); iChannelLayout.add(iA.second);
		iBasicColoursLayout.set_dimensions(12, 12);
		for (auto const& basicColour : sBasicColours)
			iBasicColoursLayout.add(std::make_shared<colour_box>(*this, basicColour));
		iCustomColoursLayout.set_dimensions(2, 12);
		std::fill(iCustomColours.begin(), iCustomColours.end(), colour::White);
		for (auto customColour = iCustomColours.begin(); customColour != iCustomColours.end(); ++customColour)
			iCustomColoursLayout.add(std::make_shared<colour_box>(*this, *customColour, customColour));
		button_box().add_button(standard_button::Ok);
		button_box().add_button(standard_button::Cancel);
		iSink += iH.first.checked([this]() { set_current_channel(ChannelHue); });
		iSink += iS.first.checked([this]() { set_current_channel(ChannelSaturation); });
		iSink += iV.first.checked([this]() { set_current_channel(ChannelValue); });
		iSink += iR.first.checked([this]() { set_current_channel(ChannelRed); });
		iSink += iG.first.checked([this]() { set_current_channel(ChannelGreen); });
		iSink += iB.first.checked([this]() { set_current_channel(ChannelBlue); });
		iSink += iA.first.checked([this]() { set_current_channel(ChannelAlpha); });
		iSink += iH.second.value_changed([this]() { if (iUpdatingWidgets) return; auto hsv = selected_colour_as_hsv(); hsv.set_hue(iH.second.value()); select_colour(hsv, iH.second); });
		iSink += iS.second.value_changed([this]() { if (iUpdatingWidgets) return; auto hsv = selected_colour_as_hsv(); hsv.set_saturation(iS.second.value() / 100.0); select_colour(hsv, iS.second); });
		iSink += iV.second.value_changed([this]() { if (iUpdatingWidgets) return; auto hsv = selected_colour_as_hsv(); hsv.set_value(iV.second.value() / 100.0); select_colour(hsv, iV.second); });
		iSink += iR.second.value_changed([this]() { if (iUpdatingWidgets) return; select_colour(selected_colour().with_red(static_cast<colour::component>(iR.second.value())), iR.second); });
		iSink += iG.second.value_changed([this]() { if (iUpdatingWidgets) return; select_colour(selected_colour().with_green(static_cast<colour::component>(iG.second.value())), iG.second); });
		iSink += iB.second.value_changed([this]() { if (iUpdatingWidgets) return; select_colour(selected_colour().with_blue(static_cast<colour::component>(iB.second.value())), iB.second); });
		iSink += iA.second.value_changed([this]() 
		{ 
			if (iUpdatingWidgets) 
				return;
			if (iSelectedColour.is<colour>())
				select_colour(selected_colour().with_alpha(static_cast<colour::component>(iA.second.value())), iA.second); 
			else
			{
				auto hsv = selected_colour_as_hsv();
				hsv.set_alpha(iA.second.value() / 255.0);
				select_colour(hsv, iA.second);
			}
		});
		iSink += iRgb.text_changed([this]() { if (iUpdatingWidgets) return; select_colour(colour{ iRgb.text() }, iRgb); });

		iSink += iAddToCustomColours.clicked([this]()
		{
			if (iCurrentCustomColour == iCustomColours.end())
				iCurrentCustomColour = iCustomColours.begin();
			*iCurrentCustomColour = selected_colour();
			if (iCurrentCustomColour != iCustomColours.end())
				++iCurrentCustomColour;
			update();
		});

		update_widgets(*this);

		centre_on_parent();
	}

	colour_dialog::mode_e colour_dialog::current_mode() const
	{
		if (iSelectedColour.is<hsv_colour>())
			return ModeHSV;
		else
			return ModeRGB;
	}

	colour_dialog::channel_e colour_dialog::current_channel() const
	{
		return iCurrentChannel;
	}

	void colour_dialog::set_current_channel(channel_e aChannel)
	{
		if (iCurrentChannel != aChannel)
		{
			iCurrentChannel = aChannel;
			selection_changed.trigger();
			update();
		}
	}

	hsv_colour colour_dialog::selected_colour_as_hsv(bool aChangeRepresentation) const
	{
		if (iSelectedColour.is<colour>())
		{
			hsv_colour result = static_variant_cast<const colour&>(iSelectedColour).to_hsv();
			if (aChangeRepresentation)
				iSelectedColour = result;
			return result;
		}
		else
			return static_variant_cast<const hsv_colour&>(iSelectedColour);
	}

	void colour_dialog::select_colour(const representations& aColour, const i_widget& aUpdatingWidget)
	{
		if (iUpdatingWidgets)
			return;
		if (iSelectedColour != aColour)
		{
			iSelectedColour = aColour;
			update_widgets(aUpdatingWidget);
			selection_changed.trigger();
		}
	}

	colour_dialog::custom_colour_list::iterator colour_dialog::current_custom_colour() const
	{
		return iCurrentCustomColour;
	}

	void colour_dialog::set_current_custom_colour(custom_colour_list::iterator aCustomColour)
	{
		iCurrentCustomColour = aCustomColour;
		update_widgets(*this);
		update();
	}

	void colour_dialog::update_widgets(const i_widget& aUpdatingWidget)
	{
		if (iUpdatingWidgets)
			return;
		iUpdatingWidgets = true;
		if (&aUpdatingWidget != &iH.second)
			iH.second.set_value(static_cast<int32_t>(selected_colour_as_hsv(false).hue()));
		if (&aUpdatingWidget != &iS.second)
			iS.second.set_value(static_cast<int32_t>(selected_colour_as_hsv(false).saturation() * 100.0));
		if (&aUpdatingWidget != &iV.second)
			iV.second.set_value(static_cast<int32_t>(selected_colour_as_hsv(false).value() * 100.0));
		if (&aUpdatingWidget != &iR.second)
			iR.second.set_value(selected_colour().red());
		if (&aUpdatingWidget != &iG.second)
			iG.second.set_value(selected_colour().green());
		if (&aUpdatingWidget != &iB.second)
			iB.second.set_value(selected_colour().blue());
		if (&aUpdatingWidget != &iA.second)
			iA.second.set_value(selected_colour().alpha());
		if (&aUpdatingWidget != &iRgb)
			iRgb.set_text(selected_colour().to_hex_string());
		iUpdatingWidgets = false;
	}
}