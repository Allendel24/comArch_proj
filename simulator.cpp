#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio>

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
        std::cerr << "error: usage: " << argv[0] << " <machine-code file>\n";
        std::exit(1);
    }

    // เปิด machine code
    std::ifstream filePtr(argv[1]);
    if (!filePtr) {
        std::cerr << "error: can't open file " << argv[1] << '\n';
        std::exit(1);
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
            std::cerr << "error: pc out of bounds\n";
            std::exit(1);
        }

        // ดึง คำสั่ง จาก Memory บรรทัดปัจจุบัน (pc) ออกมา
        int instr = state.mem[state.pc];

        // แกะรหัส คำสั่งนั้น (จะเป็นตัวเลขก้อนยาวๆ ต้องตัดแบ่งเป็นส่วนๆ)
        int opcode = (instr >> 22) & 0x7; // เลื่อนตัวเลขไปทางขวา 22 ก้าว แล้วตัดเอามาแค่ 3 ตัวท้ายสุด ชนิดของคำสั่ง (ทำอะไรดี?)
        int rs = (instr >> 19) & 0x7;     // เลื่อนตัวเลขไปขวา 19 ก้าว แล้วตัดมา 3 ตัวท้าย (ใช้ข้อมูลจากตัวไหน)
        int rt = (instr >> 16) & 0x7;     // เลื่อนไปขวา 16 ก้าว แล้วตัดมา 3 ตัวท้าย (ใช้ข้อมูลจากตัวไหน)
        int rd = instr & 0x7;             // ไม่ต้องเลื่อนทิ้ง อยู่ริมขวาสุดอยู่แล้ว ตัด 3 ตัวท้ายออกมาได้เลย (เก็บผลลัพธ์ไว้ที่ไหน)
        int imm16 = instr & 0xFFFF;       // ม่ต้องเลื่อน แต่เปลี่ยนแม่พิมพ์ให้ใหญ่ขึ้นเป็นตัดเอา 16 ตัวท้ายออกมาเลย (ตัวเลขพิเศษ)

        // เริ่มลงมือทำตามชนิดของคำสั่ง (opcode)
        switch (opcode) {
            case 0: // add: เอาข้อมูลมา บวก กัน
                state.reg[rd] = state.reg[rs] + state.reg[rt];
                state.pc += 1; // ขยับไปทำบรรทัดถัดไป
                break;

            case 1: // nand: เอาข้อมูลมาเทียบกันแบบ NAND
                state.reg[rd] = ~(state.reg[rs] & state.reg[rt]);
                state.pc += 1;
                break;

            case 2: // lw: ดึง ข้อมูลจาก Memory มาเก็บในรีจิสเตอร์
                state.reg[rt] = state.mem[state.reg[rs] + convertNum(imm16)];
                state.pc += 1;
                break;

            case 3: // sw: บันทึก ข้อมูลจากรีจิสเตอร์ ลงไปในเมมโมรี่
                state.mem[state.reg[rs] + convertNum(imm16)] = state.reg[rt];
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
                    state.pc = state.reg[rs];
                    state.reg[rt] = nextPC;
                }
                break;

            case 6: // halt: สั่งหยุดเครื่อง
                state.pc += 1;
                state.reg[0] = 0;
                instrCount++;
                printState(&state); // โชว์สถานะตอนจบ
                std::cout << "machine halted\n";
                std::cout << "total of " << instrCount << " instructions executed\n";
                std::cout << "final state of machine:\n";
                std::exit(0); // ออกจากโปรแกรมไปเลย
                break;

            case 7: // noop: ไม่ต้องทำอะไร
                state.pc += 1;
                break;

            default: // เจอรหัสคำสั่งที่เครื่องไม่รู้จัก
                std::cerr << "error: invalid opcode\n";
                std::exit(1);
        }

        // กฎเหล็กของเครื่องนี้: รีจิสเตอร์ตัวแรก (reg[0]) ต้องเป็น 0 เสมอ ห้ามแอบเปลี่ยนค่า!
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