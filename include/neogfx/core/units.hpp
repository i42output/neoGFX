// units.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/core/device_metrics.hpp>

namespace neogfx
{ 
    enum class units
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

    class i_units_context
    {
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
        padding dpi_scale(const padding& aPadding) const
        {
            return aPadding * dpi_scale_factor();
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
        scoped_units_context(i_units_context const& aNewContext);
        ~scoped_units_context();
        // operations
    public:
        static i_units_context const& current_context();
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
    inline T convert_units(i_units_context const& aUnitsContext, units aSourceUnits, units aDestinationUnits, const T& aValue);

    template <typename Units>
    class basic_scoped_units
    {
        // exceptions
    public:
        struct units_scope_ended : std::logic_error { units_scope_ended() : std::logic_error("neogfx::basic_scoped_units_scope_ended") {} };
        // types
    public:
        typedef Units units;
        // construction
    public:
        basic_scoped_units(units aNewUnits) : iSavedUnits{ current_units() }
        {
            set_units(aNewUnits);
        }
        basic_scoped_units(i_units_context const& aNewContext, units aNewUnits) : iScopedContext{ aNewContext }, iSavedUnits { current_units() }
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
    class basic_length
    {
        typedef basic_length<T> self_type;
    public:
        struct unknown_unit : std::runtime_error { unknown_unit(std::string const& aUnitName) : std::runtime_error{ "neoGFX: Error: Unknown unit '" + aUnitName + "'" } {} };
    public:
        typedef T value_type;
    public:
        explicit basic_length(value_type aValue = value_type{}, neogfx::units aUnits = neogfx::units::Pixels) :
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
            return std::forward_as_tuple(iValue, iUnits) < std::forward_as_tuple(aRhs.iValue, aRhs.iUnits);
        }
    public:
        operator value_type() const
        {
            return value();
        }
        value_type value() const
        {
            if (units() == units::NoUnitsAsMaximumLength)
                return std::numeric_limits<value_type>::infinity();
            return convert_units(scoped_units_context::current_context(), units(), basic_scoped_units<neogfx::units>::current_units(), unconverted_value());
        }
        value_type unconverted_value() const
        {
            return iValue;
        }
        neogfx::units units() const
        {
            return iUnits;
        }
        std::string to_string(bool aToEmit = true) const
        {
            if (units() == neogfx::units::NoUnitsAsMaximumLength)
                return (aToEmit ? "std::numeric_limits<dimension>::infinity()" : "max");
            std::ostringstream oss;
            oss << unconverted_value();
            if (aToEmit)
                oss << "_";
            switch (units())
            {
            case neogfx::units::Pixels:
                oss << "px";
                break;
            case neogfx::units::ScaledPixels:
                oss << "dip";
                break;
            case neogfx::units::Points:
                oss << "pt";
                break;
            case neogfx::units::Picas:
                oss << "pc";
                break;
            case neogfx::units::Ems:
                oss << "em";
                break;
            case neogfx::units::Millimetres:
                oss << "mm";
                break;
            case neogfx::units::Centimetres:
                oss << "cm";
                break;
            case neogfx::units::Inches:
                oss << "in";
                break;
            case neogfx::units::Percentage:
                oss << "pct";
                break;
            }
            return oss.str();
        }
        static self_type from_string(std::string const& aValue)
        {
            static const std::unordered_map<std::string, neogfx::units> sUnitTypes
            {
                { "px", neogfx::units::Pixels },
                { "dip", neogfx::units::ScaledPixels },
                { "dp", neogfx::units::ScaledPixels },
                { "pt", neogfx::units::Points },
                { "pc", neogfx::units::Picas },
                { "em", neogfx::units::Ems },
                { "mm", neogfx::units::Millimetres },
                { "cm", neogfx::units::Centimetres },
                { "in", neogfx::units::Inches },
                { "pct", neogfx::units::Percentage }
            };
            if (aValue == "max")
                return self_type{ 0.0, neogfx::units::NoUnitsAsMaximumLength };
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
        neogfx::units iUnits;
    };

    class units_converter
    {
    public:
        struct bad_parameter : std::logic_error { bad_parameter() : std::logic_error("neogfx::units_converter::bad_parameter") {} };
        // construction
    public:
        units_converter() = delete;
        units_converter(i_units_context const& aContext);
        units_converter(i_units_context const& aContext, neogfx::units aUnits);
        ~units_converter();
        // operations
    public:
        neogfx::units units() const;
        void set_units(neogfx::units aUnits);
    public:
        vector2 to_device_units(const vector2& aValue) const;
        dimension to_device_units(dimension aValue) const;
        delta to_device_units(const delta& aValue) const;
        size to_device_units(const size& aValue) const;
        point to_device_units(const point& aValue) const;
        rect to_device_units(const rect& aValue) const;
        padding to_device_units(const padding& aValue) const;
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
        padding from_device_units(const padding& aValue) const;
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
        i_units_context const& iContext;
        neogfx::units iUnits;
    };

    template<typename T>
    inline T from_units(i_units_context const& aSourceUnitsContext, units aUnits, const T& aValue)
    {
        if (basic_scoped_units<units>::current_units() == aUnits)
            return aValue;
        units_converter uc(aSourceUnitsContext);
        uc.set_units(aUnits);
        auto const result = static_cast<T>(uc.to_device_units(aValue));
        uc.set_units(basic_scoped_units<units>::current_units());
        return static_cast<T>(uc.from_device_units(result));
    }

    template<typename T>
    inline T from_units(units aUnits, const T& aValue)
    {
        return from_units(scoped_units_context::current_context(), aUnits, aValue);
    }

    template<typename T>
    inline T to_units(i_units_context const& aSourceUnitsContext, units aNewUnits, const T& aValue)
    {
        if (basic_scoped_units<units>::current_units() == aNewUnits)
            return aValue;
        T result = units_converter(aSourceUnitsContext).to_device_units(aValue);
        basic_scoped_units<units> su(aSourceUnitsContext, aNewUnits);
        return units_converter(aSourceUnitsContext).from_device_units(result);
    }

    template<typename T>
    inline T to_units(units aNewUnits, const T& aValue)
    {
        return to_units(scoped_units_context::current_context(), aNewUnits, aValue);
    }

    template<typename T>
    inline T convert_units(i_units_context const& aSourceUnitsContext, i_units_context const& aDestinationUnitsContext, units aSourceUnits, units aDestinationUnits, const T& aValue)
    {
        if (aSourceUnits == aDestinationUnits)
        {
            if (aSourceUnits != units::Pixels ||
                (aSourceUnitsContext.device_metrics().horizontal_dpi() == aDestinationUnitsContext.device_metrics().horizontal_dpi() &&
                    aSourceUnitsContext.device_metrics().vertical_dpi() == aDestinationUnitsContext.device_metrics().vertical_dpi()))
            {
                return aValue;
            }
        }
        T const sourceInDeviceUnits = units_converter(aSourceUnitsContext, aSourceUnits).to_device_units(aValue);
        T const sourceInMillimetres = units_converter(aSourceUnitsContext, units::Millimetres).from_device_units(sourceInDeviceUnits);
        T const destinationInDeviceUnits = units_converter(aDestinationUnitsContext, units::Millimetres).to_device_units(sourceInMillimetres);
        auto const result = units_converter(aDestinationUnitsContext, aDestinationUnits).from_device_units(destinationInDeviceUnits);
        if (aDestinationUnits == units::Pixels || aDestinationUnits == units::ScaledPixels)
        {
            if constexpr (std::is_scalar_v<T>)
                return std::round(result);
            else
                return result.round();
        }
        return result;
    }

    template<typename T>
    inline T convert_units(i_units_context const& aUnitsContext, units aSourceUnits, units aDestinationUnits, const T& aValue)
    {
        return convert_units(aUnitsContext, aUnitsContext, aSourceUnits, aDestinationUnits, aValue);
    }

    template <typename T>
    inline T from_px(T aValue)
    {
        return from_units(units::Pixels, aValue);
    }

    template <typename T>
    inline T from_dip(T aValue)
    {
        return from_units(units::ScaledPixels, aValue);
    }

    template <typename T>
    inline T from_pt(T aValue)
    {
        return from_units(units::Points, aValue);
    }

    template <typename T>
    inline T from_pc(T aValue)
    {
        return from_units(units::Picas, aValue);
    }

    template <typename T>
    inline T from_em(T aValue)
    {
        return from_units(units::Ems, aValue);
    }

    template <typename T>
    inline T from_mm(T aValue)
    {
        return from_units(units::Millimetres, aValue);
    }

    template <typename T>
    inline T from_cm(T aValue)
    {
        return from_units(units::Centimetres, aValue);
    }

    template <typename T>
    inline T from_in(T aValue)
    {
        return from_units(units::Inches, aValue);
    }

    template <typename T>
    inline T from_pct(T aValue)
    {
        return from_units(units::Percentage, aValue);
    }

    template <typename ResultT, typename T>
    inline ResultT from_px(T aValue)
    {
        return static_cast<ResultT>(from_units(units::Pixels, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_dip(T aValue)
    {
        return static_cast<ResultT>(from_units(units::ScaledPixels, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_pt(T aValue)
    {
        return static_cast<ResultT>(from_units(units::Points, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_pc(T aValue)
    {
        return static_cast<ResultT>(from_units(units::Picas, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_em(T aValue)
    {
        return static_cast<ResultT>(from_units(units::Ems, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_mm(T aValue)
    {
        return static_cast<ResultT>(from_units(units::Millimetres, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_cm(T aValue)
    {
        return static_cast<ResultT>(from_units(units::Centimetres, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_in(T aValue)
    {
        return static_cast<ResultT>(from_units(units::Inches, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT from_pct(T aValue)
    {
        return static_cast<ResultT>(from_units(units::Percentage, aValue));
    }

    template <typename T>
    inline T to_px(T aValue)
    {
        return to_units(units::Pixels, aValue);
    }

    template <typename T>
    inline T to_dip(T aValue)
    {
        return to_units(units::ScaledPixels, aValue);
    }

    template <typename T>
    inline T to_pt(T aValue)
    {
        return to_units(units::Points, aValue);
    }

    template <typename T>
    inline T to_pc(T aValue)
    {
        return to_units(units::Picas, aValue);
    }

    template <typename T>
    inline T to_em(T aValue)
    {
        return to_units(units::Ems, aValue);
    }

    template <typename T>
    inline T to_mm(T aValue)
    {
        return to_units(units::Millimetres, aValue);
    }

    template <typename T>
    inline T to_cm(T aValue)
    {
        return to_units(units::Centimetres, aValue);
    }

    template <typename T>
    inline T to_in(T aValue)
    {
        return to_units(units::Inches, aValue);
    }

    template <typename T>
    inline T to_pct(T aValue)
    {
        return to_units(units::Percentage, aValue);
    }

    template <typename ResultT, typename T>
    inline ResultT to_px(T aValue)
    {
        return static_cast<ResultT>(to_units(units::Pixels, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_dip(T aValue)
    {
        return static_cast<ResultT>(to_units(units::ScaledPixels, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_pt(T aValue)
    {
        return static_cast<ResultT>(to_units(units::Points, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_pc(T aValue)
    {
        return static_cast<ResultT>(to_units(units::Picas, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_em(T aValue)
    {
        return static_cast<ResultT>(to_units(units::Ems, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_mm(T aValue)
    {
        return static_cast<ResultT>(to_units(units::Millimetres, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_cm(T aValue)
    {
        return static_cast<ResultT>(to_units(units::Centimetres, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_in(T aValue)
    {
        return static_cast<ResultT>(to_units(units::Inches, aValue));
    }

    template <typename ResultT, typename T>
    inline ResultT to_pct(T aValue)
    {
        return static_cast<ResultT>(to_units(units::Percentage, aValue));
    }

    template <typename T>
    inline basic_length<T> px(T aValue)
    {
        return basic_length<T>{ aValue, units::Pixels };
    }

    template <typename T>
    inline basic_length<T> dip(T aValue)
    {
        return basic_length<T>{ aValue, units::ScaledPixels };
    }

    template <typename T>
    inline basic_length<T> pt(T aValue)
    {
        return basic_length<T>{ aValue, units::Points };
    }

    template <typename T>
    inline basic_length<T> pc(T aValue)
    {
        return basic_length<T>{ aValue, units::Picas };
    }

    template <typename T>
    inline basic_length<T> em(T aValue)
    {
        return basic_length<T>{ aValue, units::Ems };
    }

    template <typename T>
    inline basic_length<T> mm(T aValue)
    {
        return basic_length<T>{ aValue, units::Millimetres };
    }

    template <typename T>
    inline basic_length<T> cm(T aValue)
    {
        return basic_length<T>{ aValue, units::Centimetres };
    }

    template <typename T>
    inline basic_length<T> in(T aValue)
    {
        return basic_length<T>{ aValue, units::Inches };
    }

    template <typename T>
    inline basic_length<T> pct(T aValue)
    {
        return basic_length<T>{ aValue, units::Percentage };
    }

    template <typename T>
    inline T ceil_rasterized(T aValue)
    {
        return from_px(std::ceil(to_px(aValue)));
    }

    template <typename T>
    inline basic_delta<T> ceil_rasterized(const basic_delta<T>& aDelta)
    {
        return basic_delta<T>{ ceil_rasterized(aDelta.dx), ceil_rasterized(aDelta.dy) };
    }

    template <typename T>
    inline basic_size<T> ceil_rasterized(const basic_size<T>& aSize)
    {
        return basic_size<T>{ ceil_rasterized(aSize.cx), ceil_rasterized(aSize.cy) };
    }

    template <typename T>
    inline basic_point<T> ceil_rasterized(const basic_point<T>& aPoint)
    {
        return basic_point<T>{ ceil_rasterized(aPoint.x), ceil_rasterized(aPoint.y) };
    }

    template <typename T>
    inline basic_box_areas<T> ceil_rasterized(const basic_box_areas<T>& aAreas)
    {
        return basic_box_areas<T>{ ceil_rasterized(aAreas.left), ceil_rasterized(aAreas.top), ceil_rasterized(aAreas.right), ceil_rasterized(aAreas.bottom) };
    }

    template <typename T>
    inline basic_rect<T> ceil_rasterized(const basic_rect<T>& aRect)
    {
        return basic_rect<T>{ ceil_rasterized(aRect.position()), ceil_rasterized(aRect.extents()) };
    }

    template <typename T>
    inline T ceil_rasterized(const basic_length<T>& aLength)
    {
        return ceil_rasterized(static_cast<T>(aLength));
    }

    template <typename T>
    inline T floor_rasterized(T aValue)
    {
        return from_px(std::floor(to_px(aValue)));
    }

    template <typename T>
    inline basic_delta<T> floor_rasterized(const basic_delta<T>& aDelta)
    {
        return basic_delta<T>{ floor_rasterized(aDelta.dx), floor_rasterized(aDelta.dy) };
    }

    template <typename T>
    inline basic_size<T> floor_rasterized(const basic_size<T>& aSize)
    {
        return basic_size<T>{ floor_rasterized(aSize.cx), floor_rasterized(aSize.cy) };
    }

    template <typename T>
    inline basic_point<T> floor_rasterized(const basic_point<T>& aPoint)
    {
        return basic_point<T>{ floor_rasterized(aPoint.x), floor_rasterized(aPoint.y) };
    }

    template <typename T>
    inline basic_rect<T> floor_rasterized(const basic_rect<T>& aRect)
    {
        return basic_rect<T>{ floor_rasterized(aRect.position()), floor_rasterized(aRect.extents()) };
    }

    template <typename T>
    inline T floor_rasterized(const basic_length<T>& aLength)
    {
        return floor_rasterized(static_cast<T>(aLength));
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

        inline basic_length<default_geometry_value_type> operator ""_dp(long double aValue)
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

        inline basic_length<default_geometry_value_type> operator ""_dp(unsigned long long int aValue)
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
    typedef basic_scoped_units<units> scoped_units;
}
