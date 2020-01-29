// units.hpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2019 Leigh Johnston.  All Rights Reserved.
  
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
#include <neogfx/core/geometrical.hpp>
#include <neogfx/core/device_metrics.hpp>

namespace neogfx
{ 
    class length_units
    {
    public:
        enum units
        {
            Pixels,
            Default = Pixels,
            ScaledPixels,
            Points,
            Picas,
            Ems,
            Millimetres,
            Millimeters = units::Millimetres,
            Centimetres,
            Centimeters = units::Centimetres,
            Inches,
            Percentage,
            NoUnitsAsMaximumLength
        };
    };

    class i_units_context
    {
    public:
        struct no_device_metrics : std::logic_error { no_device_metrics() : std::logic_error("neogfx::i_units_context::no_device_metrics") {} };
    public:
        virtual bool high_dpi() const = 0;
        virtual dimension dpi_scale_factor() const = 0;
    public:
        virtual bool device_metrics_available() const = 0;
        virtual const i_device_metrics& device_metrics() const = 0;
        // helpers
    public:
        template <typename T>
        std::enable_if_t<std::is_scalar_v<T>, T> dpi_scale(T aValue) const
        {
            return static_cast<T>(static_cast<dimension>(aValue) * dpi_scale_factor());
        }
        dimension dpi_scale(dimension aValue) const
        {
            return aValue * dpi_scale_factor();
        }
        vector2 dpi_scale(const vector2& aVector) const
        {
            return aVector * dpi_scale_factor();
        }
        size dpi_scale(const size& aSize) const
        {
            auto result = aSize;
            if (result.cx != size::max_dimension())
                result.cx *= dpi_scale_factor();
            if (result.cy != size::max_dimension())
                result.cy *= dpi_scale_factor();
            return result;
        }
        delta dpi_scale(const delta& aDelta) const
        {
            auto result = aDelta;
            if (result.dx != size::max_dimension())
                result.dx *= dpi_scale_factor();
            if (result.dy != size::max_dimension())
                result.dy *= dpi_scale_factor();
            return result;
        }
        point dpi_scale(const point& aPoint) const
        {
            return aPoint * dpi_scale_factor();
        }
        margins dpi_scale(const margins& aMargins) const
        {
            return aMargins * dpi_scale_factor();
        }
        template <typename T>
        T&& dpi_select(T&& aLowDpiValue, T&& aHighDpiValue) const
        {
            return std::forward<T>(high_dpi() ? aHighDpiValue : aLowDpiValue);
        }
    };

    class scoped_units_context
    {
        // construction
    public:
        scoped_units_context(const i_units_context& aNewContext);
        ~scoped_units_context();
        // operations
    public:
        static const i_units_context& current_context();
        // implementation
    private:
        void set_context(const i_units_context* aNewContext);
        void restore_saved_context();
        static const i_units_context*& current_context_for_this_thread();
        // attributes
    private:
        const i_units_context* iSavedContext;
    };

    template<typename T>
    inline T convert_units(const i_units_context& aUnitsContext, length_units::units aSourceUnits, length_units::units aDestinationUnits, const T& aValue);

    template <typename Units>
    class basic_scoped_units
    {
        // exceptions
    public:
        struct units_scope_ended : std::logic_error { units_scope_ended() : std::logic_error("neogfx::basic_scoped_units::units_scope_ended") {} };
        // types
    public:
        typedef Units units;
        // construction
    public:
        basic_scoped_units(units aNewUnits) : iSavedUnits{ current_units() }
        {
            set_units(aNewUnits);
        }
        basic_scoped_units(const i_units_context& aNewContext, units aNewUnits) : iScopedContext{ aNewContext }, iSavedUnits { current_units() }
        {
            set_units(aNewUnits);
        }
        ~basic_scoped_units()
        {
            end();
        }
        // operations
    public:
        static units current_units()
        {
            return current_units_for_this_thread();
        }
        units saved_units() const
        {
            if (iSavedUnits != std::nullopt)
                return *iSavedUnits;
            throw units_scope_ended();
        }
        void end()
        {
            restore_saved_units();
        }
        // implementation
    private:
        void set_units(units aNewUnits)
        {
            current_units_for_this_thread() = aNewUnits;
        }
        void restore_saved_units()
        {
            if (iSavedUnits != std::nullopt)
            {
                set_units(*iSavedUnits);
                iSavedUnits = std::nullopt;
            }
        }
        static units& current_units_for_this_thread() 
        {
            thread_local units tCurrentUnits = units::Default;
            return tCurrentUnits;
        }
        // attributes
    private:
        std::optional<scoped_units_context> iScopedContext;
        std::optional<units> iSavedUnits;
    };

    template <typename T>
    class basic_length : public length_units
    {
        typedef basic_length<T> self_type;
    public:
        struct unknown_unit : std::runtime_error { unknown_unit(const std::string& aUnitName) : std::runtime_error{ "neoGFX: Error: Unknown unit '" + aUnitName + "'" } {} };
    public:
        typedef T value_type;
    public:
        explicit basic_length(value_type aValue = value_type{}, length_units::units aUnits = length_units::Pixels) :
            iValue{ aValue }, iUnits{ aUnits }
        {
        }
    public:
        bool operator==(const self_type& aRhs) const
        {
            return iValue == aRhs.iValue && iUnits == aRhs.iUnits;
        }
        bool operator!=(const self_type& aRhs) const
        {
            return !(*this == aRhs);
        }
        bool operator<(const self_type& aRhs) const
        {
            return std::tie(iValue, iUnits) < std::tie(aRhs.iValue, aRhs.iUnits);
        }
    public:
        operator value_type() const
        {
            return value();
        }
        value_type value() const
        {
            if (units() == length_units::NoUnitsAsMaximumLength)
                return std::numeric_limits<value_type>::max();
            return convert_units(scoped_units_context::current_context(), units(), basic_scoped_units<length_units::units>::current_units(), unconverted_value());
        }
        value_type unconverted_value() const
        {
            return iValue;
        }
        length_units::units units() const
        {
            return iUnits;
        }
        std::string to_string(bool aToEmit = true) const
        {
            if (units() == length_units::NoUnitsAsMaximumLength)
                return (aToEmit ? "std::numeric_limits<dimension>::max()" : "max");
            std::ostringstream oss;
            oss << unconverted_value();
            if (aToEmit)
                oss << "_";
            switch (units())
            {
            case length_units::Pixels:
                oss << "px";
                break;
            case length_units::ScaledPixels:
                oss << "dip";
                break;
            case length_units::Points:
                oss << "pt";
                break;
            case length_units::Picas:
                oss << "pc";
                break;
            case length_units::Ems:
                oss << "em";
                break;
            case length_units::Millimetres:
                oss << "mm";
                break;
            case length_units::Centimetres:
                oss << "cm";
                break;
            case length_units::Inches:
                oss << "in";
                break;
            case length_units::Percentage:
                oss << "pct";
                break;
            }
            return oss.str();
        }
        static self_type from_string(const std::string& aValue)
        {
            static const std::unordered_map<std::string, length_units::units> sUnitTypes
            {
                { "px", length_units::Pixels },
                { "dip", length_units::ScaledPixels },
                { "pt", length_units::Points },
                { "pc", length_units::Picas },
                { "em", length_units::Ems },
                { "mm", length_units::Millimetres },
                { "cm", length_units::Centimetres },
                { "in", length_units::Inches },
                { "pct", length_units::Percentage }
            };
            if (aValue == "max")
                return self_type{ 0.0, length_units::NoUnitsAsMaximumLength };
            self_type result;
            std::istringstream iss{ aValue };
            iss >> result.iValue;
            std::string unitName;
            iss >> unitName;
            auto u = sUnitTypes.find(unitName);
            if (u == sUnitTypes.end())
                throw unknown_unit(unitName);
            result.iUnits = u->second;
            return result;
        }
    private:
        value_type iValue;
        length_units::units iUnits;
    };

    class length_units_converter
    {
    public:
        struct bad_parameter : std::logic_error { bad_parameter() : std::logic_error("neogfx::length_units_converter::bad_parameter") {} };
        struct device_metrics_unavailable : std::logic_error { device_metrics_unavailable() : std::logic_error("neogfx::length_units_converter::device_metrics_unavailable") {} };
        // construction
    public:
        length_units_converter() = delete;
        length_units_converter(const i_units_context& aContext);
        length_units_converter(const i_units_context& aContext, length_units::units aUnits);
        ~length_units_converter();
        // operations
    public:
        length_units::units units() const;
        void set_units(length_units::units aUnits);
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
        length_units::units iUnits;
    };

    template<typename T>
    inline T from_units(const i_units_context& aSourceUnitsContext, length_units::units aUnits, const T& aValue)
    {
        if (basic_scoped_units<length_units::units>::current_units() == aUnits)
            return aValue;
        length_units_converter uc(aSourceUnitsContext);
        uc.set_units(aUnits);
        T result = uc.to_device_units(aValue);
        uc.set_units(basic_scoped_units<length_units::units>::current_units());
        return uc.from_device_units(result);
    }

    template<typename T>
    inline T from_units(length_units::units aUnits, const T& aValue)
    {
        return from_units(scoped_units_context::current_context(), aUnits, aValue);
    }

    template<typename T>
    inline T to_units(const i_units_context& aSourceUnitsContext, length_units::units aNewUnits, const T& aValue)
    {
        if (basic_scoped_units<length_units::units>::current_units() == aNewUnits)
            return aValue;
        T result = length_units_converter(aSourceUnitsContext).to_device_units(aValue);
        basic_scoped_units<length_units::units> su(aSourceUnitsContext, aNewUnits);
        return length_units_converter(aSourceUnitsContext).from_device_units(result);
    }

    template<typename T>
    inline T to_units(length_units::units aNewUnits, const T& aValue)
    {
        return to_units(scoped_units_context::current_context(), aNewUnits, aValue);
    }

    template<typename T>
    inline T convert_units(const i_units_context& aSourceUnitsContext, const i_units_context& aDestinationUnitsContext, length_units::units aSourceUnits, length_units::units aDestinationUnits, const T& aValue)
    {
        if (aSourceUnits == aDestinationUnits)
        {
            if (aSourceUnits != length_units::Pixels ||
                (aSourceUnitsContext.device_metrics().horizontal_dpi() == aDestinationUnitsContext.device_metrics().horizontal_dpi() &&
                    aSourceUnitsContext.device_metrics().vertical_dpi() == aDestinationUnitsContext.device_metrics().vertical_dpi()))
            {
                return aValue;
            }
        }
        T const sourceInDeviceUnits = length_units_converter(aSourceUnitsContext, aSourceUnits).to_device_units(aValue);
        T const sourceInMillimetres = length_units_converter(aSourceUnitsContext, length_units::Millimetres).from_device_units(sourceInDeviceUnits);
        T const destinationInDeviceUnits = length_units_converter(aDestinationUnitsContext, length_units::Millimetres).to_device_units(sourceInMillimetres);
        auto const result = length_units_converter(aDestinationUnitsContext, aDestinationUnits).from_device_units(destinationInDeviceUnits);
        if (aDestinationUnits == length_units::Pixels || aDestinationUnits == length_units::ScaledPixels)
        {
            if constexpr (std::is_scalar_v<T>)
                return std::round(result);
            else
                return result.round();
        }
        return result;
    }

    template<typename T>
    inline T convert_units(const i_units_context& aUnitsContext, length_units::units aSourceUnits, length_units::units aDestinationUnits, const T& aValue)
    {
        return convert_units(aUnitsContext, aUnitsContext, aSourceUnits, aDestinationUnits, aValue);
    }

    template <typename T>
    inline T from_px(T aValue)
    {
        return from_units(length_units::Pixels, aValue);
    }

    template <typename T>
    inline T from_dip(T aValue)
    {
        return from_units(length_units::ScaledPixels, aValue);
    }

    template <typename T>
    inline T from_pt(T aValue)
    {
        return from_units(length_units::Points, aValue);
    }

    template <typename T>
    inline T from_pc(T aValue)
    {
        return from_units(length_units::Picas, aValue);
    }

    template <typename T>
    inline T from_em(T aValue)
    {
        return from_units(length_units::Ems, aValue);
    }

    template <typename T>
    inline T from_mm(T aValue)
    {
        return from_units(length_units::Millimetres, aValue);
    }

    template <typename T>
    inline T from_cm(T aValue)
    {
        return from_units(length_units::Centimetres, aValue);
    }

    template <typename T>
    inline T from_in(T aValue)
    {
        return from_units(length_units::Inches, aValue);
    }

    template <typename T>
    inline T from_pct(T aValue)
    {
        return from_units(length_units::Percentage, aValue);
    }

    template <typename ResultT, typename T>
    inline ResultT from_px(T aValue)
    {
        return static_cast<ResultT>(from_units(length_units::Pixels, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_dip(T aValue)
    {
        return static_cast<ResultT>(from_units(length_units::ScaledPixels, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_pt(T aValue)
    {
        return static_cast<ResultT>(from_units(length_units::Points, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_pc(T aValue)
    {
        return static_cast<ResultT>(from_units(length_units::Picas, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_em(T aValue)
    {
        return static_cast<ResultT>(from_units(length_units::Ems, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_mm(T aValue)
    {
        return static_cast<ResultT>(from_units(length_units::Millimetres, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_cm(T aValue)
    {
        return static_cast<ResultT>(from_units(length_units::Centimetres, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_in(T aValue)
    {
        return static_cast<ResultT>(from_units(length_units::Inches, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_pct(T aValue)
    {
        return static_cast<ResultT>(from_units(length_units::Percentage, aValue));
    }

    template <typename T>
    inline T to_px(T aValue)
    {
        return to_units(length_units::Pixels, aValue);
    }

    template <typename T>
    inline T to_dip(T aValue)
    {
        return to_units(length_units::ScaledPixels, aValue);
    }

    template <typename T>
    inline T to_pt(T aValue)
    {
        return to_units(length_units::Points, aValue);
    }

    template <typename T>
    inline T to_pc(T aValue)
    {
        return to_units(length_units::Picas, aValue);
    }

    template <typename T>
    inline T to_em(T aValue)
    {
        return to_units(length_units::Ems, aValue);
    }

    template <typename T>
    inline T to_mm(T aValue)
    {
        return to_units(length_units::Millimetres, aValue);
    }

    template <typename T>
    inline T to_cm(T aValue)
    {
        return to_units(length_units::Centimetres, aValue);
    }

    template <typename T>
    inline T to_in(T aValue)
    {
        return to_units(length_units::Inches, aValue);
    }

    template <typename T>
    inline T to_pct(T aValue)
    {
        return to_units(length_units::Percentage, aValue);
    }

    template <typename ResultT, typename T>
    inline ResultT to_px(T aValue)
    {
        return static_cast<ResultT>(to_units(length_units::Pixels, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_dip(T aValue)
    {
        return static_cast<ResultT>(to_units(length_units::ScaledPixels, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_pt(T aValue)
    {
        return static_cast<ResultT>(to_units(length_units::Points, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_pc(T aValue)
    {
        return static_cast<ResultT>(to_units(length_units::Picas, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_em(T aValue)
    {
        return static_cast<ResultT>(to_units(length_units::Ems, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_mm(T aValue)
    {
        return static_cast<ResultT>(to_units(length_units::Millimetres, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_cm(T aValue)
    {
        return static_cast<ResultT>(to_units(length_units::Centimetres, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_in(T aValue)
    {
        return static_cast<ResultT>(to_units(length_units::Inches, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_pct(T aValue)
    {
        return static_cast<ResultT>(to_units(length_units::Percentage, aValue));
    }

    template <typename T>
    inline basic_length<T> px(T aValue)
    {
        return basic_length<T>{ aValue, length_units::Pixels };
    }

    template <typename T>
    inline basic_length<T> dip(T aValue)
    {
        return basic_length<T>{ aValue, length_units::ScaledPixels };
    }

    template <typename T>
    inline basic_length<T> pt(T aValue)
    {
        return basic_length<T>{ aValue, length_units::Points };
    }

    template <typename T>
    inline basic_length<T> pc(T aValue)
    {
        return basic_length<T>{ aValue, length_units::Picas };
    }

    template <typename T>
    inline basic_length<T> em(T aValue)
    {
        return basic_length<T>{ aValue, length_units::Ems };
    }

    template <typename T>
    inline basic_length<T> mm(T aValue)
    {
        return basic_length<T>{ aValue, length_units::Millimetres };
    }

    template <typename T>
    inline basic_length<T> cm(T aValue)
    {
        return basic_length<T>{ aValue, length_units::Centimetres };
    }

    template <typename T>
    inline basic_length<T> in(T aValue)
    {
        return basic_length<T>{ aValue, length_units::Inches };
    }

    template <typename T>
    inline basic_length<T> pct(T aValue)
    {
        return basic_length<T>{ aValue, length_units::Percentage };
    }

    template <typename T>
    inline T rasterize(T aValue)
    {
        return from_px(std::ceil(to_px(aValue)));
    }

    template <typename T>
    inline basic_delta<T> rasterize(const basic_delta<T>& aDelta)
    {
        return basic_delta<T>{ rasterize(aDelta.dx), rasterize(aDelta.dy) };
    }

    template <typename T>
    inline basic_size<T> rasterize(const basic_size<T>& aSize)
    {
        return basic_size<T>{ rasterize(aSize.cx), rasterize(aSize.cy) };
    }

    template <typename T>
    inline basic_point<T> rasterize(const basic_point<T>& aPoint)
    {
        return basic_point<T>{ rasterize(aPoint.x), rasterize(aPoint.y) };
    }

    template <typename T>
    inline T rasterize(const basic_length<T>& aLength)
    {
        return rasterize(static_cast<T>(aLength));
    }

    namespace unit_literals
    {
        inline basic_length<default_geometry_value_type> operator ""_px(long double aValue)
        {
            return px(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_dip(long double aValue)
        {
            return dip(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_pt(long double aValue)
        {
            return pt(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_pc(long double aValue)
        {
            return pc(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_em(long double aValue)
        {
            return em(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_mm(long double aValue)
        {
            return mm(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_cm(long double aValue)
        {
            return cm(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_in(long double aValue)
        {
            return in(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_pct(long double aValue)
        {
            return pct(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_px(unsigned long long int aValue)
        {
            return px(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_dip(unsigned long long int aValue)
        {
            return dip(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_pt(unsigned long long int aValue)
        {
            return pt(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_pc(unsigned long long int aValue)
        {
            return pc(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_em(unsigned long long int aValue)
        {
            return em(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_mm(unsigned long long int aValue)
        {
            return mm(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_cm(unsigned long long int aValue)
        {
            return cm(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_in(unsigned long long int aValue)
        {
            return in(static_cast<default_geometry_value_type>(aValue));
        }

        inline basic_length<default_geometry_value_type> operator ""_pct(unsigned long long int aValue)
        {
            return pct(static_cast<default_geometry_value_type>(aValue));
        }
    }

    using namespace unit_literals;

    typedef basic_length<default_geometry_value_type> length;
    typedef length_units::units units;
    typedef basic_scoped_units<units> scoped_units;
    typedef length_units_converter units_converter;
}
