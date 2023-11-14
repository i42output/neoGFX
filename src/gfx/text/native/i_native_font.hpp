// i_native_font.hpp
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
#include <neolib/core/reference_counted.hpp>
#include <neogfx/core/geometrical.hpp>
#include <neogfx/core/device_metrics.hpp>
#include <neogfx/gfx/text/font.hpp>
#include "i_native_font_face.hpp"

namespace neogfx
{
    class i_native_font : public neolib::i_reference_counted
    {
        // types
    public:
        typedef i_native_font abstract_type;
        // construction
    public:
        virtual ~i_native_font() = default;
        // operations
    public:
        virtual i_string const& family_name() const = 0;
        virtual bool has_style(font_style aStyle) const = 0;
        virtual uint32_t style_count() const = 0;
        virtual font_style style(uint32_t aStyleIndex) const = 0;
        virtual i_string const& style_name(uint32_t aStyleIndex) const = 0;
        virtual void remove_style(font_style aStyle) = 0;
        virtual void remove_style(uint32_t aStyleIndex) = 0;
        virtual void create_face(font_style aStyle, font::point_size aSize, stroke aOutline, i_device_resolution const& aDevice, i_ref_ptr<i_native_font_face>& aResult) = 0;
        virtual void create_face(i_string const& aStyleName, font::point_size aSize, stroke aOutline, i_device_resolution const& aDevice, i_ref_ptr<i_native_font_face>& aResult) = 0;
        virtual void create_face(font_info const& aFontInfo, i_device_resolution const& aDevice, i_ref_ptr<i_native_font_face>& aResult) = 0;
        // helpers
    public:
        font_style min_style() const
        {
            auto minStyle = style(0u);
            for (auto si = 1u; si < style_count(); ++si)
                minStyle = std::min(minStyle, style(si));
            return minStyle;
        }
        font_weight min_weight() const
        {
            auto minWeight = font_info::weight_from_style_name(style_name(0));
            for (auto si = 1u; si < style_count(); ++si)
                minWeight = std::min(minWeight, font_info::weight_from_style_name(style_name(si)));
            return minWeight;
        }
        ref_ptr<i_native_font_face> create_face(font_style aStyle, font::point_size aSize, stroke aOutline, i_device_resolution const& aDevice)
        {
            ref_ptr<i_native_font_face> result;
            create_face(aStyle, aSize, aOutline, aDevice, result);
            return result;
        }
        ref_ptr<i_native_font_face> create_face(std::string const& aStyleName, font::point_size aSize, stroke aOutline, i_device_resolution const& aDevice)
        {
            ref_ptr<i_native_font_face> result;
            create_face(string{ aStyleName }, aSize, aOutline, aDevice, result);
            return result;
        }
        ref_ptr<i_native_font_face> create_face(font_info const& aFontInfo, i_device_resolution const& aDevice)
        {
            ref_ptr<i_native_font_face> result;
            create_face(aFontInfo, aDevice, result);
            return result;
        }
    };
}