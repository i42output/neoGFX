// style.cpp
/*
  neogfx C++ GUI Library
  Copyright (c) 2015 Leigh Johnston.  All Rights Reserved.
  
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
#include <neolib/json.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/style.hpp>
#include <neogfx/hid/i_surface_manager.hpp>

namespace neogfx
{
    style::style(const std::string& aName) :
        iName(aName),
        iMargins(2.0),
        iSpacing(2.0, 2.0),
        iFontInfo(service<i_font_manager>().default_system_font_info())
    {
        iPalette.Changed([this]() { handle_change(style_aspect::Colour); });
    }

    style::style(const std::string& aName, const i_style& aOther) :
        iName(aName),
        iMargins(aOther.margins()),
        iSpacing(aOther.spacing()),
        iPalette(aOther.palette()),
        iFontInfo(aOther.font_info())
    {
        iPalette.Changed([this]() { handle_change(style_aspect::Colour); });
    }

    style::style(const i_style& aOther) :
        iName(aOther.name()),
        iMargins(aOther.margins()),
        iSpacing(aOther.spacing()),
        iPalette(aOther.palette()),
        iFontInfo(aOther.font_info())
    {
        iPalette.Changed([this]() { handle_change(style_aspect::Colour); });
    }

    style::style(const style& aOther) :
        style(static_cast<const i_style&>(aOther))
    {
    }

    style::~style()
    {
    }

    style& style::operator=(const i_style& aOther)
    {
        if (*this != aOther)
        {
            iName = aOther.name();
            iMargins = aOther.margins();
            iSpacing = aOther.spacing();
            iPalette = aOther.palette();
            iFontInfo = aOther.font_info();
            handle_change(style_aspect::Font);
        }
        return *this;
    }

    bool style::operator==(const i_style& aOther) const
    {
        return iName == aOther.name() && 
            iMargins == aOther.margins() &&
            iSpacing == aOther.spacing() &&
            iPalette == aOther.palette() &&
            iFontInfo == aOther.font_info();
    }

    bool style::operator!=(const i_style& aOther) const
    {
        return !(*this == aOther);
    }

    const std::string& style::name() const
    {
        return iName;
    }

    const neogfx::margins& style::margins() const
    {
        return iMargins;
    }

    void style::set_margins(const neogfx::margins& aMargins)
    {
        if (iMargins != aMargins)
        {
            iMargins = aMargins;
            handle_change(style_aspect::Geometry);
        }
    }

    const size& style::spacing() const
    {
        return iSpacing;
    }

    void style::set_spacing(const size& aSpacing)
    {
        if (iSpacing != aSpacing)
        {
            iSpacing = aSpacing;
            handle_change(style_aspect::Geometry);
        }
    }

    const i_palette& style::palette() const
    {
        return iPalette;
    }

    i_palette& style::palette()
    {
        return iPalette;
    }

    void style::set_palette(const i_palette& aPalette)
    {
        if (iPalette != aPalette)
        {
            iPalette = aPalette;
            handle_change(style_aspect::Colour);
        }
    }

    const font_info& style::font_info() const
    {
        return iFontInfo;
    }

    void style::set_font_info(const neogfx::font_info& aFontInfo)
    {
        if (iFontInfo != aFontInfo)
        {
            iFontInfo = aFontInfo;
            iFont.reset();
            handle_change(style_aspect::Font);
        }
    }

    const font& style::font() const
    {
        if (iFont == std::nullopt)
        {
            iFont = neogfx::font(iFontInfo);
        }
        return *iFont;
    }

    void style::handle_change(style_aspect aAspect)
    {
        Changed.trigger(aAspect);
        if (&service<i_app>().current_style() == this)
        {
            service<i_app>().current_style_changed().trigger(aAspect);
            service<i_surface_manager>().layout_surfaces();
        }
    }
}