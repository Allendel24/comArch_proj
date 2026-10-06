#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000
#define MAXLABELS 1000

typedef enum {
    OP_ADD  = 0,
    OP_NAND = 1,
    OP_LW   = 2,
    OP_SW   = 3,
    OP_BEQ  = 4,
    OP_JALR = 5,
    OP_HALT = 6,
    OP_NOOP = 7,
    OP_FILL = 8,
    OP_UNKNOWN = -1
} OpcodeType;

int Read(FILE *, char *, char *, char *, char *, char *);
int Num(char *);
OpcodeType getOpcodeType(const char *);

char labels[MAXLABELS][MAXLINELENGTH];
int  addrs[MAXLABELS];
int  numLabels = 0;

int findLabel(char *name)
{
    int i;
    for (i = 0; i < numLabels; i++){
        if (!strcmp(labels[i], name)){
             return addrs[i];
        }
    }
    printf("error: undefined label %s\n", name);
    exit(1);
}

int value(char *s)            
{
    return Num(s) ? atoi(s) : findLabel(s);
}

int reg(char *s)
{
    /* 1. เช็คก่อนว่าค่าที่ส่งมาเป็นตัวเลขหรือไม่ */
    if (!Num(s)) {
        printf("error: register is not a number: %s\n", s);
        exit(1);
    }

    /* 2. แปลงสตริงเป็นตัวเลข */
    int r = atoi(s);

    /* 3. เช็คขอบเขตหมายเลขรีจิสเตอร์ของ LC-2K (ต้องอยู่ระหว่าง 0 ถึง 7 เท่านั้น) */
    if (r < 0 || r > 7) {
        printf("error: register out of range [0-7]: %d\n", r);
        exit(1);
    }

    return r;
}

int checkOffset(int off)
{
    if (off < -32768 || off > 32767) {
        printf("error: offset %d does not fit in 16 bits\n", off);
        exit(1);
    }
    return off & 0xFFFF;
}

OpcodeType getOpcodeType(const char *op)
{
    if (strcmp(op, "add") == 0)   return OP_ADD;
    if (strcmp(op, "nand") == 0)  return OP_NAND;
    if (strcmp(op, "lw") == 0)    return OP_LW;
    if (strcmp(op, "sw") == 0)    return OP_SW;
    if (strcmp(op, "beq") == 0)   return OP_BEQ;
    if (strcmp(op, "jalr") == 0)  return OP_JALR;
    if (strcmp(op, "halt") == 0)  return OP_HALT;
    if (strcmp(op, "noop") == 0)  return OP_NOOP;
    if (strcmp(op, ".fill") == 0) return OP_FILL;
    return OP_UNKNOWN;
}

int main(int argc, char *argv[])
{
    FILE *in, *out;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
         arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    int pc, i, code;

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n", argv[0]);
        exit(1);
    }
    in = fopen(argv[1], "r");
    if (in == NULL) {
        printf("error in opening %s\n", argv[1]);
        exit(1);
    }

    out = fopen(argv[2], "w");
    if (out == NULL) {
        printf("error in opening %s\n", argv[2]);
        fclose(in);
        exit(1);
    }
    
    /* pass 1: collect labels */
    pc = 0;
    while (Read(in, label, opcode, arg0, arg1, arg2)) {
        if (opcode[0] == '\0') {
            continue; /* blank line */
        }

        if (label[0] != '\0') {
            for (i = 0; i < numLabels; i++) {
                if (!strcmp(labels[i], label)) {
                    printf("error: duplicate label %s\n", label);
                    exit(1);
                }
            }
            if (numLabels >= MAXLABELS) {
                printf("error: too many labels\n");
                exit(1);
            }
            strcpy(labels[numLabels], label);
            addrs[numLabels++] = pc;
        }
        pc++;
    }

    /* pass 2: generate machine code */
    rewind(in);
    pc = 0;
    while (Read(in, label, opcode, arg0, arg1, arg2)) {
        if (opcode[0] == '\0') {
            continue;
        }

        OpcodeType op = getOpcodeType(opcode);

        switch (op) {
            case OP_FILL:
                code = value(arg0);
                break;

            case OP_ADD:
            case OP_NAND:
                code = (op << 22) | (reg(arg0) << 19) | (reg(arg1) << 16) | reg(arg2);
                break;

            case OP_LW:
            case OP_SW:
                code = (op << 22) | (reg(arg0) << 19) | (reg(arg1) << 16) | checkOffset(value(arg2));
                break;

            case OP_BEQ: {
                int off = Num(arg2) ? atoi(arg2) : findLabel(arg2) - (pc + 1);
                code = (OP_BEQ << 22) | (reg(arg0) << 19) | (reg(arg1) << 16) | checkOffset(off);
                break;
            }

            case OP_JALR:
                code = (OP_JALR << 22) | (reg(arg0) << 19) | (reg(arg1) << 16);
                break;

            case OP_HALT:
                code = OP_HALT << 22;
                break;

            case OP_NOOP:
                code = OP_NOOP << 22;
                break;

            default:
                printf("error: unrecognized opcode %s\n", opcode);
                exit(1);
        }

        fprintf(out, "%d\n", code);
        pc++;
    }

    fclose(in);
    fclose(out);
    return 0;
}

int Read(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) return 0;
    if (strchr(line, '\n') == NULL) {
        printf("error: line too long\n");
        exit(1);
    }
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) ptr += strlen(label);
    sscanf(ptr, "%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]%*[\t\n ]%[^\t\n ]",
        opcode, arg0, arg1, arg2);
    return 1;
}

int Num(char *string)
{
    int i;
    return (sscanf(string, "%d", &i)) == 1;
}
