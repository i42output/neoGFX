// i_native_clipboard.hpp
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
#include <neogfx/gfx/image.hpp>

namespace neogfx
{
    struct clipboard_error : std::runtime_error { using runtime_error::runtime_error; };

    class i_native_clipboard
    {
    public:
        struct failed_to_open_clipboard : clipboard_error { failed_to_open_clipboard() : clipboard_error{"neogfx::i_native_clipboard::failed_to_open_clipboard"} {} };
        struct failed_to_get_clipboard_data : clipboard_error { failed_to_get_clipboard_data() : clipboard_error{"neogfx::i_native_clipboard::failed_to_get_clipboard_data"} {} };
        struct failed_to_set_clipboard_data : clipboard_error { failed_to_set_clipboard_data() : clipboard_error{"neogfx::i_native_clipboard::failed_to_set_clipboard_data"} {} };
        struct unsupported_clipboard_operation : clipboard_error { unsupported_clipboard_operation() : clipboard_error{ "neogfx::i_native_clipboard::unsupported_clipboard_operation" } {} };
    public:
        virtual bool has_text() const = 0;
        virtual std::string text() const = 0;
        virtual void set_text(std::string const& aText) = 0;
        virtual bool has_image() const = 0;
        virtual neogfx::image image() const = 0;
        virtual void set_image(const neogfx::image& aImage) = 0;
    };
}