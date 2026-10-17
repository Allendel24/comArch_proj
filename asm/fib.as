lw      0       1       v0              # load re1 = 0
lw      0       2       v1              # load re2 = 1
lw      0       3       n               # load re3 = n = 5
lw      0       5       neg1            # load re5 = -1
loop    beq     3       0       done    # goto  //done if re3 = 0
        add     1       2       4       # re4 = re1 + re2   -> X = A + B
        add     2       0       1       # re1 = re2 + 0     -> A = B
        add     4       0       2       # re2 = re4 + 0     -> X = B
        add     3       5       3       # re3 = re3 + (-1)
        beq     0       0       loop    # go to //loop
done    sw      0       1       sum     # store the result re1 -> sum
        halt
v0      .fill   0
v1      .fill   1
neg1    .fill   -1
n       .fill   27       # find the fib(5) number
sum     .fill   0