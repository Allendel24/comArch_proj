        lw      0       2       mcand       # reg2: ตัวตั้ง (จะเบิ้ลค่า x2 ไปเรื่อยๆ ตอนเช็คบิต)
        lw      0       3       mplier      # reg3: ตัวคูณ
        lw      0       4       one         # reg4: ตัวเช็คบิต (เริ่มที่บิต 0)
        lw      0       6       limit       # หลุดลูปที่ 2^15 เพราะเลขบวก LC-2K คิดแค่ 15 บิต
        add     0       0       1           # เคลียร์ reg1 เก็บผลลัพธ์รวม
loop    nand    3       4       5           # LC-2K ไม่มี AND ตรงๆ ต้องทำ NAND สองรอบ
        nand    5       5       5           # ได้ reg5 = mplier & mask
        beq     5       0       skip        # บิตปัจจุบันเป็น 0 -> ข้ามไป ไม่ต้องบวก
        add     1       2       1           # ถ้าบิตเป็น 1 ค่อยบวกตัวตั้งเข้าไปในผลลัพธ์
skip    add     2       2       2           # เลื่อนตัวตั้งไปทางซ้าย 1 บิต (บวกตัวเอง)
        add     4       4       4           # เลื่อน mask ไปบิตถัดไป
        beq     4       6       done        # ครบ 15 บิต (mask ชน limit) จบการคูณ
        beq     0       0       loop
done    halt
mcand   .fill   32766
mplier  .fill   10383
one     .fill   1
limit   .fill   32768