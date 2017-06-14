// scrollable_widget.cpp
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

#include <neogfx/neogfx.hpp>
#include <neolib/raii.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/gui/widget/scrollable_widget.hpp>

namespace neogfx
{
	scrollable_widget::scrollable_widget(scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		framed_widget(aFrameStyle),
		iVerticalScrollbar(*this, scrollbar_type::Vertical, aScrollbarStyle),
		iHorizontalScrollbar(*this, scrollbar_type::Horizontal, aScrollbarStyle),
		iIgnoreScrollbarUpdates(0)
	{
		if (has_surface())
			init();
	}
	
	scrollable_widget::scrollable_widget(i_widget& aParent, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		framed_widget(aParent, aFrameStyle),
		iVerticalScrollbar(*this, scrollbar_type::Vertical, aScrollbarStyle),
		iHorizontalScrollbar(*this, scrollbar_type::Horizontal, aScrollbarStyle),
		iIgnoreScrollbarUpdates(0)
	{
		if (has_surface())
			init();
	}
	
	scrollable_widget::scrollable_widget(i_layout& aLayout, scrollbar_style aScrollbarStyle, frame_style aFrameStyle) :
		framed_widget(aLayout, aFrameStyle),
		iVerticalScrollbar(*this, scrollbar_type::Vertical, aScrollbarStyle),
		iHorizontalScrollbar(*this, scrollbar_type::Horizontal, aScrollbarStyle),
		iIgnoreScrollbarUpdates(0)
	{
		if (has_surface())
			init();
	}
	
	scrollable_widget::~scrollable_widget()
	{
	}

	void scrollable_widget::scroll_to(i_widget& aChild)
	{
		(void)aChild;
		/* todo */
	}

	void scrollable_widget::layout_items_completed()
	{
		framed_widget::layout_items_completed();
		if (!layout_items_in_progress() && !iIgnoreScrollbarUpdates)
			update_scrollbar_visibility();
	}

	void scrollable_widget::resized()
	{
		framed_widget::resized();
		if (!layout_items_in_progress() && !iIgnoreScrollbarUpdates)
			update_scrollbar_visibility();
	}

	rect scrollable_widget::client_rect(bool aIncludeMargins) const
	{
		rect result = framed_widget::client_rect(aIncludeMargins);
		if (vertical_scrollbar().visible())
		{
			if (vertical_scrollbar().style() == scrollbar_style::Normal)
				result.cx -= vertical_scrollbar().width(*this);
			else if (vertical_scrollbar().style() == scrollbar_style::Menu)
			{
				result.y += vertical_scrollbar().width(*this);
				result.cy -= vertical_scrollbar().width(*this) * 2.0;
			}
			else if (vertical_scrollbar().style() == scrollbar_style::Scroller)
				result.cy -= vertical_scrollbar().width(*this) * 2.0;
		}
		if (horizontal_scrollbar().visible())
		{
			if (horizontal_scrollbar().style() == scrollbar_style::Normal)
				result.cy -= horizontal_scrollbar().width(*this);
			else if (vertical_scrollbar().style() == scrollbar_style::Menu)
			{
				result.x += horizontal_scrollbar().width(*this);
				result.cx -= horizontal_scrollbar().width(*this) * 2.0;
			}
			else if (vertical_scrollbar().style() == scrollbar_style::Scroller)
				result.cx -= horizontal_scrollbar().width(*this) * 2.0;
		}
		return result;
	}

	void scrollable_widget::paint_non_client_after(graphics_context& aGraphicsContext) const
	{
		framed_widget::paint_non_client_after(aGraphicsContext);
		if (vertical_scrollbar().visible())
			vertical_scrollbar().render(aGraphicsContext);
		if (horizontal_scrollbar().visible())
			horizontal_scrollbar().render(aGraphicsContext);
		if (vertical_scrollbar().visible() && horizontal_scrollbar().visible() && vertical_scrollbar().style() == horizontal_scrollbar().style() && vertical_scrollbar().style() == scrollbar_style::Normal)
		{
			point oldOrigin = aGraphicsContext.origin();
			aGraphicsContext.set_origin(point(0.0, 0.0));
			auto scrollbarColour = background_colour();
			aGraphicsContext.fill_rect(
				rect(
					point(scrollbar_geometry(aGraphicsContext, horizontal_scrollbar()).right(), scrollbar_geometry(aGraphicsContext, iVerticalScrollbar).bottom()), 
					size(scrollbar_geometry(aGraphicsContext, iVerticalScrollbar).width(), scrollbar_geometry(aGraphicsContext, horizontal_scrollbar()).height())),
				scrollbarColour.light() ? scrollbarColour.darker(0x40) : scrollbarColour.lighter(0x40));
			aGraphicsContext.set_origin(oldOrigin);
		}
	}

	void scrollable_widget::mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta)
	{
		bool handledVertical = false;
		bool handledHorizontal = false;
		mouse_wheel verticalSense = mouse_wheel::Vertical;
		mouse_wheel horizontalSense = mouse_wheel::Horizontal;
		if (app::instance().keyboard().is_key_pressed(ScanCode_LSHIFT) || app::instance().keyboard().is_key_pressed(ScanCode_RSHIFT))
			std::swap(verticalSense, horizontalSense);
		if ((aWheel & verticalSense) != mouse_wheel::None && vertical_scrollbar().visible())
			handledVertical = vertical_scrollbar().set_position(vertical_scrollbar().position() + (((verticalSense == mouse_wheel::Vertical ? aDelta.dy : aDelta.dx) >= 0.0 ? -1.0 : 1.0) * vertical_scrollbar().step()));
		if ((aWheel & horizontalSense) != mouse_wheel::None && horizontal_scrollbar().visible())
			handledHorizontal = horizontal_scrollbar().set_position(horizontal_scrollbar().position() + (((horizontalSense == mouse_wheel::Horizontal ? aDelta.dx : aDelta.dy) >= 0.0 ? -1.0 : 1.0) * horizontal_scrollbar().step()));
		mouse_wheel passOn = static_cast<mouse_wheel>(
			aWheel & ((handledVertical ? ~verticalSense : verticalSense) | (handledHorizontal ? ~horizontalSense : horizontalSense)));
		if (passOn != mouse_wheel::None)
			framed_widget::mouse_wheel_scrolled(passOn, aDelta);
	}

	void scrollable_widget::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		if (aButton == mouse_button::Middle)
		{
			bool handled = false;
			if (vertical_scrollbar().visible())
			{
				vertical_scrollbar().track();
				handled = true;
			}
			if (horizontal_scrollbar().visible())
			{
				horizontal_scrollbar().track();
				handled = true;
			}
			if (handled)
				set_capture();
			else
				framed_widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
		}
		else
		{
			framed_widget::mouse_button_pressed(aButton, aPosition, aKeyModifiers);
			if (aButton == mouse_button::Left)
			{
				if (vertical_scrollbar().clicked_element() == i_scrollbar::ElementNone && horizontal_scrollbar().clicked_element() == i_scrollbar::ElementNone)
				{
					if (vertical_scrollbar().visible() && vertical_scrollbar().element_at(*this, aPosition + origin()) != i_scrollbar::ElementNone)
					{
						update(true);
						vertical_scrollbar().click_element(vertical_scrollbar().element_at(*this, aPosition + origin()));
					}
					else if (horizontal_scrollbar().visible() && horizontal_scrollbar().element_at(*this, aPosition + origin()) != i_scrollbar::ElementNone)
					{
						update(true);
						horizontal_scrollbar().click_element(horizontal_scrollbar().element_at(*this, aPosition + origin()));
					}
				}
			}
		}
	}

	void scrollable_widget::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		framed_widget::mouse_button_double_clicked(aButton, aPosition, aKeyModifiers);
		if (aButton == mouse_button::Left)
		{
			if (vertical_scrollbar().clicked_element() == i_scrollbar::ElementNone && horizontal_scrollbar().clicked_element() == i_scrollbar::ElementNone)
			{
				if (vertical_scrollbar().visible() && vertical_scrollbar().element_at(*this, aPosition + origin()) != i_scrollbar::ElementNone)
				{
					update(true);
					vertical_scrollbar().click_element(vertical_scrollbar().element_at(*this, aPosition + origin()));
				}
				else if (horizontal_scrollbar().visible() && horizontal_scrollbar().element_at(*this, aPosition + origin()) != i_scrollbar::ElementNone)
				{
					update(true);
					horizontal_scrollbar().click_element(horizontal_scrollbar().element_at(*this, aPosition + origin()));
				}
			}
		}
	}

	void scrollable_widget::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		framed_widget::mouse_button_released(aButton, aPosition);
		if (aButton == mouse_button::Left)
		{
			if (vertical_scrollbar().clicked_element() != i_scrollbar::ElementNone)
			{
				update(true);
				vertical_scrollbar().unclick_element();
			}
			else if (horizontal_scrollbar().clicked_element() != i_scrollbar::ElementNone)
			{
				update(true);
				horizontal_scrollbar().unclick_element();
			}
		}
		else if (aButton == mouse_button::Middle)
		{
			vertical_scrollbar().untrack();
			horizontal_scrollbar().untrack();
		}
	}

	void scrollable_widget::mouse_moved(const point& aPosition)
	{
		framed_widget::mouse_moved(aPosition);
		vertical_scrollbar().update(*this, aPosition + origin());
		horizontal_scrollbar().update(*this, aPosition + origin());
	}

	void scrollable_widget::mouse_entered()
	{
		framed_widget::mouse_entered();
		vertical_scrollbar().update(*this);
		horizontal_scrollbar().update(*this);
	}

	void scrollable_widget::mouse_left()
	{
		framed_widget::mouse_left();
		vertical_scrollbar().update(*this);
		horizontal_scrollbar().update(*this);
	}

	bool scrollable_widget::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
		bool handled = true;
		switch (aScanCode)
		{
		case ScanCode_LEFT:
			horizontal_scrollbar().set_position(horizontal_scrollbar().position() - horizontal_scrollbar().step());
			break;
		case ScanCode_RIGHT:
			horizontal_scrollbar().set_position(horizontal_scrollbar().position() + horizontal_scrollbar().step());
			break;
		case ScanCode_UP:
			vertical_scrollbar().set_position(vertical_scrollbar().position() - vertical_scrollbar().step());
			break;
		case ScanCode_DOWN:
			vertical_scrollbar().set_position(vertical_scrollbar().position() + vertical_scrollbar().step());
			break;
		case ScanCode_PAGEUP:
			vertical_scrollbar().set_position(vertical_scrollbar().position() - vertical_scrollbar().page());
			break;
		case ScanCode_PAGEDOWN:
			vertical_scrollbar().set_position(vertical_scrollbar().position() + vertical_scrollbar().page());
			break;
		case ScanCode_HOME:
			if (horizontal_scrollbar().visible() && !(aKeyModifiers & KeyModifier_CTRL))
				horizontal_scrollbar().set_position(horizontal_scrollbar().minimum());
			else
				vertical_scrollbar().set_position(vertical_scrollbar().minimum());
			break;
		case ScanCode_END:
			if (horizontal_scrollbar().visible() && !(aKeyModifiers & KeyModifier_CTRL))
				horizontal_scrollbar().set_position(horizontal_scrollbar().maximum());
			else
				vertical_scrollbar().set_position(vertical_scrollbar().maximum());
			break;
		default:
			handled = framed_widget::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
			break;
		}
		return handled;
	}

	const i_scrollbar& scrollable_widget::vertical_scrollbar() const
	{
		return iVerticalScrollbar;
	}

	i_scrollbar& scrollable_widget::vertical_scrollbar()
	{
		return iVerticalScrollbar;
	}

	const i_scrollbar& scrollable_widget::horizontal_scrollbar() const
	{
		return iHorizontalScrollbar;
	}

	i_scrollbar& scrollable_widget::horizontal_scrollbar()
	{
		return iHorizontalScrollbar;
	}

	void scrollable_widget::init()
	{
		scoped_units su(static_cast<scrollable_widget&>(*this), UnitsCentimetres);
		vertical_scrollbar().set_step(std::ceil(units_converter(static_cast<scrollable_widget&>(*this)).to_device_units(1.0)));
		horizontal_scrollbar().set_step(std::ceil(units_converter(static_cast<scrollable_widget&>(*this)).to_device_units(1.0)));
	}

	scrollable_widget::child_widget_scrolling_disposition_e scrollable_widget::scrolling_disposition() const
	{
		return static_cast<child_widget_scrolling_disposition_e>(ScrollChildWidgetVertically | ScrollChildWidgetHorizontally);
	}

	scrollable_widget::child_widget_scrolling_disposition_e scrollable_widget::scrolling_disposition(const i_widget&) const
	{
		return static_cast<child_widget_scrolling_disposition_e>(ScrollChildWidgetVertically | ScrollChildWidgetHorizontally);
	}

	rect scrollable_widget::scrollbar_geometry(const i_units_context& aContext, const i_scrollbar& aScrollbar) const
	{
		switch (aScrollbar.type())
		{
		case scrollbar_type::Vertical:
			if (vertical_scrollbar().style() == scrollbar_style::Normal)
				return convert_units(*this, aContext,
					rect{ window_rect().top_right() - point{aScrollbar.width(*this) + effective_frame_width(), -effective_frame_width()},
						size{aScrollbar.width(*this), window_rect().cy - (horizontal_scrollbar().visible() ? horizontal_scrollbar().width(*this) : 0.0) - effective_frame_width() * 2.0} });
			else // scrollbar_style::Menu
				return convert_units(*this, aContext, window_rect().deflate(size{ effective_frame_width() }));
		case scrollbar_type::Horizontal:
			if (horizontal_scrollbar().style() == scrollbar_style::Normal)
				return convert_units(*this, aContext, 
					rect(window_rect().bottom_left() - point(-effective_frame_width(), aScrollbar.width(*this) + effective_frame_width()), 
						size(window_rect().cx - (vertical_scrollbar().visible() ? vertical_scrollbar().width(*this) : 0.0) - effective_frame_width() * 2.0, aScrollbar.width(*this))));
			else // scrollbar_style::Menu
				return convert_units(*this, aContext, window_rect().deflate(size{ effective_frame_width() }));
		default:
			return rect{};
		}
	}

	void scrollable_widget::scrollbar_updated(const i_scrollbar& aScrollbar, i_scrollbar::update_reason_e)
	{
		if (iIgnoreScrollbarUpdates)
			return;
		point scrollPosition = units_converter(*this).from_device_units(point(static_cast<coordinate>(horizontal_scrollbar().position()), static_cast<coordinate>(vertical_scrollbar().position())));
		if (iOldScrollPosition != scrollPosition)
		{
			for (auto& c : children())
			{
				point delta = -(scrollPosition - iOldScrollPosition);
				if (aScrollbar.type() == scrollbar_type::Horizontal || (scrolling_disposition(*c) & ScrollChildWidgetVertically) == DontScrollChildWidget)
					delta.y = 0.0;
				if (aScrollbar.type() == scrollbar_type::Vertical || (scrolling_disposition(*c) & ScrollChildWidgetHorizontally) == DontScrollChildWidget)
					delta.x = 0.0;
				c->move(c->position() + delta);
			}
			if (aScrollbar.type() == scrollbar_type::Vertical)
			{
				iOldScrollPosition.y = scrollPosition.y;
			}
			else if (aScrollbar.type() == scrollbar_type::Horizontal)
			{
				iOldScrollPosition.x = scrollPosition.x;
			}
		}
		update(true);
	}

	colour scrollable_widget::scrollbar_colour(const i_scrollbar&) const
	{
		return background_colour();
	}

	const i_widget& scrollable_widget::as_widget() const
	{
		return *this;
	}

	void scrollable_widget::update_scrollbar_visibility()
	{
		{
			neolib::scoped_counter sc(iIgnoreScrollbarUpdates);
			update_scrollbar_visibility(UsvStageInit);
			if (client_rect().cx > vertical_scrollbar().width(*this) &&
				client_rect().cy > horizontal_scrollbar().width(*this))
			{
				update_scrollbar_visibility(UsvStageCheckVertical1);
				update_scrollbar_visibility(UsvStageCheckHorizontal);
				update_scrollbar_visibility(UsvStageCheckVertical2);
			}
			update_scrollbar_visibility(UsvStageDone);
		}
		vertical_scrollbar().update(*this);
		horizontal_scrollbar().update(*this);
	}

	void scrollable_widget::update_scrollbar_visibility(usv_stage_e aStage)
	{
		switch (aStage)
		{
		case UsvStageInit:
			if ((scrolling_disposition() & ScrollChildWidgetVertically) == ScrollChildWidgetVertically)
				vertical_scrollbar().hide();
			if ((scrolling_disposition() & ScrollChildWidgetHorizontally) == ScrollChildWidgetHorizontally)
				horizontal_scrollbar().hide();
			iOldScrollPosition = point{};
			iOldScrollbarValues = std::make_pair(vertical_scrollbar().position(), horizontal_scrollbar().position());
			layout_items();
			break;
		case UsvStageCheckVertical1:
		case UsvStageCheckVertical2:
			if ((scrolling_disposition() & ScrollChildWidgetVertically) == ScrollChildWidgetVertically)
			{
				auto cr = client_rect();
				for (auto& c : children())
				{
					if (c->hidden() || c->extents().cx == 0.0 || c->extents().cy == 0.0)
						continue;
					if ((scrolling_disposition(*c) & ScrollChildWidgetVertically) == DontScrollChildWidget)
						continue;
					if (c->position().y < cr.top() || c->position().y + c->extents().cy > cr.bottom())
					{
						vertical_scrollbar().show();
						layout_items();
						break;
					}
				}
			}
			break;
		case UsvStageCheckHorizontal:
			if ((scrolling_disposition() & ScrollChildWidgetHorizontally) == ScrollChildWidgetHorizontally)
			{
				auto cr = client_rect();
				for (auto& c : children())
				{
					if (c->hidden() || c->extents().cx == 0.0 || c->extents().cy == 0.0)
						continue;
					if ((scrolling_disposition(*c) & ScrollChildWidgetHorizontally) == DontScrollChildWidget)
						continue;
					if (c->position().x < cr.left() || c->position().x + c->extents().cx > cr.right())
					{
						horizontal_scrollbar().show();
						layout_items();
						break;
					}
				}
			}
			break;
		case UsvStageDone:
			{
				point max;
				for (auto& c : children())
				{
					if (c->hidden() || c->extents().cx == 0.0 || c->extents().cy == 0.0)
						continue;
					if ((scrolling_disposition(*c) & ScrollChildWidgetHorizontally) == ScrollChildWidgetHorizontally)
						max.x = std::max(max.x, units_converter(*c).to_device_units(c->position().x + c->extents().cx));
					if ((scrolling_disposition(*c) & ScrollChildWidgetVertically) == ScrollChildWidgetVertically)
						max.y = std::max(max.y, units_converter(*c).to_device_units(c->position().y + c->extents().cy));
				}
				if (has_layout())
				{
					if ((scrolling_disposition() & ScrollChildWidgetHorizontally) == ScrollChildWidgetHorizontally)
						max.x += layout().margins().right;
					if ((scrolling_disposition() & ScrollChildWidgetVertically) == ScrollChildWidgetVertically)
						max.y += layout().margins().bottom;
				}
				if ((scrolling_disposition() & ScrollChildWidgetVertically) == ScrollChildWidgetVertically)
				{
					vertical_scrollbar().set_minimum(0.0);
					vertical_scrollbar().set_maximum(max.y);
					vertical_scrollbar().set_page(units_converter(*this).to_device_units(client_rect()).height());
					vertical_scrollbar().set_position(iOldScrollbarValues.first);
				}
				if ((scrolling_disposition() & ScrollChildWidgetHorizontally) == ScrollChildWidgetHorizontally)
				{
					horizontal_scrollbar().set_minimum(0.0);
					horizontal_scrollbar().set_maximum(max.x);
					horizontal_scrollbar().set_page(units_converter(*this).to_device_units(client_rect()).width());
					horizontal_scrollbar().set_position(iOldScrollbarValues.second);
				}
			}
			break;
		default:
			break;
		}
	}
}