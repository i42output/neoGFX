// units.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2019, 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/core/units.hpp>
#include <neogfx/hid/i_surface_manager.hpp>

namespace neogfx
{ 
    scoped_units_context::scoped_units_context(i_units_context const& aNewContext) : iSavedContext{ current_context_for_this_thread() }
    {
        set_context(&aNewContext);
    }
    
    scoped_units_context::~scoped_units_context()
    {
        restore_saved_context();
    }
   
    i_units_context const& scoped_units_context::current_context()
    {
        auto currentContext = current_context_for_this_thread();
        if (currentContext != nullptr)
            return *currentContext;
        return service<i_surface_manager>().display();
    }
    
    void scoped_units_context::set_context(const i_units_context* aNewContext)
    {
        current_context_for_this_thread() = aNewContext;
    }
    
    void scoped_units_context::restore_saved_context()
    {
        set_context(iSavedContext);
    }
    
    const i_units_context*& scoped_units_context::current_context_for_this_thread()
    {
        thread_local const i_units_context* tCurrentContext = nullptr;
        return tCurrentContext;
    }

    length_units_converter::length_units_converter(i_units_context const& aContext) :
        iContext(aContext), iUnits{ basic_scoped_units<length_units::units>::current_units() }
    {
    }

    length_units_converter::length_units_converter(i_units_context const& aContext, length_units::units aUnits) :
        iContext(aContext), iUnits{ aUnits }
    {
    }

    length_units_converter::~length_units_converter()
    {
    }

    length_units::units length_units_converter::units() const
    {
        return iUnits;
    }

    void length_units_converter::set_units(length_units::units aUnits)
    {
        iUnits = aUnits;
    }

    vector2 length_units_converter::to_device_units(const vector2& aValue) const
    {
        return iContext.device_metrics_available() ? to_device_units(iContext.device_metrics().extents(), aValue) : 
            units() == length_units::units::Pixels ? aValue : throw device_metrics_unavailable();
    }

    dimension length_units_converter::to_device_units(dimension aValue) const
    {
        return iContext.device_metrics_available() ? to_device_units(iContext.device_metrics().extents(), size{ aValue, 0 }).cx : 
            units() == length_units::units::Pixels ? aValue : throw device_metrics_unavailable();
    }

    delta length_units_converter::to_device_units(const delta& aValue) const
    {
        return iContext.device_metrics_available() ? to_device_units(iContext.device_metrics().extents(), aValue) : 
            units() == length_units::units::Pixels ? aValue : throw device_metrics_unavailable();
    }

    size length_units_converter::to_device_units(const size& aValue) const
    {
        return iContext.device_metrics_available() ? to_device_units(iContext.device_metrics().extents(), aValue) : 
            units() == length_units::units::Pixels ? aValue : throw device_metrics_unavailable();
    }

    point length_units_converter::to_device_units(const point& aValue) const
    {
        return iContext.device_metrics_available() ? to_device_units(iContext.device_metrics().extents(), aValue) : 
            units() == length_units::units::Pixels ? aValue : throw device_metrics_unavailable();
    }

    rect length_units_converter::to_device_units(const rect& aValue) const
    {
        return iContext.device_metrics_available() ? to_device_units(iContext.device_metrics().extents(), aValue) : 
            units() == length_units::units::Pixels ? aValue : throw device_metrics_unavailable();
    }

    padding length_units_converter::to_device_units(const padding& aValue) const
    {
        if (iContext.device_metrics_available())
        {
            auto const extents = iContext.device_metrics().extents();
            return padding{
                to_device_units(extents, size{ aValue.left, 0 }).cx,
                to_device_units(extents, size{ 0, aValue.top }).cy,
                to_device_units(extents, size{ aValue.right, 0 }).cx,
                to_device_units(extents, size{ 0, aValue.bottom }).cy };
        }
        else
            return units() == length_units::units::Pixels ? aValue : throw device_metrics_unavailable();
    }

    vector2 length_units_converter::to_device_units(const size& aExtents, const vector2& aValue) const
    {
        if (!iContext.device_metrics_available() && units() != length_units::units::Pixels && units() != length_units::units::Percentage)
            throw device_metrics_unavailable();
        switch (units())
        {
        default:
        case length_units::units::Pixels:
            return aValue;
        case length_units::units::ScaledPixels:
            return iContext.dpi_scale(aValue);
        case length_units::units::Points:
            return vector2{ aValue } *= vector2{ (1.0 / 72.0) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 72.0)* static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Picas:
            return vector2{ aValue } *= vector2{ (1.0 / 6.0) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 6.0) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Ems:
            return vector2{ aValue } *= vector2{ 1.0 * static_cast<dimension>(iContext.device_metrics().em_size()) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 1.0 * static_cast<dimension>(iContext.device_metrics().em_size()) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Millimetres:
            return vector2{ aValue } *= vector2{ (1.0 / 25.4) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 25.4) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Centimetres:
            return vector2{ aValue } *= vector2{ (1.0 / 2.54) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 2.54) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Inches:
            return vector2{ aValue } *= vector2{ 1.0 * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 1.0 * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Percentage:
            return vector2{ aExtents.cx, aExtents.cy } *= vector2{ aValue.x / 100.0, aValue.y / 100.0 };
        }
    }

    dimension length_units_converter::to_device_units(const size& aExtents, dimension aValue) const
    {
        if (!iContext.device_metrics_available() && units() != length_units::units::Pixels && units() != length_units::units::Percentage)
            throw device_metrics_unavailable();
        return to_device_units(aExtents, size{ aValue, 0 }).cx;
    }

    delta length_units_converter::to_device_units(const size& aExtents, const delta& aValue) const
    {
        if (!iContext.device_metrics_available() && units() != length_units::units::Pixels && units() != length_units::units::Percentage)
            throw device_metrics_unavailable();
        switch (units())
        {
        default:
        case length_units::units::Pixels:
            return aValue;
        case length_units::units::ScaledPixels:
            return iContext.dpi_scale(aValue);
        case length_units::units::Points:
            return aValue * delta{ (1.0 / 72.0) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 72.0) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Picas:
            return aValue * delta{ (1.0 / 6.0) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 6.0) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Ems:
            return aValue * delta{ 1.0 * static_cast<dimension>(iContext.device_metrics().em_size()) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 1.0 * static_cast<dimension>(iContext.device_metrics().em_size()) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Millimetres:
            return aValue * delta{ (1.0 / 25.4) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 25.4) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Centimetres:
            return aValue * delta{ (1.0 / 2.54) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 2.54) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Inches:
            return aValue * delta{ 1.0 * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 1.0 * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Percentage:
            return delta{ aExtents.to_delta() } * delta{ aValue.dx / 100.0, aValue.dy / 100.0 };
        }
    }

    size length_units_converter::to_device_units(const size& aExtents, const size& aValue) const
    {
        if (!iContext.device_metrics_available() && units() != length_units::units::Pixels && units() != length_units::units::Percentage)
            throw device_metrics_unavailable();
        switch (units())
        {
        default:
        case length_units::units::Pixels:
            return aValue;
        case length_units::units::ScaledPixels:
            return iContext.dpi_scale(aValue);
        case length_units::units::Points:
            return aValue * size{ (1.0 / 72.0) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 72.0) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Picas:
            return aValue * size{ (1.0 / 6.0) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 6.0) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Ems:
            return aValue * size{ 1.0 * static_cast<dimension>(iContext.device_metrics().em_size()) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 1.0 * static_cast<dimension>(iContext.device_metrics().em_size()) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Millimetres:
            return aValue * size{ (1.0 / 25.4) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 25.4) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Centimetres:
            return aValue * size{ (1.0 / 2.54) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 2.54) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Inches:
            return aValue * size{ 1.0 * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 1.0 * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Percentage:
            return size{ aExtents } * size{ aValue.cx / 100.0, aValue.cy / 100.0 };
        }
    }

    point length_units_converter::to_device_units(const size& aExtents, const point& aValue) const
    {
        if (!iContext.device_metrics_available() && units() != length_units::units::Pixels && units() != length_units::units::Percentage)
            throw device_metrics_unavailable();
        switch (units())
        {
        default:
        case length_units::units::Pixels:
            return aValue;
        case length_units::units::ScaledPixels:
            return iContext.dpi_scale(aValue);
        case length_units::units::Points:
            return aValue * point{ (1.0 / 72.0) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 72.0) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Picas:
            return aValue * point{ (1.0 / 6.0) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 6.0) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Ems:
            return aValue * point{ 1.0 * static_cast<dimension>(iContext.device_metrics().em_size()) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 1.0 * static_cast<dimension>(iContext.device_metrics().em_size()) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Millimetres:
            return aValue * point{ (1.0 / 25.4) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 25.4) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Centimetres:
            return aValue * point{ (1.0 / 2.54) * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / 2.54) * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Inches:
            return aValue * point{ 1.0 * static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 1.0 * static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Percentage:
            return point{ size{ aExtents } } * point{ aValue.x / 100.0, aValue.y / 100.0 };
        }
    }

    rect length_units_converter::to_device_units(const size& aExtents, const rect& aValue) const
    {
        if (!iContext.device_metrics_available() && units() != length_units::units::Pixels && units() != length_units::units::Percentage)
            throw device_metrics_unavailable();
        return rect(to_device_units(aExtents, aValue.position()), to_device_units(aExtents, aValue.extents()));
    }

    vector2 length_units_converter::from_device_units(const vector2& aValue) const
    {
        return iContext.device_metrics_available() ? from_device_units(iContext.device_metrics().extents(), aValue) :
            units() == length_units::units::Pixels ? aValue : throw device_metrics_unavailable();
    }

    dimension length_units_converter::from_device_units(dimension aValue) const
    {
        return iContext.device_metrics_available() ? from_device_units(iContext.device_metrics().extents(), size{ aValue, 0 }).cx :
            units() == length_units::units::Pixels ? aValue : throw device_metrics_unavailable();
    }

    delta length_units_converter::from_device_units(const delta& aValue) const
    {
        return iContext.device_metrics_available() ? from_device_units(iContext.device_metrics().extents(), aValue) :
            units() == length_units::units::Pixels ? aValue : throw device_metrics_unavailable();
    }

    size length_units_converter::from_device_units(const size& aValue) const
    {
        return iContext.device_metrics_available() ? from_device_units(iContext.device_metrics().extents(), aValue) :
            units() == length_units::units::Pixels ? aValue : throw device_metrics_unavailable();
    }

    point length_units_converter::from_device_units(const point& aValue) const
    {
        return iContext.device_metrics_available() ? from_device_units(iContext.device_metrics().extents(), aValue) :
            units() == length_units::units::Pixels ? aValue : throw device_metrics_unavailable();
    }

    rect length_units_converter::from_device_units(const rect& aValue) const
    {
        return iContext.device_metrics_available() ? from_device_units(iContext.device_metrics().extents(), aValue) :
            units() == length_units::units::Pixels ? aValue : throw device_metrics_unavailable();
    }

    padding length_units_converter::from_device_units(const padding& aValue) const
    {
        if (iContext.device_metrics_available())
        {
            auto const extents = iContext.device_metrics().extents();
            return padding{
                from_device_units(extents, size{ aValue.left, 0 }).cx,
                from_device_units(extents, size{ 0, aValue.top }).cy,
                from_device_units(extents, size{ aValue.right, 0 }).cx,
                from_device_units(extents, size{ 0, aValue.bottom }).cy };
        }
        else
            return units() == length_units::units::Pixels ? aValue : throw device_metrics_unavailable();
    }

    vector2 length_units_converter::from_device_units(const size& aExtents, const vector2& aValue) const
    {
        if (!iContext.device_metrics_available() && units() != length_units::units::Pixels && units() != length_units::units::Percentage)
            throw device_metrics_unavailable();
        switch (units())
        {
        default:
        case length_units::units::Pixels:
            return aValue;
        case length_units::units::ScaledPixels:
            return aValue / iContext.dpi_scale(1.0);
        case length_units::units::Points:
            return vector2{ aValue } *= vector2{ 72.0 / static_cast<scalar>(iContext.device_metrics().horizontal_dpi()), 72.0 / static_cast<scalar>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Picas:
            return vector2{ aValue } *= vector2{ 6.0 / static_cast<scalar>(iContext.device_metrics().horizontal_dpi()), 6.0 / static_cast<scalar>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Ems:
            return vector2{ aValue } *= vector2{ (1.0 / static_cast<scalar>(iContext.device_metrics().em_size())) / static_cast<scalar>(iContext.device_metrics().horizontal_dpi()), (1.0 / static_cast<dimension>(iContext.device_metrics().em_size())) / static_cast<dimension>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Millimetres:
            return vector2{ aValue } *= vector2{ 25.4 / static_cast<scalar>(iContext.device_metrics().horizontal_dpi()), 25.4 / static_cast<scalar>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Centimetres:
            return vector2{ aValue } *= vector2{ 2.54 / static_cast<scalar>(iContext.device_metrics().horizontal_dpi()), 2.54 / static_cast<scalar>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Inches:
            return vector2{ aValue } *= vector2{ 1.0 / static_cast<scalar>(iContext.device_metrics().horizontal_dpi()), 1.0 / static_cast<scalar>(iContext.device_metrics().vertical_dpi()) };
        case length_units::units::Percentage:
            return (vector2{ aValue } /= vector2{ aExtents.cx, aExtents.cy }) *= vector2{ 100.0, 100.0 };
        }
    }

    dimension length_units_converter::from_device_units(const size& aExtents, dimension aValue) const
    {
        if (!iContext.device_metrics_available() && units() != length_units::units::Pixels && units() != length_units::units::Percentage)
            throw device_metrics_unavailable();
        return from_device_units(aExtents, size{aValue, 0.0}).cx;
    }

    delta length_units_converter::from_device_units(const size& aExtents, const delta& aValue) const
    {
        if (!iContext.device_metrics_available() && units() != length_units::units::Pixels && units() != length_units::units::Percentage)
            throw device_metrics_unavailable();
        switch (units())
        {
        default:
        case length_units::units::Pixels:
            return aValue;
        case length_units::units::ScaledPixels:
            return aValue / delta{ iContext.dpi_scale(1.0) };
        case length_units::units::Points:
            return delta{ aValue } * delta(72.0 / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 72.0 / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Picas:
            return delta{ aValue } * delta(6.0 / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 6.0 / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Ems:
            return delta{ aValue } * delta((1.0 / static_cast<dimension>(iContext.device_metrics().em_size())) / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / static_cast<dimension>(iContext.device_metrics().em_size())) / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Millimetres:
            return delta{ aValue } * delta(25.4 / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 25.4 / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Centimetres:
            return delta{ aValue } * delta(2.54 / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 2.54 / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Inches:
            return delta{ aValue } * delta(1.0 / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 1.0 / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Percentage:
            return delta{ aValue } / delta(aExtents.to_delta()) * delta(100.0, 100.0);
        }
    }

    size length_units_converter::from_device_units(const size& aExtents, const size& aValue) const
    {
        if (!iContext.device_metrics_available() && units() != length_units::units::Pixels && units() != length_units::units::Percentage)
            throw device_metrics_unavailable();
        switch (units())
        {
        default:
        case length_units::units::Pixels:
            return aValue;
        case length_units::units::ScaledPixels:
            return aValue / iContext.dpi_scale(1.0);
        case length_units::units::Points:
            return size{ aValue } * size(72.0 / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 72.0 / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Picas:
            return size{ aValue } * size(6.0 / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 6.0 / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Ems:
            return size{ aValue } * size((1.0 / static_cast<dimension>(iContext.device_metrics().em_size())) / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / static_cast<dimension>(iContext.device_metrics().em_size())) / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Millimetres:
            return size{ aValue } * size(25.4 / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 25.4 / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Centimetres:
            return size{ aValue } * size(2.54 / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 2.54 / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Inches:
            return size{ aValue } * size(1.0 / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 1.0 / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Percentage:
            return size{ aValue } / size(aExtents) * size(100.0, 100.0);
        }
    }

    point length_units_converter::from_device_units(const size& aExtents, const point& aValue) const
    {
        if (!iContext.device_metrics_available() && units() != length_units::units::Pixels && units() != length_units::units::Percentage)
            throw device_metrics_unavailable();
        switch (units())
        {
        default:
        case length_units::units::Pixels:
            return aValue;
        case length_units::units::ScaledPixels:
            return aValue / iContext.dpi_scale(1.0);
        case length_units::units::Points:
            return point{ aValue } * point(72.0 / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 72.0 / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Picas:
            return point{ aValue } * point(6.0 / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 6.0 / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Ems:
            return point{ aValue } * point((1.0 / static_cast<dimension>(iContext.device_metrics().em_size())) / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), (1.0 / static_cast<dimension>(iContext.device_metrics().em_size())) / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Millimetres:
            return point{ aValue } * point(25.4 / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 25.4 / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Centimetres:
            return point{ aValue } * point(2.54 / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 2.54 / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Inches:
            return point{ aValue } * point(1.0 / static_cast<dimension>(iContext.device_metrics().horizontal_dpi()), 1.0 / static_cast<dimension>(iContext.device_metrics().vertical_dpi()));
        case length_units::units::Percentage:
            return point{ aValue } / point(aExtents) * point(100.0, 100.0);
        }
    }

    rect length_units_converter::from_device_units(const size& aExtents, const rect& aValue) const
    {
        if (!iContext.device_metrics_available() && units() != length_units::units::Pixels && units() != length_units::units::Percentage)
            throw device_metrics_unavailable();
        return rect{ from_device_units(aExtents, aValue.position()), from_device_units(aExtents, aValue.extents()) };
    }
}