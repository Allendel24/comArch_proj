CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic
BUILD_DIR := build
OUTPUT_DIR := output
ifeq ($(OS),Windows_NT)
EXEEXT := .exe
else
EXEEXT :=
endif
ASSEMBLER := $(BUILD_DIR)/assembler$(EXEEXT)
SIMULATOR := $(BUILD_DIR)/simulator$(EXEEXT)
ASSEMBLY_SOURCES := $(wildcard asm/*.as)
MACHINE_FILES := $(patsubst asm/%.as,$(OUTPUT_DIR)/%.mc,$(ASSEMBLY_SOURCES))
TRACE_FILES := $(patsubst $(OUTPUT_DIR)/%.mc,$(OUTPUT_DIR)/%.trace,$(MACHINE_FILES))
CLEAN_FILES := $(ASSEMBLER) $(SIMULATOR) $(MACHINE_FILES) $(TRACE_FILES)

ifeq ($(OS),Windows_NT)
MKDIR = powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$@' | Out-Null"
empty :=
space := $(empty) $(empty)
comma := ,
WINDOWS_CLEAN_FILES := $(subst $(space),$(comma),$(foreach file,$(CLEAN_FILES),'$(file)'))
CLEAN = powershell -NoProfile -Command "Remove-Item -Force -ErrorAction SilentlyContinue @($(WINDOWS_CLEAN_FILES))"
else
MKDIR = mkdir -p "$@"
CLEAN = rm -f $(CLEAN_FILES)
endif

.PHONY: all assembler simulator run run-mult run-comb clean

all: $(ASSEMBLER) $(SIMULATOR) $(MACHINE_FILES)

assembler: $(ASSEMBLER)
simulator: $(SIMULATOR)

$(BUILD_DIR) $(OUTPUT_DIR):
	$(MKDIR)

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
	$(CLEAN)
