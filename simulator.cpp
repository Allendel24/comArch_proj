#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio> 

#define NUMMEMORY 65536 
#define NUMREGS 8 

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);

// เพิ่มฟังก์ชันสำหรับทำ Sign-extension ให้กับ immediate 16-bit
int convertNum(int num) {
    if (num & (1 << 15)) {
        num -= (1 << 16);
    }
    return num;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "error: usage: " << argv[0] << " <machine-code file>\n";
        std::exit(1);
    }

    std::ifstream filePtr(argv[1]);
    if (!filePtr) {
        std::cerr << "error: can't open file " << argv[1] << '\n';
        std::exit(1);
    }

    stateType state;
    state.numMemory = 0;

    while (filePtr >> state.mem[state.numMemory]) {
        // std::cout << "memory[" << state.numMemory << "]=" << state.mem[state.numMemory] << '\n';
        state.numMemory++;
        
        if (state.numMemory >= NUMMEMORY) {
            break;
        }
    }
    filePtr.close();
    
    // --- เริ่มต้นส่วน Initialization ---
    state.pc = 0;
    for (int i = 0; i < NUMREGS; i++) {
        state.reg[i] = 0;
    }
    int instrCount = 0;

    // --- เริ่มต้น Simulation Loop ---
    while (true) {
        // 1. พิมพ์สถานะก่อน Execute ทุกครั้ง
        printState(&state);

        // ป้องกันการเข้าถึง Memory นอกขอบเขต
        if (state.pc < 0 || state.pc >= NUMMEMORY) {
            std::cerr << "error: pc out of bounds\n";
            std::exit(1);
        }

        // 2. Fetch คำสั่ง
        int instr = state.mem[state.pc];

        // 3. Decode คำสั่ง (แกะบิต)
        int opcode = (instr >> 22) & 0x7;
        int rs = (instr >> 19) & 0x7;
        int rt = (instr >> 16) & 0x7;
        int rd = instr & 0x7;
        int imm16 = instr & 0xFFFF;

        // 4. Execute คำสั่งตาม Opcode
        switch (opcode) {
            case 0: // add
                state.reg[rd] = state.reg[rs] + state.reg[rt];
                state.pc += 1;
                break;

            case 1: // nand
                state.reg[rd] = ~(state.reg[rs] & state.reg[rt]);
                state.pc += 1;
                break;

            case 2: // lw
                state.reg[rt] = state.mem[state.reg[rs] + convertNum(imm16)];
                state.pc += 1;
                break;

            case 3: // sw
                state.mem[state.reg[rs] + convertNum(imm16)] = state.reg[rt];
                state.pc += 1;
                break;

            case 4: // beq
                if (state.reg[rs] == state.reg[rt]) {
                    state.pc = state.pc + 1 + convertNum(imm16);
                } else {
                    state.pc += 1;
                }
                break;

            case 5: // jalr
                { 
                    // ใช้ block scope {...} เพื่อสร้างตัวแปรใน switch-case
                    int nextPC = state.pc + 1;
                    state.pc = state.reg[rs];
                    state.reg[rt] = nextPC;
                }
                break;

            case 6: // halt
                state.pc += 1;
                state.reg[0] = 0; // บังคับให้ reg[0] เป็น 0 เสมอก่อนพิมพ์สถานะสุดท้าย
                instrCount++;
                printState(&state);
                std::cout << "machine halted\n";
                std::cout << "total of " << instrCount << " instructions executed\n";
                std::cout << "final state of machine:\n";
                std::exit(0);
                break;

            case 7: // noop
                state.pc += 1;
                break;

            default:
                std::cerr << "error: invalid opcode\n";
                std::exit(1);
        }

        // 5. บังคับกฎ Hardware Constraint และนับคำสั่ง
        state.reg[0] = 0; // reg[0] ต้องเป็น 0 เสมอ
        instrCount++;
    }

    return 0;
}

void printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i = 0; i < statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i = 0; i < NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}