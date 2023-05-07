        lw      0       1       init_val ; Load reg1 with initial value (5)
        lw      0       2       neg_one  ; Load reg2 with -1
        lw      0       3       zero     ; Load reg3 with 0
loop    sub     1       2       1        ; invalid opcode
        beq     1       3       exit     ; Branch to label "exit" if reg1 equals reg3 (loop ends)
        sw      1       4       arr      ; Store reg4 value in the memory address (arr + reg1)
        beq     0       0       loop     ; Branch back to label "loop"
exit    halt                            ; Halt the program
init_val .fill  5                       ; Initial value
neg_one  .fill -1                       ; -1 value for decrementing
zero     .fill  0                       ; Zero value for comparison
arr      .fill  0 0 0 0 0               ; Array of 5 elements
