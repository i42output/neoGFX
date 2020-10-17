// style.cpp
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

#include <neogfx/neogfx.hpp>
#include <neolib/file/json.hpp>
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/style.hpp>

namespace neogfx
{
    style::style(std::string const& aName) :
        iName{ aName },
        iPadding{ 2.0, 2.0, 4.0, 4.0, 4.0 },
        iSpacing{ 2.0, 2.0 }
    {
        iPalette.Changed([this]() { handle_change(style_aspect::Color); });
    }

    style::style(std::string const& aName, const i_style& aOther) :
        iName{ aName },
        iPadding{ aOther.all_padding() },
        iSpacing{ aOther.spacing() },
        iPalette{ aOther.palette() }
    {
        iFontInfo[font_role::Caption] = aOther.font_info(font_role::Caption);
        iFontInfo[font_role::Menu] = aOther.font_info(font_role::Menu);
        iFontInfo[font_role::Toolbar] = aOther.font_info(font_role::Toolbar);
        iFontInfo[font_role::StatusBar] = aOther.font_info(font_role::StatusBar);
        iFontInfo[font_role::Widget] = aOther.font_info(font_role::Widget);
        iPalette.Changed([this]() { handle_change(style_aspect::Color); });
    }

    style::style(const i_style& aOther) :
        style{ aOther.name() , aOther }
    {
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
            iPadding = aOther.all_padding();
            iSpacing = aOther.spacing();
            iPalette = aOther.palette();
            iFontInfo[font_role::Caption] = aOther.font_info(font_role::Caption);
            iFontInfo[font_role::Menu] = aOther.font_info(font_role::Menu);
            iFontInfo[font_role::Toolbar] = aOther.font_info(font_role::Toolbar);
            iFontInfo[font_role::StatusBar] = aOther.font_info(font_role::StatusBar);
            iFontInfo[font_role::Widget] = aOther.font_info(font_role::Widget);
            handle_change(style_aspect::Font);
        }
        return *this;
    }

    bool style::operator==(const i_style& aOther) const
    {
        return iName == aOther.name() && 
            iPadding == aOther.all_padding() &&
            iSpacing == aOther.spacing() &&
            iPalette == aOther.palette() &&
            iFontInfo[font_role::Caption] == aOther.font_info(font_role::Caption) &&
            iFontInfo[font_role::Menu] == aOther.font_info(font_role::Menu) &&
            iFontInfo[font_role::Toolbar] == aOther.font_info(font_role::Toolbar) &&
            iFontInfo[font_role::StatusBar] == aOther.font_info(font_role::StatusBar) &&
            iFontInfo[font_role::Widget] == aOther.font_info(font_role::Widget);
    }

    bool style::operator!=(const i_style& aOther) const
    {
        return !(*this == aOther);
    }

    std::string const& style::name() const
    {
        return iName;
    }

    const style::padding_list& style::all_padding() const
    {
        return iPadding;
    }

    const neogfx::padding& style::padding(padding_role aPaddingRole) const
    {
        return iPadding[static_cast<std::size_t>(aPaddingRole)];
    }

    void style::set_padding(padding_role aPaddingRole, const neogfx::padding& aPadding)
    {
        if (iPadding[static_cast<std::size_t>(aPaddingRole)] != aPadding)
        {
            iPadding[static_cast<std::size_t>(aPaddingRole)] = aPadding;
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
            handle_change(style_aspect::Color);
        }
    }

    const font_info& style::font_info(font_role aRole) const
    {
        if (iFontInfo[aRole] == std::nullopt)
        {
            switch (aRole)
            {
            case font_role::Caption:
                iFontInfo[aRole] = service<i_font_manager>().default_system_font_info(system_font_role::Caption);
                break;
            case font_role::Menu:
                iFontInfo[aRole] = service<i_font_manager>().default_system_font_info(system_font_role::Menu);
                break;
            case font_role::Toolbar:
                iFontInfo[aRole] = service<i_font_manager>().default_system_font_info(system_font_role::Toolbar);
                break;
            case font_role::StatusBar:
                iFontInfo[aRole] = service<i_font_manager>().default_system_font_info(system_font_role::StatusBar);
                break;
            case font_role::Widget:
                iFontInfo[aRole] = service<i_font_manager>().default_system_font_info(system_font_role::Widget);
                break;
            }
        }
        return *iFontInfo[aRole];
    }

    void style::set_font_info(font_role aRole, const neogfx::font_info& aFontInfo)
    {
        if (iFontInfo[aRole] != aFontInfo)
        {
            iFontInfo[aRole] = aFontInfo;
            iFont[aRole].reset();
            handle_change(style_aspect::Font);
        }
    }

    const font& style::font(font_role aRole) const
    {
        if (iFont[aRole] == std::nullopt)
            iFont[aRole].emplace(font_info(aRole));
        return *iFont[aRole];
    }

    void style::handle_change(style_aspect aAspect)
    {
        Changed.trigger(aAspect);
        if (&service<i_app>().current_style() == this)
            service<i_app>().current_style_changed().async_trigger(aAspect);
    }
}