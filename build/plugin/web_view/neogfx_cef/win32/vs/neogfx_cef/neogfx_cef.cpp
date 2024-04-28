/*
  neogfx_cef.cpp

  Copyright (c) 2024 Leigh Johnston.  All Rights Reserved.

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

#include <iostream>
#include <boost/program_options.hpp>
#include <cef/include/cef_app.h>

int main(int argc, char* argv[])
{
    boost::program_options::options_description description;
    description.add_options()
        ("user-data-dir", boost::program_options::value<std::string>()->implicit_value(""), "");
    boost::program_options::variables_map options;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(description).allow_unregistered().run(), options);

    CefMainArgs mainArgs{ ::GetModuleHandle(nullptr) };

    CefSettings settings;
    if (options.count("user-data-dir"))
    {
        std::cout << "neogfx_cef: user-data-dir: [" << options.at("user-data-dir").as<std::string>() << "]" << std::endl;
        CefString rootCachePath{ options.at("user-data-dir").as<std::string>() };
        settings.root_cache_path = *rootCachePath.GetWritableStruct();
        rootCachePath.Detach();
    }
    settings.windowless_rendering_enabled = true;

    CefInitialize(mainArgs, settings, nullptr, nullptr);

    CefRunMessageLoop();

    CefShutdown();

    return 0;
}
