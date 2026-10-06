#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUMMEMORY 65536
#define NUMREGS 8
#define MAXLINELENGTH 1000

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);

/* แปลงเลข offset 16 บิตให้เป็น signed int (Sign Extension) */
int signExtend16(int num)
{
    if (num & (1 << 15)) {
        num -= (1 << 16);
    }
    return num;
}

int main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    static stateType state;
    FILE *filePtr;
    int count = 0, halted = 0;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    /* โหลด machine code เข้า memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL; state.numMemory++) {
        if (sscanf(line, "%d", state.mem + state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            fclose(filePtr);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }
    fclose(filePtr);

    state.pc = 0;

    /* จำลองวงรอบการทำงานของ CPU */
    while (!halted) {
        int inst, op, a, b, d, off;

        if (state.pc < 0 || state.pc >= NUMMEMORY) {
            printf("error: pc out of range\n");
            exit(1);
        }

        printState(&state);

        inst = state.mem[state.pc];
        count++;
        state.pc++;

        op  = (inst >> 22) & 7;
        a   = (inst >> 19) & 7;
        b   = (inst >> 16) & 7;
        d   = inst & 7;
        off = signExtend16(inst & 0xFFFF);

        switch (op) {
            case 0: /* add: reg[d] = reg[a] + reg[b] */
                state.reg[d] = state.reg[a] + state.reg[b];
                break;

            case 1: /* nand: reg[d] = ~(reg[a] & reg[b]) */
                state.reg[d] = ~(state.reg[a] & state.reg[b]);
                break;

            case 2: /* lw: reg[b] = mem[reg[a] + offset] */
                state.reg[b] = state.mem[state.reg[a] + off];
                break;

            case 3: /* sw: mem[reg[a] + offset] = reg[b] */
                state.mem[state.reg[a] + off] = state.reg[b];
                break;

            case 4: /* beq: if (reg[a] == reg[b]) pc += offset */
                if (state.reg[a] == state.reg[b]) {
                    state.pc += off;
                }
                break;

            case 5: { /* jalr: reg[b] = pc; pc = reg[a] */
                int next_addr = state.reg[a];
                state.reg[b] = state.pc;
                state.pc = next_addr;
                break;
            }

            case 6: /* halt */
                halted = 1;
                break;

            case 7: /* noop */
                break;

            default:
                printf("error: unknown opcode %d\n", op);
                exit(1);
        }
    }

    printf("machine halted\n");
    printf("total of %d instructions executed\n", count);
    printf("final state of machine:\n");
    printState(&state);

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