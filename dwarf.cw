assert CORESIZE % 4==0

org loop

loop: add.ab #4,bomb
mov.i bomb,@bomb
jmp loop
bomb: dat #0,#0

