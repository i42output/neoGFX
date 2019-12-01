// nrc.cpp
/*
neoGFX Resource Compiler
Copyright(C) 2016 Leigh Johnston

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
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <neolib/json.hpp>
#include <neolib/application.hpp>

#include "resource_parser.hpp"
#include "ui_parser.hpp"

namespace neogfx::nrc
{

    struct invalid_file : std::runtime_error
    {
        invalid_file() : std::runtime_error("Not a valid neoGFX resource meta file (.nrc)!") {}
        invalid_file(const std::string& aReason) : std::runtime_error("Not a valid neoGFX resource meta file (.nrc), " + aReason + "!") {}
    };
    struct bad_usage : std::runtime_error { bad_usage() : std::runtime_error("Bad usage") {} };

    void parse_ui(const boost::filesystem::path& aInputFilename, neolib::i_plugin_manager& aPluginManager, const neolib::fjson_string& aNamespace, const neolib::fjson_value& aItem, std::ofstream& aOutput)
    {
        auto const& ui = aItem.as<neolib::fjson_object>();
        auto ns = aNamespace + (ui.has("namespace") ? "/" + ui.at("namespace").text() : "");
        ui_parser uiParser{ aInputFilename, aPluginManager, ns, ui, aOutput };
    }
}

int main(int argc, char* argv[])
{
    using namespace neogfx;
    using namespace nrc;

    neolib::application_info appInfo
    {
        neolib::to_program_arguments(argc, argv),
        "nrc neoGFX resource compiler",
        "i42 Software",
        neolib::version{ 1, 0, 0, 0 },
        "Copyright (c) 2019 Leigh Johnston",
        {}, {}, {}, ".nel"
    };
    neolib::application app{ appInfo };

    std::cout << "------ " << appInfo.name() << " ------" << std::endl;
    std::cout << appInfo.copyright() << std::endl << std::endl;

    std::cout << "Loading neoGFX element libraries..." << std::endl;
    app.plugin_manager().load_plugins();
    for (auto const& plugin : app.plugin_manager().plugins())
        std::cout << "Element library '" << plugin->name() << "' loaded." << std::endl;

    std::cout << std::endl;

    std::vector<std::string> options; // todo: use boost.ProgramOptions
    std::vector<std::string> files;
    for (int a = 1; a < argc; ++a)
        if (argv[a][0] == '-')
            options.push_back(argv[a]);
        else
            files.push_back(argv[a]);
    try
    {
        if (files.size() < 1 || files.size() > 2)
        {
            throw bad_usage();
        }

        boost::filesystem::path const inputFileName{ files[0] };
        std::cout << "Resource meta file: " << inputFileName << std::endl;
        neolib::fjson const input{ inputFileName.string() };
        if (!input.has_root())
            throw invalid_file("bad root node");

        std::string const outputDirectory{ files.size() > 1 ? files[1] : boost::filesystem::current_path().string() };

        std::string resourceFileName;
        std::string uiFileName;
        if (options.empty() || options[0] == "-embed")
        {
            resourceFileName = inputFileName.filename().stem().string() + ".res.cpp";
            uiFileName = inputFileName.filename().stem().string() + ".ui.hpp";
        }
        else if (options[0] == "-archive")
            throw not_yet_implemented("nrc");
        else
            throw bad_usage();

        if (options.empty() || options[0] == "-embed")
        {
            std::optional<std::ofstream> resourceOutput;
            std::optional<std::ofstream> uiOutput;
            auto const& ns = input.root().as<neolib::fjson_object>().has("namespace") ? input.root().as<neolib::fjson_object>().at("namespace").text() : "";
            std::optional<resource_parser> resourceParser;
            for (const auto& item : input.root())
            {
                if (item.name() == "resource")
                {
                    if (resourceOutput == std::nullopt)
                    {
                        auto resourceOutputPath = outputDirectory + "/" + resourceFileName;
                        std::cout << "Creating " << resourceOutputPath << "..." << std::endl;
                        resourceOutput.emplace(resourceOutputPath);
                        *resourceOutput << "// This is an automatically generated file, do not edit!" << std::endl << std::endl;
                        *resourceOutput << "#include <neogfx/app/resource_manager.hpp>" << std::endl << std::endl;
                    }
                    if (resourceParser == std::nullopt)
                        resourceParser.emplace(inputFileName, ns, *resourceOutput);
                    resourceParser->parse(item);
                }
                else if (item.name() == "ui")
                {
                    if (uiOutput == std::nullopt)
                    {
                        auto uiOutputPath = outputDirectory + "/" + uiFileName;
                        std::cout << "Creating " << uiOutputPath << "..." << std::endl;
                        uiOutput.emplace(uiOutputPath);
                    }
                    parse_ui(inputFileName, app.plugin_manager(), ns, item, *uiOutput);
                }
            }
        }
    }
    catch (const bad_usage&)
    {
        std::cerr << "Usage: " << argv[0] << " [-embed|-archive] <input path> [<output directory>]" << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
