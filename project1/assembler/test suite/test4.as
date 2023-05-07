        lw      0       1       init_a   ; Load reg1 with initial value of a (5)
        lw      0       2       init_b   ; Load reg2 with initial value of b (10)
        lw      0       3       sub_addr ; Load reg3 with the address of the subroutine
        jalr    3       4               ; Jump to subroutine and store return address in reg4
main    halt                            ; Halt the program

; Subroutine to add reg1 and reg2, store result in reg5, and return
sub_add sw      1       5       res_add  ; Store reg1 + reg2 in memory location res_add
        jalr    4       0               ; Return to main program

; Memory locations
init_a  .fill   5                       ; Initial value of a
init_b  .fill   10                      ; Initial value of b
res_add .fill   0                       ; Memory location for storing addition result
sub_addr .fill  sub_add                 ; Address of the subroutine
sub_addr .fill  sub_add                 ; Duplicate definition of labels
