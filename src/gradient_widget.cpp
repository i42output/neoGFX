// gradient_widget.cpp
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
#include "app.hpp"
#include "image.hpp"
#include "colour_dialog.hpp"
#include "context_menu.hpp"
#include "menu.hpp"
#include "gradient_widget.hpp"

namespace neogfx
{
	namespace
	{
		static const dimension ALPHA_PATTERN_SIZE = 4;
		static const dimension SMALL_ALPHA_PATTERN_SIZE = 3;
		static const dimension BORDER_THICKNESS = 1;
		static const dimension BORDER_SPACER_THICKNESS = 1;
		static const dimension BAR_CONTENTS_HEIGHT = 16;
		static const dimension BAR_HEIGHT = BAR_CONTENTS_HEIGHT + (BORDER_THICKNESS * 2) + (BORDER_SPACER_THICKNESS * 2);
		static const dimension STOP_POINTER_HEIGHT = 6;
		static const dimension STOP_WIDTH = 11;
		static const dimension STOP_HEIGHT = STOP_WIDTH + STOP_POINTER_HEIGHT;
		static const dimension CONTROL_HEIGHT = BAR_HEIGHT + STOP_HEIGHT * 2;

		inline void draw_alpha_background(graphics_context& aGraphicsContext, const rect& aRect, dimension aAlphaPatternSize = ALPHA_PATTERN_SIZE)
		{
			aGraphicsContext.scissor_on(aRect);
			for (coordinate x = 0; x < aRect.width(); x += aAlphaPatternSize)
			{
				bool alt = false;
				if (static_cast<uint32_t>((x / aAlphaPatternSize)) % 2 == 1)
					alt = !alt;
				for (coordinate y = 0; y < aRect.height(); y += aAlphaPatternSize)
				{
					aGraphicsContext.fill_rect(rect{ aRect.top_left() + point{ x, y }, size{ aAlphaPatternSize, aAlphaPatternSize } }, alt ? colour{ 160, 160, 160 } : colour{ 255, 255, 255 });
					alt = !alt;
				}
			}
			aGraphicsContext.scissor_off();
		}
	}

	gradient_widget::gradient_widget() : iTracking{false}
	{
		set_margins(neogfx::margins{});
	}

	gradient_widget::gradient_widget(i_widget& aParent) : 
		widget(aParent), iTracking{ false }
	{
		set_margins(neogfx::margins{});
	}

	gradient_widget::gradient_widget(i_layout& aLayout) :
		widget(aLayout), iTracking{ false }
	{
		set_margins(neogfx::margins{});
	}

	neogfx::size_policy gradient_widget::size_policy() const
	{
		return neogfx::size_policy{ neogfx::size_policy::Expanding, neogfx::size_policy::Minimum };
	}

	size gradient_widget::minimum_size(const optional_size& aAvailableSpace) const
	{
		if (has_minimum_size())
			return widget::minimum_size(aAvailableSpace);
		scoped_units su(*this, UnitsPixels);
		return convert_units(*this, su.saved_units(), size(CONTROL_HEIGHT * 3, CONTROL_HEIGHT));
	}

	void gradient_widget::paint(graphics_context& aGraphicsContext) const
	{
		scoped_units su{ *this, aGraphicsContext, UnitsPixels };
		rect rectContents = contents_rect();
		color frameColour = (background_colour().dark() ? background_colour().lighter(0x60) : background_colour().darker(0x60));
		draw_alpha_background(aGraphicsContext, rectContents);
		gradient selection = iSelection;
		selection.set_direction(gradient::Horizontal);
		aGraphicsContext.fill_rect(rectContents, selection);
		rectContents.inflate(size{ BORDER_THICKNESS });
		aGraphicsContext.draw_rect(rectContents, pen(frameColour.mid(background_colour()), BORDER_THICKNESS));
		rectContents.inflate(size{ BORDER_THICKNESS });
		aGraphicsContext.draw_rect(rectContents, pen(frameColour, BORDER_THICKNESS));
		for (gradient::colour_stop_list::const_iterator i = iSelection.colour_stops().begin(); i != iSelection.colour_stops().end(); ++i)
			draw_colour_stop(aGraphicsContext, *i);
		for (gradient::alpha_stop_list::const_iterator i = iSelection.alpha_stops().begin(); i != iSelection.alpha_stops().end(); ++i)
			draw_alpha_stop(aGraphicsContext, *i);
	}

	void gradient_widget::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Right)
		{
			auto stopIter = stop_at(aPosition);
			if (stopIter.is<gradient::colour_stop_list::iterator>())
			{
				auto& stop = *static_variant_cast<gradient::colour_stop_list::iterator>(stopIter);
				action selectColourAction{ "Select stop colour..." };
				selectColourAction.triggered([this, &stop]()
				{
					colour_dialog cd{ *this, stop.second };
					if (cd.exec() == dialog::Accepted)
					{
						stop.second = cd.selected_colour();
						update();
					}
				});
				action deleteStopAction{ "Delete stop" };
				deleteStopAction.triggered([this, &stop]()
				{
					for (auto s = iSelection.colour_stops().begin(); s != iSelection.colour_stops().end(); ++s)
						if (&*s == &stop)
						{
							iSelection.colour_stops().erase(s);
							update();
							break;
						}
				});
				if (iSelection.colour_stops().size() <= 2)
					deleteStopAction.disable();
				iMenu = std::make_unique<context_menu>(*this, aPosition + window_rect().top_left() + surface().surface_position());
				iMenu->menu().add_action(selectColourAction);
				iMenu->menu().add_action(deleteStopAction);
				iMenu->exec();
				iMenu.reset();
			};
		}
	}

	void gradient_widget::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		widget::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left)
		{
			auto stopIter = stop_at(aPosition);
			if (stopIter.is<gradient::colour_stop_list::iterator>())
			{
				auto& stop = *static_variant_cast<gradient::colour_stop_list::iterator>(stopIter);
				colour_dialog cd{ *this, stop.second };
				if (cd.exec() == dialog::Accepted)
				{
					stop.second = cd.selected_colour();
					update();
				}
			}
			else if (stopIter.is<gradient::alpha_stop_list::iterator>())
			{
			}
		}
	}

	void gradient_widget::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		widget::mouse_button_released(aButton, aPosition);
	}

	void gradient_widget::mouse_moved(const point& aPosition)
	{
		widget::mouse_moved(aPosition);
	}

	rect gradient_widget::contents_rect() const
	{
		rect r = client_rect(false);
		r.move(point{ std::floor(STOP_WIDTH / 2), STOP_HEIGHT });
		r.cx = r.width() - STOP_WIDTH;
		r.cy = BAR_HEIGHT;
		r.deflate(size{ BORDER_THICKNESS });
		r.deflate(size{ BORDER_THICKNESS });
		return r;
	}

	gradient_widget::stop_const_iterator gradient_widget::stop_at(const point& aPosition) const
	{
		for (auto i = iSelection.colour_stops().begin(); i != iSelection.colour_stops().end(); ++i)
			if (colour_stop_rect(*i).contains(aPosition))
				return i;
		for (auto i = iSelection.alpha_stops().begin(); i != iSelection.alpha_stops().end(); ++i)
			if (alpha_stop_rect(*i).contains(aPosition))
				return i;
		return stop_const_iterator{};
	}

	gradient_widget::stop_iterator gradient_widget::stop_at(const point& aPosition)
	{
		for (auto i = iSelection.colour_stops().begin(); i != iSelection.colour_stops().end(); ++i)
			if (colour_stop_rect(*i).contains(aPosition))
				return i;
		for (auto i = iSelection.alpha_stops().begin(); i != iSelection.alpha_stops().end(); ++i)
			if (alpha_stop_rect(*i).contains(aPosition))
				return i;
		return stop_iterator{};
	}

	rect gradient_widget::colour_stop_rect(const gradient::colour_stop& aColourStop) const
	{
		rect result = contents_rect();
		result.x = result.left() + std::floor((result.width() - 1.0) * aColourStop.first) - std::floor(STOP_WIDTH / 2);
		result.y = result.bottom() + BORDER_THICKNESS + BORDER_SPACER_THICKNESS;
		result.cx = STOP_WIDTH;
		result.cy = STOP_HEIGHT;
		return result;
	}

	rect gradient_widget::alpha_stop_rect(const gradient::alpha_stop& aAlphaStop) const
	{
		rect result = contents_rect();
		result.x = result.left() + std::floor((result.width() - 1.0) * aAlphaStop.first) - std::floor(STOP_WIDTH / 2);
		result.y = result.top() - BORDER_THICKNESS - BORDER_SPACER_THICKNESS - STOP_HEIGHT;
		result.cx = STOP_WIDTH;
		result.cy = STOP_HEIGHT;
		return result;
	}

	void gradient_widget::draw_colour_stop(graphics_context& aGraphicsContext, const gradient::colour_stop& aColourStop) const
	{
		rect r = colour_stop_rect(aColourStop);
		static const uint8_t stopGlpyhPattern[17][11] =
		{
			{ 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 1, 4, 1, 0, 0, 0, 0 },
			{ 0, 0, 0, 1, 4, 3, 3, 1, 0, 0, 0 },
			{ 0, 0, 1, 4, 3, 3, 3, 3, 1, 0, 0 },
			{ 0, 1, 4, 3, 3, 3, 3, 3, 3, 1, 0 },
			{ 1, 4, 3, 3, 3, 3, 3, 3, 3, 3, 1 },
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{ 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1 },
			{ 1, 2, 9, 9, 9, 9, 9, 9, 9, 2, 1 },
			{ 1, 2, 9, 9, 9, 9, 9, 9, 9, 2, 1 },
			{ 1, 2, 9, 9, 9, 9, 9, 9, 9, 2, 1 },
			{ 1, 2, 9, 9, 9, 9, 9, 9, 9, 2, 1 },
			{ 1, 2, 9, 9, 9, 9, 9, 9, 9, 2, 1 },
			{ 1, 2, 9, 9, 9, 9, 9, 9, 9, 2, 1 },
			{ 1, 2, 9, 9, 9, 9, 9, 9, 9, 2, 1 },
			{ 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1 },
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
		};
		colour transparentColour{ 255, 255, 255, 0 };
		colour backgroundColour = background_colour();
		color frameColour = (background_colour().dark() ? background_colour().lighter(0x60) : background_colour().darker(0x60));
		image stopGlyph{
			stopGlpyhPattern,
			{
				{0, transparentColour},
				{1, frameColour},
				{2, frameColour.mid(backgroundColour)},
				{3, iCurrentColourStop == boost::none || &iSelection.colour_stops()[*iCurrentColourStop] != &aColourStop ? backgroundColour : app::instance().current_style().selection_colour()},
				{4, iCurrentColourStop == boost::none || &iSelection.colour_stops()[*iCurrentColourStop] != &aColourStop ? backgroundColour : app::instance().current_style().selection_colour().lighter(0x40)},
				{9, aColourStop.second}} };
		aGraphicsContext.draw_texture(r.top_left(), texture{ stopGlyph });
	}

	void gradient_widget::draw_alpha_stop(graphics_context& aGraphicsContext, const gradient::alpha_stop& aAlphaStop) const 
	{
		rect r = alpha_stop_rect(aAlphaStop);
		static const uint8_t stopGlpyhPattern[17][11] =
		{
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{ 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1 },
			{ 1, 2, 9, 9, 9, 9, 9, 9, 9, 2, 1 },
			{ 1, 2, 9, 9, 9, 9, 9, 9, 9, 2, 1 },
			{ 1, 2, 9, 9, 9, 9, 9, 9, 9, 2, 1 },
			{ 1, 2, 9, 9, 9, 9, 9, 9, 9, 2, 1 },
			{ 1, 2, 9, 9, 9, 9, 9, 9, 9, 2, 1 },
			{ 1, 2, 9, 9, 9, 9, 9, 9, 9, 2, 1 },
			{ 1, 2, 9, 9, 9, 9, 9, 9, 9, 2, 1 },
			{ 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1 },
			{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
			{ 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 1 },
			{ 0, 1, 2, 3, 3, 3, 3, 3, 3, 1, 0 },
			{ 0, 0, 1, 2, 3, 3, 3, 3, 1, 0, 0 },
			{ 0, 0, 0, 1, 2, 3, 3, 1, 0, 0, 0 },
			{ 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0 },
			{ 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 }
		};
		colour transparentColour{ 255, 255, 255, 0 };
		colour backgroundColour = background_colour();
		color frameColour = (background_colour().dark() ? background_colour().lighter(0x60) : background_colour().darker(0x60));
		draw_alpha_background(aGraphicsContext, rect{ r.top_left() + point{2.0, 2.0}, size{7.0, 7.0} }, SMALL_ALPHA_PATTERN_SIZE);
		image stopGlyph{
			stopGlpyhPattern,
			{
				{ 0, transparentColour },
				{ 1, frameColour },
				{ 2, frameColour.mid(backgroundColour) },
				{ 3, iCurrentAlphaStop == boost::none || &iSelection.alpha_stops()[*iCurrentAlphaStop] != &aAlphaStop ? backgroundColour : app::instance().current_style().selection_colour() },
				{ 4, iCurrentAlphaStop == boost::none || &iSelection.alpha_stops()[*iCurrentAlphaStop] != &aAlphaStop ? backgroundColour : app::instance().current_style().selection_colour().lighter(0x40) },
				{ 9, colour::White.with_alpha(aAlphaStop.second) } } };
		aGraphicsContext.draw_texture(r.top_left(), texture{ stopGlyph });
	}
}