// linux_skin.cpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2026 Leigh Johnston.  All Rights Reserved.

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

#include <neogfx/gui/widget/default_skin.hpp>
#include "native_skin.hpp"

namespace neogfx
{
    namespace native::linux
    {
        namespace
        {
            // n.b. a Linux desktop has no theming service for a toolkit that draws its own widgets (GTK's and
            // Qt's themes are their own), so the native skin is the default skin under the native skin's name
            class skin : public default_skin
            {
            public:
                std::string const& name() const override
                {
                    static std::string const sName = "Native";
                    return sName;
                }
            };
        }
    }

    namespace native
    {
        i_skin& native_skin()
        {
            static native::linux::skin sNativeSkin;
            return sNativeSkin;
        }
    }
}
