#include "Assembler.hpp"

#include <exception>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "usage: " << argv[0] << " <input.as> <output.mc>\n";
        return 2;
    }
    try {
        Assembler::assemble(argv[1], argv[2]);
    } catch (const std::exception& error) {
        std::cerr << "assembler: " << error.what() << '\n';
        return 1;
    }
}
