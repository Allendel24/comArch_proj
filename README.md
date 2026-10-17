
mkdir -p build output
g++ -std=c++17 -O2 Assembler.cpp assembler_main.cpp -o build/assembler
g++ -std=c++17 -O2 simulator.cpp -o build/simulator
./build/assembler asm/mult.as output/mult.mc
./build/simulator output/mult.mc > output/mult.trace