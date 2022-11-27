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

    bool spacer::has_parent_widget() const
    {
        if (has_parent_layout())
            return parent_layout().has_parent_widget();
        else
            return false;
    }

    const i_widget& spacer::parent_widget() const
    {
        if (has_parent_widget())
            return parent_layout().parent_widget();
        throw no_parent_widget();
    }

    i_widget& spacer::parent_widget()
    {
        return const_cast<i_widget&>(to_const(*this).parent_widget());
    }

    void spacer::set_parent_widget(i_widget*)
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
        return has_parent_widget() && parent_widget().has_surface() ? 
            parent_widget().surface().ppi() >= HIGH_DPI_PPI :
            service<i_surface_manager>().display().metrics().ppi() >= HIGH_DPI_PPI;
    }

    dimension spacer::dpi_scale_factor() const
    {
        return default_dpi_scale_factor(
            has_parent_widget() && parent_widget().has_surface() ?
                parent_widget().surface().ppi() : 
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

    bool spacer::has_margin() const noexcept
    {
        return false;
    }

    margin spacer::margin() const
    {
        return neogfx::margin{};
    }

    bool spacer::has_border() const noexcept
    {
        return false;
    }

    border spacer::border() const
    {
        return neogfx::border{};
    }

    bool spacer::has_padding() const noexcept
    {
        return false;
    }

    padding spacer::padding() const
    {
        return neogfx::padding{};
    }

    bool spacer::device_metrics_available() const
    {
        return has_parent_layout() && parent_layout().device_metrics_available();
    }

    const i_device_metrics& spacer::device_metrics() const
    {
        if (device_metrics_available())
            return parent_layout().device_metrics();
        return service<i_surface_manager>().display().metrics();
    }

    void spacer::layout_as(const point&, const size& aSize)
    {
#ifdef NEOGFX_DEBUG
        if (debug::layoutItem == this)
            service<debug::logger>() << "spacer::layout_as(..., " << aSize << ")" << endl;
#endif
        set_extents(aSize);
    }

    bool spacer::visible() const
    {
        return true;
    }

    horizontal_spacer::horizontal_spacer() :
        spacer{ neogfx::expansion_policy::ExpandHorizontally }
    {
    }

    horizontal_spacer::horizontal_spacer(i_layout& aParent) :
        spacer{ aParent, neogfx::expansion_policy::ExpandHorizontally }
    {
    }

    vertical_spacer::vertical_spacer() :
        spacer{ neogfx::expansion_policy::ExpandVertically }
    {
    }

    vertical_spacer::vertical_spacer(i_layout& aParent) :
        spacer{ aParent, neogfx::expansion_policy::ExpandVertically }
    {
    }
}