#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>

namespace {
constexpr std::size_t kMemorySize = 65536;
constexpr std::size_t kRegisterCount = 8;

struct MachineState {
    int pc = 0;
    std::array<std::int32_t, kMemorySize> memory{};
    std::array<std::int32_t, kRegisterCount> registers{};
    std::size_t loaded_words = 0;
};

std::int32_t fromBits(std::uint32_t value) {
    if (value <= 0x7fffffffU) return static_cast<std::int32_t>(value);
    return static_cast<std::int32_t>(static_cast<std::int64_t>(value) - 0x100000000LL);
}

int signExtend16(std::uint32_t value) {
    value &= 0xffff;
    return value & 0x8000 ? static_cast<int>(value) - 0x10000 : static_cast<int>(value);
}

void printState(const MachineState& state) {
    std::cout << "\n@@@\nstate:\n\tpc " << state.pc << "\n\tmemory:\n";
    for (std::size_t i = 0; i < state.loaded_words; ++i) {
        std::cout << "\t\tmem[ " << i << " ] " << state.memory[i] << '\n';
    }
    std::cout << "\tregisters:\n";
    for (std::size_t i = 0; i < state.registers.size(); ++i) {
        std::cout << "\t\treg[ " << i << " ] " << state.registers[i] << '\n';
    }
    std::cout << "end state\n";
}

bool validAddress(std::int64_t address) {
    return address >= 0 && static_cast<std::uint64_t>(address) < kMemorySize;
}
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <machine-code.mc>\n";
        return 2;
    }

    std::ifstream input(argv[1]);
    if (!input) {
        std::cerr << "simulator: cannot open machine-code file '" << argv[1] << "'\n";
        return 1;
    }

    MachineState state;
    for (std::int32_t word; input >> word;) {
        if (state.loaded_words == kMemorySize) {
            std::cerr << "simulator: program exceeds memory capacity\n";
            return 1;
        }
        state.memory[state.loaded_words++] = word;
    }
    if (!input.eof()) {
        std::cerr << "simulator: invalid machine-code input\n";
        return 1;
    }
    if (state.loaded_words == 0) {
        std::cerr << "simulator: machine-code file is empty\n";
        return 1;
    }

    std::uint64_t instruction_count = 0;
    while (true) {
        if (!validAddress(state.pc)) {
            std::cerr << "simulator: pc out of bounds: " << state.pc << '\n';
            return 1;
        }
        printState(state);

        const std::uint32_t instruction = static_cast<std::uint32_t>(state.memory[state.pc]);
        const int opcode = (instruction >> 22) & 7;
        const int reg_a = (instruction >> 19) & 7;
        const int reg_b = (instruction >> 16) & 7;
        const int dest = instruction & 7;
        const int offset = signExtend16(instruction);
        ++state.pc;
        ++instruction_count;

        switch (opcode) {
            case 0:
                state.registers[dest] = fromBits(static_cast<std::uint32_t>(state.registers[reg_a]) +
                                                 static_cast<std::uint32_t>(state.registers[reg_b]));
                break;
            case 1:
                state.registers[dest] = fromBits(~(static_cast<std::uint32_t>(state.registers[reg_a]) &
                                                   static_cast<std::uint32_t>(state.registers[reg_b])));
                break;
            case 2:
            case 3: {
                const std::int64_t address = static_cast<std::int64_t>(state.registers[reg_a]) + offset;
                if (!validAddress(address)) {
                    std::cerr << "simulator: memory address out of bounds: " << address << '\n';
                    return 1;
                }
                if (opcode == 2) state.registers[reg_b] = state.memory[static_cast<std::size_t>(address)];
                else state.memory[static_cast<std::size_t>(address)] = state.registers[reg_b];
                break;
            }
            case 4:
                if (state.registers[reg_a] == state.registers[reg_b]) state.pc += offset;
                break;
            case 5: {
                const int target = state.registers[reg_a];
                state.registers[reg_b] = state.pc;
                state.pc = target;
                break;
            }
            case 6:
                state.registers[0] = 0;
                std::cout << "machine halted\ntotal of " << instruction_count
                          << " instructions executed\nfinal state of machine:\n";
                printState(state);
                return 0;
            case 7:
                break;
        }
        state.registers[0] = 0;
    }
}
