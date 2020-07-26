// settings_dialog.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <neolib/app/i_settings.hpp>
#include <neogfx/gui/dialog/dialog.hpp>
#include <neogfx/gui/widget/tree_view.hpp>
#include <neogfx/gui/widget/framed_widget.hpp>

namespace neogfx
{
    class i_setting_widget_factory : public i_reference_counted
    {
    public:
        struct unsupported_setting_type : std::runtime_error { unsupported_setting_type() : std::runtime_error{ "neogfx::i_setting_widget_factory::unsupported_setting_type" } {} };
    public:
        typedef i_setting_widget_factory abstract_type;
    public:
        virtual ~i_setting_widget_factory() = default;
    public:
        virtual std::shared_ptr<i_widget> create_widget(neolib::i_setting& aSetting, i_layout& aLayout, sink& aSink) const = 0;
    };

    class settings_dialog : public dialog
    {
    public:
        settings_dialog(neolib::i_settings& aSettings, ref_ptr<i_setting_widget_factory> aWidgetFactory = {}, image&& aIcon = image{ ":/neogfx/resources/images/settings.png" });
        settings_dialog(i_widget& aParent, neolib::i_settings& aSettings, ref_ptr<i_setting_widget_factory> aWidgetFactory = {}, image&& aIcon = image{ ":/neogfx/resources/images/settings.png" });
        ~settings_dialog();
    private:
        void init();
    private:
        neolib::i_settings& iSettings;
        ref_ptr<i_setting_widget_factory> iWidgetFactory;
        sink iSink;
        horizontal_layout iLayout;
        tree_view iTree;
        framed_scrollable_widget iDetails;
        vertical_layout iDetailLayout;
        image iIcon;
        texture iBackground;
    };
}