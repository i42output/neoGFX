// widget.cpp
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
#include <neogfx/gui/widget/widget.ipp>
#include <neogfx/gui/widget/i_button.hpp>
#include <neogfx/gui/widget/i_push_button.hpp>
#include <neogfx/gui/widget/i_image_widget.hpp>
#include <neogfx/gui/widget/i_text_widget.hpp>
#include <neogfx/gui/widget/i_title_bar.hpp>
#include <neogfx/gui/widget/i_status_bar.hpp>
#include <neogfx/gui/window/i_window.hpp>
#include <neogfx/gui/widget/i_terminal.hpp>

namespace neogfx
{
    template class widget<>;
    template class widget<i_button>;
    template class widget<i_push_button>;
    template class widget<i_radio_button>;
    template class widget<i_image_widget>;
    template class widget<i_text_widget>;
    template class widget<i_title_bar>;
    template class widget<i_status_bar>;
    template class widget<i_window>;
    template class widget<i_terminal>;
}
