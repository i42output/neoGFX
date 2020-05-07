// dialog.hpp
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
#include <neogfx/app/i_app.hpp>
#include <neogfx/app/i_basic_services.hpp>
#include <neogfx/app/event_processing_context.hpp>
#include <neogfx/gui/dialog/dialog.hpp>

namespace neogfx
{
    dialog::dialog(window_style aStyle) :
        window{ aStyle }
    {
        init();
    }

    dialog::dialog(const std::string& aDialogTitle, window_style aStyle) :
        window{ aDialogTitle, aStyle }
    {
        init();
    }

    dialog::dialog(const size& aDimensions, window_style aStyle) :
        window{ aDimensions, aStyle }
    {
        init();
    }

    dialog::dialog(const size& aDimensions, const std::string& aDialogTitle, window_style aStyle) :
        window{ aDimensions, aDialogTitle, aStyle }
    {
        init();
    }

    dialog::dialog(const point& aPosition, const size& aDimensions, window_style aStyle) :
        window{ rect{ aPosition, aDimensions }, {}, aStyle }
    {
        init();
    }

    dialog::dialog(const point& aPosition, const size& aDimensions, const std::string& aDialogTitle, window_style aStyle) :
        window{ rect{ aPosition, aDimensions }, aDialogTitle, aStyle }
    {
        init();
    }

    dialog::dialog(i_widget& aParent, window_style aStyle) :
        window{ aParent, aStyle }
    {
        init();
    }

    dialog::dialog(i_widget& aParent, const std::string& aDialogTitle, window_style aStyle) :
        window{ aParent, aDialogTitle, aStyle }
    {
        init();
    }

    dialog::dialog(i_widget& aParent, const size& aDimensions, window_style aStyle) :
        window{ aParent, aDimensions, aStyle }
    {
        init();
    }

    dialog::dialog(i_widget& aParent, const size& aDimensions, const std::string& aDialogTitle, window_style aStyle) :
        window{ aParent, aDimensions, aDialogTitle, aStyle }
    {
        init();
    }

    dialog::dialog(i_widget& aParent, const point& aPosition, const size& aDimensions, window_style aStyle) :
        window{ aParent, rect{ aPosition, aDimensions }, aStyle }
    {
        init();
    }

    dialog::dialog(i_widget& aParent, const point& aPosition, const size& aDimensions, const std::string& aDialogTitle, window_style aStyle) :
        window{ aParent, rect{ aPosition, aDimensions }, aDialogTitle, aStyle }
    {
        init();
    }

    dialog::~dialog()
    {
    }

    void dialog::accept()
    {
        if (result() != dialog_result::Accepted)
        {
            bool canAccept = true;
            TryAccept.trigger(canAccept);
            if (canAccept)
                set_result(dialog_result::Accepted);
            else
                service<i_basic_services>().system_beep();
        }
    }

    void dialog::reject()
    {
        if (result() != dialog_result::Rejected)
        {
            bool canReject = true;
            TryReject.trigger(canReject);
            if (canReject)
                set_result(dialog_result::Rejected);
            else
                service<i_basic_services>().system_beep();
        }
    }

    dialog_result dialog::result() const
    {
        if (iResult != std::nullopt)
            return *iResult;
        return dialog_result::NoResult;
    }

    void dialog::set_result(dialog_result aResult)
    {
        iResult = aResult;
    }

    size dialog::set_standard_layout(const size& aControlSpacing, bool aCreateButtonBox, bool aDpiScaling)
    {
        auto ajustedSpacing = (aDpiScaling ? dpi_scale(aControlSpacing) : aControlSpacing);
        set_margins(neogfx::margins{});
        client_layout().set_margins(neogfx::margins{ ajustedSpacing.cx, ajustedSpacing.cy, ajustedSpacing.cx, ajustedSpacing.cy });
        client_layout().set_spacing(ajustedSpacing);
        button_box_layout().set_margins(neogfx::margins{ ajustedSpacing.cx, ajustedSpacing.cy, ajustedSpacing.cx, ajustedSpacing.cy });
        button_box_layout().set_spacing(ajustedSpacing);
        if (aCreateButtonBox)
            button_box().layout().set_spacing(ajustedSpacing);
        return ajustedSpacing;
    }

    dialog_button_box& dialog::button_box()
    {
        if (iButtonBox == std::nullopt)
        {
            iButtonBox.emplace(button_box_layout());
            button_box().layout().set_spacing(client_layout().spacing());
            button_box().Accepted([this]()
            {
                accept();
            });
            button_box().Rejected([this]()
            {
                reject();
            });
        }
        return *iButtonBox;
    }

    dialog_result dialog::exec()
    {
        destroyed_flag destroyed{ surface() };
        event_processing_context epc(service<async_task>(), "neogfx::dialog");
        while (iResult == std::nullopt)
        {
            service<i_app>().process_events(epc);
            if (destroyed && result() == dialog_result::NoResult)
                set_result(dialog_result::Rejected);
        }
        return *iResult;
    }

    neogfx::size_policy dialog::size_policy() const
    {
        if (window::has_size_policy())
            return window::size_policy();
        return size_constraint::Minimum;
    }

    bool dialog::can_close() const
    {
        bool canReject = true;
        TryReject.trigger(canReject);
        return canReject;
    }

    bool dialog::key_pressed(scan_code_e aScanCode, key_code_e aKeyCode, key_modifiers_e aKeyModifiers)
    {
        switch (aScanCode)
        {
        case ScanCode_RETURN:
            accept();
            return true;
        case ScanCode_ESCAPE:
            if ((style() & window_style::Close) == window_style::Close)
                reject();
            return true;
        default:
            return window::key_pressed(aScanCode, aKeyCode, aKeyModifiers);
        }
    }

    bool dialog::has_layout(standard_layout aStandardLayout) const
    {
        switch (aStandardLayout)
        {
        case standard_layout::ButtonBox:
            return true;
        default:
            return window::has_layout(aStandardLayout);
        }
    }

    const i_layout& dialog::layout(standard_layout aStandardLayout, layout_position aPosition) const
    {
        switch (aStandardLayout)
        {
        case standard_layout::ButtonBox:
            return iButtonBoxLayout;
        default:
            return window::layout(aStandardLayout, aPosition);
        }
    }

    i_layout& dialog::layout(standard_layout aStandardLayout, layout_position aPosition)
    {
        return const_cast<i_layout&>(to_const(*this).layout(aStandardLayout, aPosition));
    }

    void dialog::init()
    {
        non_client_layout().add_at(non_client_layout().index_of(status_bar_layout()), iButtonBoxLayout);
        iButtonBoxLayout.set_weight(size{});
        set_standard_layout(service<i_app>().current_style().spacing(), false);
    }
}