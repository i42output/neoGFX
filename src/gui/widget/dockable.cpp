// dockable.cpp
/*
  neogfx C++ GUI Library
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
#include <neogfx/app/i_app.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gui/widget/dockable.hpp>

namespace neogfx
{
    dockable_title_bar::dockable_title_bar(i_dockable& aDockable) :
        widget{ aDockable.as_widget().layout() },
        iDockable{ aDockable },
        iUpdater{service<async_task>(), [this](neolib::callback_timer& aTimer)
        {
            aTimer.again();
            update();
        }, 100},
        iLayout{ *this },
        iTitle{ iLayout, aDockable.title() },
        iSpacer{ iLayout },
        iPinButton{ iLayout, push_button_style::TitleBar },
        iUnpinButton{ iLayout, push_button_style::TitleBar },
        iCloseButton{ iLayout, push_button_style::TitleBar }
    {
        set_margins(neogfx::margins{});
        layout().set_margins(neogfx::margins{});

        iPinButton.set_size_policy(neogfx::size_policy{ size_constraint::Minimum, size_constraint::Minimum });
        iUnpinButton.set_size_policy(neogfx::size_policy{ size_constraint::Minimum, size_constraint::Minimum });
        iCloseButton.set_size_policy(neogfx::size_policy{ size_constraint::Minimum, size_constraint::Minimum });

        iPinButton.hide();
        iUnpinButton.hide();

        iSink += service<i_surface_manager>().dpi_changed([this](i_surface&)
        {
            update_textures();
            managing_layout().layout_items(true);
            update(true);
        });
        iSink += service<i_app>().current_style_changed([this](style_aspect aAspect)
        {
            if ((aAspect & style_aspect::Color) == style_aspect::Color) 
                update_textures();
        });

        update_textures();
    }

    size dockable_title_bar::minimum_size(const optional_size& aAvailableSpace) const
    {
        if (has_minimum_size())
            return widget::minimum_size(aAvailableSpace);
        else
            return widget::minimum_size(aAvailableSpace) + size{ 2.0_cm, 0.0 };
    }

    bool dockable_title_bar::transparent_background() const
    {
        return false;
    }

    color dockable_title_bar::background_color() const
    {
        if (has_background_color())
            return widget::background_color();
        else if (!iDockable.as_widget().has_focus() && !iDockable.as_widget().child_has_focus())
            return widget::container_background_color();
        else
            return service<i_app>().current_style().palette().color(color_role::Selection);
    }

    neogfx::focus_policy dockable_title_bar::focus_policy() const
    {
        return neogfx::focus_policy::ClickFocus;
    }

    void dockable_title_bar::update_textures()
    {
        auto ink = service<i_app>().current_style().palette().color(color_role::Text);
        auto paper = background_color();
        const char* sCloseTexturePattern
        {
            "[8,8]"
            "{0,paper}"
            "{1,ink}"
            "{2,ink_with_alpha}"

            "12000021"
            "21200212"
            "02122120"
            "00211200"
            "00211200"
            "02122120"
            "21200212"
            "12000021"
        };
        const char* sCloseHighDpiTexturePattern
        {
            "[16,16]"
            "{0,paper}"
            "{1,ink}"
            "{2,ink_with_alpha}"

            "1120000000000211"
            "1112000000002111"
            "2111200000021112"
            "0211120000211120"
            "0021112002111200"
            "0002111221112000"
            "0000211111120000"
            "0000021111200000"
            "0000021111200000"
            "0000211111120000"
            "0002111221112000"
            "0021112002111200"
            "0211120000211120"
            "2111200000021112"
            "1112000000002111"
            "1120000000000211"

        };
        if (iCloseTexture == std::nullopt || iCloseTexture->first != ink)
        {
            iCloseTexture.emplace(
                ink,
                !high_dpi() ?
                    neogfx::image{
                        "neogfx::dockable_title_bar::iCloseTexture::" + ink.to_string(),
                        sCloseTexturePattern, { { "paper", color{} },{ "ink", ink },{ "ink_with_alpha", ink.with_alpha(0x80) } } } :
                    neogfx::image{
                        "neogfx::dockable_title_bar::iCloseHighDpiTexture::" + ink.to_string(),
                        sCloseHighDpiTexturePattern, { { "paper", color{} }, { "ink", ink }, { "ink_with_alpha", ink.with_alpha(0x80) } }, 2.0 });
        }
        iCloseButton.set_image(iCloseTexture->second);
    }

    dockable::dockable(const std::shared_ptr<i_widget>& aDockableWidget, const std::string& aTitle, dock_area aAcceptableDocks) :
        iTitle{ aTitle }, 
        iAcceptableDocks {aAcceptableDocks }, 
        iLayout{ *this },
        iTitleBar{ *this },
        iDockedWidget{ aDockableWidget },
        iDock{ nullptr }
    {
        set_margins(neogfx::margins{});
        set_size_policy(size_constraint::Expanding);
        layout().set_margins(neogfx::margins{});
        layout().set_size_policy(size_constraint::Expanding);
        layout().add(*aDockableWidget);
        docked_widget().set_size_policy(size_constraint::Expanding);
    }

    const neolib::string& dockable::title() const
    {
        return iTitle;
    }

    bool dockable::can_dock(const i_dock& aDock) const
    {
        return (iAcceptableDocks & aDock.area()) != dock_area::None;
    }

    bool dockable::is_docked() const
    {
        return iDock != nullptr;
    }

    void dockable::dock(i_dock& aDock)
    {
        if (is_docked())
            undock();
        aDock.add(ref_ptr{ static_cast<i_dockable&>(*this) });
        iDock = &aDock;
        Docked.trigger(aDock);
    }

    void dockable::undock()
    {
        if (!is_docked())
            throw not_docked();
        auto& dock = *iDock;
        dock.remove(ref_ptr{ static_cast<i_dockable&>(*this) });
        iDock = nullptr;
        Undocked.trigger(dock);
    }

    const i_widget& dockable::as_widget() const
    {
        return *this;
    }

    i_widget& dockable::as_widget()
    {
        return *this;
    }

    const i_widget& dockable::docked_widget() const
    {
        return *iDockedWidget;
    }

    i_widget& dockable::docked_widget()
    {
        return *iDockedWidget;
    }

    color dockable::frame_color() const
    {
        return has_frame_color() ? framed_widget::frame_color() : framed_widget::frame_color().shade(0x08);
    }
}