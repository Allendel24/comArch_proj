        lw      0       1       n           # โหลดพารามิเตอร์ n และ r
        lw      0       2       r
        lw      0       6       combAdr
        jalr    6       7                   # กระโดดไปฟังก์ชัน comb โดยให้ reg7 เก็บ return address
        halt
comb    lw      0       6       pos1        # --- Prologue: จัดการ Frame บน Stack ---
        sw      5       7       stack       # เซฟ return address ก่อน เพราะเดี๋ยวมี nested call ทับ reg7
        add     5       6       5
        sw      5       1       stack       # เซฟ n
        add     5       6       5
        sw      5       2       stack       # เซฟ r
        add     5       6       5
        sw      5       4       stack       # เซฟ reg4 (เอาไว้พักผลลัพธ์แรก)
        add     5       6       5
        beq     2       0       base        # เช็ค base cases: r == 0 หรือ n == r
        beq     1       2       base
        lw      0       6       neg1        # เตรียมค่า -1 ไว้ใช้ลดค่า
        add     1       6       1           # คำนวณเทอมแรก: C(n-1, r)
        lw      0       6       combAdr
        jalr    6       7
        add     3       0       4           # พักผลของ C(n-1, r) ลง reg4 ก่อนโดนเทอมสองทับ
        lw      0       6       neg1
        add     2       6       2           # คำนวณเทอมสอง: C(n-1, r-1)
        lw      0       6       combAdr
        jalr    6       7
        add     3       4       3           # รวมคำตอบ: reg3 = เทอมแรก + เทอมสอง
        beq     0       0       ret
base    lw      0       3       pos1        # เข้า base case ให้คืนค่า 1
ret     lw      0       6       neg1        # --- Epilogue: คืนค่ารีจิสเตอร์จาก Stack (LIFO) ---
        add     5       6       5
        lw      5       4       stack
        add     5       6       5
        lw      5       2       stack
        add     5       6       5
        lw      5       1       stack
        add     5       6       5
        lw      5       7       stack       # ดึง return address เดิมกลับมา
        jalr    7       6                   # กระโดดกลับคนเรียก
pos1    .fill   1
neg1    .fill   -1
combAdr .fill   comb
n       .fill   7
r       .fill   3
stack   .fill   0
        .fill   0
        .fill   0
        .fill   0
        .fill   0
        .fill   0
        .fill   0
        .fill   0