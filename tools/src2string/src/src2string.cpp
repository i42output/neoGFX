// glsl2hpp.cpp : Defines the entry point for the console application.
//

#include <fstream>
#include <string>
#include <filesystem>

int main(int argc, char* argv[])
{
    if (argc != 5)
        exit(EXIT_FAILURE);
    const char* const inputFile = argv[1];
    const char* const outputFile = argv[2];
    auto const srcType = std::filesystem::path{ inputFile }.extension().string();
    const char* const srcNamespace = argv[3];
    const char* const srcSymbol = argv[4];
    std::ifstream input(inputFile);
    std::ofstream output(outputFile);
    output << "namespace " << srcNamespace << "\n{" << std::endl;
    output << "    const char* const " << srcSymbol << " =" << std::endl;
    output << "    1 + R\"" << srcType << "(" << std::endl;
    std::string line;
    while (std::getline(input, line))
    {
        output << line << std::endl;
    }
    output << "    )" << srcType << "\";" << std::endl;
    output << "}" << std::endl;

    return 0;
}

