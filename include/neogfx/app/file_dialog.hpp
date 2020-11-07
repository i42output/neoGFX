// file_dialog.hpp
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
#include <string>
#include <vector>
#include <optional>

namespace neogfx
{
    class i_widget;

    // todo: use std::filesystem

    using file_path = std::string;
    using file_paths = std::vector<file_path>;
    using optional_file_path = std::optional<file_path>; ///< If dialog is cancelled value is std::nullopt
    using optional_file_paths = std::optional<file_paths>; ///< If dialog is cancelled value is std::nullopt

    using filter_pattern = std::string;
    using filter_patterns = std::vector<filter_pattern>;

    struct file_dialog_spec
    {
        std::optional<std::string> title;
        optional_file_path defaultPathAndFile;
        filter_patterns filterPatterns;
        std::optional<std::string> filterPatternDescription;
    };

    optional_file_path save_file_dialog(file_dialog_spec const& aSpec = {});
    optional_file_paths open_file_dialog(file_dialog_spec const& aSpec = {}, bool aAllowMultipleSelection = false);
    optional_file_path select_folder_dialog(std::optional<std::string> const& aTitle = {}, optional_file_path const& aDefaultPath = {});

    optional_file_path save_file_dialog(i_widget& aParent, file_dialog_spec const& aSpec = {});
    optional_file_paths open_file_dialog(i_widget& aParent, file_dialog_spec const& aSpec = {}, bool aAllowMultipleSelection = false);
    optional_file_path select_folder_dialog(i_widget& aParent, std::optional<std::string> const& aTitle = {}, optional_file_path const& aDefaultPath = {});
}