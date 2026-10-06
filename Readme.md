# LC-2K Assembler & Simulator

โปรเจกต์นี้เป็นการจำลองระบบประมวลผลคำสั่งของสถาปัตยกรรมคอมพิวเตอร์ LC-2K (32-bit, 8 registers) ประกอบด้วยตัวแปลงภาษา Assembly เป็น Machine code และตัวโปรแกรมจำลอง CPU

---

## ไฟล์ในโปรเจกต์

* `assembler.c` : ตัวแปลภาษา assembly (`.as`) ออกมาเป็น machine code ฐานสิบ (`.mc`) ทำงานแบบ 2-pass
* `simulator.c` : ตัวจำลองการทำงานของฮาร์ดแวร์ อ่าน machine code เข้า memory แล้ว fetch-decode-execute ตามลำดับ
* `mult.as` : โปรแกรมคำนวณการคูณเลขโดยใช้วิธี shift-and-add (ใช้คำสั่ง nand ช่วยเช็คบิต)
* `comb.as` : โปรแกรมคำนวณ Combination $C(n, r)$ แบบ recursive โดยจำลอง stack ด้วย pointer บน memory

---

## วิธีคอมไพล์และรันโปรแกรม

### 1. คอมไพล์โค้ด C
ใช้คำสั่ง gcc ตามปกติ:
```bash
gcc -o assembler assembler.c
gcc -o simulator simulator.c
```

### 2. แปลง Assembly เป็น Machine Code
รัน assembler โดยส่งไฟล์ `.as` และตั้งชื่อไฟล์ `.mc` ปลายทาง:
```bash
# ตัวอย่างรันโปรแกรมคูณ
./assembler mult.as mult.mc

# ตัวอย่างรันโปรแกรม Combination
./assembler comb.as comb.mc
```

### 3. รันบน Simulator
รันไฟล์ `.mc` เพื่อดูผลการประมวลผลและค่าในรีจิสเตอร์:
```bash
# รันโปรแกรมคูณ
./simulator mult.mc

# รันโปรแกรม combination (บันทึกออกไฟล์เพราะ output ยาว)
./simulator comb.mc > output.txt
```

---

## ผลลัพธ์ที่ควรได้
* `mult.mc` : ผลคูณ $32766 \times 10383$ ได้ค่าเก็บใน `reg[1]` เท่ากับ **340209378**
* `comb.mc` : คำนวณ $C(7, 3)$ ได้ผลลัพธ์ใน `reg[3]` เท่ากับ **35**