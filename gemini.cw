len	equ fin-var
	org cic
var:
	dat #0,#99
cic:
	mov.i *var,@var
	cmp.ab var,#len
	jmp ne
	jmp endcopy
ne:	add.a #1,var
	add.ab #1,var
	jmp cic
endcopy:
	;mov #99,91
	;nop
	jmp 92
fin: dat #0,#0
