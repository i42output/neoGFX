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

#include <set>
#include <map>
#include <vector>
#include <functional>

#include <neogfx/app/settings.hpp>
#include <neogfx/gui/dialog/dialog.hpp>
#include <neogfx/gui/widget/tree_view.hpp>
#include <neogfx/gui/widget/framed_widget.hpp>

namespace neogfx
{
    class grid_layout;

    class i_setting_widget_factory : public i_reference_counted
    {
    public:
        struct unsupported_setting_type : std::runtime_error { unsupported_setting_type() : std::runtime_error{ "neogfx::i_setting_widget_factory::unsupported_setting_type" } {} };
    public:
        typedef i_setting_widget_factory abstract_type;
    public:
        virtual ~i_setting_widget_factory() = default;
    public:
        virtual void create_widget(neolib::i_setting& aSetting, i_layout& aLayout, i_string const& aFormat, sink& aSink, i_ref_ptr<i_widget>& aResult) const = 0;
        // helpers
    public:
        ref_ptr<i_widget> create_widget(neolib::i_setting& aSetting, i_layout& aLayout, i_string const& aFormat, sink& aSink) const
        {
            ref_ptr<i_widget> result;
            create_widget(aSetting, aLayout, aFormat, aSink, result);
            return result;
        }
    };

    class settings_dialog : public dialog
    {
        meta_object(dialog)
    public:
        // aGridGroups: keys of groups (e.g. "environment.formats") whose settings are laid out in a grid: a column
        // of labels and a column of widgets, one setting per row
        settings_dialog(neolib::i_settings& aSettings, ref_ptr<i_setting_widget_factory> aWidgetFactory = {}, ref_ptr<i_setting_icons> aIcons = {}, 
            std::set<std::string> const& aGridGroups = {});
        settings_dialog(i_widget& aParent, neolib::i_settings& aSettings, ref_ptr<i_setting_widget_factory> aWidgetFactory = {}, ref_ptr<i_setting_icons> aIcons = {}, 
            std::set<std::string> const& aGridGroups = {});
        ~settings_dialog();
    private:
        void init();
        // a setting array's element count changed (pending or applied): add rows for new elements; show rows up to the count
        void update_array_rows(std::string const& aArrayKey);
    private:
        struct array_row
        {
            i_widget* title = nullptr;
            std::vector<i_widget*> widgets; // cell widgets placed in the table itself
            std::vector<i_layout*> cells; // cells holding more than a single widget
        };
    private:
        neolib::i_settings& iSettings;

        ref_ptr<i_setting_widget_factory> iWidgetFactory;
        ref_ptr<i_setting_icons> iIcons;
        std::set<std::string> iGridGroups;
        neolib::i_setting* iFocusedTextSetting = nullptr;
        // widget creation state (settings array rows are added after init())
        std::map<std::string, i_widget*> iGroupWidgets; // group or subgroup key -> its widget
        std::map<std::string, std::pair<grid_layout*, std::uint32_t>> iGroupGrids; // grid, next free row
        std::map<std::string, grid_layout*> iArrayTables; // setting array key -> its table
        std::map<std::string, std::vector<array_row>> iArrayRows; // setting array key -> its rows (by element index)
        std::function<void(neolib::i_setting&)> iAddSettingWidget;
        sink iSink;
        horizontal_layout iLayout;
        tree_view iTree;
        framed_scrollable_widget iDetails;
        vertical_layout iDetailLayout;
        texture iBackground;
    };
}