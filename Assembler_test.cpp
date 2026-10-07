#include "assembler.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: missing input file argument" << std::endl;
        exit(1);
    }
    Assembler assembler(argv[1]);
    return 0;
}