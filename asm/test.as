lw 0 1 a
lw 0 2 b
lw 0 3 p
lw 0 7 max
func1 nand 1 2 3
nand 3 3 3
beq 3 0 func2
add 2 2 2
func2 add 3 3 3
add 1 1 1
beq 

finish halt
a .fill 2
b .fill 3
p .fill 1
max .fill 32768