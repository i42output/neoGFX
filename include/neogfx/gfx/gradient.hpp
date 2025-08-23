// gradient.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2020 Leigh Johnston.  All Rights Reserved.
  
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

#include <neolib/core/variant.hpp>
#include <neolib/app/i_settings.hpp>

#include <neogfx/gfx/i_gradient.hpp>

namespace neogfx
{
    template <gradient_sharing Sharing>
    class basic_gradient : public reference_counted<i_gradient>
    {
        // types
    public:
        typedef i_gradient abstract_type;
        typedef neolib::pair<scalar, sRGB_color> color_stop;
        typedef neolib::pair<scalar, sRGB_color::view_component> alpha_stop;
        typedef neolib::vector<color_stop> color_stop_list;
        typedef neolib::vector<alpha_stop> alpha_stop_list;
        typedef color_stop_list::abstract_type abstract_color_stop_list;
        typedef alpha_stop_list::abstract_type abstract_alpha_stop_list;
        // construction
    public:
        basic_gradient();
        basic_gradient(const basic_gradient& aOther);
        basic_gradient(const i_gradient& aOther);
        basic_gradient(const i_ref_ptr<i_gradient>& aObject);
        explicit basic_gradient(const i_string& aCssDeclaration);
        explicit basic_gradient(const sRGB_color& aColor);
        basic_gradient(const sRGB_color& aColor, gradient_direction aDirection);
        basic_gradient(const sRGB_color& aColor1, const sRGB_color& aColor2, gradient_direction aDirection = gradient_direction::Vertical);
        basic_gradient(const abstract_color_stop_list& aColorStops, gradient_direction aDirection = gradient_direction::Vertical);
        basic_gradient(const abstract_color_stop_list& aColorStops, const abstract_alpha_stop_list& aAlphaStops, gradient_direction aDirection = gradient_direction::Vertical);
        basic_gradient(const basic_gradient& aOther, const abstract_color_stop_list& aColorStops);
        basic_gradient(const basic_gradient& aOther, const abstract_color_stop_list& aColorStops, const abstract_alpha_stop_list& aAlphaStops);
        basic_gradient(const neolib::i_vector<sRGB_color::abstract_type>& aColors, gradient_direction aDirection = gradient_direction::Vertical);
        basic_gradient(const std::initializer_list<sRGB_color>& aColors, gradient_direction aDirection = gradient_direction::Vertical);
    public:
        basic_gradient& operator=(const i_gradient& aOther) override;
    public:
        void clone(neolib::i_ref_ptr<i_gradient> & aResult) const override;
        // meta
    public:
        gradient_id id() const override;
        bool is_singular() const override;
        // operations
    public:
        abstract_color_stop_list const& color_stops() const override;
        abstract_color_stop_list& color_stops() override;
        abstract_alpha_stop_list const& alpha_stops() const override;
        abstract_alpha_stop_list& alpha_stops() override;
        abstract_color_stop_list::const_iterator find_color_stop(scalar aPos, bool aToInsert = false) const override;
        abstract_color_stop_list::const_iterator find_color_stop(scalar aPos, scalar aStart, scalar aEnd, bool aToInsert = false) const override;
        abstract_alpha_stop_list::const_iterator find_alpha_stop(scalar aPos, bool aToInsert = false) const override;
        abstract_alpha_stop_list::const_iterator find_alpha_stop(scalar aPos, scalar aStart, scalar aEnd, bool aToInsert = false) const override;
        abstract_color_stop_list::iterator find_color_stop(scalar aPos, bool aToInsert = false) override;
        abstract_color_stop_list::iterator find_color_stop(scalar aPos, scalar aStart, scalar aEnd, bool aToInsert = false) override;
        abstract_alpha_stop_list::iterator find_alpha_stop(scalar aPos, bool aToInsert = false) override;
        abstract_alpha_stop_list::iterator find_alpha_stop(scalar aPos, scalar aStart, scalar aEnd, bool aToInsert = false) override;
        abstract_color_stop_list::iterator insert_color_stop(scalar aPos) override;
        abstract_color_stop_list::iterator insert_color_stop(scalar aPos, scalar aStart, scalar aEnd) override;
        abstract_alpha_stop_list::iterator insert_alpha_stop(scalar aPos) override;
        abstract_alpha_stop_list::iterator insert_alpha_stop(scalar aPos, scalar aStart, scalar aEnd) override;
        sRGB_color at(scalar aPos) const override;
        sRGB_color at(scalar aPos, scalar aStart, scalar aEnd) const override;
        sRGB_color color_at(scalar aPos) const override;
        sRGB_color color_at(scalar aPos, scalar aStart, scalar aEnd) const override;
        sRGB_color::view_component alpha_at(scalar aPos) const override;
        sRGB_color::view_component alpha_at(scalar aPos, scalar aStart, scalar aEnd) const override;
        basic_gradient& reverse() override;
        basic_gradient& set_alpha(sRGB_color::view_component aAlpha) override;
        basic_gradient& set_combined_alpha(sRGB_color::view_component aAlpha) override;
        gradient_direction direction() const override;
        basic_gradient& set_direction(gradient_direction aDirection) override;
        gradient_orientation orientation() const override;
        basic_gradient& set_orientation(gradient_orientation aOrientation) override;
        gradient_shape shape() const override;
        basic_gradient& set_shape(gradient_shape aShape) override;
        gradient_size size() const override;
        basic_gradient& set_size(gradient_size aSize) override;
        const optional_vec2& exponents() const override;
        basic_gradient& set_exponents(const optional_vec2 & aExponents) override;
        const optional_point& center() const override;
        basic_gradient& set_center(const optional_point & aCenter) override;
        const std::optional<gradient_tile>& tile() const override;
        basic_gradient& set_tile(const std::optional<gradient_tile>& aTile) override;
        scalar smoothness() const override;
        basic_gradient& set_smoothness(scalar aSmoothness) override;
        const optional_rect& bounding_box() const override;
        basic_gradient& set_bounding_box(const optional_rect& aBoundingBox) override;
        basic_gradient& set_bounding_box_if_none(const optional_rect& aBoundingBox) override;
        // helpers
    public:
        basic_gradient with_bounding_box(const optional_rect& aBoundingBox) const
        {
            auto result = *this;
            result.set_bounding_box(aBoundingBox);
            return result;
        }
        basic_gradient with_bounding_box_if_none(const optional_rect& aBoundingBox) const
        {
            auto result = *this;
            if (result.bounding_box() == std::nullopt)
                result.set_bounding_box(aBoundingBox);
            return result;
        }
        // shader
    public:
        const i_gradient_sampler& colors() const override;
        const i_gradient_filter& filter() const override;
        // object
    private:
        void share_object(i_ref_ptr<i_gradient>& aRef) const override;
        // implementation
    private:
        i_gradient const& object() const;
        i_gradient& object();
        // attributes
    private:
        mutable neolib::ref_ptr<i_gradient> iObject;
        optional_rect iBoundingBox;
    };

    using shared_gradient = basic_gradient<gradient_sharing::Shared>;
    using unique_gradient = basic_gradient<gradient_sharing::Unique>;
    using gradient = shared_gradient;

    typedef neolib::optional<gradient> optional_gradient;
    typedef neolib::variant<color, gradient> color_or_gradient;
    typedef neolib::optional<color_or_gradient> optional_color_or_gradient;

    inline void apply_bounding_box(color_or_gradient& aColorOrGradient, rect const& aBoundingBox)
    {
        if (std::holds_alternative<gradient>(aColorOrGradient))
            static_variant_cast<gradient>(aColorOrGradient).set_bounding_box(aBoundingBox);
    }

    inline void apply_bounding_box(optional_color_or_gradient& aColorOrGradient, rect const& aBoundingBox)
    {
        if (aColorOrGradient != std::nullopt)
            apply_bounding_box(*aColorOrGradient, aBoundingBox);
    }

    inline color_or_gradient with_bounding_box(color_or_gradient const& aColorOrGradient, rect const& aBoundingBox, bool aOnlyIfTiled = false)
    {
        if (std::holds_alternative<gradient>(aColorOrGradient))
        {
            auto const& g = static_variant_cast<gradient const&>(aColorOrGradient);
            if (!aOnlyIfTiled || (g.tile() != std::nullopt && !g.tile()->aligned))
                return g.with_bounding_box(aBoundingBox);
        }
        return aColorOrGradient;
    }


    inline bool operator==(const gradient& aLhs, const gradient& aRhs)
    {
        if (aLhs.is_singular() != aRhs.is_singular())
            return false;
        else if (aLhs.is_singular())
            return false;
        else if (aLhs.id() == aRhs.id())
            return true;
        else
            return std::forward_as_tuple(aLhs.color_stops(), aLhs.alpha_stops(), aLhs.direction(), aLhs.orientation(), aLhs.shape(), aLhs.size(), aLhs.exponents(), aLhs.center(), aLhs.tile(), aLhs.smoothness()) ==
            std::forward_as_tuple(aRhs.color_stops(), aRhs.alpha_stops(), aRhs.direction(), aRhs.orientation(), aRhs.shape(), aRhs.size(), aRhs.exponents(), aRhs.center(), aRhs.tile(), aRhs.smoothness());
    }

    inline std::partial_ordering operator<=>(const gradient& aLhs, const gradient& aRhs)
    {
        if (aLhs.is_singular() || aRhs.is_singular())
        {
            if (aLhs.is_singular() == aRhs.is_singular())
                return std::partial_ordering::unordered;
            if (aLhs.is_singular() < aRhs.is_singular())
                return std::partial_ordering::less;
            else
                return std::partial_ordering::greater;
        }
        else if (aLhs.id() == aRhs.id())
            return std::partial_ordering::equivalent;
        else
            return std::forward_as_tuple(aLhs.color_stops(), aLhs.alpha_stops(), aLhs.direction(), aLhs.orientation(), aLhs.shape(), aLhs.size(), aLhs.exponents(), aLhs.center(), aLhs.tile(), aLhs.smoothness()) <=>
            std::forward_as_tuple(aRhs.color_stops(), aRhs.alpha_stops(), aRhs.direction(), aRhs.orientation(), aRhs.shape(), aRhs.size(), aRhs.exponents(), aRhs.center(), aRhs.tile(), aRhs.smoothness());
    }

    inline optional_color_or_gradient with_bounding_box(optional_color_or_gradient const& aColorOrGradient, rect const& aBoundingBox)
    {
        if (aColorOrGradient != std::nullopt)
            return with_bounding_box(*aColorOrGradient, aBoundingBox);
        return aColorOrGradient;
    }

    template <typename Elem, typename Traits>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const i_gradient& aGradient)
    {
        aStream << std::setprecision(4) << "[";
        aStream << "(";
        for (auto const& stop : aGradient.color_stops())
            aStream << (&stop != &aGradient.color_stops()[0] ? ", " : "") << stop.first() << ", " << sRGB_color{ stop.second() };
        aStream << ")";
        aStream << ", (";
        for (auto const& stop : aGradient.alpha_stops())
            aStream << (&stop != &aGradient.alpha_stops()[0] ? ", " : "") << stop.first() << ", " << stop.second() * 1.0 / std::numeric_limits<sRGB_color::view_component>::max();
        aStream << ")";
        aStream << ", " << enum_to_string(aGradient.direction());
        if (std::holds_alternative<scalar>(aGradient.orientation()))
            aStream << ", " << std::get<scalar>(aGradient.orientation());
        else
            aStream << ", " << enum_to_string(std::get<corner>(aGradient.orientation()));
        aStream << ", " << enum_to_string(aGradient.shape());
        aStream << ", " << enum_to_string(aGradient.size());
        aStream << ", (";
        if (aGradient.exponents())
            aStream << aGradient.exponents()->x << ", " << aGradient.exponents()->y;
        aStream << ")";
        aStream << ", (";
        if (aGradient.center())
            aStream << aGradient.center()->x << ", " << aGradient.center()->y;
        aStream << ")";
        aStream << ", (";
        if (aGradient.tile())
            aStream << aGradient.tile()->extents.cx << ", " << aGradient.tile()->extents.cy << ", " << aGradient.tile()->aligned;
        aStream << ")";
        aStream << ", " << aGradient.smoothness();
        aStream << "]";
        return aStream;
    }

    template <typename Elem, typename Traits>
    inline std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& aStream, i_gradient& aGradient)
    {
        auto previousImbued = aStream.getloc();
        if (typeid(std::use_facet<std::ctype<char>>(previousImbued)) != typeid(neolib::comma_as_whitespace))
            aStream.imbue(std::locale{ previousImbued, new neolib::comma_as_whitespace{} });
        char ignore;
        std::string tempString;
        scalar tempScalar;
        aStream >> ignore >> ignore;
        gradient::color_stop_list colorStops;
        while (aStream)
        {
            gradient::color_stop colorStop;
            aStream >> colorStop.first() >> colorStop.second();
            if (aStream)
                colorStops.push_back(colorStop);
        }
        aStream.clear();
        aStream >> ignore >> ignore;
        gradient::alpha_stop_list alphaStops;
        while (aStream)
        {
            gradient::alpha_stop alphaStop;
            aStream >> alphaStop.first() >> tempScalar;
            alphaStop.second() = static_cast<sRGB_color::view_component>(tempScalar * std::numeric_limits<sRGB_color::view_component>::max());
            if (aStream)
                alphaStops.push_back(alphaStop);
        }
        aStream.clear();
        aStream >> ignore;
        gradient_direction direction;
        aStream >> tempString;
        direction = neolib::string_to_enum<gradient_direction>(tempString);
        gradient_orientation orientation;
        if (aStream >> tempScalar)
        {
            orientation = tempScalar;
        }
        else
        {
            aStream.clear();
            aStream >> tempString;
            orientation = neolib::string_to_enum<corner>(tempString);
        }
        gradient_shape shape;
        aStream >> tempString;
        shape = neolib::string_to_enum<gradient_shape>(tempString);
        gradient_size size;
        aStream >> tempString;
        size = neolib::string_to_enum<gradient_size>(tempString);
        aStream >> ignore;
        optional_vec2 exponents;
        if (aStream >> tempScalar)
        {
            scalar y;
            aStream >> y;
            exponents.emplace(tempScalar, y);
        }
        else
            aStream.clear();
        aStream >> ignore;
        aStream >> ignore;
        optional_point center;
        if (aStream >> tempScalar)
        {
            scalar y;
            aStream >> y;
            center.emplace(tempScalar, y);
        }
        else
            aStream.clear();
        aStream >> ignore;
        aStream >> ignore;
        std::optional<gradient_tile> tile;
        if (aStream >> tempScalar)
        {
            scalar cy;
            bool aligned;
            aStream >> cy;
            aStream >> aligned;
            tile = gradient_tile{ neogfx::size{ tempScalar, cy }, aligned };
        }
        else
            aStream.clear();
        aStream >> ignore;
        scalar smoothness;
        aStream >> smoothness;
        aStream >> ignore;
        aGradient = gradient{ colorStops, alphaStops, direction };
        aGradient.set_orientation(orientation);
        aGradient.set_shape(shape);
        aGradient.set_size(size);
        aGradient.set_exponents(exponents);
        aGradient.set_center(center);
        aGradient.set_tile(tile);
        aGradient.set_smoothness(smoothness);
        aStream.imbue(previousImbued);
        return aStream;
    }

    template <typename Elem, typename Traits>
    inline std::basic_ostream<Elem, Traits>& operator<<(std::basic_ostream<Elem, Traits>& aStream, const color_or_gradient& aColorOrGradient)
    {
        aStream << '[';
        aStream << aColorOrGradient.index();
        if (aColorOrGradient.index() != 0)
        {
            aStream << ',';
            if (std::holds_alternative<color>(aColorOrGradient))
                aStream << static_variant_cast<color const&>(aColorOrGradient);
            else if (std::holds_alternative<gradient>(aColorOrGradient))
                aStream << static_variant_cast<gradient const&>(aColorOrGradient);
        }
        aStream << ']';
        return aStream;
    }

    template <typename Elem, typename Traits>
    inline std::basic_istream<Elem, Traits>& operator>>(std::basic_istream<Elem, Traits>& aStream, color_or_gradient& aColorOrGradient)
    {
        auto previousImbued = aStream.getloc();
        if (typeid(std::use_facet<std::ctype<char>>(previousImbued)) != typeid(neolib::comma_and_brackets_as_whitespace))
            aStream.imbue(std::locale{ previousImbued, new neolib::comma_as_whitespace{} });
        std::size_t index;
        aStream >> index;
        switch (index)
        {
        case 0:
            break;
        case 1:
            {
                color temp;
                aStream >> temp;
                aColorOrGradient = temp;
            }
            break;
        case 2:
            {
                gradient temp;
                aStream >> temp;
                aColorOrGradient = temp;
            }
            break;
        }
        aStream.imbue(previousImbued);
        return aStream;
    }
}

define_setting_type_as(neogfx::unique_gradient, neogfx::gradient)

template <>
struct neolib::as_setting<neogfx::gradient>
{
    typedef neogfx::unique_gradient type;
};