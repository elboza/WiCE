	org cic
var:
	dat #0,#99
cic:
	mov.i *var,@var
	cmp.ab var,#10
	jmp ne
	jmp endcopy
ne:	add.a #1,var
	add.ab #1,var
	jmp cic
endcopy:
	mov #99,91
	jmp 91

