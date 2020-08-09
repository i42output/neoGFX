// gradient_manager.hpp
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
#include <neogfx/gfx/i_gradient_manager.hpp>

namespace neogfx
{
    class gradient_manager : public i_gradient_manager
    {
        friend class gradient_object;
        // types
    protected:
        typedef ref_ptr<i_gradient> gradient_pointer;
        typedef std::pair<gradient_pointer, uint32_t> gradient_list_entry;
        typedef neolib::jar<gradient_list_entry> gradient_list;
    private:
        friend neolib::cookie item_cookie(const gradient_list_entry&);
        // operations
    public:
        void clear_gradients() override;
        // implementation
    protected:
        void add_ref(gradient_id aId) override;
        void release(gradient_id aId) override;
        long use_count(gradient_id aId) const override;
    protected:
        gradient_id allocate_gradient_id();
        const gradient_list& gradients() const;
        gradient_list& gradients();
        neolib::ref_ptr<i_gradient> add_gradient(neolib::ref_ptr<i_gradient> aGradient);
    private:
        void do_find_gradient(gradient_id aId, neolib::i_ref_ptr<i_gradient>& aResult) const override;
        void do_create_gradient(neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(const i_gradient& aOther, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(const i_string& aCssDeclaration, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(const sRGB_color& aColor, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(const sRGB_color& aColor, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(const sRGB_color& aColor1, const sRGB_color& aColor2, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(const i_gradient::color_stop_list& aColorStops, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(const i_gradient::color_stop_list& aColorStops, const i_gradient::alpha_stop_list& aAlphaStops, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(const i_gradient& aOther, const i_gradient::color_stop_list& aColorStops, const i_gradient::alpha_stop_list& aAlphaStops, neolib::i_ref_ptr<i_gradient>& aResult) override;
        void do_create_gradient(const neolib::i_vector<sRGB_color>& aColors, gradient_direction aDirection, neolib::i_ref_ptr<i_gradient>& aResult) override;
    private:
        void cleanup();
    private:
        gradient_list iGradients;
    };
}