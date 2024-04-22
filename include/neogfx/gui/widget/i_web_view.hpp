// i_web_view.hpp
/*
  neogfx C++ App/Game Engine
  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.
  
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

#include <neogfx/core/event.hpp>
#include <neogfx/gui/widget/i_widget.hpp>

namespace neogfx
{
    class i_web_view : public i_widget
    {
        // events
    public:
        // types
    public:
        using abstract_type = i_web_view;
        // operations
    public:
    };

    class i_web_view_factory : public i_reference_counted
    {
        // types
    public:
        using abstract_type = i_web_view_factory;
        // interface
    public:
        virtual void create_web_view(i_widget& aParent, i_ref_ptr<i_web_view>& aWebView) = 0;
        virtual void create_web_view(i_layout& aLayout, i_ref_ptr<i_web_view>& aWebView) = 0;
        // helpers
    public:
        ref_ptr<i_web_view> create_web_view(i_widget& aParent)
        {
            ref_ptr<i_web_view> newWebView;
            create_web_view(aParent, newWebView);
            return newWebView;
        }
        ref_ptr<i_web_view> create_web_view(i_layout& aLayout)
        {
            ref_ptr<i_web_view> newWebView;
            create_web_view(aLayout, newWebView);
            return newWebView;
        }
        // discovery
    public:
        static uuid const& iid() { static uuid const sIid{ 0x81c2c56, 0x6a27, 0x4026, 0xa8ee, { 0x13, 0x3b, 0x3f, 0xcc, 0xba, 0xb4 } }; return sIid; }
    };
}
