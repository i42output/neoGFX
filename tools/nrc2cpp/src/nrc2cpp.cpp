// nrc2cpp.cpp
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

#include <fstream>
#include <iostream>
#include <neolib/xml.hpp>

struct invalid_file : std::runtime_error 
{ 
	invalid_file() : std::runtime_error("Not a valid neogfx resource meta file (.nrc)!") {} 
	invalid_file(const std::string& aReason) : std::runtime_error("Not a valid neogfx resource meta file (.nrc), " + aReason + "!") {}
};

int main(int argc, char* argv[])
{
	std::cout << "nrc2cpp neogfx resource compiler" << std::endl;
	std::cout << "Copyright (c) 2016 Leigh Johnston" << std::endl << std::endl;
	if (argc < 2 || argc > 3)
	{
		std::cerr << "Usage: " << argv[0] << " <input path> [<output path>]" << std::endl;
		return EXIT_FAILURE;
	}
	try
	{
		std::string inputFileName(argv[1]);
		std::cout << "Resource meta file: " << inputFileName << std::endl;
		neolib::xml input(inputFileName);
		if (!input.got_root() || input.root().name() != "nrc")
			throw invalid_file("bad root node");
		for (const auto& resource : input.root())
		{
			if (resource.name() == "resource")
			{
				for (const auto& file : resource)
				{
					if (file.name() == "file")
					{
						std::cout << "Processing " << std::string(file.text()) << "..." << std::endl;
					}
				}
			}
		}
		std::string outputFileName;
		if (argc == 3)
			outputFileName = argv[2];
		if (outputFileName.empty())
		{
			outputFileName = inputFileName;
			std::string::size_type dot = outputFileName.rfind('.');
			if (dot == std::string::npos)
				outputFileName += ".cpp";
			else
				outputFileName = outputFileName.substr(0, dot) + ".cpp";
		}
		std::ofstream output(outputFileName);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return 0;
}

