// glsl2hpp.cpp : Defines the entry point for the console application.
//

#include <fstream>
#include <string>

int main(int argc, char* argv[])
{
    std::ifstream input(argv[1]);
    std::ofstream output(argv[2]);
    output << "namespace glsl\n{\n\n";
    output << "\tconst char* const " << argv[3] << " =";
    std::string line;
    while (std::getline(input, line))
    {
        output << "\n";
        output << "\t\"" << line << "\\n\"";
    }
    output << ";\n\n}\n";
    return 0;
}

