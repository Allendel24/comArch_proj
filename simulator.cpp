#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio>

using namespace std;

// กำหนดขนาดของระบบ : Memory มี 65536 ช่อง และมีที่เก็บข้อมูลพักชั่วคราว รีจิสเตอร์ 8 ตัว
#define NUMMEMORY 65536
#define NUMREGS 8

// สร้างโครงสร้าง stateStruct ไว้ดูว่าตอนนี้ทำงานถึงบรรทัดไหน (pc), Memory มีอะไรบ้าง (mem), รีจิสเตอร์มีค่าเท่าไหร่ (reg)
typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);

// ฟังก์ชันนี้เอาไว้จัดการ เลขติดลบ ให้ทำงานในคอมได้อย่างถูกต้อง
int convertNum(int num) {
    if (num & (1 << 15)) {
        num -= (1 << 16);
    }
    return num;
}

int main(int argc, char *argv[]) {
    // เช็คก่อนว่าตอนสั่งรันโปรแกรม มี machine code มาด้วยไหม
    if (argc != 2) {
        cerr << "error: usage: " << argv[0] << " <machine-code file>\n";
        exit(1);
    }

    // เปิด machine code
    ifstream filePtr(argv[1]);
    if (!filePtr) {
        cerr << "error: can't open file " << argv[1] << '\n';
        exit(1);
    }

    stateType state;
    state.numMemory = 0;

    // อ่านตัวเลขในไฟล์ทีละบรรทัด มาเรียงเก็บไว้ใน Memory
    while (filePtr >> state.mem[state.numMemory]) {
        state.numMemory++;
        
        // ถ้าไฟล์มีขนาดเกินขนาด Memory ให้หยุดอ่าน
        if (state.numMemory >= NUMMEMORY) {
            break;
        }
    }
    filePtr.close();
    
    // รีเซ็ตเครื่องก่อนเริ่มทำงานจริง : เริ่มที่บรรทัดแรก (pc = 0) และล้างค่ารีจิสเตอร์ทั้งหมดให้เป็น 0
    state.pc = 0;
    for (int i = 0; i < NUMREGS; i++) {
        state.reg[i] = 0;
    }
    int instrCount = 0; // นับว่าทำงานไปกี่คำสั่งแล้ว

    // วงจรการทำงานหลักของเครื่อง (จะวนไปเรื่อย ๆ จนกว่าจะเจอคำสั่งหยุด)
    while (true) {
        // โชว์ stateStruct ของระบบก่อนทำงานทุกครั้ง
        printState(&state);

        // เช็คความปลอดภัย : ห้ามเครื่องกระโดดไปอ่าน Memory นอกกรอบ
        if (state.pc < 0 || state.pc >= NUMMEMORY) {
            cerr << "error: pc out of bounds\n";
            exit(1);
        }

        // ดึง คำสั่ง จาก Memory บรรทัดปัจจุบัน (pc) ออกมา
        int instr = state.mem[state.pc];

        // แกะรหัส คำสั่งนั้น (จะเป็นตัวเลขก้อนยาวๆ ต้องตัดแบ่งเป็นส่วนๆ)
        int opcode = (instr >> 22) & 0x7;
        int rs = (instr >> 19) & 0x7;
        int rt = (instr >> 16) & 0x7;
        int rd = instr & 0x7;
        int imm16 = instr & 0xFFFF;
        
        int addr; // ตัวแปรสำหรับเช็คตำแหน่ง Address ใน Memory

        // เริ่มลงมือทำตามชนิดของคำสั่ง (opcode)
        switch (opcode) {
            case 0: // add: เอาข้อมูลมา บวก กัน
                state.reg[rd] = state.reg[rs] + state.reg[rt];
                state.pc += 1;
                break;

            case 1: // nand: เอาข้อมูลมาเทียบกันแบบ NAND
                state.reg[rd] = ~(state.reg[rs] & state.reg[rt]);
                state.pc += 1;
                break;

            case 2: // lw: ดึง ข้อมูลจาก Memory มาเก็บในรีจิสเตอร์
                addr = state.reg[rs] + convertNum(imm16);
                if (addr < 0 || addr >= NUMMEMORY) {
                    cerr << "Error: memory address out of bounds\n";
                    exit(1);
                }
                state.reg[rt] = state.mem[addr];
                state.pc += 1;
                break;

            case 3: // sw: บันทึก ข้อมูลจากรีจิสเตอร์ ลงไปในเมมโมรี่
                addr = state.reg[rs] + convertNum(imm16);
                if (addr < 0 || addr >= NUMMEMORY) {
                    cerr << "Error: memory address out of bounds\n";
                    exit(1);
                }
                state.mem[addr] = state.reg[rt];
                state.pc += 1;
                break;

            case 4: // beq: กระโดด ถ้าข้อมูลสองตัวเท่ากัน
                if (state.reg[rs] == state.reg[rt]) {
                    state.pc = state.pc + 1 + convertNum(imm16); // กระโดดไปบรรทัดเป้าหมาย
                } else {
                    state.pc += 1; // ถ้าไม่เท่ากัน ก็ทำบรรทัดถัดไปปกติ
                }
                break;

            case 5: // jalr: กระโดดและจำทางกลับ
                {
                    int nextPC = state.pc + 1;
                    state.reg[rt] = nextPC;   // เก็บ PC+1 ไว้ใน regB ก่อน
                    state.pc = state.reg[rs]; // แล้วค่อยกระโดดไปที่ address ใน regA
                }
                break;

            case 6: // halt: สั่งหยุดเครื่อง
                state.pc += 1;
                state.reg[0] = 0;
                instrCount++;
                cout << "machine halted\n";
                cout << "total of " << instrCount << " instructions executed\n";
                cout << "final state of machine:\n";
                printState(&state); // โชว์สถานะตอนจบ ก่อน exit
                exit(0); // ออกจากโปรแกรมไปเลย
                break;

            case 7: // noop: ไม่ต้องทำอะไร
                state.pc += 1;
                break;

            default: // เจอรหัสคำสั่งที่เครื่องไม่รู้จัก
                cerr << "error: invalid opcode\n";
                exit(1);
        }

        // รีจิสเตอร์ตัวแรก (reg[0]) ต้องเป็น 0 เสมอ ห้ามแอบเปลี่ยนค่า!
        state.reg[0] = 0;
        instrCount++;
    }

    return 0;
}

// ฟังก์ชันสำหรับปริ้น stateStruct ของระบบออกมาดูบนหน้าจอ
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