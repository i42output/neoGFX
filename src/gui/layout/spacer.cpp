// spacer.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2015, 2020 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/gui/widget/i_widget.hpp>
#include <neogfx/gui/layout/i_layout.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gui/layout/spacer.hpp>

namespace neogfx
{
    spacer::spacer(neogfx::expansion_policy aExpansionPolicy) :
        iParentLayout{ nullptr }, 
        iExpansionPolicy{ aExpansionPolicy }
    {
        set_alive();
    }

    spacer::spacer(i_layout& aParentLayout, neogfx::expansion_policy aExpansionPolicy) :
        iParentLayout{ nullptr }, 
        iExpansionPolicy{ aExpansionPolicy }
    {
        aParentLayout.add(*this);
        set_alive();
    }

    spacer::~spacer()
    {
        if (has_parent_layout())
            parent_layout().remove(*this);
    }

    bool spacer::is_layout() const
    {
        return false;
    }

    const i_layout& spacer::as_layout() const
    {
        throw not_a_layout();
    }

    i_layout& spacer::as_layout()
    {
        throw not_a_layout();
    }

    bool spacer::is_spacer() const
    {
        return true;
    }

    const i_spacer& spacer::as_spacer() const
    {
        return *this;
    }

    i_spacer& spacer::as_spacer()
    {
        return *this;
    }

    bool spacer::is_widget() const
    {
        return false;
    }

    const i_widget& spacer::as_widget() const
    {
        throw not_a_widget();
    }

    i_widget& spacer::as_widget()
    {
        throw not_a_widget();
    }

    bool spacer::has_parent_layout() const
    {
        return iParentLayout != nullptr;
    }

    const i_layout& spacer::parent_layout() const
    {
        if (has_parent_layout())
            return *iParentLayout;
        throw no_parent_layout();
    }

    i_layout& spacer::parent_layout()
    {
        return const_cast<i_layout&>(to_const(*this).parent_layout());
    }

    void spacer::set_parent_layout(i_layout* aParentLayout)
    {
        iParentLayout = aParentLayout;
    }

    bool spacer::has_layout_owner() const
    {
        if (has_parent_layout())
            return parent_layout().has_layout_owner();
        else
            return false;
    }

    const i_widget& spacer::layout_owner() const
    {
        if (has_layout_owner())
            return parent_layout().layout_owner();
        throw no_layout_owner();
    }

    i_widget& spacer::layout_owner()
    {
        return const_cast<i_widget&>(to_const(*this).layout_owner());
    }

    void spacer::set_layout_owner(i_widget*)
    {
        // do nothing
    }

    expansion_policy spacer::expansion_policy() const
    {
        return iExpansionPolicy;
    }

    void spacer::set_expansion_policy(neogfx::expansion_policy aExpansionPolicy)
    {
        if (iExpansionPolicy != aExpansionPolicy)
        {
            iExpansionPolicy = aExpansionPolicy;
            update_layout();
        }
    }

    bool spacer::high_dpi() const
    {
        return has_layout_owner() && layout_owner().has_surface() ? 
            layout_owner().surface().ppi() >= 150.0 : 
            service<i_surface_manager>().display().metrics().ppi() >= 150.0;
    }

    dimension spacer::dpi_scale_factor() const
    {
        return default_dpi_scale_factor(
            has_layout_owner() && layout_owner().has_surface() ?
                layout_owner().surface().ppi() : 
                service<i_surface_manager>().display().metrics().ppi());
    }

    size_policy spacer::size_policy() const
    {
        if (has_size_policy())
            return base_type::size_policy();
        else if (has_fixed_size())
            return size_constraint::Fixed;
        neogfx::size_policy result{ size_constraint::Minimum };
        if ((iExpansionPolicy & neogfx::expansion_policy::ExpandHorizontally) == neogfx::expansion_policy::ExpandHorizontally)
            result.set_horizontal_size_policy(size_constraint::Expanding);
        if ((iExpansionPolicy & neogfx::expansion_policy::ExpandVertically) == neogfx::expansion_policy::ExpandVertically)
            result.set_vertical_size_policy(size_constraint::Expanding);
        return result;
    }

    bool spacer::has_padding() const
    {
        return false;
    }

    neogfx::padding spacer::padding() const
    {
        return neogfx::padding{};
    }

    void spacer::set_padding(optional_padding const&, bool)
    {
        /* do nothing */
    }

    bool spacer::device_metrics_available() const
    {
        return has_parent_layout() && parent_layout().device_metrics_available();
    }

    const i_device_metrics& spacer::device_metrics() const
    {
        if (device_metrics_available())
            return parent_layout().device_metrics();
        throw no_device_metrics();
    }

    void spacer::layout_as(const point&, const size& aSize)
    {
        set_extents(aSize);
    }

    bool spacer::visible() const
    {
        return true;
    }

    horizontal_spacer::horizontal_spacer() :
        spacer(neogfx::expansion_policy::ExpandHorizontally)
    {
    }

    horizontal_spacer::horizontal_spacer(i_layout& aParent) :
        spacer(aParent, neogfx::expansion_policy::ExpandHorizontally)
    {
    }

    vertical_spacer::vertical_spacer() :
        spacer(neogfx::expansion_policy::ExpandVertically)
    {
    }

    vertical_spacer::vertical_spacer(i_layout& aParent) :
        spacer(aParent, neogfx::expansion_policy::ExpandVertically)
    {
    }
}