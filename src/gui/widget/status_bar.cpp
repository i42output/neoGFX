// status_bar.cpp
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

#include <neogfx/app/i_app.hpp>
#include <neogfx/app/i_help.hpp>
#include <neogfx/hid/i_surface_manager.hpp>
#include <neogfx/gui/widget/status_bar.hpp>

namespace neogfx
{
    status_bar::separator::separator() : 
        widget{}
    {
    }

    neogfx::size_policy status_bar::separator::size_policy() const
    {
        if (widget::has_size_policy())
            return widget::size_policy();
        return neogfx::size_policy{ size_constraint::Minimum, size_constraint::Expanding };
    }

    size status_bar::separator::minimum_size(const optional_size& aAvailableSpace) const
    {
        if (widget::has_minimum_size())
            return widget::minimum_size(aAvailableSpace);
        return units_converter(*this).from_device_units(size{ 2.0, 2.0 }) + padding().size();
    }

    void status_bar::separator::paint(i_graphics_context& aGc) const
    {
        scoped_units su{ *this, units::Pixels };
        rect line = client_rect(false);
        line.deflate(size{ 0.0_dip, 2.0_dip });
        line.cx = 1.0;
        color ink = (has_base_color() ? base_color() : service<i_app>().current_style().palette().color(color_role::Base));
        aGc.fill_rect(line, ink.darker(0x40).with_alpha(0.5));
        ++line.x;
        aGc.fill_rect(line, ink.lighter(0x40).with_alpha(0.5));
    }

    status_bar::keyboard_lock_status::keyboard_lock_status(i_layout& aLayout) :
        widget{ aLayout },
        iLayout{ *this }
    {
        iLayout.add(std::make_shared<separator>());
        auto insertLock = std::make_shared<label>();
        insertLock->text_widget().set_size_hint(size_hint{ "Insert" });
        insertLock->text_widget().set_font_role(font_role::StatusBar);
        iLayout.add(insertLock);
        iLayout.add(std::make_shared<separator>());
        auto capsLock = std::make_shared<label>();
        capsLock->text_widget().set_size_hint(size_hint{ "CAP" });
        capsLock->text_widget().set_font_role(font_role::StatusBar);
        iLayout.add(capsLock);
        iLayout.add(std::make_shared<separator>());
        auto numLock = std::make_shared<label>();
        numLock->text_widget().set_size_hint(size_hint{ "NUM" });
        numLock->text_widget().set_font_role(font_role::StatusBar);
        iLayout.add(numLock);
        iLayout.add(std::make_shared<separator>());
        auto scrlLock = std::make_shared<label>();
        scrlLock->text_widget().set_size_hint(size_hint{ "SCRL" });
        scrlLock->text_widget().set_font_role(font_role::StatusBar);
        iLayout.add(scrlLock);
        iUpdater = std::make_unique<neolib::callback_timer>(service<i_async_task>(), root().surface(), [insertLock, capsLock, numLock, scrlLock](neolib::callback_timer& aTimer)
        {
            aTimer.again();
            auto const& keyboard = service<i_keyboard>();
            insertLock->set_text((keyboard.locks() & keyboard_locks::InsertLock) == keyboard_locks::InsertLock ?
                "Insert" : std::string{});
            capsLock->set_text((keyboard.locks() & keyboard_locks::CapsLock) == keyboard_locks::CapsLock ?
                "CAP" : std::string{});
            numLock->set_text((keyboard.locks() & keyboard_locks::NumLock) == keyboard_locks::NumLock ?
                "NUM" : std::string{});
            scrlLock->set_text((keyboard.locks() & keyboard_locks::ScrollLock) == keyboard_locks::ScrollLock ?
                "SCRL" : std::string{});
        }, 100);
    }

    neogfx::size_policy status_bar::keyboard_lock_status::size_policy() const
    {
        if (has_size_policy())
            return widget::size_policy();
        return neogfx::size_policy{ size_constraint::Minimum, size_constraint::Expanding };
    }

    status_bar::size_grip_widget::size_grip_widget(i_layout& aLayout) :
        image_widget{ aLayout }
    {
        set_ignore_mouse_events(false);
        set_placement(cardinal::SouthEast);
    }

    neogfx::size_policy status_bar::size_grip_widget::size_policy() const
    {
        return neogfx::size_policy{ size_constraint::Minimum, size_constraint::Expanding };
    }

    widget_part status_bar::size_grip_widget::hit_test(const point&) const
    {
        return widget_part{ root().as_widget(), widget_part::BorderBottomRight };
    }

    bool status_bar::size_grip_widget::ignore_non_client_mouse_events() const
    {
        return false;
    }

    status_bar::status_bar(i_standard_layout_container& aContainer, style aStyle) :
        widget{ aContainer.status_bar_layout() },
        iStyle{ aStyle },
        iLayout{ *this },
        iNormalLayout{ iLayout },
        iMessageLayout{ iNormalLayout },
        iMessageWidget{ iMessageLayout },
        iIdleLayout{ iNormalLayout },
        iIdleWidget{ iIdleLayout },
        iNormalWidgetContainer{ iLayout },
        iNormalWidgetLayout{ iNormalWidgetContainer },
        iPermanentWidgetLayout{ iLayout },
        iKeyboardLockStatus{ iLayout },
        iSizeGrip{ iLayout }
    {
        aContainer.set_status_bar(*this);
        init();
    }

    bool status_bar::have_message() const
    {
        return iMessage != std::nullopt || service<i_app>().help().help_active();
    }

    std::string status_bar::message() const
    {
        if (service<i_app>().help().help_active())
            return service<i_app>().help().active_help().help_text();
        else if (iMessage != std::nullopt)
            return *iMessage;
        throw no_message();
    }

    void status_bar::set_message(const std::string& aMessage)
    {
        iMessage = aMessage;
        update_widgets();
    }

    void status_bar::clear_message()
    {
        iMessage = std::nullopt;
        update_widgets();
    }

    void status_bar::add_normal_widget(i_widget& aWidget)
    {
        iNormalWidgetLayout.add(aWidget);
        aWidget.set_font_role(font_role::StatusBar);
    }

    void status_bar::add_normal_widget_at(widget_index aPosition, i_widget& aWidget)
    {
        iNormalWidgetLayout.add_at(aPosition, aWidget);
        aWidget.set_font_role(font_role::StatusBar);
    }

    void status_bar::add_normal_widget(std::shared_ptr<i_widget> aWidget)
    {
        iNormalWidgetLayout.add(aWidget);
        aWidget->set_font_role(font_role::StatusBar);
    }

    void status_bar::add_normal_widget_at(widget_index aPosition, std::shared_ptr<i_widget> aWidget)
    {
        iNormalWidgetLayout.add_at(aPosition, aWidget);
        aWidget->set_font_role(font_role::StatusBar);
    }

    void status_bar::add_permanent_widget(i_widget& aWidget)
    {
        iPermanentWidgetLayout.add(aWidget);
        aWidget.set_font_role(font_role::StatusBar);
    }

    void status_bar::add_permanent_widget_at(widget_index aPosition, i_widget& aWidget)
    {
        iPermanentWidgetLayout.add_at(aPosition, aWidget);
        aWidget.set_font_role(font_role::StatusBar);
    }

    void status_bar::add_permanent_widget(std::shared_ptr<i_widget> aWidget)
    {
        iPermanentWidgetLayout.add(aWidget);
        aWidget->set_font_role(font_role::StatusBar);
    }

    void status_bar::add_permanent_widget_at(widget_index aPosition, std::shared_ptr<i_widget> aWidget)
    {
        iPermanentWidgetLayout.add_at(aPosition, aWidget);
        aWidget->set_font_role(font_role::StatusBar);
    }

    i_layout& status_bar::normal_layout()
    {
        return iNormalWidgetLayout;
    }

    i_layout& status_bar::permanent_layout()
    {
        return iPermanentWidgetLayout;
    }

    label& status_bar::message_widget()
    {
        return iMessageWidget;
    }

    label& status_bar::idle_widget()
    {
        return iIdleWidget;
    }

    neogfx::size_policy status_bar::size_policy() const
    {
        if (has_size_policy())
            return widget::size_policy();
        return neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum };
    }

    widget_part status_bar::hit_test(const point& aPosition) const
    {
        point const gripOrigin = client_rect().bottom_right() - (iSizeGrip.origin() - origin());
        rect const gripRect = { gripOrigin, size{ client_rect().bottom_right() - gripOrigin } };
        if (gripRect.contains(aPosition))
            return widget_part{ root().as_widget(), widget_part::BorderBottomRight };
        return widget::hit_test(aPosition);
    }

    bool status_bar::is_managing_layout() const
    {
        return true;
    }

    const i_widget& status_bar::as_widget() const
    {
        return *this;
    }

    i_widget& status_bar::as_widget()
    {
        return *this;
    }

    const i_widget& status_bar::size_grip() const
    {
        return iSizeGrip;
    }

    void status_bar::init()
    {
        set_padding(neogfx::padding{});
        iLayout.set_padding(neogfx::padding{});
        iLayout.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
        iNormalLayout.set_padding(neogfx::padding{});
        iNormalLayout.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
        iMessageLayout.set_padding(neogfx::padding{});
        iMessageLayout.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
        iMessageWidget.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
        iMessageWidget.set_font_role(neogfx::font_role::StatusBar);
        iIdleLayout.set_padding(neogfx::padding{});
        iIdleLayout.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
        iIdleWidget.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
        iNormalWidgetContainer.set_padding(neogfx::padding{});
        iNormalWidgetLayout.set_padding(neogfx::padding{});
        iPermanentWidgetLayout.set_padding(neogfx::padding{});
        auto update_size_grip = [this](style_aspect)
        {
            auto ink1 = (has_base_color() ? base_color() : service<i_app>().current_style().palette().color(color_role::Base));
            ink1 = ink1.shaded(0x40);
            auto ink2 = ink1.darker(0x30);
            if (iSizeGripTexture == std::nullopt || iSizeGripTexture->first != ink1)
            {
                const char* sSizeGripTextureImagePattern
                {
                    "[13,13]"
                    "{0,paper}"
                    "{1,ink1}"
                    "{2,ink2}"

                    "0000000000000"
                    "0000000000210"
                    "0000000000110"
                    "0000000000000"
                    "0000000210210"
                    "0000000110110"
                    "0000000000000"
                    "0000210210210"
                    "0000110110110"
                    "0000000000000"
                    "0210210210210"
                    "0110110110110"
                    "0000000000000"
                };
                const char* sSizeGripHighDpiTextureImagePattern
                {
                    "[26,26]"
                    "{0,paper}"
                    "{1,ink1}"
                    "{2,ink2}"

                    "00000000000000000000000000"
                    "00000000000000000000000000"
                    "00000000000000000000221100"
                    "00000000000000000000221100"
                    "00000000000000000000111100"
                    "00000000000000000000111100"
                    "00000000000000000000000000"
                    "00000000000000000000000000"
                    "00000000000000221100221100"
                    "00000000000000221100221100"
                    "00000000000000111100111100"
                    "00000000000000111100111100"
                    "00000000000000000000000000"
                    "00000000000000000000000000"
                    "00000000221100221100221100"
                    "00000000221100221100221100"
                    "00000000111100111100111100"
                    "00000000111100111100111100"
                    "00000000000000000000000000"
                    "00000000000000000000000000"
                    "00221100221100221100221100"
                    "00221100221100221100221100"
                    "00111100111100111100111100"
                    "00111100111100111100111100"
                    "00000000000000000000000000"
                    "00000000000000000000000000"
                };
                iSizeGripTexture.emplace(ink1, !high_dpi() ?
                    neogfx::image{ "neogfx::status_bar::iSizeGripTexture::" + ink1.to_string(), sSizeGripTextureImagePattern, { { "paper", color{} }, { "ink1", ink1 }, { "ink2", ink2 } } } :
                    neogfx::image{ "neogfx::status_bar::iSizeGripHighDpiTexture::" + ink1.to_string(), sSizeGripHighDpiTextureImagePattern, { { "paper", color{} },{ "ink1", ink1 },{ "ink2", ink2 } }, 2.0 });
            }
            iSizeGrip.set_image(iSizeGripTexture->second);
        };
        iSink += service<i_surface_manager>().dpi_changed([update_size_grip](i_surface&) { update_size_grip(style_aspect::Geometry); });
        iSink += service<i_app>().current_style_changed(update_size_grip);
        update_size_grip(style_aspect::Color);
        iSink += service<i_app>().help().help_activated([this](const i_help_source&) { update_widgets();    });
        iSink += service<i_app>().help().help_deactivated([this](const i_help_source&) { update_widgets(); });
        update_widgets();
    }

    void status_bar::update_widgets()
    {
        bool showMessage = (iStyle & style::DisplayMessage) == style::DisplayMessage && have_message();
        iMessageWidget.set_text(have_message() ? message() : std::string{});
        iMessageWidget.show(showMessage);
        iIdleWidget.show(!showMessage);
        iNormalWidgetContainer.show(!showMessage);
        iKeyboardLockStatus.show((iStyle & style::DisplayKeyboardLocks) == style::DisplayKeyboardLocks);
        iSizeGrip.show((iStyle & style::DisplaySizeGrip) == style::DisplaySizeGrip);
    }
}