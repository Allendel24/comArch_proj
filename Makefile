CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic
BUILD_DIR := build
OUTPUT_DIR := output
ASSEMBLER := $(BUILD_DIR)/assembler
SIMULATOR := $(BUILD_DIR)/simulator
ASSEMBLY_SOURCES := $(wildcard asm/*.as)
MACHINE_FILES := $(patsubst asm/%.as,$(OUTPUT_DIR)/%.mc,$(ASSEMBLY_SOURCES))
TRACE_FILES := $(patsubst $(OUTPUT_DIR)/%.mc,$(OUTPUT_DIR)/%.trace,$(MACHINE_FILES))

.PHONY: all assembler simulator run run-mult run-comb clean

all: $(ASSEMBLER) $(SIMULATOR) $(MACHINE_FILES)

assembler: $(ASSEMBLER)
simulator: $(SIMULATOR)

$(BUILD_DIR) $(OUTPUT_DIR):
	mkdir -p $@

$(ASSEMBLER): Assembler.cpp Assembler.hpp assembler_main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) Assembler.cpp assembler_main.cpp -o $@

$(SIMULATOR): simulator.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) simulator.cpp -o $@

$(OUTPUT_DIR)/%.mc: asm/%.as $(ASSEMBLER) | $(OUTPUT_DIR)
	$(ASSEMBLER) $< $@

$(OUTPUT_DIR)/%.trace: $(OUTPUT_DIR)/%.mc $(SIMULATOR) | $(OUTPUT_DIR)
	$(SIMULATOR) $< > $@

run: run-mult run-comb
run-mult: $(OUTPUT_DIR)/mult.trace
run-comb: $(OUTPUT_DIR)/comb.trace

clean:
	rm -f $(ASSEMBLER) $(SIMULATOR)
	rm -f $(MACHINE_FILES) $(TRACE_FILES)
	rmdir $(OUTPUT_DIR) 2>/dev/null || true
