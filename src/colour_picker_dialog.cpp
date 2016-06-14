// colour_picker_dialog.cpp
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
#include "colour_picker_dialog.hpp"

namespace neogfx
{
	colour_picker_dialog::colour_box::colour_box(colour_picker_dialog& aParent, const colour& aColour) :
		framed_widget(aParent, SolidFrame), iParent(aParent), iColour(aColour)
	{
		set_margins(neogfx::margins{});
	}

	size colour_picker_dialog::colour_box::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return framed_widget::minimum_size(aAvailableSpace);
		scoped_units su(*this, UnitsPixels);
		size result = framed_widget::minimum_size(aAvailableSpace);
		result += size{ 16, 14 };
		return result;
	}

	size colour_picker_dialog::colour_box::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (has_maximum_size())
			return framed_widget::maximum_size(aAvailableSpace);
		return minimum_size();
	}

	void colour_picker_dialog::colour_box::paint(graphics_context& aGraphicsContext) const
	{
		framed_widget::paint(aGraphicsContext);
		aGraphicsContext.fill_rect(client_rect(false), iColour);
	}

	colour_picker_dialog::x_picker::x_picker(colour_picker_dialog& aParent) :
		framed_widget(aParent.iRightTopLayout), iParent(aParent)
	{
		set_margins(neogfx::margins{});
	}

	size colour_picker_dialog::x_picker::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return framed_widget::minimum_size(aAvailableSpace);
		scoped_units su(*this, UnitsPixels);
		size result = framed_widget::minimum_size(aAvailableSpace);
		result += size{ 32, 256 };
		return result;
	}

	size colour_picker_dialog::x_picker::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (has_maximum_size())
			return framed_widget::maximum_size(aAvailableSpace);
		return minimum_size();
	}

	void colour_picker_dialog::x_picker::paint(graphics_context& aGraphicsContext) const
	{
		framed_widget::paint(aGraphicsContext);
		scoped_units su(*this, UnitsPixels);
		rect cr = client_rect(false);
		for (uint32_t x = 0; x < cr.height(); ++x)
		{
			double nx = x / (cr.height() - 1.0);
			rect line{ cr.top_left() + point{ 0.0, cr.height() - x - 1.0 }, size{ cr.width(), 1.0 } };
			if (iParent.mode() == ModeHSV)
			{
				hsv_colour hsvColour{ nx * 359.0, 1.0, 1.0 };
				aGraphicsContext.fill_rect(line, hsvColour.to_rgb());
			}
		}
	}

	colour_picker_dialog::yz_picker::yz_picker(colour_picker_dialog& aParent) :
		framed_widget(aParent.iRightTopLayout), iParent(aParent)
	{
		set_margins(neogfx::margins{});
	}

	size colour_picker_dialog::yz_picker::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return framed_widget::minimum_size(aAvailableSpace);
		scoped_units su(*this, UnitsPixels);
		size result = framed_widget::minimum_size(aAvailableSpace);
		result += size{ 256, 256 };
		return result;
	}

	size colour_picker_dialog::yz_picker::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (has_maximum_size())
			return framed_widget::maximum_size(aAvailableSpace);
		return minimum_size();
	}

	void colour_picker_dialog::yz_picker::paint(graphics_context& aGraphicsContext) const
	{
		framed_widget::paint(aGraphicsContext);
		rect cr = client_rect(false);
		for (uint32_t y = 0; y < cr.width(); ++y)
		{
			for (uint32_t z = 0; z < cr.height(); ++z)
			{
				double ny = y / (cr.width() - 1.0);
				double nz = z / (cr.height() - 1.0);
				if (iParent.mode() == ModeHSV)
				{
					hsv_colour hsvColour = iParent.selected_colour().to_hsv();
					hsvColour.set_saturation(ny);
					hsvColour.set_value(nz);
					aGraphicsContext.draw_pixel(cr.top_left() + point{ static_cast<coordinate>(y), cr.height() - z - 1.0 }, hsvColour.to_rgb());
				}
			}
		}
	}

	colour_picker_dialog::colour_selection::colour_selection(colour_picker_dialog& aParent) :
		framed_widget(aParent.iRightBottomLayout), iParent(aParent)
	{
		set_margins(neogfx::margins{});
	}

	size colour_picker_dialog::colour_selection::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return framed_widget::minimum_size(aAvailableSpace);
		scoped_units su(*this, UnitsPixels);
		size result = framed_widget::minimum_size(aAvailableSpace);
		result += size{ 60, 80 };
		return result;
	}

	size colour_picker_dialog::colour_selection::maximum_size(const optional_size& aAvailableSpace) const
	{
		if (has_maximum_size())
			return framed_widget::maximum_size(aAvailableSpace);
		return minimum_size();
	}

	void colour_picker_dialog::colour_selection::paint(graphics_context& aGraphicsContext) const
	{
		framed_widget::paint(aGraphicsContext);
		rect cr = client_rect(false);
		aGraphicsContext.fill_rect(cr, iParent.selected_colour());
		cr.y = cr.cy / 2.0;
		aGraphicsContext.fill_rect(cr, iParent.current_colour());
	}

	colour_picker_dialog::colour_picker_dialog() :
		dialog("Select Colour", Modal | Titlebar | Close),
		iMode(ModeHSV),
		iLayout(*this),
		iLayout2(iLayout),
		iLeftLayout(iLayout2),
		iRightLayout(iLayout2),
		iRightTopLayout(iRightLayout),
		iRightBottomLayout(iRightLayout, alignment::Left | alignment::Top),
		iColourSelection(*this),
		iChannelLayout(iRightBottomLayout),
		iBasicColoursLabel{ iLeftLayout, "&Basic colours" },
		iBasicColoursLayout{ iLeftLayout },
		iSpacer{ iLeftLayout },
		iCustomColoursLayout{ iLeftLayout },
		iCustomColoursLabel{ iLeftLayout, "&Custom colours" },
		iYZPicker(*this),
		iXPicker(*this),
		iH{ *this, *this },
		iS{ *this, *this },
		iV{ *this, *this },
		iR{ *this, *this },
		iG{ *this, *this },
		iB{ *this, *this },
		iA{ *this, *this },
		iRgb{ *this, *this },
		iAddToCustomColours{iRightLayout, "&Add to Custom Colours" }
	{
		init();
	}

	colour_picker_dialog::colour_picker_dialog(i_widget& aParent) :
		dialog(aParent, "Select Colour", Modal | Titlebar | Close),
		iMode(ModeHSV),
		iLayout(*this),
		iLayout2(iLayout),
		iLeftLayout(iLayout2),
		iRightLayout(iLayout2),
		iRightTopLayout(iRightLayout),
		iRightBottomLayout(iRightLayout, alignment::Left | alignment::Top),
		iColourSelection(*this),
		iChannelLayout(iRightBottomLayout),
		iBasicColoursLabel{iLeftLayout, "&Basic colours"},
		iBasicColoursLayout{iLeftLayout},
		iSpacer{iLeftLayout},
		iCustomColoursLayout{iLeftLayout},
		iCustomColoursLabel{iLeftLayout, "&Custom colours"},
		iYZPicker(*this),
		iXPicker(*this),
		iH{ *this, *this },
		iS{ *this, *this },
		iV{ *this, *this },
		iR{ *this, *this },
		iG{ *this, *this },
		iB{ *this, *this },
		iA{ *this, *this },
		iRgb{ *this, *this },
		iAddToCustomColours{iRightLayout, "&Add to Custom Colours"}
	{
		init();
	}

	colour_picker_dialog::~colour_picker_dialog()
	{
	}

	colour_picker_dialog::mode_e colour_picker_dialog::mode() const
	{
		return iMode;
	}

	void colour_picker_dialog::set_mode(mode_e aMode)
	{
		if (iMode != aMode)
		{
			iMode = aMode;
			update();
		}
	}

	colour colour_picker_dialog::current_colour() const
	{
		return iCurrentColour;
	}

	colour colour_picker_dialog::selected_colour() const
	{
		return iSelectedColour;
	}

	void colour_picker_dialog::init()
	{
		scoped_units su(static_cast<framed_widget&>(*this), UnitsPixels);
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
		set_margins(neogfx::margins{16.0});
		iLayout.set_margins(neogfx::margins{});
		iLayout.set_spacing(16.0);
		iLayout2.set_margins(neogfx::margins{});
		iLayout2.set_spacing(16.0);
		iRightLayout.set_spacing(16.0);
		iRightTopLayout.set_spacing(16.0);
		iRightBottomLayout.set_spacing(8.0);
		iChannelLayout.set_spacing(8.0);
		iH.first.label().text().set_text("&Hue:"); iH.second.set_size_policy(size_policy::Minimum); iH.second.text_box().set_hint("000000"); iH.second.set_minimum(0); iH.second.set_maximum(360); iH.second.set_step(1);
		iS.first.label().text().set_text("&Sat:"); iS.second.set_size_policy(size_policy::Minimum); iS.second.text_box().set_hint("000000"); iS.second.set_minimum(0); iS.second.set_maximum(100); iS.second.set_step(1);
		iV.first.label().text().set_text("&Val:"); iV.second.set_size_policy(size_policy::Minimum); iV.second.text_box().set_hint("000000"); iV.second.set_minimum(0); iV.second.set_maximum(100); iV.second.set_step(1);
		iR.first.label().text().set_text("&Red:"); iR.second.set_size_policy(size_policy::Minimum); iR.second.text_box().set_hint("000000"); iR.second.set_minimum(0); iR.second.set_maximum(255); iR.second.set_step(1);
		iG.first.label().text().set_text("&Green:"); iG.second.set_size_policy(size_policy::Minimum); iG.second.text_box().set_hint("000000"); iG.second.set_minimum(0); iG.second.set_maximum(255); iG.second.set_step(1);
		iB.first.label().text().set_text("&Blue:"); iB.second.set_size_policy(size_policy::Minimum); iB.second.text_box().set_hint("000000"); iB.second.set_minimum(0); iB.second.set_maximum(255); iB.second.set_step(1);
		iA.first.label().text().set_text("&Alpha:"); iA.second.set_size_policy(size_policy::Minimum); iA.second.text_box().set_hint("000000"); iA.second.set_minimum(0); iA.second.set_maximum(255); iA.second.set_step(1);
		iRgb.first.text().set_text("&#:"); iRgb.second.set_size_policy(size_policy::Minimum); iRgb.second.set_hint("000000"); 
		iChannelLayout.set_dimensions(4, 4);
		iChannelLayout.add_item(iH.first); iChannelLayout.add_item(iH.second);
		iChannelLayout.add_item(iR.first); iChannelLayout.add_item(iR.second);
		iChannelLayout.add_item(iS.first); iChannelLayout.add_item(iS.second);
		iChannelLayout.add_item(iG.first); iChannelLayout.add_item(iG.second);
		iChannelLayout.add_item(iV.first); iChannelLayout.add_item(iV.second);
		iChannelLayout.add_item(iB.first); iChannelLayout.add_item(iB.second);
		iChannelLayout.add_item(iRgb.first); iChannelLayout.add_item(iRgb.second);
		iChannelLayout.add_item(iA.first); iChannelLayout.add_item(iA.second);
		iBasicColoursLayout.set_dimensions(12, 12);
		for (auto const& basicColour : sBasicColours)
			iBasicColoursLayout.add_item(std::make_shared<colour_box>(*this, basicColour));
		iCustomColoursLayout.set_dimensions(2, 12);
		for (uint32_t i = 0; i < 24; ++i)
			iCustomColoursLayout.add_item(std::make_shared<colour_box>(*this, colour::White));
		button_box().add_button(dialog_button_box::Ok);
		button_box().add_button(dialog_button_box::Cancel);
		resize(minimum_size());
		rect desktopRect{ app::instance().surface_manager().desktop_rect(surface()) };
		move_surface((desktopRect.extents() - surface_size()) / 2.0);
	}
}