// file_dialog.hpp
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

#pragma once

#include <neogfx/neogfx.hpp>
#include <string>
#include <vector>
#include <optional>

namespace neogfx
{
    using file_path = std::string;
    using file_paths = std::vector<file_path>;

    using optional_file_path = std::optional<file_path>; ///< If dialog is cancelled value is std::nullopt

    struct file_dialog_spec
    {
        std::string title;
        optional_file_path defaultPathAndFile;
        std::vector<std::string> filterPatterns;
        std::string filterPatternDescription;
    };

    optional_file_path save_file_dialog(const file_dialog_spec& aSpec = {});
    file_paths open_file_dialog(const file_dialog_spec& aSpec = {}, bool aAllowMultipleSelection = false);
    optional_file_path select_folder_dialog(const std::string& aTitle = {}, const optional_file_path& aDefaultPath = {});
}