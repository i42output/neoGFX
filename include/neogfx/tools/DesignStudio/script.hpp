// script.hpp
/*
  neoGFX Design Studio
  Copyright(C) 2021 Leigh Johnston
  
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

#include <neogfx/tools/DesignStudio/DesignStudio.hpp>
#include <neogfx/tools/DesignStudio/i_script.hpp>

namespace neogfx::DesignStudio
{
    class script : public i_script
    {
        typedef script self_type;
        typedef i_script base_type;
    public:
        typedef base_type abstract_type;
    public:
        i_string const& text() const override
        {
            return iText;
        }
        void set_text(i_string const& aText) override
        {
            iText = aText;
        }
    private:
        string iText;
    };
}