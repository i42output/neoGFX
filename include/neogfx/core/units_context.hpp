// units_context.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/core/i_units_context.hpp>

namespace neogfx
{ 
    class units_context : public i_units_context
    {
    public:
        units_context(const i_units_context& aSource);
    public:
        bool high_dpi() const override;
        dimension dpi_scale_factor() const override;
    public:
        bool device_metrics_available() const override;
        const i_device_metrics& device_metrics() const override;
        neogfx::units units() const override;
        neogfx::units set_units(neogfx::units aUnits) const override;
    private:
        const i_units_context& iSource;
        mutable neogfx::units iUnits;
    };

    class units_converter
    {
    public:
        struct bad_parameter : std::logic_error { bad_parameter() : std::logic_error("neogfx::units_converter::bad_parameter") {} };
        struct device_metrics_unavailable : std::logic_error { device_metrics_unavailable() : std::logic_error("neogfx::units_converter::device_metrics_unavailable") {} };
        // construction
    public:
        units_converter() = delete;
        units_converter(const i_units_context& aContext);
        ~units_converter();
        // operations
    public:
        neogfx::units saved_units() const;
        neogfx::units units() const;
        neogfx::units set_units(neogfx::units aUnits) const;
    public:
        vector2 to_device_units(const vector2& aValue) const;
        dimension to_device_units(dimension aValue) const;
        delta to_device_units(const delta& aValue) const;
        size to_device_units(const size& aValue) const;
        point to_device_units(const point& aValue) const;
        rect to_device_units(const rect& aValue) const;
        margins to_device_units(const margins& aValue) const;
        vector2 to_device_units(const size& aExtents, const vector2& aValue) const;
        dimension to_device_units(const size& aExtents, dimension aValue) const;
        delta to_device_units(const size& aExtents, const delta& aValue) const;
        size to_device_units(const size& aExtents, const size& aValue) const;
        point to_device_units(const size& aExtents, const point& aValue) const;
        rect to_device_units(const size& aExtents, const rect& aValue) const;
        vector2 from_device_units(const vector2& aValue) const;
        dimension from_device_units(dimension aValue) const;
        delta from_device_units(const delta& aValue) const;
        size from_device_units(const size& aValue) const;
        point from_device_units(const point& aValue) const;
        rect from_device_units(const rect& aValue) const;
        margins from_device_units(const margins& aValue) const;
        vector2 from_device_units(const size& aExtents, const vector2& aValue) const;
        dimension from_device_units(const size& aExtents, dimension aValue) const;
        delta from_device_units(const size& aExtents, const delta& aValue) const;
        size from_device_units(const size& aExtents, const size& aValue) const;
        point from_device_units(const size& aExtents, const point& aValue) const;
        rect from_device_units(const size& aExtents, const rect& aValue) const;
    public:
        template <typename T, uint32_t D>
        basic_vector<T, D> to_device_units(const basic_vector<T, D>& aValue)
        {
            basic_vector<T, D> result;
            for (uint32_t i = 0; i < D; ++i)
                result[i] = to_device_units(aValue[i]);
            return result;
        }
        template <typename T, uint32_t D>
        basic_vector<T, D> to_device_units(const size& aExtents, const basic_vector<T, D>& aValue)
        {
            basic_vector<T, D> result;
            for (uint32_t i = 0; i < D; ++i)
                result[i] = to_device_units(aExtents, aValue[i]);
            return result;
        }
        template <typename T, uint32_t D>
        basic_vector<T, D> from_device_units(const basic_vector<T, D>& aValue)
        {
            basic_vector<T, D> result;
            for (uint32_t i = 0; i < D; ++i)
                result[i] = from_device_units(aValue[i]);
            return result;
        }
        template <typename T, uint32_t D>
        basic_vector<T, D> from_device_units(const size& aExtents, const basic_vector<T, D>& aValue)
        {
            basic_vector<T, D> result;
            for (uint32_t i = 0; i < D; ++i)
                result[i] = from_device_units(aExtents, aValue[i]);
            return result;
        }
        // attributes
    private:
        const i_units_context& iContext;
        neogfx::units iSavedUnits;
    };

    class scoped_units
    {
        // construction
    public:
        scoped_units(const i_units_context& aUnitsContext, units aNewUnits) :
            iUnitsContext1(aUnitsContext), iUnitsContext2(aUnitsContext), iSavedUnits1(aUnitsContext.units()), iSavedUnits2(aUnitsContext.units())
        {
            iUnitsContext1.set_units(aNewUnits);
            iUnitsContext2.set_units(aNewUnits);
        }
        scoped_units(const i_units_context& aUnitsContext1, const i_units_context& aUnitsContext2, units aNewUnits) :
            iUnitsContext1(aUnitsContext1), iUnitsContext2(aUnitsContext2), iSavedUnits1(aUnitsContext1.units()), iSavedUnits2(aUnitsContext2.units())
        {
            iUnitsContext1.set_units(aNewUnits);
            iUnitsContext2.set_units(aNewUnits);
        }
        ~scoped_units()
        {
            restore_saved_units();
        }
        // operations
    public:
        units saved_units() const
        {
            return iSavedUnits1;
        }
        void restore_saved_units()
        {
            iUnitsContext1.set_units(iSavedUnits1);
            iUnitsContext2.set_units(iSavedUnits2);
        }
        // attributes
    private:
        const i_units_context& iUnitsContext1;
        const i_units_context& iUnitsContext2;
        units iSavedUnits1;
        units iSavedUnits2;
    };

    template<typename T>
    inline T as_units(const i_units_context& aSourceUnitsContext, units aUnits, const T& aValue)
    {
        if (aSourceUnitsContext.units() == aUnits)
            return aValue;
        units_converter uc(aSourceUnitsContext);
        uc.set_units(aUnits);
        T result = uc.to_device_units(aValue);
        uc.set_units(uc.saved_units());
        return uc.from_device_units(result);
    }

    template<typename T>
    inline T convert_units(const i_units_context& aSourceUnitsContext, units aNewUnits, const T& aValue)
    {
        if (aSourceUnitsContext.units() == aNewUnits)
            return aValue;
        T result = units_converter(aSourceUnitsContext).to_device_units(aValue);
        scoped_units su(aSourceUnitsContext, aNewUnits);
        return units_converter(aSourceUnitsContext).from_device_units(result);
    }

    template<typename T>
    inline T convert_units(const i_units_context& aSourceUnitsContext, const i_units_context& aDestinationUnitsContext, const T& aValue)
    {
        if (aSourceUnitsContext.units() == aDestinationUnitsContext.units())
        {
            if (aSourceUnitsContext.units() != units::Pixels ||
                (aSourceUnitsContext.device_metrics().horizontal_dpi() == aDestinationUnitsContext.device_metrics().horizontal_dpi() &&
                    aSourceUnitsContext.device_metrics().vertical_dpi() == aDestinationUnitsContext.device_metrics().vertical_dpi()))
            {
                return aValue;
            }
        }
        T result = units_converter(aSourceUnitsContext).to_device_units(aValue);
        {
            scoped_units su1(aSourceUnitsContext, units::Millimeters);
            result = units_converter(aSourceUnitsContext).from_device_units(aValue);
            scoped_units su2(aDestinationUnitsContext, units::Millimeters);
            result = units_converter(aDestinationUnitsContext).to_device_units(result);
        }
        return units_converter(aDestinationUnitsContext).from_device_units(result);
    }
}
