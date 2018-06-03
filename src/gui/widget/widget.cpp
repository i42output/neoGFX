// widget.cpp
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
#include <neolib/raii.hpp>
#include <neogfx/app/app.hpp>
#include <neogfx/gui/widget/widget.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/hid/i_surface_window.hpp>

namespace neogfx
{
	class widget::layout_timer : public pause_rendering, neolib::callback_timer
	{
	public:
		layout_timer(i_window& aWindow, neolib::async_task& aIoTask, std::function<void(callback_timer&)> aCallback) :
			pause_rendering{ aWindow }, neolib::callback_timer{ aIoTask, aCallback, 0 }
		{
		}
		~layout_timer()
		{
		}
	};

	i_widget* widget::debug;

	widget::widget() :
		iSingular{ false },
		iParent{ nullptr },
		iSurface{ nullptr },
		iRoot { nullptr },
		iAddingChild{ false },
		iLinkBefore{ nullptr },
		iLinkAfter{ nullptr },
		iParentLayout{ nullptr },
		iLayoutId{ 0u },
		iLayoutInProgress{ 0 },
		iUnitsContext{ *this },
		iMinimumSize{},
		iMaximumSize{},
		iVisible{ true },
		iEnabled{ true },
		iFocusPolicy{ focus_policy::NoFocus },
		iOpacity{ 1.0 },
		iForegroundColour{},
		iBackgroundColour{},
		iIgnoreMouseEvents{ false },
		iIgnoreNonClientMouseEvents{ true }
	{
	}
	
	widget::widget(i_widget& aParent) :
		iSingular{ false },
		iParent{ nullptr },
		iSurface{ nullptr },
		iRoot{ nullptr },
		iAddingChild{ false },
		iLinkBefore{ nullptr },
		iLinkAfter{ nullptr },
		iParentLayout{ nullptr },
		iLayoutId{ 0u },
		iLayoutInProgress{ 0 },
		iUnitsContext{ *this },
		iMinimumSize{},
		iMaximumSize{},
		iVisible{ true },
		iEnabled{ true },
		iFocusPolicy{ focus_policy::NoFocus },
		iOpacity{ 1.0 },
		iForegroundColour{},
		iBackgroundColour{},
		iIgnoreMouseEvents{ false },
		iIgnoreNonClientMouseEvents{ true }
	{
		aParent.add(*this);
	}

	widget::widget(i_layout& aLayout) :
		iSingular{ false },
		iParent{ nullptr },
		iSurface{ nullptr },
		iRoot{ nullptr },
		iAddingChild{ false },
		iLinkBefore{ nullptr },
		iLinkAfter{ nullptr },
		iParentLayout{ nullptr },
		iLayoutId{ 0u },
		iLayoutInProgress{ 0 },
		iUnitsContext{ *this },
		iMinimumSize{},
		iMaximumSize{},
		iVisible{ true },
		iEnabled{ true },
		iFocusPolicy{ focus_policy::NoFocus },
		iOpacity{ 1.0 },
		iForegroundColour{},
		iBackgroundColour{},
		iIgnoreMouseEvents{ false },
		iIgnoreNonClientMouseEvents{ true }
	{
		aLayout.add(*this);
	}

	widget::~widget()
	{
		unlink();
		if (app::instance().keyboard().is_keyboard_grabbed_by(*this))
			app::instance().keyboard().ungrab_keyboard(*this);
		remove_all();
		{
			auto layout = iLayout;
			iLayout.reset();
		}
		if (has_parent())
			parent().remove(*this);
	}

	neolib::i_lifetime& widget::as_lifetime()
	{
		return *this;
	}

	bool widget::device_metrics_available() const
	{
		return has_surface();
	}

	const i_device_metrics& widget::device_metrics() const
	{
		if (device_metrics_available())
			return surface();
		throw no_device_metrics();
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
			{
				iParent = nullptr;
				iSurface = nullptr;
			}
		}
	}

	bool widget::is_root() const
	{
		return false;
	}

	bool widget::has_root() const
	{
		return find_root() != nullptr;
	}

	const i_window& widget::root() const
	{
		auto existingRoot = find_root();
		if (existingRoot != nullptr)
			return *existingRoot;
		throw no_root();
	}

	i_window& widget::root()
	{
		return const_cast<i_window&>(const_cast<const widget*>(this)->root());
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
		if (is_root())
			iParent = &aParent;
		else if (aParent.adding_child())
			iParent = &aParent;
		else
			aParent.add(*this);
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
		return has_parent(false) && aWidget.has_parent(false) && &parent() == &aWidget.parent();
	}

	bool widget::adding_child() const
	{
		return iAddingChild;
	}

	i_widget& widget::add(i_widget& aChild)
	{
		return add(std::shared_ptr<i_widget>{ std::shared_ptr<i_widget>{}, &aChild });
	}

	i_widget& widget::add(std::shared_ptr<i_widget> aChild)
	{
		neolib::scoped_flag sf{ iAddingChild };
		if (aChild->has_parent() && &aChild->parent() == this)
			return *aChild;
		i_widget* oldParent = aChild->has_parent() ? &aChild->parent() : nullptr;
		if (oldParent != nullptr)
			aChild = oldParent->remove(*aChild, true);
		iChildren.push_back(aChild);
		aChild->set_parent(*this);
		aChild->set_singular(false);
		if (has_root())
			root().widget_added(*aChild);
		return *aChild;
	}

	std::shared_ptr<i_widget> widget::remove(i_widget& aChild, bool aSingular)
	{
		auto existing = find(aChild, false);
		if (existing == iChildren.end())
			return std::shared_ptr<i_widget>{};
		auto keep = *existing;
		if (aSingular)
			keep->set_singular(true);
		iChildren.erase(existing);
		if (has_layout())
			layout().remove(aChild);
		if (has_root())
			root().widget_removed(aChild);
		return keep;
	}

	void widget::remove_all()
	{
		while (!iChildren.empty())
			remove(*iChildren.back(), true);
	}

	bool widget::has_children() const
	{
		return !iChildren.empty();
	}

	const widget::widget_list& widget::children() const
	{
		return iChildren;
	}

	widget::widget_list::const_iterator widget::last() const
	{
		if (!has_children())
		{
			if (has_parent())
				return parent().find(*this);
			else
				throw no_children();
		}
		else
			return iChildren.back()->last();
	}

	widget::widget_list::iterator widget::last()
	{
		if (!has_children())
		{
			if (has_parent())
				return parent().find(*this);
			else
				throw no_children();
		}
		else
			return iChildren.back()->last();
	}

	widget::widget_list::const_iterator widget::find(const i_widget& aChild, bool aThrowIfNotFound) const
	{
		for (auto i = iChildren.begin(); i != iChildren.end(); ++i)
			if (&**i == &aChild)
				return i;
		if (aThrowIfNotFound)
			throw not_child();
		else
			return iChildren.end();
	}

	widget::widget_list::iterator widget::find(const i_widget& aChild, bool aThrowIfNotFound)
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
			auto me = parent().find(*this);
			if (me != parent().children().begin())
				return **(*(me - 1))->last();
			else
				return parent();
		}
		else if (has_children())
			return **last();
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
			auto me = parent().find(*this);
			if (me + 1 != parent().children().end())
				return *(*(me + 1));
			else if (parent().has_parent())
			{
				auto myParent = parent().parent().find(parent());
				while ((*myParent)->has_parent() && (*myParent)->parent().has_parent() &&
					myParent + 1 == (*myParent)->parent().children().end())
					myParent = (*myParent)->parent().parent().find((*myParent)->parent());
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
		auto existingSurface = find_surface();
		if (existingSurface != nullptr)
			return *existingSurface;
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
		set_layout(std::shared_ptr<i_layout>{ std::shared_ptr<i_layout>{}, &aLayout });
	}

	void widget::set_layout(std::shared_ptr<i_layout> aLayout)
	{
		if (iLayout == aLayout)
			throw layout_already_set();
		iLayout = aLayout;
		if (iLayout != nullptr)
		{
			iLayout->set_layout_owner(this);
			for (auto& c : iChildren)
				iLayout->add(c);
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

	bool widget::is_layout() const
	{
		return false;
	}

	const i_layout& widget::as_layout() const
	{
		throw not_a_layout();
	}

	i_layout& widget::as_layout()
	{
		throw not_a_layout();
	}

	bool widget::is_widget() const
	{
		return true;
	}

	const i_widget& widget::as_widget() const
	{
		return *this;
	}

	i_widget& widget::as_widget()
	{
		return *this;
	}

	bool widget::has_parent_layout() const
	{
		return iParentLayout != nullptr;
	}
	
	const i_layout& widget::parent_layout() const
	{
		if (has_parent_layout())
			return *iParentLayout;
		throw no_parent_layout();
	}

	i_layout& widget::parent_layout()
	{
		return const_cast<i_layout&>(const_cast<const widget*>(this)->parent_layout());
	}

	void widget::set_parent_layout(i_layout* aParentLayout)
	{
		iParentLayout = aParentLayout;
	}

	bool widget::has_layout_owner() const
	{
		return has_parent_layout() && parent_layout().has_layout_owner();
	}

	const i_widget& widget::layout_owner() const
	{
		if (has_layout_owner())
			return parent_layout().layout_owner();
		throw no_layout_owner();
	}

	i_widget& widget::layout_owner()
	{
		return const_cast<i_widget&>(const_cast<const widget*>(this)->layout_owner());
	}

	void widget::set_layout_owner(i_widget* aOwner)
	{
		if (aOwner != nullptr)
			set_parent(*aOwner);
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
		else if (can_defer_layout() && has_root())
		{
			if (!iLayoutTimer)
			{
				iLayoutTimer = std::make_unique<layout_timer>(root(), app::instance(), [this](neolib::callback_timer&)
				{
					if (root().has_native_window())
					{
						auto t = std::move(iLayoutTimer);
						layout_items();
						update();
					}
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
		{
			layout_completed.trigger();
			update();
		}
	}

	bool widget::high_dpi() const
	{
		return has_root() && root().has_surface() ? 
			root().surface().ppi() >= 150.0 : 
			app::instance().surface_manager().display().metrics().ppi() >= 150.0;
	}

	dimension widget::dpi_scale_factor() const
	{
		return has_root() && root().has_surface() ?
			default_dpi_scale_factor(root().surface().ppi()) :
			app::instance().default_dpi_scale_factor();
	}

	bool widget::has_logical_coordinate_system() const
	{
		return iLogicalCoordinateSystem != boost::none;
	}

	logical_coordinate_system widget::logical_coordinate_system() const
	{
		if (has_logical_coordinate_system())
			return *iLogicalCoordinateSystem;
		return neogfx::logical_coordinate_system::AutomaticGui;
	}

	void widget::set_logical_coordinate_system(const optional_logical_coordinate_system& aLogicalCoordinateSystem)
	{
		iLogicalCoordinateSystem = aLogicalCoordinateSystem;
	}

	point widget::position() const
	{
		return units_converter(*this).from_device_units(iPosition);
	}

	void widget::set_position(const point& aPosition)
	{
		move(aPosition);
	}

	point widget::origin() const
	{
		if (!is_root() && has_parent())
			return position() + parent().origin();
		else
			return point{};
	}

	void widget::move(const point& aPosition)
	{
		if (iPosition != units_converter(*this).to_device_units(aPosition))
		{
			update(true);
			iPosition = units_converter(*this).to_device_units(aPosition);
			update(true);
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
		return rect{origin(), extents()};
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
		else if (to_client_coordinates(window_rect()).contains(aPosition))
			return widget_part::NonClient;
		else
			return widget_part::Nowhere;
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
		if (has_minimum_size())
			return units_converter(*this).from_device_units(*iMinimumSize);
		else if (has_layout())
		{
			auto result = layout().minimum_size(aAvailableSpace != boost::none ? *aAvailableSpace - margins().size() : aAvailableSpace);
			if (result.cx != 0.0)
				result.cx += margins().size().cx;
			if (result.cy != 0.0)
				result.cy += margins().size().cy;
			return result;
		}
		else
			return margins().size();
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
		if (has_maximum_size())
			return units_converter(*this).from_device_units(*iMaximumSize);
		else if (size_policy() == neogfx::size_policy::Minimum || size_policy() == neogfx::size_policy::Fixed)
			return minimum_size(aAvailableSpace);
		else if (has_layout())
		{
			auto result = layout().maximum_size(aAvailableSpace != boost::none ? *aAvailableSpace - margins().size() : aAvailableSpace);
			if (result.cx != 0.0)
				result.cx += margins().size().cx;
			if (result.cy != 0.0)
				result.cy += margins().size().cy;
			return result;
		}
		else
			return size::max_size();
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
		const auto& adjustedMargins =
			(has_margins() ?
				*iMargins :
				app::instance().current_style().margins() * dpi_scale(1.0));
		return units_converter(*this).from_device_units(adjustedMargins);
	}

	void widget::set_margins(const optional_margins& aMargins, bool aUpdateLayout)
	{
		optional_margins newMargins = (aMargins != boost::none ? units_converter(*this).to_device_units(*aMargins) : optional_margins{});
		if (iMargins != newMargins)
		{
			iMargins = newMargins;
			if (aUpdateLayout && has_managing_layout())
				managing_layout().layout_items(true);
		}
	}

	void widget::layout_as(const point& aPosition, const size& aSize)
	{
		move(aPosition);
		if (extents() != aSize)
			resize(aSize);
		else if (has_layout() && layout().invalidated())
			layout_items();
	}

	uint32_t widget::layout_id() const
	{
		return iLayoutId;
	}

	void widget::next_layout_id()
	{
		if (++iLayoutId == static_cast<uint32_t>(-1))
			iLayoutId = 0;
		if (has_layout())
			layout().next_layout_id();
	}

	void widget::update(bool aIncludeNonClient)
	{
		if ((!is_root() && !has_parent()) || !has_root() || !root().has_native_surface() || effectively_hidden() || layout_items_in_progress())
			return;
		update(aIncludeNonClient ? to_client_coordinates(window_rect()) : client_rect());
	}

	void widget::update(const rect& aUpdateRect)
	{
		if ((!is_root() && !has_parent()) || !has_root() || !root().has_native_surface() || effectively_hidden() || layout_items_in_progress())
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
		auto& cachedRect = (aIncludeNonClient ? iDefaultClipRect.first : iDefaultClipRect.second);
		if (cachedRect != boost::none)
			return *cachedRect;
		rect clipRect = to_client_coordinates(window_rect());
		if (!aIncludeNonClient)
			clipRect = clipRect.intersection(client_rect());
		if (has_parent() && !is_root())
			clipRect = clipRect.intersection(to_client_coordinates(parent().to_window_coordinates(parent().default_clip_rect())));
		return *(cachedRect = clipRect);
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

		iDefaultClipRect = std::make_pair(boost::none, boost::none);

		const rect updateRect = update_rect();

		const rect nonClientClipRect = default_clip_rect(true).intersection(updateRect);

		aGraphicsContext.set_extents(extents());
		aGraphicsContext.set_origin(origin());

		scoped_opacity sc{ aGraphicsContext, opacity() };

		{
			scoped_scissor scissor(aGraphicsContext, nonClientClipRect);
			paint_non_client(aGraphicsContext);
		}

		{
			const rect clipRect = default_clip_rect().intersection(updateRect);

			aGraphicsContext.set_extents(client_rect().extents());
			aGraphicsContext.set_origin(origin());

			scoped_scissor scissor(aGraphicsContext, clipRect);
			scoped_coordinate_system scs(aGraphicsContext, origin(), extents(), logical_coordinate_system());

			painting.trigger(aGraphicsContext);
			paint(aGraphicsContext);
			painted.trigger(aGraphicsContext);

			for (auto i = iChildren.rbegin(); i != iChildren.rend(); ++i)
			{
				const auto& c = *i;
				rect intersection = clipRect.intersection(to_client_coordinates(c->window_rect()));
				if (!intersection.empty())
					c->render(aGraphicsContext);
			}

			children_painted.trigger(aGraphicsContext);
		}

		aGraphicsContext.set_extents(extents());
		aGraphicsContext.set_origin(origin());
		{
			scoped_scissor scissor(aGraphicsContext, nonClientClipRect);
			paint_non_client_after(aGraphicsContext);
		}
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
		// do nothing
	}

	void widget::paint(graphics_context&) const
	{
		// do nothing
	}

	double widget::opacity() const
	{
		return iOpacity;
	}

	void widget::set_opacity(double aOpacity)
	{
		if (iOpacity != aOpacity)
		{
			iOpacity = aOpacity;
			update(true);
		}
	}

	double widget::transparency() const
	{
		return 1.0 - opacity();
	}

	void widget::set_transparency(double aTransparency)
	{
		set_opacity(1.0 - aTransparency);
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
		while (w->transparent_background() && w->has_parent())
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
		if (iFont != aFont)
		{
			iFont = aFont;
			if (has_managing_layout())
				managing_layout().layout_items(true);
			update();
		}
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

	bool widget::show(bool aVisible)
	{
		if (iVisible != aVisible)
		{
			bool isEntered = entered();
			iVisible = aVisible;
			if (!visible() && isEntered)
			{
				if (!is_root())
					root().as_widget().mouse_entered(root().mouse_position());
				else
					mouse_left();
			}
			visibility_changed.trigger();
			if (has_parent_layout())
				parent_layout().invalidate();
			if (effectively_hidden())
			{
				if (root().has_focused_widget() &&
					(root().focused_widget().is_descendent_of(*this) || &root().focused_widget() == this))
				{
					root().release_focused_widget(root().focused_widget());
				}
			}
			else
				update();
			return true;
		}
		return false;
	}

	bool widget::show()
	{
		return show(true);
	}

	bool widget::hide()
	{
		return show(false);
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

	bool widget::enable(bool aEnable)
	{
		if (iEnabled != aEnable)
		{
			bool isEntered = entered();
			iEnabled = aEnable;
			if (!enabled() && isEntered)
			{
				if (!is_root())
					root().as_widget().mouse_entered(root().mouse_position());
				else
					mouse_left();
			}
			update();
			return true;
		}
		return false;
	}

	bool widget::enable()
	{
		return enable(true);
	}

	bool widget::disable()
	{
		return enable(false);
	}

	bool widget::entered() const
	{
		return root().has_entered_widget() && &root().entered_widget() == this;
	}

	bool widget::can_capture() const
	{
		return true;
	}

	bool widget::capturing() const
	{
		return surface().as_surface_window().has_capturing_widget() && &surface().as_surface_window().capturing_widget() == this;
	}

	void widget::set_capture(capture_reason aReason)
	{
		if (can_capture())
		{
			switch (aReason)
			{
			case capture_reason::MouseEvent:
				if (!mouse_event_is_non_client())
					surface().as_surface_window().set_capture(*this);
				else
					surface().as_surface_window().non_client_set_capture(*this);
				break;
			default:
				surface().as_surface_window().set_capture(*this);
				break;
			}
		}
		else
			throw widget_cannot_capture();
	}

	void widget::release_capture(capture_reason aReason)
	{
		switch (aReason)
		{
		case capture_reason::MouseEvent:
			if (!mouse_event_is_non_client())
				surface().as_surface_window().release_capture(*this);
			else
				surface().as_surface_window().non_client_release_capture(*this);
			break;
		default:
			surface().as_surface_window().set_capture(*this);
			break;
		}
		
	}

	void widget::non_client_set_capture()
	{
		if (can_capture())
			surface().as_surface_window().non_client_set_capture(*this);
		else
			throw widget_cannot_capture();
	}

	void widget::non_client_release_capture()
	{
		surface().as_surface_window().non_client_release_capture(*this);
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
		return has_root() && root().is_active() && root().has_focused_widget() && &root().focused_widget() == this;
	}

	void widget::set_focus(focus_reason aFocusReason)
	{
		root().set_focused_widget(*this, aFocusReason);
	}

	void widget::release_focus()
	{
		root().release_focused_widget(*this);
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

	bool widget::ignore_non_client_mouse_events() const
	{
		return iIgnoreNonClientMouseEvents;
	}

	void widget::set_ignore_non_client_mouse_events(bool aIgnoreNonClientMouseEvents)
	{
		iIgnoreNonClientMouseEvents = aIgnoreNonClientMouseEvents;
	}

	bool widget::mouse_event_is_non_client() const
	{
		if (!has_root() || !root().has_native_surface() || !surface().as_surface_window().current_event_is_non_client())
			return false;
		else
			return true;
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
			set_capture(capture_reason::MouseEvent);
	}

	void widget::mouse_button_double_clicked(mouse_button aButton, const point& aPosition, key_modifiers_e aKeyModifiers)
	{
		if (aButton == mouse_button::Middle && has_parent())
			parent().mouse_button_double_clicked(aButton, aPosition + position(), aKeyModifiers);
		else if (capture_ok(hit_test(aPosition)) && can_capture())
			set_capture(capture_reason::MouseEvent);
	}

	void widget::mouse_button_released(mouse_button aButton, const point& aPosition)
	{
		if (aButton == mouse_button::Middle && has_parent())
			parent().mouse_button_released(aButton, aPosition + position());
		else if (capturing())
			release_capture(capture_reason::MouseEvent);
	}

	void widget::mouse_moved(const point&)
	{
		// do nothing
	}

	void widget::mouse_entered(const point&)
	{
		// do nothing
	}

	void widget::mouse_left()
	{
		// do nothing
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
			while (w != this && (w->hidden() || (w->disabled() && !aForHitTest) || (!mouse_event_is_non_client() && w->ignore_mouse_events()) || (mouse_event_is_non_client() && w->ignore_non_client_mouse_events())))
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
		return graphics_context{ *this };
	}

	const i_surface* widget::find_surface() const
	{
		if (iSurface != nullptr)
			return iSurface;
		if (is_surface())
			return (iSurface = &surface());
		if (has_parent(false))
			return (iSurface = parent().find_surface());
		return nullptr;
	}

	i_surface* widget::find_surface()
	{
		return const_cast<i_surface*>(const_cast<const widget*>(this)->find_surface());
	}

	const i_window* widget::find_root() const
	{
		if (iRoot != nullptr)
			return iRoot;
		const i_widget* w = this;
		while (!w->is_root() && w->has_parent(true))
			w = &w->parent();
		if (w->is_root())
			return (iRoot = &w->root());
		return nullptr;
	}

	i_window* widget::find_root()
	{
		return const_cast<i_window*>(const_cast<const widget*>(this)->find_root());
	}
}

