// i_geometry.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neolib/core/i_enum.hpp>
#include <neogfx/core/units.hpp>

namespace neogfx
{
    enum class size_constraint : uint32_t
    {
        Fixed,
        Minimum,
        Maximum,
        Expanding,
        MinimumExpanding, // minimum unless a weight is defined
        ExpandingUniform, // leftover pixels are unwanted to ensure siblings are the same (pixel perfect) size after weighting
        Manual
    };

    enum class aspect_ratio : uint32_t
    {
        Ignore,
        Stretch,
        Keep,
        KeepExpanding
    };

    enum class cardinal : uint32_t
    {
        NorthWest,
        North,
        NorthEast,
        West,
        Center,
        East,
        SouthWest,
        South,
        SouthEast
    };

    enum class corner : uint32_t
    {
        TopLeft,
        TopRight,
        BottomRight,
        BottomLeft
    };
}

template <>
const neolib::enum_enumerators_t<neogfx::size_constraint> neolib::enum_enumerators_v<neogfx::size_constraint>
{
    declare_enum_string(neogfx::size_constraint, Fixed)
    declare_enum_string(neogfx::size_constraint, Minimum)
    declare_enum_string(neogfx::size_constraint, Maximum)
    declare_enum_string(neogfx::size_constraint, Expanding)
    declare_enum_string(neogfx::size_constraint, ExpandingUniform)
    declare_enum_string(neogfx::size_constraint, Manual)
};

template <>
const neolib::enum_enumerators_t<neogfx::aspect_ratio> neolib::enum_enumerators_v<neogfx::aspect_ratio>
{
    declare_enum_string(neogfx::aspect_ratio, Ignore)
    declare_enum_string(neogfx::aspect_ratio, Stretch)
    declare_enum_string(neogfx::aspect_ratio, Keep)
    declare_enum_string(neogfx::aspect_ratio, KeepExpanding)
};

template <>
const neolib::enum_enumerators_t<neogfx::cardinal> neolib::enum_enumerators_v<neogfx::cardinal>
{
    declare_enum_string(neogfx::cardinal, NorthWest)
    declare_enum_string(neogfx::cardinal, North)
    declare_enum_string(neogfx::cardinal, NorthEast)
    declare_enum_string(neogfx::cardinal, West)
    declare_enum_string(neogfx::cardinal, Center)
    declare_enum_string(neogfx::cardinal, East)
    declare_enum_string(neogfx::cardinal, SouthWest)
    declare_enum_string(neogfx::cardinal, South)
    declare_enum_string(neogfx::cardinal, SouthEast)
};

template <>
const neolib::enum_enumerators_t<neogfx::corner> neolib::enum_enumerators_v<neogfx::corner>
{
    declare_enum_string(neogfx::corner, TopLeft)
    declare_enum_string(neogfx::corner, TopRight)
    declare_enum_string(neogfx::corner, BottomRight)
    declare_enum_string(neogfx::corner, BottomLeft)
};

namespace neogfx
{
    class size_policy
    {
    public:
        struct no_aspect_ratio : std::logic_error { no_aspect_ratio() : std::logic_error("neogfx::size_policy::no_aspect_ratio") {} };
    public:
        size_policy(size_constraint aConstraint, const optional_size& aAspectRatio = {}) :
            iHorizontalConstraint{ aConstraint }, iVerticalConstraint{ aConstraint }, iAspectRatio{ aAspectRatio }
        {
        }
        size_policy(size_constraint aHorizontalConstraint, size_constraint aVerticalConstraint, const optional_size& aAspectRatio = {}) :
            iHorizontalConstraint{ aHorizontalConstraint }, iVerticalConstraint{ aVerticalConstraint }, iAspectRatio{ aAspectRatio }
        {
        }
    public:
        bool operator==(const size_policy& aRhs) const
        {
            return iHorizontalConstraint == aRhs.iHorizontalConstraint && iVerticalConstraint == aRhs.iVerticalConstraint && iAspectRatio == aRhs.iAspectRatio;
        }
        bool operator!=(const size_policy& aRhs) const
        {
            return !(*this == aRhs);
        }
    public:
        size_constraint horizontal_size_policy(bool aIgnoreUniformity = true) const
        {
            if (iHorizontalConstraint != size_constraint::ExpandingUniform)
                return iHorizontalConstraint;
            return aIgnoreUniformity ? size_constraint::Expanding : size_constraint::ExpandingUniform;
        }
        size_constraint vertical_size_policy(bool aIgnoreUniformity = true) const
        {
            if (iVerticalConstraint != size_constraint::ExpandingUniform)
                return iVerticalConstraint;
            return aIgnoreUniformity ? size_constraint::Expanding : size_constraint::ExpandingUniform;
        }
        void set_size_policy(size_constraint aConstraint)
        {
            iHorizontalConstraint = aConstraint;
            iVerticalConstraint = aConstraint;
        }
        void set_horizontal_size_policy(size_constraint aHorizontalConstraint)
        {
            iHorizontalConstraint = aHorizontalConstraint;
        }
        void set_vertical_size_policy(size_constraint aVerticalConstraint)
        {
            iVerticalConstraint = aVerticalConstraint;
        }
        bool maintain_aspect_ratio() const
        {
            return iAspectRatio != std::nullopt;
        }
        size aspect_ratio() const
        {
            if (maintain_aspect_ratio())
                return *iAspectRatio;
            throw no_aspect_ratio();
        }
        void set_aspect_ratio(const optional_size& aAspectRatio)
        {
            iAspectRatio = aAspectRatio;
        }
    public:
        static size_policy from_string(const std::string& aHorizontalConstraint, const std::string& aVerticalConstraint)
        {
            return size_policy{
                neolib::string_to_enum<size_constraint>(aHorizontalConstraint),
                neolib::string_to_enum<size_constraint>(aVerticalConstraint) };
        }
        void to_string(std::string& aHorizontalConstraint, std::string& aVerticalConstraint) const
        {
            aHorizontalConstraint = enum_to_string(horizontal_size_policy());
            aVerticalConstraint = enum_to_string(vertical_size_policy());
        }
    private:
        size_constraint iHorizontalConstraint;
        size_constraint iVerticalConstraint;
        optional_size iAspectRatio;
    };

    typedef std::optional<size_policy> optional_size_policy;

    class i_geometry : public i_units_context
    {
    public:
        struct no_fixed_size : std::logic_error { no_fixed_size() : std::logic_error("neogfx::i_geometry::no_fixed_size") {} };
    public:
        virtual point position() const = 0;
        virtual void set_position(const point& aPosition) = 0;
        virtual size extents() const = 0;
        virtual void set_extents(const size& aExtents) = 0;
        virtual bool has_size_policy() const = 0;
        virtual neogfx::size_policy size_policy() const = 0;
        virtual void set_size_policy(const optional_size_policy& aSizePolicy, bool aUpdateLayout = true) = 0;
        virtual bool has_weight() const = 0;
        virtual size weight() const = 0;
        virtual void set_weight(const optional_size& aWeight, bool aUpdateLayout = true) = 0;
        virtual bool has_minimum_size() const = 0;
        virtual size minimum_size(const optional_size& aAvailableSpace = {}) const = 0;
        virtual void set_minimum_size(const optional_size& aMinimumSize, bool aUpdateLayout = true) = 0;
        virtual bool has_maximum_size() const = 0;
        virtual size maximum_size(const optional_size& aAvailableSpace = {}) const = 0;
        virtual void set_maximum_size(const optional_size& aMaximumSize, bool aUpdateLayout = true) = 0;
        virtual bool has_fixed_size() const = 0;
        virtual size fixed_size() const = 0;
        virtual void set_fixed_size(const optional_size& aFixedSize, bool aUpdateLayout = true) = 0;
    public:
        virtual bool has_margins() const = 0;
        virtual neogfx::margins margins() const = 0;
        virtual void set_margins(const optional_margins& aMargins, bool aUpdateLayout = true) = 0;
        // helpers
    public:
        neogfx::size_policy effective_size_policy() const
        {
            auto effectivePolicy = size_policy();
            if (effectivePolicy.horizontal_size_policy() == size_constraint::MinimumExpanding)
                effectivePolicy.set_horizontal_size_policy(has_weight() ? size_constraint::Expanding : size_constraint::Minimum);
            if (effectivePolicy.vertical_size_policy() == size_constraint::MinimumExpanding)
                effectivePolicy.set_vertical_size_policy(has_weight() ? size_constraint::Expanding : size_constraint::Minimum);
            return effectivePolicy;
        }
        void set_size_policy(size_constraint aConstraint, bool aUpdateLayout = true)
        {
            set_size_policy(neogfx::size_policy{ aConstraint }, aUpdateLayout);
        }
        void set_size_policy(size_constraint aConstraint, const size& aAspectRatio, bool aUpdateLayout = true)
        {
            set_size_policy(neogfx::size_policy{ aConstraint, aAspectRatio }, aUpdateLayout);
        }
        void set_size_policy(size_constraint aHorizontalConstraint, size_constraint aVerticalConstraint, bool aUpdateLayout = true)
        {
            set_size_policy(neogfx::size_policy{ aHorizontalConstraint, aVerticalConstraint }, aUpdateLayout);
        }
        void set_size_policy(size_constraint aHorizontalConstraint, size_constraint aVerticalConstraint, const size& aAspectRatio, bool aUpdateLayout = true)
        {
            set_size_policy(neogfx::size_policy{ aHorizontalConstraint, aVerticalConstraint, aAspectRatio }, aUpdateLayout);
        }
        void set_minimum_width(dimension aWidth, bool aUpdateLayout = true)
        {
            auto newSize = minimum_size();
            newSize.cx = aWidth;
            set_minimum_size(newSize, aUpdateLayout);
        }
        void set_minimum_height(dimension aHeight, bool aUpdateLayout = true)
        {
            auto newSize = minimum_size();
            newSize.cy = aHeight;
            set_minimum_size(newSize, aUpdateLayout);
        }
        void set_maximum_width(dimension aWidth, bool aUpdateLayout = true)
        {
            auto newSize = maximum_size();
            newSize.cx = aWidth;
            set_maximum_size(newSize, aUpdateLayout);
        }
        void set_maximum_height(dimension aHeight, bool aUpdateLayout = true)
        {
            auto newSize = maximum_size();
            newSize.cy = aHeight;
            set_maximum_size(newSize, aUpdateLayout);
        }
    };

    class scoped_fixed_size_suppression
    {
    public:
        scoped_fixed_size_suppression(i_geometry& aGeometry) :
            iGeometry{ aGeometry },
            iFixedSize{ aGeometry.has_fixed_size() ? aGeometry.fixed_size() : std::optional<size>{} }
        {
            iGeometry.set_fixed_size({}, false);
        }
        ~scoped_fixed_size_suppression()
        {
            iGeometry.set_fixed_size(iFixedSize, false);
        }
    private:
        i_geometry& iGeometry;
        std::optional<size> iFixedSize;
    };

    struct size_hint
    {
        std::string primaryHint;
        std::string secondaryHint;

        operator bool() const
        {
            return !primaryHint.empty() || !secondaryHint.empty();
        }
        bool operator==(const size_hint& aOther) const
        {
            return primaryHint == aOther.primaryHint && secondaryHint == aOther.secondaryHint;
        }
        bool operator!=(const size_hint& aOther) const
        {
            return !(*this == aOther);
        }
    };
}