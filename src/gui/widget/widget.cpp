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

#include <neogfx/neogfx.hpp>
#include <neolib/raii.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include "../window/native/i_native_window.hpp"
#include "../../hid/native/i_native_surface.hpp"

namespace neogfx
{
	class widget::layout_timer : public pause_rendering, neolib::callback_timer
	{
	public:
		layout_timer(i_surface& aSurface, neolib::io_task& aIoTask, std::function<void(callback_timer&)> aCallback) :
			pause_rendering(aSurface), neolib::callback_timer(aIoTask, aCallback, 0)
		{
		}
	};


	widget::device_metrics_forwarder::device_metrics_forwarder(widget& aOwner) :
		iOwner(aOwner)
	{
	}

	bool widget::device_metrics_forwarder::metrics_available() const
	{
		return true;
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
		iSingular{ false },
		iParent{ nullptr },
		iSurface{ nullptr },
		iLinkBefore{ nullptr },
		iLinkAfter{ nullptr },
		iDeviceMetricsForwarder{ *this },
		iUnitsContext{ iDeviceMetricsForwarder },
		iMinimumSize{},
		iMaximumSize{},
		iLayoutInProgress{ 0 },
		iVisible{ true },
		iEnabled{ true },
		iFocusPolicy{ focus_policy::NoFocus },
		iForegroundColour{},
		iBackgroundColour{},
		iIgnoreMouseEvents{ false }
	{
	}
	
	widget::widget(i_widget& aParent) :
		iSingular{ false },
		iParent{ nullptr },
		iSurface{ nullptr },
		iLinkBefore{ nullptr },
		iLinkAfter{ nullptr },
		iDeviceMetricsForwarder{ *this },
		iUnitsContext{ iDeviceMetricsForwarder },
		iMinimumSize{},
		iMaximumSize{},
		iLayoutInProgress{ 0 },
		iVisible{ true },
		iEnabled{ true },
		iFocusPolicy{ focus_policy::NoFocus },
		iForegroundColour{},
		iBackgroundColour{},
		iIgnoreMouseEvents{ false }
	{
		aParent.add_widget(*this);
	}

	widget::widget(i_layout& aLayout) :
		iSingular{ false },
		iParent{ nullptr },
		iSurface{ nullptr },
		iLinkBefore{ nullptr },
		iLinkAfter{ nullptr },
		iDeviceMetricsForwarder{ *this },
		iUnitsContext{ iDeviceMetricsForwarder },
		iMinimumSize{},
		iMaximumSize{},
		iLayoutInProgress{ 0 },
		iVisible{ true },
		iEnabled{ true },
		iFocusPolicy{ focus_policy::NoFocus },
		iForegroundColour{},
		iBackgroundColour{},
		iIgnoreMouseEvents{ false }
	{
		aLayout.add_item(*this);
	}

	widget::~widget()
	{
		unlink();
		if (app::instance().keyboard().is_keyboard_grabbed_by(*this))
			app::instance().keyboard().ungrab_keyboard(*this);
		remove_widgets();
		{
			auto layout = iLayout;
			iLayout.reset();
		}
		if (has_parent())
			parent().remove_widget(*this);
	}

	neolib::i_destroyable& widget::as_destroyable()
	{
		return *this;
	}

	const i_device_metrics& widget::device_metrics() const
	{
		return iDeviceMetricsForwarder;
	}

	units widget::units() const
	{
		return iUnitsContext.units();
	}

	units widget::set_units(neogfx::units aUnits) const
	{
		return iUnitsContext.set_units(aUnits);
	}

	bool widget::is_singular() const
	{
		return iSingular;
	}

	void widget::set_singular(bool aSingular)
	{
		if (iSingular != aSingular)
		{
			iSingular = aSingular;
			if (iSingular)
				iParent = nullptr;
		}
	}

	bool widget::is_root() const
	{
		return false;
	}

	bool widget::has_parent(bool aSameSurface) const
	{
		return iParent != nullptr && (!aSameSurface || same_surface(*iParent));
	}

	const i_widget& widget::parent() const
	{
		if (!has_parent(false))
			throw no_parent();
		return *iParent;
	}

	i_widget& widget::parent()
	{
		if (!has_parent(false))
			throw no_parent();
		return *iParent;
	}

	void widget::set_parent(i_widget& aParent)
	{
		if (has_parent() && &parent() == &aParent)
			return;
		if (!is_root())
		{
			bool onSurface = has_surface();
			if (onSurface && same_surface(aParent))
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

	const i_widget& widget::ultimate_ancestor(bool aSameSurface) const
	{
		const i_widget* w = this;
		while (w->has_parent(aSameSurface))
			w = &w->parent();
		return *w;
	}

	i_widget& widget::ultimate_ancestor(bool aSameSurface)
	{
		i_widget* w = this;
		while (w->has_parent(aSameSurface))
			w = &w->parent();
		return *w;
	}

	bool widget::is_ancestor_of(const i_widget& aWidget, bool aSameSurface) const
	{
		const i_widget* w = &aWidget;
		while (w->has_parent(aSameSurface))
		{
			w = &w->parent();
			if (w == this)
				return true;
		}
		return false;
	}

	bool widget::is_descendent_of(const i_widget& aWidget, bool aSameSurface) const
	{
		return aWidget.is_ancestor_of(*this, aSameSurface);
	}
		
	bool widget::is_sibling_of(const i_widget& aWidget) const
	{
		return has_parent() && aWidget.has_parent() && &parent() == &aWidget.parent();
	}

	void widget::add_widget(i_widget& aWidget)
	{
		add_widget(std::shared_ptr<i_widget>{ std::shared_ptr<i_widget>{}, &aWidget });
	}

	void widget::add_widget(std::shared_ptr<i_widget> aWidget)
	{
		if (aWidget->has_parent() && &aWidget->parent() == this)
			return;
		i_widget* oldParent = aWidget->has_parent() ? &aWidget->parent() : nullptr;
		if (find_child(*aWidget, false) == iChildren.end())
			iChildren.push_back(aWidget);
		if (oldParent != nullptr)
			oldParent->remove_widget(*aWidget);
		aWidget->set_parent(*this);
		aWidget->set_singular(false);
		if (has_surface())
			surface().widget_added(*aWidget);
	}

	std::shared_ptr<i_widget> widget::remove_widget(i_widget& aWidget, bool aSingular)
	{
		auto existing = find_child(aWidget, false);
		if (existing == iChildren.end())
			return std::shared_ptr<i_widget>{};
		auto keep = *existing;
		if (aSingular)
			keep->set_singular(true);
		iChildren.erase(existing);
		if (has_layout())
			layout().remove_item(aWidget);
		if (has_surface())
			surface().widget_removed(aWidget);
		return keep;
	}

	void widget::remove_widgets()
	{
		while (!iChildren.empty())
			remove_widget(*iChildren.back(), true);
	}

	bool widget::has_children() const
	{
		return !iChildren.empty();
	}

	const widget::widget_list& widget::children() const
	{
		return iChildren;
	}

	widget::widget_list::const_iterator widget::last_child() const
	{
		if (!has_children())
		{
			if (has_parent())
				return parent().find_child(*this);
			else
				throw no_children();
		}
		else
			return iChildren.back()->last_child();
	}

	widget::widget_list::iterator widget::last_child()
	{
		if (!has_children())
		{
			if (has_parent())
				return parent().find_child(*this);
			else
				throw no_children();
		}
		else
			return iChildren.back()->last_child();
	}

	widget::widget_list::const_iterator widget::find_child(const i_widget& aChild, bool aThrowIfNotFound) const
	{
		for (auto i = iChildren.begin(); i != iChildren.end(); ++i)
			if (&**i == &aChild)
				return i;
		if (aThrowIfNotFound)
			throw not_child();
		else
			return iChildren.end();
	}

	widget::widget_list::iterator widget::find_child(const i_widget& aChild, bool aThrowIfNotFound)
	{
		for (auto i = iChildren.begin(); i != iChildren.end(); ++i)
			if (&**i == &aChild)
				return i;
		if (aThrowIfNotFound)
			throw not_child();
		else
			return iChildren.end();
	}

	const i_widget& widget::before() const
	{
		if (iLinkBefore != nullptr)
			return *iLinkBefore;
		if (has_parent())
		{
			auto me = parent().find_child(*this);
			if (me != parent().children().begin())
				return **(*(me - 1))->last_child();
			else
				return parent();
		}
		else if (has_children())
			return **last_child();
		else
			return *this;
	}

	i_widget& widget::before()
	{
		return const_cast<i_widget&>(const_cast<const widget*>(this)->before());
	}

	const i_widget& widget::after() const
	{
		if (iLinkAfter != nullptr)
			return *iLinkAfter;
		if (has_children())
			return *iChildren.front();
		if (has_parent())
		{
			auto me = parent().find_child(*this);
			if (me + 1 != parent().children().end())
				return *(*(me + 1));
			else if (parent().has_parent())
			{
				auto myParent = parent().parent().find_child(parent());
				while ((*myParent)->has_parent() && (*myParent)->parent().has_parent() &&
					myParent + 1 == (*myParent)->parent().children().end())
					myParent = (*myParent)->parent().parent().find_child((*myParent)->parent());
				if ((*myParent)->has_parent() && myParent + 1 != (*myParent)->parent().children().end())
					return **(myParent + 1);
				else if ((*(myParent))->has_parent())
					return (*(myParent))->parent();
				else
					return **myParent;
			}
			else
				return parent();
		}
		else
			return *this;
	}

	i_widget& widget::after()
	{
		return const_cast<i_widget&>(const_cast<const widget*>(this)->after());
	}

	void widget::link_before(i_widget* aPreviousWidget)
	{
		iLinkBefore = aPreviousWidget;
	}

	void widget::link_after(i_widget* aNextWidget)
	{
		iLinkAfter = aNextWidget;
	}

	void widget::unlink()
	{
		if (iLinkBefore != nullptr)
			iLinkBefore->link_after(iLinkAfter);
		if (iLinkAfter != nullptr)
			iLinkAfter->link_before(iLinkBefore);
		iLinkBefore = nullptr;
		iLinkAfter = nullptr;
	}

	bool widget::has_surface() const
	{
		return find_surface() != nullptr;
	}

	const i_surface& widget::surface() const
	{
		auto maybeSurface = find_surface();
		if (maybeSurface != nullptr)
			return *maybeSurface;
		else
			throw no_surface();
	}

	i_surface& widget::surface()
	{
		return const_cast<i_surface&>(const_cast<const widget*>(this)->surface());
	}

	bool widget::is_surface() const
	{
		return false;
	}

	bool widget::has_layout() const
	{
		return iLayout != nullptr;
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
		if (iLayout != nullptr)
		{
			iLayout->set_owner(this);
			for (auto& c : iChildren)
				iLayout->add_item(c);
		}
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

	bool widget::has_parent_layout() const
	{
		if (!has_parent())
			return false;
		const i_widget* w = &parent();
		while (!w->has_layout() && w->has_parent())
			w = &w->parent();
		return w->has_layout();
	}
	
	const i_layout& widget::parent_layout() const
	{
		if (!has_parent())
			throw no_parent_layout();
		const i_widget* w = &parent();
		while (!w->has_layout() && w->has_parent())
			w = &w->parent();
		if (w->has_layout())
			return w->layout();
		throw no_parent_layout();
	}

	i_layout& widget::parent_layout()
	{
		return const_cast<i_layout&>(const_cast<const widget*>(this)->parent_layout());
	}

	void widget::layout_items(bool aDefer)
	{
		if (layout_items_in_progress())
			return;
		if (!aDefer)
		{
			iLayoutTimer.reset();
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
				iLayoutTimer = std::make_unique<layout_timer>(surface(), app::instance(), [this](neolib::callback_timer&)
				{
					auto t = std::move(iLayoutTimer);
					if (!surface().destroyed())
						layout_items();
					update();
				});
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

	point widget::origin(bool) const
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
		position_changed.trigger();
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
		size_changed.trigger();
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

	const i_widget& widget::widget_at(const point& aPosition) const
	{
		if (client_rect().contains(aPosition))
		{
			for (const auto& c : children())
				if (c->visible() && to_client_coordinates(c->window_rect()).contains(aPosition))
					return c->widget_at(aPosition - c->position());
		}
		return *this;
	}

	i_widget& widget::widget_at(const point& aPosition)
	{
		return const_cast<i_widget&>(const_cast<const widget*>(this)->widget_at(aPosition));
	}

	widget_part widget::hit_test(const point& aPosition) const
	{
		if (client_rect().contains(aPosition))
			return widget_part::Client;
		else
			return widget_part::NonClient;
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
				layout().minimum_size(aAvailableSpace != boost::none ? *aAvailableSpace - margins().size() : aAvailableSpace) + margins().size() :
				margins().size();
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
					layout().maximum_size(aAvailableSpace != boost::none ? *aAvailableSpace - margins().size() : aAvailableSpace) + margins().size() :
					size{ std::numeric_limits<size::dimension_type>::max(), std::numeric_limits<size::dimension_type>::max() };
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
		if ((!is_root() && !has_parent()) || !has_surface() || surface().destroyed() || effectively_hidden() || layout_items_in_progress())
			return;
		update(aIncludeNonClient ? to_client_coordinates(window_rect()) : client_rect());
	}

	void widget::update(const rect& aUpdateRect)
	{
		if ((!is_root() && !has_parent()) || !has_surface() || surface().destroyed() || effectively_hidden() || layout_items_in_progress())
			return;
		if (aUpdateRect.empty())
			return;
		surface().invalidate_surface(to_window_coordinates(aUpdateRect));
	}

	bool widget::requires_update() const
	{
		return surface().has_invalidated_area() && !surface().invalidated_area().intersection(window_rect()).empty();
	}

	rect widget::update_rect() const
	{
		if (!requires_update())
			throw no_update_rect();
		return to_client_coordinates(surface().invalidated_area().intersection(window_rect()));
	}

	rect widget::default_clip_rect(bool aIncludeNonClient) const
	{
		rect clipRect = to_client_coordinates(window_rect());
		if (!aIncludeNonClient)
			clipRect = clipRect.intersection(client_rect());
		if (has_parent() && !is_root())
			clipRect = clipRect.intersection(to_client_coordinates(parent().to_window_coordinates(parent().default_clip_rect())));
		return clipRect;
	}

	bool widget::ready_to_render() const
	{
		return iLayoutTimer == nullptr;
	}

	void widget::render(graphics_context& aGraphicsContext) const
	{
		if (effectively_hidden())
			return;
		if (!requires_update())
			return;
		
		const rect updateRect = update_rect();

		const rect nonClientClipRect = default_clip_rect(true).intersection(updateRect);

		aGraphicsContext.set_extents(extents());
		aGraphicsContext.set_origin(origin(true));
		aGraphicsContext.scissor_on(nonClientClipRect);
		paint_non_client(aGraphicsContext);
		aGraphicsContext.scissor_off();

		const rect clipRect = default_clip_rect().intersection(updateRect);

		aGraphicsContext.set_extents(client_rect().extents());
		aGraphicsContext.set_origin(origin());
		aGraphicsContext.scissor_on(clipRect);
		aGraphicsContext.clear_depth_buffer();
		scoped_coordinate_system scs(aGraphicsContext, origin(), extents(), logical_coordinate_system());
		painting.trigger(aGraphicsContext);
		paint(aGraphicsContext);
		aGraphicsContext.scissor_off();

		for (auto i = iChildren.rbegin(); i != iChildren.rend(); ++i)
		{
			const auto& c = *i;
			rect intersection = clipRect.intersection(to_client_coordinates(c->window_rect()));
			if (!intersection.empty())
				c->render(aGraphicsContext);
		}

		aGraphicsContext.set_extents(extents());
		aGraphicsContext.set_origin(origin(true));
		aGraphicsContext.scissor_on(nonClientClipRect);
		paint_non_client_after(aGraphicsContext);
		aGraphicsContext.scissor_off();
	}

	bool widget::transparent_background() const
	{
		return !is_root();
	}

	void widget::paint_non_client(graphics_context& aGraphicsContext) const
	{
		if (has_background_colour() || !transparent_background())
			aGraphicsContext.fill_rect(update_rect(), background_colour());
	}

	void widget::paint_non_client_after(graphics_context&) const
	{
	}

	void widget::paint(graphics_context&) const
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
			return app::instance().current_style().palette().foreground_colour();
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
			return app::instance().current_style().palette().background_colour();
	}

	void widget::set_background_colour(const optional_colour& aBackgroundColour)
	{
		iBackgroundColour = aBackgroundColour;
		update();
	}

	colour widget::container_background_colour() const
	{
		const i_widget* w = this;
		while (w->transparent_background() && w->has_parent() && same_surface(w->parent()))
			w = &w->parent();
		if (!w->transparent_background() && w->has_background_colour())
			return w->background_colour();
		else
			return app::instance().current_style().palette().colour();
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
		return iVisible && (iMaximumSize == boost::none || (iMaximumSize->cx != 0.0 && iMaximumSize->cy != 0.0));
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
			if (has_parent_layout())
				parent_layout().invalidate();
			if (effectively_hidden())
			{
				if (surface().has_focused_widget() &&
					(surface().focused_widget().is_descendent_of(*this) || &surface().focused_widget() == this))
				{
					surface().release_focused_widget(surface().focused_widget());
				}
			}
			else
				update();
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

	bool widget::can_capture() const
	{
		return true;
	}

	bool widget::capturing() const
	{
		return surface().has_capturing_widget() && &surface().capturing_widget() == this;
	}

	void widget::set_capture()
	{
		if (can_capture())
			surface().set_capture(*this);
		else
			throw widget_cannot_capture();
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
		return has_surface() && !surface().destroyed() && surface().surface_type() == surface_type::Window &&
			static_cast<const i_native_window&>(surface().native_surface()).is_active() &&
			surface().has_focused_widget() && &surface().focused_widget() == this;
	}

	void widget::set_focus(focus_reason aFocusReason)
	{
		surface().set_focused_widget(*this, aFocusReason);
	}

	void widget::release_focus()
	{
		surface().release_focused_widget(*this);
	}

	void widget::focus_gained(focus_reason)
	{
		update();
		focus_event.trigger(focus_event::FocusGained);
	}

	void widget::focus_lost(focus_reason)
	{
		update();
		focus_event.trigger(focus_event::FocusLost);
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

	void widget::mouse_button_pressed(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		if (aButton == mouse_button::Middle && has_parent())
			parent().mouse_button_pressed(aButton, aPosition + position(), aKeyModifiers);
		else if (capture_ok(hit_test(aPosition)) && can_capture())
			set_capture();
	}

	void widget::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		if (aButton == mouse_button::Middle && has_parent())
			parent().mouse_button_double_clicked(aButton, aPosition + position(), aKeyModifiers);
		else if (capture_ok(hit_test(aPosition)) && can_capture())
			set_capture();
	}

	void widget::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		if (aButton == mouse_button::Middle && has_parent())
			parent().mouse_button_released(aButton, aPosition + position());
		else if (capturing())
			release_capture();
	}

	void widget::mouse_moved(const point&)
	{
	}

	void widget::mouse_entered()
	{
	}

	void widget::mouse_left()
	{
	}

	neogfx::mouse_cursor widget::mouse_cursor() const
	{
		if (has_parent())
			return parent().mouse_cursor();
		return mouse_system_cursor::Arrow;
	}

	bool widget::key_pressed(scan_code_e, key_code_e, key_modifiers_e)
	{
		return false;
	}

	bool widget::key_released(scan_code_e, key_code_e, key_modifiers_e)
	{
		return false;
	}

	bool widget::text_input(const std::string&)
	{
		return false;
	}

	bool widget::sys_text_input(const std::string&)
	{
		return false;
	}

	const i_widget& widget::widget_for_mouse_event(const point& aPosition, bool aForHitTest) const
	{
		if (client_rect().contains(aPosition))
		{
			const i_widget* w = &widget_at(aPosition);
			while (w != this && (w->hidden() || (w->disabled() && !aForHitTest) || w->ignore_mouse_events()))
			{
				w = &w->parent();
			}
			return *w;
		}
		else
			return *this;
	}

	i_widget& widget::widget_for_mouse_event(const point& aPosition, bool aForHitTest)
	{
		return const_cast<i_widget&>(const_cast<const widget*>(this)->widget_for_mouse_event(aPosition, aForHitTest));
	}

	graphics_context widget::create_graphics_context() const
	{
		return graphics_context(*this);
	}

	i_surface* widget::find_surface() const
	{
		if (iSurface != nullptr)
			return iSurface;
		const i_widget* w = this;
		while (!w->is_root() && !w->is_surface() && w->has_parent(false))
			w = &w->parent();
		if (w->is_surface())
			return (iSurface = const_cast<i_surface*>(&w->surface()));
		return nullptr;
	}
}

