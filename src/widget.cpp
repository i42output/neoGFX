// widget.cpp
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
#include <neolib/raii.hpp>
#include "app.hpp"
#include "widget.hpp"
#include "i_layout.hpp"

namespace neogfx
{
	widget::device_metrics_forwarder::device_metrics_forwarder(widget& aOwner) :
		iOwner(aOwner)
	{
	}

	size widget::device_metrics_forwarder::extents() const
	{
		return iOwner.iSize;
	}

	dimension widget::device_metrics_forwarder::horizontal_dpi() const
	{
		return iOwner.surface().horizontal_dpi();
	}

	dimension widget::device_metrics_forwarder::vertical_dpi() const
	{
		return iOwner.surface().vertical_dpi();
	}

	dimension widget::device_metrics_forwarder::em_size() const
	{
		return static_cast<dimension>(iOwner.font().size() / 72.0 * horizontal_dpi());
	}

	widget::widget() :
		iParent(0),
		iLinkBefore(this),
		iLinkAfter(this),
		iDeviceMetricsForwarder(*this),
		iUnitsContext(iDeviceMetricsForwarder),
		iMinimumSize{},
		iMaximumSize{},
		iLayoutInProgress(0),
		iVisible(true),
		iEnabled(true),
		iFocusPolicy(focus_policy::NoFocus),
		iForegroundColour{},
		iBackgroundColour{},
		iIgnoreMouseEvents(false)
	{
	}
	
	widget::widget(i_widget& aParent) :
		iParent(0),
		iLinkBefore(this),
		iLinkAfter(this),
		iDeviceMetricsForwarder(*this),
		iUnitsContext(iDeviceMetricsForwarder),
		iMinimumSize{},
		iMaximumSize{},
		iLayoutInProgress(0),
		iVisible(true),
		iEnabled(true),
		iFocusPolicy(focus_policy::NoFocus),
		iForegroundColour{},
		iBackgroundColour{},
		iIgnoreMouseEvents(false)
	{
		aParent.add_widget(*this);
	}

	widget::widget(i_layout& aLayout) :
		iParent(0),
		iLinkBefore(this),
		iLinkAfter(this),
		iDeviceMetricsForwarder(*this),
		iUnitsContext(iDeviceMetricsForwarder),
		iMinimumSize{},
		iMaximumSize{},
		iLayoutInProgress(0),
		iVisible(true),
		iEnabled(true),
		iFocusPolicy(focus_policy::NoFocus),
		iForegroundColour{},
		iBackgroundColour{},
		iIgnoreMouseEvents(false)
	{
		aLayout.add_item(*this);
	}

	widget::~widget()
	{
		remove_widgets();
		{
			auto layout = iLayout;
			iLayout.reset();
		}
		if (has_parent())
			parent().remove_widget(*this);
		else
			unlink();
	}

	const i_device_metrics& widget::device_metrics() const
	{
		return iDeviceMetricsForwarder;
	}

	units_e widget::units() const
	{
		return iUnitsContext.units();
	}

	units_e widget::set_units(units_e aUnits) const
	{
		return iUnitsContext.set_units(aUnits);
	}

	bool widget::is_root() const
	{
		return false;
	}

	bool widget::has_parent() const
	{
		return iParent != 0;
	}

	const i_widget& widget::parent() const
	{
		if (!has_parent())
			throw no_parent();
		return *iParent;
	}

	i_widget& widget::parent()
	{
		if (!has_parent())
			throw no_parent();
		return *iParent;
	}

	void widget::set_parent(i_widget& aParent)
	{
		if (!is_root())
		{
			bool onSurface = has_surface();
			if (onSurface && &surface() != &aParent.surface())
			{
				surface().widget_removed(*this);
				onSurface = false;
			}
			iParent = &aParent;
			if (!onSurface && has_surface())
				surface().widget_added(*this);
		}
		else
			iParent = &aParent;
		parent_changed();
	}

	void widget::parent_changed()
	{
		if (!is_root() && has_managing_layout())
			managing_layout().layout_items(true);
	}

	const i_widget& widget::ultimate_ancestor() const
	{
		const i_widget* w = this;
		while (w->has_parent())
			w = &w->parent();
		return *w;
	}

	i_widget& widget::ultimate_ancestor()
	{
		i_widget* w = this;
		while (w->has_parent())
			w = &w->parent();
			return *w;
	}

	bool widget::is_ancestor(const i_widget& aWidget) const
	{
		const i_widget* parent = &aWidget;
		while (parent->has_parent())
		{
			parent = &parent->parent();
			if (parent == this)
				return true;
		}
		return false;
	}

	bool widget::is_sibling(const i_widget& aWidget) const
	{
		return has_parent() && aWidget.has_parent() && &parent() == &aWidget.parent();
	}

	i_widget& widget::link_before() const
	{
		return *iLinkBefore;
	}

	void widget::set_link_before(i_widget& aWidget)
	{
		if (iLinkBefore != &aWidget)
		{
			aWidget.set_link_before_ptr(link_before());
			link_before().set_link_after_ptr(aWidget);
			set_link_before_ptr(aWidget);
			link_before().set_link_after_ptr(*this);
		}
	}

	void widget::set_link_before_ptr(i_widget& aWidget)
	{
		iLinkBefore = &aWidget;
	}

	i_widget& widget::link_after() const
	{
		return *iLinkAfter;
	}

	void widget::set_link_after(i_widget& aWidget)
	{
		if (iLinkAfter != &aWidget)
		{
			aWidget.set_link_after_ptr(link_after());
			link_after().set_link_before_ptr(aWidget);
			set_link_after_ptr(aWidget);
			link_after().set_link_before_ptr(*this);
		}
	}

	void widget::set_link_after_ptr(i_widget& aWidget)
	{
		iLinkAfter = &aWidget;
	}

	void widget::unlink()
	{
		link_before().set_link_after_ptr(link_after());
		link_after().set_link_before_ptr(link_before());
		iLinkBefore = this;
		iLinkAfter = this;
	}

	void widget::add_widget(i_widget& aWidget)
	{
		if (aWidget.has_parent() && &aWidget.parent() == this)
			return;
		aWidget.set_parent(*this);
		if (iChildren.empty())
			set_link_after(aWidget);
		else
			iChildren.back()->set_link_after(aWidget);
		iChildren.push_back(std::shared_ptr<i_widget>(std::shared_ptr<i_widget>(), &aWidget));
		if (has_surface())
			surface().widget_added(aWidget);
	}

	void widget::add_widget(std::shared_ptr<i_widget> aWidget)
	{
		if (aWidget->has_parent() && &aWidget->parent() == this)
			return;
		aWidget->set_parent(*this);
		if (iChildren.empty())
			set_link_after(*aWidget);
		else 
			iChildren.back()->set_link_after(*aWidget);
		iChildren.push_back(aWidget);
		if (has_surface())
			surface().widget_added(*aWidget);
	}

	void widget::remove_widget(i_widget& aWidget)
	{
		if (!aWidget.has_parent() && &aWidget.parent() != this)
			throw not_child();
		aWidget.unlink();
		for (auto i = iChildren.begin(); i != iChildren.end(); ++i)
			if (&**i == &aWidget)
			{
				iChildren.erase(i);
				break;
			}
		if (has_surface())
			surface().widget_removed(aWidget);
	}

	void widget::remove_widgets()
	{
		while(!iChildren.empty())
		{
			auto child = iChildren.back();
			iChildren.pop_back();
			if (has_surface())
				surface().widget_removed(*child);
		}
	}

	const widget::widget_list& widget::children() const
	{
		return iChildren;
	}

	bool widget::has_surface() const
	{
		if (has_parent())
			return parent().has_surface();
		return false;
	}

	const i_surface& widget::surface() const
	{
		if (has_parent())
			return parent().surface();
		throw no_parent();
	}

	i_surface& widget::surface()
	{
		if (has_parent())
			return parent().surface();
		throw no_parent();
	}

	bool widget::has_layout() const
	{
		return iLayout.get() != 0;
	}

	void widget::set_layout(i_layout& aLayout)
	{
		iLayout = std::shared_ptr<i_layout>(std::shared_ptr<i_layout>(), &aLayout);
		iLayout->set_owner(this);
		for (auto& c : iChildren)
			iLayout->add_item(c);
	}

	void widget::set_layout(std::shared_ptr<i_layout> aLayout)
	{
		iLayout = aLayout;
		iLayout->set_owner(this);
		for (auto& c : iChildren)
			iLayout->add_item(c);
	}

	const i_layout& widget::layout() const
	{
		if (!iLayout)
			throw no_layout();
		return *iLayout;
	}
	
	i_layout& widget::layout()
	{
		if (!iLayout)
			throw no_layout();
		return *iLayout;
	}

	bool widget::can_defer_layout() const
	{
		return false;
	}

	bool widget::has_managing_layout() const
	{
		const i_widget* w = this;
		while (w->has_parent())
		{
			w = &w->parent();
			if (w->is_managing_layout())
				return true;
		}
		return false;
	}

	const i_widget& widget::managing_layout() const
	{
		const i_widget* w = this;
		while (w->has_parent())
		{
			w = &w->parent();
			if (w->is_managing_layout())
				return *w;
		}
		throw no_managing_layout();
	}

	i_widget& widget::managing_layout()
	{
		return const_cast<i_widget&>(const_cast<const widget*>(this)->managing_layout());
	}

	bool widget::is_managing_layout() const
	{
		return false;
	}

	void widget::layout_items(bool aDefer)
	{
		if (layout_items_in_progress())
			return;
		if (!aDefer)
		{
			if (has_layout())
			{
				layout_items_started();
				if (is_root() && size_policy() != neogfx::size_policy::Manual)
				{
					size desiredSize = extents();
					switch (size_policy().horizontal_size_policy())
					{
					case neogfx::size_policy::Fixed:
					case neogfx::size_policy::Minimum:
						desiredSize.cx = minimum_size(extents()).cx;
						break;
					case neogfx::size_policy::Maximum:
						desiredSize.cx = maximum_size(extents()).cx;
						break;
					default:
						break;
					}
					switch (size_policy().vertical_size_policy())
					{
					case neogfx::size_policy::Fixed:
					case neogfx::size_policy::Minimum:
						desiredSize.cy = minimum_size(extents()).cy;
						break;
					case neogfx::size_policy::Maximum:
						desiredSize.cy = maximum_size(extents()).cy;
						break;
					default:
						break;
					}
					resize(desiredSize);
				}
				layout().layout_items(client_rect(false).top_left(), client_rect(false).extents());
				layout_items_completed();
			}
		}
		else if (can_defer_layout())
		{
			if (!iLayoutTimer)
			{
				iLayoutTimer = std::unique_ptr<neolib::callback_timer>(new neolib::callback_timer(app::instance(), [this](neolib::callback_timer&)
				{
					widget::layout_items();
					iLayoutTimer.reset();
				}, 40));
			}
		}
		else if (has_managing_layout())
		{
			throw widget_cannot_defer_layout();
		}
	}

	void widget::layout_items_started()
	{
		++iLayoutInProgress;
	}

	bool widget::layout_items_in_progress() const
	{
		return iLayoutInProgress != 0;
	}

	void widget::layout_items_completed()
	{
		if (--iLayoutInProgress == 0)
			update();
	}

	logical_coordinate_system widget::logical_coordinate_system() const
	{
		return neogfx::logical_coordinate_system::AutomaticGui;
	}

	point widget::position() const
	{
		return units_converter(*this).from_device_units(iPosition);
	}

	void widget::set_position(const point& aPosition)
	{
		move(aPosition);
	}

	point widget::origin(bool aNonClient) const
	{
		if (has_parent())
			return position() + parent().origin(false);
		else
			return point{};
	}

	void widget::move(const point& aPosition)
	{
		if (iPosition != units_converter(*this).to_device_units(aPosition))
		{
			update();
			iPosition = units_converter(*this).to_device_units(aPosition);
			update();
			moved();
		}
	}

	void widget::moved()
	{
	}
	
	size widget::extents() const
	{
		return units_converter(*this).from_device_units(iSize);
	}

	void widget::set_extents(const size& aSize)
	{
		resize(aSize);
	}

	void widget::resize(const size& aSize)
	{
		if (iSize != units_converter(*this).to_device_units(aSize))
		{
			update();
			iSize = units_converter(*this).to_device_units(aSize);
			update();
			resized();
		}
	}

	void widget::resized()
	{
		layout_items();
	}

	rect widget::window_rect() const
	{
		return rect{origin(true), extents()};
	}

	rect widget::client_rect(bool aIncludeMargins) const
	{
		if (!aIncludeMargins)
			return rect{ margins().top_left(), extents() - margins().size() };
		else
			return rect{ point{}, extents() };
	}

	i_widget& widget::widget_at(const point& aPosition)
	{
		if (client_rect().contains(aPosition))
		{
			for (const auto& c : children())
				if (c->visible() && rect(c->position(), c->extents()).contains(aPosition))
					return c->widget_at(aPosition - c->position());
		}
		return *this;
	}

	bool widget::has_size_policy() const
	{
		return iSizePolicy != boost::none;
	}

	size_policy widget::size_policy() const
	{
		if (has_size_policy())
			return *iSizePolicy;
		else
			return size_policy::Expanding;
	}

	void widget::set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout)
	{
		if (iSizePolicy != aSizePolicy)
		{
			iSizePolicy = aSizePolicy;
			if (aUpdateLayout && has_managing_layout())
				managing_layout().layout_items(true);
		}
	}

	bool widget::has_weight() const
	{
		return iWeight != boost::none;
	}

	size widget::weight() const
	{
		if (has_weight())
			return *iWeight;
		return 1.0;
	}

	void widget::set_weight(const optional_size& aWeight, bool aUpdateLayout)
	{
		if (iWeight != aWeight)
		{
			iWeight = aWeight;
			if (aUpdateLayout && has_managing_layout())
				managing_layout().layout_items(true);
		}
	}

	bool widget::has_minimum_size() const
	{
		return iMinimumSize != boost::none;
	}

	size widget::minimum_size(const optional_size& aAvailableSpace) const
	{
		return has_minimum_size() ?
			units_converter(*this).from_device_units(*iMinimumSize) :
			has_layout() ? 
				layout().minimum_size(aAvailableSpace) + margins().size() :
				size{};
	}

	void widget::set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout)
	{
		optional_size newMinimumSize = (aMinimumSize != boost::none ? units_converter(*this).to_device_units(*aMinimumSize) : optional_size());
		if (iMinimumSize != newMinimumSize)
		{
			iMinimumSize = newMinimumSize;
			if (aUpdateLayout && has_managing_layout())
				managing_layout().layout_items(true);
		}
	}

	bool widget::has_maximum_size() const
	{
		return iMaximumSize != boost::none;
	}

	size widget::maximum_size(const optional_size& aAvailableSpace) const
	{
		return size_policy() == neogfx::size_policy::Minimum || size_policy() == neogfx::size_policy::Fixed ?
			minimum_size(aAvailableSpace) :
			has_maximum_size() ?
				units_converter(*this).from_device_units(*iMaximumSize) :
				has_layout() ?
					layout().maximum_size(aAvailableSpace) :
					size(std::numeric_limits<size::dimension_type>::max(), std::numeric_limits<size::dimension_type>::max());
	}

	void widget::set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout)
	{
		optional_size newMaximumSize = (aMaximumSize != boost::none ? units_converter(*this).to_device_units(*aMaximumSize) : optional_size());
		if (iMaximumSize != newMaximumSize)
		{
			iMaximumSize = newMaximumSize;
			if (aUpdateLayout && has_managing_layout())
				managing_layout().layout_items(true);
		}
	}

	bool widget::has_margins() const
	{
		return iMargins != boost::none;
	}

	margins widget::margins() const
	{
		return units_converter(*this).from_device_units(has_margins() ? *iMargins : app::instance().current_style().margins());
	}

	void widget::set_margins(const optional_margins& aMargins, bool aUpdateLayout)
	{
		optional_margins newMargins = (aMargins != boost::none ? units_converter(*this).to_device_units(*aMargins) : optional_margins());
		if (iMargins != newMargins)
		{
			iMargins = newMargins;
			if (aUpdateLayout && has_managing_layout())
				managing_layout().layout_items(true);
		}
	}

	void widget::update(bool aIncludeNonClient)
	{
		if ((!is_root() && !has_parent()) || !has_surface() || surface().destroyed() || hidden() || layout_items_in_progress())
			return;
		update(aIncludeNonClient ? rect{ origin(true) - origin(), extents() } : client_rect());
	}

	void widget::update(const rect& aUpdateRect)
	{
		if ((!is_root() && !has_parent()) || !has_surface() || surface().destroyed() || hidden() || layout_items_in_progress())
			return;
		if (aUpdateRect.empty())
			return;
		if (iUpdateRects.find(aUpdateRect) == iUpdateRects.end())
		{
			iUpdateRects.insert(aUpdateRect);
			if ((iBackgroundColour == boost::none || iBackgroundColour->alpha() != 0xFF) && has_parent() && has_surface() && &parent().surface() == &surface())
				parent().update(rect(aUpdateRect.position() + position() + (origin() - origin(true)), aUpdateRect.extents()));
			else
				surface().invalidate_surface(aUpdateRect + origin());
			for (auto& c : iChildren)
			{
				if (c->hidden())
					continue;
				rect rectChild(c->position(), c->extents());
				rect intersection = aUpdateRect.intersection(rectChild);
				if (!intersection.empty())
					c->update();
			}
		}
	}

	bool widget::requires_update() const
	{
		if (!surface().native_surface().using_frame_buffer())
			return true;
		else
			return !iUpdateRects.empty();
	}

	rect widget::update_rect() const
	{
		if (iUpdateRects.empty())
			throw no_update_rect();
		rect result = *(iUpdateRects.begin());
		for (const auto& ur : iUpdateRects)
			result = result.combine(ur);
		return result;
	}

	rect widget::default_clip_rect(bool aIncludeNonClient) const
	{
		rect clipRect = window_rect();
		clipRect = clipRect - origin(aIncludeNonClient);
		if (!aIncludeNonClient)
			clipRect = clipRect.intersection(client_rect());
		if (has_parent() && !is_root())
			clipRect = clipRect.intersection(parent().default_clip_rect() - point(origin(aIncludeNonClient) - parent().origin()));
		return clipRect;
	}

	void widget::render(graphics_context& aGraphicsContext) const
	{
		if (effectively_hidden())
		{
			iUpdateRects.clear();
			return;
		}
		if (requires_update())
		{
			aGraphicsContext.set_extents(extents());
			aGraphicsContext.set_origin(origin(true));
			aGraphicsContext.scissor_on(default_clip_rect(true));
			paint_non_client(aGraphicsContext);
			aGraphicsContext.scissor_off();
			aGraphicsContext.set_extents(client_rect().extents());
			aGraphicsContext.set_origin(origin() + client_rect().position());
			aGraphicsContext.scissor_on(default_clip_rect());
			auto savedCoordinateSystem = aGraphicsContext.logical_coordinate_system();
			if (savedCoordinateSystem != logical_coordinate_system())
			{
				aGraphicsContext.set_logical_coordinate_system(logical_coordinate_system());
				if (logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGui)
					aGraphicsContext.set_origin(origin() + client_rect().position());
				else if (logical_coordinate_system() == neogfx::logical_coordinate_system::AutomaticGame)
					aGraphicsContext.set_origin(point{
						(origin() + client_rect().bottom_left()).x,
						surface().extents().cy - (origin() + client_rect().bottom_left()).y });
			}
			painting.trigger(aGraphicsContext);
			paint(aGraphicsContext);
			if (savedCoordinateSystem != aGraphicsContext.logical_coordinate_system())
				aGraphicsContext.set_logical_coordinate_system(savedCoordinateSystem);
			aGraphicsContext.scissor_off();
		}
		iUpdateRects.clear();
		for (auto i = iChildren.rbegin(); i != iChildren.rend(); ++i)
		{
			const auto& c = *i;
			rect rectChild(c->position(), c->extents());
			rect intersection = client_rect().intersection(rectChild);
			if (!intersection.empty())
				c->render(aGraphicsContext);
		}
	}

	bool widget::transparent_background() const
	{
		return !is_root();
	}

	void widget::paint_non_client(graphics_context& aGraphicsContext) const
	{
		if (has_background_colour() || !transparent_background())
		{
			if (surface().native_surface().using_frame_buffer())
				for (const auto& ur : iUpdateRects)
					aGraphicsContext.fill_solid_rect(ur + (origin() - origin(true)), background_colour());
			else
				aGraphicsContext.fill_solid_rect(client_rect() + (origin() - origin(true)), background_colour());
		}
	}

	void widget::paint(graphics_context& aGraphicsContext) const
	{
	}

	bool widget::has_foreground_colour() const
	{
		return iForegroundColour != boost::none;
	}

	colour widget::foreground_colour() const
	{
		if (has_foreground_colour())
			return *iForegroundColour;
		else
			return app::instance().current_style().foreground_colour();
	}

	void widget::set_foreground_colour(const optional_colour& aForegroundColour)
	{
		iForegroundColour = aForegroundColour;
		update();
	}

	bool widget::has_background_colour() const
	{
		return iBackgroundColour != boost::none;
	}

	colour widget::background_colour() const
	{
		if (has_background_colour())
			return *iBackgroundColour;
		else
			return app::instance().current_style().background_colour();
	}

	void widget::set_background_colour(const optional_colour& aBackgroundColour)
	{
		iBackgroundColour = aBackgroundColour;
		update();
	}

	colour widget::container_background_colour() const
	{
		const i_widget* w = this;
		while (w->transparent_background() && w->has_parent())
			w = &w->parent();
		if (!w->transparent_background() && w->has_background_colour())
			return w->background_colour();
		else
			return app::instance().current_style().colour();
	}

	bool widget::has_font() const
	{
		return iFont != boost::none;
	}

	const font& widget::font() const
	{
		if (has_font())
			return *iFont;
		else
			return app::instance().current_style().font();
	}

	void widget::set_font(const optional_font& aFont)
	{
		iFont = aFont;
		if (has_managing_layout())
			managing_layout().layout_items(true);
		update();
	}

	bool widget::visible() const
	{
		return iVisible;
	}

	bool widget::effectively_visible() const
	{
		return visible() && (is_root() || !has_parent() || parent().effectively_visible());
	}

	bool widget::hidden() const
	{
		return !visible();
	}

	bool widget::effectively_hidden() const
	{
		return !effectively_visible();
	}

	void widget::show(bool aVisible)
	{
		if (iVisible != aVisible)
		{
			iVisible = aVisible;
			visibility_changed.trigger();
			if (has_managing_layout())
				managing_layout().layout_items(true);
		}
	}

	void widget::show()
	{
		show(true);
	}

	void widget::hide()
	{
		show(false);
	}

	bool widget::enabled() const
	{
		return iEnabled;
	}

	bool widget::effectively_enabled() const
	{
		return enabled() && (is_root() || !has_parent() || parent().effectively_enabled());
	}
	
	bool widget::disabled() const
	{
		return !enabled();
	}

	bool widget::effectively_disabled() const
	{
		return !effectively_enabled();
	}

	void widget::enable(bool aEnable)
	{
		if (iEnabled != aEnable)
		{
			iEnabled = aEnable;
			update();
		}
	}

	void widget::enable()
	{
		enable(true);
	}

	void widget::disable()
	{
		enable(false);
	}

	bool widget::entered() const
	{
		return surface().has_entered_widget() && &surface().entered_widget() == this;
	}

	bool widget::capturing() const
	{
		return surface().has_capturing_widget() && &surface().capturing_widget() == this;
	}

	void widget::set_capture()
	{
		surface().set_capture(*this);
	}

	void widget::release_capture()
	{
		surface().release_capture(*this);
	}

	void widget::captured()
	{
	}

	void widget::released()
	{
	}

	focus_policy widget::focus_policy() const
	{
		return iFocusPolicy;
	}

	void widget::set_focus_policy(neogfx::focus_policy aFocusPolicy)
	{
		iFocusPolicy = aFocusPolicy;
	}

	bool widget::has_focus() const
	{
		return surface().has_focused_widget() && &surface().focused_widget() == this;
	}

	void widget::set_focus()
	{
		surface().set_focused_widget(*this);
	}

	void widget::release_focus()
	{
		surface().release_focused_widget(*this);
	}

	void widget::focus_gained()
	{
		update();
	}

	void widget::focus_lost()
	{
		update();
	}

	bool widget::ignore_mouse_events() const
	{
		return iIgnoreMouseEvents;
	}

	void widget::set_ignore_mouse_events(bool aIgnoreMouseEvents)
	{
		iIgnoreMouseEvents = aIgnoreMouseEvents;
	}

	void widget::mouse_wheel_scrolled(mouse_wheel aWheel, delta aDelta)
	{
		if (has_parent())
			parent().mouse_wheel_scrolled(aWheel, aDelta);
	}

	void widget::mouse_button_pressed(mouse_button aButton, const point& aPosition)
	{
		if (aButton == mouse_button::Middle && has_parent())
			parent().mouse_button_pressed(aButton, aPosition + position());
		else
		{
			set_capture();
			update();
		}
	}

	void widget::mouse_button_double_clicked(mouse_button aButton, const point& aPosition)
	{
	}

	void widget::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		if (capturing())
			release_capture();
		update();
	}

	void widget::mouse_moved(const point& aPosition)
	{
	}

	void widget::mouse_entered()
	{
	}

	void widget::mouse_left()
	{
	}

	void widget::set_default_mouse_cursor()
	{
		surface().set_mouse_cursor(mouse_system_cursor::Arrow);
	}

	void widget::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
	}

	void widget::key_released(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
	{
	}

	void widget::text_input(const std::string& aText)
	{
	}

	i_widget& widget::widget_for_mouse_event(const point& aPosition)
	{
		if (client_rect().contains(aPosition))
		{
			i_widget* w = &widget_at(aPosition);
			while (w != this && (w->hidden() || w->disabled() || w->ignore_mouse_events()))
			{
				w = &w->parent();
			}
			return *w;
		}
		else
			return *this;
	}

	graphics_context widget::create_graphics_context() const
	{
		return graphics_context(*this);
	}
}

