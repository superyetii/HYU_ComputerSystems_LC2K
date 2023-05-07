        lw      0   1   var           load reg1 with var(3) (symbolic address)
        lw      0   2   dec             load reg2 with dec(10) (symbolic address)
        add     0   0   3               load reg3 with 0 (numeric address)
        lw      0   4   sub            load reg4 with -1 (symbolic address)
        lw      0   5   defau            load reg5 with address of def but error undefined label***
        jalr    5   6                   stores address of sw to reg6 and go to def
        sw      0   3   100             save reg3 to offset 100
        halt
def    add     3   1   3               add var to reg3 saves it to reg3
        add     2   4   2               decrement reg2
        beq     0   2   end            if dec reaches 0 go to halt
        beq     0   0   def
end    jalr    6   7
defad   .fill    def
dec    .fill    10
var  .fill    3
sub   .fill    -1
