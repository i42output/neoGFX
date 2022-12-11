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
#include <neogfx/gui/window/window.hpp>

namespace neogfx
{
    status_bar::separator::separator(i_widget& aStatusBar) :
        widget{}, iStatusBar{ aStatusBar }
    {
        set_padding(neogfx::padding{ 4.0, 0.0 });
    }

    status_bar::separator::separator(i_layout& aStatusBarLayout) :
        widget{ aStatusBarLayout }, iStatusBar{ parent() }
    {
        set_padding(neogfx::padding{ 4.0, 0.0 });
    }

    neogfx::size_policy status_bar::separator::size_policy() const
    {
        if (widget::has_size_policy())
            return widget::size_policy();
        return neogfx::size_policy{ size_constraint::Minimum, size_constraint::Expanding };
    }

    size status_bar::separator::minimum_size(optional_size const& aAvailableSpace) const
    {
        if (widget::has_minimum_size())
            return widget::minimum_size(aAvailableSpace);
        return units_converter{ *this }.from_device_units(size{ 2.0, 2.0 }) + internal_spacing().size();
    }

    void status_bar::separator::paint(i_graphics_context& aGc) const
    {
        scoped_units su{ *this, units::Pixels };
        rect line = client_rect(false);
        line.deflate(size{ 0.0_dip, 2.0_dip });
        line.cx = 1.0;
        color ink = (iStatusBar.has_background_color() ? iStatusBar.background_color() : service<i_app>().current_style().palette().color(color_role::Background));
        aGc.fill_rect(line, ink.darker(0x40).with_alpha(0.5));
        ++line.x;
        aGc.fill_rect(line, ink.lighter(0x40).with_alpha(0.5));
    }

    status_bar::keyboard_lock_status::keyboard_lock_status(i_layout& aLayout) :
        widget{ aLayout },
        iLayout{ *this }
    {
        set_padding(neogfx::padding{});
        iLayout.set_padding(neogfx::padding{});
        iLayout.set_ignore_visibility(true);
        iLayout.add(make_ref<separator>(parent()));
        auto insertLock = make_ref<label>();
        insertLock->text_widget().set_text("Insert"_s);
        insertLock->text_widget().set_font_role(font_role::StatusBar);
        iLayout.add(insertLock);
        iLayout.add(make_ref<separator>(parent()));
        auto capsLock = make_ref<label>();
        capsLock->text_widget().set_text("CAP"_s);
        capsLock->text_widget().set_font_role(font_role::StatusBar);
        iLayout.add(capsLock);
        iLayout.add(make_ref<separator>(parent()));
        auto numLock = make_ref<label>();
        numLock->text_widget().set_text("NUM"_s);
        numLock->text_widget().set_font_role(font_role::StatusBar);
        iLayout.add(numLock);
        iLayout.add(make_ref<separator>(parent()));
        auto scrlLock = make_ref<label>();
        scrlLock->text_widget().set_text("SCRL"_s);
        scrlLock->text_widget().set_font_role(font_role::StatusBar);
        iLayout.add(scrlLock);
        iUpdater = std::make_unique<widget_timer>(*this, [insertLock, capsLock, numLock, scrlLock](widget_timer& aTimer)
        {
            aTimer.again();
            auto const& keyboard = service<i_keyboard>();
            insertLock->show((keyboard.locks()& keyboard_locks::InsertLock) == keyboard_locks::InsertLock);
            capsLock->show((keyboard.locks()& keyboard_locks::CapsLock) == keyboard_locks::CapsLock);
            numLock->show((keyboard.locks()& keyboard_locks::NumLock) == keyboard_locks::NumLock);
            scrlLock->show((keyboard.locks()& keyboard_locks::ScrollLock) == keyboard_locks::ScrollLock);
        }, std::chrono::milliseconds{ 100 });
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
        set_padding(neogfx::padding{ 4.0, 0.0, 0.0, 0.0 });
        set_ignore_mouse_events(false);
        set_ignore_non_client_mouse_events(false);
        set_placement(cardinal::SouthEast);
    }

    neogfx::size_policy status_bar::size_grip_widget::size_policy() const
    {
        return neogfx::size_policy{ size_constraint::Minimum, size_constraint::Expanding };
    }

    widget_part status_bar::size_grip_widget::part(const point&) const
    {
        return widget_part{ root().as_widget(), widget_part::BorderBottomRight };
    }

    status_bar::status_bar(i_standard_layout_container& aContainer, style aStyle) :
        widget{ aContainer.status_bar_layout() },
        iStyle{ aStyle },
        iLayout{ *this },
        iNormalLayout{ iLayout },
        iMessageLayout{ iNormalLayout },
        iMessageLabel{ iMessageLayout },
        iIdleLayout{ iNormalLayout },
        iIdleLabel{ iIdleLayout },
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

    i_string const& status_bar::message() const
    {
        if (service<i_app>().help().help_active())
            return service<i_app>().help().active_help().help_text();
        else if (iMessage != std::nullopt)
            return *iMessage;
        throw no_message();
    }

    void status_bar::set_message(i_string const& aMessage)
    {
        if (iMessage != aMessage)
        {
            iMessage = aMessage;
            update_widgets();
        }
    }

    void status_bar::clear_message()
    {
        if (iMessage)
        {
            iMessage = std::nullopt;
            update_widgets();
        }
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

    void status_bar::add_normal_widget(i_ref_ptr<i_widget> const& aWidget)
    {
        iNormalWidgetLayout.add(aWidget);
        aWidget->set_font_role(font_role::StatusBar);
    }

    void status_bar::add_normal_widget_at(widget_index aPosition, i_ref_ptr<i_widget> const& aWidget)
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

    void status_bar::add_permanent_widget(i_ref_ptr<i_widget> const& aWidget)
    {
        iPermanentWidgetLayout.add(aWidget);
        aWidget->set_font_role(font_role::StatusBar);
    }

    void status_bar::add_permanent_widget_at(widget_index aPosition, i_ref_ptr<i_widget> const& aWidget)
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
        return iMessageLabel;
    }

    label& status_bar::idle_widget()
    {
        return iIdleLabel;
    }

    neogfx::size_policy status_bar::size_policy() const
    {
        if (has_size_policy())
            return widget::size_policy();
        return neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum };
    }

    widget_part status_bar::part(const point& aPosition) const
    {
        point const gripOrigin = client_rect().bottom_right() - (iSizeGrip.origin() - origin());
        rect const gripRect = { gripOrigin, size{ client_rect().bottom_right() - gripOrigin } };
        if (gripRect.contains(aPosition))
            return widget_part{ root().as_widget(), widget_part::BorderBottomRight };
        return widget::part(aPosition);
    }

    bool status_bar::is_managing_layout() const
    {
        return true;
    }

    bool status_bar::has_palette_color(color_role aColorRole) const
    {
        if (aColorRole == color_role::Background && (iStyle & style::BackgroundAsWindowBorder) == style::BackgroundAsWindowBorder)
        {
            auto owningFrame = dynamic_cast<window const*>(&parent());
            if (owningFrame)
                return true;
            else
                return widget<i_status_bar>::has_palette_color(aColorRole);
        }
        else
            return widget<i_status_bar>::has_palette_color(aColorRole);
    }

    color status_bar::palette_color(color_role aColorRole) const
    {
        if (aColorRole == color_role::Background && (iStyle & style::BackgroundAsWindowBorder) == style::BackgroundAsWindowBorder)
        {
            auto owningFrame = dynamic_cast<window const*>(&parent());
            if (owningFrame)
                return owningFrame->frame_color().darker(0x40);
            else
                return widget<i_status_bar>::palette_color(aColorRole);
        }
        else
            return widget<i_status_bar>::palette_color(aColorRole);
    }

    void status_bar::set_font(optional_font const& aFont)
    {
        widget<i_status_bar>::set_font(aFont);
        iMessageLabel.set_font(aFont);
        iIdleLabel.set_font(aFont);
        for (auto& w : iKeyboardLockStatus.children())
            w->set_font(aFont);
    }

    const i_widget& status_bar::size_grip() const
    {
        return iSizeGrip;
    }

    void status_bar::init()
    {
        if ((iStyle & style::BackgroundAsWindowBorder) == style::BackgroundAsWindowBorder && dynamic_cast<window const*>(&parent()) != nullptr)
            set_background_opacity(1.0);
        set_padding({});
        iLayout.set_padding(neogfx::padding{});
        iLayout.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
        iNormalLayout.set_padding(neogfx::padding{});
        iNormalLayout.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
        iMessageLayout.set_padding(neogfx::padding{});
        iMessageLayout.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
        iMessageLabel.set_padding(neogfx::padding{});
        iMessageLabel.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
        iMessageLabel.set_font_role(neogfx::font_role::StatusBar);
        iIdleLayout.set_padding(neogfx::padding{});
        iIdleLayout.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
        iIdleLabel.set_padding(neogfx::padding{});
        iIdleLabel.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
        iIdleLabel.set_font_role(neogfx::font_role::StatusBar);
        iNormalWidgetContainer.set_padding(neogfx::padding{});
        iNormalWidgetContainer.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
        iNormalWidgetLayout.set_padding(neogfx::padding{});
        iNormalWidgetLayout.set_size_policy(neogfx::size_policy{ size_constraint::Expanding, size_constraint::Minimum });
        iNormalWidgetLayout.set_ignore_visibility(true);
        iPermanentWidgetLayout.set_padding(neogfx::padding{});
        iPermanentWidgetLayout.set_ignore_visibility(true);
        auto update_size_grip = [this](style_aspect)
        {
            auto ink1 = (has_background_color() ? background_color() : service<i_app>().current_style().palette().color(color_role::Background));
            ink1 = ink1.shaded(0x60);
            auto ink2 = ink1.darker(0x30);
            if (iSizeGripTexture[ink1] == std::nullopt)
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
                iSizeGripTexture[ink1].emplace(
                    image{
                        dpi_select("neogfx:status_bar::iSizeGripTexture::"s, "neogfx::status_bar::iSizeGripHighDpiTexture::"s) + ink1.to_string(),
                        dpi_select(sSizeGripTextureImagePattern, sSizeGripHighDpiTextureImagePattern), { { "paper", color{} }, { "ink1", ink1 }, { "ink2", ink2 } }, dpi_select(1.0, 2.0) });
            }
            iSizeGrip.set_image(iSizeGripTexture[ink1].value());
        };
        iSink += service<i_surface_manager>().dpi_changed([update_size_grip](i_surface&) { update_size_grip(style_aspect::Geometry); });
        iSink += service<i_app>().current_style_changed(update_size_grip);
        iSink += root().window_event([update_size_grip](window_event& we) { if (we.type() == window_event_type::FocusGained || we.type() == window_event_type::FocusLost) update_size_grip(style_aspect::Color); });
        update_size_grip(style_aspect::Color);
        iSink += service<i_app>().help().help_activated([this](const i_help_source&) { update_widgets(); });
        iSink += service<i_app>().help().help_deactivated([this](const i_help_source&) { update_widgets(); });
        update_widgets();
    }

    void status_bar::update_widgets()
    {
        bool showMessage = (iStyle & style::DisplayMessage) == style::DisplayMessage && have_message();
        iMessageLabel.set_text(have_message() ? string{ message() } : string{});
        iMessageLabel.show(showMessage);
        iIdleLabel.show(!showMessage);
        iNormalWidgetContainer.show(!showMessage);
        iKeyboardLockStatus.show((iStyle & style::DisplayKeyboardLocks) == style::DisplayKeyboardLocks);
        iSizeGrip.show((iStyle & style::DisplaySizeGrip) == style::DisplaySizeGrip);
    }
}