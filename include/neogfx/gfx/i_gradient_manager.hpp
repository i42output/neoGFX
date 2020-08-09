// i_gradient_manager.hpp
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
#include <neolib/core/jar.hpp>
#include <neogfx/gfx/i_gradient.hpp>

namespace neogfx
{
    class i_gradient_manager : public neolib::i_cookie_consumer
    {
        friend class gradient_object;
        // exceptions
    public:
        struct gradient_not_found : std::logic_error { gradient_not_found() : std::logic_error("neogfx::i_gradient_manager::gradient_not_found") {} };
        // types
    public:
        typedef i_gradient_manager abstract_type;
        // operations
    public:
        virtual void clear_gradients() = 0;
        // helpers
    public:
        neolib::ref_ptr<i_gradient> find_gradient(gradient_id aId) const
        {
            neolib::ref_ptr<i_gradient> result;
            do_find_gradient(aId, result);
            return result;
        }
        neolib::ref_ptr<i_gradient> create_gradient()
        {
            neolib::ref_ptr<i_gradient> result;
            do_create_gradient(result);
            return result;
        }
        neolib::ref_ptr<i_gradient> create_gradient(const i_gradient& aOther)
        {
            neolib::ref_ptr<i_gradient> result;
            do_create_gradient(aOther, result);
            return result;
        }
        neolib::ref_ptr<i_gradient> create_gradient(const i_string& aCssDeclaration)
        {
            neolib::ref_ptr<i_gradient> result;
            do_create_gradient(aCssDeclaration, result);
            return result;
        }
        neolib::ref_ptr<i_gradient> create_gradient(const sRGB_color& aColor)
        {
            neolib::ref_ptr<i_gradient> result;
            do_create_gradient(aColor, result);
            return result;
        }
        neolib::ref_ptr<i_gradient> create_gradient(const sRGB_color& aColor, gradient_direction aDirection)
        {
            neolib::ref_ptr<i_gradient> result;
            do_create_gradient(aColor, aDirection, result);
            return result;
        }
        neolib::ref_ptr<i_gradient> create_gradient(const sRGB_color& aColor1, const sRGB_color& aColor2, gradient_direction aDirection = gradient_direction::Vertical)
        {
            neolib::ref_ptr<i_gradient> result;
            do_create_gradient(aColor1, aColor2, aDirection, result);
            return result;
        }
        neolib::ref_ptr<i_gradient> create_gradient(const i_gradient::color_stop_list& aColorStops, gradient_direction aDirection = gradient_direction::Vertical)
        {
            neolib::ref_ptr<i_gradient> result;
            do_create_gradient(aColorStops, aDirection, result);
            return result;
        }
        neolib::ref_ptr<i_gradient> create_gradient(const i_gradient::color_stop_list& aColorStops, const i_gradient::alpha_stop_list& aAlphaStops, gradient_direction aDirection = gradient_direction::Vertical)
        {
            neolib::ref_ptr<i_gradient> result;
            do_create_gradient(aColorStops, aAlphaStops, aDirection, result);
            return result;
        }
        neolib::ref_ptr<i_gradient> create_gradient(const i_gradient& aOther, const i_gradient::color_stop_list& aColorStops, const i_gradient::alpha_stop_list& aAlphaStops)
        {
            neolib::ref_ptr<i_gradient> result;
            do_create_gradient(aOther, aColorStops, aAlphaStops, result);
            return result;
        }
        neolib::ref_ptr<i_gradient> create_gradient(const neolib::i_vector<sRGB_color>& aColors, gradient_direction aDirection = gradient_direction::Vertical)
        {
            neolib::ref_ptr<i_gradient> result;
            do_create_gradient(aColors, aDirection, result);
            return result;
        }
        neolib::ref_ptr<i_gradient> create_gradient(const std::initializer_list<sRGB_color>& aColors, gradient_direction aDirection = gradient_direction::Vertical)
        {
            return create_gradient(neolib::vector<sRGB_color>{aColors}, aDirection);
        }
        // implementation
    private:
        virtual void do_find_gradient(gradient_id aId, neolib::i_ref_ptr<i_gradient>& aResult) const = 0;
        virtual void do_create_gradient(neolib::i_ref_ptr<i_gradient>& aResult) = 0;
        virtual void do_create_gradient(const i_gradient& aOther, neolib::i_ref_ptr<i_gradient>& aResult) = 0;
        virtual void do_create_gradient(const i_string& aCssDeclaration, neolib::i_ref_ptr<i_gradient>& aResult) = 0;
        virtual void do_create_gradient(const sRGB_color& aColor, neolib::i_ref_ptr<i_gradient>& aResult) = 0;
        virtual void do_create_gradient(const sRGB_color& aColor, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult) = 0;
        virtual void do_create_gradient(const sRGB_color& aColor1, const sRGB_color& aColor2, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult) = 0;
        virtual void do_create_gradient(const i_gradient::color_stop_list& aColorStops, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult) = 0;
        virtual void do_create_gradient(const i_gradient::color_stop_list& aColorStops, const i_gradient::alpha_stop_list& aAlphaStops, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult) = 0;
        virtual void do_create_gradient(const i_gradient& aOther, const i_gradient::color_stop_list& aColorStops, const i_gradient::alpha_stop_list& aAlphaStops, neolib::i_ref_ptr<i_gradient>& aResult) = 0;
        virtual void do_create_gradient(const neolib::i_vector<sRGB_color>& aColors, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult) = 0;
    };
}