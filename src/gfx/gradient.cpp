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
    gradient::gradient()
    {
    }

    gradient::gradient(const gradient& aOther) : 
        gradient{ static_cast<const i_gradient&>(aOther) }
    {
    }

    gradient::gradient(const i_gradient& aOther) :
        iCopy{ true },
        iBoundingBox{ aOther.bounding_box() }
    {
        aOther.share_object(iObject);
    }

    gradient::gradient(const i_ref_ptr<i_gradient>& aObject) :
        iObject{ aObject }
    {
    }

    gradient::gradient(const i_string& aCssDeclaration) :
        iObject{ service<i_gradient_manager>().create_gradient(aCssDeclaration) }
    {
    }

    gradient::gradient(const sRGB_color& aColor) :
        iObject{ service<i_gradient_manager>().create_gradient(aColor) }
    {
    }

    gradient::gradient(const sRGB_color& aColor, gradient_direction aDirection) :
        iObject{ service<i_gradient_manager>().create_gradient(aColor, aDirection) }
    {
    }

    gradient::gradient(const sRGB_color& aColor1, const sRGB_color& aColor2, gradient_direction aDirection) :
        iObject{ service<i_gradient_manager>().create_gradient(aColor1, aColor2, aDirection) }
    {
    }

    gradient::gradient(const abstract_color_stop_list& aColorStops, gradient_direction aDirection) :
        iObject{ service<i_gradient_manager>().create_gradient(aColorStops, aDirection) }
    {
    }

    gradient::gradient(const abstract_color_stop_list& aColorStops, const abstract_alpha_stop_list& aAlphaStops, gradient_direction aDirection) :
        iObject{ service<i_gradient_manager>().create_gradient(aColorStops, aAlphaStops, aDirection) }
    {
    }

    gradient::gradient(const gradient& aOther, const abstract_color_stop_list& aColorStops, const abstract_alpha_stop_list& aAlphaStops) :
        iObject{ service<i_gradient_manager>().create_gradient(aOther, aColorStops, aAlphaStops) }
    {
    }

    gradient::gradient(const neolib::i_vector<sRGB_color>& aColors, gradient_direction aDirection) :
        iObject{ service<i_gradient_manager>().create_gradient(aColors, aDirection) }
    {
    }

    gradient::gradient(const std::initializer_list<sRGB_color>& aColors, gradient_direction aDirection) :
        gradient{ neolib::vector<sRGB_color>{aColors}, aDirection }
    {
    }

    gradient& gradient::operator=(const i_gradient& aOther)
    {
        if (&aOther == this)
            return *this;
        iCopy = true;
        iBoundingBox = aOther.bounding_box();
        aOther.share_object(iObject); 
        return *this;
    }

    void gradient::clone(neolib::i_ref_ptr<i_gradient>& aResult) const
    {
        aResult = neolib::make_ref<gradient>(*this);
    }

    gradient_id gradient::id() const
    {
        return object().id();
    }

    bool gradient::is_singular() const
    {
        return iObject == nullptr;
    }

    gradient::abstract_color_stop_list const& gradient::color_stops() const
    {
        return object().color_stops();
    }

    gradient::abstract_color_stop_list& gradient::color_stops()
    {
        return object().color_stops();
    }

    gradient::abstract_alpha_stop_list const& gradient::alpha_stops() const
    {
        return object().alpha_stops();
    }

    gradient::abstract_alpha_stop_list& gradient::alpha_stops()
    {
        return object().alpha_stops();
    }

    gradient::abstract_color_stop_list::iterator gradient::find_color_stop(scalar aPos, bool aToInsert)
    {
        return object().find_color_stop(aPos, aToInsert);
    }

    gradient::abstract_color_stop_list::iterator gradient::find_color_stop(scalar aPos, scalar aStart, scalar aEnd, bool aToInsert)
    {
        return object().find_color_stop(aPos, aStart, aEnd, aToInsert);
    }

    gradient::abstract_alpha_stop_list::iterator gradient::find_alpha_stop(scalar aPos, bool aToInsert)
    {
        return object().find_alpha_stop(aPos, aToInsert);
    }

    gradient::abstract_alpha_stop_list::iterator gradient::find_alpha_stop(scalar aPos, scalar aStart, scalar aEnd, bool aToInsert)
    {
        return object().find_alpha_stop(aPos, aStart, aEnd, aToInsert);
    }

    gradient::abstract_color_stop_list::iterator gradient::insert_color_stop(scalar aPos)
    {
        return object().insert_color_stop(aPos);
    }

    gradient::abstract_color_stop_list::iterator gradient::insert_color_stop(scalar aPos, scalar aStart, scalar aEnd)
    {
        return object().insert_color_stop(aPos, aStart, aEnd);
    }

    gradient::abstract_alpha_stop_list::iterator gradient::insert_alpha_stop(scalar aPos)
    {
        return object().insert_alpha_stop(aPos);
    }

    gradient::abstract_alpha_stop_list::iterator gradient::insert_alpha_stop(scalar aPos, scalar aStart, scalar aEnd)
    {
        return object().insert_alpha_stop(aPos, aStart, aEnd);
    }

    sRGB_color gradient::at(scalar aPos) const
    {
        return object().at(aPos);
    }

    sRGB_color gradient::at(scalar aPos, scalar aStart, scalar aEnd) const
    {
        return object().at(aPos, aStart, aEnd);
    }

    sRGB_color gradient::color_at(scalar aPos) const
    {
        return object().color_at(aPos);
    }

    sRGB_color gradient::color_at(scalar aPos, scalar aStart, scalar aEnd) const
    {
        return object().color_at(aPos, aStart, aEnd);
    }

    sRGB_color::view_component gradient::alpha_at(scalar aPos) const
    {
        return object().alpha_at(aPos);
    }

    sRGB_color::view_component gradient::alpha_at(scalar aPos, scalar aStart, scalar aEnd) const
    {
        return object().alpha_at(aPos, aStart, aEnd);
    }

    void gradient::reverse()
    {
        object().reverse();
    }

    void gradient::set_alpha(sRGB_color::view_component aAlpha)
    {
        object().set_alpha(aAlpha);
    }

    void gradient::set_combined_alpha(sRGB_color::view_component aAlpha)
    {
        object().set_combined_alpha(aAlpha);
    }

    gradient_direction gradient::direction() const
    {
        return object().direction();
    }

    void gradient::set_direction(gradient_direction aDirection)
    {
        object().set_direction(aDirection);
    }

    gradient_orientation gradient::orientation() const
    {
        return object().orientation();
    }

    void gradient::set_orientation(gradient_orientation aOrientation)
    {
        object().set_orientation(aOrientation);
    }

    gradient_shape gradient::shape() const
    {
        return object().shape();
    }

    void gradient::set_shape(gradient_shape aShape)
    {
        object().set_shape(aShape);
    }

    gradient_size gradient::size() const
    {
        return object().size();
    }

    void gradient::set_size(gradient_size aSize)
    {
        object().set_size(aSize);
    }

    const optional_vec2& gradient::exponents() const
    {
        return object().exponents();
    }

    void gradient::set_exponents(const optional_vec2& aExponents)
    {
        object().set_exponents(aExponents);
    }

    const optional_point& gradient::center() const
    {
        return object().center();
    }

    void gradient::set_center(const optional_point& aCenter)
    {
        object().set_center(aCenter);
    }

    scalar gradient::smoothness() const
    {
        return object().smoothness();
    }

    void gradient::set_smoothness(scalar aSmoothness)
    {
        object().set_smoothness(aSmoothness);
    }

    const optional_rect& gradient::bounding_box() const
    {
        if (iBoundingBox != std::nullopt)
            return iBoundingBox;
        if (!is_singular())
            return object().bounding_box();
        return iBoundingBox;
    }

    void gradient::set_bounding_box(const optional_rect& aBoundingBox)
    {
        iBoundingBox = aBoundingBox;
    }

    const i_gradient_sampler& gradient::colors() const
    {
        return object().colors();
    }

    const i_gradient_filter& gradient::filter() const
    {
        return object().filter();
    }

    void gradient::share_object(i_ref_ptr<i_gradient>& aRef) const
    {
        aRef = iObject;
    }

    const i_gradient& gradient::object() const
    {
        if (is_singular())
            iObject = service<i_gradient_manager>().create_gradient();
        return *iObject;
    }

    i_gradient& gradient::object()
    {
        if (is_singular())
            return const_cast<i_gradient&>(to_const(*this).object());
        if (iCopy)
        {
            iCopy = false;
            iObject = iObject->clone();
        }
        return *iObject;
    }
}
