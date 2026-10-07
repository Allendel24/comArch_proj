#include "Assembler.hpp"

#include <cstdint>
#include <cctype>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
constexpr std::size_t kMemorySize = 65536;

struct SourceLine {
    int address;
    std::string opcode;
    std::vector<std::string> args;
};

const std::unordered_map<std::string, int> opcodes = {
    {"add", 0}, {"nand", 1}, {"lw", 2}, {"sw", 3},
    {"beq", 4}, {"jalr", 5}, {"halt", 6}, {"noop", 7}
};

long long parseNumber(const std::string& token, int line) {
    std::size_t consumed = 0;
    try {
        const long long value = std::stoll(token, &consumed, 10);
        if (consumed != token.size()) throw std::invalid_argument("trailing characters");
        return value;
    } catch (const std::exception&) {
        throw std::runtime_error("line " + std::to_string(line) + ": invalid number '" + token + "'");
    }
}

int registerNumber(const std::string& token, int line) {
    const long long value = parseNumber(token, line);
    if (value < 0 || value > 7) {
        throw std::runtime_error("line " + std::to_string(line) + ": register must be in [0, 7]");
    }
    return static_cast<int>(value);
}

int signedOffset(long long value, int line) {
    if (value < -32768 || value > 32767) {
        throw std::runtime_error("line " + std::to_string(line) + ": offset does not fit in 16 bits");
    }
    return static_cast<int>(value) & 0xffff;
}

bool isNumericLiteral(const std::string& token) {
    if (token.empty()) return false;
    const std::size_t first_digit = (token[0] == '+' || token[0] == '-') ? 1 : 0;
    return first_digit < token.size() && std::isdigit(static_cast<unsigned char>(token[first_digit]));
}

int resolveValue(const std::string& token, int line,
                 const std::unordered_map<std::string, int>& labels) {
    if (!isNumericLiteral(token)) {
        const auto found = labels.find(token);
        if (found == labels.end()) {
            throw std::runtime_error("line " + std::to_string(line) + ": undefined label '" + token + "'");
        }
        return found->second;
    }

    const long long value = parseNumber(token, line);
    if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max()) {
        throw std::runtime_error("line " + std::to_string(line) + ": value is outside 32-bit range");
    }
    return static_cast<int>(value);
}

std::vector<SourceLine> readSource(const std::string& path,
                                   std::unordered_map<std::string, int>& labels) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("cannot open assembly file '" + path + "'");

    std::vector<SourceLine> lines;
    std::string text;
    int source_line = 0;
    while (std::getline(input, text)) {
        ++source_line;
        const auto comment = text.find('#');
        if (comment != std::string::npos) text.erase(comment);

        std::istringstream words(text);
        std::vector<std::string> fields;
        for (std::string field; words >> field;) fields.push_back(std::move(field));
        if (fields.empty()) continue;
        if (lines.size() == kMemorySize) {
            throw std::runtime_error("line " + std::to_string(source_line) + ": program exceeds LC-2K memory capacity");
        }

        std::size_t index = 0;
        if (opcodes.count(fields[0]) == 0 && fields[0] != ".fill") {
            const std::string& name = fields[index++];
            if (labels.count(name)) {
                throw std::runtime_error("line " + std::to_string(source_line) + ": duplicate label '" + name + "'");
            }
            labels.emplace(name, static_cast<int>(lines.size()));
            if (index == fields.size()) {
                throw std::runtime_error("line " + std::to_string(source_line) + ": label has no instruction");
            }
        }

        SourceLine line{static_cast<int>(lines.size()), fields[index++], {}};
        if (line.opcode != ".fill" && opcodes.count(line.opcode) == 0) {
            throw std::runtime_error("line " + std::to_string(source_line) + ": unknown opcode '" + line.opcode + "'");
        }
        while (index < fields.size()) line.args.push_back(fields[index++]);

        const std::size_t expected = line.opcode == "add" || line.opcode == "nand" ||
            line.opcode == "lw" || line.opcode == "sw" || line.opcode == "beq" ? 3 :
            line.opcode == "jalr" ? 2 : line.opcode == ".fill" ? 1 : 0;
        if (line.args.size() != expected) {
            throw std::runtime_error("line " + std::to_string(source_line) + ": wrong number of operands for '" + line.opcode + "'");
        }
        lines.push_back(std::move(line));
    }
    if (input.bad()) throw std::runtime_error("error reading assembly file '" + path + "'");
    return lines;
}

int encode(const SourceLine& line, const std::unordered_map<std::string, int>& labels) {
    if (line.opcode == ".fill") return resolveValue(line.args[0], line.address + 1, labels);

    const int opcode = opcodes.at(line.opcode);
    const int a = line.args.size() >= 1 ? registerNumber(line.args[0], line.address + 1) : 0;
    const int b = line.args.size() >= 2 ? registerNumber(line.args[1], line.address + 1) : 0;
    std::uint32_t instruction = static_cast<std::uint32_t>(opcode) << 22;
    instruction |= static_cast<std::uint32_t>(a) << 19;
    instruction |= static_cast<std::uint32_t>(b) << 16;

    if (line.opcode == "add" || line.opcode == "nand") {
        instruction |= static_cast<std::uint32_t>(registerNumber(line.args[2], line.address + 1));
    } else if (line.opcode == "lw" || line.opcode == "sw" || line.opcode == "beq") {
        long long offset;
        const auto label = labels.find(line.args[2]);
        if (label != labels.end()) {
            offset = line.opcode == "beq" ? label->second - (line.address + 1) : label->second;
        } else if (isNumericLiteral(line.args[2])) {
            offset = parseNumber(line.args[2], line.address + 1);
        } else {
            throw std::runtime_error("line " + std::to_string(line.address + 1) + ": undefined label '" + line.args[2] + "'");
        }
        instruction |= static_cast<std::uint32_t>(signedOffset(offset, line.address + 1));
    }
    return static_cast<int>(instruction);
}
}

void Assembler::assemble(const std::string& input_path, const std::string& output_path) {
    std::unordered_map<std::string, int> labels;
    const auto lines = readSource(input_path, labels);
    std::vector<int> machine_code;
    machine_code.reserve(lines.size());
    for (const auto& line : lines) machine_code.push_back(encode(line, labels));

    std::ofstream output(output_path);
    if (!output) throw std::runtime_error("cannot open output file '" + output_path + "'");
    for (const int word : machine_code) output << word << '\n';
    if (!output) throw std::runtime_error("error writing machine code to '" + output_path + "'");
}
