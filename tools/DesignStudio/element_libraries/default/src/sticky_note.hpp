// sticky_note.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2020 Leigh Johnston
  
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
#include <random>
#include <neogfx/gui/widget/widget.hpp>

namespace neogfx::DesignStudio
{
    class sticky_note : public widget<>
    {
        typedef widget<> base_type;
    public:
        sticky_note()
        {
            thread_local std::random_device tEntropy;
            thread_local std::mt19937 tGenerator(tEntropy());
            thread_local std::uniform_real_distribution<> tDistribution(0.0, 360.0);
            set_background_color(color::from_hsl(tDistribution(tGenerator), 1.0, 0.9));
            set_minimum_size(size{ 128.0_dip, 128.0_dip });
        }
    protected:
        void paint(i_graphics_context& aGc) const
        {
            base_type::paint(aGc);
            auto cr = client_rect(true);
            cr.cy = 8.0_dip;
            aGc.fill_rect(cr, color::Black.with_alpha(0.125));
        }
    };
}
