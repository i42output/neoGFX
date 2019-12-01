// resource_parser.cpp
/*
neoGFX Resource Compiler
Copyright(C) 2019 Leigh Johnston

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

#include "resource_parser.hpp"

namespace neogfx::nrc
{
    struct failed_to_read_resource_file : std::runtime_error
    {
        failed_to_read_resource_file(const std::string& aPath) : std::runtime_error("Failed to read resource file '" + aPath + "'!") {}
    };

    resource_parser::resource_parser(const boost::filesystem::path& aInputFilename, const neolib::fjson_string& aNamespace, std::ofstream& aOutput) :
        iInputFilename{ aInputFilename }, iNamespace{ aNamespace }, iOutput{ aOutput }, iResourceIndex{ 0u }
    {
    }

    void resource_parser::parse(const neolib::fjson_value& aItem)
    {
        auto const& resource = aItem.as<neolib::fjson_object>();

        typedef neolib::fjson_string symbol_t;
        std::vector<std::string> resourcePaths;
        auto const& resourcePrefix = iNamespace + (resource.has("namespace") ? ("/" + resource.at("namespace").text()) : "");

        auto const& resourceRef = resource.has("ref") ? resource.at("ref").text() : "";
        auto symbol = resourcePrefix;
        if (!symbol.empty() && !resourceRef.empty())
            symbol += "_";
        symbol += resourceRef;

        iOutput << "namespace nrc" << std::endl << "{" << std::endl;
        iOutput << "namespace" << std::endl << "{" << std::endl;

        auto nextResourceIndex = iResourceIndex;

        for (const auto& resourceItem : resource.contents())
        {
            auto process_file = [&](const neolib::fjson_string& aInputFilename)
            {
                std::cout << "Processing " << aInputFilename << "..." << std::endl;
                resourcePaths.push_back((!resourcePrefix.empty() ? resourcePrefix + "/" : "") + aInputFilename);
                std::string resourcePath = boost::filesystem::path(iInputFilename).parent_path().string();
                if (!resourcePath.empty())
                    resourcePath += "/";
                resourcePath += aInputFilename;
                std::ifstream resourceFile(resourcePath, std::ios_base::in | std::ios_base::binary);
                iOutput << "\tconst unsigned char resource_" << nextResourceIndex << "_data[] =" << std::endl << "\t{" << std::endl;
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
                            iOutput << ", " << std::endl;
                        iOutput << "\t\t";
                        for (std::size_t j = 0; j != amount;)
                        {
                            iOutput << "0x";
                            iOutput.width(2);
                            iOutput.fill('0');
                            iOutput << std::hex << std::uppercase << static_cast<unsigned int>(buffer[j]);
                            if (++j != amount)
                                iOutput << ", ";
                        }
                        doneSome = true;
                    }
                    else
                    {
                        iOutput << std::endl;
                        break;
                    }
                }
                if (resourceFile.fail() && !resourceFile.eof())
                    throw failed_to_read_resource_file(resourcePath);
                iOutput << "\t};" << std::endl;
                ++nextResourceIndex;
            };

            if (resourceItem.name() == "file")
                process_file(resourceItem.text());
            else if (resourceItem.name() == "files")
                for (auto const& fileItem : resourceItem)
                    process_file(fileItem.text());
            else
                continue;
        }

        iOutput << "\n\tstruct register_" << "resource_" << iResourceIndex << std::endl << "\t{" << std::endl;
        iOutput << "\t\tregister_" << "resource_" << iResourceIndex << "()" << std::endl << "\t\t{" << std::endl;
        for (std::size_t i = 0; i < resourcePaths.size(); ++i, ++iResourceIndex)
        {
            iOutput << "\t\t\tneogfx::resource_manager::instance().add_module_resource("
                << "\":/" << resourcePaths[i] << "\", " << "resource_" << iResourceIndex << "_data, " << "sizeof(resource_" << iResourceIndex << "_data)"
                << ");" << std::endl;
        }

        iOutput << "\t\t}" << std::endl;

        iOutput << "\t} " << symbol << ";" << std::endl;

        iOutput << "}" << std::endl << "}" << std::endl << std::endl;

        iOutput << "extern \"C\" void* nrc_" << symbol << " = &nrc::" << symbol << ";" << std::endl << std::endl;
    }
}
