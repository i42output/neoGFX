// nrc2.cpp
/*
neogfx C++ GUI Library
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

#include <neolib/neolib.hpp>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <neolib/json.hpp>

struct invalid_file : std::runtime_error 
{ 
    invalid_file() : std::runtime_error("Not a valid neogfx resource meta file (.nrc)!") {} 
    invalid_file(const std::string& aReason) : std::runtime_error("Not a valid neogfx resource meta file (.nrc), " + aReason + "!") {}
};
struct failed_to_read_resource_file : std::runtime_error
{
    failed_to_read_resource_file(const std::string& aPath) : std::runtime_error("Failed to read resource file '" + aPath + "'!") {}
};
struct bad_usage : std::runtime_error { bad_usage() : std::runtime_error("Bad usage") {} };

int main(int argc, char* argv[])
{
    std::cout << "nrc neogfx resource compiler" << std::endl;
    std::cout << "Copyright (c) 2016 Leigh Johnston" << std::endl << std::endl;
    std::vector<std::string> options;
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

        std::string inputFileName{ files[0] };
        std::cout << "Resource meta file: " << inputFileName << std::endl;
        neolib::rjson input{ inputFileName };
        if (!input.has_root())
            throw invalid_file("bad root node");

        std::string outputFileName;
        if (files.size() > 1)
            outputFileName = files[1];
        if (outputFileName.empty())
        {
            outputFileName = inputFileName;
            std::string::size_type dot = outputFileName.rfind('.');
            if (options.empty() || options[0] == "-embed")
            {
                if (dot == std::string::npos)
                    outputFileName += ".cpp";
                else
                    outputFileName = outputFileName.substr(0, dot) + ".cpp";
            }
            else if (options[0] == "-archive")
            {
                if (dot == std::string::npos)
                    outputFileName += ".na";
                else
                    outputFileName = outputFileName.substr(0, dot) + ".na";
            }
            else
            {
                throw bad_usage();
            }
        }
        if (options.empty() || options[0] == "-embed")
        {
            std::ofstream output{ outputFileName };
            output << "// This is a automatically generated file, do not edit!" << std::endl;
            output << "#include <neogfx/app/resource_manager.hpp>" << std::endl << std::endl;
            output << "namespace nrc" << std::endl << "{" << std::endl;
            output << "namespace" << std::endl << "{" << std::endl;
            std::vector<std::string> resourcePaths;
            std::vector<neolib::rjson_string> symbols;
            uint32_t resourceIndex = 0;
            for (const auto& item : input.root())
            {
                if (item.name() == "resource")
                {
                    auto const& resource = item.as<neolib::rjson_object>();
                    auto const& resourcePrefix = resource.has("prefix") ? resource.at("prefix").text() : "";
                    for (const auto& resourceItem : item)
                    {
                        auto process_file = [&](const neolib::rjson_string& aFile)
                        {
                            std::cout << "Processing " << aFile << "..." << std::endl;
                            resourcePaths.push_back((!resourcePrefix.empty() ? resourcePrefix + "/" : "") + aFile);
                            std::string resourcePath = boost::filesystem::path(inputFileName).parent_path().string();
                            if (!resourcePath.empty())
                                resourcePath += "/";
                            resourcePath += aFile;
                            std::ifstream resourceFile(resourcePath, std::ios_base::in | std::ios_base::binary);
                            output << "\tconst unsigned char resource_" << resourceIndex << "_data[] =" << std::endl << "\t{" << std::endl;
                            const std::size_t kBufferSize = 32;
                            bool doneSome = false;
                            unsigned char buffer[kBufferSize];
                            while (resourceFile)
                            {
                                resourceFile.read(reinterpret_cast<char*>(buffer), kBufferSize);
                                std::streamsize amount = resourceFile.gcount();
                                if (amount != 0)
                                {
                                    if (doneSome)
                                        output << ", " << std::endl;
                                    output << "\t\t";
                                    for (std::size_t j = 0; j != amount;)
                                    {
                                        output << "0x";
                                        output.width(2);
                                        output.fill('0');
                                        output << std::hex << std::uppercase << static_cast<unsigned int>(buffer[j]);
                                        if (++j != amount)
                                            output << ", ";
                                    }
                                    doneSome = true;
                                }
                                else
                                {
                                    output << std::endl;
                                    break;
                                }
                            }
                            if (resourceFile.fail() && !resourceFile.eof())
                                throw failed_to_read_resource_file(resourcePath);
                            output << "\t};" << std::endl;
                            ++resourceIndex;
                        };
                        if (resourceItem.name() == "file")
                            process_file(resourceItem.text());
                        else if (resourceItem.name() == "files")
                            for(auto const& fileItem : resourceItem)
                                process_file(fileItem.text());
                    }
                    output << "\n\tstruct register_data" << std::endl << "\t{" << std::endl;
                    output << "\t\tregister_data()" << std::endl << "\t\t{" << std::endl;
                    for (std::size_t i = 0; i < resourcePaths.size(); ++i)
                    {
                        output << "\t\t\tneogfx::resource_manager::instance().add_module_resource("
                            << "\":/" << resourcePaths[i] << "\", " << "resource_" << i << "_data, " << "sizeof(resource_" << i << "_data)"
                            << ");" << std::endl;
                    }
                    output << "\t\t}" << std::endl;
                    auto const& resourceRef = resource.has("ref") ? resource.at("ref").text() : "";
                    auto symbol = resourcePrefix;
                    if (!symbol.empty() && !resourceRef.empty())
                        symbol += "_";
                    symbol += resourceRef;
                    output << "\t} " << symbol << ";" << std::endl;
                    symbols.push_back(symbol);
                }
            }
            output << "}" << std::endl << "}" << std::endl << std::endl;
            for (auto const& symbol : symbols)
                output << "extern \"C\" void* nrc_" << symbol << " = &nrc::" << symbol << ";" << std::endl;
        }
    }
    catch (const bad_usage&)
    {
        std::cerr << "Usage: " << argv[0] << " [-embed|-archive] <input path> [<output path>]" << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}

