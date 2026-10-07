        lw      0       1       five    # load reg1 with 5
        lw      1       2       3       # load reg2 with -1
start   add     1       2       1       # decrement reg1
        beq     0       1       2       # stop when reg1 is zero
        beq     0       0       start   # repeat the loop
        noop
done    halt
five    .fill   5
neg1    .fill   -1
stAddr  .fill   start
