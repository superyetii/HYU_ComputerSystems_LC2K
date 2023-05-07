	lw	0	2	mcand
	lw	0	3	mplier
	lw	0	4	max
	lw	0	5	filt
loop	nor	2	5	7
	beq	0	7	mult
	beq	0	0	mark
mult	add	1	3	1
mark	lw	0	7	neg1
	add	4	7	4
	beq	0	4	end
	add	3	3	3
	add	5	5	5
	lw	0	7	pos1
	add	5	7	5
	beq	0	0	loop
end	halt
neg1	.fill	-1
pos1	.fill	1
max	.fill	15
filt	.fill	-2
mcand	.fill	12328
mplier	.fill	32766
