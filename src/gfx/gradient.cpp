// gradient.cpp
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

#include <neogfx/neogfx.hpp>
#include <neogfx/gfx/gradient.hpp>
#include <neogfx/gfx/i_gradient_manager.hpp>

namespace neogfx
{
    template <gradient_sharing Sharing>
    basic_gradient<Sharing>::basic_gradient()
    {
    }

    template <gradient_sharing Sharing>
    basic_gradient<Sharing>::basic_gradient(const basic_gradient& aOther) :
        basic_gradient{ static_cast<const i_gradient&>(aOther) }
    {
    }

    template <gradient_sharing Sharing>
    basic_gradient<Sharing>::basic_gradient(const i_gradient& aOther) :
        iBoundingBox{ aOther.bounding_box() }
    {
        if constexpr (Sharing == gradient_sharing::Shared)
            aOther.share_object(iObject);
        else
            iObject = aOther.clone();
    }

    template <gradient_sharing Sharing>
    basic_gradient<Sharing>::basic_gradient(const i_ref_ptr<i_gradient>& aObject)
    {
        if constexpr (Sharing == gradient_sharing::Shared)
            iObject = aObject;
        else
            iObject = aObject->clone();
    }

    template <gradient_sharing Sharing>
    basic_gradient<Sharing>::basic_gradient(const i_string& aCssDeclaration) :
        iObject{ service<i_gradient_manager>().create_gradient(aCssDeclaration) }
    {
    }

    template <gradient_sharing Sharing>
    basic_gradient<Sharing>::basic_gradient(const sRGB_color& aColor) :
        iObject{ service<i_gradient_manager>().create_gradient(aColor) }
    {
    }

    template <gradient_sharing Sharing>
    basic_gradient<Sharing>::basic_gradient(const sRGB_color& aColor, gradient_direction aDirection) :
        iObject{ service<i_gradient_manager>().create_gradient(aColor, aDirection) }
    {
    }

    template <gradient_sharing Sharing>
    basic_gradient<Sharing>::basic_gradient(const sRGB_color& aColor1, const sRGB_color& aColor2, gradient_direction aDirection) :
        iObject{ service<i_gradient_manager>().create_gradient(aColor1, aColor2, aDirection) }
    {
    }

    template <gradient_sharing Sharing>
    basic_gradient<Sharing>::basic_gradient(const abstract_color_stop_list& aColorStops, gradient_direction aDirection) :
        iObject{ service<i_gradient_manager>().create_gradient(aColorStops, aDirection) }
    {
    }

    template <gradient_sharing Sharing>
    basic_gradient<Sharing>::basic_gradient(const abstract_color_stop_list& aColorStops, const abstract_alpha_stop_list& aAlphaStops, gradient_direction aDirection) :
        iObject{ service<i_gradient_manager>().create_gradient(aColorStops, aAlphaStops, aDirection) }
    {
    }

    template <gradient_sharing Sharing>
    basic_gradient<Sharing>::basic_gradient(const basic_gradient& aOther, const abstract_color_stop_list& aColorStops, const abstract_alpha_stop_list& aAlphaStops) :
        iObject{ service<i_gradient_manager>().create_gradient(aOther, aColorStops, aAlphaStops) }
    {
    }

    template <gradient_sharing Sharing>
    basic_gradient<Sharing>::basic_gradient(const neolib::i_vector<sRGB_color>& aColors, gradient_direction aDirection) :
        iObject{ service<i_gradient_manager>().create_gradient(aColors, aDirection) }
    {
    }

    template <gradient_sharing Sharing>
    basic_gradient<Sharing>::basic_gradient(const std::initializer_list<sRGB_color>& aColors, gradient_direction aDirection) :
        basic_gradient{ neolib::vector<sRGB_color>{aColors}, aDirection }
    {
    }

    template <gradient_sharing Sharing>
    basic_gradient<Sharing>& basic_gradient<Sharing>::operator=(const i_gradient& aOther)
    {
        if (&aOther == this)
            return *this;
        iBoundingBox = aOther.bounding_box();
        if constexpr (Sharing == gradient_sharing::Shared)
            aOther.share_object(iObject);
        else
            iObject = aOther.clone();
        return *this;
    }

    template <gradient_sharing Sharing>
    void basic_gradient<Sharing>::clone(neolib::i_ref_ptr<i_gradient>& aResult) const
    {
        aResult = service<i_gradient_manager>().create_gradient(*this);
    }

    template <gradient_sharing Sharing>
    gradient_id basic_gradient<Sharing>::id() const
    {
        return object().id();
    }

    template <gradient_sharing Sharing>
    bool basic_gradient<Sharing>::is_singular() const
    {
        return iObject == nullptr;
    }

    template <gradient_sharing Sharing>
    typename basic_gradient<Sharing>::abstract_color_stop_list const& basic_gradient<Sharing>::color_stops() const
    {
        return object().color_stops();
    }

    template <gradient_sharing Sharing>
    typename basic_gradient<Sharing>::abstract_color_stop_list& basic_gradient<Sharing>::color_stops()
    {
        return object().color_stops();
    }

    template <gradient_sharing Sharing>
    typename basic_gradient<Sharing>::abstract_alpha_stop_list const& basic_gradient<Sharing>::alpha_stops() const
    {
        return object().alpha_stops();
    }

    template <gradient_sharing Sharing>
    typename basic_gradient<Sharing>::abstract_alpha_stop_list& basic_gradient<Sharing>::alpha_stops()
    {
        return object().alpha_stops();
    }

    template <gradient_sharing Sharing>
    typename basic_gradient<Sharing>::abstract_color_stop_list::const_iterator basic_gradient<Sharing>::find_color_stop(scalar aPos, bool aToInsert) const
    {
        return object().find_color_stop(aPos, aToInsert);
    }

    template <gradient_sharing Sharing>
    basic_gradient<Sharing>::abstract_color_stop_list::const_iterator basic_gradient<Sharing>::find_color_stop(scalar aPos, scalar aStart, scalar aEnd, bool aToInsert) const
    {
        return object().find_color_stop(aPos, aStart, aEnd, aToInsert);
    }

    template <gradient_sharing Sharing>
    typename basic_gradient<Sharing>::abstract_alpha_stop_list::const_iterator basic_gradient<Sharing>::find_alpha_stop(scalar aPos, bool aToInsert) const
    {
        return object().find_alpha_stop(aPos, aToInsert);
    }

    template <gradient_sharing Sharing>
    typename basic_gradient<Sharing>::abstract_alpha_stop_list::const_iterator basic_gradient<Sharing>::find_alpha_stop(scalar aPos, scalar aStart, scalar aEnd, bool aToInsert) const
    {
        return object().find_alpha_stop(aPos, aStart, aEnd, aToInsert);
    }

    template <gradient_sharing Sharing>
    typename basic_gradient<Sharing>::abstract_color_stop_list::iterator basic_gradient<Sharing>::find_color_stop(scalar aPos, bool aToInsert)
    {
        return object().find_color_stop(aPos, aToInsert);
    }

    template <gradient_sharing Sharing>
    typename basic_gradient<Sharing>::abstract_color_stop_list::iterator basic_gradient<Sharing>::find_color_stop(scalar aPos, scalar aStart, scalar aEnd, bool aToInsert)
    {
        return object().find_color_stop(aPos, aStart, aEnd, aToInsert);
    }

    template <gradient_sharing Sharing>
    typename basic_gradient<Sharing>::abstract_alpha_stop_list::iterator basic_gradient<Sharing>::find_alpha_stop(scalar aPos, bool aToInsert)
    {
        return object().find_alpha_stop(aPos, aToInsert);
    }

    template <gradient_sharing Sharing>
    typename basic_gradient<Sharing>::abstract_alpha_stop_list::iterator basic_gradient<Sharing>::find_alpha_stop(scalar aPos, scalar aStart, scalar aEnd, bool aToInsert)
    {
        return object().find_alpha_stop(aPos, aStart, aEnd, aToInsert);
    }

    template <gradient_sharing Sharing>
    typename basic_gradient<Sharing>::abstract_color_stop_list::iterator basic_gradient<Sharing>::insert_color_stop(scalar aPos)
    {
        return object().insert_color_stop(aPos);
    }

    template <gradient_sharing Sharing>
    typename basic_gradient<Sharing>::abstract_color_stop_list::iterator basic_gradient<Sharing>::insert_color_stop(scalar aPos, scalar aStart, scalar aEnd)
    {
        return object().insert_color_stop(aPos, aStart, aEnd);
    }

    template <gradient_sharing Sharing>
    typename basic_gradient<Sharing>::abstract_alpha_stop_list::iterator basic_gradient<Sharing>::insert_alpha_stop(scalar aPos)
    {
        return object().insert_alpha_stop(aPos);
    }

    template <gradient_sharing Sharing>
    typename basic_gradient<Sharing>::abstract_alpha_stop_list::iterator basic_gradient<Sharing>::insert_alpha_stop(scalar aPos, scalar aStart, scalar aEnd)
    {
        return object().insert_alpha_stop(aPos, aStart, aEnd);
    }

    template <gradient_sharing Sharing>
    sRGB_color basic_gradient<Sharing>::at(scalar aPos) const
    {
        return object().at(aPos);
    }

    template <gradient_sharing Sharing>
    sRGB_color basic_gradient<Sharing>::at(scalar aPos, scalar aStart, scalar aEnd) const
    {
        return object().at(aPos, aStart, aEnd);
    }

    template <gradient_sharing Sharing>
    sRGB_color basic_gradient<Sharing>::color_at(scalar aPos) const
    {
        return object().color_at(aPos);
    }

    template <gradient_sharing Sharing>
    sRGB_color basic_gradient<Sharing>::color_at(scalar aPos, scalar aStart, scalar aEnd) const
    {
        return object().color_at(aPos, aStart, aEnd);
    }

    template <gradient_sharing Sharing>
    sRGB_color::view_component basic_gradient<Sharing>::alpha_at(scalar aPos) const
    {
        return object().alpha_at(aPos);
    }

    template <gradient_sharing Sharing>
    sRGB_color::view_component basic_gradient<Sharing>::alpha_at(scalar aPos, scalar aStart, scalar aEnd) const
    {
        return object().alpha_at(aPos, aStart, aEnd);
    }

    template <gradient_sharing Sharing>
    void basic_gradient<Sharing>::reverse()
    {
        object().reverse();
    }

    template <gradient_sharing Sharing>
    void basic_gradient<Sharing>::set_alpha(sRGB_color::view_component aAlpha)
    {
        object().set_alpha(aAlpha);
    }

    template <gradient_sharing Sharing>
    void basic_gradient<Sharing>::set_combined_alpha(sRGB_color::view_component aAlpha)
    {
        object().set_combined_alpha(aAlpha);
    }

    template <gradient_sharing Sharing>
    gradient_direction basic_gradient<Sharing>::direction() const
    {
        return object().direction();
    }

    template <gradient_sharing Sharing>
    void basic_gradient<Sharing>::set_direction(gradient_direction aDirection)
    {
        object().set_direction(aDirection);
    }

    template <gradient_sharing Sharing>
    gradient_orientation basic_gradient<Sharing>::orientation() const
    {
        return object().orientation();
    }

    template <gradient_sharing Sharing>
    void basic_gradient<Sharing>::set_orientation(gradient_orientation aOrientation)
    {
        object().set_orientation(aOrientation);
    }

    template <gradient_sharing Sharing>
    gradient_shape basic_gradient<Sharing>::shape() const
    {
        return object().shape();
    }

    template <gradient_sharing Sharing>
    void basic_gradient<Sharing>::set_shape(gradient_shape aShape)
    {
        object().set_shape(aShape);
    }

    template <gradient_sharing Sharing>
    gradient_size basic_gradient<Sharing>::size() const
    {
        return object().size();
    }

    template <gradient_sharing Sharing>
    void basic_gradient<Sharing>::set_size(gradient_size aSize)
    {
        object().set_size(aSize);
    }

    template <gradient_sharing Sharing>
    const optional_vec2& basic_gradient<Sharing>::exponents() const
    {
        return object().exponents();
    }

    template <gradient_sharing Sharing>
    void basic_gradient<Sharing>::set_exponents(const optional_vec2& aExponents)
    {
        object().set_exponents(aExponents);
    }

    template <gradient_sharing Sharing>
    const optional_point& basic_gradient<Sharing>::center() const
    {
        return object().center();
    }

    template <gradient_sharing Sharing>
    void basic_gradient<Sharing>::set_center(const optional_point& aCenter)
    {
        object().set_center(aCenter);
    }

    template <gradient_sharing Sharing>
    scalar basic_gradient<Sharing>::smoothness() const
    {
        return object().smoothness();
    }

    template <gradient_sharing Sharing>
    void basic_gradient<Sharing>::set_smoothness(scalar aSmoothness)
    {
        object().set_smoothness(aSmoothness);
    }

    template <gradient_sharing Sharing>
    const optional_rect& basic_gradient<Sharing>::bounding_box() const
    {
        if (iBoundingBox != std::nullopt)
            return iBoundingBox;
        if (!is_singular())
            return object().bounding_box();
        return iBoundingBox;
    }

    template <gradient_sharing Sharing>
    void basic_gradient<Sharing>::set_bounding_box(const optional_rect& aBoundingBox)
    {
        iBoundingBox = aBoundingBox;
    }

    template <gradient_sharing Sharing>
    const i_gradient_sampler& basic_gradient<Sharing>::colors() const
    {
        return object().colors();
    }

    template <gradient_sharing Sharing>
    const i_gradient_filter& basic_gradient<Sharing>::filter() const
    {
        return object().filter();
    }

    template <gradient_sharing Sharing>
    void basic_gradient<Sharing>::share_object(i_ref_ptr<i_gradient>& aRef) const
    {
        if constexpr (Sharing == gradient_sharing::Shared)
            aRef = iObject;
        else
            aRef = iObject->clone();
    }

    template <gradient_sharing Sharing>
    const i_gradient& basic_gradient<Sharing>::object() const
    {
        if (is_singular())
            iObject = service<i_gradient_manager>().create_gradient();
        return *iObject;
    }

    template <gradient_sharing Sharing>
    i_gradient& basic_gradient<Sharing>::object()
    {
        if (is_singular())
            return const_cast<i_gradient&>(to_const(*this).object());
        if constexpr (Sharing == gradient_sharing::Shared)
        {
            if (iObject->reference_count() > 2)
                iObject = iObject->clone();
        }
        return *iObject;
    }

    template class basic_gradient<gradient_sharing::Shared>;
    template class basic_gradient<gradient_sharing::Unique>;
}
